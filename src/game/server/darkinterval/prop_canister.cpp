#include "cbase.h"
#include "ai_basenpc.h" // for stun variety
#include "env_gravity_vortex.h" // for black hole variety
#include "env_explosion.h" // for default variety
#include "props.h"
#include "grenade_toxiccloud.h" // for toxic variety
#include "vcollide_parse.h"
#include "ieffects.h"
#include "materialsystem/imaterialsystemhardwareconfig.h"
#include "te_effect_dispatch.h"

class CPropCanister : public CPhysicsProp
{
public:
	CPropCanister();
	~CPropCanister();
	void Precache(void);
	void Spawn(void);
	bool CreateVPhysics(void);
//	void OnTakeDamage(CTakeDamageInfo &info) {};
	void Event_Killed(const CTakeDamageInfo &info);
	void BreakCanister(CBaseEntity *pBreaker, const CTakeDamageInfo &info);

	DECLARE_CLASS(CPropCanister, CPhysicsProp);
	DECLARE_DATADESC();
	int m_canister_type_int;
	float m_explosion_radius_float;
	float m_explosion_damage_float;
private:
	enum ExploType_t
	{
		EXPT_DEFAULT, // explosive barrel
		EXPT_CRYO, // cryo explosion with freeze damage and NPC freezing
		EXPT_PLASMA, // vorticell storage explosion
		EXPT_ION, // electrical discharge
		EXPT_TOXIN, // synth scanner gas attack-esque
		EXPT_STUN, // causes NPCs to fall in live ragdolls
		EXPT_BLACKHOLE, // very short burst of gravity
	};

	void ExplodeDefault(Vector &origin, QAngle &angles, const CTakeDamageInfo &info);
	void ExplodeCryo(Vector &origin, QAngle &angles, const CTakeDamageInfo &info);
	void ExplodePlasma(Vector &origin, QAngle &angles, const CTakeDamageInfo &info);
	void ExplodeIon(Vector &origin, QAngle &angles, const CTakeDamageInfo &info);
	void ExplodeToxin(Vector &origin, QAngle &angles, const CTakeDamageInfo &info);
	void ExplodeStun(Vector &origin, QAngle &angles, const CTakeDamageInfo &info);
	void ExplodeBlackHole(Vector &origin, QAngle &angles, const CTakeDamageInfo &info);
};

LINK_ENTITY_TO_CLASS(prop_canister, CPropCanister);
LINK_ENTITY_TO_CLASS(prop_cannister, CPropCanister);
LINK_ENTITY_TO_CLASS(prop_explosive_canister, CPropCanister);
LINK_ENTITY_TO_CLASS(prop_explosive_cannister, CPropCanister);
LINK_ENTITY_TO_CLASS(prop_explosivecanister, CPropCanister);
LINK_ENTITY_TO_CLASS(prop_explosivecannister, CPropCanister);

CPropCanister::CPropCanister()
{
}

CPropCanister::~CPropCanister()
{
}

void CPropCanister::Precache(void)
{
	if (GetModelName() == NULL_STRING)
	{
		Msg("%s at (%.3f, %.3f, %.3f) has no model name!\n", GetClassname(), GetAbsOrigin().x, GetAbsOrigin().y, GetAbsOrigin().z);
		SetModelName(AllocPooledString("models/error.mdl"));
	}

	PrecacheModel(STRING(GetModelName()));

	switch (m_canister_type_int)
	{
	case EXPT_CRYO:
	{
		PrecacheScriptSound("IceBody.Shatter");
		PrecacheParticleSystem("freeze_statue_shatter");
	}
	break;
	case EXPT_TOXIN:
	{
		UTIL_PrecacheOther("toxic_cloud");
	}
	break;
	case EXPT_STUN:
	{
		PrecacheParticleSystem("biospore_explosion_core");
	}
	break;
	default:
		break;
	}
	BaseClass::Precache();
}

void CPropCanister::Spawn(void)
{
	Precache();
	BaseClass::Spawn();

	CreateVPhysics();

	if (VPhysicsGetObject())
	{
		SetSolid(SOLID_VPHYSICS);
		SetMoveType(MOVETYPE_VPHYSICS);
	}
	else
	{
		SetSolid(SOLID_BBOX);
		SetMoveType(MOVETYPE_PUSH);
	}
	SetSolidFlags(FSOLID_NOT_STANDABLE);

	m_takedamage = DAMAGE_YES;
}

