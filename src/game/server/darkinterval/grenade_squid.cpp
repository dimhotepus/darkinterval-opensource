//========================================================================//
//
// Purpose: 
//
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "ai_basenpc.h"
#include "grenade_squid.h"
#include "fire.h"
#include "soundent.h"
#include "decals.h"
#include "smoke_trail.h"
#include "hl2_shareddefs.h"
#include "vstdlib/random.h"
#include "engine/IEngineSound.h"
#include "particle_parse.h"
#include "particle_system.h"
#include "soundenvelope.h"
#include "ai_utils.h"
#include "te_effect_dispatch.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

static ConVar		sk_squid_spit_dmg("sk_squid_spit_dmg", "20");
static ConVar		sk_squid_spit_life_time("sk_squid_spit_life_time", "2.0");
//static ConVar		sk_squid_spit_radius("sk_squid_spit_radius", "40");
//static ConVar		sk_squid_spit_burn_time("sk_squid_spit_burn_time", "5.0");
ConVar				sk_squid_spit_speed("sk_squid_spit_speed", "600");

LINK_ENTITY_TO_CLASS(squid_spit, CSquidSpit);

BEGIN_DATADESC(CSquidSpit)

DEFINE_FIELD(m_bSizeBig, FIELD_BOOLEAN),
DEFINE_FIELD(m_flCreationTime, FIELD_TIME),
DEFINE_FIELD(m_hGasCloud, FIELD_EHANDLE),

// Function pointers
DEFINE_ENTITYFUNC(SquidSpitTouch),

END_DATADESC()

CSquidSpit::CSquidSpit(void) : m_bSizeBig(true)/*, m_pHissSound(NULL)*/
{
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSquidSpit::Precache(void)
{
	PrecacheModel(SPITBALLMODEL_LARGE);
	PrecacheModel(SPITBALLMODEL_SMALL);

	PrecacheScriptSound("NPC_Squid.SpitHit");

	PrecacheParticleSystem("antlion_spit_player");
	PrecacheParticleSystem("antlion_spit");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CSquidSpit::Spawn(void)
{
	Precache();
	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_FLYGRAVITY);
	SetSolidFlags(FSOLID_NOT_STANDABLE);

	SetModel(SPITBALLMODEL_LARGE);
	UTIL_SetSize(this, vec3_origin, vec3_origin);

	SetUse(&CBaseGrenade::DetonateUse);
	SetTouch(&CSquidSpit::SquidSpitTouch);
	SetNextThink(gpGlobals->curtime + 0.1f);

	m_flDamage = sk_squid_spit_dmg.GetFloat();
	m_DmgRadius = 40;
	m_takedamage = DAMAGE_NO;
	m_iHealth = 1;

	SetGravity(UTIL_ScaleForGravity(600));
	SetFriction(0.8f);

	SetCollisionGroup(HL2COLLISION_GROUP_SPIT);

	AddEFlags(EFL_FORCE_CHECK_TRANSMIT);

	// We're self-illuminating, so we don't take or give shadows
	AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);

	// Create the dust effect in place
	m_hSpitEffect = (CParticleSystem *)CreateEntityByName("info_particle_system");
	if (m_hSpitEffect != NULL)
	{
		// Setup our basic parameters
		m_hSpitEffect->KeyValue("start_active", "1");
		m_hSpitEffect->KeyValue("effect_name", "antlion_spit_trail");
		m_hSpitEffect->SetParent(this);
		m_hSpitEffect->SetLocalOrigin(vec3_origin);
		DispatchSpawn(m_hSpitEffect);
		if (gpGlobals->curtime > 0.5f)
			m_hSpitEffect->Activate();
	}

	m_flCreationTime = gpGlobals->curtime;
}

void CSquidSpit::SetSpitSize(int nSize)
{
	switch (nSize)
	{
	case SPIT_LARGE:
	{
		m_bSizeBig = true;
		SetModel(SPITBALLMODEL_LARGE);
		break;
	}
	case SPIT_SMALL:
	{
		m_bSizeBig = false;
		m_flDamage *= 0.25f;
		SetModel(SPITBALLMODEL_SMALL);
		break;
	}
	}
}

void CSquidSpit::Event_Killed(const CTakeDamageInfo &info)
{
	Detonate();
}

