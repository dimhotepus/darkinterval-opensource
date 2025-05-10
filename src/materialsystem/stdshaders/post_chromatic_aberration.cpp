#include "BaseVSShader.h"

#include "PassThrough_vs30.inc"
#include "post_chromatic_aberration_ps30.inc"

BEGIN_VS_SHADER( Post_Chromatic_Aberration, "Help for Post_Chromatic_Aberration" )

	BEGIN_SHADER_PARAMS
	SHADER_PARAM(COLORFACTOR_R, SHADER_PARAM_TYPE_FLOAT, "1.0", "")
	SHADER_PARAM(COLORFACTOR_G, SHADER_PARAM_TYPE_FLOAT, "1.0", "")
	SHADER_PARAM(COLORFACTOR_B, SHADER_PARAM_TYPE_FLOAT, "1.0", "")
	SHADER_PARAM( FBTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB", "" )
	END_SHADER_PARAMS

	SHADER_FALLBACK
	{
		if (g_pHardwareConfig->GetDXSupportLevel() < 81)
		{
			return "Wireframe";
		}
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
 
			int fmt = VERTEX_POSITION;
			
			pShaderShadow->VertexShaderVertexFormat( fmt, 1, 0, 0 ); //sets the vertex format for the .fxc
			pShaderShadow->SetVertexShader( "PassThrough_vs30", 0 );
			pShaderShadow->SetPixelShader( "post_chromatic_aberration_ps30" );
				
			DefaultFog();
		}

		DYNAMIC_STATE
		{
			float fR = params[COLORFACTOR_R]->GetFloatValue() / 100;
			float fG = params[COLORFACTOR_G]->GetFloatValue() / 100;
			float fB = params[COLORFACTOR_B]->GetFloatValue() / 100;
			float vScale[4] = {fR, fG, fB, 1};
			pShaderAPI->SetPixelShaderConstant(0, vScale);
			BindTexture( SHADER_SAMPLER0, FBTEXTURE, -1 );
		}
		Draw();
	}
END_SHADER