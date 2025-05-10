//========================================================================//
//
// Purpose: Lightmap only shader
// DI changes - added emissive blend, flesh interior and cloak passes,
// parallax-corrected cubemaps, and removed outline and softedges.
// $Header: $
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"
#include "convar.h"
#include "lightmappedgeneric_dx9_helper.h"
#ifdef DARKINTERVAL
#include "emissive_scroll_blended_pass_helper.h"
#include "flesh_interior_blended_pass_helper.h"
#include "cloak_blended_pass_helper.h"
#endif
static LightmappedGeneric_DX9_Vars_t s_info;
#ifdef DARKINTERVAL
BEGIN_VS_SHADER( DI_LightmappedGeneric_DX90,
				 "Help for DI_LightmappedGeneric_DX90" )
#else // "SDK_" to make it compile and not replace stock shaders (forbidden)
BEGIN_VS_SHADER(SDK_LightmappedGeneric,
	"Help for LightmappedGeneric" )
#endif
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( ALBEDO, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "albedo (Base texture with no baked lighting)" )
		SHADER_PARAM( SELFILLUMTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint" )
		SHADER_PARAM( DETAIL, SHADER_PARAM_TYPE_TEXTURE, "shadertest/detail", "detail texture" )
		SHADER_PARAM( DETAILFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $detail" )
		SHADER_PARAM( DETAILSCALE, SHADER_PARAM_TYPE_FLOAT, "4", "scale of the detail texture" )

		SHADER_PARAM( ALPHA2, SHADER_PARAM_TYPE_FLOAT, "1", "" )

		// detail (multi-) texturing
		SHADER_PARAM( DETAILBLENDMODE, SHADER_PARAM_TYPE_INTEGER, "0", "mode for combining detail texture with base. 0=normal, 1= additive, 2=alpha blend detail over base, 3=crossfade" )
		SHADER_PARAM( DETAILBLENDFACTOR, SHADER_PARAM_TYPE_FLOAT, "1", "blend amount for detail texture." )
		SHADER_PARAM( DETAILTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "detail texture tint" )

		SHADER_PARAM( ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "envmap" )
		SHADER_PARAM( ENVMAPFRAME, SHADER_PARAM_TYPE_INTEGER, "", "" )
		SHADER_PARAM( ENVMAPMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_envmask", "envmap mask" )
		SHADER_PARAM( ENVMAPMASKFRAME, SHADER_PARAM_TYPE_INTEGER, "", "" ) // none of the assets make use of these parameters
#ifndef DARKINTERVAL
		SHADER_PARAM( ENVMAPMASKTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$envmapmask texcoord transform" )
#endif		
		SHADER_PARAM( ENVMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "envmap tint" )
		SHADER_PARAM( BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "bump map" )
		SHADER_PARAM( BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap" )
		SHADER_PARAM( BUMPTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform" )
		SHADER_PARAM( ENVMAPCONTRAST, SHADER_PARAM_TYPE_FLOAT, "0.0", "contrast 0 == normal 1 == color*color" )
		SHADER_PARAM( ENVMAPSATURATION, SHADER_PARAM_TYPE_FLOAT, "1.0", "saturation 0 == greyscale 1 == normal" )
		SHADER_PARAM( FRESNELREFLECTION, SHADER_PARAM_TYPE_FLOAT, "1.0", "1.0 == mirror, 0.0 == water" )
		SHADER_PARAM( NORMALMAPALPHAENVMAPMASK, SHADER_PARAM_TYPE_INTEGER, "0", "Enables masking the envmap by the alpha channel of the bumpmap")
		SHADER_PARAM( NODIFFUSEBUMPLIGHTING, SHADER_PARAM_TYPE_INTEGER, "0", "0 == Use diffuse bump lighting, 1 = No diffuse bump lighting" )
		SHADER_PARAM( BUMPMAP2, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader3_normal", "bump map" )
		SHADER_PARAM( BUMPFRAME2, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap" )
		SHADER_PARAM( BUMPTRANSFORM2, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform" )
		SHADER_PARAM( BUMPMASK, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader3_normal", "bump map" )
		SHADER_PARAM( BASETEXTURE2, SHADER_PARAM_TYPE_TEXTURE, "shadertest/lightmappedtexture", "Blended texture" )
		SHADER_PARAM( FRAME2, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $basetexture2" )
		SHADER_PARAM( BASETEXTURENOENVMAP, SHADER_PARAM_TYPE_BOOL, "0", "" )
		SHADER_PARAM( BASETEXTURE2NOENVMAP, SHADER_PARAM_TYPE_BOOL, "0", "" )
		SHADER_PARAM( DETAIL_ALPHA_MASK_BASE_TEXTURE, SHADER_PARAM_TYPE_BOOL, "0", 
			"If this is 1, then when detail alpha=0, no base texture is blended and when "
			"detail alpha=1, you get detail*base*lightmap" )
		SHADER_PARAM( LIGHTWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "light munging lookup texture" )
		SHADER_PARAM( BLENDMODULATETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "texture to use r/g channels for blend range for" )
		SHADER_PARAM( MASKEDBLENDING, SHADER_PARAM_TYPE_INTEGER, "0", "blend using texture with no vertex alpha. For using texture blending on non-displacements" )
		SHADER_PARAM( BLENDMASKTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$blendmodulatetexture texcoord transform" )
		SHADER_PARAM( SSBUMP, SHADER_PARAM_TYPE_INTEGER, "0", "whether or not to use alternate bumpmap format with height" )
		SHADER_PARAM( SEAMLESS_SCALE, SHADER_PARAM_TYPE_FLOAT, "0", "Scale factor for 'seamless' texture mapping. 0 means to use ordinary mapping" )
		SHADER_PARAM( ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0.0", "" )	
#ifndef DARKINTERVAL // thrown out
		SHADER_PARAM(SOFTEDGES, SHADER_PARAM_TYPE_BOOL, "0", "Enable soft edges to distance coded textures.")
		SHADER_PARAM(EDGESOFTNESSSTART, SHADER_PARAM_TYPE_FLOAT, "0.6", "Start value for soft edges for distancealpha.");
		SHADER_PARAM(EDGESOFTNESSEND, SHADER_PARAM_TYPE_FLOAT, "0.5", "End value for soft edges for distancealpha.");

		SHADER_PARAM(OUTLINE, SHADER_PARAM_TYPE_BOOL, "0", "Enable outline for distance coded textures.")
		SHADER_PARAM(OUTLINECOLOR, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "color of outline for distance coded images.")
		SHADER_PARAM(OUTLINEALPHA, SHADER_PARAM_TYPE_FLOAT, "0.0", "alpha value for outline")
		SHADER_PARAM(OUTLINESTART0, SHADER_PARAM_TYPE_FLOAT, "0.0", "outer start value for outline")
		SHADER_PARAM(OUTLINESTART1, SHADER_PARAM_TYPE_FLOAT, "0.0", "inner start value for outline")
		SHADER_PARAM(OUTLINEEND0, SHADER_PARAM_TYPE_FLOAT, "0.0", "inner end value for outline")
		SHADER_PARAM(OUTLINEEND1, SHADER_PARAM_TYPE_FLOAT, "0.0", "outer end value for outline")
#endif
#ifdef DARKINTERVAL
		// Emissive Scroll Pass
		SHADER_PARAM(EMISSIVEBLENDENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enable emissive blend pass")
		SHADER_PARAM(EMISSIVEBLENDBASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "self-illumination map")
		SHADER_PARAM(EMISSIVEBLENDSCROLLVECTOR, SHADER_PARAM_TYPE_VEC2, "[0.11 0.124]", "Emissive scroll vec")
		SHADER_PARAM(EMISSIVEBLENDSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "1.0", "Emissive blend strength")
		SHADER_PARAM(EMISSIVEBLENDTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "self-illumination map")
		SHADER_PARAM(EMISSIVEBLENDTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint")
		SHADER_PARAM(EMISSIVEBLENDFLOWTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "flow map")

		// Flesh Interior Pass
		SHADER_PARAM(FLESHINTERIORENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enable Flesh interior blend pass")
		SHADER_PARAM(FLESHINTERIORTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh color texture")
		SHADER_PARAM(FLESHINTERIORNOISETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh noise texture")
		SHADER_PARAM(FLESHBORDERTEXTURE1D, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh border 1D texture")
		SHADER_PARAM(FLESHNORMALTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh normal texture")
		SHADER_PARAM(FLESHSUBSURFACETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh subsurface texture")
		SHADER_PARAM(FLESHCUBETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh cubemap texture")
		SHADER_PARAM(FLESHBORDERNOISESCALE, SHADER_PARAM_TYPE_FLOAT, "1.5", "Flesh Noise UV scalar for border")
		SHADER_PARAM(FLESHDEBUGFORCEFLESHON, SHADER_PARAM_TYPE_BOOL, "0", "Flesh Debug full flesh")
		SHADER_PARAM(FLESHEFFECTCENTERRADIUS1, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius")
		SHADER_PARAM(FLESHEFFECTCENTERRADIUS2, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius")
		SHADER_PARAM(FLESHEFFECTCENTERRADIUS3, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius")
		SHADER_PARAM(FLESHEFFECTCENTERRADIUS4, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius")
		SHADER_PARAM(FLESHSUBSURFACETINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Subsurface Color")
		SHADER_PARAM(FLESHBORDERWIDTH, SHADER_PARAM_TYPE_FLOAT, "0.3", "Flesh border")
		SHADER_PARAM(FLESHBORDERSOFTNESS, SHADER_PARAM_TYPE_FLOAT, "0.42", "Flesh border softness (> 0.0 && <= 0.5)")
		SHADER_PARAM(FLESHBORDERTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Flesh border Color")
		SHADER_PARAM(FLESHGLOBALOPACITY, SHADER_PARAM_TYPE_FLOAT, "1.0", "Flesh global opacity")
		SHADER_PARAM(FLESHGLOSSBRIGHTNESS, SHADER_PARAM_TYPE_FLOAT, "0.66", "Flesh gloss brightness")
		SHADER_PARAM(FLESHSCROLLSPEED, SHADER_PARAM_TYPE_FLOAT, "1.0", "Flesh scroll speed")
	
		// Cloak Pass
		SHADER_PARAM(CLOAKPASSENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enables cloak render in a second pass")
		SHADER_PARAM(CLOAKFACTOR, SHADER_PARAM_TYPE_FLOAT, "0.0", "")
		SHADER_PARAM(CLOAKCOLORTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Cloak color tint")
		SHADER_PARAM(REFRACTAMOUNT, SHADER_PARAM_TYPE_FLOAT, "2", "")

		// Parallax cubemaps
		SHADER_PARAM(ENVMAPPARALLAXOBB1, SHADER_PARAM_TYPE_VEC4, "[1 0 0 0]", "The first line of the parallax correction OBB matrix")
		SHADER_PARAM(ENVMAPPARALLAXOBB2, SHADER_PARAM_TYPE_VEC4, "[0 1 0 0]", "The second line of the parallax correction OBB matrix")
		SHADER_PARAM(ENVMAPPARALLAXOBB3, SHADER_PARAM_TYPE_VEC4, "[0 0 1 0]", "The third line of the parallax correction OBB matrix")
		SHADER_PARAM(ENVMAPORIGIN, SHADER_PARAM_TYPE_VEC3, "[0 0 0]", "The world space position of the env_cubemap being corrected")
#endif

END_SHADER_PARAMS

	void SetupVars( LightmappedGeneric_DX9_Vars_t& info )
	{
		info.m_nBaseTexture = BASETEXTURE;
		info.m_nBaseTextureFrame = FRAME;
		info.m_nBaseTextureTransform = BASETEXTURETRANSFORM;
		info.m_nAlbedo = ALBEDO;
		info.m_nSelfIllumTint = SELFILLUMTINT;

		info.m_nAlpha2 = ALPHA2;

		info.m_nDetail = DETAIL;
		info.m_nDetailFrame = DETAILFRAME;
		info.m_nDetailScale = DETAILSCALE;
		info.m_nDetailTextureCombineMode = DETAILBLENDMODE;
		info.m_nDetailTextureBlendFactor = DETAILBLENDFACTOR;
		info.m_nDetailTint = DETAILTINT;

		info.m_nEnvmap = ENVMAP;
		info.m_nEnvmapFrame = ENVMAPFRAME;
		info.m_nEnvmapMask = ENVMAPMASK;
		info.m_nEnvmapMaskFrame = 0;
#ifndef DARKINTERVAL
		info.m_nEnvmapMaskTransform = ENVMAPMASKTRANSFORM;
#endif
		info.m_nEnvmapTint = ENVMAPTINT;
		info.m_nBumpmap = BUMPMAP;
		info.m_nBumpFrame = BUMPFRAME;
		info.m_nBumpTransform = BUMPTRANSFORM;
		info.m_nEnvmapContrast = ENVMAPCONTRAST;
		info.m_nEnvmapSaturation = ENVMAPSATURATION;
		info.m_nFresnelReflection = FRESNELREFLECTION;
		info.m_nNoDiffuseBumpLighting = NODIFFUSEBUMPLIGHTING;
		info.m_nNormalMapAlphaEnvMapMask = NORMALMAPALPHAENVMAPMASK;
		info.m_nBumpmap2 = BUMPMAP2;
		info.m_nBumpFrame2 = BUMPFRAME2;
		info.m_nBumpTransform2 = BUMPTRANSFORM2;
		info.m_nBumpMask = BUMPMASK;
		info.m_nBaseTexture2 = BASETEXTURE2;
		info.m_nBaseTexture2Frame = FRAME2;
		info.m_nBaseTextureNoEnvmap = BASETEXTURENOENVMAP;
		info.m_nBaseTexture2NoEnvmap = BASETEXTURE2NOENVMAP;
		info.m_nDetailAlphaMaskBaseTexture = DETAIL_ALPHA_MASK_BASE_TEXTURE;
		info.m_nFlashlightTexture = FLASHLIGHTTEXTURE;
		info.m_nFlashlightTextureFrame = FLASHLIGHTTEXTUREFRAME;
		info.m_nLightWarpTexture = LIGHTWARPTEXTURE;
		info.m_nBlendModulateTexture = BLENDMODULATETEXTURE;
		info.m_nMaskedBlending = MASKEDBLENDING;
		info.m_nBlendMaskTransform = BLENDMASKTRANSFORM;
		info.m_nSelfShadowedBumpFlag = SSBUMP;
		info.m_nSeamlessMappingScale = 0.0f;
		info.m_nAlphaTestReference = ALPHATESTREFERENCE;
#ifndef DARKINTERVAL // thrown out
		info.m_nSoftEdges = SOFTEDGES;
		info.m_nEdgeSoftnessStart = EDGESOFTNESSSTART;
		info.m_nEdgeSoftnessEnd = EDGESOFTNESSEND;
		info.m_nOutline = OUTLINE;
		info.m_nOutlineColor = OUTLINECOLOR;
		info.m_nOutlineAlpha = OUTLINEALPHA;
		info.m_nOutlineStart0 = OUTLINESTART0;
		info.m_nOutlineStart1 = OUTLINESTART1;
		info.m_nOutlineEnd0 = OUTLINEEND0;
		info.m_nOutlineEnd1 = OUTLINEEND1;
#endif
#ifdef DARKINTERVAL
		// Parallax cubemaps
		info.m_nEnvmapParallaxObb1 = ENVMAPPARALLAXOBB1;
		info.m_nEnvmapParallaxObb2 = ENVMAPPARALLAXOBB2;
		info.m_nEnvmapParallaxObb3 = ENVMAPPARALLAXOBB3;
		info.m_nEnvmapOrigin = ENVMAPORIGIN;
#endif
	}

	SHADER_FALLBACK
	{
		if( g_pHardwareConfig->GetDXSupportLevel() < 90 )
			return "LightmappedGeneric_DX8";

		return 0;
	}
#ifdef DARKINTERVAL
	// Emissive Scroll Pass
	void SetupVarsEmissiveScrollBlendedPass(EmissiveScrollBlendedPassVars_t &info)
	{
		info.m_nBlendStrength = EMISSIVEBLENDSTRENGTH;
		info.m_nBaseTexture = EMISSIVEBLENDBASETEXTURE;
		info.m_nFlowTexture = EMISSIVEBLENDFLOWTEXTURE;
		info.m_nEmissiveTexture = EMISSIVEBLENDTEXTURE;
		info.m_nEmissiveTint = EMISSIVEBLENDTINT;
		info.m_nEmissiveScrollVector = EMISSIVEBLENDSCROLLVECTOR;
	}

	// Flesh Interior Pass
	void SetupVarsFleshInteriorBlendedPass(FleshInteriorBlendedPassVars_t &info)
	{
		info.m_nFleshTexture = FLESHINTERIORTEXTURE;
		info.m_nFleshNoiseTexture = FLESHINTERIORNOISETEXTURE;
		info.m_nFleshBorderTexture1D = FLESHBORDERTEXTURE1D;
		info.m_nFleshNormalTexture = FLESHNORMALTEXTURE;
		info.m_nFleshSubsurfaceTexture = FLESHSUBSURFACETEXTURE;
		info.m_nFleshCubeTexture = FLESHCUBETEXTURE;

		info.m_nflBorderNoiseScale = FLESHBORDERNOISESCALE;
		info.m_nflDebugForceFleshOn = FLESHDEBUGFORCEFLESHON;
		info.m_nvEffectCenterRadius1 = FLESHEFFECTCENTERRADIUS1;
		info.m_nvEffectCenterRadius2 = FLESHEFFECTCENTERRADIUS2;
		info.m_nvEffectCenterRadius3 = FLESHEFFECTCENTERRADIUS3;
		info.m_nvEffectCenterRadius4 = FLESHEFFECTCENTERRADIUS4;

		info.m_ncSubsurfaceTint = FLESHSUBSURFACETINT;
		info.m_nflBorderWidth = FLESHBORDERWIDTH;
		info.m_nflBorderSoftness = FLESHBORDERSOFTNESS;
		info.m_ncBorderTint = FLESHBORDERTINT;
		info.m_nflGlobalOpacity = FLESHGLOBALOPACITY;
		info.m_nflGlossBrightness = FLESHGLOSSBRIGHTNESS;
		info.m_nflScrollSpeed = FLESHSCROLLSPEED;
	}

	// Cloak Pass
	void SetupVarsCloakBlendedPass(CloakBlendedPassVars_t &info)
	{
		info.m_nCloakFactor = CLOAKFACTOR;
		info.m_nCloakColorTint = CLOAKCOLORTINT;
		info.m_nRefractAmount = REFRACTAMOUNT;

		// Delete these lines if not bump mapping!
		info.m_nBumpmap = BUMPMAP;
		info.m_nBumpFrame = BUMPFRAME;
		info.m_nBumpTransform = BUMPTRANSFORM;
	}

	bool NeedsPowerOfTwoFrameBufferTexture(IMaterialVar **params, bool bCheckSpecificToThisFrame) const
	{
		if (params[CLOAKPASSENABLED]->GetIntValue()) // If material supports cloaking
		{
			if (bCheckSpecificToThisFrame == false) // For setting model flag at load time
				return true;
			else if ((params[CLOAKFACTOR]->GetFloatValue() > 0.0f) && (params[CLOAKFACTOR]->GetFloatValue() < 1.0f)) // Per-frame check
				return true;
			// else, not cloaking this frame, so check flag2 in case the base material still needs it
		}

		// Check flag2 if not drawing cloak pass
		return IS_FLAG2_SET(MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE);
	}

	bool IsTranslucent(IMaterialVar **params) const
	{
		if (params[CLOAKPASSENABLED]->GetIntValue()) // If material supports cloaking
		{
			if ((params[CLOAKFACTOR]->GetFloatValue() > 0.0f) && (params[CLOAKFACTOR]->GetFloatValue() < 1.0f)) // Per-frame check
				return true;
			// else, not cloaking this frame, so check flag in case the base material still needs it
		}

		// Check flag if not drawing cloak pass
		return IS_FLAG_SET(MATERIAL_VAR_TRANSLUCENT);
	}

	// Set up anything that is necessary to make decisions in SHADER_FALLBACK.
	SHADER_INIT_PARAMS()
	{
		SetupVars( s_info );
		InitParamsLightmappedGeneric_DX9( this, params, pMaterialName, s_info );

		// Emissive Scroll Pass
		if (!params[EMISSIVEBLENDENABLED]->IsDefined())
		{
			params[EMISSIVEBLENDENABLED]->SetIntValue(0);
		}
		else if (params[EMISSIVEBLENDENABLED]->GetIntValue())
		{
			EmissiveScrollBlendedPassVars_t e_info;
			SetupVarsEmissiveScrollBlendedPass(e_info);
			InitParamsEmissiveScrollBlendedPass(this, params, pMaterialName, e_info);
		}

		// Flesh Interior Pass
		if (!params[FLESHINTERIORENABLED]->IsDefined())
		{
			params[FLESHINTERIORENABLED]->SetIntValue(0);
		}
		else if (params[FLESHINTERIORENABLED]->GetIntValue())
		{
			FleshInteriorBlendedPassVars_t info;
			SetupVarsFleshInteriorBlendedPass(info);
			InitParamsFleshInteriorBlendedPass(this, params, pMaterialName, info);
		}

		// Cloak Pass
		if (!params[CLOAKPASSENABLED]->IsDefined())
		{
			params[CLOAKPASSENABLED]->SetIntValue(0);
		}
		else if (params[CLOAKPASSENABLED]->GetIntValue())
		{
			CloakBlendedPassVars_t info;
			SetupVarsCloakBlendedPass(info);
			InitParamsCloakBlendedPass(this, params, pMaterialName, info);
		}
	}

	SHADER_INIT
	{
		if (g_pHardwareConfig->SupportsBorderColor())
		{
			params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight_border");
		}
		else
		{
			params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight001");
		}

		SetupVars( s_info );
		InitLightmappedGeneric_DX9( this, params, s_info );

		// Emissive Scroll Pass
		if (params[EMISSIVEBLENDENABLED]->GetIntValue())
		{
			EmissiveScrollBlendedPassVars_t info;
			SetupVarsEmissiveScrollBlendedPass(info);
			InitEmissiveScrollBlendedPass(this, params, info);
		}

		// Flesh Interior Pass
		if (params[FLESHINTERIORENABLED]->GetIntValue())
		{
			FleshInteriorBlendedPassVars_t info;
			SetupVarsFleshInteriorBlendedPass(info);
			InitFleshInteriorBlendedPass(this, params, info);
		}

		// Cloak Pass
		if (params[CLOAKPASSENABLED]->GetIntValue())
		{
			CloakBlendedPassVars_t info;
			SetupVarsCloakBlendedPass(info);
			InitCloakBlendedPass(this, params, info);
		}
	}

	SHADER_DRAW
	{
		DrawLightmappedGeneric_DX9( this, params, pShaderAPI, pShaderShadow, s_info, pContextDataPtr );

		// Emissive Scroll Pass
		if (params[EMISSIVEBLENDENABLED]->GetIntValue())
		{
			// If ( snapshotting ) or ( we need to draw this frame )
			if ((pShaderShadow != NULL) || (params[EMISSIVEBLENDSTRENGTH]->GetFloatValue() > 0.0f))
			{
				EmissiveScrollBlendedPassVars_t info;
				SetupVarsEmissiveScrollBlendedPass(info);
				DrawEmissiveScrollBlendedPass(this, params, pShaderAPI, pShaderShadow, info, vertexCompression);
			}
			else // We're not snapshotting and we don't need to draw this frame
			{
				// Skip this pass!
				Draw(false);
			}
		}

		// Flesh Interior Pass
		if (params[FLESHINTERIORENABLED]->GetIntValue())
		{
			// If ( snapshotting ) or ( we need to draw this frame )
			if ((pShaderShadow != NULL) || (true))
			{
				FleshInteriorBlendedPassVars_t info;
				SetupVarsFleshInteriorBlendedPass(info);
				DrawFleshInteriorBlendedPass(this, params, pShaderAPI, pShaderShadow, info, vertexCompression);
			}
			else // We're not snapshotting and we don't need to draw this frame
			{
				// Skip this pass!
				Draw(false);
			}
		}

		// Skip the standard rendering if cloak pass is fully opaque
		bool bDrawStandardPass = true;
		if (params[CLOAKPASSENABLED]->GetIntValue() && (pShaderShadow == NULL)) // && not snapshotting
		{
			CloakBlendedPassVars_t info;
			SetupVarsCloakBlendedPass(info);
			if (CloakBlendedPassIsFullyOpaque(params, info))
			{
				bDrawStandardPass = false;
			}
		}
		
		// Standard rendering pass
	//	if (bDrawStandardPass)
	//	{
	//		Eye_Refract_Vars_t info;
	//		SetupVarsEyeRefract(info);
	//		Draw_Eyes_Refract(this, params, pShaderAPI, pShaderShadow, info);
	//	}
	//	else
	//	{
	//		// Skip this pass!
	//		Draw(false);
	//	}

		// Cloak Pass
		if (params[CLOAKPASSENABLED]->GetIntValue())
		{
			// If ( snapshotting ) or ( we need to draw this frame )
			if ((pShaderShadow != NULL) || ((params[CLOAKFACTOR]->GetFloatValue() > 0.0f) && (params[CLOAKFACTOR]->GetFloatValue() < 1.0f)))
			{
				CloakBlendedPassVars_t info;
				SetupVarsCloakBlendedPass(info);
				DrawCloakBlendedPass(this, params, pShaderAPI, pShaderShadow, info, vertexCompression);
			}
			else // We're not snapshotting and we don't need to draw this frame
			{
				// Skip this pass!
				Draw(false);
			}
		}
	}
#else
	SHADER_FALLBACK
	{
		if ( g_pHardwareConfig->GetDXSupportLevel() < 90 )
		return "LightmappedGeneric_DX8";

	return 0;
	}

	// Set up anything that is necessary to make decisions in SHADER_FALLBACK.
	SHADER_INIT_PARAMS()
	{
		SetupVars(s_info);
		InitParamsLightmappedGeneric_DX9(this, params, pMaterialName, s_info);
	}

	SHADER_INIT
	{
		SetupVars(s_info);
	InitLightmappedGeneric_DX9(this, params, s_info);
	}

	SHADER_DRAW
	{
		DrawLightmappedGeneric_DX9(this, params, pShaderAPI, pShaderShadow, s_info, pContextDataPtr);
	}
#endif // DARKINTERVAL
END_SHADER
