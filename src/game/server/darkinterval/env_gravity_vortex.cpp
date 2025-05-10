//========= Dark Interval, 2025. ==============================================//
//
// Purpose: The gravity vortex is a reworked hopwire grenade vortex made to 
// be used with particle storms and as a standalone env entity. 
// It can pull in physical objects and players and consume their mass to create
// a densified prop.
//=============================================================================//

#include "cbase.h"
#include "ai_basenpc.h"
#include "env_gravity_vortex.h"
#include "filters.h"
#include "_monsters_encompass.h"
#include "physics_prop_ragdoll.h"
#include "player.h"
#include "tier0\memdbgon.h"

// TODO:
// - fov/frustum option, LOS check option
// - visual FX options, refract and funnel
// - ability to spawn items instead of prop physics as the dense ball
// - parenting (replace ownership with?)
// - ent_text detailed info
// - sounds: turn on, turn off, consume prop, spawn dense prop, looping sound for pulling (separate for inverse)
// - outputs

LINK_ENTITY_TO_CLASS(env_gravity_vortex, CEnvGravityVortex);
//-----------------------------------------------------------------------------
// Purpose: Returns the amount of mass consumed by the vortex
//-----------------------------------------------------------------------------
float CEnvGravityVortex::GetConsumedMass(void) const
{
	return m_mass_float;
}

//-----------------------------------------------------------------------------
// Purpose: Creation utility
//-----------------------------------------------------------------------------
CEnvGravityVortex *CEnvGravityVortex::Create(const Vector &origin, float innerRadius, float outerRadius, float strength, float duration, CBaseEntity *pOwner)
{
	if (pOwner == NULL)
		return NULL;

	// Create an instance of the vortex
	CEnvGravityVortex *pVortex = (CEnvGravityVortex *)CreateEntityByName("env_gravity_vortex");
	if (pVortex == NULL)
		return NULL;

	pVortex->SetOwnerEntity(pOwner);

	// Start the vortex working
	pVortex->m_enabled_bool = true;
	pVortex->StartPull(origin, innerRadius, outerRadius, strength, duration);

	return pVortex;
}

void CEnvGravityVortex::Precache(void)
{
	PrecacheModel(STRING(GetModelName()));
	BaseClass::Precache();
}

