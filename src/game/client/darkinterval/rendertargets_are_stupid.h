#ifndef RENDERTARGETS_ARE_STUPID_H
#define RENDERTARGETS_ARE_STUPID_H

#include "baseclientrendertargets.h"

class IMaterialSystem;
class IMaterialSystemHardwareConfig;

class CDIRenderTargets : public CBaseClientRenderTargets
{
	DECLARE_CLASS_GAMEROOT(CDIRenderTargets, CBaseClientRenderTargets);

public:
	virtual void InitClientRenderTargets(IMaterialSystem *pMaterialSystem, IMaterialSystemHardwareConfig *pHardwareConfig);
	virtual void ShutdownClientRenderTargets();

	ITexture *GetDICameraTexture();
	ITexture *GetDICameraTexture2();

	ITexture *GetDIWaterReflectionTexture();
	ITexture *GetDIWaterRefractionTexture();

	ITexture *GetDIWaterReflectionTexture_Downscaled();

	ITexture *GetTestHUDTexture();
	ITexture *GetHUDLeftTexture();
	ITexture *GetHUDRightTexture();
#ifdef PORTAL_COMPILE
	ITexture* GetPortal1Texture(void);
	ITexture* GetPortal2Texture(void);
	ITexture* GetDepthDoublerTexture(void);

	//recursive views require different water textures
	ITexture* GetWaterReflectionTextureForStencilDepth(int iStencilDepth);
	ITexture* GetWaterRefractionTextureForStencilDepth(int iStencilDepth);
#endif

private:
	CTextureReference m_DICameraTexture;
	CTextureReference m_DICameraTexture2;
	CTextureReference m_DIWaterReflectionTexture;
	CTextureReference m_DIWaterRefractionTexture;

	CTextureReference m_DIWaterReflectionTexture_Downscaled;

	CTextureReference m_TestHUDTexture;
	CTextureReference m_HUDLeftTexture;
	CTextureReference m_HUDRightTexture;
#ifdef PORTAL_COMPILE
	CTextureReference m_Portal1Texture;
	CTextureReference m_Portal2Texture;
	CTextureReference m_DepthDoublerTexture;

	CTextureReference m_WaterRefractionTextures[2];
	CTextureReference m_WaterReflectionTextures[2];
#endif
};

extern CDIRenderTargets *g_pDIRenderTargets;

#endif

#if 0
#define RENDERTARGETS_ARE_STUPID_H

#include "baseclientrendertargets.h"

// Externs
class IMaterialSystem;
class IMaterialSystemHardwareConfig;

class CMyRenderTargets : public CBaseClientRenderTargets
{
	DECLARE_CLASS_GAMEROOT(CMyRenderTargets, CBaseClientRenderTargets);
public:
	virtual void InitClientRenderTargets(IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig, int iWaterTextureSize = 1024, int iCameraTextureSize = 256);
	virtual void ShutdownClientRenderTargets(void);

	ITexture *GetPortal1Texture();
	ITexture *GetPortal2Texture();

private:
	CTextureReference m_Portal1Texture;
	CTextureReference m_Portal2Texture;
};

extern CMyRenderTargets *g_pMyRenderTargets;

#endif
