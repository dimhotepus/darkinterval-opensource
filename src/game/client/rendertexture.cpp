//========================================================================//
//
// Purpose: 
// Implements local hooks into named renderable textures.
// See matrendertexture.cpp in material system for list of available RT's
//
//=============================================================================//

#include "materialsystem/imesh.h"
#include "materialsystem/itexture.h"
#include "materialsystem/MaterialSystemUtil.h"
#include "tier1/strtools.h"
#include "rendertexture.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void ReleaseRenderTargets( void );

void AddReleaseFunc( void )
{
	static bool bAdded = false;
	if( !bAdded )
	{
		bAdded = true;
		materials->AddReleaseFunc( ReleaseRenderTargets );
	}
}

//=============================================================================
// Power of Two Frame Buffer Texture
//=============================================================================
static CTextureReference s_pPowerOfTwoFrameBufferTexture;
ITexture *GetPowerOfTwoFrameBufferTexture( void )
{
	if ( !s_pPowerOfTwoFrameBufferTexture )
	{
		s_pPowerOfTwoFrameBufferTexture.Init( materials->FindTexture( "_rt_PowerOfTwoFB", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_pPowerOfTwoFrameBufferTexture ) );
		AddReleaseFunc();
	}
	
	return s_pPowerOfTwoFrameBufferTexture;
}

//=============================================================================
// Fullscreen Texture
//=============================================================================
static CTextureReference s_pFullscreenTexture;
ITexture *GetFullscreenTexture( void )
{
	if ( !s_pFullscreenTexture )
	{
		s_pFullscreenTexture.Init( materials->FindTexture( "_rt_Fullscreen", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_pFullscreenTexture ) );
		AddReleaseFunc();
	}

	return s_pFullscreenTexture;
}

//=============================================================================
// Camera Texture
//=============================================================================
static CTextureReference s_pCameraTexture;
ITexture *GetCameraTexture( void )
{
	if ( !s_pCameraTexture )
	{
		s_pCameraTexture.Init( materials->FindTexture( "_rt_Camera", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_pCameraTexture ) );
		AddReleaseFunc();
	}
	
	return s_pCameraTexture;
}

//=============================================================================
// Full Frame Depth Texture
//=============================================================================
static CTextureReference s_pFullFrameDepthTexture;
ITexture *GetFullFrameDepthTexture( void )
{
	if ( !s_pFullFrameDepthTexture )
	{
		s_pFullFrameDepthTexture.Init( materials->FindTexture("_rt_FullFrameDepth", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_pFullFrameDepthTexture ) );
		AddReleaseFunc();
	}

	return s_pFullFrameDepthTexture;
}

//=============================================================================
// Full Frame Buffer Textures
//=============================================================================
static CTextureReference s_pFullFrameFrameBufferTexture[MAX_FB_TEXTURES];
ITexture *GetFullFrameFrameBufferTexture( int textureIndex )
{
	if ( !s_pFullFrameFrameBufferTexture[textureIndex] )
	{
		char name[256];
		if( textureIndex != 0 )
		{
			sprintf( name, "_rt_FullFrameFB%d", textureIndex );
		}
		else
		{
			Q_strcpy( name, "_rt_FullFrameFB" );
		}
		s_pFullFrameFrameBufferTexture[textureIndex].Init( materials->FindTexture( name, TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_pFullFrameFrameBufferTexture[textureIndex] ) );
		AddReleaseFunc();
	}
	
	return s_pFullFrameFrameBufferTexture[textureIndex];
}


//=============================================================================
// Water reflection
//=============================================================================
static CTextureReference s_pWaterReflectionTexture;
ITexture *GetWaterReflectionTexture( void )
{
	if ( !s_pWaterReflectionTexture )
	{
		s_pWaterReflectionTexture.Init( materials->FindTexture( "_rt_WaterReflection", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_pWaterReflectionTexture ) );
		AddReleaseFunc();
	}
	
	return s_pWaterReflectionTexture;
}

