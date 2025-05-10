#include "BaseVSShader.h"
#include "convar.h"
#include "di_lightmapped_ice_vs30.inc"
#include "di_lightmapped_ice_ps30.inc"

BEGIN_VS_SHADER(DI_LightmappedIce_DX90, "DI_LightmappedIce_DX90")

BEGIN_SHADER_PARAMS
SHADER_PARAM(BACKSURFACE, SHADER_PARAM_TYPE_BOOL, "0.0", "specify that this is the back surface of the ice")

SHADER_PARAM(BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "normal map")
SHADER_PARAM(SPECMASKTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "specular reflection mask")
SHADER_PARAM(LIGHTWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "1D ramp texture for tinting scalar diffuse term")
SHADER_PARAM(FRESNELWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "1D ramp texture for controlling fresnel falloff")
SHADER_PARAM(OPACITYTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "1D ramp texture for controlling fresnel falloff")
SHADER_PARAM(ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "", "environment map")

SHADER_PARAM(UVSCALE, SHADER_PARAM_TYPE_FLOAT, "0.02", "uv projection scale")
SHADER_PARAM(BUMPSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "1.0", "bump map strength")
SHADER_PARAM(FRESNELBUMPSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "1.0", "bump map strength for fresnel")
SHADER_PARAM(TRANSLUCENTFRESNELMINMAXEXP, SHADER_PARAM_TYPE_VEC3, "[0.8 1.0 1.0]", "fresnel params")

SHADER_PARAM(INTERIOR, SHADER_PARAM_TYPE_BOOL, "1", "Enable interior layer")
SHADER_PARAM(INTERIORFOGSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "0.06", "fog strength (scales with thickness of the interior volume)")
SHADER_PARAM(INTERIORFOGLIMIT, SHADER_PARAM_TYPE_FLOAT, "0.8", "fog opacity beyond the range of destination alpha depth (in low-precision depth mode)")
SHADER_PARAM(INTERIORFOGNORMALBOOST, SHADER_PARAM_TYPE_FLOAT, "0.0", "degree to boost interior thickness/fog by 'side-on'ness of vertex normals to the camera")
SHADER_PARAM(INTERIORBACKGROUNDBOOST, SHADER_PARAM_TYPE_FLOAT, "7", "boosts the brightness of bright background pixels")
SHADER_PARAM(INTERIORAMBIENTSCALE, SHADER_PARAM_TYPE_FLOAT, "0.3", "scales ambient light in the interior volume");
SHADER_PARAM(INTERIORBACKLIGHTSCALE, SHADER_PARAM_TYPE_FLOAT, "0.3", "scales backlighting in the interior volume");
SHADER_PARAM(INTERIORCOLOR, SHADER_PARAM_TYPE_VEC3, "[0.7 0.5 0.45]", "tints light in the interior volume")
SHADER_PARAM(INTERIORREFRACTSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "0.015", "strength of bumped refract of the background seen through the interior")
SHADER_PARAM(INTERIORREFRACTBLUR, SHADER_PARAM_TYPE_FLOAT, "0.2", "strength of blur applied to the background seen through the interior")

SHADER_PARAM(DIFFUSESCALE, SHADER_PARAM_TYPE_FLOAT, "1.0", "")
SHADER_PARAM(PHONGEXPONENT, SHADER_PARAM_TYPE_FLOAT, "1.0", "specular exponent")
SHADER_PARAM(PHONGBOOST, SHADER_PARAM_TYPE_FLOAT, "1.0", "specular boost")
SHADER_PARAM(PHONGEXPONENT2, SHADER_PARAM_TYPE_FLOAT, "1.0", "specular exponent")
SHADER_PARAM(PHONGBOOST2, SHADER_PARAM_TYPE_FLOAT, "1.0", "specular boost")
SHADER_PARAM(RIMLIGHTEXPONENT, SHADER_PARAM_TYPE_FLOAT, "1.0", "rim light exponent")
SHADER_PARAM(RIMLIGHTBOOST, SHADER_PARAM_TYPE_FLOAT, "1.0", "rim light boost")
SHADER_PARAM(BASECOLORTINT, SHADER_PARAM_TYPE_VEC3, "[1.0 1.0 1.0]", "base texture tint")
SHADER_PARAM(ENVMAPTINT, SHADER_PARAM_TYPE_VEC3, "[1.0 1.0 1.0]", "tints the environment reflection")

