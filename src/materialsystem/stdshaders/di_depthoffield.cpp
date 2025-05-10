//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: Depth of field material
//
//===========================================================================//

#include "BaseVSShader.h"
#include "di_depth_of_field_vs20.inc"
#include "di_depth_of_field_ps20b.inc"
#include "convar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// 8 samples
static const float s_flPoissonConstsQuality0[16] = {
	0.0, 0.0,
	0.527837, -0.085868,
	-0.040088, 0.536087,
	-0.670445, -0.179949,
	-0.419418, -0.616039,
	0.440453, -0.639399,
	-0.757088, 0.349334,
	0.574619, 0.685879
};

// 16 samples
static const float s_flPoissonConstsQuality1[32] = {
	0.0747,		-0.8341,
	-0.9138,	0.3251,
	0.8667,		-0.3029,
	-0.4642,	0.2187,
	-0.1505,	0.7320,
	0.7310,		-0.6786,
	0.2859,		-0.3254,
	-0.1311,	-0.2292,
	0.3518,		0.6470,
	-0.7485,	-0.6307,
	0.1687,		0.1873,
	-0.3604,	-0.7483,
	-0.5658,	-0.1521,
	0.7102,		0.0536,
	-0.6056,	0.7747,
	0.7793,		0.6194
};

// 32 samples
static const float s_flPoissonConstsQuality2[64] = {
	0.0854f, -0.0644f,
	0.8744f, 0.1665f,
	0.2329f, 0.3995f,
	-0.7804f, 0.5482f,
	-0.4577f, 0.7647f,
	-0.1936f, 0.5564f,
	0.4205f, -0.5768f,
	-0.0304f, -0.9050f,
	-0.5215f, 0.1854f,
	0.3161f, -0.2954f,
	0.0666f, -0.5564f,
	-0.2137f, -0.0072f,
	-0.4112f, -0.3311f,
	0.6438f, -0.2484f,
	-0.9055f, -0.0360f,
	0.8323f, 0.5268f,
	0.5592f, 0.3459f,
	-0.6797f, -0.5201f,
	-0.4325f, -0.8857f,
	0.8768f, -0.4197f,
	0.3090f, -0.8646f,
	0.5034f, 0.8603f,
	0.3752f, 0.0627f,
	-0.0161f, 0.2627f,
	0.0969f, 0.7054f,
	-0.2291f, -0.6595f,
	-0.5887f, -0.1100f,
	0.7048f, -0.6528f,
	-0.8438f, 0.2706f,
	-0.5061f, 0.4653f,
	-0.1245f, -0.3302f,
	-0.1801f, 0.8486f
};

ConVar mat_visualize_dof("mat_visualize_dof", "0");

DEFINE_FALLBACK_SHADER(DI_DepthOfField, DI_DepthOfField_dx9)
BEGIN_VS_SHADER_FLAGS(DI_DepthOfField_dx9, "Depth of Field", SHADER_NOT_EDITABLE)
BEGIN_SHADER_PARAMS
SHADER_PARAM(BASETEXTURE, SHADER_PARAM_TYPE_TEXTURE, "", "")
SHADER_PARAM(BASETEXTURE2, SHADER_PARAM_TYPE_TEXTURE, "shadertest/detail", "detail texture")
SHADER_PARAM(DOF_START_DISTANCE, SHADER_PARAM_TYPE_FLOAT, "0.0", "")
SHADER_PARAM(DOF_POWER, SHADER_PARAM_TYPE_FLOAT, "0.0", "")
SHADER_PARAM(DOF_MAX, SHADER_PARAM_TYPE_FLOAT, "0.0", "")
END_SHADER_PARAMS

SHADER_INIT_PARAMS()
{
	SET_PARAM_FLOAT_IF_NOT_DEFINED(DOF_START_DISTANCE, 100.0f);
	SET_PARAM_FLOAT_IF_NOT_DEFINED(DOF_POWER, 0.4f);
	SET_PARAM_FLOAT_IF_NOT_DEFINED(DOF_MAX, 0.7f);
}