void CEnvGravityVortex::Spawn(void)
{
	char *szModel = ( char * )STRING(GetModelName());
	if ( !szModel || !*szModel )
	{
		Warning("%s at (%.3f, %.3f, %.3f) has no model name!\n", GetClassname(), GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);
		szModel = "models/props_junk/metal_paintcan001b.mdl";
		SetModelName(AllocPooledString(szModel));
	}

	Precache();
	SetModel(STRING(GetModelName()));
	SetRenderMode(kRenderNone);
	AddEffects(EF_NOSHADOW);

	// We don't want this to be solid, because we don't want it to collide with the hydra.
	SetSolid(SOLID_VPHYSICS);
	AddSolidFlags(FSOLID_NOT_SOLID);
	VPhysicsInitShadow(true, true);

	// Disable movement on this sucker, we're going to move him manually
	SetMoveType(MOVETYPE_FLY);

	if (m_enabled_bool)
	{
		StartPull(GetAbsOrigin(), m_inner_radius_float, m_outer_radius_float,
			m_strength_float, m_duration_float);
	}

	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Pulls physical objects towards the vortex center, killing them if they come too near
//-----------------------------------------------------------------------------
void CEnvGravityVortex::PullThink(void)
{
	if (!m_enabled_bool || CURTIME > m_end_time)
	{
		StopPull(true);
	}

	if (GetOwnerEntity())
	{
		SetAbsOrigin(GetOwnerEntity()->GetAbsOrigin());
	}

	if (m_vortex_flags_bit & VORTEX_PLAYER_PULL)
	{
		// Pull any players close enough to us
		PullPlayersInRange();
	}

//	if ( ( m_vortex_flags_bit & VORTEX_PROP_PULL ) == false && ( m_vortex_flags_bit & VORTEX_NPC_PULL ) == false )
//		return;

	Vector mins, maxs;
	mins = GetAbsOrigin() - Vector(m_outer_radius_float, m_outer_radius_float, m_outer_radius_float);
	maxs = GetAbsOrigin() + Vector(m_outer_radius_float, m_outer_radius_float, m_outer_radius_float);

	const int size = 512;
	CBaseEntity *pEnts[ size ];
	int numEnts = UTIL_EntitiesInSphere(pEnts, size, GetAbsOrigin(), m_outer_radius_float, 0); // UTIL_EntitiesInBox(pEnts, 512, mins, maxs, 0);

	for (int i = 0; i < numEnts; i++)
	{
		IPhysicsObject *pPhysObject = NULL;
		// Attempt to kill and ragdoll any victims in range
		if ( KillNPCInRange(pEnts[ i ], &pPhysObject) == false )
		{
			// If we didn't have a valid victim, see if we can just get the vphysics object
			pPhysObject = pEnts[ i ]->VPhysicsGetObject();
			if ( pPhysObject == NULL )
				continue;
		}

		if ( m_vortex_flags_bit & VORTEX_PROP_PULL )
		{
			// don't affect props with motion disabled
			if ( pEnts[ i ]->VPhysicsGetObject() && pEnts[ i ]->VPhysicsGetObject()->IsMotionEnabled() == false )
			{
				continue;
			}
			float mass;
			CRagdollProp *pRagdoll = dynamic_cast< CRagdollProp* >( pEnts[ i ] );
			if ( pRagdoll != NULL )
			{
				if ( pRagdoll->GetOwnerEntity() && pRagdoll->GetOwnerEntity()->IsNPC() && pRagdoll->GetOwnerEntity()->IsAlive() )
					continue; // such is the case with live ragdolls. Don't drag them in (CH08_JUNKYARD)

				ragdoll_t *pRagdollPhys = pRagdoll->GetRagdoll();
				mass = 0.0f;

				// Find the aggregate mass of the whole ragdoll
				for ( int j = 0; j < pRagdollPhys->listCount; ++j )
				{
					mass += pRagdollPhys->list[ j ].pObject->GetMass();
				}
			} else
			{
				mass = pPhysObject->GetMass();
			}

			Vector	vecForce = GetAbsOrigin() - pEnts[ i ]->WorldSpaceCenter();
			Vector	vecForce2D = vecForce;
			vecForce2D[ 2 ] = 0.0f;
			float	dist2D = VectorNormalize(vecForce2D);
			float	dist = VectorNormalize(vecForce);

			if ( GetOwnerEntity() && pEnts[ i ] == GetOwnerEntity() )
			{
				continue;
			}
			
			// scale the objects gravity
			if ( dist < m_outer_radius_float - 64 )
			{
			//	pEnts[ i ]->SetGravity(m_pull_gravity_scale_float);
			}
			else
			{
			//	pEnts[ i ]->SetGravity(1.0f);
			}

			// FIXME: Need a more deterministic method here
			if ( dist < m_inner_radius_float )
			{
				if ( m_vortex_flags_bit & VORTEX_PROP_KILL )
				{
					ConsumeEntity(pEnts[ i ]);
					continue;
				}

				if( m_negate_inner_forces_bool)				
					vecForce *= -1;
			}

			// Must be within the radius
			if ( dist > m_outer_radius_float )
				continue;

			// Find the pull force
			vecForce *= ( 1.0f - ( dist2D / m_outer_radius_float ) ) * m_strength_float * mass;

			if ( pEnts[ i ]->VPhysicsGetObject() )
			{
				// Pull the object in
				pEnts[ i ]->VPhysicsTakeDamage(CTakeDamageInfo(this, this, vecForce, GetAbsOrigin(), m_strength_float, DMG_BLAST));
			}
		}
	}

	SetNextThink(CURTIME + 0.1f);
}

//-----------------------------------------------------------------------------
// Purpose: Adds the entity's mass to the aggregate mass consumed
//-----------------------------------------------------------------------------
void CEnvGravityVortex::ConsumeEntity(CBaseEntity *pEnt)
{
	if (pEnt->IsPlayer())
		return;

	if (pEnt->ClassMatches(GetClassName()))
		return; // don't suck in other gravity vortices

	if ( pEnt->IsNPC() && ( m_vortex_flags_bit & VORTEX_NPC_KILL ) == false )
		return; // may happen even without sucking in NPCs, if they walk past the inner radius

	//	if (pEnt->IsBSPModel())
	//		return;

	//	if (m_filter_handle.Get() != NULL && !(m_filter_handle->PassesFilter(this, pEnt)))
	//		return;

	if (GetOwnerEntity() && pEnt == GetOwnerEntity())
		return;

	// Get our base physics object
	IPhysicsObject *pPhysObject = pEnt->VPhysicsGetObject();
	if (pPhysObject == NULL)
		return;

	// Ragdolls need to report the sum of all their parts
	CRagdollProp *pRagdoll = dynamic_cast< CRagdollProp* >(pEnt);
	if (pRagdoll != NULL)
	{
		// Find the aggregate mass of the whole ragdoll
		ragdoll_t *pRagdollPhys = pRagdoll->GetRagdoll();
		for (int j = 0; j < pRagdollPhys->listCount; ++j)
		{
			m_mass_float += pRagdollPhys->list[j].pObject->GetMass();
		}
	}
	else
	{
		// Otherwise we just take the normal mass
		m_mass_float += pPhysObject->GetMass();
	}

	/*
	CEffectData	data;
	data.m_nEntIndex = entindex();
	data.m_nAttachmentIndex = 0;
	data.m_vOrigin = pEnt->GetAbsOrigin();
	data.m_flScale = 5;
	DispatchEffect("TeslaZap", data);
	*/

	// Destroy the entity
	//	if( pEnt->GetBaseAnimating() && !(pEnt->GetBaseAnimating()->IsDissolving()))
	//		pEnt->GetBaseAnimating()->Dissolve(NULL, CURTIME, false, ENTITY_DISSOLVE_ELECTRICAL);
	//	else
	UTIL_Remove(pEnt);

	if (m_mass_float >= m_maxmass_float)
		CreateDenseProp();
}

//-----------------------------------------------------------------------------
// Purpose: Causes players within the radius to be sucked in
//-----------------------------------------------------------------------------
void CEnvGravityVortex::PullPlayersInRange(void)
{
	CBasePlayer *player = UTIL_GetLocalPlayer();

	Vector	vecForce = GetAbsOrigin() - player->WorldSpaceCenter();
	float	dist = VectorNormalize(vecForce);

	// FIXME: Need a more deterministic method here
	if (dist < m_inner_radius_float)
	{
		if ( m_vortex_flags_bit & VORTEX_PLAYER_KILL )
		{
			player->TakeHealth(-50, DMG_CRUSH);
		}
		if (GetOwnerEntity())
		{
			CNPC_ParticleStorm *stormOwner = dynamic_cast<CNPC_ParticleStorm*>(GetOwnerEntity());
			if (stormOwner)
			{
				stormOwner->SetPlayerIsCaught(true);
			}
		}
	}

	// Must be within the radius
	if (dist > m_outer_radius_float)
		return;

	float mass = player->VPhysicsGetObject()->GetMass();
	float playerForce = m_strength_float * 0.05f;

	// Find the pull force
	// NOTE: We might want to make this non-linear to give more of a "grace distance"
	vecForce *= (1.0f - (dist / m_outer_radius_float)) * playerForce * mass;
	vecForce[2] *= 20;

	player->SetBaseVelocity(vecForce);
	player->AddFlag(FL_BASEVELOCITY);

	// Make sure the player moves
	//	if (/*vecForce.z > 0 &&*/ (player->GetFlags() & FL_ONGROUND))
	{
		player->SetGroundEntity(NULL);
		player->VelocityPunch(Vector(0, 0, 32));
	}
}

//-----------------------------------------------------------------------------
// Purpose: Attempts to kill an NPC if it's within range and other criteria
// Input  : *pVictim - NPC to assess
//			**pPhysObj - pointer to the ragdoll created if the NPC is killed
// Output :	bool - whether or not the NPC was killed and the returned pointer is valid
//-----------------------------------------------------------------------------
bool CEnvGravityVortex::KillNPCInRange(CBaseEntity *pVictim, IPhysicsObject **pPhysObj)
{
	if (!(m_vortex_flags_bit & VORTEX_NPC_KILL))
	{
		return false;
	}

	CBaseCombatCharacter *pBCC = pVictim->MyCombatCharacterPointer();
	CAI_BaseNPC *ownerNPC = NULL;
	if (GetOwnerEntity()) ownerNPC = dynamic_cast<CAI_BaseNPC*>(GetOwnerEntity());

	// See if we can ragdoll
	if (pBCC != NULL && pBCC->CanBecomeRagdoll()
		&& (m_filter_handle == NULL || (m_filter_handle.Get() && m_filter_handle->PassesFilter(this, pBCC))))
	{
		// don't affect the owner
		if ( GetOwnerEntity() && pBCC == GetOwnerEntity() )
			return false;

		// Don't bother with striders
		if (FClassnameIs(pBCC, "npc_strider"))
			return false;

		// Don't bother with other pstorms
		if (FClassnameIs(pBCC, "npc_pstorm"))
			return false;

		// TODO: Make this an interaction between the NPC and the vortex

		// Become ragdoll
		CTakeDamageInfo info(this, this, 1.0f, DMG_GENERIC);
		CBaseEntity *pRagdoll = CreateServerRagdoll(pBCC, 0, info, COLLISION_GROUP_INTERACTIVE_DEBRIS, true);
		pRagdoll->SetCollisionBounds(pVictim->CollisionProp()->OBBMins(), pVictim->CollisionProp()->OBBMaxs());

		// Necessary to cause it to do the appropriate death cleanup
		CTakeDamageInfo ragdollInfo(this, this, 10000.0, DMG_GENERIC | DMG_REMOVENORAGDOLL);
		pVictim->TakeDamage(ragdollInfo);

		// Return the pointer to the ragdoll
		*pPhysObj = pRagdoll->VPhysicsGetObject();
		return true;
	}

	// Wasn't able to ragdoll this target
	*pPhysObj = NULL;
	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Creates a dense ball with a mass equal to the aggregate mass consumed by the vortex
//-----------------------------------------------------------------------------
void CEnvGravityVortex::CreateDenseProp(void)
{
	if (m_dense_prop_limit_int == 0) return;

	CBaseEntity *pBall = CreateEntityByName("prop_physics");

	pBall->SetModel(GetModelName().ToCStr());
	pBall->SetAbsOrigin(GetAbsOrigin());
	pBall->Spawn();

	IPhysicsObject *pObj = pBall->VPhysicsGetObject();
	if (pObj != NULL)
	{
		pObj->SetMass(GetConsumedMass());
	}

	m_mass_float = 0; // reset the mass
	if( m_dense_prop_limit_int > 0)
		m_dense_prop_limit_int -= 1;
	
	StopPull(true);
}

//-----------------------------------------------------------------------------
// Purpose: Starts the vortex working
//-----------------------------------------------------------------------------
void CEnvGravityVortex::StartPull(const Vector &origin, float innerRadius, float outerRadius, float strength, float duration)
{
	SetAbsOrigin(origin);
	m_end_time = CURTIME + duration;
	m_inner_radius_float = innerRadius;
	m_strength_float = strength;

	// Start our client-side effect
	EntityMessageBegin(this, true);
	WRITE_BYTE(0);
	MessageEnd();

	SetThink(&CEnvGravityVortex::PullThink);
	SetNextThink(CURTIME + 0.1f);
}

void CEnvGravityVortex::StopPull(bool doFx)
{
	if ( doFx )
	{
		EntityMessageBegin(this, true);
		WRITE_BYTE(1);
		MessageEnd();
	}

	if ( m_vortex_flags_bit & VORTEX_STOP_REMOVE )
	{
		SetThink(&CBaseEntity::SUB_Remove);
		SetNextThink(CURTIME + 1.0f);
	}

	SetThink(NULL);
	SetNextThink(CURTIME);
}

void CEnvGravityVortex::InputTurnOn(inputdata_t &inputdata)
{
	m_enabled_bool = true;
	StartPull(GetAbsOrigin(), m_inner_radius_float, m_outer_radius_float,
		m_strength_float, m_duration_float);
}

void CEnvGravityVortex::InputTurnOff(inputdata_t &inputdata)
{
	m_enabled_bool = false;
}

void CEnvGravityVortex::InputSetVortexInnerRadius(inputdata_t &inputdata)
{
	SetVortexInnerRadius(inputdata.value.Float());
}

void CEnvGravityVortex::InputSetVortexOuterRadius(inputdata_t &inputdata)
{
	SetVortexOuterRadius(inputdata.value.Float());
}

void CEnvGravityVortex::InputSetVortexDuration(inputdata_t &inputdata)
{
	SetVortexDuration(inputdata.value.Float());
}

void CEnvGravityVortex::InputSetVortexStrength(inputdata_t &inputdata)
{
	SetVortexStrength(inputdata.value.Float());
}

void CEnvGravityVortex::InputSetVortexMaxMass(inputdata_t &inputdata)
{
	SetVortexMaxMass(inputdata.value.Float());
}

void CEnvGravityVortex::InputSetVortexDensePropLimit(inputdata_t &inputdata)
{
	SetVortexDensePropLimit(inputdata.value.Int());
}

void CEnvGravityVortex::InputSetVortexPlayerPull(inputdata_t &inputdata)
{
	SetVortexPlayerPull(inputdata.value.Bool());
}

void CEnvGravityVortex::InputSetVortexPlayerKill(inputdata_t &inputdata)
{
	SetVortexPlayerKill(inputdata.value.Bool());
}

void CEnvGravityVortex::InputSetVortexPropPull(inputdata_t &inputdata)
{
	SetVortexPropPull(inputdata.value.Bool());
}

void CEnvGravityVortex::InputSetVortexPropKill(inputdata_t &inputdata)
{
	SetVortexPropKill(inputdata.value.Bool());
}

void CEnvGravityVortex::InputSetVortexNPCPull(inputdata_t &inputdata)
{
	SetVortexNPCPull(inputdata.value.Bool());
}

void CEnvGravityVortex::InputSetVortexNPCKill(inputdata_t &inputdata)
{
	SetVortexNPCKill(inputdata.value.Bool());
}

void CEnvGravityVortex::InputSetVortexRemoveOnStop(inputdata_t &inputdata)
{
	SetVortexRemoveOnStop(inputdata.value.Bool());
}

void CEnvGravityVortex::InputSetVortexGravityScale(inputdata_t &inputdata)
{
	SetVortexGravityScale(inputdata.value.Float());
}

BEGIN_DATADESC(CEnvGravityVortex)
	DEFINE_THINKFUNC(PullThink),
	DEFINE_KEYFIELD(m_enabled_bool, FIELD_BOOLEAN, "StartEnabled"),
	DEFINE_KEYFIELD(m_dense_prop_limit_int, FIELD_INTEGER, "DensePropLimit"),
	DEFINE_KEYFIELD(m_maxmass_float, FIELD_FLOAT, "MaxMass"),
	DEFINE_KEYFIELD(m_inner_radius_float, FIELD_FLOAT, "InnerRadius"),
	DEFINE_KEYFIELD(m_outer_radius_float, FIELD_FLOAT, "OuterRadius"),
	DEFINE_KEYFIELD(m_strength_float, FIELD_FLOAT, "PullStrength"),
	DEFINE_KEYFIELD(m_duration_float, FIELD_FLOAT, "PullDuration"),
	DEFINE_KEYFIELD(m_pull_gravity_scale_float, FIELD_FLOAT, "GravityScale"),
	DEFINE_KEYFIELD(m_negate_inner_forces_bool, FIELD_BOOLEAN, "NegateInnerForces"),
	DEFINE_FIELD(m_filter_handle, FIELD_EHANDLE),
	DEFINE_FIELD(m_mass_float, FIELD_FLOAT),
	DEFINE_FIELD(m_end_time, FIELD_TIME),
	DEFINE_FIELD(m_vortex_flags_bit, FIELD_INTEGER),
	DEFINE_INPUTFUNC(FIELD_VOID, "TurnVortexOn", InputTurnOn),
	DEFINE_INPUTFUNC(FIELD_VOID, "TurnVortexOff", InputTurnOff),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetVortexInnerRadius", InputSetVortexInnerRadius),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetVortexOuterRadius", InputSetVortexOuterRadius),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetVortexDuration", InputSetVortexDuration),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetVortexStrength", InputSetVortexStrength),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetVortexMaxMass", InputSetVortexMaxMass),
	DEFINE_INPUTFUNC(FIELD_BOOLEAN, "SetVortexPlayerPull", InputSetVortexPlayerPull),
	DEFINE_INPUTFUNC(FIELD_BOOLEAN, "SetVortexPlayerKill", InputSetVortexPlayerKill),
	DEFINE_INPUTFUNC(FIELD_BOOLEAN, "SetVortexPropPull", InputSetVortexPropPull),
	DEFINE_INPUTFUNC(FIELD_BOOLEAN, "SetVortexPropKill", InputSetVortexPropKill),
	DEFINE_INPUTFUNC(FIELD_BOOLEAN, "SetVortexNPCPull", InputSetVortexNPCPull),
	DEFINE_INPUTFUNC(FIELD_BOOLEAN, "SetVortexNPCKill", InputSetVortexNPCKill),
	DEFINE_INPUTFUNC(FIELD_BOOLEAN, "SetVortexRemoveOnStop", InputSetVortexRemoveOnStop),
	DEFINE_INPUTFUNC(FIELD_INTEGER, "SetVortexDensePropLimit", InputSetVortexDensePropLimit),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetVortexGravityScale", InputSetVortexGravityScale),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CEnvGravityVortex, DT_EnvGravityVortex)
END_SEND_TABLE()