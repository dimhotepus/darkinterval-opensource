#include "cbase.h"
#include "rendertargets_are_stupid.h"
#include "materialsystem/imaterialsystem.h"
#include "rendertexture.h"

void CDIRenderTargets::InitClientRenderTargets(IMaterialSystem *pMaterialSystem, IMaterialSystemHardwareConfig *pHardwareConfig)
{
	m_DICameraTexture.Init(pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_Camera_DI", 
		1024, 512, 
		RT_SIZE_DEFAULT,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SHARED,
		0,
		CREATERENDERTARGETFLAGS_HDR));
	/*
		1024, 1024, 
		RT_SIZE_FULL_FRAME_BUFFER,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SHARED,
		0,
		CREATERENDERTARGETFLAGS_HDR));
	*/
	m_DICameraTexture2.Init(pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_Camera_DI2",
		1, 1, RT_SIZE_FULL_FRAME_BUFFER,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SHARED,
		0,
		CREATERENDERTARGETFLAGS_HDR));

	m_DIWaterReflectionTexture.Init(pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_WaterReflection_DI",
		512, 512, 
		RT_SIZE_PICMIP,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SHARED,
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
		CREATERENDERTARGETFLAGS_HDR));

	m_DIWaterRefractionTexture.Init(pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_WaterRefraction_DI",
		512, 512, RT_SIZE_PICMIP,
		IMAGE_FORMAT_RGBA8888,
		MATERIAL_RT_DEPTH_SHARED,
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
		CREATERENDERTARGETFLAGS_HDR));

	m_DIWaterReflectionTexture_Downscaled.Init(pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_WaterReflection_DI_Downscaled",
		256, 256, RT_SIZE_PICMIP,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SHARED,
		TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
		CREATERENDERTARGETFLAGS_HDR));

#ifdef PORTAL_COMPILE
	m_Portal1Texture.Init(pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_Portal1",
		1, 1, RT_SIZE_FULL_FRAME_BUFFER,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SHARED,
		0,
		CREATERENDERTARGETFLAGS_HDR));

	m_Portal2Texture.Init(pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_Portal2",
		1, 1, RT_SIZE_FULL_FRAME_BUFFER,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SHARED,
		0,
		CREATERENDERTARGETFLAGS_HDR));

	m_DepthDoublerTexture.Init(pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_DepthDoubler",
		512, 512, RT_SIZE_DEFAULT,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_SHARED,
		0,
		CREATERENDERTARGETFLAGS_HDR));

	m_WaterReflectionTextures[0].Init(
		pMaterialSystem->CreateNamedRenderTargetTextureEx2(
			"_rt_PortalWaterReflection_Depth1",
			512, 512, RT_SIZE_PICMIP,
			pMaterialSystem->GetBackBufferFormat(),
			MATERIAL_RT_DEPTH_SEPARATE,
			TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
			CREATERENDERTARGETFLAGS_HDR));

	m_WaterReflectionTextures[1].Init(
		pMaterialSystem->CreateNamedRenderTargetTextureEx2(
			"_rt_PortalWaterReflection_Depth2",
			256, 256, RT_SIZE_PICMIP,
			pMaterialSystem->GetBackBufferFormat(),
			MATERIAL_RT_DEPTH_SEPARATE,
			TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
			CREATERENDERTARGETFLAGS_HDR));
	
	//Refractions
	m_WaterRefractionTextures[0].Init(
		pMaterialSystem->CreateNamedRenderTargetTextureEx2(
			"_rt_PortalWaterRefraction_Depth1",
			512, 512, RT_SIZE_PICMIP,
			// This is different than reflection because it has to have alpha for fog factor.
			IMAGE_FORMAT_RGBA8888,
			MATERIAL_RT_DEPTH_SEPARATE,
			TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
			CREATERENDERTARGETFLAGS_HDR));

	m_WaterRefractionTextures[1].Init(
		pMaterialSystem->CreateNamedRenderTargetTextureEx2(
			"_rt_PortalWaterRefraction_Depth2",
			256, 256, RT_SIZE_PICMIP,
			// This is different than reflection because it has to have alpha for fog factor.
			IMAGE_FORMAT_RGBA8888,
			MATERIAL_RT_DEPTH_SEPARATE,
			TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
			CREATERENDERTARGETFLAGS_HDR));
#endif
	
	m_TestHUDTexture.Init(pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"_rt_testhud",
		512, 512, RT_SIZE_FULL_FRAME_BUFFER,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_NONE,
		0,
		CREATERENDERTARGETFLAGS_HDR));

	m_HUDLeftTexture.Init(pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"__rt_hud_left",
		4096, 4096, RT_SIZE_FULL_FRAME_BUFFER,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_NONE,
		0,
		CREATERENDERTARGETFLAGS_HDR));

	m_HUDRightTexture.Init(pMaterialSystem->CreateNamedRenderTargetTextureEx2(
		"__rt_hud_right",
		4096, 4096, RT_SIZE_FULL_FRAME_BUFFER,
		pMaterialSystem->GetBackBufferFormat(),
		MATERIAL_RT_DEPTH_NONE,
		0,
		CREATERENDERTARGETFLAGS_HDR));

	BaseClass::InitClientRenderTargets(pMaterialSystem, pHardwareConfig);
}

