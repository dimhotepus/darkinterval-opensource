//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $Header: $
// $NoKeywords: $
//=============================================================================//
#include "BaseVSShader.h"
#include "sky_alpha_vs20.inc"
#include "sky_alpha_ps20.inc"
#include "sky_alpha_ps20b.inc"

#include "ConVar.h"

BEGIN_VS_SHADER( DI_Sky_DX90, "Help for Sky_DX9 shader" )

	BEGIN_SHADER_PARAMS
		SHADER_PARAM( BASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "tools/toolsblack", "")
		SHADER_PARAM( COLOR, SHADER_PARAM_TYPE_VEC3, "[ 1 1 1]", "color multiplier" )
		SHADER_PARAM( ALPHA, SHADER_PARAM_TYPE_FLOAT, "1.0", "unused" )
	END_SHADER_PARAMS

	SHADER_FALLBACK
	{
		if( g_pHardwareConfig->GetDXSupportLevel() < 90 )
		{
			return "sky_dx6";
		}
		return 0;
	}

	SHADER_INIT_PARAMS()
	{
		SET_FLAGS( MATERIAL_VAR_NOFOG );
		SET_FLAGS( MATERIAL_VAR_IGNOREZ );
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
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			pShaderShadow->VertexShaderVertexFormat( VERTEX_POSITION, 1, NULL, 0 );

			DECLARE_STATIC_VERTEX_SHADER( sky_alpha_vs20 );
			SET_STATIC_VERTEX_SHADER( sky_alpha_vs20 );

			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				DECLARE_STATIC_PIXEL_SHADER( sky_alpha_ps20b );
				SET_STATIC_PIXEL_SHADER( sky_alpha_ps20b );
			}
			else
			{
				DECLARE_STATIC_PIXEL_SHADER( sky_alpha_ps20 );
				SET_STATIC_PIXEL_SHADER( sky_alpha_ps20 );
			}
			// we are writing linear values from this shader.
			pShaderShadow->EnableSRGBWrite( true );

			pShaderShadow->EnableAlphaWrites( true );
		}

		DYNAMIC_STATE
		{
			BindTexture( SHADER_SAMPLER0, BASETEXTURE, FRAME );
			float c1[4]={0,0,0,0};
			pShaderAPI->SetVertexShaderConstant( VERTEX_SHADER_SHADER_SPECIFIC_CONST_0, c1);

			float c0[4]={1,1,1,1};
			if (params[COLOR]->IsDefined())
			{
				memcpy(c0,params[COLOR]->GetVecValue(),3*sizeof(float));
			}
			
			pShaderAPI->SetPixelShaderConstant(0,c0,1);
			DECLARE_DYNAMIC_VERTEX_SHADER( sky_alpha_vs20 );
			SET_DYNAMIC_VERTEX_SHADER( sky_alpha_vs20 );

			// Texture coord transform
			SetVertexShaderTextureTransform( VERTEX_SHADER_SHADER_SPECIFIC_CONST_1, BASETEXTURETRANSFORM );

			if( g_pHardwareConfig->SupportsPixelShaders_2_b() )
			{
				DECLARE_DYNAMIC_PIXEL_SHADER( sky_alpha_ps20b );
				SET_DYNAMIC_PIXEL_SHADER_COMBO( WRITE_DEPTH_TO_DESTALPHA, pShaderAPI->ShouldWriteDepthToDestAlpha() );
				SET_DYNAMIC_PIXEL_SHADER( sky_alpha_ps20b );
			}
			else
			{
				DECLARE_DYNAMIC_PIXEL_SHADER( sky_alpha_ps20 );
				SET_DYNAMIC_PIXEL_SHADER( sky_alpha_ps20 );
			}
		}
		Draw( );
	}

END_SHADER

