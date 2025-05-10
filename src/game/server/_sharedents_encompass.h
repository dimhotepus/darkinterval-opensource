#ifndef _SHAREDENTS_ENCOMPASS_H
#define _SHAREDENTS_ENCOMPASS_H
#include "basegrenade_shared.h"
#include "env_spritetrail.h"
#include "sprite.h"

// SHARED PROJECTILE SYSTEM
// This is basically expanded copy of the 
// bullsquid spit projectiles.
// Now it's more generalised and supports
// different types of projectile:
// acid (like original), plasma, 
// inflammable, bio spore... et cetera. 
// There's no need to copy the original grenade
// and just tweak the effects a bit.
// Can just have one shared projectile and all 
// "encompassed" actors can reference it.
class CParticleSystem;
enum ProjectileSize_e
{
	Projectile_SMALL = 10,
	Projectile_LARGE,
	Projectile_SPECIAL,
};

enum ProjectileBehavior_e
{
	Projectile_CONTACT = 20,
	Projectile_HOMING,
	Projectile_STICKYDELAY,
	Projectile_STICKYSENSOR,
	Projectile_BOUNCEDELAY,
	Projectile_BOUNCESENSOR,
	Projectile_CLUSTER,
};

enum ProjectileType_e
{
	Projectile_ACID = 100,		// squid/antlion spit
	Projectile_PLASMA,			// glider synth bomb
	Projectile_INFERNO,			// cremator bomb
	Projectile_LIGHTNING,		// tesla bomb
	Projectile_SPORE,			// antlion guard projectile
	Projectile_SMOKE,			// MP smoke grenade
	Projectile_BLAST,			// regular bomb - Crab synths use this for example
	Projectile_NEEDLE,			// Hydra needle
	Projectile_MUDBALL,			// Hydra Smacker projectile
};

class CSharedProjectile : public CBaseGrenade
{
	DECLARE_CLASS(CSharedProjectile, CBaseGrenade);
public:
	CSharedProjectile(void)
	{
		m_projectiletravel_sound = NULL;
	}
	~CSharedProjectile(void)
	{
		CBaseEntity* e_projectileparticle = (CBaseEntity*)m_projectileparticle_handle;
		if (e_projectileparticle)
		{
			UTIL_Remove(e_projectileparticle);
		}
	}
	virtual void Precache(void);
	virtual void Spawn(void);
	void OnRestore();
	virtual void Event_Killed(const CTakeDamageInfo &info)
	{
		Detonate();
	}

	void ContextThink_Lifetime(void);
	void ContextThink_Doppler(void);
//	void Think_Homer(void);
	void Think_Timer(void);
	void Think_Sensor(void);
	void Touch_Sensor(CBaseEntity *pOther);

	void Detonate(void);

	void ProjectileTouch(CBaseEntity *pOther);

	inline void StickTo(CBaseEntity *pOther, trace_t &tr);

	void DetonationFX(void);

	void TimerFX(void);

	void DetonationDamage(void);

	void KillEffects(void);

	void SetProjectileStats(int nSize, int nType, int nBehavior, float damage, float radius, const float gravity = 600, const float sensordist = 128, const float delaytime = 3.0);

	void CreateProjectileEffects(void);

	int	GetProjectileSize(void) { return m_projectilesize_int; }
	int GetProjectileType(void) { return m_projectiletype_int; }
	int GetProjectileBehavior(void) { return m_projectilebehavior_int; }

	bool IsProjectileSticky(void)
	{
		if (GetProjectileBehavior() == Projectile_STICKYDELAY
			|| GetProjectileBehavior() == Projectile_STICKYSENSOR)
			return true;
		return false;
	}

	bool IsProjectileBouncy(void)
	{
		if (GetProjectileBehavior() == Projectile_BOUNCEDELAY
			|| GetProjectileBehavior() == Projectile_BOUNCESENSOR)
			return true;
		return false;
	}

	virtual	unsigned int	PhysicsSolidMaskForEntity(void) const { return (BaseClass::PhysicsSolidMaskForEntity() | CONTENTS_WATER); }

private:
	CParticleSystem				*m_projectileparticle_handle;
	CSoundPatch					*m_projectiletravel_sound;
	int							m_projectilesize_int;
	int							m_projectiletype_int;
	int							m_projectilebehavior_int;
	float						m_projectiledelaytime_float;
	float						m_projectilesensordist_float;
	float						m_projectileticktime_time;
	float						m_projectilecreation_time;

	CSprite						*m_projectile_sprite;

public:
	DECLARE_DATADESC();
};

#include "props.h"
#include "datacache/imdlcache.h"

// FLETCHETTES
static const char *HUNTER_FLECHETTE_MODEL = "models/weapons/hunter_flechette.mdl";

static const char *s_szHunterFlechetteBubbles = "HunterFlechetteBubbles";
static const char *s_szHunterFlechetteSeekThink = "HunterFlechetteSeekThink";
static const char *s_szHunterFlechetteDangerSoundThink = "HunterFlechetteDangerSoundThink";
static const char *s_szHunterFlechetteSpriteTrail = "sprites/bluelaser1.vmt";
static int s_nHunterFlechetteImpact = -2;
static int s_nFlechetteFuseAttach = -1;

#define FLECHETTE_AIR_VELOCITY	2500

class CHunterFlechette : public CPhysicsProp, public IParentPropInteraction
{
	DECLARE_CLASS(CHunterFlechette, CPhysicsProp);

public:
	CHunterFlechette();
	~CHunterFlechette();

	Class_T Classify() { return CLASS_NONE; }

	bool WasThrownBack()
	{
		return m_bThrownBack;
	}

public:
	void Spawn();
	void Activate();
	void Precache();
	void Shoot(Vector &vecVelocity, bool bBright);
	void SetSeekTarget(CBaseEntity *pTargetEntity);
	void Explode();

	bool CreateVPhysics();

	unsigned int PhysicsSolidMaskForEntity() const;
	static CHunterFlechette *FlechetteCreate(const Vector &vecOrigin, const QAngle &angAngles, CBaseEntity *pentOwner = NULL);

	// IParentPropInteraction
	void OnParentCollisionInteraction(parentCollisionInteraction_t eType, int index, gamevcollisionevent_t *pEvent);
	void OnParentPhysGunDrop(CBasePlayer *pPhysGunUser, PhysGunDrop_t Reason);

protected:
	void SetupGlobalModelData();

	void StickTo(CBaseEntity *pOther, trace_t &tr);

	void BubbleThink();
	void DangerSoundThink();
	void ExplodeThink();
	void DopplerThink();
	void SeekThink();

	bool CreateSprites(bool bBright);

	void FlechetteTouch(CBaseEntity *pOther);

	Vector m_vecShootPosition;
	EHANDLE m_hSeekTarget;
	bool m_bThrownBack;

	DECLARE_DATADESC();
	//DECLARE_SERVERCLASS();
};

///// FLETCHETTES

#endif