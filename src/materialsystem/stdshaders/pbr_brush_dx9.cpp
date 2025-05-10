//// 
// TODO LIST:
// - option to use $accumulatemask as a mask for albedo replacement (to avoid snow inside of cars, etc.) // done, via $accumulatetexture's alpha
// - $mro2, MRO blending
// - masked blending
// - animated emissive
// - mro scaling values
// - look into parallax mapping
// - tie accumulation direction and dot to map-placed entity // done, for now via worldspawn
// - animated accumulate textures
////

// Includes for all shaders
#include "BaseVSShader.h"
#include "cpp_shader_constant_register_map.h"
#include "convar.h"

// Includes for PS30
#include "pbr_vs30.inc"
#include "pbr_ps30.inc"

// Includes for PS20b
#include "pbr_vs20b.inc"
#include "pbr_ps20b.inc"

// Defining samplers
const Sampler_t SAMPLER_BASETEXTURE =		SHADER_SAMPLER0;
const Sampler_t SAMPLER_NORMAL =			SHADER_SAMPLER1;
const Sampler_t SAMPLER_ENVMAP =			SHADER_SAMPLER2;
const Sampler_t SAMPLER_BASETEXTURE2 =		SHADER_SAMPLER3;
const Sampler_t SAMPLER_SHADOWDEPTH =		SHADER_SAMPLER4;
const Sampler_t SAMPLER_RANDOMROTATION =	SHADER_SAMPLER5;
const Sampler_t SAMPLER_FLASHLIGHT =		SHADER_SAMPLER6;
const Sampler_t SAMPLER_LIGHTMAP =			SHADER_SAMPLER7;
//const Sampler_t SAMPLER_BLENDMODULATE =		SHADER_SAMPLER9; // masked blending
const Sampler_t SAMPLER_MRO =				SHADER_SAMPLER10;
const Sampler_t SAMPLER_EMISSIVE =			SHADER_SAMPLER11; // NOTE: This doubles as envmapmask sampler. If both are specified (and not using base's or bumpmap's alphas as envmapmask), then this sampler's rgb will be used for Emissive, and the alpha will be used for Envmapmask
const Sampler_t SAMPLER_MRO2 =				SHADER_SAMPLER12;
const Sampler_t SAMPLER_NORMAL2 =			SHADER_SAMPLER13;
const Sampler_t SAMPLER_ACCUMULATE_TEXTURE = SHADER_SAMPLER15; // dynamic snow, ash, rust...
const Sampler_t SAMPLER_ACCUMULATE_BUMP	 =	SHADER_SAMPLER14; // dynamic accumulate normalmap
//const Sampler_t SAMPLER_EMISSIVE2 =			SHADER_SAMPLER15; // have chosen to sacrifice emission blending to have a spare sampler for snow/ash texture layer.

// Convars
static ConVar mat_fullbright("mat_fullbright", "0", FCVAR_CHEAT);
static ConVar mat_specular("mat_specular", "1", FCVAR_CHEAT);
static ConVar mat_pbr_force_20b("mat_pbr_force_20b", "0", FCVAR_CHEAT);
ConVar mat_pbr_disable_accumulation("mat_pbr_disable_accumulate", "0");
static ConVar mat_pbr_disable_blendmask("mat_pbr_disable_blendmask", "1");

// Variables for this shader
struct PBR_Brush_Vars_t
{
	PBR_Brush_Vars_t()
	{
		memset(this, 0xFF, sizeof(*this));
	}

	int baseTexture;
	int baseTexture2;
	int baseColor;
	int normalTexture;
	int bumpMap;
	int bumpMap2;
	int bumpFrame;
	int bumpFrame2;
	//	int bumpScale;
	//	int bumpScale2;
	int envMap;
	int baseTextureFrame;
	int baseTextureTransform;
	int baseTexture2Frame;
	//	int baseTexture2Transform;
	//	int useParallax;
	//	int parallaxDepth;
	//	int parallaxCenter;
	int alphaTestReference;
	int flashlightTexture;
	int flashlightTextureFrame;
	int emissiveTexture;
	int emissiveTint;
	//int emissiveTexture2; // not supported yet
	int mroTexture;
	int mroTexture2;
	int mroFrame;
	int mroFrame2;
	int useEnvAmbient;
	int blendModulateTexture;

	// Masking, Tint, Contrast and Saturation Parameters
	int envMapMask;
	int envMapContrast;
	int envMapSaturation;
	int	envMapTint;
	int envMapMaskFrame;
	int envMapMaskInverse;

	// MRO tweak
	int MROtint;
	int MROtintReplace;

	// Detailtexture implementation
	//	int detailTexture;
	//	int detailBlendMode;
	//	int detailBlendFactor;
	//	int detailTransform;
	//	int detailScale;
	//	int detailTint;

	// Dynamic accumulation implementation
	int accumDot;
	int accumDir;
	int enableAccumulation;
	int accumulateTexture; // if specified, its alpha will be used for blending between basetexture and accumulatedTexture.
	int accumulateBaseStrength;
	int accumulateBumpmap;
	int accumulateBumpStrength;
	int accumDotOverride;
	int accumDirOverride;
	int accumulateTextureTransform;

	// Alphablend
//	int alphaBlend;
	int alphaBlendColor; // without alphablend, it is used for accum tinting
};

//static const float kDefaultBumpScale = 1.0f;
//static const float kDefaultBumpScale2 = 1.0f;

// Beginning the shader
BEGIN_VS_SHADER(PBR_Brush, "PBR shader")

