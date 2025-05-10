#ifndef C_ProjectedLight_H
#define C_ProjectedLight_H
#ifdef _WIN32
#pragma once
#endif

#include "c_baseentity.h"
#include "basetypes.h"

#ifdef DARKINTERVAL // from Mapbase
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_ProjectedLight : public C_BaseEntity
{
	DECLARE_CLASS(C_ProjectedLight, C_BaseEntity);
public:
	DECLARE_CLIENTCLASS();

	void SetMaterial(IMaterial *pMaterial);
	void SetLightColor(byte r, byte g, byte b, byte a);
	void SetSize(float flSize);
	void SetRotation(float flRotation);

	virtual void OnDataChanged(DataUpdateType_t updateType);
	void	ShutDownLightHandle(void);

	virtual void Simulate();

	void	UpdateLight(void);

	C_ProjectedLight();
	~C_ProjectedLight();

	static void SetVisibleBBoxMinHeight(float flVisibleBBoxMinHeight) { m_flVisibleBBoxMinHeight = flVisibleBBoxMinHeight; }
	static float GetVisibleBBoxMinHeight(void) { return m_flVisibleBBoxMinHeight; }
	static C_ProjectedLight *Create();

private:

	inline bool IsBBoxVisible(void);
	bool IsBBoxVisible(Vector vecExtentsMin,
		Vector vecExtentsMax);

	ClientShadowHandle_t m_LightHandle;
	bool m_bForceUpdate;

	EHANDLE	m_hTargetEntity;
	bool m_bDontFollowTarget;

	bool		m_bState;
	bool		m_bAlwaysUpdate;
	float		m_flLightFOV;
	float		m_flLightHorFOV;
	bool		m_bEnableShadows;
	bool		m_bLightOnlyTarget;
	bool		m_bLightWorld;
	bool		m_bCameraSpace;
	float		m_flBrightnessScale;
	color32		m_LightColor;
	Vector		m_CurrentLinearFloatLightColor;
	float		m_flCurrentLinearFloatLightAlpha;
	float		m_flCurrentBrightnessScale;
	float		m_flColorTransitionTime;
	float		m_flAmbient;
	float		m_flNearZ;
	float		m_flFarZ;
	char		m_SpotlightTextureName[MAX_PATH];
	CTextureReference m_SpotlightTexture;
	int			m_nSpotlightTextureFrame;
	int			m_nShadowQuality;
	float		m_flConstantAtten;
	float		m_flLinearAtten;
	float		m_flQuadraticAtten;
	float		m_flShadowAtten;
	float		m_flShadowFilter;

	bool		m_bAlwaysDraw;
	//bool		m_bProjectedTextureVersion;

	Vector	m_vecExtentsMin;
	Vector	m_vecExtentsMax;

	static float m_flVisibleBBoxMinHeight;
};

bool C_ProjectedLight::IsBBoxVisible(void)
{
	return IsBBoxVisible(GetAbsOrigin() + m_vecExtentsMin, GetAbsOrigin() + m_vecExtentsMax);
}

#else

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_ProjectedLight : public C_BaseEntity
{
	DECLARE_CLASS(C_ProjectedLight, C_BaseEntity);
public:
	DECLARE_CLIENTCLASS();

	C_ProjectedLight();
	~C_ProjectedLight();

	virtual void OnDataChanged(DataUpdateType_t updateType);
	void	ShutDownLightHandle(void);

	virtual void Simulate();

	void	UpdateLight(bool bForceUpdate);

	bool	ShadowsEnabled();

	float	GetFOV();

private:

	ClientShadowHandle_t m_LightHandle;

	EHANDLE	m_hTargetEntity;

	bool	m_bState;
	float	m_flLightFOV;
	bool	m_bEnableShadows;
	bool	m_bLightOnlyTarget;
	bool	m_bLightWorld;
	bool	m_bCameraSpace;
	color32	m_cLightColor;
	float	m_flAmbient;
	char	m_SpotlightTextureName[MAX_PATH];
	int		m_nSpotlightTextureFrame;
	int		m_nShadowQuality;
	bool	m_bCurrentShadow;

public:
	C_ProjectedLight  *m_pNext;
};

C_ProjectedLight* GetEnvProjectedTextureList();

#endif

#endif // C_ProjectedLight_H