//=============================================================================
// Water refraction
//=============================================================================
static CTextureReference s_pWaterRefractionTexture;
ITexture *GetWaterRefractionTexture( void )
{
	if ( !s_pWaterRefractionTexture )
	{
		s_pWaterRefractionTexture.Init( materials->FindTexture( "_rt_WaterRefraction", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_pWaterRefractionTexture ) );
		AddReleaseFunc();
	}
	
	return s_pWaterRefractionTexture;
}

//=============================================================================
// Small Buffer HDR0
//=============================================================================
static CTextureReference s_pSmallBufferHDR0;
ITexture *GetSmallBufferHDR0( void )
{
	if ( !s_pSmallBufferHDR0 )
	{
		s_pSmallBufferHDR0.Init( materials->FindTexture( "_rt_SmallHDR0", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_pSmallBufferHDR0 ) );
		AddReleaseFunc();
	}
	
	return s_pSmallBufferHDR0;
}

//=============================================================================
// Small Buffer HDR1
//=============================================================================
static CTextureReference s_pSmallBufferHDR1;
ITexture *GetSmallBufferHDR1( void )
{
	if ( !s_pSmallBufferHDR1 )
	{
		s_pSmallBufferHDR1.Init( materials->FindTexture( "_rt_SmallHDR1", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_pSmallBufferHDR1 ) );
		AddReleaseFunc();
	}
	
	return s_pSmallBufferHDR1;
}

//=============================================================================
// Quarter Sized FB0
//=============================================================================
static CTextureReference s_pQuarterSizedFB0;
ITexture *GetSmallBuffer0( void )
{
	if ( !s_pQuarterSizedFB0 )
	{
		s_pQuarterSizedFB0.Init( materials->FindTexture( "_rt_SmallFB0", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_pQuarterSizedFB0 ) );
		AddReleaseFunc();
	}
	
	return s_pQuarterSizedFB0;
}

//=============================================================================
// Quarter Sized FB1
//=============================================================================
static CTextureReference s_pQuarterSizedFB1;
ITexture *GetSmallBuffer1( void )
{
	if ( !s_pQuarterSizedFB1 )
	{
		s_pQuarterSizedFB1.Init( materials->FindTexture( "_rt_SmallFB1", TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_pQuarterSizedFB1 ) );
		AddReleaseFunc();
	}
	
	return s_pQuarterSizedFB1;
}

//=============================================================================
// Teeny Textures
//=============================================================================
static CTextureReference s_TeenyTextures[MAX_TEENY_TEXTURES];
ITexture *GetTeenyTexture( int which )
{
	Assert( which < MAX_TEENY_TEXTURES );

	if ( !s_TeenyTextures[which] )
	{
		char nbuf[20];
		sprintf( nbuf, "_rt_TeenyFB%d", which );
		s_TeenyTextures[which].Init( materials->FindTexture( nbuf, TEXTURE_GROUP_RENDER_TARGET ) );
		Assert( !IsErrorTexture( s_TeenyTextures[which] ) );
		AddReleaseFunc();
	}
	return s_TeenyTextures[which];
}
#ifdef DARKINTERVAL
//=============================================================================
// Custom Textures
//=============================================================================
static CTextureReference s_pDICameraTexture;
ITexture *GetDICameraTexture(void)
{
	if (!s_pDICameraTexture)
	{
		s_pDICameraTexture.Init(materials->FindTexture("_rt_Camera_DI", TEXTURE_GROUP_RENDER_TARGET));
		Assert(!IsErrorTexture(s_pDICameraTexture));
		Msg("_rt_Camera_DI created\n");
		AddReleaseFunc();
	}

	return s_pDICameraTexture;
}

static CTextureReference s_pDICameraTexture2;
ITexture *GetDICameraTexture2(void)
{
	if (!s_pDICameraTexture2)
	{
		s_pDICameraTexture2.Init(materials->FindTexture("_rt_Camera_DI2", TEXTURE_GROUP_RENDER_TARGET));
		Assert(!IsErrorTexture(s_pDICameraTexture2));
		AddReleaseFunc();
	}

	return s_pDICameraTexture2;
}