bool CPropCanister::CreateVPhysics(void)
{
	// Create the object in the physics system
	bool asleep = HasSpawnFlags(SF_PHYSPROP_START_ASLEEP) ? true : false;

	solid_t tmpSolid;
	PhysModelParseSolid(tmpSolid, this, GetModelIndex());
/*
	if (m_massScale > 0)
	{
		tmpSolid.params.mass *= m_massScale;
	}

	if (m_inertiaScale > 0)
	{
		tmpSolid.params.inertia *= m_inertiaScale;
		if (tmpSolid.params.inertia < 0.5)
			tmpSolid.params.inertia = 0.5;
	}
*/
	PhysGetMassCenterOverride(this, modelinfo->GetVCollide(GetModelIndex()), tmpSolid);
	if (HasSpawnFlags(SF_PHYSPROP_NO_COLLISIONS))
	{
		tmpSolid.params.enableCollisions = false;
	}

//	PhysSolidOverride(tmpSolid, m_iszOverrideScript);

	IPhysicsObject *pPhysicsObject = VPhysicsInitNormal(SOLID_VPHYSICS, 0, asleep, &tmpSolid);

	if (!pPhysicsObject)
	{
		SetSolid(SOLID_NONE);
		SetMoveType(MOVETYPE_NONE);
		Warning("ERROR!: Can't create physics object for %s\n", STRING(GetModelName()));
	}
	else
	{
	//	if (m_damageType == 1)
	//	{
	//		PhysSetGameFlags(pPhysicsObject, FVPHYSICS_DMG_SLICE);
	//	}
		if (HasSpawnFlags(SF_PHYSPROP_MOTIONDISABLED) /*|| m_damageToEnableMotion > 0 || m_flForceToEnableMotion > 0*/)
		{
			pPhysicsObject->EnableMotion(false);
		}
	}

	// fix up any noncompliant blades.
	if (HasInteraction(PROPINTER_PHYSGUN_LAUNCH_SPIN_Z))
	{
		if (!(VPhysicsGetObject()->GetGameFlags() & FVPHYSICS_DMG_SLICE))
		{
			PhysSetGameFlags(pPhysicsObject, FVPHYSICS_DMG_SLICE);

#if 0
			if (g_pDeveloper->GetInt())
			{
				// Highlight them in developer mode.
				m_debugOverlays |= (OVERLAY_TEXT_BIT | OVERLAY_BBOX_BIT);
			}
#endif
		}
	}

	if (HasInteraction(PROPINTER_PHYSGUN_DAMAGE_NONE))
	{
		PhysSetGameFlags(pPhysicsObject, FVPHYSICS_NO_IMPACT_DMG);
	}

	if (HasSpawnFlags(SF_PHYSPROP_PREVENT_PICKUP))
	{
		PhysSetGameFlags(pPhysicsObject, FVPHYSICS_NO_PLAYER_PICKUP);
	}

	return true;
}

void CPropCanister::Event_Killed(const CTakeDamageInfo &info)
{
	IPhysicsObject *pPhysics = VPhysicsGetObject();
	if (pPhysics && !pPhysics->IsMoveable())
	{
		pPhysics->EnableMotion(true);
		VPhysicsTakeDamage(info);
	}
	BreakCanister(info.GetInflictor(), info);
}

