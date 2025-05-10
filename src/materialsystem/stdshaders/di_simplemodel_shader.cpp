//===================== Copyright (c) Valve Corporation. All Rights Reserved. ======================
//
// Example shader that can be applied to models
//
//==================================================================================================

#include "BaseVSShader.h"
#include "convar.h"
#include "di_simplemodel_functions.h"

BEGIN_VS_SHADER(DrawModel, "Help for Example Model Shader")

BEGIN_SHADER_PARAMS
SHADER_PARAM(ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0.0", "")
SHADER_PARAM(NORMALMAP, SHADER_PARAM_TYPE_TEXTURE, "bumpmaps/bump_flat", "normal map" )
SHADER_PARAM(NORMALMAP_ANIMATED_FRAME, SHADER_PARAM_TYPE_INTEGER, "0", "frame number for $normalmap")
SHADER_PARAM(NORMALMAP_TRANSFORM, SHADER_PARAM_TYPE_MATRIX, "center .5 .5 scale 1 1 rotate 0 translate 0 0", "$bumpmap texcoord transform")

END_SHADER_PARAMS

void SetupVars(DI_SimpleModel_Vars_t& info)
{
	info.m_nBaseTexture = BASETEXTURE;
	info.m_nBaseTextureFrame = FRAME;
	info.m_nBaseTextureTransform = BASETEXTURETRANSFORM;
	info.m_nAlphaTestReference = ALPHATESTREFERENCE;
	info.m_nFlashlightTexture = FLASHLIGHTTEXTURE;
	info.m_nFlashlightTextureFrame = FLASHLIGHTTEXTUREFRAME;
	info.m_nNormalmap = NORMALMAP;
	info.m_nNormalmapFrame = NORMALMAP_ANIMATED_FRAME;
	info.m_nNormalmapTransform = NORMALMAP_TRANSFORM;
}

SHADER_INIT_PARAMS()
{
	DI_SimpleModel_Vars_t info;
	SetupVars(info);
	DI_SimpleModel_InitParams(this, params, pMaterialName, info);
}

SHADER_FALLBACK
{
	return 0;
}

SHADER_INIT
{
	DI_SimpleModel_Vars_t info;
	SetupVars(info);
	DI_SimpleModel_InitShader(this, params, info);
}

SHADER_DRAW
{
	DI_SimpleModel_Vars_t info;
	SetupVars(info);
	DI_SimpleModel_DrawShader(this, params, pShaderAPI, pShaderShadow, info, vertexCompression, pContextDataPtr);
}

END_SHADER

