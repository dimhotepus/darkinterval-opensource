//===================== Copyright (c) Valve Corporation. All Rights Reserved. ======================
//
// Example shader that can be applied to models
//
//==================================================================================================

#include "BaseVSShader.h"
#include "convar.h"
#include "Foliage_dx9_helper.h"

#ifdef GAME_SHADER_DLL
DEFINE_FALLBACK_SHADER( DI_Foliage, DI_Foliage_DX90 )
BEGIN_VS_SHADER( DI_Foliage_DX90, "Help for Example Model Shader" )
#else
DEFINE_FALLBACK_SHADER( Foliage, DI_Foliage_DX90 )
BEGIN_VS_SHADER( Foliage_DX90, "Help for Example Model Shader" )
#endif

	BEGIN_SHADER_PARAMS
		SHADER_PARAM( ALPHATESTREFERENCE, SHADER_PARAM_TYPE_FLOAT, "0.0", "" )
		// vertexlitgeneric tree sway animation control
		SHADER_PARAM(TREESWAY, SHADER_PARAM_TYPE_INTEGER, "1", "");
		SHADER_PARAM(TREESWAYHEIGHT, SHADER_PARAM_TYPE_FLOAT, "1000", "");
		SHADER_PARAM(TREESWAYSTARTHEIGHT, SHADER_PARAM_TYPE_FLOAT, "0.2", "");
		SHADER_PARAM(TREESWAYRADIUS, SHADER_PARAM_TYPE_FLOAT, "300", "");
		SHADER_PARAM(TREESWAYSTARTRADIUS, SHADER_PARAM_TYPE_FLOAT, "0.1", "");
		SHADER_PARAM(TREESWAYSPEED, SHADER_PARAM_TYPE_FLOAT, "1", "");
		SHADER_PARAM(TREESWAYSPEEDHIGHWINDMULTIPLIER, SHADER_PARAM_TYPE_FLOAT, "2", "");
		SHADER_PARAM(TREESWAYSTRENGTH, SHADER_PARAM_TYPE_FLOAT, "10", "");
		SHADER_PARAM(TREESWAYSCRUMBLESPEED, SHADER_PARAM_TYPE_FLOAT, "0.1", "");
		SHADER_PARAM(TREESWAYSCRUMBLESTRENGTH, SHADER_PARAM_TYPE_FLOAT, "0.1", "");
		SHADER_PARAM(TREESWAYSCRUMBLEFREQUENCY, SHADER_PARAM_TYPE_FLOAT, "0.1", "");
		SHADER_PARAM(TREESWAYFALLOFFEXP, SHADER_PARAM_TYPE_FLOAT, "1.5", "");
		SHADER_PARAM(TREESWAYSCRUMBLEFALLOFFEXP, SHADER_PARAM_TYPE_FLOAT, "1.0", "");
		SHADER_PARAM(TREESWAYSPEEDLERPSTART, SHADER_PARAM_TYPE_FLOAT, "3", "");
		SHADER_PARAM(TREESWAYSPEEDLERPEND, SHADER_PARAM_TYPE_FLOAT, "6", "");
	END_SHADER_PARAMS

	void SetupVars(Foliage_DX9_Vars_t& info )
	{
		info.m_nBaseTexture = BASETEXTURE;
		info.m_nBaseTextureFrame = FRAME;
		info.m_nBaseTextureTransform = BASETEXTURETRANSFORM;
		info.m_nAlphaTestReference = ALPHATESTREFERENCE;
		info.m_nFlashlightTexture = FLASHLIGHTTEXTURE;
		info.m_nFlashlightTextureFrame = FLASHLIGHTTEXTUREFRAME;

		info.m_nTreeSway = TREESWAY;
		info.m_nTreeSwayHeight = TREESWAYHEIGHT;
		info.m_nTreeSwayStartHeight = TREESWAYSTARTHEIGHT;
		info.m_nTreeSwayRadius = TREESWAYRADIUS;
		info.m_nTreeSwayStartRadius = TREESWAYSTARTRADIUS;
		info.m_nTreeSwaySpeed = TREESWAYSPEED;
		info.m_nTreeSwaySpeedHighWindMultiplier = TREESWAYSPEEDHIGHWINDMULTIPLIER;
		info.m_nTreeSwayStrength = TREESWAYSTRENGTH;
		info.m_nTreeSwayScrumbleSpeed = TREESWAYSCRUMBLESPEED;
		info.m_nTreeSwayScrumbleStrength = TREESWAYSCRUMBLESTRENGTH;
		info.m_nTreeSwayScrumbleFrequency = TREESWAYSCRUMBLEFREQUENCY;
		info.m_nTreeSwayFalloffExp = TREESWAYFALLOFFEXP;
		info.m_nTreeSwayScrumbleFalloffExp = TREESWAYSCRUMBLEFALLOFFEXP;
		info.m_nTreeSwaySpeedLerpStart = TREESWAYSPEEDLERPSTART;
		info.m_nTreeSwaySpeedLerpEnd = TREESWAYSPEEDLERPEND;
	}

	SHADER_INIT_PARAMS()
	{
		Foliage_DX9_Vars_t info;
		SetupVars( info );
		InitParamsFoliage_DX9( this, params, pMaterialName, info );
	}

	SHADER_FALLBACK
	{
		return 0;
	}

	SHADER_INIT
	{
		Foliage_DX9_Vars_t info;
		SetupVars( info );
		InitFoliage_DX9( this, params, info );
	}

	SHADER_DRAW
	{
		Foliage_DX9_Vars_t info;
		SetupVars( info );
		DrawFoliage_DX9( this, params, pShaderAPI, pShaderShadow, info, vertexCompression, pContextDataPtr );
	}

END_SHADER