void CDIRenderTargets::ShutdownClientRenderTargets()
{
	m_DICameraTexture.Shutdown();
	m_DICameraTexture2.Shutdown();
	m_DIWaterReflectionTexture.Shutdown();
	m_DIWaterRefractionTexture.Shutdown();
	m_DIWaterReflectionTexture_Downscaled.Shutdown();
	m_TestHUDTexture.Shutdown();

#ifdef PORTAL_COMPILE
	m_Portal1Texture.Shutdown();
	m_Portal2Texture.Shutdown();
	m_DepthDoublerTexture.Shutdown();

	for (int i = 0; i < 2; ++i)
	{
		m_WaterReflectionTextures[i].Shutdown();
		m_WaterRefractionTextures[i].Shutdown();
	}
#endif

	BaseClass::ShutdownClientRenderTargets();
}

ITexture *CDIRenderTargets::GetDICameraTexture()
{
	return m_DICameraTexture;
}

ITexture *CDIRenderTargets::GetDICameraTexture2()
{
	return m_DICameraTexture2;
}

ITexture *CDIRenderTargets::GetDIWaterReflectionTexture()
{
	return m_DIWaterReflectionTexture;
}

ITexture *CDIRenderTargets::GetDIWaterRefractionTexture()
{
	return m_DIWaterRefractionTexture;
}

ITexture *CDIRenderTargets::GetDIWaterReflectionTexture_Downscaled()
{
	return m_DIWaterReflectionTexture_Downscaled;
}

#ifdef PORTAL_COMPILE
ITexture *CDIRenderTargets::GetPortal1Texture()
{
	return m_Portal1Texture;
}

ITexture *CDIRenderTargets::GetPortal2Texture()
{
	return m_Portal2Texture;
}

ITexture *CDIRenderTargets::GetDepthDoublerTexture()
{
	return m_DepthDoublerTexture;
}

ITexture* CDIRenderTargets::GetWaterReflectionTextureForStencilDepth(int iStencilDepth)
{
	if (iStencilDepth > 2)
		return NULL;

	if (iStencilDepth == 0)
		return m_WaterReflectionTexture; //from CBaseClientRenderTargets

	return m_WaterReflectionTextures[iStencilDepth - 1];
}

ITexture* CDIRenderTargets::GetWaterRefractionTextureForStencilDepth(int iStencilDepth)
{
	if (iStencilDepth > 2)
		return NULL;

	if (iStencilDepth == 0)
		return m_WaterRefractionTexture; //from CBaseClientRenderTargets

	return m_WaterRefractionTextures[iStencilDepth - 1];
}
#endif

ITexture *CDIRenderTargets::GetTestHUDTexture()
{
	return m_TestHUDTexture;
}

ITexture *CDIRenderTargets::GetHUDLeftTexture()
{
	return m_HUDLeftTexture;
}

ITexture *CDIRenderTargets::GetHUDRightTexture()
{
	return m_HUDRightTexture;
}

static CDIRenderTargets g_DIRenderTargets;
EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CDIRenderTargets, IClientRenderTargets, CLIENTRENDERTARGETS_INTERFACE_VERSION, g_DIRenderTargets);
CDIRenderTargets *g_pDIRenderTargets = &g_DIRenderTargets;


#if 0

#include "cbase.h"
#include "rendertargets_are_stupid.h"
#include "materialsystem/imaterialsystem.h"
#include "rendertexture.h"

void CMyRenderTargets::InitClientRenderTargets(IMaterialSystem* pMaterialSystem, IMaterialSystemHardwareConfig* pHardwareConfig, int iWaterTextureSize, int iCameraTextureSize)
{
	Msg("Initialised rendertarget _rt_Portal1 & 2\n");
	// Portals 
	m_Portal1Texture.Init(
		pMaterialSystem->CreateNamedRenderTargetTextureEx2(
			"_rt_Portal1",
			1024, 1024, RT_SIZE_OFFSCREEN,
			pMaterialSystem->GetBackBufferFormat(),
			MATERIAL_RT_DEPTH_SHARED,
			TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
			CREATERENDERTARGETFLAGS_HDR)
	);

	m_Portal2Texture.Init(
		pMaterialSystem->CreateNamedRenderTargetTextureEx2(
			"_rt_Portal2",
			1, 1, RT_SIZE_FULL_FRAME_BUFFER,
			pMaterialSystem->GetBackBufferFormat(),
			MATERIAL_RT_DEPTH_SHARED,
			0,
			CREATERENDERTARGETFLAGS_HDR)
	);

	BaseClass::InitClientRenderTargets(pMaterialSystem, pHardwareConfig, 1024, 1024);
}

ITexture* CMyRenderTargets::GetPortal1Texture()
{
	return m_Portal1Texture;
}

ITexture* CMyRenderTargets::GetPortal2Texture()
{
	return m_Portal2Texture;
}

void CMyRenderTargets::ShutdownClientRenderTargets()
{
	m_Portal1Texture.Shutdown();
	m_Portal2Texture.Shutdown();

	BaseClass::ShutdownClientRenderTargets();
}

static CMyRenderTargets g_MyRenderTargets;

EXPOSE_SINGLE_INTERFACE_GLOBALVAR(CMyRenderTargets, CBaseClientRenderTargets,
	CLIENTRENDERTARGETS_INTERFACE_VERSION, g_MyRenderTargets);

CMyRenderTargets *g_pMyRenderTargets = &g_MyRenderTargets;

#endif