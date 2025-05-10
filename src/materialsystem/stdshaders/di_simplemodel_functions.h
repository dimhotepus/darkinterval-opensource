#ifndef DI_SIMPLEMODEL_FUNCTIONS_H
#define DI_SIMPLEMODEL_FUNCTIONS_H

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
struct DI_SimpleModel_Vars_t
{
	DI_SimpleModel_Vars_t() { memset(this, 0xFF, sizeof(*this)); }

	int m_nBaseTexture;
	int m_nBaseTextureFrame;
	int m_nBaseTextureTransform;
	int m_nNormalmap;
	int m_nNormalmapFrame;
	int m_nNormalmapTransform;
	int m_nAlphaTestReference;
	int m_nFlashlightTexture;
	int m_nFlashlightTextureFrame;
};

void DI_SimpleModel_InitParams(CBaseVSShader *pShader, IMaterialVar** params,
	const char *pMaterialName, DI_SimpleModel_Vars_t &info);

void DI_SimpleModel_InitShader(CBaseVSShader *pShader, IMaterialVar** params,
	DI_SimpleModel_Vars_t &info);

void DI_SimpleModel_DrawShader(CBaseVSShader *pShader, IMaterialVar** params, IShaderDynamicAPI *pShaderAPI,
	IShaderShadow* pShaderShadow,
	DI_SimpleModel_Vars_t &info, VertexCompressionType_t vertexCompression,
	CBasePerMaterialContextData **pContextDataPtr);

#endif // DI_SIMPLEMODEL_FUNCTIONS_H
