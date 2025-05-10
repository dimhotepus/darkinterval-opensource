//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//===========================================================================//
#include "BaseVSShader.h"
#include "foliage_dx9_helper.h"
#include "convar.h"
#include "cpp_shader_constant_register_map.h"
#include "foliage_vs20.inc"
#include "foliage_ps20b.inc"
#include "commandbuilder.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar mat_fullbright( "mat_fullbright", "0", FCVAR_CHEAT );
static ConVar r_lightwarpidentity( "r_lightwarpidentity", "0", FCVAR_CHEAT );
static ConVar r_rimlight( "r_rimlight", "1", FCVAR_CHEAT );

// Textures may be bound to the following samplers:
//	SHADER_SAMPLER0	 Base (Albedo) / Gloss in alpha
//	SHADER_SAMPLER4	 Flashlight Shadow Depth Map
//	SHADER_SAMPLER5	 Normalization cube map
//	SHADER_SAMPLER6	 Flashlight Cookie

extern int g_nSnapShots;

//-----------------------------------------------------------------------------
// Initialize shader parameters
//-----------------------------------------------------------------------------
void InitParamsFoliage_DX9( CBaseVSShader *pShader, IMaterialVar** params, const char *pMaterialName, Foliage_DX9_Vars_t &info )
{	
	// FLASHLIGHTFIXME: Do ShaderAPI::BindFlashlightTexture
	Assert( info.m_nFlashlightTexture >= 0 );

	if ( g_pHardwareConfig->SupportsBorderColor() )
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight_border" );
	}
	else
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue( "effects/flashlight001" );
	}
	
	InitIntParam(info.m_nTreeSway, params, 0);
	InitFloatParam(info.m_nTreeSwayHeight, params, 1000.0f);
	InitFloatParam(info.m_nTreeSwayStartHeight, params, 0.2f);
	InitFloatParam(info.m_nTreeSwayRadius, params, 300.0f);
	InitFloatParam(info.m_nTreeSwayStartRadius, params, 0.1f);
	InitFloatParam(info.m_nTreeSwaySpeed, params, 1.0f);
	InitFloatParam(info.m_nTreeSwaySpeedHighWindMultiplier, params, 2.0f);
	InitFloatParam(info.m_nTreeSwayStrength, params, 10.0f);
	InitFloatParam(info.m_nTreeSwayScrumbleSpeed, params, 0.1f);
	InitFloatParam(info.m_nTreeSwayScrumbleStrength, params, 0.1f);
	InitFloatParam(info.m_nTreeSwayScrumbleFrequency, params, 0.1f);
	InitFloatParam(info.m_nTreeSwayFalloffExp, params, 1.0f);
	InitFloatParam(info.m_nTreeSwayScrumbleFalloffExp, params, 1.0f);
	InitFloatParam(info.m_nTreeSwaySpeedLerpStart, params, 3.0f);
	InitFloatParam(info.m_nTreeSwaySpeedLerpEnd, params, 6.0f);

	// This shader can be used with hw skinning
	SET_FLAGS2( MATERIAL_VAR2_SUPPORTS_HW_SKINNING );
	SET_FLAGS2( MATERIAL_VAR2_LIGHTING_VERTEX_LIT );
}

//-----------------------------------------------------------------------------
// Initialize shader
//-----------------------------------------------------------------------------
void InitFoliage_DX9( CBaseVSShader *pShader, IMaterialVar** params, Foliage_DX9_Vars_t &info )
{
	Assert( info.m_nFlashlightTexture >= 0 );
	pShader->LoadTexture( info.m_nFlashlightTexture, TEXTUREFLAGS_SRGB );
	
	bool bIsBaseTextureTranslucent = false;
	if ( params[info.m_nBaseTexture]->IsDefined() )
	{
		pShader->LoadTexture( info.m_nBaseTexture, TEXTUREFLAGS_SRGB );
		
		if ( params[info.m_nBaseTexture]->GetTextureValue()->IsTranslucent() )
		{
			bIsBaseTextureTranslucent = true;
		}
	}
}