// Setting up vmt parameters
BEGIN_SHADER_PARAMS;
SHADER_PARAM(BASETEXTURE2, SHADER_PARAM_TYPE_TEXTURE, "shadertest/lightmappedtexture", "Blended texture");
SHADER_PARAM(BLENDMODULATIONMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/lightmappedtexture", "Blendmodulate texture");
SHADER_PARAM(FRAME2, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $basetexture2");
SHADER_PARAM(ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0", "");
SHADER_PARAM(MRO, SHADER_PARAM_TYPE_TEXTURE, "", "Texture with metalness in R, roughness in G, ambient occlusion in B.");
SHADER_PARAM(MRO2, SHADER_PARAM_TYPE_TEXTURE, "", "Texture with metalness in R, roughness in G, ambient occlusion in B.");
SHADER_PARAM(MROFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $mro");
SHADER_PARAM(MROFRAME2, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $mro2");
SHADER_PARAM(EMISSIVE, SHADER_PARAM_TYPE_TEXTURE, "", "Emissive texture");
SHADER_PARAM(EMISSIVE2, SHADER_PARAM_TYPE_TEXTURE, "", "Emissive texture");
SHADER_PARAM(NORMALTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Normal texture (deprecated, use $bumpmap)");
SHADER_PARAM(BUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "", "Normal texture");
SHADER_PARAM(BUMPFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap/$normaltexture");
//SHADER_PARAM(BUMPSCALE, SHADER_PARAM_TYPE_FLOAT, "1.0", "bumpmap strength multiplier");
SHADER_PARAM(BUMPMAP2, SHADER_PARAM_TYPE_TEXTURE, "", "Normal texture 2");
SHADER_PARAM(BUMPFRAME2, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $bumpmap2/$normaltexture2");
//SHADER_PARAM(BUMPSCALE2, SHADER_PARAM_TYPE_FLOAT, "1", "bumpmap2 strength multiplier");
SHADER_PARAM(USEENVAMBIENT, SHADER_PARAM_TYPE_BOOL, "0", "Use the cubemaps to compute ambient light.");
SHADER_PARAM(EMISSIVETINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Color to multiply emissive texture with");

// Envmapping parameters
SHADER_PARAM(ENVMAP, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_env", "Set the cubemap for this material.");
SHADER_PARAM(ENVMAPTINT, SHADER_PARAM_TYPE_COLOR, "", "Tints the $envmap");
SHADER_PARAM(ENVMAPCONTRAST, SHADER_PARAM_TYPE_FLOAT, "", "0-1 where 1 is full contrast and 0 is pure cubemap");
SHADER_PARAM(ENVMAPSATURATION, SHADER_PARAM_TYPE_FLOAT, "", "0-1 where 1 is default pure cubemap and 0 is b/w.");
SHADER_PARAM(ENVMAPMASK, SHADER_PARAM_TYPE_TEXTURE, "shadertest/shadertest_envmask", "Envmap mask");
SHADER_PARAM(ENVMAPMASKFRAME, SHADER_PARAM_TYPE_INTEGER, "", "")
SHADER_PARAM(INVERTENVMAPMASK, SHADER_PARAM_TYPE_BOOL, "0", "Whenever we want to invert the mask.");

// MRO tweaks
SHADER_PARAM(MROTINT, SHADER_PARAM_TYPE_COLOR, "[1 1 1]", "Scales or replaces MRO values");
SHADER_PARAM(MROTINTREPLACE, SHADER_PARAM_TYPE_FLOAT, "0", "If 0, MROTint values will set MRO values rather than scale them");

// Detail textures
//SHADER_PARAM(DETAIL, SHADER_PARAM_TYPE_TEXTURE, "", "Texture");
//SHADER_PARAM(DETAILTEXTURETRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$detail & $detail2 texcoord transform");
//SHADER_PARAM(DETAILFRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $detail");
//SHADER_PARAM(DETAILSCALE, SHADER_PARAM_TYPE_FLOAT, "4", "scale of the detail texture");
//SHADER_PARAM(DETAILTINT, SHADER_PARAM_TYPE_COLOR, "", "Tints $Detail");
//SHADER_PARAM(DETAILBLENDMODE, SHADER_PARAM_TYPE_INTEGER, "0", "Check VDC for more information");
//SHADER_PARAM(DETAILBLENDFACTOR, SHADER_PARAM_TYPE_FLOAT, "", "How much detail is it that you want?");

// Dynamic accumulation
SHADER_PARAM(ENABLEACCUMULATION, SHADER_PARAM_TYPE_BOOL, "0", "Perform dynamic accumulation pixels on this material");
SHADER_PARAM(ACCUMULATETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "Accumulated layer texture");
SHADER_PARAM(ACCUMULATEBASESTRENGTH, SHADER_PARAM_TYPE_FLOAT, "1.0", "");
SHADER_PARAM(ACCUMULATEBUMPMAP, SHADER_PARAM_TYPE_TEXTURE, "", "Accumulated layer texture");
SHADER_PARAM(ACCUMULATEBUMPSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "1.0", "");
SHADER_PARAM(ACCUMULATEDOTOVERRIDE, SHADER_PARAM_TYPE_FLOAT, "0", "If greater than 0, will override the accumulation dot dir taken from worldspawn settings");
SHADER_PARAM(ACCUMULATEDIROVERRIDE, SHADER_PARAM_TYPE_COLOR, "[-1 -1 -1]", "If greater than -1 -1 -1, will override the accumulation dir taken from worldspawn settings");
SHADER_PARAM(ACCUMULATETEXTURETRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$accumulatetexture texcoord transform")

SHADER_PARAM(ALPHABLENDCOLOR, SHADER_PARAM_TYPE_COLOR, "", "");

END_SHADER_PARAMS;

// Setting up variables for this shader
void SetupVars(PBR_Brush_Vars_t &info)
{
	info.baseTexture = BASETEXTURE;
	info.baseTexture2 = BASETEXTURE2;
	info.blendModulateTexture = BLENDMODULATIONMAP;
	info.baseColor = COLOR;
	info.normalTexture = NORMALTEXTURE;
	info.bumpMap = BUMPMAP;
	info.bumpFrame = BUMPFRAME;
	//	info.bumpScale = BUMPSCALE;
	info.bumpMap2 = BUMPMAP2;
	info.bumpFrame2 = BUMPFRAME2;
	//	info.bumpScale2 = BUMPSCALE2;
	info.baseTextureFrame = FRAME;
	info.baseTexture2Frame = FRAME2;
	info.baseTextureTransform = BASETEXTURETRANSFORM;
	//	info.baseTexture2Transform = BASETEXTURETRANSFORM;
	info.alphaTestReference = ALPHATESTREFERENCE;
	info.flashlightTexture = FLASHLIGHTTEXTURE;
	info.flashlightTextureFrame = FLASHLIGHTTEXTUREFRAME;
	info.emissiveTexture = EMISSIVE;
	info.emissiveTint = EMISSIVETINT;
	//info.emissiveTexture2 = EMISSIVE2; // not supported yet
	info.mroTexture = MRO;
	info.mroTexture2 = MRO2;
	info.mroFrame = MROFRAME;
	info.mroFrame2 = MROFRAME2;
	info.useEnvAmbient = USEENVAMBIENT;
	//info.useParallax = PARALLAX;
	//info.parallaxDepth = PARALLAXDEPTH;
	//info.parallaxCenter = PARALLAXCENTER;

	// Detail textures...
	//	info.detailTexture = DETAIL;
	//	info.detailTransform = DETAILTEXTURETRANSFORM;
	//	info.detailFrame = DETAILFRAME; // not supported yet
	//	info.detailScale = DETAILSCALE;
	//	info.detailTint = DETAILTINT;
	//	info.detailBlendMode = DETAILBLENDMODE;
	//	info.detailBlendFactor = DETAILBLENDFACTOR;

	// Envmapping...
	info.envMap = ENVMAP;
	info.envMapMask = ENVMAPMASK;
	info.envMapMaskFrame = ENVMAPMASKFRAME;
	info.envMapTint = ENVMAPTINT;
	info.envMapContrast = ENVMAPCONTRAST;
	info.envMapSaturation = ENVMAPSATURATION;
	info.envMapMaskInverse = INVERTENVMAPMASK;

	// Dynamic accumulation
	info.enableAccumulation = ENABLEACCUMULATION;
	info.accumulateTexture = ACCUMULATETEXTURE;
	info.accumulateBaseStrength = ACCUMULATEBASESTRENGTH;
	info.accumulateBumpmap = ACCUMULATEBUMPMAP;
	info.accumulateBumpStrength = ACCUMULATEBUMPSTRENGTH;
	info.accumDotOverride = ACCUMULATEDOTOVERRIDE;
	info.accumDirOverride = ACCUMULATEDIROVERRIDE;
	info.accumulateTextureTransform = ACCUMULATETEXTURETRANSFORM;

	info.alphaBlendColor = ALPHABLENDCOLOR;

	// MRO tweaks
	info.MROtint = MROTINT;
	info.MROtintReplace = MROTINTREPLACE;
};

// Initializing parameters
SHADER_INIT_PARAMS()
{
	// Fallback for changed parameter
	if (params[NORMALTEXTURE]->IsDefined())
		params[BUMPMAP]->SetStringValue(params[NORMALTEXTURE]->GetStringValue());

	// Dynamic lights need a bumpmap
	if (!params[BUMPMAP]->IsDefined())
		params[BUMPMAP]->SetStringValue("dev/flat_normal");

	// Dynamic lights need a bumpmap
	if (!params[BUMPMAP2]->IsDefined() && params[BASETEXTURE2]->IsDefined()) // not having it defined usually means the artist wants to blend under same normalmap.
		params[BUMPMAP2]->SetStringValue(params[BUMPMAP]->GetStringValue());

	// Set a good default mro texture
	if (!params[MRO]->IsDefined())
		params[MRO]->SetStringValue("pbr/default_mro");

	// Set a good default mro texture
	if (!params[MRO2]->IsDefined() && params[BASETEXTURE2]->IsDefined()) // not having it defined usually means the artist wants to blend with the same MRO.
		params[MRO2]->SetStringValue(params[MRO]->GetStringValue());
	
	// PBR relies heavily on envmaps
	if (!params[ENVMAP]->IsDefined())
		params[ENVMAP]->SetStringValue("env_cubemap");
	
	// Check if the hardware supports flashlight border color
	if (g_pHardwareConfig->SupportsBorderColor())
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight_border");
	}
	else
	{
		params[FLASHLIGHTTEXTURE]->SetStringValue("effects/flashlight001");
	}
};

// Define shader fallback
SHADER_FALLBACK
{
	// TODO: Reasonable fallback
	if (!(g_pHardwareConfig->SupportsPixelShaders_2_b()))
	{
		return "LightmappedGeneric";
	}

	return 0;
};

SHADER_INIT
{
	PBR_Brush_Vars_t info;
	SetupVars(info);

	//	SET_PARAM_FLOAT_IF_NOT_DEFINED(info.bumpScale, kDefaultBumpScale);
	//	SET_PARAM_FLOAT_IF_NOT_DEFINED(info.bumpScale2, kDefaultBumpScale2);

	// Dynamic accumulation
	SET_PARAM_INT_IF_NOT_DEFINED(info.enableAccumulation, 0);
	SET_PARAM_FLOAT_IF_NOT_DEFINED(info.accumulateBaseStrength, 1.0f);
	SET_PARAM_FLOAT_IF_NOT_DEFINED(info.accumulateBumpStrength, 1.0f);
	SET_PARAM_FLOAT_IF_NOT_DEFINED(info.accumDotOverride, 0);
	
	Assert(info.flashlightTexture >= 0);
		LoadTexture(info.flashlightTexture, TEXTUREFLAGS_SRGB);

	if (params[info.baseTexture]->IsDefined())
		LoadTexture(info.baseTexture, TEXTUREFLAGS_SRGB);

	if (params[info.baseTexture2]->IsDefined())
		LoadTexture(info.baseTexture2, TEXTUREFLAGS_SRGB);

	if (params[info.blendModulateTexture]->IsDefined())
		LoadTexture(info.blendModulateTexture, TEXTUREFLAGS_SRGB);

	//Assert(info.bumpMap >= 0);
	if (params[BUMPMAP]->IsDefined())
		LoadBumpMap(info.bumpMap);

	if (params[BUMPMAP2]->IsDefined() && params[BASETEXTURE2]->IsDefined())
		LoadBumpMap(info.bumpMap2);

	//	Assert(info.envMap >= 0);
	int envMapFlags = g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE ? TEXTUREFLAGS_SRGB : 0;
	envMapFlags |= TEXTUREFLAGS_ALL_MIPS;
		LoadCubeMap(info.envMap, envMapFlags);
		
	if (!params[info.envMapMaskFrame]->IsDefined())
		params[info.envMapMaskFrame]->SetIntValue(0);

	if (!params[info.envMapTint]->IsDefined())
		params[info.envMapTint]->SetVecValue(1.0f, 1.0f, 1.0f);

	if (!params[info.envMapContrast]->IsDefined())
		params[info.envMapContrast]->SetFloatValue(0.0f);

	if (!params[info.envMapSaturation]->IsDefined())
		params[info.envMapSaturation]->SetFloatValue(1.0f);

	if (!params[info.envMapMaskInverse]->IsDefined())
		params[info.envMapMaskInverse]->SetIntValue(0);

	// No texture means no self-illum or env mask in base alpha
	if (!params[info.baseTexture]->IsDefined())
	{
		CLEAR_FLAGS(MATERIAL_VAR_SELFILLUM);
		CLEAR_FLAGS(MATERIAL_VAR_BASEALPHAENVMAPMASK);
	}

//	if (!params[info.normalTexture]->IsDefined())
//	{
//		CLEAR_FLAGS(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK);
//	}

	if (params[ENVMAPMASK]->IsDefined())
		LoadTexture(info.envMapMask, TEXTUREFLAGS_SRGB);

	if (params[EMISSIVE]->IsDefined())
		LoadTexture(info.emissiveTexture, TEXTUREFLAGS_SRGB);

	//if ( params[EMISSIVE2]->IsDefined() && params[BASETEXTURE2]->IsDefined())
	//	LoadTexture(info.emissiveTexture2, TEXTUREFLAGS_SRGB); // not supported yet

	Assert(info.mroTexture >= 0);
	if (params[MRO]->IsDefined())
		LoadTexture(info.mroTexture);

	if (params[MRO2]->IsDefined() && params[BASETEXTURE2]->IsDefined())
		LoadTexture(info.mroTexture2);

	/*
	if (params[info.detailTexture]->IsDefined())
	{
		if (params[info.detailBlendMode]->GetIntValue() > 0)
		{
			LoadTexture(info.detailTexture);
		}
		else
		{
			LoadTexture(info.detailTexture, TEXTUREFLAGS_SRGB); // when mod2x, load as sRGB
		}
	}
	*/
	// dynamic accumulation
	if (params[info.accumulateTexture]->IsDefined())
	{
		LoadTexture(info.accumulateTexture, TEXTUREFLAGS_SRGB);
	}

	if (params[info.accumulateBumpmap]->IsDefined())
	{
		LoadTexture(info.accumulateBumpmap); // this is a normalmap too, so no sRGB // fixme: shouldn't this be LoadBumpmap?
	}

	if (IS_FLAG_SET(MATERIAL_VAR_MODEL)) // Set material var2 flags specific to models
	{
		SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_HW_SKINNING);             // Required for skinning
		SET_FLAGS2(MATERIAL_VAR2_DIFFUSE_BUMPMAPPED_MODEL);         // Required for dynamic lighting
		SET_FLAGS2(MATERIAL_VAR2_NEEDS_TANGENT_SPACES);             // Required for dynamic lighting
		SET_FLAGS2(MATERIAL_VAR2_LIGHTING_VERTEX_LIT);              // Required for dynamic lighting
		SET_FLAGS2(MATERIAL_VAR2_NEEDS_BAKED_LIGHTING_SNAPSHOTS);   // Required for ambient cube
		SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_FLASHLIGHT);              // Required for flashlight
		SET_FLAGS2(MATERIAL_VAR2_USE_FLASHLIGHT);                   // Required for flashlight
	}
	else // Set material var2 flags specific to brushes
	{
		SET_FLAGS2(MATERIAL_VAR2_LIGHTING_LIGHTMAP);                // Required for lightmaps
		SET_FLAGS2(MATERIAL_VAR2_LIGHTING_BUMPED_LIGHTMAP);         // Required for lightmaps
		SET_FLAGS2(MATERIAL_VAR2_SUPPORTS_FLASHLIGHT);              // Required for flashlight
		SET_FLAGS2(MATERIAL_VAR2_USE_FLASHLIGHT);                   // Required for flashlight
	}
};

// Drawing the shader
SHADER_DRAW
{
	PBR_Brush_Vars_t info;
	SetupVars(info);

	// Setting up booleans
	bool bHasBaseTexture = params[info.baseTexture]->IsTexture();
	bool bIsBlended = params[info.baseTexture2]->IsTexture();
	bool bMaskedBlending = bIsBlended && params[info.blendModulateTexture]->IsTexture() && !mat_pbr_disable_blendmask.GetBool();

	bool bHasNormalTexture = params[info.bumpMap]->IsTexture();
	bool bHasMroTexture = params[info.mroTexture]->IsTexture();
	bool bHasEmissiveTexture = params[info.emissiveTexture]->IsTexture();
	bool bHasEmissiveTint = params[info.emissiveTint]->IsDefined();
	bool bHasEnvTexture = params[info.envMap]->IsTexture();
	bool bIsAlphaTested = IS_FLAG_SET(MATERIAL_VAR_ALPHATEST) != 0;
	bool bHasFlashlight = UsingFlashlight(params);
	bool bHasColor = params[info.baseColor]->IsDefined();
	bool bLightMapped = true;
	bool bUseEnvAmbient = false; // (params[info.useEnvAmbient]->GetIntValue() == 1);
	int nEnvmapmaskMode = 0;
	if (params[info.envMapMask]->IsDefined() && params[info.envMapMask]->IsTexture() && !bHasEmissiveTexture) nEnvmapmaskMode = 1;
	if (IS_FLAG_SET(MATERIAL_VAR_BASEALPHAENVMAPMASK)) nEnvmapmaskMode = 2;
	if (IS_FLAG_SET(MATERIAL_VAR_NORMALMAPALPHAENVMAPMASK)) nEnvmapmaskMode = 3;
	bool bInvertEnvMapMask = (params[info.envMapMaskInverse]->GetIntValue() == 1);
	bool bEnableAccumulation = (params[info.enableAccumulation]->GetIntValue() == 1 && mat_pbr_disable_accumulation.GetBool() == 0);
	bool bHasAccumulateTexture = params[info.accumulateTexture]->IsTexture();
	bool bHasAccumulateBumpmap = params[info.accumulateBumpmap]->IsTexture();
	
	bool bHasNormalTexture2 = bIsBlended && params[info.bumpMap2]->IsTexture();
	bool bHasMroTexture2 = bIsBlended && params[info.mroTexture2]->IsTexture();
	// bool bHasEmissiveTexture2 = bIsBlended && params[info.emissiveTexture2]->IsTexture(); // not yet supported
	//	bool bHasEmissiveScale2 = bIsBlended && params[info.emissiveScale2]->IsDefined(); // not yet supported

	//	bool bHasDetailTexture = params[info.detailTexture]->IsTexture();

	// Determining whether we're dealing with a fully opaque material
	BlendType_t nBlendType = EvaluateBlendRequirements(info.baseTexture, true);
	bool bFullyOpaque = (nBlendType != BT_BLENDADD) && (nBlendType != BT_BLEND) && !bIsAlphaTested;

	if (IsSnapshotting())
	{
		// If alphatest is on, enable it
		pShaderShadow->EnableAlphaTest(bIsAlphaTested);

		if (params[info.alphaTestReference]->GetFloatValue() > 0.0f)
		{
			pShaderShadow->AlphaFunc(SHADER_ALPHAFUNC_GEQUAL, params[info.alphaTestReference]->GetFloatValue());
		}

		if (bHasFlashlight)
		{
			pShaderShadow->EnableBlending(true);
			pShaderShadow->BlendFunc(SHADER_BLEND_ONE, SHADER_BLEND_ONE); // Additive blending
		}
		else
		{
			SetDefaultBlendingShadowState(info.baseTexture, true);
		}

		int nShadowFilterMode = bHasFlashlight ? g_pHardwareConfig->GetShadowFilterMode() : 0;

		// Setting up samplers
		pShaderShadow->EnableTexture(SAMPLER_BASETEXTURE, true);    // Basecolor texture
		pShaderShadow->EnableSRGBRead(SAMPLER_BASETEXTURE, true);   // Basecolor is sRGB
		pShaderShadow->EnableTexture(SAMPLER_EMISSIVE, true);       // Emissive texture
		pShaderShadow->EnableSRGBRead(SAMPLER_EMISSIVE, true);      // Emissive is sRGB
		pShaderShadow->EnableTexture(SAMPLER_LIGHTMAP, true);       // Lightmap texture
		pShaderShadow->EnableSRGBRead(SAMPLER_LIGHTMAP, false);     // Lightmaps aren't sRGB
		pShaderShadow->EnableTexture(SAMPLER_MRO, true);            // MRO texture
		pShaderShadow->EnableSRGBRead(SAMPLER_MRO, false);          // MRO isn't sRGB
		pShaderShadow->EnableTexture(SAMPLER_NORMAL, true);         // Normal texture
		pShaderShadow->EnableSRGBRead(SAMPLER_NORMAL, false);       // Normals aren't sRGB
		
		pShaderShadow->EnableTexture(SAMPLER_ACCUMULATE_TEXTURE, true);    // dynamic accumulate
		pShaderShadow->EnableSRGBRead(SAMPLER_ACCUMULATE_TEXTURE, true);   // is sRGB
		pShaderShadow->EnableTexture(SAMPLER_ACCUMULATE_BUMP, true);    // dynamic accumulate bump
		pShaderShadow->EnableSRGBRead(SAMPLER_ACCUMULATE_BUMP, false);   // is a normal map, no sRGB

		if (bIsBlended)
		{
			pShaderShadow->EnableTexture(SAMPLER_BASETEXTURE2, true);    // Basecolor 2 texture
			pShaderShadow->EnableSRGBRead(SAMPLER_BASETEXTURE2, true);   // Basecolor 2 is sRGB
			pShaderShadow->EnableTexture(SAMPLER_NORMAL2, true);         // Normal texture 2
			pShaderShadow->EnableSRGBRead(SAMPLER_NORMAL2, false);       // Normals aren't sRGB
			if (bMaskedBlending)
			{
			//	pShaderShadow->EnableTexture(SAMPLER_BLENDMODULATE, true);
			//	pShaderShadow->EnableSRGBRead(SAMPLER_BLENDMODULATE, false);
			}
			pShaderShadow->EnableTexture(SAMPLER_MRO2, true);           // MRO texture 2
			pShaderShadow->EnableSRGBRead(SAMPLER_MRO2, false);         // MRO isn't sRGB
		//	pShaderShadow->EnableTexture(SAMPLER_EMISSIVE2, true);       // Emissive texture
		//	pShaderShadow->EnableSRGBRead(SAMPLER_EMISSIVE2, true);      // Emissive is sRGB
		}

		// If the flashlight is on, set up its textures
		if (bHasFlashlight)
		{
			pShaderShadow->EnableTexture(SAMPLER_SHADOWDEPTH, true);        // Shadow depth map
			pShaderShadow->SetShadowDepthFiltering(SAMPLER_SHADOWDEPTH);
			pShaderShadow->EnableSRGBRead(SAMPLER_SHADOWDEPTH, false);
			pShaderShadow->EnableTexture(SAMPLER_RANDOMROTATION, true);     // Noise map
			pShaderShadow->EnableTexture(SAMPLER_FLASHLIGHT, true);         // Flashlight cookie
			pShaderShadow->EnableSRGBRead(SAMPLER_FLASHLIGHT, true);

			FogToBlack(); // todo: explain why
		}
		else
		{
			DefaultFog(); // todo: explain why
		}

		// Setting up envmap
		if (bHasEnvTexture)
		{
			pShaderShadow->EnableTexture(SAMPLER_ENVMAP, true); // Envmap
			if (g_pHardwareConfig->GetHDRType() == HDR_TYPE_NONE)
			{
				pShaderShadow->EnableSRGBRead(SAMPLER_ENVMAP, true); // Envmap is only sRGB with HDR disabled?
			}

			if (nEnvmapmaskMode == 1)
			{
				pShaderShadow->EnableTexture(SAMPLER_EMISSIVE, true);
				pShaderShadow->EnableSRGBRead(SAMPLER_EMISSIVE, true);
			}
		}
		/*
		// Detail texture
		if (bHasDetailTexture)
		pShaderShadow->EnableTexture(SAMPLER_DETAIL, true);
		{
		if (params[info.detailBlendMode]->GetIntValue() > 0)
		{
		pShaderShadow->EnableSRGBRead(SAMPLER_DETAIL, false);
		}
		else
		{
		pShaderShadow->EnableSRGBRead(SAMPLER_DETAIL, true);
		}
		}
		*/
		// Enabling sRGB writing
		// See common_ps_fxc.h line 349
		// PS2b shaders and up write sRGB
		pShaderShadow->EnableSRGBWrite(true);
		if (IS_FLAG_SET(MATERIAL_VAR_MODEL))
		{
			// We only need the position and surface normal
			unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL | VERTEX_FORMAT_COMPRESSED;
			// We only need one texcoord, in the default float2 size
			pShaderShadow->VertexShaderVertexFormat(flags, 1, 0, 0);
		}
		else
		{
			// We need the position, surface normal, and vertex compression format
			unsigned int flags = VERTEX_POSITION | VERTEX_NORMAL;
			if (bIsBlended)  flags |= VERTEX_COLOR;
			// We need three texcoords, all in the default float2 size
			pShaderShadow->VertexShaderVertexFormat(flags, 3, 0, 0);
		}

		int useDynamicAccumulation = params[info.enableAccumulation]->GetIntValue() && (mat_pbr_disable_accumulation.GetBool() == 0);

		if (!g_pHardwareConfig->SupportsShaderModel_3_0() || mat_pbr_force_20b.GetBool())
		{
			// Setting up static vertex shader
			DECLARE_STATIC_VERTEX_SHADER(pbr_vs20b);
			SET_STATIC_VERTEX_SHADER_COMBO(BLENDED, bIsBlended);
			SET_STATIC_VERTEX_SHADER_COMBO(TREESWAY, false);
			SET_STATIC_VERTEX_SHADER(pbr_vs20b);

			// Setting up static pixel shader
			DECLARE_STATIC_PIXEL_SHADER(pbr_ps20b);
			SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
			SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode);
			SET_STATIC_PIXEL_SHADER_COMBO(LIGHTMAPPED, bLightMapped);
			SET_STATIC_PIXEL_SHADER_COMBO(EMISSIVE, bHasEmissiveTexture);
			SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMASKINALPHA, nEnvmapmaskMode);
			SET_STATIC_PIXEL_SHADER_COMBO(INVERTENVMAPMASK, !bInvertEnvMapMask);
			SET_STATIC_PIXEL_SHADER_COMBO(ENABLEACCUMULATION, useDynamicAccumulation);
			SET_STATIC_PIXEL_SHADER_COMBO(ACCUMULATEBUMPMAP, bHasAccumulateBumpmap);
			SET_STATIC_PIXEL_SHADER_COMBO(BLENDED, bIsBlended);
			SET_STATIC_PIXEL_SHADER_COMBO(BLENDMODULATE, bIsBlended && bMaskedBlending);
			SET_STATIC_PIXEL_SHADER_COMBO(ALPHABLEND, false);
			SET_STATIC_PIXEL_SHADER(pbr_ps20b);
		}
		else
		{
			// Setting up static vertex shader
			DECLARE_STATIC_VERTEX_SHADER(pbr_vs30);
			SET_STATIC_VERTEX_SHADER_COMBO(BLENDED, bIsBlended);
			SET_STATIC_VERTEX_SHADER_COMBO(TREESWAY, false);
			SET_STATIC_VERTEX_SHADER(pbr_vs30);

			// Setting up static pixel shader
			DECLARE_STATIC_PIXEL_SHADER(pbr_ps30);
			SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHT, bHasFlashlight);
			SET_STATIC_PIXEL_SHADER_COMBO(FLASHLIGHTDEPTHFILTERMODE, nShadowFilterMode);
			SET_STATIC_PIXEL_SHADER_COMBO(LIGHTMAPPED, bLightMapped);
			SET_STATIC_PIXEL_SHADER_COMBO(USEENVAMBIENT, bUseEnvAmbient);
			SET_STATIC_PIXEL_SHADER_COMBO(ENVMAPMASKINALPHA, nEnvmapmaskMode);
			SET_STATIC_PIXEL_SHADER_COMBO(INVERTENVMAPMASK, !bInvertEnvMapMask);
			SET_STATIC_PIXEL_SHADER_COMBO(EMISSIVE, bHasEmissiveTexture /*|| bHasEmissiveTexture2*/);
			SET_STATIC_PIXEL_SHADER_COMBO(PARALLAXLAYER, false);
			SET_STATIC_PIXEL_SHADER_COMBO(ENABLEACCUMULATION, useDynamicAccumulation);
			SET_STATIC_PIXEL_SHADER_COMBO(ACCUMULATEBUMPMAP, bHasAccumulateBumpmap);
			SET_STATIC_PIXEL_SHADER_COMBO(BLENDED, bIsBlended);
			SET_STATIC_PIXEL_SHADER_COMBO(BLENDMODULATE, (bIsBlended && bMaskedBlending));
			SET_STATIC_PIXEL_SHADER_COMBO(ALPHABLEND, false); // only on models
			SET_STATIC_PIXEL_SHADER(pbr_ps30);
		}

		// Setting up fog
		//	DefaultFog(); // todo: should it be here or above in flashlight check?

		// HACK HACK HACK - enable alpha writes all the time so that we have them for underwater stuff
		pShaderShadow->EnableAlphaWrites(bFullyOpaque);
	}
	else // Not snapshotting -- begin dynamic state
	{
		bool bLightingOnly = mat_fullbright.GetInt() == 2 && !IS_FLAG_SET(MATERIAL_VAR_NO_DEBUG_OVERRIDE);

		// Setting up albedo texture
		if (bHasBaseTexture)
		{
			BindTexture(SAMPLER_BASETEXTURE, info.baseTexture, info.baseTextureFrame);
			if (bIsBlended)
			{
				if (CanUseEditorMaterials())
				{
					// swap base 1 and 2 for Hammer
					BindTexture(SAMPLER_BASETEXTURE2, info.baseTexture, info.baseTextureFrame);
					BindTexture(SAMPLER_BASETEXTURE, info.baseTexture2, info.baseTexture2Frame);
				}
				else
				{
					BindTexture(SAMPLER_BASETEXTURE2, info.baseTexture2, info.baseTexture2Frame);
				//	if (bMaskedBlending)
				//		BindTexture(SAMPLER_BLENDMODULATE, info.blendModulateTexture, 0);
				}
			}
		}
		else
		{
			pShaderAPI->BindStandardTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY);
		}

		// Setting up vmt color
		float accumulateBaseStrength = params[info.accumulateBaseStrength]->GetFloatValue();
		Vector color;
		if (bHasColor)
		{
			params[info.baseColor]->GetVecValue(color.Base(), 3);
		}
		else
		{
			color = Vector{ 1.f, 1.f, 1.f };
		}
		float baseParams[4] = { color[0], color[1], color[2], accumulateBaseStrength };
		pShaderAPI->SetPixelShaderConstant(PSREG_SELFILLUMTINT, baseParams);

		// Dynamic accumulation
		float accumParams[4] = { 0.0, 0.0, 1.0, 0.5 };
		float accumParams2[4] = { 0.0, 0.0, 0.0, 0.0 };
		if (bEnableAccumulation)
		{
#if 1
			Vector accumDir = pShaderAPI->GetVectorRenderingParameter(VECTOR_RENDERPARM_SNOW_DIRECTION);
			float accumDot = pShaderAPI->GetFloatRenderingParameter(FLOAT_RENDERPARM_SNOW_DOT);
#else
			Vector accumDir = Vector(0.f, 0.f, 1.0f);
			float accumDot = 0.5f;
			accumDot = mat_pbr_dynamicsnow_dot.GetFloat();
#endif

			if (params[info.accumDotOverride]->GetFloatValue() > 0.0f)
			{
				accumDot = params[info.accumDotOverride]->GetFloatValue();
			}

			if (params[info.accumDirOverride]->IsDefined())
			{
			//	float accumDirOverride[3] = { 0.0, 0.0, 1.0 };
			//	params[info.accumDirOverride]->GetVecValue(accumDirOverride, 3);
				
			//	if (accumDirOverride[0] > -1.0 && accumDirOverride[1] > -1.0 && accumDirOverride[2] > -1.0)
			//	{
			//		accumDir.x = accumDirOverride[0];
			//		accumDir.y = accumDirOverride[1];
			//		accumDir.z = accumDirOverride[2];
			//	}
			}

			accumParams[0] = accumDir.x;
			accumParams[1] = accumDir.y;
			accumParams[2] = accumDir.z;
			accumParams[3] = accumDot;

			accumParams2[0] = IS_PARAM_DEFINED(info.accumulateBumpStrength) ? params[info.accumulateBumpStrength]->GetFloatValue() : 1.0f;

			// if alphablend is 0 (and it always is for brushes), alphablendcolor is still used - for accumulate tinting
			Vector alphaBlendColor;
			params[info.alphaBlendColor]->GetVecValue(alphaBlendColor.Base(), 3);
			accumParams2[1] = (alphaBlendColor.x);
			accumParams2[2] = (alphaBlendColor.y);
			accumParams2[3] = (alphaBlendColor.z);
			
			pShaderAPI->SetPixelShaderConstant(PSREG_ENVMAP_FRESNEL__SELFILLUMMASK, accumParams);
			pShaderAPI->SetPixelShaderConstant(PSREG_SELFILLUM_SCALE_BIAS_EXP, accumParams2);

			if (bHasAccumulateTexture)
			{
				BindTexture(SAMPLER_ACCUMULATE_TEXTURE, info.accumulateTexture);
			}
			else
			{
				pShaderAPI->BindStandardTexture(SAMPLER_ACCUMULATE_TEXTURE, TEXTURE_WHITE);
			}

			if (bHasAccumulateBumpmap)
			{
				BindTexture(SAMPLER_ACCUMULATE_BUMP, info.accumulateBumpmap);
			}
			else
			{
				if (bHasNormalTexture)
				{
					BindTexture(SAMPLER_ACCUMULATE_BUMP, info.normalTexture);
				}
				else
				{
					pShaderAPI->BindStandardTexture(SAMPLER_ACCUMULATE_BUMP, TEXTURE_NORMALMAP_FLAT);
				}
			}
		}

		// Setting up emissive texture
		if (bHasEmissiveTexture)
		{
			BindTexture(SAMPLER_EMISSIVE, info.emissiveTexture, 0);
		}
		else
		{
			pShaderAPI->BindStandardTexture(SAMPLER_EMISSIVE, TEXTURE_BLACK);
		}

		// Setting up environment map
		if (bHasEnvTexture)
		{
			BindTexture(SAMPLER_ENVMAP, info.envMap, 0);
			
			if (nEnvmapmaskMode == 1)
			{
				BindTexture(SAMPLER_EMISSIVE, info.envMapMask, info.envMapMaskFrame);
			}
		}
		else
		{
			pShaderAPI->BindStandardTexture(SAMPLER_ENVMAP, TEXTURE_BLACK);
		}
		/*
		if (bHasDetailTexture)
		{
		//DevMsg("Detail Texture Set \n");
		BindTexture(SHADER_SAMPLER15, info.detailTexture, 0);
		}
		*/

		// Setting up emissive texture
		// note: not putting it inside the if(bHasEmissiveTexture) - 
		// there may be desire of not having emissive 1 but wanting emissive 2
		// and not having to specify emissive 1 as black in the vmt.
	//	if (bHasEmissiveTexture2)
	//	{
	//		BindTexture(SAMPLER_EMISSIVE2, info.emissiveTexture2, 0);
	//	}
	//	else
	//	{
	//		pShaderAPI->BindStandardTexture(SAMPLER_EMISSIVE2, TEXTURE_BLACK);
	//	}

		// Setting up emissive scale
		Vector EmissiveTint;
		float EmissiveTintAndScale[4] = { 1.0, 1.0, 1.0, 1.5 };
		if (bHasEmissiveTint)
		{
			params[info.emissiveTint]->GetVecValue(EmissiveTint.Base(), 3);
		}
		else
		{
			EmissiveTint.Init(1.0f, 1.0f, 1.0f);
		}
		EmissiveTintAndScale[0] = EmissiveTint.x;
		EmissiveTintAndScale[1] = EmissiveTint.y;
		EmissiveTintAndScale[2] = EmissiveTint.z;
		EmissiveTintAndScale[3] = 1.0f;

		pShaderAPI->SetPixelShaderConstant(2, EmissiveTintAndScale);

		// Setting up normal map
		if (bHasNormalTexture)
		{
			BindTexture(SAMPLER_NORMAL, info.bumpMap, 0);
			if( bIsBlended && bHasNormalTexture2)
				BindTexture(SAMPLER_NORMAL2, info.bumpMap2, info.bumpFrame2);
		}
		else
		{
			pShaderAPI->BindStandardTexture(SAMPLER_NORMAL, TEXTURE_NORMALMAP_FLAT);
			if (bIsBlended && bHasNormalTexture2) // weird scenario
				BindTexture(SAMPLER_NORMAL2, TEXTURE_NORMALMAP_FLAT);
		}

		// Setting up mro map
		if (bHasMroTexture)
		{
			BindTexture(SAMPLER_MRO, info.mroTexture, info.mroFrame);
			if (bIsBlended && bHasMroTexture2)
				BindTexture(SAMPLER_MRO2, info.mroTexture2, info.mroFrame2);
		}
		else
		{
			pShaderAPI->BindStandardTexture(SAMPLER_MRO, TEXTURE_WHITE); // this shouldn't happen, but just to be sure
			if (bIsBlended && bHasMroTexture2) // weird scenario
				BindTexture(SAMPLER_MRO2, TEXTURE_WHITE);
		}

		// Getting the light state
		LightState_t lightState;
		pShaderAPI->GetDX9LightState(&lightState);

		// Brushes don't need ambient cubes or dynamic lights
		if (!IS_FLAG_SET(MATERIAL_VAR_MODEL) && (params[info.useEnvAmbient]->GetIntValue() == 0))
		{
			lightState.m_bAmbientLight = false;
			lightState.m_nNumLights = 0;
		}

		// Setting up the flashlight related textures and variables
		bool bFlashlightShadows = false;
		if (bHasFlashlight)
		{
			Assert(info.flashlightTexture >= 0 && info.flashlightTextureFrame >= 0);
			Assert(params[info.flashlightTexture]->IsTexture());
			BindTexture(SAMPLER_FLASHLIGHT, info.flashlightTexture, info.flashlightTextureFrame);
			VMatrix worldToTexture;
			ITexture *pFlashlightDepthTexture;
			FlashlightState_t state = pShaderAPI->GetFlashlightStateEx(worldToTexture, &pFlashlightDepthTexture);
			bFlashlightShadows = state.m_bEnableShadows && (pFlashlightDepthTexture != NULL);

			SetFlashLightColorFromState(state, pShaderAPI, PSREG_FLASHLIGHT_COLOR);

			if (pFlashlightDepthTexture && g_pConfig->ShadowDepthTexture() && state.m_bEnableShadows)
			{
				BindTexture(SAMPLER_SHADOWDEPTH, pFlashlightDepthTexture, 0);
				pShaderAPI->BindStandardTexture(SAMPLER_RANDOMROTATION, TEXTURE_SHADOW_NOISE_2D);
			}
		}

		// Getting fog info
		MaterialFogMode_t fogType = pShaderAPI->GetSceneFogMode();
		int fogIndex = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z) ? 1 : 0;

		// Getting skinning info
		int numBones = pShaderAPI->GetCurrentNumBones();

		// Some debugging stuff
		bool bWriteDepthToAlpha = false;
		bool bWriteWaterFogToAlpha = false;
		if (bFullyOpaque)
		{
			bWriteDepthToAlpha = pShaderAPI->ShouldWriteDepthToDestAlpha();
			bWriteWaterFogToAlpha = (fogType == MATERIAL_FOG_LINEAR_BELOW_FOG_Z);
			AssertMsg(!(bWriteDepthToAlpha && bWriteWaterFogToAlpha),
				"Can't write two values to alpha at the same time.");
		}

		float vEyePos_SpecExponent[4];
		pShaderAPI->GetWorldSpaceCameraPosition(vEyePos_SpecExponent);

		// Determining the max level of detail for the envmap
		int iEnvMapLOD = 6;
		auto envTexture = params[info.envMap]->GetTextureValue();
		if (envTexture)
		{
			// Get power of 2 of texture width
			int width = envTexture->GetMappingWidth();
			int mips = 0;
			while (width >>= 1)
				++mips;

			// Cubemap has 4 sides so 2 mips less
			iEnvMapLOD = mips;
		}

		// Dealing with very high and low resolution cubemaps
		if (iEnvMapLOD > 12)
			iEnvMapLOD = 12;
		if (iEnvMapLOD < 4)
			iEnvMapLOD = 4;

		// This has some spare space
		vEyePos_SpecExponent[3] = iEnvMapLOD;
		pShaderAPI->SetPixelShaderConstant(PSREG_EYEPOS_SPEC_EXPONENT, vEyePos_SpecExponent, 1);

		// Setting lightmap texture
		s_pShaderAPI->BindStandardTexture(SAMPLER_LIGHTMAP, TEXTURE_LIGHTMAP_BUMPED);

		if (!g_pHardwareConfig->SupportsShaderModel_3_0() || mat_pbr_force_20b.GetBool())
		{
			// Setting up dynamic vertex shader
			DECLARE_DYNAMIC_VERTEX_SHADER(pbr_vs20b);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(DOWATERFOG, fogIndex);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, numBones > 0);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(LIGHTING_PREVIEW, pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING) != 0);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, (int)vertexCompression);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
			SET_DYNAMIC_VERTEX_SHADER(pbr_vs20b);

			// Setting up dynamic pixel shader
			DECLARE_DYNAMIC_PIXEL_SHADER(pbr_ps20b);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo());
			SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
			SET_DYNAMIC_PIXEL_SHADER(pbr_ps20b);
		}
		else
		{
			// Setting up dynamic vertex shader
			DECLARE_DYNAMIC_VERTEX_SHADER(pbr_vs30);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(DOWATERFOG, fogIndex);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(SKINNING, numBones > 0);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(LIGHTING_PREVIEW, pShaderAPI->GetIntRenderingParameter(INT_RENDERPARM_ENABLE_FIXED_LIGHTING) != 0);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(COMPRESSED_VERTS, (int)vertexCompression);
			SET_DYNAMIC_VERTEX_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
			SET_DYNAMIC_VERTEX_SHADER(pbr_vs30);

			// Setting up dynamic pixel shader
			DECLARE_DYNAMIC_PIXEL_SHADER(pbr_ps30);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(NUM_LIGHTS, lightState.m_nNumLights);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITEWATERFOGTODESTALPHA, bWriteWaterFogToAlpha);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(WRITE_DEPTH_TO_DESTALPHA, bWriteDepthToAlpha);
			SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo());
			SET_DYNAMIC_PIXEL_SHADER_COMBO(FLASHLIGHTSHADOWS, bFlashlightShadows);
			SET_DYNAMIC_PIXEL_SHADER(pbr_ps30);
		}

		// Setting up base texture transform
		SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, info.baseTextureTransform);

		//	if( bIsBlended )
		//		SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_8, info.baseTexture2Transform);

		if (bHasAccumulateTexture && g_pHardwareConfig->SupportsShaderModel_3_0() && !mat_pbr_force_20b.GetBool())
			SetVertexShaderTextureTransform(VERTEX_SHADER_SHADER_SPECIFIC_CONST_10, info.accumulateTextureTransform);

		// This is probably important
		SetModulationPixelShaderDynamicState_LinearColorSpace(PSREG_DIFFUSE_MODULATION);

		// Send ambient cube to the pixel shader, force to black if not available
		pShaderAPI->SetPixelShaderStateAmbientLightCube(PSREG_AMBIENT_CUBE, !lightState.m_bAmbientLight);

		// Send lighting array to the pixel shader
		pShaderAPI->CommitPixelShaderLighting(PSREG_LIGHT_INFO_ARRAY);

		// Handle mat_fullbright 2 (diffuse lighting only)
		if (bLightingOnly)
		{
			pShaderAPI->BindStandardTexture(SAMPLER_BASETEXTURE, TEXTURE_GREY); // Basecolor
		}

		// Handle mat_specular 0 (no envmap reflections)
		if (!mat_specular.GetBool())
		{
			pShaderAPI->BindStandardTexture(SAMPLER_ENVMAP, TEXTURE_BLACK); // Envmap
		}

		// Sending fog info to the pixel shader
		pShaderAPI->SetPixelShaderFogParams(PSREG_FOG_PARAMS);

		// Set up shader modulation color
		float modulationColor[4] = { 1.0, 1.0, 1.0, 1.0 };
		ComputeModulationColor(modulationColor);
		float flLScale = pShaderAPI->GetLightMapScaleFactor();
		modulationColor[0] *= flLScale;
		modulationColor[1] *= flLScale;
		modulationColor[2] *= flLScale;
		pShaderAPI->SetPixelShaderConstant(PSREG_DIFFUSE_MODULATION, modulationColor);

		// MRO tweaks
		float MROtint[3] = { 1.0, 1.0, 1.0 };
		params[info.MROtint]->GetVecValue(MROtint, 3);
		float MROTintReplace = params[info.MROtintReplace]->GetFloatValue();
		float fMROTint[4] = { MROtint[0], MROtint[1], MROtint[2], MROTintReplace };
		pShaderAPI->SetPixelShaderConstant(27, fMROTint);

		// More flashlight related stuff
		if (bHasFlashlight)
		{
			VMatrix worldToTexture;
			float atten[4], pos[4], tweaks[4];

			const FlashlightState_t &flashlightState = pShaderAPI->GetFlashlightState(worldToTexture);
			SetFlashLightColorFromState(flashlightState, pShaderAPI, PSREG_FLASHLIGHT_COLOR);

			BindTexture(SAMPLER_FLASHLIGHT, flashlightState.m_pSpotlightTexture, flashlightState.m_nSpotlightTextureFrame);

			// Set the flashlight attenuation factors
			atten[0] = flashlightState.m_fConstantAtten;
			atten[1] = flashlightState.m_fLinearAtten;
			atten[2] = flashlightState.m_fQuadraticAtten;
			atten[3] = flashlightState.m_FarZ;
			pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_ATTENUATION, atten, 1);

			// Set the flashlight origin
			pos[0] = flashlightState.m_vecLightOrigin[0];
			pos[1] = flashlightState.m_vecLightOrigin[1];
			pos[2] = flashlightState.m_vecLightOrigin[2];
			pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_POSITION_RIM_BOOST, pos, 1);

			pShaderAPI->SetPixelShaderConstant(PSREG_FLASHLIGHT_TO_WORLD_TEXTURE, worldToTexture.Base(), 4);

			// Tweaks associated with a given flashlight
			tweaks[0] = ShadowFilterFromState(flashlightState);
			tweaks[1] = ShadowAttenFromState(flashlightState);
			HashShadow2DJitter(flashlightState.m_flShadowJitterSeed, &tweaks[2], &tweaks[3]);
			pShaderAPI->SetPixelShaderConstant(PSREG_ENVMAP_TINT__SHADOW_TWEAKS, tweaks, 1);
		}
		else
		{
			float fEnvMapSaturation = 1;
			fEnvMapSaturation = params[info.envMapSaturation]->GetFloatValue();
			float fEnvMapContrast = 0;
			fEnvMapContrast = params[info.envMapContrast]->GetFloatValue();

			if (!params[info.envMapSaturation]->GetFloatValue() != 0)
			{
				fEnvMapSaturation = 1.0f;
			}

			// Packing to fit it into 32 constant registers.
			float fEnvMapTint[3] = { 0, 0, 0 };
			params[info.envMapTint]->GetVecValue(fEnvMapTint, 3);
			// floor makes sure that envmapcontrast values below 0.01 doesn't affect the saturation.
			float fCombine[4] = { fEnvMapTint[0], fEnvMapTint[1], fEnvMapTint[2], (floor(fEnvMapContrast * 100) * 0.01) + (fEnvMapSaturation * 0.01) };

			pShaderAPI->SetPixelShaderConstant(31, fCombine);
		}
	}

	// Actually draw the shader
	Draw();
};

// Closing it off
END_SHADER;