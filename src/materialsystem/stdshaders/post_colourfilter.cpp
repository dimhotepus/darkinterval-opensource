#include "BaseVSShader.h"

#include "PassThrough_vs30.inc"
#include "post_colourfilter_ps30.inc"

ConVar mat_grain_scale_override( "mat_grain_scale_override", "2.0" );//was 4.2

BEGIN_VS_SHADER( Post_Screenfilter, "" )

	BEGIN_SHADER_PARAMS
	SHADER_PARAM( SCALE,			SHADER_PARAM_TYPE_FLOAT,		"1.0", "")
	SHADER_PARAM( COLOR,			SHADER_PARAM_TYPE_COLOR,		"[1 1 1]", "")
	SHADER_PARAM( LUMINANCE,		SHADER_PARAM_TYPE_COLOR,		"[1 1 1]", "")
	SHADER_PARAM( INNER_RADIUS,		SHADER_PARAM_TYPE_FLOAT,		".1", "")
	SHADER_PARAM( OUTER_RADIUS,		SHADER_PARAM_TYPE_FLOAT,		"1.0", "")
	SHADER_PARAM( FBTEXTURE,		SHADER_PARAM_TYPE_TEXTURE,		"_rt_FullFrameFB", "" )
	SHADER_PARAM( BLUR_AMOUNT,		SHADER_PARAM_TYPE_FLOAT,		"1", "")
	END_SHADER_PARAMS

	SHADER_INIT_PARAMS()
	{
		if ( !params[SCALE]->IsDefined() )
		{
			params[SCALE]->SetFloatValue( 1.0f );
		}

		if (!params[COLOR]->IsDefined())
		{
			params[COLOR]->SetVecValue(1.0f, 1.0f, 1.0f);
		}

		if (!params[LUMINANCE]->IsDefined())
		{
			params[LUMINANCE]->SetVecValue(0.2125f, 0.7154f, 0.0721f); // previously hardcoded
		}

		if ( !params[ FBTEXTURE ]->IsDefined() )
		{
			params[ FBTEXTURE ]->SetStringValue( "_rt_FullFrameFB" );
		}

		if ( !params[ INNER_RADIUS ]->IsDefined() )
		{
			params[ INNER_RADIUS ]->SetFloatValue( 0.38f );
		}

		if ( !params[ OUTER_RADIUS ]->IsDefined() )
		{
			params[ OUTER_RADIUS ]->SetFloatValue( 0.45f );
		}

		if (!params[BLUR_AMOUNT]->IsDefined())
		{
			params[BLUR_AMOUNT]->SetFloatValue(1.0f);
		}

	}

	SHADER_FALLBACK
	{
		return 0;
	}

	SHADER_INIT
	{
		if( params[FBTEXTURE]->IsDefined() )
		{
			LoadTexture( FBTEXTURE );
		}
	}

	SHADER_DRAW
	{
		SHADOW_STATE
		{
			pShaderShadow->EnableDepthWrites( false );
			pShaderShadow->EnableTexture( SHADER_SAMPLER0, true );
			//pShaderShadow->EnableSRGBRead( SHADER_SAMPLER0, false );
 
			int fmt = VERTEX_POSITION;
			
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 );
			pShaderShadow->SetVertexShader( "PassThrough_vs30", 0 );
			pShaderShadow->SetPixelShader( "post_colourfilter_ps30" );
							
			DefaultFog();
		}

		DYNAMIC_STATE
		{
			BindTexture( SHADER_SAMPLER0, FBTEXTURE, -1 );
			
			//========================================
			// night vision brightness level
			//========================================
			float Scale = params[SCALE]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant(0, &Scale);
							
			//========================================
			//	Time % 1000 for scrolling
			//========================================
			float flCurTime = 0.0f;
			float flTime = pShaderAPI->CurrentTime();
			flCurTime = flTime;
			flCurTime -= (float)( (int)( flCurTime / 1000.0f ) ) * 1000.0f;
			pShaderAPI->SetPixelShaderConstant(2, &flCurTime);
			
			//========================================
			// Outer vignette radius
			//========================================
			float outerRadius = params[OUTER_RADIUS]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant(3, &outerRadius);

			//========================================
			// Inner vignette radius
			//========================================
			float innerRadius = params[INNER_RADIUS]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant(4, &innerRadius);

			// Colour definition
			pShaderAPI->SetPixelShaderConstant(7, params[COLOR]->GetVecValue());

			// Luminance
			pShaderAPI->SetPixelShaderConstant(8, params[LUMINANCE]->GetVecValue());

			// Blur (multiplied by 0.001 in the shader)
			float blur = params[BLUR_AMOUNT]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant(9, &blur);
		}
		Draw();
	}
END_SHADER