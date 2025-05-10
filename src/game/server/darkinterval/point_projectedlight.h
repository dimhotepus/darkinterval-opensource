#ifndef	POINT_PROJECTEDLIGHT_H
#define	POINT_PROJECTEDLIGHT_H
#include "shareddefs.h"
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
#ifdef DARKINTERVAL
#if 1
#define ENV_PROJECTEDTEXTURE_STARTON			(1<<0)
#define ENV_PROJECTEDTEXTURE_ALWAYSUPDATE		(1<<1)
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CProjectedLight : public CPointEntity
{
	DECLARE_CLASS(CProjectedLight, CPointEntity);
public:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CProjectedLight();
	bool KeyValue(const char *szKeyName, const char *szValue);
	virtual bool GetKeyValue(const char *szKeyName, char *szValue, int iMaxLen);

	// Always transmit to clients
	virtual int UpdateTransmitState();
	virtual void Spawn(void);
	virtual void Activate(void);
	void SetParent(CBaseEntity* pNewParent, int iAttachment = -1);
	void SetFOV(float fFov);
	void SetSpotlightColor(Vector vecColor);
	void TurnOn(void);
	void TurnOff(void);
	void EnableShadows(void);
	void DisableShadows(void);
	void SetBrightness(float brightness);
	void SetAmbient(float ambient);
	void SetNearZ(float nearz);
	void SetFarZ(float farz);

	void InputTurnOn(inputdata_t &inputdata);
	void InputTurnOff(inputdata_t &inputdata);
	void InputAlwaysUpdateOn(inputdata_t &inputdata);
	void InputAlwaysUpdateOff(inputdata_t &inputdata);
	void InputSetFOV(inputdata_t &inputdata);
	void InputSetVerFOV(inputdata_t &inputdata);
	void InputSetHorFOV(inputdata_t &inputdata);
	void InputSetTarget(inputdata_t &inputdata);
	void InputSetCameraSpace(inputdata_t &inputdata);
	void InputSetLightOnlyTarget(inputdata_t &inputdata);
	void InputSetLightWorld(inputdata_t &inputdata);
	void InputSetEnableShadows(inputdata_t &inputdata);
	void InputSetLightColor(inputdata_t &inputdata);
	void InputSetSpotlightTexture(inputdata_t &inputdata);
	void InputSetAmbient(inputdata_t &inputdata);
	void InputSetSpotlightFrame(inputdata_t &inputdata);
	void InputSetBrightness(inputdata_t &inputdata);
	void InputSetColorTransitionTime(inputdata_t &inputdata);
	void InputSetConstant(inputdata_t &inputdata) { m_flConstantAtten = CorrectConstantAtten(inputdata.value.Float()); }
	void InputSetLinear(inputdata_t &inputdata) { m_flLinearAtten = CorrectLinearAtten(inputdata.value.Float()); }
	void InputSetQuadratic(inputdata_t &inputdata) { m_flQuadraticAtten = CorrectQuadraticAtten(inputdata.value.Float()); }
	void InputSetShadowAtten(inputdata_t &inputdata) { m_flShadowAtten = inputdata.value.Float(); }
	void InputSetNearZ(inputdata_t &inputdata);
	void InputSetFarZ(inputdata_t &inputdata);
	void InputAlwaysDrawOn(inputdata_t &inputdata) { m_bAlwaysDraw = true; }
	void InputAlwaysDrawOff(inputdata_t &inputdata) { m_bAlwaysDraw = false; }
	void InputStopFollowingTarget(inputdata_t &inputdata) { m_bDontFollowTarget = true; }
	void InputStartFollowingTarget(inputdata_t &inputdata) { m_bDontFollowTarget = false; }
	void InputSetFilter(inputdata_t &inputdata);

	// Corrects keyvalue/input attenuation for internal FlashlightEffect_t attenuation.
	float CorrectConstantAtten(float fl) { return fl * 0.5f; }
	float CorrectLinearAtten(float fl) { return fl * 100.0f; }
	float CorrectQuadraticAtten(float fl) { return fl * 10000.0f; }

	void InitialThink(void);

	CNetworkHandle(CBaseEntity, m_hTargetEntity);
	CNetworkVar(bool, m_bDontFollowTarget);

private:

	CNetworkVar(bool, m_bState);
	CNetworkVar(bool, m_bAlwaysUpdate);
	CNetworkVar(float, m_flLightFOV);
	CNetworkVar(float, m_flLightHorFOV);
	CNetworkVar(bool, m_bEnableShadows);
	CNetworkVar(bool, m_bLightOnlyTarget);
	CNetworkVar(bool, m_bLightWorld);
	CNetworkVar(bool, m_bCameraSpace);
	CNetworkVar(float, m_flBrightnessScale);
	CNetworkColor32(m_LightColor);
	CNetworkVar(float, m_flColorTransitionTime);
	CNetworkVar(float, m_flAmbient);
	CNetworkString(m_SpotlightTextureName, MAX_PATH);
	CNetworkVar(int, m_nSpotlightTextureFrame);
	CNetworkVar(float, m_flNearZ);
	CNetworkVar(float, m_flFarZ);
	CNetworkVar(int, m_nShadowQuality);
	CNetworkVar(float, m_flConstantAtten);
	CNetworkVar(float, m_flLinearAtten);
	CNetworkVar(float, m_flQuadraticAtten);
	CNetworkVar(float, m_flShadowAtten);

	CNetworkVar(float, m_flShadowFilter);

	CNetworkVar(bool, m_bAlwaysDraw);

	// 1 = New projected texture
	// 0 = Non-Mapbase projected texture, e.g. one that uses the VDC parenting fix instead of the spawnflag
	// Not needed on the client right now, change to CNetworkVar when it actually is needed
	bool m_bProjectedTextureVersion;
};
#else
class CProjectedLight : public CPointEntity
{
	DECLARE_CLASS(CProjectedLight, CPointEntity);
public:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CProjectedLight();
	bool KeyValue(const char *szKeyName, const char *szValue);

	// Always transmit to clients
	virtual int UpdateTransmitState();
	virtual void Activate(void);
	void SetFOV(float fFov);
	void TurnOn(void) { m_projectedlighton_bool = true; }
	void TurnOff(void) { m_projectedlighton_bool = false; }
	//	void SetShadowsEnabled(bool bEnabled);
	//	void SetSpotlightTexture(bool bEnabled);
	void SetSpotlightColor(Vector vecColor);
	// [0] = Quadratic [1] = Linear [2] = Constant
	void SetSpotlightAttenuation(Vector vecAtten);

	void InputTurnOn(inputdata_t &inputdata);
	void InputTurnOff(inputdata_t &inputdata);
	void InputSetFOV(inputdata_t &inputdata);
	//	void InputSetShadowsEnabled(inputdata_t &inputdata);
	//	void InputSetSpotlightTexture(inputdata_t &inputdata);
	void InputSetSpotlightColor(inputdata_t &inputdata);

	void InitialThink(void);
private:
	CNetworkVar(bool, m_projectedlighton_bool);
	CNetworkVar(float, m_projectedlightfov_float);
	CNetworkVar(Vector, m_projectedlightatten_vec);
	CNetworkVar(Vector, m_projectedlightcolor_vec);
};
#endif
#else // DARKINTERVAL
// stuff
#endif 
#endif // PROJECTEDLIGHT_H