void CPropCanister::BreakCanister(CBaseEntity *pBreaker, const CTakeDamageInfo &info)
{
	IGameEvent * event = gameeventmanager->CreateEvent("break_prop");

	if (event)
	{
		if (pBreaker && pBreaker->IsPlayer())
		{
			event->SetInt("userid", ToBasePlayer(pBreaker)->GetUserID());
		}
		else
		{
			event->SetInt("userid", 0);
		}
		event->SetInt("entindex", entindex());
		gameeventmanager->FireEvent(event);
	}

	m_takedamage = DAMAGE_NO;
	m_OnBreak.FireOutput(pBreaker, this);

	Vector velocity;
	AngularImpulse angVelocity;
	IPhysicsObject *pPhysics = GetRootPhysicsObjectForBreak();

	Vector origin;
	QAngle angles;
	AddSolidFlags(FSOLID_NOT_SOLID);
	if (pPhysics)
	{
		pPhysics->GetVelocity(&velocity, &angVelocity);
		pPhysics->GetPosition(&origin, &angles);
		pPhysics->RecheckCollisionFilter();
	}
	else
	{
		velocity = GetAbsVelocity();
		QAngleToAngularImpulse(GetLocalAngularVelocity(), angVelocity);
		origin = GetAbsOrigin();
		angles = GetAbsAngles();
	}

	PhysBreakSound(this, VPhysicsGetObject(), GetAbsOrigin());
	
	CBaseEntity *pAttacker = info.GetAttacker();
	/*
	if (m_hLastAttacker)
	{
		// Pass along the person who made this explosive breakable explode.
		// This way the player allies can get immunity from barrels exploded by the player.
		pAttacker = m_hLastAttacker;
	}
	else if (m_hPhysicsAttacker)
	{
		// If I have a physics attacker and was influenced in the last 2 seconds,
		// Make the attacker my physics attacker. This helps protect citizens from dying
		// in the explosion of a physics object that was thrown by the player's physgun
		// and exploded on impact.
		if (CURTIME - m_flLastPhysicsInfluenceTime <= 2.0f)
		{
			pAttacker = m_hPhysicsAttacker;
		}
	}
	*/
	
	// Allow derived classes to emit special things
	OnBreak(velocity, angVelocity, pBreaker);

	breakablepropparams_t params(origin, angles, velocity, angVelocity);
	params.impactEnergyScale = m_impactEnergyScale;
	params.defCollisionGroup = GetCollisionGroup();
	if (params.defCollisionGroup == COLLISION_GROUP_NONE)
	{
		// don't automatically make anything COLLISION_GROUP_NONE or it will
		// collide with debris being ejected by breaking
		params.defCollisionGroup = COLLISION_GROUP_INTERACTIVE;
	}
	params.defBurstScale = 100;

	if (m_iszBreakModelMessage != NULL_STRING)
	{
		CPVSFilter filter(GetAbsOrigin());
		UserMessageBegin(filter, STRING(m_iszBreakModelMessage));
		WRITE_SHORT(GetModelIndex());
		WRITE_VEC3COORD(GetAbsOrigin());
		WRITE_ANGLES(GetAbsAngles());
		MessageEnd();
		UTIL_Remove(this);
		return;
	}
	
	// no damage/damage force? set a burst of 100 for some movement
	else if (m_PerformanceMode != PM_NO_GIBS)
	{
		PropBreakableCreateAll(GetModelIndex(), pPhysics, params, this, -1, (m_PerformanceMode == PM_FULL_GIBS));
	}

	switch (m_canister_type_int)
	{
	case EXPT_CRYO:
		ExplodeCryo(origin, angles, info);
		break;
	case EXPT_PLASMA:
		ExplodePlasma(origin, angles, info);
		break;
	case EXPT_ION:
		ExplodeIon(origin, angles, info);
		break;
	case EXPT_TOXIN:
		ExplodeToxin(origin, angles, info);
		break;
	case EXPT_STUN:
		ExplodeStun(origin, angles, info);
		break;
	case EXPT_BLACKHOLE:
		ExplodeBlackHole(origin, angles, info);
		break;
	default:
		ExplodeDefault(origin, angles, info);
		break;
	}

	UTIL_Remove(this);
}

