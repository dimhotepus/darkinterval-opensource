//========================================================================//
//
// Purpose: 
//
//=============================================================================//
#ifndef FLASHLIGHTEFFECT_H
#define FLASHLIGHTEFFECT_H
#ifdef _WIN32
#pragma once
#endif

struct dlight_t;

class CFlashlightEffect
{
public:
#ifdef DARKINTERVAL
	CFlashlightEffect(int nEntIndex = 0, const char *pszFlashlightTexture = NULL);
#else
	CFlashlightEffect(int nEntIndex = 0);
#endif
	virtual ~CFlashlightEffect();
#ifdef DARKINTERVAL
	virtual void UpdateLight(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance, bool bUseOffset = true);
#else
	virtual void UpdateLight(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance);
#endif
	void TurnOn();
	void TurnOff();
	bool IsOn(void) { return m_bIsOn; }

	ClientShadowHandle_t GetFlashlightHandle(void) { return m_FlashlightHandle; }
	void SetFlashlightHandle(ClientShadowHandle_t Handle) { m_FlashlightHandle = Handle; }
		
protected:

	void LightOff();
	void LightOffOld();
	void LightOffNew();
#ifdef DARKINTERVAL
	void UpdateLightNew(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, bool bUseOffset);
#else
	void UpdateLightNew(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp);
#endif
	void UpdateLightOld(const Vector &vecPos, const Vector &vecDir, int nDistance);

	bool m_bIsOn;
	int m_nEntIndex;
	ClientShadowHandle_t m_FlashlightHandle;

	// Vehicle headlight dynamic light pointer
	dlight_t *m_pPointLight;
	float m_flDistMod;

	// Texture for flashlight
	CTextureReference m_FlashlightTexture;
};

class CHeadlightEffect : public CFlashlightEffect
{
public:

	CHeadlightEffect();
	~CHeadlightEffect();
#ifdef DARKINTERVAL
	virtual void UpdateLight(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance = 3000, float colorR = 1.0f, float colorG = 1.0f, float Colorb = 1.0f);
#else
	virtual void UpdateLight(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance);
#endif
};
#ifdef DARKINTERVAL
class CProjectedLightEffect : public CFlashlightEffect
{
public:

	CProjectedLightEffect();
	~CProjectedLightEffect();

	virtual void UpdateLight(
		const Vector &vecPos, 
		const Vector &vecDir, 
		const Vector &vecRight, 
		const Vector &vecUp, 
		float fDistance = 1024.0f, 
		float fFov = 45.0f,
		Vector vAttenCombo = Vector(0,200,0),
		Vector vColorCombo = Vector(1.0f, 1.0f, 1.0f)
	);
};
#endif // DARKINTERVAL
#endif // FLASHLIGHTEFFECT_H
