#include "BaseVSShader.h"
#include "PassThrough_vs30.inc"
#include "post_psychedelic_ps30.inc"

BEGIN_VS_SHADER(Post_Influence, "Help for Post_Cubic")

BEGIN_SHADER_PARAMS
SHADER_PARAM(FBTEXTURE, SHADER_PARAM_TYPE_TEXTURE, "_rt_FullFrameFB", "")
SHADER_PARAM(INFLUENCETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "Environment maps/scene_dark01", "")
SHADER_PARAM(WEIGHT, SHADER_PARAM_TYPE_FLOAT, "0.5", "Influence Filter Weight")
SHADER_PARAM(VALUE, SHADER_PARAM_TYPE_FLOAT, "0.5", "Influence Filter Value")
END_SHADER_PARAMS

SHADER_FALLBACK
{
	return 0;
}

SHADER_INIT_PARAMS()
{
	if (!params[FBTEXTURE]->IsDefined())
	{
		params[FBTEXTURE]->SetStringValue("_rt_FullFrameFB");
	}

	if (!params[INFLUENCETEXTURE]->IsDefined())
	{
		params[INFLUENCETEXTURE]->SetStringValue("Environment maps/scene_dark01");
	}

	if (!params[WEIGHT]->IsDefined())
	{
		params[WEIGHT]->SetFloatValue(0.5f);
	}

	if (!params[VALUE]->IsDefined())
	{
		params[VALUE]->SetFloatValue(0.5f);
	}
}

SHADER_INIT
{
	if (params[FBTEXTURE]->IsDefined())
	{
		LoadTexture(FBTEXTURE);
	}

	if (params[INFLUENCETEXTURE]->IsDefined())
	{
		LoadTexture(INFLUENCETEXTURE);
	}
}

SHADER_DRAW
{
	SHADOW_STATE
	{
		pShaderShadow->EnableDepthWrites(false);
		pShaderShadow->EnableTexture(SHADER_SAMPLER0, true);
		pShaderShadow->EnableTexture(SHADER_SAMPLER1, true);

		int fmt = VERTEX_POSITION;

		pShaderShadow->VertexShaderVertexFormat(fmt, 1, 0, 0); //sets the vertex format for the .fxc
		pShaderShadow->SetVertexShader("PassThrough_vs30", 0);
		pShaderShadow->SetPixelShader("post_psychedelic_ps30");

		DefaultFog();
	}

	DYNAMIC_STATE
	{
		BindTexture(SHADER_SAMPLER0, FBTEXTURE, -1);
		BindTexture(SHADER_SAMPLER1, INFLUENCETEXTURE, -1);
		float weight = params[WEIGHT]->GetFloatValue();
		pShaderAPI->SetPixelShaderConstant(0, &weight);
		float value = params[VALUE]->GetFloatValue();
		pShaderAPI->SetPixelShaderConstant(1, &value);
	}

	Draw();
}
END_SHADER