#ifndef MATERIAL_MODIFY_CONTROL_H
#define MATERIAL_MODIFY_CONTROL_H
#pragma once

#include "cbase.h"

//------------------------------------------------------------------------------
// FIXME: This really should inherit from something	more lightweight.
//------------------------------------------------------------------------------

#define MATERIAL_MODIFY_STRING_SIZE			255
#define MATERIAL_MODIFY_ANIMATION_UNSET		-1

// Must match C_MaterialModifyControl.cpp
enum MaterialModifyMode_t
{
	MATERIAL_MODIFY_MODE_NONE = 0,
	MATERIAL_MODIFY_MODE_SETVAR = 1,
	MATERIAL_MODIFY_MODE_ANIM_SEQUENCE = 2,
	MATERIAL_MODIFY_MODE_FLOAT_LERP = 3,
};

ConVar debug_materialmodifycontrol("debug_materialmodifycontrol", "0");

class CMaterialModifyControl : public CBaseEntity
{
public:

	DECLARE_CLASS(CMaterialModifyControl, CBaseEntity);

	CMaterialModifyControl();

	void Spawn(void);
	bool KeyValue(const char *szKeyName, const char *szValue);
	int UpdateTransmitState();
	int ShouldTransmit(const CCheckTransmitInfo *pInfo);

	void SetMaterialVar(inputdata_t &inputdata);
	void SetMaterialVarToCurrentTime(inputdata_t &inputdata);
	void InputStartAnimSequence(inputdata_t &inputdata);
	void InputStartFloatLerp(inputdata_t &inputdata);

	virtual int	ObjectCaps(void) { return BaseClass::ObjectCaps() & ~FCAP_ACROSS_TRANSITION; }

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
public:
	CNetworkString(m_szMaterialName, MATERIAL_MODIFY_STRING_SIZE);
	CNetworkString(m_szMaterialVar, MATERIAL_MODIFY_STRING_SIZE);
	CNetworkString(m_szMaterialVarValue, MATERIAL_MODIFY_STRING_SIZE);
private:
	CNetworkVar(int, m_iFrameStart);
	CNetworkVar(int, m_iFrameEnd);
	CNetworkVar(bool, m_bWrap);
	CNetworkVar(float, m_flFramerate);
	CNetworkVar(bool, m_bNewAnimCommandsSemaphore);
	CNetworkVar(float, m_flFloatLerpStartValue);
	CNetworkVar(float, m_flFloatLerpEndValue);
	CNetworkVar(float, m_flFloatLerpTransitionTime);
	CNetworkVar(int, m_nModifyMode);
};
#endif