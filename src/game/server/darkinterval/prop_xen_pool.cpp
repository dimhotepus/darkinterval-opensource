#include "cbase.h"
#include "player.h"
#include "props.h"
#include "soundenvelope.h"
#include "engine/ienginesound.h"

#define SF_XENPOOL_FAST 1<<17

class CXenPoolLiquidSurface : public CBaseAnimating
{
	DECLARE_CLASS(CXenPoolLiquidSurface, CBaseAnimating);
	void Precache();
	void Spawn();
}; 

LINK_ENTITY_TO_CLASS(xen_pool_liquid, CXenPoolLiquidSurface);

void CXenPoolLiquidSurface::Precache()
{
	BaseClass::Precache();
	PrecacheModel("models/props_foliage_new/xen_pool_liquid.mdl");
}

void CXenPoolLiquidSurface::Spawn()
{
	Precache();
	CBaseEntity *pOwner = (GetOwnerEntity()) ? GetOwnerEntity() : NULL;

	if (pOwner)
	{
		SetAbsAngles(GetOwnerEntity()->GetAbsAngles());
		SetAbsOrigin(GetOwnerEntity()->GetAbsOrigin());
		SetParent(GetOwnerEntity(), -1);
		SetLocalOrigin(Vector(0,0,0));
	}

	SetModel("models/props_foliage_new/xen_pool_liquid.mdl");
	SetSolid(SOLID_VPHYSICS);
}

//-----------------------------------------------------------------------------
// Single-entity healing xen pool
//-----------------------------------------------------------------------------
class CPropXenPool : public CBaseAnimating
{
public:
	DECLARE_CLASS(CPropXenPool, CBaseAnimating);

	void Spawn();
	void Precache(void);
	bool CreateVPhysics(void);
	bool KeyValue(const char *szKeyName, const char *szValue);
	virtual int	ObjectCaps(void) { return BaseClass::ObjectCaps() | m_iCaps; }
private:
	void	HealThink(void);
	int		m_iCaps;

	DECLARE_DATADESC();
protected:
	bool	m_ishealingsomething_bool;
	CSoundPatch *m_ambient_sndpatch;		// Regular sound heard outside the pool
	CSoundPatch *m_healing_sndpatch;		// Played when something is in contact with the pool and receiving health from it
	CSoundPatch		*GetAmbientSound(void);
	CSoundPatch		*GetHealingSound(void);
};

LINK_ENTITY_TO_CLASS(prop_xen_pool, CPropXenPool);
LINK_ENTITY_TO_CLASS(prop_xenpool, CPropXenPool);
LINK_ENTITY_TO_CLASS(xen_pool, CPropXenPool);
LINK_ENTITY_TO_CLASS(xenpool, CPropXenPool);

BEGIN_DATADESC(CPropXenPool)

DEFINE_FIELD(m_iCaps, FIELD_INTEGER),
DEFINE_FIELD(m_ishealingsomething_bool, FIELD_BOOLEAN),

// Sounds
DEFINE_SOUNDPATCH(m_ambient_sndpatch),
DEFINE_SOUNDPATCH(m_healing_sndpatch),

// Function Pointers
DEFINE_THINKFUNC(HealThink),

END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pkvd - 
//-----------------------------------------------------------------------------
bool CPropXenPool::KeyValue(const char *szKeyName, const char *szValue)
{
//	if (FStrEq(szKeyName, "dmdelay"))
//	{
//		m_iReactivate = atoi(szValue);
//		return(true);
//	}

	return(BaseClass::KeyValue(szKeyName, szValue));
}

void CPropXenPool::Precache(void)
{
	PrecacheModel("models/props_foliage_new/xen_pool.mdl");
	PrecacheModel("models/props_foliage_new/xen_pool_liquid.mdl");

	PrecacheScriptSound("XenPool.AmbientHum");
	PrecacheScriptSound("XenPool.HealingHum");
}