SHADER_PARAM(UVPROJOFFSET, SHADER_PARAM_TYPE_VEC3, "[0 0 0]", "Center for UV projection")
SHADER_PARAM(BBMIN, SHADER_PARAM_TYPE_VEC3, "[0 0 0]", "")
SHADER_PARAM(BBMAX, SHADER_PARAM_TYPE_VEC3, "[0 0 0]", "")

SHADER_PARAM(CONTACTSHADOWS, SHADER_PARAM_TYPE_BOOL, "1", "")

SHADER_PARAM(BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "")

SHADER_PARAM(TRANSLUCENT, SHADER_PARAM_TYPE_INTEGER, "0", "translucency toggle")
END_SHADER_PARAMS

SHADER_FALLBACK
{
	if (!g_pHardwareConfig->SupportsShaderModel_3_0())
		return "LightmappedGeneric";

	if (g_pHardwareConfig->GetDXSupportLevel() < 90)
		return "LightmappedGeneric_DX8";

	return 0;
}

// Set up anything that is necessary to make decisions in SHADER_FALLBACK.
SHADER_INIT_PARAMS()
{
	InitFloatParam(UVSCALE, params, 0.02f);
	InitFloatParam(BUMPSTRENGTH, params, 1.0f);
	InitFloatParam(FRESNELBUMPSTRENGTH, params, 0.4f);

	InitIntParam(INTERIOR, params, 1);
	InitFloatParam(INTERIORFOGSTRENGTH, params, 0.06f);
	InitFloatParam(INTERIORFOGLIMIT, params, 0.8f);
	InitFloatParam(INTERIORFOGNORMALBOOST, params, 1.0f);
	InitFloatParam(INTERIORBACKGROUNDBOOST, params, 1.0f);
	InitFloatParam(INTERIORAMBIENTSCALE, params, 3.0f);
	InitFloatParam(INTERIORBACKLIGHTSCALE, params, 3.0f);
	InitFloatParam(INTERIORREFRACTSTRENGTH, params, 0.1f);
	InitFloatParam(INTERIORREFRACTBLUR, params, 0.7f);

	InitFloatParam(DIFFUSESCALE, params, 1.0f);
	InitFloatParam(PHONGEXPONENT, params, 40.0f);
	InitFloatParam(PHONGBOOST, params, 8.0f);
	InitFloatParam(PHONGEXPONENT2, params, 8.0f);
	InitFloatParam(PHONGBOOST2, params, 20.0f);
	InitFloatParam(RIMLIGHTEXPONENT, params, 8.0f);
	InitFloatParam(RIMLIGHTBOOST, params, 20.0f);

	InitIntParam(CONTACTSHADOWS, params, 0);
	InitIntParam(BUMPFRAME, params, 0);
	InitIntParam(TRANSLUCENT, params, 0);

	if (!params[BACKSURFACE]->IsDefined())
	{
		params[BACKSURFACE]->SetIntValue(0);
	}
	SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);
	if (!params[UVSCALE]->IsDefined())
	{
		params[UVSCALE]->SetFloatValue(0.02f);
	}
	if (!params[BUMPSTRENGTH]->IsDefined())
	{
		params[BUMPSTRENGTH]->SetFloatValue(1.0f);
	}
	if (!params[FRESNELBUMPSTRENGTH]->IsDefined())
	{
		params[FRESNELBUMPSTRENGTH]->SetFloatValue(1.0f);
	}
	if (!params[TRANSLUCENTFRESNELMINMAXEXP]->IsDefined())
	{
		params[TRANSLUCENTFRESNELMINMAXEXP]->SetVecValue(0.8f, 1.0f, 1.0f);
	}
	if (!params[INTERIOR]->IsDefined())
	{
		params[INTERIOR]->SetIntValue(1);
	}
	if (!params[INTERIORFOGSTRENGTH]->IsDefined())
	{
		params[INTERIORFOGSTRENGTH]->SetFloatValue(0.06f);
	}
	if (!params[INTERIORFOGLIMIT]->IsDefined())
	{
		params[INTERIORFOGLIMIT]->SetFloatValue(0.8f);
	}
	if (!params[INTERIORFOGNORMALBOOST]->IsDefined())
	{
		params[INTERIORFOGNORMALBOOST]->SetFloatValue(1.0f);
	}
	if (!params[INTERIORBACKGROUNDBOOST]->IsDefined())
	{
		params[INTERIORBACKGROUNDBOOST]->SetFloatValue(1.0f);
	}
	if (!params[INTERIORAMBIENTSCALE]->IsDefined())
	{
		params[INTERIORAMBIENTSCALE]->SetFloatValue(3.0f);
	}
	if (!params[INTERIORBACKLIGHTSCALE]->IsDefined())
	{
		params[INTERIORBACKLIGHTSCALE]->SetFloatValue(3.0f);
	}
	if (!params[INTERIORCOLOR]->IsDefined())
	{
		params[INTERIORCOLOR]->SetVecValue(0.5f, 0.7f, 0.66f);
	}
	if (!params[INTERIORREFRACTSTRENGTH]->IsDefined())
	{
		params[INTERIORREFRACTSTRENGTH]->SetFloatValue(0.1f);
	}
	if (!params[INTERIORREFRACTBLUR]->IsDefined())
	{
		params[INTERIORREFRACTBLUR]->SetFloatValue(0.7f);
	}
	if (!params[DIFFUSESCALE]->IsDefined())
	{
		params[DIFFUSESCALE]->SetFloatValue(1.0f);
	}
	if (!params[PHONGEXPONENT]->IsDefined())
	{
		params[PHONGEXPONENT]->SetFloatValue(40.0f);
	}
	if (!params[PHONGBOOST]->IsDefined())
	{
		params[PHONGBOOST]->SetFloatValue(8.0f);
	}
	if (!params[PHONGEXPONENT2]->IsDefined())
	{
		params[PHONGEXPONENT2]->SetFloatValue(8.0f);
	}
	if (!params[PHONGBOOST2]->IsDefined())
	{
		params[PHONGBOOST2]->SetFloatValue(20.0f);
	}
	if (!params[RIMLIGHTEXPONENT]->IsDefined())
	{
		params[RIMLIGHTEXPONENT]->SetFloatValue(8.0f);
	}
	if (!params[RIMLIGHTBOOST]->IsDefined())
	{
		params[RIMLIGHTBOOST]->SetFloatValue(20.0f);
	}
	if (!params[BASECOLORTINT]->IsDefined())
	{
		params[BASECOLORTINT]->SetVecValue(1.0f, 1.0f, 1.0f);
	}
	if (!params[ENVMAPTINT]->IsDefined())
	{
		params[ENVMAPTINT]->SetVecValue(1.0f, 1.0f, 1.0f);
	}
	if (!params[UVPROJOFFSET]->IsDefined())
	{
		params[UVPROJOFFSET]->SetVecValue(0.0f, 0.0f, 0.0f);
	}
	if (!params[BBMIN]->IsDefined())
	{
		params[BBMIN]->SetVecValue(0.0f, 0.0f, 0.0f);
	}
	if (!params[BBMAX]->IsDefined())
	{
		params[BBMAX]->SetVecValue(0.0f, 0.0f, 0.0f);
	}
	if (!params[CONTACTSHADOWS]->IsDefined())
	{
		params[CONTACTSHADOWS]->SetIntValue(0);
	}
	if (!params[BUMPFRAME]->IsDefined())
	{
		params[BUMPFRAME]->SetIntValue(0);
	}
	
	if (g_pHardwareConfig->SupportsBorderColor())
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight_border");
	}
	else
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight001");
	}

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
	SET_FLAGS2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);

	if (g_pConfig->UseBumpmapping() && params[BUMPMAP]->IsDefined())
	{
		SET_FLAGS2(MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP);
	}

	SET_FLAGS(MATERIAL_VAR_TRANSLUCENT);

	if (!params[TRANSLUCENT]->IsDefined() || params[TRANSLUCENT]->GetIntValue() == 0)
	{
		CLEAR_FLAGS(MATERIAL_VAR_TRANSLUCENT);
	}
	
	// No texture means no self-illum or env mask in base alpha
	if (!params[BASETEXTURE]->IsDefined())
	{
		CLEAR_FLAGS(MATERIAL_VAR_SELFILLUM);
		CLEAR_FLAGS(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	}

	if (!params[ENVMAPTINT]->IsDefined())
		params[ENVMAPTINT]->SetVecValue(1.0f, 1.0f, 1.0f);
}

