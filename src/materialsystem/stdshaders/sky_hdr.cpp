//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//=============================================================================
#include "BaseVSShader.h"

BEGIN_VS_SHADER( Sky_HDR, "Help for Sky shader" )

	BEGIN_SHADER_PARAMS
	END_SHADER_PARAMS

	SHADER_FALLBACK
	{
		if( g_pHardwareConfig->GetDXSupportLevel() < 90 )
		{
			return "UnlitGeneric";
		}
		return 0;
	}

	SHADER_INIT
	{
		if (params[BASETEXTURE]->IsDefined())
		{
			LoadTexture( BASETEXTURE );
		}
	}
	SHADER_DRAW
	{
		SHADOW_STATE
		{
			SetInitialShadowState();

			pShaderShadow->EnableAlphaWrites( true );
			pShaderShadow->EnableTexture(SHADER_SAMPLER0, true );
			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 1, /*NULL,*/ 0, 0 );

			pShaderShadow->SetVertexShader( "sky_hdr_vs20", 0 );
			pShaderShadow->SetPixelShader( "sky_hdr_ps20", 0 );
		}

		DYNAMIC_STATE
		{
			BindTexture(SHADER_SAMPLER0, BASETEXTURE, FRAME );
			SetVertexShaderTextureTransform( 90, BASETEXTURETRANSFORM );
		}
		Draw( );
	}

END_SHADER