class CFoliage_DX9_Context : public CBasePerMaterialContextData
{
public:
	CCommandBufferBuilder< CFixedCommandStorageBuffer< 800 > > m_SemiStaticCmdsOut;
	bool m_bFastPath;

};

//-----------------------------------------------------------------------------
// Draws the shader
//-----------------------------------------------------------------------------
void DrawFoliage_DX9_Internal( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow,
	bool bHasFlashlight, Foliage_DX9_Vars_t &info, VertexCompressionType_t vertexCompression,
							CBasePerMaterialContextData **pContextDataPtr )
{
	bool bHasBaseTexture = (info.m_nBaseTexture != -1) && params[info.m_nBaseTexture]->IsTexture();
	bool bIsAlphaTested = IS_FLAG_SET( MATERIAL_VAR_ALPHATEST ) != 0;

	BlendType_t nBlendType= pShader->EvaluateBlendRequirements( info.m_nBaseTexture, true );
	bool bFullyOpaque = ( nBlendType != BT_BLENDADD ) && ( nBlendType != BT_BLEND ) && !bIsAlphaTested && !bHasFlashlight;

	CFoliage_DX9_Context *pContextData = reinterpret_cast< CFoliage_DX9_Context *> ( *pContextDataPtr );
	if ( !pContextData )
	{
		pContextData = new CFoliage_DX9_Context;
		*pContextDataPtr = pContextData;
	}

	bool hasDiffuseLighting = true; //bVertexLitGeneric;

	bool bTreeSway = (GetIntParam(info.m_nTreeSway, params, 0) != 0);
	int nTreeSwayMode = GetIntParam(info.m_nTreeSway, params, 0);
	nTreeSwayMode = clamp(nTreeSwayMode, 0, 2);

	if (pShader->IsSnapshotting() || (!pContextData) || (pContextData->m_bMaterialVarsChanged))
	{
		if (pShader->IsSnapshotting())
		{
			pShaderShadow->EnableAlphaTest(bIsAlphaTested);

			if (info.m_nAlphaTestReference != -1 && params[info.m_nAlphaTestReference]->GetFloatValue() > 0.0f)
			{
				pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_GEQUAL, params[info.m_nAlphaTestReference]->GetFloatValue());
			}

			int nShadowFilterMode = 0;
			if (bHasFlashlight)
			{
				if (params[info.m_nBaseTexture]->IsTexture())
				{
					pShader->SetAdditiveBlendingShadowState(info.m_nBaseTexture, true);
				}

				if (bIsAlphaTested)
				{
					// disable alpha test and use the zfunc zequals since alpha isn't guaranteed to 
					// be the same on both the regular pass and the flashlight pass.
					pShaderShadow->EnableAlphaTest(false);
					pShaderShadow->DepthFunc(SHADER_DEPTHFUNC_EQUAL);
				}
				pShaderShadow->EnableBlending(true);
				pShaderShadow->EnableDepthWrites(false);

				// Be sure not to write to dest alpha
				pShaderShadow->EnableAlphaWrites(false);

				nShadowFilterMode = g_pHardwareConfig->GetShadowFilterMode();	// Based upon vendor and device dependent formats
			}
			else // not flashlight pass
			{
				if (params[info.m_nBaseTexture]->IsTexture())
				{
					pShader->SetDefaultBlendingShadowState(info.m_nBaseTexture, true);
				}
			}

			bool bHalfLambert = IS_FLAG_SET(MATERIAL_VAR_HALFLAMBERT);

			unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
			int userDataSize = 0;

			// Always enable...will bind white if nothing specified...
			pShaderShadow->EnableTexture(SHADER_SAMPLER0, true);		// Base (albedo) map
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0, true);

			if (bHasFlashlight)
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER4, true);	// Shadow depth map
				pShaderShadow->SetShadowDepthFiltering(SHADER_SAMPLER4);
				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER4, false);
				pShaderShadow->EnableTexture(SHADER_SAMPLER5, true);	// Noise map
				pShaderShadow->EnableTexture(SHADER_SAMPLER6, true);	// Flashlight cookie
				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER6, true);
				userDataSize = 4; // tangent S
			}

			// Always enable, since flat normal will be bound
			pShaderShadow->EnableTexture(SHADER_SAMPLER3, true);		// Normal map
			userDataSize = 4; // tangent S
			pShaderShadow->EnableTexture(SHADER_SAMPLER5, true);		// Normalizing cube map
			pShaderShadow->EnableSRGBWrite(true);

			// texcoord0 : base texcoord, texcoord2 : decal hw morph delta
			int pTexCoordDim[3] = { 2, 0, 3 };
			int nTexCoordCount = 1;

			// This shader supports compressed vertices, so OR in that flag:
			flags |= VERTEX_FORMAT_COMPRESSED;

			pShaderShadow->VertexShaderVertexFormat(flags, nTexCoordCount, pTexCoordDim, userDataSize);

			DECLARE_STATIC_VERTEX_SHADER(foliage_vs20);
			SET_STATIC_VERTEX_SHADER_COMBO(HALFLAMBERT, bHalfLambert);
			SET_STATIC_VERTEX_SHADER_COMBO(TREESWAY, bTreeSway ? nTreeSwayMode : 0);
			SET_STATIC_VERTEX_SHADER(foliage_vs20);

			// Assume we're only going to get in here if we support 2b
			DECLARE_STATIC_PIXEL_SHADER(foliage_ps20b);
			SET_STATIC_PIXEL_SHADER_COMBO(DIFFUSELIGHTING, hasDiffuseLighting);
		//	SET_STATIC_PIXEL_SHADER_COMBO(HALFLAMBERT, bHalfLambert);
			SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
			SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode);
			SET_STATIC_PIXEL_SHADER_COMBO(CONVERT_TO_SRGB, 0);
			SET_STATIC_PIXEL_SHADER(foliage_ps20b);

			if (bHasFlashlight)
			{
				pShader->FogToBlack();
			}
			else
			{
				pShader->DefaultFog();
			}

			// HACK HACK HACK - enable alpha writes all the time so that we have them for underwater stuff
			pShaderShadow->EnableAlphaWrites(bFullyOpaque);
		}

		if (pShaderAPI && ((!pContextData) || (pContextData->m_bMaterialVarsChanged)))
		{
			/*^*/ // 		printf("\t\t[3] pShaderAPI && ( (! pContextData ) || ( pContextData->m_bMaterialVarsChanged ) )  TRUE \n");
			if (!pContextData)								// make sure allocated
			{
				++g_nSnapShots;
				pContextData = new CFoliage_DX9_Context;
				*pContextDataPtr = pContextData;
			}
			pContextData->m_SemiStaticCmdsOut.Reset();
			pContextData->m_SemiStaticCmdsOut.SetPixelShaderFogParams(PSREG_FOG_PARAMS);

			if (bHasBaseTexture)
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture(pShader, SHADER_SAMPLER0, info.m_nBaseTexture, info.m_nBaseTextureFrame);
			}
			else
			{
				pContextData->m_SemiStaticCmdsOut.BindTexture(pShader, SHADER_SAMPLER0, TEXTURE_WHITE, 0);
			}

			if (bHasFlashlight)
			{
				// Tweaks associated with a given flashlight
				VMatrix worldToTexture;
				const FlashlightState_t &flashlightState = pShaderAPI->GetFlashlightState(worldToTexture);
				float tweaks[4];
				tweaks[0] = flashlightState.m_flShadowFilterSize / 1024;
				tweaks[1] = ShadowAttenFromState(flashlightState);
				pShader->HashShadow2DJitter(flashlightState.m_flShadowJitterSeed, &tweaks[2], &tweaks[3]);
				pShaderAPI->SetPixelShaderConstant(2, tweaks, 1);

				// Dimensions of screen, used for screen-space noise map sampling
				float vScreenScale[4] = { 1280.0f / 32.0f, 720.0f / 32.0f, 0, 0 };
				int nWidth, nHeight;
				pShaderAPI->GetBackBufferDimensions(nWidth, nHeight);
				vScreenScale[0] = (float)nWidth / 32.0f;
				vScreenScale[1] = (float)nHeight / 32.0f;
				pShaderAPI->SetPixelShaderConstant(31, vScreenScale, 1);
			}

			// Treesway
			if (bTreeSway)
			{
				float flParams[4] = { 0.0f, 0.0f, 0.0f, 0.0f };

				flParams[0] = GetFloatParam(info.m_nTreeSwaySpeedHighWindMultiplier, params, 2.0f);
				flParams[1] = GetFloatParam(info.m_nTreeSwayScrumbleFalloffExp, params, 1.0f);
				flParams[2] = GetFloatParam(info.m_nTreeSwayFalloffExp, params, 1.0f);
				flParams[3] = GetFloatParam(info.m_nTreeSwayScrumbleSpeed, params, 3.0f);
				pContextData->m_SemiStaticCmdsOut.SetVertexShaderConstant(VERTEX_SHADER_SHADER_SPECIFIC_CONST_6, flParams);

				flParams[0] = GetFloatParam(info.m_nTreeSwaySpeedLerpStart, params, 3.0f);
				flParams[1] = GetFloatParam(info.m_nTreeSwaySpeedLerpEnd, params, 6.0f);
				flParams[2] = 0;
				flParams[3] = 0;
				pContextData->m_SemiStaticCmdsOut.SetVertexShaderConstant(VERTEX_SHADER_SHADER_SPECIFIC_CONST_4, flParams);

				flParams[0] = GetFloatParam(info.m_nTreeSwayHeight, params, 1000.0f);
				flParams[1] = GetFloatParam(info.m_nTreeSwayStartHeight, params, 0.1f);
				flParams[2] = GetFloatParam(info.m_nTreeSwayRadius, params, 300.0f);
				flParams[3] = GetFloatParam(info.m_nTreeSwayStartRadius, params, 0.2f);
				pContextData->m_SemiStaticCmdsOut.SetVertexShaderConstant(VERTEX_SHADER_SHADER_SPECIFIC_CONST_8, flParams);

				flParams[0] = GetFloatParam(info.m_nTreeSwaySpeed, params, 1.0f);
				flParams[1] = GetFloatParam(info.m_nTreeSwayStrength, params, 10.0f);
				flParams[2] = GetFloatParam(info.m_nTreeSwayScrumbleFrequency, params, 12.0f);
				flParams[3] = GetFloatParam(info.m_nTreeSwayScrumbleStrength, params, 10.0f);
				pContextData->m_SemiStaticCmdsOut.SetVertexShaderConstant(VERTEX_SHADER_SHADER_SPECIFIC_CONST_9, flParams);
			}
			//

			pContextData->m_SemiStaticCmdsOut.End();
		}
	}
	
	if (pShaderAPI)
	{
		CCommandBufferBuilder< CFixedCommandStorageBuffer< 1000 > > DynamicCmdsOut;
		DynamicCmdsOut.Call(pContextData->m_SemiStaticCmdsOut.Base());

		bool bFlashlightShadows = false;
		if (bHasFlashlight)
		{
			VMatrix worldToTexture;
			ITexture *pFlashlightDepthTexture;
			FlashlightState_t state = pShaderAPI->GetFlashlightStateEx(worldToTexture, &pFlashlightDepthTexture);
			bFlashlightShadows = state.m_bEnableShadows && (pFlashlightDepthTexture != NULL);

			if (pFlashlightDepthTexture && g_pConfig->ShadowDepthTexture() && state.m_bEnableShadows)
			{
				pShader->BindTexture(SHADER_SAMPLER4, pFlashlightDepthTexture, 0);
				DynamicCmdsOut.BindStandardTexture(SHADER_SAMPLER5, TEXTURE_SHADOW_NOISE_2D);
			}

			SetFlashLightColorFromState(state, pShaderAPI, PSREG_FLASHLIGHT_COLOR );

			Assert(info.m_nFlashlightTexture >= 0 && info.m_nFlashlightTextureFrame >= 0);
			pShader->BindTexture(SHADER_SAMPLER6, state.m_pSpotlightTexture, state.m_nSpotlightTextureFrame);
		}

		// Set up light combo state
		LightState_t lightState = { 0, false, false };
		if (!bHasFlashlight || IsX360())
		{
			pShaderAPI->GetDX9LightState(&lightState);
		}

		MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();
		int fogIndex = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z) ? 1 : 0;
		int numBones = pShaderAPI->GetCurrentNumBones();

		bool bWriteDepthToAlpha = false;
		bool bWriteWaterFogToAlpha = false;
		if (bFullyOpaque)
		{
			bWriteDepthToAlpha = pShaderAPI->ShouldWriteDepthToDestAlpha();
			bWriteWaterFogToAlpha = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z);
			AssertMsg(!(bWriteDepthToAlpha && bWriteWaterFogToAlpha), "Can't write two values to alpha at the same time.");
		}
		else
		{
			//can't write a special value to dest alpha if we're actually using as-intended alpha
			bWriteDepthToAlpha = false;
			bWriteWaterFogToAlpha = false;
		}

		DECLARE_DYNAMIC_VERTEX_SHADER(foliage_vs20);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(DYNAMIC_LIGHT, lightState.HasDynamicLight());
		SET_DYNAMIC_VERTEX_SHADER_COMBO(STATIC_LIGHT, lightState.m_bStaticLight ? 1 : 0);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(DOWATERFOG, fogIndex);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, numBones > 0);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(LIGHTING_PREVIEW, pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING) != 0);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, (int)vertexCompression);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
		//	SET_DYNAMIC_VERTEX_SHADER( foliage_vs20 );
		SET_DYNAMIC_VERTEX_SHADER_CMD(DynamicCmdsOut, foliage_vs20);

		DECLARE_DYNAMIC_PIXEL_SHADER(foliage_ps20b);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo());
		SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
		//	SET_DYNAMIC_PIXEL_SHADER( foliage_ps20b );
		SET_DYNAMIC_PIXEL_SHADER_CMD(DynamicCmdsOut, foliage_ps20b);

		pShader->SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, info.m_nBaseTextureTransform);
		pShader->SetModulationPixelShaderDynamicState_LinearColorSpace(1);
		pShader->SetAmbientCubeDynamicStateVertexShader();

		if (!bHasFlashlight)
		{
			pShaderAPI->BindStandardTexture(SHADER_SAMPLER5, TEXTURE_NORMALIZATION_CUBEMAP_SIGNED);
		}

		pShaderAPI->SetPixelShaderStateAmbientLightCube(PSREG_AMBIENT_CUBE, false);	// Force to black if not bAmbientLight
		pShaderAPI->CommitPixelShaderLighting(PSREG_LIGHT_INFO_ARRAY);
		
		pShaderAPI->SetPixelShaderFogParams(PSREG_FOG_PARAMS);

		if (bHasFlashlight)
		{
			VMatrix worldToTexture;
			float atten[4], pos[4], tweaks[4];

			const FlashlightState_t &flashlightState = pShaderAPI->GetFlashlightState(worldToTexture);
			SetFlashLightColorFromState(flashlightState, pShaderAPI, PSREG_FLASHLIGHT_COLOR);

			pShader->BindTexture(SHADER_SAMPLER6, flashlightState.m_pSpotlightTexture, flashlightState.m_nSpotlightTextureFrame);

			atten[0] = flashlightState.m_fConstantAtten;		// Set the flashlight attenuation factors
			atten[1] = flashlightState.m_fLinearAtten;
			atten[2] = flashlightState.m_fQuadraticAtten;
			atten[3] = flashlightState.m_FarZ;
			pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_ATTENUATION, atten, 1);

			pos[0] = flashlightState.m_vecLightOrigin[0];		// Set the flashlight origin
			pos[1] = flashlightState.m_vecLightOrigin[1];
			pos[2] = flashlightState.m_vecLightOrigin[2];
			pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_POSITION_RIM_BOOST, pos, 1);

			pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_TO_WORLD_TEXTURE, worldToTexture.Base(), 4);

			// Tweaks associated with a given flashlight
			tweaks[0] = ShadowFilterFromState(flashlightState);
			tweaks[1] = ShadowAttenFromState(flashlightState);
			pShader->HashShadow2DJitter(flashlightState.m_flShadowJitterSeed, &tweaks[2], &tweaks[3]);
			pShaderAPI->SetPixelShaderConstant(PSREG_ENVMAP_TINT__SHADOW_TWEAKS, tweaks, 1);

			// Dimensions of screen, used for screen-space noise map sampling
			float vScreenScale[4] = { 1280.0f / 32.0f, 720.0f / 32.0f, 0, 0 };
			int nWidth, nHeight;
			pShaderAPI->GetBackBufferDimensions(nWidth, nHeight);
			vScreenScale[0] = (float)nWidth / 32.0f;
			vScreenScale[1] = (float)nHeight / 32.0f;
			pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_SCREEN_SCALE, vScreenScale, 1);
		}

		// Treesway
		if (bTreeSway)
		{
			float fTempConst[4];
			fTempConst[1] = pShaderAPI->CurrentTime();			
			const Vector& windDir = pShaderAPI->GetVectorRenderingParameter(VECTOR_RENDERPARM_WIND_DIRECTION);
			fTempConst[2] = windDir.x;
			fTempConst[3] = windDir.y;
			
			DynamicCmdsOut.SetVertexShaderConstant(VERTEX_SHADER_SHADER_SPECIFIC_CONST_7, fTempConst);
		}

		DynamicCmdsOut.End();
		pShaderAPI->ExecuteCommandBuffer(DynamicCmdsOut.Base());
	}
	pShader->Draw();