//-----------------------------------------------------------------------------
// Purpose: Handle spitting
//-----------------------------------------------------------------------------
void CSquidSpit::SquidSpitTouch(CBaseEntity *pOther)
{
	if (pOther->IsSolidFlagSet(FSOLID_VOLUME_CONTENTS | FSOLID_TRIGGER))
	{
		// Some NPCs are triggers that can take damage (like antlion grubs). We should hit them.
		if ((pOther->m_takedamage == DAMAGE_NO) || (pOther->m_takedamage == DAMAGE_EVENTS_ONLY))
			return;
	}

	// Don't hit other spit
	if (pOther->GetCollisionGroup() == HL2COLLISION_GROUP_SPIT)
		return;

	// We want to collide with water
	const trace_t *pTrace = &CBaseEntity::GetTouchTrace();

	// copy out some important things about this trace, because the first TakeDamage
	// call below may cause another trace that overwrites the one global pTrace points
	// at.
	bool bHitWater = ((pTrace->contents & CONTENTS_WATER) != 0);
//	CBaseEntity *const pTraceEnt = pTrace->m_pEnt;
	const Vector tracePlaneNormal = pTrace->plane.normal;

	if (bHitWater)
	{
		// Splash!
	//	CEffectData data;
	//	data.m_fFlags = 0;
	//	data.m_vOrigin = pTrace->endpos;
	//	data.m_vNormal = Vector(0, 0, 1);
	//	data.m_flScale = 8.0f;

	//	DispatchEffect("watersplash", data);
		// spawn fire on contact w/ water

		trace_t trace;
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, -128), MASK_SOLID_BRUSHONLY,
			this, COLLISION_GROUP_NONE, &trace);

		// Pull out of the wall a bit
		if (trace.fraction != 1.0)
		{
			SetLocalOrigin(trace.endpos + (trace.plane.normal * 0.6));
		}

		int i;
		QAngle vecTraceAngles;
		Vector vecTraceDir;
		trace_t firetrace;

		for (i = 0; i < 1; i++)
		{
			// build a little ray
			vecTraceAngles[PITCH] = random->RandomFloat(45, 135);
			vecTraceAngles[YAW] = random->RandomFloat(0, 360);
			vecTraceAngles[ROLL] = 0.0f;

			AngleVectors(vecTraceAngles, &vecTraceDir);

			Vector vecStart, vecEnd;

			vecStart = GetAbsOrigin() + (trace.plane.normal * 16);
			vecEnd = vecStart + vecTraceDir * 32;

			UTIL_TraceLine(vecStart, vecEnd, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &firetrace);

			if (firetrace.fraction != 1.0)
			{
				FireSystem_StartFire(firetrace.endpos, 0.2, 0.20, 5.0, (SF_FIRE_SMOKELESS), (CBaseEntity*) this, FIRE_NATURAL);
				//	DispatchParticleEffect( "env_fire_tiny", GetAbsOrigin(), GetAbsAngles(), this );
				//  UTIL_DecalTrace(&firetrace, "SmallScorch");
			}
		}

		SetThink(NULL);
		UTIL_Remove(this);
	}
	else
	{
	//	// Make a splat decal
	//	trace_t *pNewTrace = const_cast<trace_t*>(pTrace);
	//	UTIL_DecalTrace(pNewTrace, "BeerSplash");
	}

	CSoundEnt::InsertSound(SOUND_DANGER, GetAbsOrigin(), m_DmgRadius * 2.0f, 0.5f, GetThrower());

	QAngle vecAngles;
	VectorAngles(tracePlaneNormal, vecAngles);

	if (pOther->IsPlayer() )
	{
		// Do a lighter-weight effect if we just hit a player
		DispatchParticleEffect("antlion_spit_player", GetAbsOrigin(), vecAngles);
	}
	else
	{
		DispatchParticleEffect("antlion_spit", GetAbsOrigin(), vecAngles);
	}

	if (pOther->IsPlayer() || pOther->IsNPC() && pOther->IsAlive())
	{
		if (GetThrower() == NULL) return;
		CAI_BaseNPC *thrower = (CAI_BaseNPC*)(GetThrower());
		thrower->SetCondition(COND_ENEMY_VULNERABLE);
	}

	Detonate();
}

void CSquidSpit::Detonate(void)
{
	m_takedamage = DAMAGE_NO;

	EmitSound("NPC_Squid.SpitHit");
/*
	// Stop our hissing sound
	if (m_pHissSound != NULL)
	{
		CSoundEnvelopeController::GetController().SoundDestroy(m_pHissSound);
		m_pHissSound = NULL;
	}
*/
	if (m_hSpitEffect)
	{
		UTIL_Remove(m_hSpitEffect);
	}

	//========================
	//Gas cloud			
	if (m_bSizeBig)
	{
		Vector gasmins, gasmaxs;
		gasmins = Vector(-32, -32, -32); gasmaxs = Vector(32, 32, 32);
		m_hGasCloud = CToxicCloud::CreateGasCloud(GetLocalOrigin(), this, "squid_spit_gas", gpGlobals->curtime + 6.0f, TOXICCLOUD_ACID, gasmins, gasmaxs);
	}
	//========================

	UTIL_Remove(this);
}
/*
void CSquidSpit::InitHissSound(void)
{
	if (m_bSizeBig == false)
		return;

	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	if (m_pHissSound == NULL)
	{
		CPASAttenuationFilter filter(this);
		m_pHissSound = controller.SoundCreate(filter, entindex(), "NPC_Antlion.PoisonBall");
		controller.Play(m_pHissSound, 1.0f, 100);
	}
}
*/
void CSquidSpit::Think(void)
{
	if ((gpGlobals->curtime - m_flCreationTime) > sk_squid_spit_life_time.GetFloat())
	{
		DispatchParticleEffect("antlion_spit_player", GetAbsOrigin(), QAngle(-90, 0, 0));
		UTIL_Remove(this);
		SetThink(NULL);
	}
/*
	InitHissSound();
	if (m_pHissSound == NULL)
		return;

	// Add a doppler effect to the balls as they travel
	CBaseEntity *pPlayer = AI_GetSinglePlayer();
	if (pPlayer != NULL)
	{
		Vector dir;
		VectorSubtract(pPlayer->GetAbsOrigin(), GetAbsOrigin(), dir);
		VectorNormalize(dir);

		float velReceiver = DotProduct(pPlayer->GetAbsVelocity(), dir);
		float velTransmitter = -DotProduct(GetAbsVelocity(), dir);

		// speed of sound == 13049in/s
		int iPitch = 100 * ((1 - velReceiver / 13049) / (1 + velTransmitter / 13049));

		// clamp pitch shifts
		if (iPitch > 250)
		{
			iPitch = 250;
		}
		if (iPitch < 50)
		{
			iPitch = 50;
		}

		// Set the pitch we've calculated
		CSoundEnvelopeController::GetController().SoundChangePitch(m_pHissSound, iPitch, 0.1f);
	}
*/
	// Set us up to think again shortly
	SetNextThink(gpGlobals->curtime + 0.05f);
}
