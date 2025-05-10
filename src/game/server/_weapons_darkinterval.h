#ifndef _WEAPONS_DARKINTERVAL_H
#define _WEAPONS_DARKINTERVAL_H
#include "basehlcombatweapon.h"
#include "env_spritetrail.h"
#include "smoke_trail.h"
#include "basegrenade_shared.h"

// Supplemental weapon classes
// like grenades, flares, other projectiles
// et cetera.

class CSlamGrenade : public CBaseGrenade
{
	DECLARE_CLASS(CSlamGrenade, CBaseGrenade);
public:
	CSlamGrenade();
	~CSlamGrenade();
	void Precache(void);
	void Spawn(void);
	void CreateEffects(void);
	void BounceSound(void);
	void SlamSensorThink();
	void SlamFlyTouch(CBaseEntity *pOther);
	void StickTo(CBaseEntity *pOther, trace_t &tr);
	void SlamDetonateThink(void);
	void SlamFlyThink(void);
	void Deactivate(void);
	void InputExplode(inputdata_t &inputdata);
	CHandle<CSprite>	m_hGlowSprite;
public:
	DECLARE_DATADESC();
	bool m_bIsAttached;
	float m_flNextBounceSoundTime;
	bool m_bPlayerOwned;
	Class_T m_NPCOwnerClass;
	Vector m_vLastPosition;

};

class CFlare : public CBaseCombatCharacter
{
public:
	DECLARE_CLASS(CFlare, CBaseCombatCharacter);

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

public:
	static CFlare *activeFlares;

	CBaseEntity *m_pOwner;
	int			m_nBounces;			// how many times has this flare bounced?
	CNetworkVar(float, m_flTimeBurnOut);	// when will the flare burn out?
	CNetworkVar(float, m_flScale);
	CNetworkVar(bool, m_bLight);
#ifndef DARKINTERVAL // old client-side smoke trail has been replaced with a smoke trail entity
	CNetworkVar(bool, m_bSmoke);
#endif
	CNetworkVar(bool, m_bPropFlare);

	CFlare();
	~CFlare();

	static CFlare *	GetActiveFlares(void)
	{
		return CFlare::activeFlares;
	}

	CFlare *GetNextFlare(void) const { return m_pNextFlare; }
#ifdef DARKINTERVAL // so flares interact with fire system and can add heat to them
	float GetHeatLevel() { return m_flCurrentHeatLevel; }
	void SetGoalHeatLevel(float heat) { m_flMaxHeatLevel = heat; }
	void InputSetGoalHeatLevel(inputdata_t &inputdata) { m_flMaxHeatLevel = inputdata.value.Float(); }

	static CFlare *Create(Vector vecOrigin,
		QAngle vecAngles,
		CBaseEntity *pOwner,
		float lifetime, float activationDelay = 0.1f, float goalHeatLevel = 0.0f);
#else
	static CFlare *Create(Vector vecOrigin, QAngle vecAngles, CBaseEntity *pOwner, float lifetime);
#endif
	virtual unsigned int PhysicsSolidMaskForEntity(void) const
	{
		return MASK_SOLID; // not NPCSOLID - that causes flares to hit npc clips
	}

	Class_T Classify(void) { return CLASS_FLARE; }

	void Precache(void);
	void Spawn(void);
	int Restore(IRestore &restore);
	void Activate(void);
	void StartBurnSound(void);

	void Launch(const Vector &direction, float speed);
	void Start(float lifeTime);
	void Die(float fadeTime);

	void FlareTouch(CBaseEntity *pOther);
	void FlareBurnTouch(CBaseEntity *pOther);
	void FlareThink(void);

	void InputStart(inputdata_t &inputdata);
	void InputDie(inputdata_t &inputdata);
	void InputLaunch(inputdata_t &inputdata);

	void RemoveFromActiveFlares(void);
	void AddToActiveFlares(void);

private:
	CSoundPatch	*m_pBurnSound;
	bool		m_bFading;
	float		m_flDuration;
	float		m_flNextDamage;
	float		m_flMaxHeatLevel;
	float		m_flCurrentHeatLevel;
	float		m_flHeatRadius;
	bool		m_bInActiveList;
	CFlare *	m_pNextFlare;
#ifdef DARKINTERVAL
	CHandle<CSprite>		m_pFlareGlow;
	CHandle<CSpriteTrail>	m_pFlareTrail;
	CHandle<SmokeTrail>		m_pStartSmokeTrail;
#endif
};
#endif // _WEAPONS_DARKINTERVAL_H