static CTextureReference s_pDIWaterReflectionTexture;
ITexture *GetDIWaterReflectionTexture(void)
{
	if (!s_pDIWaterReflectionTexture)
	{
		s_pDIWaterReflectionTexture.Init(materials->FindTexture("_rt_WaterReflection_DI", TEXTURE_GROUP_RENDER_TARGET));
		Assert(!IsErrorTexture(s_pDIWaterReflectionTexture));
		AddReleaseFunc();
	}

	return s_pDIWaterReflectionTexture;
}

static CTextureReference s_pDIWaterRefractionTexture;
ITexture *GetDIWaterRefractionTexture(void)
{
	if (!s_pDIWaterRefractionTexture)
	{
		s_pDIWaterRefractionTexture.Init(materials->FindTexture("_rt_WaterRefraction_DI", TEXTURE_GROUP_RENDER_TARGET));
		Assert(!IsErrorTexture(s_pDIWaterRefractionTexture));
		AddReleaseFunc();
	}

	return s_pDIWaterRefractionTexture;
}

static CTextureReference s_pDIWaterReflectionTexture_Downscaled;
ITexture *GetDIWaterReflectionTexture_Downscaled(void)
{
	if (!s_pDIWaterReflectionTexture_Downscaled)
	{
		s_pDIWaterReflectionTexture_Downscaled.Init(materials->FindTexture("_rt_WaterReflection_DI_Downscaled", TEXTURE_GROUP_RENDER_TARGET));
		Assert(!IsErrorTexture(s_pDIWaterReflectionTexture_Downscaled));
		AddReleaseFunc();
	}

	return s_pDIWaterReflectionTexture_Downscaled;
}

// for drawing hud into models
static CTextureReference s_pTestHUDTexture;
ITexture *GetTestHUDTexture(void)
{
	if (!s_pTestHUDTexture)
	{
		s_pTestHUDTexture.Init(materials->FindTexture("_rt_testhud", TEXTURE_GROUP_RENDER_TARGET));
		Assert(!IsErrorTexture(s_pTestHUDTexture));
	//	Msg("_rt_testhud created\n");
		AddReleaseFunc();
	}

	return s_pTestHUDTexture;
}

static CTextureReference s_pHUDLeftTexture;
ITexture *GetHUDLeftTexture(void)
{
	if (!s_pHUDLeftTexture)
	{
		s_pHUDLeftTexture.Init(materials->FindTexture("__rt_hud_left", TEXTURE_GROUP_RENDER_TARGET));
		Assert(!IsErrorTexture(s_pHUDLeftTexture));
	//	Msg("__rt_hud_left created\n");
		AddReleaseFunc();
	}

	return s_pHUDLeftTexture;
}

static CTextureReference s_pHUDRightTexture;
ITexture *GetHUDRightTexture(void)
{
	if (!s_pHUDRightTexture)
	{
		s_pHUDRightTexture.Init(materials->FindTexture("__rt_hud_right", TEXTURE_GROUP_RENDER_TARGET));
		Assert(!IsErrorTexture(s_pHUDRightTexture));
	//	Msg("__rt_hud_right created\n");
		AddReleaseFunc();
	}

	return s_pHUDRightTexture;
}

void ReleaseRenderTargets( void )
{
	s_pPowerOfTwoFrameBufferTexture.Shutdown();
	s_pCameraTexture.Shutdown();
	s_pWaterReflectionTexture.Shutdown();
	s_pWaterRefractionTexture.Shutdown();
	s_pQuarterSizedFB0.Shutdown();
	s_pQuarterSizedFB1.Shutdown();
	s_pFullFrameDepthTexture.Shutdown();
	
	for (int i=0; i<MAX_FB_TEXTURES; ++i)
		s_pFullFrameFrameBufferTexture[i].Shutdown();

	s_pDICameraTexture.Shutdown();
	s_pDICameraTexture.Shutdown();
	s_pDIWaterReflectionTexture.Shutdown();
	s_pDIWaterRefractionTexture.Shutdown();
	s_pTestHUDTexture.Shutdown();
}
#endif