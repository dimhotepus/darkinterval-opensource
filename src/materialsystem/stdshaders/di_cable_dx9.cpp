//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: A wet version of base * lightmap
//
// $Header: $
// $NoKeywords: $
//===========================================================================//

#include "BaseVSShader.h"
#ifdef DARKINTERVAL
#define FLASHLIGHT // DI - adding cables flashlight
#endif
#ifdef FLASHLIGHT
#include "di_cable_vs20.inc"
#include "di_cable_ps20.inc"
#include "di_cable_ps20b.inc"
#else
#include "cable_vs20.inc"
#include "cable_ps20.inc"
#include "cable_ps20b.inc"
#endif
#include "cpp_shader_constant_register_map.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar mat_fullbright;
#ifdef DARKINTERVAL
DEFINE_FALLBACK_SHADER( DI_Cable, DI_Cable_DX90)
BEGIN_VS_SHADER(DI_Cable_DX90,
			  "Help for Cable shader" )
#else
DEFINE_FALLBACK_SHADER(Cable, Cable_DX9)
BEGIN_VS_SHADER(Cable_DX9,
	"Help for Cable shader")
#endif
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "cable/cablenormalmap", "bumpmap texture" )
		SHADER_PARAM( MINLIGHT, SHADER_PARAM_TYPE_FLOAT, "0.1", "Minimum amount of light (0-1 value)" )
		SHADER_PARAM( MAXLIGHT, SHADER_PARAM_TYPE_FLOAT, "0.3", "Maximum amount of light" )
	END_SHADER_PARAMS
	
	SHADER_FALLBACK
	{
		if ( !(g_pHardwareConfig->SupportsPixelShaders_2_0() && g_pHardwareConfig->SupportsVertexShaders_2_0()) ||
				(g_pHardwareConfig->GetDXSupportLevel() < 90) )
		{
			return "Cable_DX8";
		}
		return 0;
	}
#ifdef FLASHLIGHT
	SHADER_INIT_PARAMS()
	{
		CLEAR_FLAGS(MATERIAL_VAR2_LIGHTING_UNLIT);
		SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_FLASHLIGHT);
		SET_FLAGS2(MATERIAL_VAR2_USE_FLASHLIGHT);
		SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);
	}
#endif
	SHADER_INIT
	{
		LoadBumpMap( BUMPMAP );
		LoadTexture( BASETEXTURE, TEXTUREFLAGS_SRGB );
#ifdef FLASHLIGHT
		if (g_pHardwareConfig->SupportsBorderColor())
		{
			params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight_border");
		}
		else
		{
			params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight001");
		}

		if (params[FLASHLIGHTTEXTURE]->IsDefined())
		{
			LoadTexture(FLASHLIGHTTEXTURE, TEXTUREFLAGS_SRGB);
		}
#endif
	}
		
	void DrawCableShader(IMaterialVar** params,	IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, bool hasFlashlight)
	{
#ifdef FLASHLIGHT
		bool bFlashlight = hasFlashlight;
#endif
		BlendType_t nBlendType = EvaluateBlendRequirements(BASETEXTURE, true);
		bool bFullyOpaque = (nBlendType != BT_BLENDADD) && (nBlendType != BT_BLEND) && !IS_FLAG_SET(MATERIAL_VAR_ALPHATEST); //dest alpha is free for special use

		SHADOW_STATE
		{
			// Enable blending?
			if (IS_FLAG_SET(MATERIAL_VAR_TRANSLUCENT))
			{
				pShaderShadow->EnableDepthWrites(false);
				pShaderShadow->EnableBlending(true);
				pShaderShadow->BlendFunc(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);
			}

			pShaderShadow->EnableAlphaTest(IS_FLAG_SET(MATERIAL_VAR_ALPHATEST));

			pShaderShadow->EnableTexture(SHADER_SAMPLER0, true);
			pShaderShadow->EnableTexture(SHADER_SAMPLER1, true);
			if (g_pHardwareConfig->GetDXSupportLevel() >= 90)
			{
				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER1, true);
			}

			int tCoordDimensions[] = { 2,2 };
			pShaderShadow->VertexShaderVertexFormat(
				VERTEX_POSITION | VERTEX_COLOR | VERTEX_TANGENT_S | VERTEX_TANGENT_T | VERTEX_NORMAL,
				2, tCoordDimensions, 0);
#ifdef FLASHLIGHT
			if (bFlashlight)
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER7, true); // flashlight sampler
				pShaderShadow->EnableTexture(SHADER_SAMPLER8, true); // depth sampler
				pShaderShadow->SetShadowDepthFiltering(SHADER_SAMPLER8);
			}
#endif
#ifdef FLASHLIGHT
			DECLARE_STATIC_VERTEX_SHADER(di_cable_vs20);
			SET_STATIC_VERTEX_SHADER(di_cable_vs20);

			if (g_pHardwareConfig->SupportsPixelShaders_2_b())
			{
				DECLARE_STATIC_PIXEL_SHADER(di_cable_ps20b);
//				SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bFlashlight);
				SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHTDEPTHFILTERMODE, 0);
				SET_STATIC_PIXEL_SHADER(di_cable_ps20b);
			}
			else
			{
				DECLARE_STATIC_PIXEL_SHADER(di_cable_ps20);
//				SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bFlashlight);
				SET_STATIC_PIXEL_SHADER(di_cable_ps20);
			}
#else
			DECLARE_STATIC_VERTEX_SHADER(cable_vs20);
			SET_STATIC_VERTEX_SHADER(cable_vs20);

			if ( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				DECLARE_STATIC_PIXEL_SHADER(cable_ps20b);
				SET_STATIC_PIXEL_SHADER(cable_ps20b);
			} 
			else
			{
				DECLARE_STATIC_PIXEL_SHADER(cable_ps20);
				SET_STATIC_PIXEL_SHADER(cable_ps20);
			}
