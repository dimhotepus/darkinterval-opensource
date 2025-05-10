//===================== Copyright (c) Valve Corporation. All Rights Reserved. ======================
//
// Example shader that can be applied to models
//
//==================================================================================================

#ifndef FOLIAGE_DX9_HELPER_H
#define FOLIAGE_DX9_HELPER_H

#include <string.h>

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class CBaseVSShader;
class IMaterialVar;
class IShaderDynamicAPI;
class IShaderShadow;

//-----------------------------------------------------------------------------
// Init params/ init/ draw methods
//-----------------------------------------------------------------------------
struct Foliage_DX9_Vars_t
{
	Foliage_DX9_Vars_t() { memset( this, 0xFF, sizeof(*this) ); }

	int m_nBaseTexture;
	int m_nBaseTextureFrame;
	int m_nBaseTextureTransform;
	int m_nAlphaTestReference;
	int m_nFlashlightTexture;
	int m_nFlashlightTextureFrame;

	int m_nTreeSway;
	int m_nTreeSwayHeight;
	int m_nTreeSwayStartHeight;
	int m_nTreeSwayRadius;
	int m_nTreeSwayStartRadius;
	int m_nTreeSwaySpeed;
	int m_nTreeSwaySpeedHighWindMultiplier;
	int m_nTreeSwayStrength;
	int m_nTreeSwayScrumbleSpeed;
	int m_nTreeSwayScrumbleStrength;
	int m_nTreeSwayScrumbleFrequency;
	int m_nTreeSwayFalloffExp;
	int m_nTreeSwayScrumbleFalloffExp;
	int m_nTreeSwaySpeedLerpStart;
	int m_nTreeSwaySpeedLerpEnd;
};

void InitParamsFoliage_DX9( CBaseVSShader *pShader, IMaterialVar** params,
						 const char *pMaterialName, Foliage_DX9_Vars_t &info );

void InitFoliage_DX9( CBaseVSShader *pShader, IMaterialVar** params, 
	Foliage_DX9_Vars_t &info );

void DrawFoliage_DX9( CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI,
				   IShaderShadow* pShaderShadow,
	Foliage_DX9_Vars_t &info, VertexCompressionType_t vertexCompression,
				   CBasePerMaterialContextData **pContextDataPtr );

#endif // FOLIAGE_DX9_HELPER_H