void CPropXenPool::Spawn(void)
{
	Precache();
	SetModel("models/props_foliage_new/xen_pool.mdl");

	SetMoveType(MOVETYPE_NONE);
	SetSolid(SOLID_VPHYSICS);
	
	AddEffects(EF_NOSHADOW);

	ResetSequence(LookupSequence("idle"));

	m_iCaps = FCAP_DONT_TRANSITION_EVER;
	CreateVPhysics();

	SetThink(&CPropXenPool::HealThink);

	SetNextThink(CURTIME + 0.2f);

	m_ishealingsomething_bool = false;

	if (GetAmbientSound())
	{
		(CSoundEnvelopeController::GetController()).Play(GetAmbientSound(), 0.0f, 75);
		(CSoundEnvelopeController::GetController()).SoundChangePitch(GetAmbientSound(), 75, 0.5f);
		(CSoundEnvelopeController::GetController()).SoundChangeVolume(GetAmbientSound(), 0.2f, 0.5f);
	}
	if (GetHealingSound())
	{
		(CSoundEnvelopeController::GetController()).Play(GetHealingSound(), 0.0f, 75);
		(CSoundEnvelopeController::GetController()).SoundChangePitch(GetHealingSound(), 100, 0.5f);
		(CSoundEnvelopeController::GetController()).SoundChangeVolume(GetHealingSound(), 0.0f, 0.5f);
	}

	if (m_nSkin == 0)
	{
		CBaseEntity *pMark = CreateEntityByName("xen_pool_liquid");
		pMark->SetOwnerEntity(this);
		pMark->Spawn();
	}
}

bool CPropXenPool::CreateVPhysics(void)
{
	VPhysicsInitStatic();
	return true;
}

void CPropXenPool::HealThink(void)
{
	m_ishealingsomething_bool = false;
	CBaseEntity *pHealEnt = NULL;
	CBroadcastRecipientFilter filter;
	while ((pHealEnt = gEntList.FindEntityInSphere(pHealEnt, GetAbsOrigin(), 32.0f)) != NULL)
	{
		if (pHealEnt->IsAlive() && (pHealEnt->IsPlayer() || pHealEnt->IsNPC()))
		{
			if (pHealEnt->GetHealth() < pHealEnt->GetMaxHealth())
			{
				int healthToGive = (SF_XENPOOL_FAST) ? 2 : 1;
				if (pHealEnt->IsNPC()) healthToGive *= 2; // heal NPCs faster, because they are generally weaker.
				pHealEnt->TakeHealth(healthToGive, DMG_GENERIC);
				m_ishealingsomething_bool = true;

				if (pHealEnt->MyCombatCharacterPointer() && pHealEnt->MyCombatCharacterPointer()->IsOnFire())
				{
					pHealEnt->MyCombatCharacterPointer()->Extinguish();
					pHealEnt->SetRenderColor(255, 255, 255);
				}

				pHealEnt->RemoveAllDecals();
			//	te->DynamicLight(filter, 0, &(GetAbsOrigin() + Vector(0, 0, 8)), 50, 170, 100, 0.1, 128, 0.5, 0.5);
			}
			else
			{
				m_ishealingsomething_bool = false;
			}
		}
	}

	if (GetAmbientSound())
	{
		(CSoundEnvelopeController::GetController()).SoundChangeVolume(GetAmbientSound(), m_ishealingsomething_bool ? 0.0f : 0.1f, 0.5f);
	}
	if (GetHealingSound())
	{
		(CSoundEnvelopeController::GetController()).SoundChangeVolume(GetHealingSound(), m_ishealingsomething_bool ? 0.8f : 0.0f, 0.5f);
	}
		
	SetNextThink(CURTIME + 0.5f);
}

CSoundPatch *CPropXenPool::GetAmbientSound(void)
{
	if (m_ambient_sndpatch == NULL)
	{
		CPASAttenuationFilter filter(this);
		m_ambient_sndpatch = (CSoundEnvelopeController::GetController()).SoundCreate(filter, entindex(), CHAN_STATIC, "XenPool.AmbientHum", ATTN_NORM);
	}

	return m_ambient_sndpatch;
}

CSoundPatch *CPropXenPool::GetHealingSound(void)
{
	if (m_healing_sndpatch == NULL)
	{
		CPASAttenuationFilter filter(this);
		m_healing_sndpatch = (CSoundEnvelopeController::GetController()).SoundCreate(filter, entindex(), CHAN_STATIC, "XenPool.HealingHum", ATTN_NORM);
	}

	return m_healing_sndpatch;
}