#if 0
	else // not snapshotting -- begin dynamic state
	{
		CCommandBufferBuilder< CFixedCommandStorageBuffer< 1000 > > DynamicCmdsOut;
		DynamicCmdsOut.Call(pContextData->m_SemiStaticCmdsOut.Base());

		bool bLightingOnly = mat_fullbright.GetInt() == 2 && !IS_FLAG_SET( MATERIAL_VAR_NO_DEBUG_OVERRIDE );

		if( bHasBaseTexture )
		{
			pShader->BindTexture( SHADER_SAMPLER0, info.m_nBaseTexture, info.m_nBaseTextureFrame );
		}
		else
		{
			pShaderAPI->BindStandardTexture( SHADER_SAMPLER0, TEXTURE_WHITE );
		}

		LightState_t lightState = { 0, false, false };
		bool bFlashlightShadows = false;
		if( bHasFlashlight )
		{
			Assert( info.m_nFlashlightTexture >= 0 && info.m_nFlashlightTextureFrame >= 0 );
			pShader->BindTexture( SHADER_SAMPLER6, info.m_nFlashlightTexture, info.m_nFlashlightTextureFrame );
			VMatrix worldToTexture;
			ITexture *pFlashlightDepthTexture;
			FlashlightState_t state = pShaderAPI->GetFlashlightStateEx( worldToTexture, &pFlashlightDepthTexture );
			bFlashlightShadows = state.m_bEnableShadows && ( pFlashlightDepthTexture != NULL );

			SetFlashLightColorFromState( state, pShaderAPI, PSREG_FLASHLIGHT_COLOR );

			if( pFlashlightDepthTexture && g_pConfig->ShadowDepthTexture() && state.m_bEnableShadows )
			{
				pShader->BindTexture( SHADER_SAMPLER4, pFlashlightDepthTexture, 0 );
				pShaderAPI->BindStandardTexture( SHADER_SAMPLER5, TEXTURE_SHADOW_NOISE_2D );
			}
		}
		else // no flashlight
		{
			pShaderAPI->GetDX9LightState( &lightState );
		}

		MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();
		int fogIndex = ( fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z ) ? 1 : 0;
		int numBones = pShaderAPI->GetCurrentNumBones();

		bool bWriteDepthToAlpha = false;
		bool bWriteWaterFogToAlpha = false;
		if( bFullyOpaque ) 
		{
			bWriteDepthToAlpha = pShaderAPI->ShouldWriteDepthToDestAlpha();
			bWriteWaterFogToAlpha = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z);
			AssertMsg( !(bWriteDepthToAlpha && bWriteWaterFogToAlpha), "Can't write two values to alpha at the same time." );
		}

		DECLARE_DYNAMIC_VERTEX_SHADER( foliage_vs20 );
		SET_DYNAMIC_VERTEX_SHADER_COMBO( DOWATERFOG, fogIndex );
		SET_DYNAMIC_VERTEX_SHADER_COMBO( SKINNING, numBones > 0 );
		SET_DYNAMIC_VERTEX_SHADER_COMBO( LIGHTING_PREVIEW, pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING)!=0);
		SET_DYNAMIC_VERTEX_SHADER_COMBO( COMPRESSED_VERTS, (int)vertexCompression );
		SET_DYNAMIC_VERTEX_SHADER_COMBO( NUM_LIGHTS, lightState.m_nNumLights );
	//	SET_DYNAMIC_VERTEX_SHADER( foliage_vs20 );
		SET_DYNAMIC_VERTEX_SHADER_CMD(DynamicCmdsOut, foliage_vs20);

		DECLARE_DYNAMIC_PIXEL_SHADER( foliage_ps20b );
		SET_DYNAMIC_PIXEL_SHADER_COMBO( NUM_LIGHTS, lightState.m_nNumLights );
		SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha );
		SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha );
		SET_DYNAMIC_PIXEL_SHADER_COMBO( PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo() );
		SET_DYNAMIC_PIXEL_SHADER_COMBO( FLASHLIGHTSHADOWS, bFlashlightShadows );
	//	SET_DYNAMIC_PIXEL_SHADER( foliage_ps20b );
		SET_DYNAMIC_PIXEL_SHADER_CMD(DynamicCmdsOut, foliage_ps20b);

		pShader->SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, info.m_nBaseTextureTransform );
		pShader->SetModulationPixelShaderDynamicState_LinearColorSpace( 1 );
		pShader->SetAmbientCubeDynamicStateVertexShader();

		if( !bHasFlashlight )
		{
			pShaderAPI->BindStandardTexture( SHADER_SAMPLER5, TEXTURE_NORMALIZATION_CUBEMAP_SIGNED );
		}

		pShaderAPI->SetPixelShaderStateAmbientLightCube( PSREG_AMBIENT_CUBE, !lightState.m_bAmbientLight );	// Force to black if not bAmbientLight
		pShaderAPI->CommitPixelShaderLighting( PSREG_LIGHT_INFO_ARRAY );

		// handle mat_fullbright 2 (diffuse lighting only)
		if( bLightingOnly )
		{
			pShaderAPI->BindStandardTexture( SHADER_SAMPLER0, TEXTURE_GREY );
		}

		pShaderAPI->SetPixelShaderFogParams( PSREG_FOG_PARAMS );

		if( bHasFlashlight )
		{
			VMatrix worldToTexture;
			float atten[4], pos[4], tweaks[4];

			const FlashlightState_t &flashlightState = pShaderAPI->GetFlashlightState( worldToTexture );
			SetFlashLightColorFromState( flashlightState, pShaderAPI, PSREG_FLASHLIGHT_COLOR );

			pShader->BindTexture( SHADER_SAMPLER6, flashlightState.m_pSpotlightTexture, flashlightState.m_nSpotlightTextureFrame );

			atten[0] = flashlightState.m_fConstantAtten;		// Set the flashlight attenuation factors
			atten[1] = flashlightState.m_fLinearAtten;
			atten[2] = flashlightState.m_fQuadraticAtten;
			atten[3] = flashlightState.m_FarZ;
			pShaderAPI->SetPixelShaderConstant( PSREG_FLASHLIGHT_ATTENUATION, atten, 1 );

			pos[0] = flashlightState.m_vecLightOrigin[0];		// Set the flashlight origin
			pos[1] = flashlightState.m_vecLightOrigin[1];
			pos[2] = flashlightState.m_vecLightOrigin[2];
			pShaderAPI->SetPixelShaderConstant( PSREG_FLASHLIGHT_POSITION_RIM_BOOST, pos, 1 );

			pShaderAPI->SetPixelShaderConstant( PSREG_FLASHLIGHT_TO_WORLD_TEXTURE, worldToTexture.Base(), 4 );

			// Tweaks associated with a given flashlight
			tweaks[0] = ShadowFilterFromState( flashlightState );
			tweaks[1] = ShadowAttenFromState( flashlightState );
			pShader->HashShadow2DJitter( flashlightState.m_flShadowJitterSeed, &tweaks[2], &tweaks[3] );
			pShaderAPI->SetPixelShaderConstant( PSREG_ENVMAP_TINT__SHADOW_TWEAKS, tweaks, 1 );

			// Dimensions of screen, used for screen-space noise map sampling
			float vScreenScale[4] = {1280.0f / 32.0f, 720.0f / 32.0f, 0, 0};
			int nWidth, nHeight;
			pShaderAPI->GetBackBufferDimensions( nWidth, nHeight );
			vScreenScale[0] = (float) nWidth  / 32.0f;
			vScreenScale[1] = (float) nHeight / 32.0f;
			pShaderAPI->SetPixelShaderConstant( PSREG_FLASHLIGHT_SCREEN_SCALE, vScreenScale, 1 );
		}

		if (bTreeSway)
		{
			float fTempConst[4];
			fTempConst[0] = 0;
			fTempConst[1] = pShaderAPI->CurrentTime();
			Vector windDir = pShaderAPI->GetVectorRenderingParameter(VECTOR_RENDERPARM_WIND_DIRECTION);
		//	pShaderAPI->SetVertexShaderConstant(VERTEX_SHADER_SHADER_SPECIFIC_CONST_7, fTempConst);
			DynamicCmdsOut.SetVertexShaderConstant(VERTEX_SHADER_SHADER_SPECIFIC_CONST_7, fTempConst);
		}

		DynamicCmdsOut.End();
		pShaderAPI->ExecuteCommandBuffer(DynamicCmdsOut.Base());
	}
	pShader->Draw();
#endif
}


//-----------------------------------------------------------------------------
// Draws the shader
//-----------------------------------------------------------------------------
void DrawFoliage_DX9( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow,
				   Foliage_DX9_Vars_t &info, VertexCompressionType_t vertexCompression, CBasePerMaterialContextData **pContextDataPtr )

{
	bool bHasFlashlight = pShader->UsingFlashlight( params );
	if ( bHasFlashlight )
	{
		DrawFoliage_DX9_Internal( pShader, params, pShaderAPI,	pShaderShadow, false, info, vertexCompression, pContextDataPtr++ );
		if ( pShaderShadow )
		{
			pShader->SetInitialShadowState( );
		}
	}
	DrawFoliage_DX9_Internal( pShader, params, pShaderAPI,	pShaderShadow, bHasFlashlight, info, vertexCompression, pContextDataPtr );
}