SHADER_FALLBACK
{
	if (g_pHardwareConfig->GetDXSupportLevel() < 90)
	{
		return "Wireframe";
	}

return 0;
}

SHADER_INIT
{
	LoadTexture(BASETEXTURE, TEXTUREFLAGS_SRGB);

	if (params[BASETEXTURE2]->IsDefined())
	{
		LoadTexture(BASETEXTURE2, TEXTUREFLAGS_SRGB);
	}
}

SHADER_DRAW
{
	SHADOW_STATE
{
	pShaderShadow->VertexShaderVertexFormat(VERTEX_POSITION, 1, 0, 0);

	pShaderShadow->EnableTexture(SHADER_SAMPLER0, true);
	pShaderShadow->EnableSRGBRead(SHADER_SAMPLER0, true);

	pShaderShadow->EnableTexture(SHADER_SAMPLER1, true);
	pShaderShadow->EnableSRGBRead(SHADER_SAMPLER1, true);

	pShaderShadow->EnableSRGBWrite(true);
	pShaderShadow->EnableAlphaWrites(true);
	pShaderShadow->EnableAlphaTest(false);

	pShaderShadow->EnableDepthWrites(false);
	pShaderShadow->EnableDepthTest(false);

	pShaderShadow->EnableBlending(true);
	pShaderShadow->BlendFunc(SHADER_BLEND_SRC_ALPHA, SHADER_BLEND_ONE_MINUS_SRC_ALPHA);

	DefaultFog();

	DECLARE_STATIC_VERTEX_SHADER(di_depth_of_field_vs20);
	SET_STATIC_VERTEX_SHADER(di_depth_of_field_vs20);

	if (g_pHardwareConfig->SupportsPixelShaders_2_b())
	{
		DECLARE_STATIC_PIXEL_SHADER(di_depth_of_field_ps20b);
		SET_STATIC_PIXEL_SHADER(di_depth_of_field_ps20b);
	}
	else
	{
		Assert(!"No ps_2_b. This shouldn't be happening");
	}
}

DYNAMIC_STATE
{
	DECLARE_DYNAMIC_VERTEX_SHADER(di_depth_of_field_vs20);
	SET_DYNAMIC_VERTEX_SHADER(di_depth_of_field_vs20);

	// Bind textures
	BindTexture(SHADER_SAMPLER0, BASETEXTURE);
	BindTexture(SHADER_SAMPLER1, BASETEXTURE2);

	float vConst[16] = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

	vConst[0] = params[DOF_START_DISTANCE]->GetFloatValue();
	vConst[1] = params[DOF_POWER]->GetFloatValue();
	vConst[2] = params[DOF_MAX]->GetFloatValue();
	
	pShaderAPI->SetPixelShaderConstant(0, vConst, 4);

	float eyePos[4];
	s_pShaderAPI->GetWorldSpaceCameraPosition(eyePos);
	s_pShaderAPI->SetPixelShaderConstant(8, eyePos, 1);
	pShaderAPI->SetPixelShaderFogParams(11);
	
	bool bVisualizeDoF = mat_visualize_dof.GetBool();

	if (g_pHardwareConfig->SupportsPixelShaders_2_b())
	{
		DECLARE_DYNAMIC_PIXEL_SHADER(di_depth_of_field_ps20b);
		SET_DYNAMIC_PIXEL_SHADER_COMBO(PIXELFOGTYPE, pShaderAPI->GetPixelFogCombo());
		SET_DYNAMIC_PIXEL_SHADER_COMBO(VISUALIZE_DOF, bVisualizeDoF);
		SET_DYNAMIC_PIXEL_SHADER(di_depth_of_field_ps20b);
	}
	else
	{
		Assert(!"No ps_2_b. This shouldn't be happening");
	}
}

	Draw();
}
END_SHADER
