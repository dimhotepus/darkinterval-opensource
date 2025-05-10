//========================================================================//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//=====================================================================================//

#include "BaseVSShader.h"
#include "vertexlitgeneric_dx9_helper.h"
#include "emissive_scroll_blended_pass_helper.h"
#include "cloak_blended_pass_helper.h"
#include "flesh_interior_blended_pass_helper.h"
#include "subsurface_helper.h"

extern ConVar r_subsurfaceinterior;

BEGIN_VS_SHADER( DI_VertexLitGeneric_DX90, "Help for VertexLitGeneric" )
	BEGIN_SHADER_PARAMS
		SHADER_PARAM( ALBEDO, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "albedo (Base texture with no baked lighting)" )
		SHADER_PARAM( SELFILLUMTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint" )
		SHADER_PARAM( DETAIL, SHADER_PARAM_TYPE_TEXTURE, "shadertest/detail", "detail texture" )
		SHADER_PARAM( DETAILFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $detail" )
		SHADER_PARAM( DETAILSCALE, SHADER_PARAM_TYPE_FLOAT, "4", "scale of the detail texture" )
		SHADER_PARAM( ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "envmap" )
		SHADER_PARAM( ENVMAPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "envmap frame number" )
		SHADER_PARAM( ENVMAPMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_envmask", "envmap mask" )
		SHADER_PARAM( ENVMAPMASKFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "" )
		SHADER_PARAM( ENVMAPMASKTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$envmapmask texcoord transform" )
		SHADER_PARAM( ENVMAPTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "envmap tint" )
		SHADER_PARAM( BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "bump map" )
		SHADER_PARAM( BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap" )
		SHADER_PARAM( BUMPTRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform" )
		SHADER_PARAM( ENVMAPCONTRAST, SHADER_PARAM_TYPE_FLOAT, "0.0", "contrast 0 == normal 1 == color*color" )
		SHADER_PARAM( ENVMAPSATURATION, SHADER_PARAM_TYPE_FLOAT, "1.0", "saturation 0 == greyscale 1 == normal" )
 	    SHADER_PARAM( SELFILLUM_ENVMAPMASK_ALPHA, SHADER_PARAM_TYPE_FLOAT,"0.0","defines that self illum value comes from env map mask alpha" )
		SHADER_PARAM( SELFILLUMFRESNEL, SHADER_PARAM_TYPE_BOOL, "0", "Self illum fresnel" )
		SHADER_PARAM( SELFILLUMFRESNELMINMAXEXP, SHADER_PARAM_TYPE_VEC4, "0", "Self illum fresnel min, max, exp" )
		SHADER_PARAM( ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0.0", "" )	
		SHADER_PARAM( FLASHLIGHTNOLAMBERT, SHADER_PARAM_TYPE_BOOL, "0", "Flashlight pass sets N.L=1.0" )

		// Debugging term for visualizing ambient data on its own
		SHADER_PARAM( AMBIENTONLY, SHADER_PARAM_TYPE_INTEGER, "0", "Control drawing of non-ambient light ()" )

		SHADER_PARAM( PHONGEXPONENT, SHADER_PARAM_TYPE_FLOAT, "5.0", "Phong exponent for local specular lights" )
		SHADER_PARAM( PHONGTINT, SHADER_PARAM_TYPE_VEC3, "5.0", "Phong tint for local specular lights" )
		SHADER_PARAM( PHONGALBEDOTINT, SHADER_PARAM_TYPE_BOOL, "1.0", "Apply tint by albedo (controlled by spec exponent texture" )
		SHADER_PARAM( LIGHTWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "1D ramp texture for tinting scalar diffuse term" )
		SHADER_PARAM( PHONGWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "warp the specular term" )
		SHADER_PARAM( PHONGFRESNELRANGES, SHADER_PARAM_TYPE_VEC3, "[0  0.5  1]", "Parameters for remapping fresnel output" )
		SHADER_PARAM( PHONGBOOST, SHADER_PARAM_TYPE_FLOAT, "1.0", "Phong overbrightening factor (specular mask channel should be authored to account for this)" )
		SHADER_PARAM( PHONGEXPONENTTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "Phong Exponent map" )
		SHADER_PARAM( PHONG, SHADER_PARAM_TYPE_BOOL, "0", "enables phong lighting" )
		SHADER_PARAM( BASEMAPALPHAPHONGMASK, SHADER_PARAM_TYPE_INTEGER, "0", "indicates that there is no normal map and that the phong mask is in base alpha" )
		SHADER_PARAM( INVERTPHONGMASK, SHADER_PARAM_TYPE_INTEGER, "0", "invert the phong mask (0=full phong, 1=no phong)" )
		SHADER_PARAM( ENVMAPFRESNEL, SHADER_PARAM_TYPE_FLOAT, "0", "Degree to which Fresnel should be applied to env map" )
		SHADER_PARAM( SELFILLUMMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "If we bind a texture here, it overrides base alpha (if any) for self illum" )

	    // detail (multi-) texturing
	    SHADER_PARAM( DETAILBLENDMODE, SHADER_PARAM_TYPE_INTEGER, "0", "mode for combining detail texture with base. 0=normal, 1= additive, 2=alpha blend detail over base, 3=crossfade" )
		SHADER_PARAM( DETAILBLENDFACTOR, SHADER_PARAM_TYPE_FLOAT, "1", "blend amount for detail texture." )
		SHADER_PARAM( DETAILTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "detail texture tint" )
		SHADER_PARAM( DETAILTEXTURETRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$detail texcoord transform" )

		// Rim lighting terms
		SHADER_PARAM( RIMLIGHT, SHADER_PARAM_TYPE_BOOL, "0", "enables rim lighting" )
		SHADER_PARAM( RIMLIGHTEXPONENT, SHADER_PARAM_TYPE_FLOAT, "4.0", "Exponent for rim lights" )
		SHADER_PARAM( RIMLIGHTBOOST, SHADER_PARAM_TYPE_FLOAT, "1.0", "Boost for rim lights" )
		SHADER_PARAM( RIMMASK, SHADER_PARAM_TYPE_BOOL, "0", "Indicates whether or not to use alpha channel of exponent texture to mask the rim term" )

	    // Seamless mapping scale
		SHADER_PARAM( SEAMLESS_BASE, SHADER_PARAM_TYPE_BOOL, "0", "whether to apply seamless mapping to the base texture. requires a smooth model." )
		SHADER_PARAM( SEAMLESS_DETAIL, SHADER_PARAM_TYPE_BOOL, "0", "where to apply seamless mapping to the detail texture." )
	    SHADER_PARAM( SEAMLESS_SCALE, SHADER_PARAM_TYPE_FLOAT, "1.0", "the scale for the seamless mapping. # of repetions of texture per inch." )

		// Emissive Scroll Pass
		SHADER_PARAM( EMISSIVEBLENDENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enable emissive blend pass" )
		SHADER_PARAM( EMISSIVEBLENDBASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "self-illumination map" )
		SHADER_PARAM( EMISSIVEBLENDSCROLLVECTOR, SHADER_PARAM_TYPE_VEC2, "[0.11 0.124]", "Emissive scroll vec" )
		SHADER_PARAM( EMISSIVEBLENDSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "1.0", "Emissive blend strength" )
		SHADER_PARAM( EMISSIVEBLENDTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "self-illumination map" )
		SHADER_PARAM( EMISSIVEBLENDTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Self-illumination tint" )
		SHADER_PARAM( EMISSIVEBLENDFLOWTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "flow map" )
		SHADER_PARAM( TIME, SHADER_PARAM_TYPE_FLOAT, "0.0", "Needs CurrentTime Proxy" )

		// Cloak Pass
		SHADER_PARAM( CLOAKPASSENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enables cloak render in a second pass" )
		SHADER_PARAM( CLOAKFACTOR, SHADER_PARAM_TYPE_FLOAT, "0.0", "" )
		SHADER_PARAM( CLOAKCOLORTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Cloak color tint" )
		SHADER_PARAM( REFRACTAMOUNT, SHADER_PARAM_TYPE_FLOAT, "2", "" )

		// Flesh Interior Pass
		SHADER_PARAM( FLESHINTERIORENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enable Flesh interior blend pass" )
		SHADER_PARAM( FLESHINTERIORTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh color texture" )
		SHADER_PARAM( FLESHINTERIORNOISETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh noise texture" )
		SHADER_PARAM( FLESHBORDERTEXTURE1D, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh border 1D texture" )
		SHADER_PARAM( FLESHNORMALTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh normal texture" )
		SHADER_PARAM( FLESHSUBSURFACETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh subsurface texture" )
		SHADER_PARAM( FLESHCUBETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Flesh cubemap texture" )
		SHADER_PARAM( FLESHBORDERNOISESCALE, SHADER_PARAM_TYPE_FLOAT, "1.5", "Flesh Noise UV scalar for border" )
		SHADER_PARAM( FLESHDEBUGFORCEFLESHON, SHADER_PARAM_TYPE_BOOL, "0", "Flesh Debug full flesh" )
		SHADER_PARAM( FLESHEFFECTCENTERRADIUS1, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius" )
		SHADER_PARAM( FLESHEFFECTCENTERRADIUS2, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius" )
		SHADER_PARAM( FLESHEFFECTCENTERRADIUS3, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius" )
		SHADER_PARAM( FLESHEFFECTCENTERRADIUS4, SHADER_PARAM_TYPE_VEC4, "[0 0 0 0.001]", "Flesh effect center and radius" )
		SHADER_PARAM( FLESHSUBSURFACETINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Subsurface Color" )
		SHADER_PARAM( FLESHBORDERWIDTH, SHADER_PARAM_TYPE_FLOAT, "0.3", "Flesh border" )
		SHADER_PARAM( FLESHBORDERSOFTNESS, SHADER_PARAM_TYPE_FLOAT, "0.42", "Flesh border softness (> 0.0 && <= 0.5)" )
		SHADER_PARAM( FLESHBORDERTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Flesh border Color" )
		SHADER_PARAM( FLESHGLOBALOPACITY, SHADER_PARAM_TYPE_FLOAT, "1.0", "Flesh global opacity" )
		SHADER_PARAM( FLESHGLOSSBRIGHTNESS, SHADER_PARAM_TYPE_FLOAT, "0.66", "Flesh gloss brightness" )
		SHADER_PARAM( FLESHSCROLLSPEED, SHADER_PARAM_TYPE_FLOAT, "1.0", "Flesh scroll speed" )

		SHADER_PARAM( SEPARATEDETAILUVS, SHADER_PARAM_TYPE_BOOL, "0", "Use texcoord1 for detail texture" )
		SHADER_PARAM( LINEARWRITE, SHADER_PARAM_TYPE_INTEGER, "0", "Disables SRGB conversion of shader results." )
		SHADER_PARAM( DEPTHBLEND, SHADER_PARAM_TYPE_INTEGER, "0", "fade at intersection boundaries. Only supported without bumpmaps" )
		SHADER_PARAM( DEPTHBLENDSCALE, SHADER_PARAM_TYPE_FLOAT, "50.0", "Amplify or reduce DEPTHBLEND fading. Lower values make harder edges." )

		SHADER_PARAM( BLENDTINTBYBASEALPHA, SHADER_PARAM_TYPE_BOOL, "0", "Use the base alpha to blend in the $color modulation")
		SHADER_PARAM( BLENDTINTCOLOROVERBASE, SHADER_PARAM_TYPE_FLOAT, "0", "blend between tint acting as a multiplication versus a replace" )
	
		// Subsurface Interior pass
	//	SHADER_PARAM( NORMALMAP, SHADER_PARAM_TYPE_TEXTURE, "models/shadertest/shader1_normal", "normal map")
		SHADER_PARAM( SUBSURFACEENABLED, SHADER_PARAM_TYPE_BOOL, "0", "Enable emissive blend pass")
		SHADER_PARAM( TRANSMATMASKSTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "Masks for material changes: fresnel hue-shift, jellyfish, forward scatter and back scatter");
		SHADER_PARAM( EFFECTMASKSTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "Masks for material effects: self-illum, color warping, iridescence and clearcoat")
		SHADER_PARAM( COLORWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "3D rgb-lookup table texture for tinting color map")
		SHADER_PARAM( FRESNELCOLORWARPTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "3D rgb-lookup table texture for tinting fresnel-based hue shift color")
		SHADER_PARAM( OPACITYTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "shadertest/BaseTexture", "Texture to control surface opacity when INTERIOR=1")
		SHADER_PARAM( IRIDESCENTWARP, SHADER_PARAM_TYPE_TEXTURE, "shader/BaseTexture", "1D lookup texture for iridescent effect")
	//	SHADER_PARAM( DETAIL, SHADER_PARAM_TYPE_TEXTURE, "shader/BaseTexture", "detail map")

		SHADER_PARAM( UVSCALE, SHADER_PARAM_TYPE_FLOAT, "1", "uv scale")
	//	SHADER_PARAM( DETAILBLENDMODE, SHADER_PARAM_TYPE_INTEGER, "0", "detail blend mode: 1=mod2x, 2=add, 3=alpha blend (detailalpha), 4=crossfade, 5=additive self-illum, 6=multiply")
	//	SHADER_PARAM( DETAILBLENDFACTOR, SHADER_PARAM_TYPE_FLOAT, "1.0", "strength of detail map")
	//	SHADER_PARAM( DETAILSCALE, SHADER_PARAM_TYPE_FLOAT, "4.0", "detail map scale based on original UV set")
	//	SHADER_PARAM( DETAILFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $detail")
	//	SHADER_PARAM( DETAILTEXTURETRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$detail texcoord transform")
		SHADER_PARAM( BUMPSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "1.0", "bump map strength")
		SHADER_PARAM( FRESNELBUMPSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "1.0", "bump map strength for fresnel")
		SHADER_PARAM( TRANSLUCENTFRESNELMINMAXEXP, SHADER_PARAM_TYPE_VEC3, "[0.8 1.0 1.0]", "translucency fresnel params")

		SHADER_PARAM( INTERIOR, SHADER_PARAM_TYPE_INTEGER, "0", "enable surface translucency (refractive/foggy interior)")
		SHADER_PARAM( INTERIORFOGSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "0.06", "fog strength")
		SHADER_PARAM( INTERIORBACKGROUNDBOOST, SHADER_PARAM_TYPE_FLOAT, "7", "boosts the brightness of bright background pixels")
		SHADER_PARAM( INTERIORAMBIENTSCALE, SHADER_PARAM_TYPE_FLOAT, "0.3", "scales ambient light in the interior volume");
		SHADER_PARAM( INTERIORBACKLIGHTSCALE, SHADER_PARAM_TYPE_FLOAT, "0.3", "scales backlighting in the interior volume");
		SHADER_PARAM( INTERIORCOLOR, SHADER_PARAM_TYPE_VEC3, "[0.7 0.5 0.45]", "tints light in the interior volume")
		SHADER_PARAM( INTERIORREFRACTSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "0.015", "strength of bumped refract of the background seen through the interior")
		SHADER_PARAM( INTERIORREFRACTBLUR, SHADER_PARAM_TYPE_FLOAT, "0.2", "strength of blur applied to the background seen through the interior")

		SHADER_PARAM( DIFFUSESOFTNORMAL, SHADER_PARAM_TYPE_FLOAT, "0.0f", "diffuse lighting uses softened normal")
		SHADER_PARAM( DIFFUSEEXPONENT, SHADER_PARAM_TYPE_FLOAT, "1.0f", "diffuse lighting exponent")
	//	SHADER_PARAM( PHONGEXPONENT, SHADER_PARAM_TYPE_FLOAT, "1.0", "specular exponent")
	//	SHADER_PARAM( PHONGSCALE, SHADER_PARAM_TYPE_FLOAT, "1.0", "specular lighting scale")
	//	SHADER_PARAM( PHONGEXPONENT2, SHADER_PARAM_TYPE_FLOAT, "1.0", "specular exponent 2")
	//	SHADER_PARAM( PHONGFRESNEL, SHADER_PARAM_TYPE_VEC3, "[0 0.5 1]", "phong fresnel terms view-forward, 45 degrees, edge")
	//	SHADER_PARAM( PHONGBOOST2, SHADER_PARAM_TYPE_FLOAT, "1.0", "specular lighting 2 scale")
	//	SHADER_PARAM( PHONGFRESNELRANGES2, SHADER_PARAM_TYPE_VEC3, "[1 1 1]", "phong fresnel terms view-forward, 45 degrees, edge")
	//	SHADER_PARAM( PHONG2SOFTNESS, SHADER_PARAM_TYPE_FLOAT, "1.0", "clearcoat uses softened normal")
	//	SHADER_PARAM( RIMLIGHTEXPONENT, SHADER_PARAM_TYPE_FLOAT, "1.0", "rim lighting exponent")
	//	SHADER_PARAM( RIMLIGHTSCALE, SHADER_PARAM_TYPE_FLOAT, "1.0", "rim lighting scale")
	//	SHADER_PARAM( PHONGCOLORTINT, SHADER_PARAM_TYPE_VEC3, "[1.0 1.0 1.0]", "specular texture tint")
		SHADER_PARAM( AMBIENTBOOST, SHADER_PARAM_TYPE_FLOAT, "0.0", "ambient boost amount")
		SHADER_PARAM( AMBIENTBOOSTMASKMODE, SHADER_PARAM_TYPE_INTEGER, "0", "masking mode for ambient scale: 0 = allover, 1 or 2 = mask by transmatmask r or g, 3 to 5 effectmask g b or a")

		SHADER_PARAM( BACKSCATTER, SHADER_PARAM_TYPE_FLOAT, "0.0", "subsurface back-scatter intensity")
		SHADER_PARAM( FORWARDSCATTER, SHADER_PARAM_TYPE_FLOAT, "0.0", "subsurface forward-scatter intensity")
		SHADER_PARAM( SSDEPTH, SHADER_PARAM_TYPE_FLOAT, "0.1", "subsurface depth over which effect is not visible")
		SHADER_PARAM( SSBENTNORMALINTENSITY, SHADER_PARAM_TYPE_FLOAT, "0.2", "subsurface bent normal intensity: higher is more view-dependent")
		SHADER_PARAM( SSCOLORTINT, SHADER_PARAM_TYPE_VEC3, "[0.2, 0.05, 0.0]", "color tint for subsurface effects")
		SHADER_PARAM( SSTINTBYALBEDO, SHADER_PARAM_TYPE_FLOAT, "0.0", "blend ss color tint to albedo color based on this factor")
		SHADER_PARAM( NORMAL2SOFTNESS, SHADER_PARAM_TYPE_FLOAT, "0.0", "mip level for effects with soft normal (clearcoat and subsurface)")
		SHADER_PARAM( HUESHIFTINTENSITY, SHADER_PARAM_TYPE_FLOAT, "0.5", "scales effect of fresnel-based hue-shift")
		SHADER_PARAM( HUESHIFTFRESNELEXPONENT, SHADER_PARAM_TYPE_FLOAT, "2.0f", "fresnel exponent for hue-shift (edge-based)")
		SHADER_PARAM( IRIDESCENCEBOOST, SHADER_PARAM_TYPE_FLOAT, "1.0f", "boost iridescence effect")
		SHADER_PARAM( IRIDESCENCEEXPONENT, SHADER_PARAM_TYPE_FLOAT, "2.0f", "fresnel exponent for iridescence effect (center-based)")
	//	SHADER_PARAM( SELFILLUMTINT, SHADER_PARAM_TYPE_VEC3, "[0.7 0.5 0.45]", "self-illum color tint")

		SHADER_PARAM( UVPROJOFFSET, SHADER_PARAM_TYPE_VEC3, "[0 0 0]", "Center for UV projection")
		SHADER_PARAM( BBMIN, SHADER_PARAM_TYPE_VEC3, "[0 0 0]", "UV projection bounding box min")
		SHADER_PARAM( BBMAX, SHADER_PARAM_TYPE_VEC3, "[0 0 0]", "UV projection bounding box max")
			
		SHADER_PARAM(TREESWAY, SHADER_PARAM_TYPE_INTEGER, "1", "");
		SHADER_PARAM(TREESWAYHEIGHT, SHADER_PARAM_TYPE_FLOAT, "1000", "");
		SHADER_PARAM(TREESWAYSTARTHEIGHT, SHADER_PARAM_TYPE_FLOAT, "0.2", "");
		SHADER_PARAM(TREESWAYRADIUS, SHADER_PARAM_TYPE_FLOAT, "300", "");
		SHADER_PARAM(TREESWAYSTARTRADIUS, SHADER_PARAM_TYPE_FLOAT, "0.1", "");
		SHADER_PARAM(TREESWAYSPEED, SHADER_PARAM_TYPE_FLOAT, "1", "");
		SHADER_PARAM(TREESWAYSPEEDHIGHWINDMULTIPLIER, SHADER_PARAM_TYPE_FLOAT, "2", "");
		SHADER_PARAM(TREESWAYSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "10", "");
		SHADER_PARAM(TREESWAYSCRUMBLESPEED, SHADER_PARAM_TYPE_FLOAT, "0.1", "");
		SHADER_PARAM(TREESWAYSCRUMBLESTRENGTH, SHADER_PARAM_TYPE_FLOAT, "0.1", "");
		SHADER_PARAM(TREESWAYSCRUMBLEFREQUENCY, SHADER_PARAM_TYPE_FLOAT, "0.1", "");
		SHADER_PARAM(TREESWAYFALLOFFEXP, SHADER_PARAM_TYPE_FLOAT, "1.5", "");
		SHADER_PARAM(TREESWAYSCRUMBLEFALLOFFEXP, SHADER_PARAM_TYPE_FLOAT, "1.0", "");
		SHADER_PARAM(TREESWAYSPEEDLERPSTART, SHADER_PARAM_TYPE_FLOAT, "3", "");
		SHADER_PARAM(TREESWAYSPEEDLERPEND, SHADER_PARAM_TYPE_FLOAT, "6", "");
	END_SHADER_PARAMS

	void SetupVars( VertexLitGeneric_DX9_Vars_t& info )
	{
		info.m_nBaseTexture = BASETEXTURE;
		info.m_nBaseTextureFrame = FRAME;
		info.m_nBaseTextureTransform = BASETEXTURETRANSFORM;
		info.m_nAlbedo = ALBEDO;
		info.m_nSelfIllumTint = SELFILLUMTINT;
		info.m_nDetail = DETAIL;
		info.m_nDetailFrame = DETAILFRAME;
		info.m_nDetailScale = DETAILSCALE;
		info.m_nEnvmap = ENVMAP;
		info.m_nEnvmapFrame = ENVMAPFRAME;
		info.m_nEnvmapMask = ENVMAPMASK;
		info.m_nEnvmapMaskFrame = ENVMAPMASKFRAME;
		info.m_nEnvmapMaskTransform = ENVMAPMASKTRANSFORM;
		info.m_nEnvmapTint = ENVMAPTINT;
		info.m_nBumpmap = BUMPMAP;
		info.m_nBumpFrame = BUMPFRAME;
		info.m_nBumpTransform = BUMPTRANSFORM;
		info.m_nEnvmapContrast = ENVMAPCONTRAST;
		info.m_nEnvmapSaturation = ENVMAPSATURATION;
		info.m_nAlphaTestReference = ALPHATESTREFERENCE;
		info.m_nFlashlightNoLambert = FLASHLIGHTNOLAMBERT;

		info.m_nFlashlightTexture = FLASHLIGHTTEXTURE;
		info.m_nFlashlightTextureFrame = FLASHLIGHTTEXTUREFRAME;
		info.m_nSelfIllumEnvMapMask_Alpha = SELFILLUM_ENVMAPMASK_ALPHA;
		info.m_nSelfIllumFresnel = SELFILLUMFRESNEL;
		info.m_nSelfIllumFresnelMinMaxExp = SELFILLUMFRESNELMINMAXEXP;

		info.m_nAmbientOnly = AMBIENTONLY;
		info.m_nPhongExponent = PHONGEXPONENT;
		info.m_nPhongExponentTexture = PHONGEXPONENTTEXTURE;
		info.m_nPhongTint = PHONGTINT;
		info.m_nPhongAlbedoTint = PHONGALBEDOTINT;
		info.m_nDiffuseWarpTexture = LIGHTWARPTEXTURE;
		info.m_nPhongWarpTexture = PHONGWARPTEXTURE;
		info.m_nPhongBoost = PHONGBOOST;
		info.m_nPhongFresnelRanges = PHONGFRESNELRANGES;
		info.m_nPhong = PHONG;
		info.m_nBaseMapAlphaPhongMask = BASEMAPALPHAPHONGMASK;
		info.m_nEnvmapFresnel = ENVMAPFRESNEL;
		info.m_nDetailTextureCombineMode = DETAILBLENDMODE;
		info.m_nDetailTextureBlendFactor = DETAILBLENDFACTOR;
		info.m_nDetailTextureTransform = DETAILTEXTURETRANSFORM;

		// Rim lighting parameters
		info.m_nRimLight = RIMLIGHT;
		info.m_nRimLightPower = RIMLIGHTEXPONENT;
		info.m_nRimLightBoost = RIMLIGHTBOOST;
		info.m_nRimMask = RIMMASK;

		// seamless
		info.m_nSeamlessScale = SEAMLESS_SCALE;
		info.m_nSeamlessDetail = SEAMLESS_DETAIL;
		info.m_nSeamlessBase = SEAMLESS_BASE;

		info.m_nSeparateDetailUVs = SEPARATEDETAILUVS;

		info.m_nLinearWrite = LINEARWRITE;
		info.m_nDetailTint = DETAILTINT;
		info.m_nInvertPhongMask = INVERTPHONGMASK;

		info.m_nDepthBlend = DEPTHBLEND;
		info.m_nDepthBlendScale = DEPTHBLENDSCALE;

		info.m_nSelfIllumMask = SELFILLUMMASK;
		info.m_nBlendTintByBaseAlpha = BLENDTINTBYBASEALPHA;
		info.m_nTintReplacesBaseColor = BLENDTINTCOLOROVERBASE;
	}

	// Cloak Pass
	void SetupVarsCloakBlendedPass( CloakBlendedPassVars_t &info )
	{
		info.m_nCloakFactor = CLOAKFACTOR;
		info.m_nCloakColorTint = CLOAKCOLORTINT;
		info.m_nRefractAmount = REFRACTAMOUNT;

		// Delete these lines if not bump mapping!
		info.m_nBumpmap = BUMPMAP;
		info.m_nBumpFrame = BUMPFRAME;
		info.m_nBumpTransform = BUMPTRANSFORM;
	}

	bool NeedsPowerOfTwoFrameBufferTexture( IMaterialVar **params, bool bCheckSpecificToThisFrame ) const 
	{ 
		if ( params[CLOAKPASSENABLED]->GetIntValue() ) // If material supports cloaking
		{
			if ( bCheckSpecificToThisFrame == false ) // For setting model flag at load time
				return true;
			else if ( ( params[CLOAKFACTOR]->GetFloatValue() > 0.0f ) && ( params[CLOAKFACTOR]->GetFloatValue() < 1.0f ) ) // Per-frame check
				return true;
			// else, not cloaking this frame, so check flag2 in case the base material still needs it
		}
		/*
		if ( params[SHEENPASSENABLED]->GetIntValue() ) // If material supports weapon sheen
			return true;
		*/
		// Check flag2 if not drawing cloak pass
		return IS_FLAG2_SET( MATERIAL_VAR2_NEEDS_POWER_OF_TWO_FRAME_BUFFER_TEXTURE ); 
	}

	// Emissive Scroll Pass
	void SetupVarsEmissiveScrollBlendedPass( EmissiveScrollBlendedPassVars_t &info )
	{
		info.m_nBlendStrength = EMISSIVEBLENDSTRENGTH;
		info.m_nBaseTexture = EMISSIVEBLENDBASETEXTURE;
		info.m_nFlowTexture = EMISSIVEBLENDFLOWTEXTURE;
		info.m_nEmissiveTexture = EMISSIVEBLENDTEXTURE;
		info.m_nEmissiveTint = EMISSIVEBLENDTINT;
		info.m_nEmissiveScrollVector = EMISSIVEBLENDSCROLLVECTOR;
		info.m_nTime = TIME;
	}

	// Flesh Interior Pass
	void SetupVarsFleshInteriorBlendedPass( FleshInteriorBlendedPassVars_t &info )
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

		info.m_nTime = TIME;
	}

	// Subsurface Interior Pass
	void SetupVarsFlesh(FleshVars_t &info)
	{
		info.m_nBaseTexture = BASETEXTURE;
		info.m_nBaseTextureTransform = BASETEXTURETRANSFORM;
		info.m_nNormalMap = BUMPMAP;
		info.m_nDetailTexture = DETAIL;
		info.m_nColorWarpTexture = COLORWARPTEXTURE;
		info.m_nFresnelColorWarpTexture = FRESNELCOLORWARPTEXTURE;
		info.m_nTransMatMasksTexture = TRANSMATMASKSTEXTURE;
		info.m_nEffectMasksTexture = EFFECTMASKSTEXTURE;
		info.m_nIridescentWarpTexture = IRIDESCENTWARP;
		info.m_nOpacityTexture = OPACITYTEXTURE;
		info.m_nBumpStrength = BUMPSTRENGTH;
		info.m_nFresnelBumpStrength = FRESNELBUMPSTRENGTH;
		info.m_nUVScale = UVSCALE;
		info.m_nDetailScale = DETAILSCALE;
		info.m_nDetailFrame = DETAILFRAME;
		info.m_nDetailBlendMode = DETAILBLENDMODE;
		info.m_nDetailBlendFactor = DETAILBLENDFACTOR;
		info.m_nDetailTextureTransform = DETAILTEXTURETRANSFORM;

		info.m_nInteriorEnable = INTERIOR;
		info.m_nInteriorFogStrength = INTERIORFOGSTRENGTH;
		info.m_nInteriorBackgroundBoost = INTERIORBACKGROUNDBOOST;
		info.m_nInteriorAmbientScale = INTERIORAMBIENTSCALE;
		info.m_nInteriorBackLightScale = INTERIORBACKLIGHTSCALE;
		info.m_nInteriorColor = INTERIORCOLOR;
		info.m_nInteriorRefractStrength = INTERIORREFRACTSTRENGTH;
		info.m_nInteriorRefractBlur = INTERIORREFRACTBLUR;

		info.m_nFresnelParams = TRANSLUCENTFRESNELMINMAXEXP;
		info.m_nDiffuseSoftNormal = DIFFUSESOFTNORMAL;
		info.m_nDiffuseExponent = DIFFUSEEXPONENT;
		info.m_nSpecExp = PHONGEXPONENT;
		info.m_nSpecScale = PHONGBOOST;
		info.m_nSpecFresnel = PHONGFRESNELRANGES;
//		info.m_nSpecExp2 = PHONGEXPONENT2;
//		info.m_nSpecScale2 = PHONGBOOST2;
//		info.m_nSpecFresnel2 = PHONGFRESNELRANGES2;
//		info.m_nPhong2Softness = PHONG2SOFTNESS;
//		info.m_nRimLightExp = RIMLIGHTEXPONENT;
//		info.m_nRimLightScale = RIMLIGHTBOOST;
		info.m_nPhongColorTint = PHONGTINT;
		info.m_nSelfIllumTint = SELFILLUMTINT;
//		info.m_nUVProjOffset = UVPROJOFFSET;
//		info.m_nBBMin = BBMIN;
//		info.m_nBBMax = BBMAX;
		info.m_nFlashlightTexture = FLASHLIGHTTEXTURE;
		info.m_nFlashlightTextureFrame = FLASHLIGHTTEXTUREFRAME;
		info.m_nBumpFrame = BUMPFRAME;
		info.m_nBackScatter = BACKSCATTER;
		info.m_nForwardScatter = FORWARDSCATTER;
		info.m_nAmbientBoost = AMBIENTBOOST;
		info.m_nAmbientBoostMaskMode = AMBIENTBOOSTMASKMODE;
		info.m_nIridescenceExponent = IRIDESCENCEEXPONENT;
		info.m_nIridescenceBoost = IRIDESCENCEBOOST;
		info.m_nHueShiftIntensity = HUESHIFTINTENSITY;
		info.m_nHueShiftFresnelExponent = HUESHIFTFRESNELEXPONENT;
		info.m_nSSDepth = SSDEPTH;
		info.m_nSSTintByAlbedo = SSTINTBYALBEDO;
		info.m_nSSBentNormalIntensity = SSBENTNORMALINTENSITY;
		info.m_nSSColorTint = SSCOLORTINT;
		info.m_nNormal2Softness = NORMAL2SOFTNESS;
	}

	bool IsTranslucent(IMaterialVar **params) const
	{
		if (params[CLOAKPASSENABLED]->GetIntValue()) // If material supports cloaking
		{
			if ((params[CLOAKFACTOR]->GetFloatValue() > 0.0f) && (params[CLOAKFACTOR]->GetFloatValue() < 1.0f)) // Per-frame check
				return true;
			// else, not cloaking this frame, so check flag in case the base material still needs it
		}
/*
		if (params[SUBSURFACEENABLED]->IsDefined() && params[SUBSURFACEENABLED]->GetIntValue())
		{
			//	if (params[INTERIOR]->IsDefined() && params[INTERIOR]->GetIntValue() == 1)
			//	{
			//		return true;
			//	}

			return false;
		}
*/
		// Check flag if not drawing cloak pass
		if (IS_FLAG_SET(MATERIAL_VAR_TRANSLUCENT))
		{
			return true;
		}

		return  false;
	}

	SHADER_INIT_PARAMS()
	{
		VertexLitGeneric_DX9_Vars_t vars;
		SetupVars(vars);
		InitParamsVertexLitGeneric_DX9(this, params, pMaterialName, true, vars);
		
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

		// Emissive Scroll Pass
		if (!params[EMISSIVEBLENDENABLED]->IsDefined() )
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

		if (!params[SUBSURFACEENABLED]->IsDefined() || !r_subsurfaceinterior.GetBool())
		{
			params[SUBSURFACEENABLED]->SetIntValue(0);
		}

		if (!params[INTERIOR]->IsDefined())
		{
			params[INTERIOR]->SetIntValue(0);
		}

		if (params[SUBSURFACEENABLED]->GetIntValue())
		{
			FleshVars_t info;
			SetupVarsFlesh(info);
			InitParamsFlesh(this, params, pMaterialName, info);
		}
	}

	SHADER_FALLBACK
	{
		if (g_pHardwareConfig->GetDXSupportLevel() < 70)
			return "VertexLitGeneric_DX6";

		if (g_pHardwareConfig->GetDXSupportLevel() < 80)
			return "VertexLitGeneric_DX7";

		if (g_pHardwareConfig->GetDXSupportLevel() < 90)
			return "VertexLitGeneric_DX8";

		return 0;
	}

	SHADER_INIT
	{
		VertexLitGeneric_DX9_Vars_t vars;
		SetupVars( vars );
		InitVertexLitGeneric_DX9( this, params, true, vars );
		
		// Cloak Pass
		if ( params[CLOAKPASSENABLED]->GetIntValue() )
		{
			CloakBlendedPassVars_t info;
			SetupVarsCloakBlendedPass( info );
			InitCloakBlendedPass( this, params, info );
		}

		// Emissive Scroll Pass
		if ( params[EMISSIVEBLENDENABLED]->GetIntValue())
		{
			EmissiveScrollBlendedPassVars_t info;
			SetupVarsEmissiveScrollBlendedPass( info );
			InitEmissiveScrollBlendedPass( this, params, info );
		}

		// Flesh Interior Pass
		if ( params[FLESHINTERIORENABLED]->GetIntValue() )
		{
			FleshInteriorBlendedPassVars_t info;
			SetupVarsFleshInteriorBlendedPass( info );
			InitFleshInteriorBlendedPass( this, params, info );
		}

		// Cloak Pass
		if (params[SUBSURFACEENABLED]->GetIntValue() && r_subsurfaceinterior.GetBool())
		{
			FleshVars_t info;
			SetupVarsFlesh(info);
			InitFlesh(this, params, info);
		}
	}

	SHADER_DRAW
	{
		// Skip the standard rendering if cloak pass is fully opaque
		bool bDrawStandardPass = true;
	
		if ( params[CLOAKPASSENABLED]->GetIntValue() && ( pShaderShadow == NULL ) ) // && not snapshotting
		{
			CloakBlendedPassVars_t info;
			SetupVarsCloakBlendedPass( info );
			if ( CloakBlendedPassIsFullyOpaque( params, info ) )
			{
				bDrawStandardPass = false;
			}
		}
		
		// Standard rendering pass
		if ( bDrawStandardPass )
		{
			VertexLitGeneric_DX9_Vars_t vars;
			SetupVars( vars );
			DrawVertexLitGeneric_DX9( this, params, pShaderAPI, pShaderShadow, true, vars, vertexCompression, pContextDataPtr );
		}
		else
		{
			// Skip this pass!
			Draw( false );
		}

		// Cloak Pass
		if ( params[CLOAKPASSENABLED]->GetIntValue() )
		{
			// If ( snapshotting ) or ( we need to draw this frame )
 			if ( ( pShaderShadow != NULL ) || ( ( params[CLOAKFACTOR]->GetFloatValue() > 0.0f ) && ( params[CLOAKFACTOR]->GetFloatValue() < 1.0f ) ) )
			{
				CloakBlendedPassVars_t info;
				SetupVarsCloakBlendedPass( info );
				DrawCloakBlendedPass( this, params, pShaderAPI, pShaderShadow, info, vertexCompression );
			}
			else // We're not snapshotting and we don't need to draw this frame
			{
				// Skip this pass!
				Draw( false );
			}
		}

		// Flesh Interior Pass
		if ( params[FLESHINTERIORENABLED]->GetIntValue() )
		{
			// If ( snapshotting ) or ( we need to draw this frame )
			if ( ( pShaderShadow != NULL ))
			{
				FleshInteriorBlendedPassVars_t info;
				SetupVarsFleshInteriorBlendedPass( info );
				DrawFleshInteriorBlendedPass( this, params, pShaderAPI, pShaderShadow, info, vertexCompression );
			}
			else // We're not snapshotting and we don't need to draw this frame
			{
				// Skip this pass!
				Draw( false );
			}
		}
				
		// Emissive Scroll Pass
		if (params[EMISSIVEBLENDENABLED]->GetIntValue() )
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
		else // don't allow drawing both emissing and subsurface at the same time.
		{
			// Subsurface scattering Pass
			if (params[SUBSURFACEENABLED]->GetIntValue() && r_subsurfaceinterior.GetBool())
			{
				// If ( snapshotting ) or ( we need to draw this frame )
				if ((pShaderShadow != NULL) || (params[BACKSCATTER]->GetFloatValue() > 0.0f) || (params[FORWARDSCATTER]->GetFloatValue() > 0.0f))
				{
					FleshVars_t info;
					SetupVarsFlesh(info);
					DrawFlesh(this, params, pShaderAPI, pShaderShadow, info, vertexCompression);
				}

				else // We're not snapshotting and we don't need to draw this frame
				{
					// Skip this pass!
					Draw(false);
				}
			}
		}
	}
END_SHADER
