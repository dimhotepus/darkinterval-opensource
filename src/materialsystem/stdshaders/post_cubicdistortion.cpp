#include "BaseVSShader.h"
#include "PassThrough_vs30.inc"
#include "post_cubicdistortion_ps30.inc"

BEGIN_VS_SHADER( Post_Cubicdistortion, "Help for Post_Cubic" )

	BEGIN_SHADER_PARAMS
	SHADER_PARAM(FBTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB", "" )
	SHADER_PARAM(DISTORTIONCOEFFICIENT, SHADER_PARAM_TYPE_FLOAT, "-0.15", "Distortion coefficient")
	SHADER_PARAM(CUBICCOEFFICIENT, SHADER_PARAM_TYPE_FLOAT, "0.15", "Cubic lense coefficient")
	SHADER_PARAM(R, SHADER_PARAM_TYPE_FLOAT, "0.00333", "Red channel distortion")
	SHADER_PARAM(G, SHADER_PARAM_TYPE_FLOAT, "0.00333", "Green channel distortion")
	SHADER_PARAM(B, SHADER_PARAM_TYPE_FLOAT, "0.00333", "Blue channel distortion")
	END_SHADER_PARAMS

	SHADER_FALLBACK
	{
		return 0;
	}

	SHADER_INIT_PARAMS()
	{
		if ( !params[ FBTEXTURE ]->IsDefined() )
			params[ FBTEXTURE ]->SetStringValue("_rt_FullFrameFB");
		if (!params[DISTORTIONCOEFFICIENT]->IsDefined())
			params[DISTORTIONCOEFFICIENT]->SetFloatValue(-0.15f);
		if (!params[CUBICCOEFFICIENT]->IsDefined())
			params[CUBICCOEFFICIENT]->SetFloatValue(0.15f);

		if (!params[R]->IsDefined()) params[R]->SetFloatValue(0.00333f);
		if (!params[G]->IsDefined()) params[G]->SetFloatValue(0.00333f);
		if (!params[B]->IsDefined()) params[B]->SetFloatValue(0.00333f);
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
 
			int fmt = VERTEX_POSITION;
			
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 ); //sets the vertex format for the .fxc
			pShaderShadow->SetVertexShader( "PassThrough_vs30", 0 );
			pShaderShadow->SetPixelShader( "post_cubicdistortion_ps30" );
				
			DefaultFog();
		}

		DYNAMIC_STATE
		{
			BindTexture( SHADER_SAMPLER0, FBTEXTURE, -1 );
			float dc = params[DISTORTIONCOEFFICIENT]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant(0, &dc);
			float cc = params[CUBICCOEFFICIENT]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant(1, &cc);
			float red = params[R]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant(2, &red);
			float grn = params[G]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant(3, &grn);
			float blu = params[B]->GetFloatValue();
			pShaderAPI->SetPixelShaderConstant(4, &blu);
		}
		Draw();
	}
END_SHADER