SHADER_INIT
{
	if (params[BASETEXTURE]->IsDefined())
	{
		LoadTexture(BASETEXTURE, TEXTUREFLAGS_SRGB);
	}
	if (params[BUMPMAP]->IsDefined())
	{
		LoadTexture(BUMPMAP);
	}
	if (params[SPECMASKTEXTURE]->IsDefined())
	{
		LoadTexture(SPECMASKTEXTURE);
	}
	if (params[LIGHTWARPTEXTURE]->IsDefined())
	{
		LoadTexture(LIGHTWARPTEXTURE, TEXTUREFLAGS_SRGB);
	}
	if (params[FRESNELWARPTEXTURE]->IsDefined())
	{
		LoadTexture(FRESNELWARPTEXTURE);
	}
	if (params[OPACITYTEXTURE]->IsDefined())
	{
		LoadTexture(OPACITYTEXTURE);
	}
	if (params[ENVMAP]->IsDefined())
	{
		LoadTexture(ENVMAP, TEXTUREFLAGS_SRGB);
	}
	if (params[ENVMAP]->IsDefined())
	{
		LoadTexture(ENVMAP, TEXTUREFLAGS_SRGB);
	}
	if (params[FLASHLIGHTTEXTURE]->IsDefined())
	{
		LoadTexture(FLASHLIGHTTEXTURE, TEXTUREFLAGS_SRGB);
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

void DrawLightmappedIceShader(IMaterialVar** params,
	IShaderDynamicAPI *pShaderAPI, IShaderShadow* pShaderShadow, bool bBumpedEnvMap, bool hasFlashlight)
{
	bool bHasBaseTexture = params[BASETEXTURE]->IsTexture();
	bool bHasBumpMap = params[BUMPMAP]->IsTexture();
	bool bHasFlashlight = UsingFlashlight(params);
	bool bBackSurface = (params[BACKSURFACE]->GetIntValue() > 0);
	bool bHasLightWarp = params[LIGHTWARPTEXTURE]->IsDefined() && params[LIGHTWARPTEXTURE]->IsTexture();
	bool bHasFresnelWarp = params[FRESNELWARPTEXTURE]->IsDefined() && params[FRESNELWARPTEXTURE]->IsTexture();
	bool bHasOpacityTexture = params[OPACITYTEXTURE]->IsDefined() && params[OPACITYTEXTURE]->IsTexture();
	bool bInteriorLayer = (params[INTERIOR]->GetIntValue() > 0);
	bool bContactShadows = (params[CONTACTSHADOWS]->GetIntValue() > 0);
	bool bHasSpecMap = params[SPECMASKTEXTURE]->IsDefined() && params[SPECMASKTEXTURE]->IsTexture();
	bool bHasEnvMap = params[ENVMAP]->IsDefined() && params[ENVMAP]->IsTexture();
	
	SHADOW_STATE
	{
		// Reset shadow state manually since we're drawing from two materials
		SetInitialShadowState();

		// Base texture on stage 0
		if (params[BASETEXTURE]->IsTexture())
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER0, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0, true);

			if (params[TRANSLUCENT]->GetIntValue() > 0)
			{
				EnableAlphaBlending(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);
				pShaderShadow->EnableAlphaWrites(false);
			}
		}

		// Bump map on stage 0
		if (params[BUMPMAP]->IsTexture())
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER1, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER1, false);
		}

		// [sRGB] Backbuffer
		pShaderShadow->EnableTexture(SHADER_SAMPLER2, true);
		pShaderShadow->EnableSRGBRead(SHADER_SAMPLER2, true);

		//  Spec mask
		if (bHasSpecMap)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER3, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER3, false);
		}

		// [sRGB] Light warp
		if (bHasLightWarp)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER4, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER4, true);
		}

		// Fresnel warp	// TODO: Could be in alpha of lightwarp
		if (bHasFresnelWarp)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER5, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER5, false);
		}

		// Opacity
		if (bHasOpacityTexture)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER6, true);
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER6, false);
		}

		pShaderShadow->EnableSRGBWrite(true);
		pShaderShadow->EnableAlphaWrites(true);

		int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_TANGENT_S | VERTEX_TANGENT_T;
		int numTexCoords = 8;
		pShaderShadow->VertexShaderVertexFormat(flags, numTexCoords, 0, 0);

		int nShadowFilterMode = 0;

		if (g_pHardwareConfig->SupportsShaderModel_3_0())
		{
			nShadowFilterMode = g_pHardwareConfig->GetShadowFilterMode();	// Based upon vendor and device dependent formats
		}

		if (params[BASETEXTURE]->IsTexture() || bBumpedEnvMap)
		{
			SetDefaultBlendingShadowState(BASETEXTURE, true);
		}
		
		if (hasFlashlight)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER8, true);	//		 Shadow depth map
			pShaderShadow->SetShadowDepthFiltering(SHADER_SAMPLER8);
			pShaderShadow->EnableTexture(SHADER_SAMPLER9, true);	//		 Noise map
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER9, false);
			pShaderShadow->EnableTexture(SHADER_SAMPLER10, true);	//[sRGB] Flashlight cookie
			pShaderShadow->EnableSRGBRead(SHADER_SAMPLER10, true);
			flags |= VERTEX_TANGENT_S | VERTEX_TANGENT_T | VERTEX_NORMAL;
		}

		// [sRGB] Envmap
		if (bHasEnvMap)
		{
			pShaderShadow->EnableTexture(SHADER_SAMPLER7, true);
			if (g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE)
			{
				pShaderShadow->EnableSRGBRead(SHADER_SAMPLER9, true);
			}
			flags |= VERTEX_TANGENT_S | VERTEX_TANGENT_T | VERTEX_NORMAL;
		}
		
		DECLARE_STATIC_VERTEX_SHADER(di_lightmapped_ice_vs30);
		SET_STATIC_VERTEX_SHADER(di_lightmapped_ice_vs30);

		DECLARE_STATIC_PIXEL_SHADER(di_lightmapped_ice_ps30);
		SET_STATIC_PIXEL_SHADER_COMBO(BACK_SURFACE, bBackSurface);
		SET_STATIC_PIXEL_SHADER_COMBO(LIGHT_WARP, bHasLightWarp);
		SET_STATIC_PIXEL_SHADER_COMBO(FRESNEL_WARP, bHasFresnelWarp);
		SET_STATIC_PIXEL_SHADER_COMBO(OPACITY_TEXTURE, bHasOpacityTexture);
		SET_STATIC_PIXEL_SHADER_COMBO(INTERIOR_LAYER, bInteriorLayer);
		SET_STATIC_PIXEL_SHADER_COMBO(HIGH_PRECISION_DEPTH, (g_pHardwareConfig->GetHDRType() == HDR_TYPE_FLOAT) ? true : false);
		SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode);
		SET_STATIC_PIXEL_SHADER_COMBO(CONTACT_SHADOW, bContactShadows);	// only do contact shadows on outer shell (which has interior layer enabled)
		SET_STATIC_PIXEL_SHADER(di_lightmapped_ice_ps30);

		// HACK HACK HACK - enable alpha writes all the time so that we have them for
		// underwater stuff. 
		// But only do it if we're not using the alpha already for translucency

		// To do: is this necessary if we won't be using ice underwater?
		pShaderShadow->EnableAlphaWrites(true);

		if (hasFlashlight)
		{
			FogToBlack();
		}
		else
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
		pShaderAPI->SetVertexShaderConstant(VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, vParams, 1);
		pShaderAPI->SetPixelShaderConstant(0, vParams, 1);

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

		DECLARE_DYNAMIC_VERTEX_SHADER(di_lightmapped_ice_vs30);
		SET_DYNAMIC_VERTEX_SHADER_COMBO(DOWATERFOG, fogIndex);
		SET_DYNAMIC_VERTEX_SHADER(di_lightmapped_ice_vs30);

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

SHADER_DRAW
{
	bool hasFlashlight = UsingFlashlight(params);
//	bool hasBump = (params[BUMPMAP]->IsTexture()) && (!g_pHardwareConfig->PreferReducedFillrate());

	if (hasFlashlight)
	{
		DrawLightmappedIceShader(params, pShaderAPI, pShaderShadow, false, false);
		SHADOW_STATE
		{
			SetInitialShadowState();
		}
	}
	DrawLightmappedIceShader(params, pShaderAPI, pShaderShadow, false, hasFlashlight);
}

END_SHADER