void CPropCanister::ExplodeDefault(Vector &origin, QAngle &angles, const CTakeDamageInfo &info)
{
	ExplosionCreate(origin, angles, info.GetAttacker(), m_explosion_damage_float, m_explosion_radius_float, SF_ENVEXPLOSION_NOSPARKS | SF_ENVEXPLOSION_NOSMOKE, 0.0f, this);

	// Find and ignite all NPC's within the radius
	CBaseEntity *pEntity = NULL;
	for (CEntitySphereQuery sphere(origin, m_explosion_radius_float); (pEntity = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity())
	{
		if (pEntity && pEntity->MyCombatCharacterPointer())
		{
			// Check damage filters so we don't ignite friendlies
			if (pEntity->PassesDamageFilter(info))
			{
				pEntity->MyCombatCharacterPointer()->Ignite(30);
			}
		}
	}
}

void CPropCanister::ExplodeCryo(Vector &origin, QAngle &angles, const CTakeDamageInfo &info)
{
	CPASAttenuationFilter filter2(this, "IceBody.Shatter");
	EmitSound(filter2, entindex(), "IceBody.Shatter");

	DispatchParticleEffect("freeze_statue_shatter", GetAbsOrigin(), GetAbsAngles(), NULL);

	// Find and freeze all NPC's within the radius
	CBaseEntity *pEntity = NULL;
	for (CEntitySphereQuery sphere(origin, m_explosion_radius_float * GetModelScale()); (pEntity = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity())
	{
		if (pEntity && pEntity->MyCombatCharacterPointer())
		{
			// Check damage filters so we don't ignite friendlies
			if (pEntity->PassesDamageFilter(info))
			{
				//	CTakeDamageInfo info_energy(pAttacker, pAttacker, pEntity->GetMaxHealth() * GetModelScale(), DMG_FREEZE);
				//	pEntity->TakeDamage(info_energy);
				pEntity->MyCombatCharacterPointer()->Freeze(m_explosion_damage_float);
			}
		}
	}
}

void CPropCanister::ExplodePlasma(Vector &origin, QAngle &angles, const CTakeDamageInfo &info)
{
}

void CPropCanister::ExplodeIon(Vector &origin, QAngle &angles, const CTakeDamageInfo &info)
{
}

void CPropCanister::ExplodeToxin(Vector &origin, QAngle &angles, const CTakeDamageInfo &info)
{
	CToxicCloud *cloud = CToxicCloud::CreateGasCloud(origin, this, "synth_scanner_gas", 
		CURTIME + m_explosion_damage_float, TOXICCLOUD_ACID, m_explosion_radius_float);
}

void CPropCanister::ExplodeStun(Vector &origin, QAngle &angles, const CTakeDamageInfo &info)
{
	// todo: sound, graphics
	DispatchParticleEffect("biospore_explosion_core", GetAbsOrigin(), GetAbsAngles(), NULL);

	// Find and freeze all NPC's within the radius
	CBaseEntity *pEntity = NULL;
	for (CEntitySphereQuery sphere(origin, m_explosion_radius_float * GetModelScale()); (pEntity = sphere.GetCurrentEntity()) != NULL; sphere.NextEntity())
	{
		if (pEntity && pEntity->IsNPC())
		{
			// Check damage filters so we don't stun friendlies
			if (pEntity->PassesDamageFilter(info))
			{
				CAI_BaseNPC *npc = dynamic_cast<CAI_BaseNPC*>(pEntity);
				if (npc && npc->CapabilitiesGet() & bits_CAP_LIVE_RAGDOLL)
				{
					variant_t input;
					input.SetFloat(m_explosion_damage_float * RandomFloat(0.8,1.2)); // for stun bombs, damage is the duration
					npc->AcceptInput("FallInRagdollForSeconds", info.GetAttacker(), info.GetAttacker(), input, 0);
				}
			}
		}
	}
}

void CPropCanister::ExplodeBlackHole(Vector &origin, QAngle &angles, const CTakeDamageInfo &info)
{
	CEnvGravityVortex *vortex = CEnvGravityVortex::Create(GetAbsOrigin(), 32, m_explosion_radius_float, m_explosion_damage_float, 2.0, this);
	if ( vortex )
	{
		vortex->SetVortexPropKill(false);
		vortex->SetVortexNPCPull(true);
		vortex->SetVortexNPCKill(true);
		vortex->SetVortexPlayerPull(true);
		vortex->SetVortexPlayerKill(true);
		vortex->SetVortexRemoveOnStop(true); // after duration is over, the vortex will self-remove
	}
	else
	{
		Warning("Failed to create the vortex\n");
	}
}

BEGIN_DATADESC(CPropCanister)
	DEFINE_KEYFIELD(m_explosion_damage_float, FIELD_FLOAT, "ExplosiveDamage"),
	DEFINE_KEYFIELD(m_explosion_radius_float, FIELD_FLOAT, "ExplosiveRadius"),
	DEFINE_KEYFIELD(m_canister_type_int, FIELD_INTEGER, "CanisterType"),
END_DATADESC()