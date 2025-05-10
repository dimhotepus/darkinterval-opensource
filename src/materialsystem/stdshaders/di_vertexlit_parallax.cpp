#include "BaseVSShader.h"
#include "convar.h"
#include "di_lightmapped_parallax_vs30.inc"
#include "di_lightmapped_parallax_ps30.inc"
#if 0
extern ConVar r_parallaxmap_debug;

BEGIN_VS_SHADER(DI_VertexLitParallax_DX90, "DI_VertexLitParallax_DX90")

	BEGIN_SHADER_PARAMS
	SHADER_PARAM(PARALLAXMAP, SHADER_PARAM_TYPE_TEXTURE, "bumpmaps/bump_flat", "bump map")
	SHADER_PARAM(PARALLAXMAPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $parallaxmap")
	SHADER_PARAM(PARALLAXMINSTEPS, SHADER_PARAM_TYPE_INTEGER, "12", "Min parallax iterations. Default is 12")
	SHADER_PARAM(PARALLAXMAXSTEPS, SHADER_PARAM_TYPE_INTEGER, "50", "Max parallax iterations. Default is 50")
	SHADER_PARAM(PARALLAXOCCLUSIONRATIO, SHADER_PARAM_TYPE_FLOAT, "0.8", "How much the parallax offset will darken the lightmap. 0-1, 1 means the pits will be fully occluded")
	SHADER_PARAM(PARALLAXMAPRANGESCALE, SHADER_PARAM_TYPE_FLOAT, "0.2", "artist-controlled height range of the height map")
	SHADER_PARAM(PARALLAXDOTPRODUCTFIX, SHADER_PARAM_TYPE_INTEGER, "0", "Parallax offset becomes smaller at acute angles. Good on displacements, but introduces unacceptable distortions in many other cases")
	SHADER_PARAM(PARALLAXOFFSET, SHADER_PARAM_TYPE_FLOAT, "0.0", "artist-controlled height plane offset")
	SHADER_PARAM(BASEMAPALPHAPARALLAXMAP, SHADER_PARAM_TYPE_INTEGER, "0", "Use it if the height map is meant to be extracted from the base texture's alpha channel")

	// Model or brush?
	SHADER_PARAM(MODEL, SHADER_PARAM_TYPE_BOOL, "0", "model or brush toggle")

	// Bumpmap isn't used for parallax, but comes into play in flashlight pass
	SHADER_PARAM(BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "bump map")
	SHADER_PARAM(BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap")
	SHADER_PARAM(BUMPTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform")

	// Detail
	SHADER_PARAM(DETAIL, SHADER_PARAM_TYPE_TEXTURE, "shadertest/WorldTwoTextureBlend_detail", "detail texture")
	SHADER_PARAM(DETAILFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $detail")
	SHADER_PARAM(DETAILSCALE, SHADER_PARAM_TYPE_FLOAT, "1.0", "scale of the detail texture")

	// Selfillum
	SHADER_PARAM(SELFILLUMTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint")

	// Unlit option for simpler model - good for backfaced materials inside glowing windows and monitors
	SHADER_PARAM(UNLIT, SHADER_PARAM_TYPE_BOOL, "0", "don't use lightmap, selfillum, or flashlight")

	// Envmap reflection
	SHADER_PARAM(ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "envmap")
	SHADER_PARAM(ENVMAPFRAME, SHADER_PARAM_TYPE_INTEGER, "", "")
	SHADER_PARAM(ENVMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "envmap tint")
	SHADER_PARAM(ENVMAPCONTRAST, SHADER_PARAM_TYPE_FLOAT, "0.0", "contrast 0 == normal 1 == color*color")
	SHADER_PARAM(ENVMAPSATURATION, SHADER_PARAM_TYPE_FLOAT, "1.0", "saturation 0 == greyscale 1 == normal")
	SHADER_PARAM(FRESNELREFLECTION, SHADER_PARAM_TYPE_FLOAT, "1.0", "1.0 == mirror, 0.0 == water")

	// Translucency test
	SHADER_PARAM(TRANSLUCENT, SHADER_PARAM_TYPE_INTEGER, "0", "translucency toggle")

	// Debug preview of data
	SHADER_PARAM(PARALLAXDEBUG, SHADER_PARAM_TYPE_INTEGER, "0", "debug previews")

	END_SHADER_PARAMS

	SHADER_FALLBACK
	{
		if (g_pHardwareConfig->GetDXSupportLevel() < 90)
			return "VertexLitGeneric_DX8";

		return 0;
	}

	// Set up anything that is necessary to make decisions in SHADER_FALLBACK.
	SHADER_INIT_PARAMS()
	{
		InitFloatParam(PARALLAXMAPRANGESCALE, params, 1.0f);
		InitIntParam(PARALLAXMINSTEPS, params, 12);
		InitIntParam(PARALLAXMAXSTEPS, params, 50);
		InitFloatParam(PARALLAXOCCLUSIONRATIO, params, 0.8f);
		InitFloatParam(PARALLAXOFFSET, params, 0.0f);
		InitIntParam(TRANSLUCENT, params, 0);
		InitIntParam(PARALLAXDOTPRODUCTFIX, params, 0);
		InitIntParam(BASEMAPALPHAPARALLAXMAP, params, 0);
		InitFloatParam(ENVMAPSATURATION, params, 0.5f);
		InitFloatParam(ENVMAPCONTRAST, params, 0.5f);
		InitFloatParam(FRESNELREFLECTION, params, 1.0f);
		InitIntParam(ENVMAPFRAME, params, 0);
		InitIntParam(UNLIT, params, 0);
		InitIntParam(PARALLAXDEBUG, params, 0);
		InitIntParam(MODEL, params, 0);

		if (g_pHardwareConfig->SupportsBorderColor())
		{
			params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight_border");
		}
		else
		{
			params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight001");
		}

		if (IsUsingGraphics() && params[ENVMAP]->IsDefined() && !CanUseEditorMaterials())
		{
			if (stricmp(params[ENVMAP]->GetStringValue(), "env_cubemap") == 0)
			{
				Warning("env_cubemap used on world geometry without rebuilding map. . ignoring: %s\n", pMaterialName);
				params[ENVMAP]->SetUndefined();
			}
		}

		SET_FLAGS(MATERIAL_VAR_NO_DEBUG_OVERRIDE);
		SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);

		if (IS_FLAG_SET(MATERIAL_VAR_MODEL)) // Set material var2 flags specific to models
		{
			SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);             // Required for skinning
			SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);             // Required for dynamic lighting
			if (params[UNLIT]->GetIntValue() == 1)
			{
				CLEAR_FLAGS2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);
				CLEAR_FLAGS2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL);
				CLEAR_FLAGS2(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS);
				CLEAR_FLAGS2(MATERIAL_VAR2_SUPPORTS_FLASHLIGHT);
				CLEAR_FLAGS2(MATERIAL_VAR2_USE_FLASHLIGHT);
				SET_FLAGS2(MATERIAL_VAR2_LIGHTING_UNLIT);
			}
			else
			{
				CLEAR_FLAGS2(MATERIAL_VAR2_LIGHTING_UNLIT);
				SET_FLAGS2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL);         // Required for dynamic lighting
				SET_FLAGS2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);              // Required for dynamic lighting
				SET_FLAGS2(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS);   // Required for ambient cube
				SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_FLASHLIGHT);              // Required for flashlight
				SET_FLAGS2(MATERIAL_VAR2_USE_FLASHLIGHT);                   // Required for flashlight
			}
		}
		else
		{
			if (params[UNLIT]->GetIntValue() == 1)
			{
				CLEAR_FLAGS2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);
				CLEAR_FLAGS2(MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP);
				SET_FLAGS2(MATERIAL_VAR2_LIGHTING_UNLIT);
			}
			else
			{
				CLEAR_FLAGS2(MATERIAL_VAR2_LIGHTING_UNLIT);
				SET_FLAGS2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);
				if (params[BASETEXTURE]->IsDefined() && g_pConfig->UseBumpmapping() && params[BUMPMAP]->IsDefined())
				{
					SET_FLAGS2(MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP);
				}
			}
		}

		if (params[TRANSLUCENT]->IsDefined() && params[TRANSLUCENT]->GetIntValue() == 1)
		{
			SET_FLAGS(MATERIAL_VAR_TRANSLUCENT);
		}

		if (!params[SELFILLUMTINT]->IsDefined())
		{
			params[SELFILLUMTINT]->SetVecValue(1.0f, 1.0f, 1.0f);
		}

		if (!params[DETAILSCALE]->IsDefined())
		{
			params[DETAILSCALE]->SetFloatValue(4.0f);
		}

		if (!params[DETAILFRAME]->IsDefined())
		{
			params[DETAILFRAME]->SetIntValue(0);
		}

		// No texture means no self-illum or env mask in base alpha
		if (!params[BASETEXTURE]->IsDefined())
		{
			CLEAR_FLAGS(MATERIAL_VAR_SELFILLUM);
			CLEAR_FLAGS(MATERIAL_VAR_BASEALPHAENVMAPMASK);
		}

		if (!params[ENVMAPTINT]->IsDefined())
			params[ENVMAPTINT]->SetVecValue(1.0f, 1.0f, 1.0f);

		// If mat_specular 0, then get rid of envmap
		if (!g_pConfig->UseSpecular() && params[ENVMAP]->IsDefined() && params[BASETEXTURE]->IsDefined())
		{
			params[ENVMAP]->SetUndefined();
		}
	}

	SHADER_INIT
	{
		SET_FLAGS(MATERIAL_VAR_MODEL);

		if (params[BASETEXTURE]->IsDefined())
		{
			LoadTexture(BASETEXTURE, TEXTUREFLAGS_SRGB);

			if (!params[BASETEXTURE]->GetTextureValue()->IsTranslucent())
			{
				CLEAR_FLAGS(MATERIAL_VAR_SELFILLUM);
				CLEAR_FLAGS(MATERIAL_VAR_BASEALPHAENVMAPMASK);
			}
		}
	if (params[PARALLAXMAP]->IsDefined())
	{
		LoadTexture(PARALLAXMAP, TEXTUREFLAGS_SRGB);
	}
	if (params[BUMPMAP]->IsDefined())
	{
		LoadBumpMap(BUMPMAP);
	}

	if (params[DETAIL]->IsDefined() && params[DETAIL]->IsTexture())
	{
		LoadTexture(DETAIL);
	}

	if (params[FLASHLIGHTTEXTURE]->IsDefined())
	{
		LoadTexture(FLASHLIGHTTEXTURE, TEXTUREFLAGS_SRGB);
	}

	// Don't alpha test if the alpha channel is used for other purposes
	if (IS_FLAG_SET(MATERIAL_VAR_SELFILLUM) || IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK))
	{
		CLEAR_FLAGS(MATERIAL_VAR_ALPHATEST);
	}

	if (!params[TRANSLUCENT]->IsDefined())
	{
		params[TRANSLUCENT]->SetIntValue(0);
	}

	if (params[ENVMAP]->IsDefined())
	{
		if (!IS_FLAG_SET(MATERIAL_VAR_ENVMAPSPHERE))
		{
			LoadCubeMap(ENVMAP, g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? TEXTUREFLAGS_SRGB : 0);
		}
		else
		{
			LoadTexture(ENVMAP);
		}

		if (!g_pHardwareConfig->SupportsCubeMaps())
		{
			SET_FLAGS(MATERIAL_VAR_ENVMAPSPHERE);
		}
	}

	// We always need this because of the flashlight.
	SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);
	}

	void DrawLightmappedParallaxedShader(IMaterialVar** params,
		IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, bool bBumpedEnvMap, bool hasFlashlight)
	{
		bool hasBaseTexture = params[BASETEXTURE]->IsTexture();
		bool hasBumpmap = (params[BUMPMAP]->IsTexture()); // && (!g_pHardwareConfig->PreferReducedFillrate());
		bool hasHeightMap = params[PARALLAXMAP]->IsTexture();
		bool useDotProductFix = (params[PARALLAXDOTPRODUCTFIX]->GetIntValue() == 1);
		bool heightMapInBaseAlpha = (params[BASEMAPALPHAPARALLAXMAP]->GetIntValue() == 1) && hasBaseTexture;
		bool hasDetailTexture = params[DETAIL]->IsTexture();
		bool hasEnvmap = params[ENVMAP]->IsTexture();
		bool isUnlit = params[UNLIT]->GetIntValue() == 1;

		int iDebug = r_parallaxmap_debug.GetInt() ? r_parallaxmap_debug.GetInt() : clamp(params[PARALLAXDEBUG]->GetIntValue(), 0, 2);


		if (hasBaseTexture)
		{
			SHADOW_STATE
			{
				if (params[TRANSLUCENT]->GetIntValue() > 0)
				{
					EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);
					pShaderShadow->EnableAlphaWrites(true);
				}

			// Base texture on stage 0
			if (params[BASETEXTURE]->IsTexture())
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER0, true);
				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0, true);
			}

			// Height map on stage 0
			if (params[PARALLAXMAP]->IsTexture())
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER1, true);
				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER1, true);
			}

			if (params[BUMPMAP]->IsTexture())
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER4, true);
			}

			if (hasDetailTexture)
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER8, true);
			}

			// Lightmap on stage 1
			pShaderShadow->EnableTexture(SHADER_SAMPLER2, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER2, g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE);

			unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
			if (IS_FLAG_SET(MATERIAL_VAR_MODEL))
			{
				// We need the position, surface normal, and vertex compression format
				flags |= VERTEX_FORMAT_COMPRESSED;
				pShaderShadow->VertexShaderVertexFormat(flags, 1, 0, 0);
			}
			else
			{
				// We only need the position and surface normal
				flags |= VERTEX_TANGENT_S | VERTEX_TANGENT_T;
				int numTexCoords = 8;
				pShaderShadow->VertexShaderVertexFormat(flags, numTexCoords, 0, 0);
			}

			if (params[BASETEXTURE]->IsTexture() || bBumpedEnvMap)
			{
				SetDefaultBlendingShadowState(BASETEXTURE, true);
			}

			if (hasFlashlight)
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER3, true);
				pShaderShadow->EnableTexture(SHADER_SAMPLER7, true);
				pShaderShadow->SetShadowDepthFiltering(SHADER_SAMPLER7);
				flags |= VERTEX_TANGENT_S | VERTEX_TANGENT_T | VERTEX_NORMAL;
			}

			if (hasEnvmap)
			{
				pShaderShadow->EnableTexture(SHADER_SAMPLER9, true);
				if (g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE)
				{
					pShaderShadow->EnableSRGBRead(SHADER_SAMPLER9, true);
				}
				flags |= VERTEX_TANGENT_S | VERTEX_TANGENT_T | VERTEX_NORMAL;
			}

			// Normalizing cube map
			pShaderShadow->EnableTexture(SHADER_SAMPLER6, true);

			// Envmap sampler

			// Pre-cache pixel shaders
			bool hasSelfIllum = IS_FLAG_SET(MATERIAL_VAR_SELFILLUM);

			// To do: what is this for? copied from worldtwotextureblend.
			pShaderShadow->EnableSRGBWrite(true);

			DECLARE_STATIC_VERTEX_SHADER(di_lightmapped_parallax_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(DOTPRODUCTFIX, useDotProductFix);
			SET_STATIC_VERTEX_SHADER(di_lightmapped_parallax_vs30);

			DECLARE_STATIC_PIXEL_SHADER(di_lightmapped_parallax_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(BASEMAPALPHAPARALLAXMAP, heightMapInBaseAlpha);
			SET_STATIC_PIXEL_SHADER_COMBO(SELFILLUM, hasSelfIllum);
			SET_STATIC_PIXEL_SHADER_COMBO(DETAILTEXTURE, hasDetailTexture);
			SET_STATIC_PIXEL_SHADER_COMBO(BUMPMAP, hasBumpmap);
			SET_STATIC_PIXEL_SHADER_COMBO(CUBEMAP, hasEnvmap);
			SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, hasFlashlight);
			SET_STATIC_PIXEL_SHADER_COMBO(UNLIT, isUnlit);
			SET_STATIC_PIXEL_SHADER_COMBO(PARALLAXDEBUG, iDebug);
			SET_STATIC_PIXEL_SHADER(di_lightmapped_parallax_ps30);

			// HACK HACK HACK - enable alpha writes all the time so that we have them for
			// underwater stuff. 
			// But only do it if we're not using the alpha already for translucency

			// To do: is this necessary if we won't be using parallax underwater?
			pShaderShadow->EnableAlphaWrites(true);

			//	if (hasFlashlight)
			//	{
			//		FogToBlack();
			//	}
			//	else
			{
				DefaultFog();
			}
			}

				DYNAMIC_STATE
			{
				float vParams[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			vParams[0] = params[PARALLAXMAPRANGESCALE]->GetFloatValue() * (useDotProductFix ? 0.01 : 1);
			vParams[1] = params[PARALLAXMINSTEPS]->GetIntValue();
			vParams[2] = params[PARALLAXMAXSTEPS]->GetIntValue();
			vParams[3] = params[PARALLAXOCCLUSIONRATIO]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant(0, vParams, 4);
			//	float vvParams[1] = { 1.0f };
			//	vvParams[0] = params[PARALLAXMAPRANGESCALE]->GetFloatValue() * (useDotProductFix ? 0.01 : 1);
			pShaderAPI->SetVertexShaderConstant(VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, vParams, 1);

			float vParams2[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
			vParams2[0] = params[PARALLAXOFFSET]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant(9, vParams2, 4);

			float mView[16];
			pShaderAPI->GetMatrix(MATERIAL_VIEW, mView);

			pShaderAPI->SetVertexShaderConstant(VERTEX_SHADER_SHADER_SPECIFIC_CONST_1, mView, 3);

			if (hasBaseTexture)
			{
				BindTexture(SHADER_SAMPLER0, BASETEXTURE, FRAME);
				//	SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, BASETEXTURETRANSFORM);
			}

			if (hasHeightMap)
			{
				BindTexture(SHADER_SAMPLER1, PARALLAXMAP, PARALLAXMAPFRAME);
			}

			pShaderAPI->BindStandardTexture(SHADER_SAMPLER2, TEXTURE_LIGHTMAP);

			if (hasBumpmap)
			{
				BindTexture(SHADER_SAMPLER4, BUMPMAP, BUMPFRAME);
				//	SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, BASETEXTURETRANSFORM);
			}

			if (hasBaseTexture)
			{
				SetModulationVertexShaderDynamicState();
			}

			if (hasEnvmap)
			{
				BindTexture(SHADER_SAMPLER9, ENVMAP, ENVMAPFRAME);

				vParams[0] = params[ENVMAPCONTRAST]->GetFloatValue();
				vParams[1] = params[ENVMAPSATURATION]->GetFloatValue();

				pShaderAPI->SetPixelShaderConstant(7, params[ENVMAPTINT]->GetVecValue());
				pShaderAPI->SetPixelShaderConstant(8, vParams, 1);
			}

			bool bFlashlightShadows = false;
			if (hasFlashlight)
			{
				VMatrix worldToTexture;
				ITexture *pFlashlightDepthTexture;
				FlashlightState_t state = pShaderAPI->GetFlashlightStateEx(worldToTexture, &pFlashlightDepthTexture);
				bFlashlightShadows = state.m_bEnableShadows && (pFlashlightDepthTexture != NULL);

				SetFlashLightColorFromState(state, pShaderAPI);

				BindTexture(SHADER_SAMPLER3, state.m_pSpotlightTexture, state.m_nSpotlightTextureFrame);

				if (pFlashlightDepthTexture && g_pConfig->ShadowDepthTexture())
				{
					BindTexture(SHADER_SAMPLER7, pFlashlightDepthTexture);
				}
			}

			pShaderAPI->BindStandardTexture(SHADER_SAMPLER6, TEXTURE_NORMALIZATION_CUBEMAP_SIGNED);

			MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();
			int fogIndex = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z) ? 1 : 0;

			DECLARE_DYNAMIC_VERTEX_SHADER(di_lightmapped_parallax_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(DOWATERFOG, fogIndex);
			SET_DYNAMIC_VERTEX_SHADER(di_lightmapped_parallax_vs30);

			DECLARE_DYNAMIC_PIXEL_SHADER(di_lightmapped_parallax_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo());
			SET_DYNAMIC_PIXEL_SHADER(di_lightmapped_parallax_ps30);

			float eyePos[4];
			pShaderAPI->GetWorldSpaceCameraPosition(eyePos);
			pShaderAPI->SetPixelShaderConstant(5, eyePos, 1);

			pShaderAPI->SetPixelShaderFogParams(6);

			if (hasFlashlight)
			{
				VMatrix worldToTexture;
				const FlashlightState_t &flashlightState = pShaderAPI->GetFlashlightState(worldToTexture);

				// Set the flashlight attenuation factors
				float atten[4];
				atten[0] = flashlightState.m_fConstantAtten;
				atten[1] = flashlightState.m_fLinearAtten;
				atten[2] = flashlightState.m_fQuadraticAtten;
				atten[3] = flashlightState.m_FarZ;
				pShaderAPI->SetPixelShaderConstant(20, atten, 1);

				// Set the flashlight origin
				float pos[4];
				pos[0] = flashlightState.m_vecLightOrigin[0];
				pos[1] = flashlightState.m_vecLightOrigin[1];
				pos[2] = flashlightState.m_vecLightOrigin[2];
				pos[3] = 1.0f;
				pShaderAPI->SetPixelShaderConstant(15, pos, 1);

				pShaderAPI->SetPixelShaderConstant(16, worldToTexture.Base(), 4);
			}

			SetPixelShaderConstantGammaToLinear(2, SELFILLUMTINT);
			}

			Draw();
		}
	}

	SHADER_DRAW
	{
		bool hasFlashlight = UsingFlashlight(params);
	//	bool hasBump = (params[BUMPMAP]->IsTexture()) && (!g_pHardwareConfig->PreferReducedFillrate());

	if (hasFlashlight)
	{
		DrawLightmappedParallaxedShader(params, pShaderAPI, pShaderShadow, false, false);
		SHADOW_STATE
		{
			SetInitialShadowState();
		}
	}
	DrawLightmappedParallaxedShader(params, pShaderAPI, pShaderShadow, false, hasFlashlight);
}

END_SHADER
#endif