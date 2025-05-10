//========================================================================//
//
// Purpose: 
// DI changes - added emissive blend, flesh interior and aftershock passes
// $NoKeywords: $
//=============================================================================//

#include "BaseVSShader.h"
#include "convar.h"
#include "refract_dx9_helper.h"
#ifdef DARKINTERVAL
#include "emissive_scroll_blended_pass_helper.h"
#include "flesh_interior_blended_pass_helper.h"
#include "aftershock_helper.h"

//DEFINE_FALLBACK_SHADER( DI_Refract_DX90, Refract_DX90 )

BEGIN_VS_SHADER(DI_Refract_DX90, "Help for Refract" )
#else
DEFINE_FALLBACK_SHADER(Refract, Refract_DX90)

BEGIN_VS_SHADER(Refract_DX90, "Help for Refract")
#endif
	BEGIN_SHADER_PARAMS
		SHADER_PARAM_OVERRIDE( COLOR, SHADER_PARAM_TYPE_COLOR, "{255 255 255}", "unused", SHADER_PARAM_NOT_EDITABLE )
		SHADER_PARAM_OVERRIDE( ALPHA, SHADER_PARAM_TYPE_FLOAT, "1.0", "unused", SHADER_PARAM_NOT_EDITABLE )
		SHADER_PARAM( REFRACTAMOUNT, SHADER_PARAM_TYPE_FLOAT, "2", "" )
		SHADER_PARAM( REFRACTTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "refraction tint" )
		SHADER_PARAM( NORMALMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "normal map" )
		SHADER_PARAM( NORMALMAP2, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "normal map" )
		SHADER_PARAM( BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $normalmap" )
		SHADER_PARAM( BUMPFRAME2, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $normalmap" )
		SHADER_PARAM( BUMPTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$normalmap texcoord transform" )
		SHADER_PARAM( BUMPTRANSFORM2, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$normalmap texcoord transform" )
		SHADER_PARAM( TIME, SHADER_PARAM_TYPE_FLOAT, "0.0f", "" )
		SHADER_PARAM( BLURAMOUNT, SHADER_PARAM_TYPE_INTEGER, "1", "0, 1, or 2 for how much blur you want" )
		SHADER_PARAM( FADEOUTONSILHOUETTE, SHADER_PARAM_TYPE_BOOL, "1", "0 for no fade out on silhouette, 1 for fade out on sillhouette" )
		SHADER_PARAM( ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "envmap" )
		SHADER_PARAM( ENVMAPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "envmap frame number" )
		SHADER_PARAM( ENVMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "envmap tint" )
		SHADER_PARAM( ENVMAPCONTRAST, SHADER_PARAM_TYPE_FLOAT, "0.0", "contrast 0 == normal 1 == color*color" )
		SHADER_PARAM( ENVMAPSATURATION, SHADER_PARAM_TYPE_FLOAT, "1.0", "saturation 0 == greyscale 1 == normal" )
		SHADER_PARAM( REFRACTTINTTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shield", "" )
		SHADER_PARAM( REFRACTTINTTEXTUREFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "" )
		SHADER_PARAM( FRESNELREFLECTION, SHADER_PARAM_TYPE_FLOAT, "1.0", "1.0 == mirror, 0.0 == water" )
		SHADER_PARAM( NOWRITEZ, SHADER_PARAM_TYPE_INTEGER, "0", "0 == write z, 1 = no write z" )
		SHADER_PARAM( MASKED, SHADER_PARAM_TYPE_BOOL, "0", "mask using dest alpha" )
		SHADER_PARAM( VERTEXCOLORMODULATE, SHADER_PARAM_TYPE_BOOL, "0","Use the vertex color to effect refract color. alpha will adjust refract amount" )
		SHADER_PARAM( FORCEALPHAWRITE, SHADER_PARAM_TYPE_BOOL, "0","Force the material to write alpha to the dest buffer" )
#ifdef DARKINTERVAL
	//	SHADER_PARAM(MIRRORABOUTVIEWPORTEDGES, SHADER_PARAM_TYPE_INTEGER, "0", "don't sample outside of the viewport")
		SHADER_PARAM(LOCALREFRACT, SHADER_PARAM_TYPE_BOOL, "0", "add simple parallax effect")
		SHADER_PARAM(LOCALREFRACTDEPTH, SHADER_PARAM_TYPE_FLOAT, "0", "parallax depth")

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
		// Aftershock
		SHADER_PARAM(SILHOUETTEENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enable Aftershock pass")
		SHADER_PARAM(COLORTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Color tint")
//		SHADER_PARAM(REFRACTAMOUNT, SHADER_PARAM_TYPE_FLOAT, "2", "")

//		SHADER_PARAM(NORMALMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "normal map")
//		SHADER_PARAM(BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap")
//		SHADER_PARAM(BUMPTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform")

		SHADER_PARAM(SILHOUETTETHICKNESS, SHADER_PARAM_TYPE_FLOAT, "1", "")
		SHADER_PARAM(SILHOUETTECOLOR, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Silhouette color tint")
		SHADER_PARAM(GROUNDMIN, SHADER_PARAM_TYPE_FLOAT, "1", "")
		SHADER_PARAM(GROUNDMAX, SHADER_PARAM_TYPE_FLOAT, "1", "")
		SHADER_PARAM(BLURAMOUNTAFTERSHOCK, SHADER_PARAM_TYPE_FLOAT, "1", "")
//		SHADER_PARAM(TIME, SHADER_PARAM_TYPE_FLOAT, "0.0", "Needs CurrentTime Proxy")
		// Chromatic Aberraion
//		SHADER_PARAM(CHROMATICOFFSET, SHADER_PARAM_TYPE_FLOAT, "0.0", "Chromatic Offest Multiplier")
#endif // DARKINTERVAL
	END_SHADER_PARAMS
// FIXME: doesn't support Fresnel!

	void SetupVars( Refract_DX9_Vars_t& info )
	{
		info.m_nBaseTexture = BASETEXTURE;
		info.m_nFrame = FRAME;
		info.m_nRefractAmount = REFRACTAMOUNT;
		info.m_nRefractTint = REFRACTTINT;
		info.m_nNormalMap = NORMALMAP;
		info.m_nNormalMap2 = NORMALMAP2;
		info.m_nBumpFrame = BUMPFRAME;
		info.m_nBumpFrame2 = BUMPFRAME2;
		info.m_nBumpTransform = BUMPTRANSFORM;
		info.m_nBumpTransform2 = BUMPTRANSFORM2;
		info.m_nBlurAmount = BLURAMOUNT;
		info.m_nFadeOutOnSilhouette = FADEOUTONSILHOUETTE;
		info.m_nEnvmap = ENVMAP;
		info.m_nEnvmapFrame = ENVMAPFRAME;
		info.m_nEnvmapTint = ENVMAPTINT;
		info.m_nEnvmapContrast = ENVMAPCONTRAST;
		info.m_nEnvmapSaturation = ENVMAPSATURATION;
		info.m_nRefractTintTexture = REFRACTTINTTEXTURE;
		info.m_nRefractTintTextureFrame = REFRACTTINTTEXTUREFRAME;
		info.m_nFresnelReflection = FRESNELREFLECTION;
		info.m_nNoWriteZ = NOWRITEZ;
		info.m_nMasked = MASKED;
		info.m_nVertexColorModulate = VERTEXCOLORMODULATE;
		info.m_nForceAlphaWrite = FORCEALPHAWRITE;
#ifdef DARKINTERVAL
	//	info.m_nChromOffset = CHROMATICOFFSET;
	//	info.m_nMirrorAboutViewportEdges = MIRRORABOUTVIEWPORTEDGES;
#endif
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

	// Aftershock Pass
	void SetupVarsAftershockPass(AftershockVars_t &info)
	{
		info.m_nColorTint = COLORTINT;
		info.m_nRefractAmount = REFRACTAMOUNT;

		info.m_nBumpmap = NORMALMAP;
		info.m_nBumpFrame = BUMPFRAME;
		info.m_nBumpTransform = BUMPTRANSFORM;

		info.m_nSilhouetteThickness = SILHOUETTETHICKNESS;
		info.m_nSilhouetteColor = SILHOUETTECOLOR;
		info.m_nGroundMin = GROUNDMIN;
		info.m_nGroundMax = GROUNDMAX;
		info.m_nBlurAmount = BLURAMOUNTAFTERSHOCK;
		info.m_nTime = TIME;
	}
#endif // DARKINTERVAL
	SHADER_INIT_PARAMS()
	{
		Refract_DX9_Vars_t info;
		SetupVars( info );
		InitParamsRefract_DX9( this, params, pMaterialName, info );
#ifdef DARKINTERVAL
		// Emissive Scroll Pass
		if (!params[EMISSIVEBLENDENABLED]->IsDefined())
		{
			params[EMISSIVEBLENDENABLED]->SetIntValue(0);
		}
		else if (params[EMISSIVEBLENDENABLED]->GetIntValue())
		{
			EmissiveScrollBlendedPassVars_t info;
			SetupVarsEmissiveScrollBlendedPass(info);
			InitParamsEmissiveScrollBlendedPass(this, params, pMaterialName, info);
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

		// Aftershock Pass
		if (!params[SILHOUETTEENABLED]->IsDefined())
		{
			params[SILHOUETTEENABLED]->SetIntValue(0);
		}
		else if (params[SILHOUETTEENABLED]->GetIntValue())
		{
			AftershockVars_t info;
			SetupVarsAftershockPass(info);
			InitParamsAftershock(this, params, pMaterialName, info);
		}
#endif // DARKINTERVAL
	}

	SHADER_FALLBACK
	{
#ifdef DARKINTERVAL
		if( g_pHardwareConfig->GetDXSupportLevel() < 90 )
#else
		if ( g_pHardwareConfig->GetDXSupportLevel() < 82 )
#endif
			return "Refract_DX80";
		return 0;
	}

	SHADER_INIT
	{
		Refract_DX9_Vars_t info;
		SetupVars( info );
		InitRefract_DX9( this, params, info );
#ifdef DARKINTERVAL
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

		// Aftershock Pass
		if (params[SILHOUETTEENABLED]->GetIntValue())
		{
			AftershockVars_t info;
			SetupVarsAftershockPass(info);
			InitAftershock(this, params, info);
		}
#endif // DARKINTERVAL
	}

	SHADER_DRAW
	{
		Refract_DX9_Vars_t info;
		SetupVars( info );
		
		// If ( snapshotting ) or ( we need to draw this frame )
		bool bHasFlashlight = this->UsingFlashlight( params );
		if ( ( pShaderShadow != NULL ) || ( bHasFlashlight == false ) )
		{
			DrawRefract_DX9( this, params, pShaderAPI, pShaderShadow, info, vertexCompression );
		}
		else
		{
			Draw( false );
		}
#ifdef DARKINTERVAL
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

		// Aftershock Pass
		if (params[SILHOUETTEENABLED]->GetIntValue())
		{
			// If ( snapshotting ) or ( we need to draw this frame )
			if ((pShaderShadow != NULL) || (true))
			{
				AftershockVars_t info;
				SetupVarsAftershockPass(info);
				DrawAftershock(this, params, pShaderAPI, pShaderShadow, info, vertexCompression);
			}
			else // We're not snapshotting and we don't need to draw this frame
			{
				// Skip this pass!
				Draw(false);
			}
		}
#endif // DARKINTERVAL
	}
END_SHADER