#endif // FLASHLIGHT
			// we are writing linear values from this shader.
			// This is kinda wrong.  We are writing linear or gamma depending on "IsHDREnabled" below.
			// The COLOR really decides if we are gamma or linear.  
			pShaderShadow->EnableSRGBWrite(true);

			FogToFogColor();

			pShaderShadow->EnableAlphaWrites(bFullyOpaque);
		}
		DYNAMIC_STATE
		{
			bool bLightingOnly = mat_fullbright.GetInt() == 2 && !IS_FLAG_SET(MATERIAL_VAR_NO_DEBUG_OVERRIDE);

			BindTexture(SHADER_SAMPLER0, BUMPMAP);
			if (bLightingOnly)
			{
				pShaderAPI->BindStandardTexture(SHADER_SAMPLER1, TEXTURE_GREY);

			}
			else
			{
				BindTexture(SHADER_SAMPLER1, BASETEXTURE);
			}

			pShaderAPI->SetPixelShaderFogParams(PSREG_FOG_PARAMS);

			float vEyePos_SpecExponent[4];
			pShaderAPI->GetWorldSpaceCameraPosition(vEyePos_SpecExponent);
			vEyePos_SpecExponent[3] = 0.0f;
			pShaderAPI->SetPixelShaderConstant(PSREG_EYEPOS_SPEC_EXPONENT, vEyePos_SpecExponent, 1);
#ifdef FLASHLIGHT	
			bool bFlashlightShadows = false;
			if (bFlashlight)
			{
				VMatrix worldToTexture;
				ITexture *pFlashlightDepthTexture;
				FlashlightState_t state = pShaderAPI->GetFlashlightStateEx(worldToTexture, &pFlashlightDepthTexture);
				bFlashlightShadows = state.m_bEnableShadows && (pFlashlightDepthTexture != NULL);

				SetFlashLightColorFromState(state, pShaderAPI);

				BindTexture(SHADER_SAMPLER7, state.m_pSpotlightTexture, state.m_nSpotlightTextureFrame);

				if (pFlashlightDepthTexture && g_pConfig->ShadowDepthTexture())
				{
					BindTexture(SHADER_SAMPLER8, pFlashlightDepthTexture);
				}
			}
#endif // FLASHLIGHT
#ifdef FLASHLIGHT	
			DECLARE_DYNAMIC_VERTEX_SHADER(di_cable_vs20);
#else
			DECLARE_DYNAMIC_VERTEX_SHADER(cable_vs20);
#endif
			SET_DYNAMIC_VERTEX_SHADER_COMBO(DOWATERFOG, pShaderAPI->GetSceneFogMode() == MATERIAL_FOG_LINEAR_BELOW_FOG_Z);
#ifdef FLASHLIGHT	
			SET_DYNAMIC_VERTEX_SHADER(di_cable_vs20);
#else
			SET_DYNAMIC_VERTEX_SHADER(cable_vs20);
#endif

			if (g_pHardwareConfig->SupportsPixelShaders_2_b())
			{
#ifdef FLASHLIGHT	
				DECLARE_DYNAMIC_PIXEL_SHADER(di_cable_ps20b);
#else
				DECLARE_DYNAMIC_PIXEL_SHADER(cable_ps20b);
#endif
				SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo());
				SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, bFullyOpaque && pShaderAPI->ShouldWriteDepthToDestAlpha());
#ifdef FLASHLIGHT	
				SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHT, hasFlashlight);
				SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, 0);
				SET_DYNAMIC_PIXEL_SHADER(di_cable_ps20b);
#else
				SET_DYNAMIC_PIXEL_SHADER(cable_ps20b);
#endif
			}
			else
			{
#ifdef FLASHLIGHT	
				DECLARE_DYNAMIC_PIXEL_SHADER(di_cable_ps20);
#else
				DECLARE_DYNAMIC_PIXEL_SHADER(cable_ps20);
#endif
				SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo());
#ifdef FLASHLIGHT	
				SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHT, hasFlashlight);
				SET_DYNAMIC_PIXEL_SHADER(di_cable_ps20);
#else
				SET_DYNAMIC_PIXEL_SHADER(cable_ps20);
#endif
			}
#ifdef FLASHLIGHT
			if (bFlashlight)
			{
				VMatrix worldToTexture;
					const FlashlightState_t &flashlightState = pShaderAPI->GetFlashlightState(worldToTexture);

					// Set the flashlight attenuation factors
					float atten[4];
					atten[0] = flashlightState.m_fConstantAtten;
					atten[1] = flashlightState.m_fLinearAtten;
					atten[2] = flashlightState.m_fQuadraticAtten;
					atten[3] = flashlightState.m_FarZ;
					pShaderAPI->SetPixelShaderConstant(22, atten, 1);

					// Set the flashlight origin
					float pos[4];
					pos[0] = flashlightState.m_vecLightOrigin[0];
					pos[1] = flashlightState.m_vecLightOrigin[1];
					pos[2] = flashlightState.m_vecLightOrigin[2];
					pos[3] = 1.0f;
					pShaderAPI->SetPixelShaderConstant(23, pos, 1);

					pShaderAPI->SetPixelShaderConstant(24, worldToTexture.Base(), 4);
				}
#endif // FLASHLIGHT
			}
		Draw();
	}

	SHADER_DRAW
	{
#ifdef DARKINTERVAL
		bool hasFlashlight = UsingFlashlight(params);
		DrawCableShader(params, pShaderAPI, pShaderShadow, hasFlashlight);
#else
		DrawCableShader(params, pShaderAPI, pShaderShadow, 0);
#endif
	}
END_SHADER
