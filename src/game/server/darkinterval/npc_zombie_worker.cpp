#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_default.h"
#include "ai_schedule.h"
#include "ai_hull.h"
#include "ai_motor.h"
#include "ai_memory.h"
#include "ai_route.h"
#include "ai_squad.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "entitylist.h"
#include "ai_task.h"
#include "activitylist.h"
#include "engine/ienginesound.h"
#include "npc_BaseZombie.h"
#include "movevars_shared.h"
#include "ieffects.h"
#include "props.h"
#include "physics_npc_solver.h"
#include "hl2_player.h"
#include "hl2_gamerules.h"

#include "basecombatweapon.h"
#include "basegrenade_shared.h"
#include "grenade_frag.h"

#include "ai_interactions.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

enum
{
	SQUAD_SLOT_ZOMBINE_SPRINT1 = LAST_SHARED_SQUADSLOT,
	SQUAD_SLOT_ZOMBINE_SPRINT2,
};

#define MIN_SPRINT_TIME 3.5f
#define MAX_SPRINT_TIME 5.5f

#define MIN_SPRINT_DISTANCE 64.0f
#define MAX_SPRINT_DISTANCE 1024.0f

#define SPRINT_CHANCE_VALUE 10
#define SPRINT_CHANCE_VALUE_DARKNESS 50

int ACT_ZOMBIEWORKER_ATTACK_FAST;

Activity ACT_ZOM_RELEASECRAB;

extern bool IsAlyxInDarknessMode();

ConVar	sk_zombie_worker_health("sk_zombie_worker_health", "0");

class CNPC_ZombieWorker : public CAI_BlendingHost<CNPC_BaseZombie>, public CDefaultPlayerPickupVPhysics
{
	DECLARE_DATADESC();
	DECLARE_CLASS(CNPC_ZombieWorker, CAI_BlendingHost<CNPC_BaseZombie>);

public:

	void Spawn(void);
	void Precache(void);
	void SetZombieModel(void);

	virtual void PrescheduleThink(void);
	virtual int SelectSchedule(void);
	virtual void BuildScheduleTestBits(void);

	virtual void HandleAnimEvent(animevent_t *pEvent);
	virtual	bool IsValidEnemy(CBaseEntity *pEnemy);

	virtual const char *GetLegsModel(void);
	virtual const char *GetTorsoModel(void);
	virtual const char *GetHeadcrabClassname(void);
	virtual const char *GetHeadcrabModel(void);

	virtual void PainSound(const CTakeDamageInfo &info);
	virtual void DeathSound(const CTakeDamageInfo &info);
	virtual void AlertSound(void);
	virtual void IdleSound(void);
	virtual void AttackSound(void);
	virtual void AttackHitSound(void);
	virtual void AttackMissSound(void);
	virtual void FootstepSound(bool fRightFoot);
	virtual void FootscuffSound(bool fRightFoot);
	virtual void MoanSound(envelopePoint_t *pEnvelope, int iEnvelopeSize);

	virtual void Event_Killed(const CTakeDamageInfo &info);
	virtual void TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr);
	virtual int OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo);
	virtual void RunTask(const Task_t *pTask);
	virtual int  MeleeAttack1Conditions(float flDot, float flDist);
	void Ignite(float flFlameLifetime, bool bNPCOnly = true, float flSize = 0.0f, bool bCalledByLevelDesigner = false);
	void Extinguish();


	virtual HeadcrabRelease_t ShouldReleaseHeadcrab(const CTakeDamageInfo &info, float flDamageThreshold) { return RELEASE_NO; };
	virtual bool ShouldBecomeTorso(const CTakeDamageInfo &info, float flDamageThreshold) { return false; };

	virtual void OnScheduleChange(void);
	virtual bool CanRunAScriptedNPCInteraction(bool bForced);

	virtual Activity NPC_TranslateActivity(Activity baseAct);

	const char *GetMoanSound(int nSound);

	bool AllowedToSprint(void);
	void Sprint(bool bMadSprint = false);
	void StopSprint(void);

	bool IsSprinting(void) { return m_flSprintTime > CURTIME; }

	void InputStartSprint(inputdata_t &inputdata);

	virtual bool HandleInteraction(int interactionType, void *data, CBaseCombatCharacter *sourceEnt);

	int m_iSkin;
	int TranslateSchedule(int scheduleType);

public:
	DEFINE_CUSTOM_AI;

private:

	float	m_flSprintTime;
	float	m_flSprintRestTime;

	float	m_flSuperFastAttackTime;

protected:
	static const char *pMoanSounds[];

};

LINK_ENTITY_TO_CLASS(npc_zombie_worker, CNPC_ZombieWorker);

BEGIN_DATADESC(CNPC_ZombieWorker)
DEFINE_FIELD(m_flSprintTime, FIELD_TIME),
DEFINE_FIELD(m_flSprintRestTime, FIELD_TIME),
DEFINE_FIELD(m_flSuperFastAttackTime, FIELD_TIME),
DEFINE_INPUTFUNC(FIELD_VOID, "StartSprint", InputStartSprint),
DEFINE_KEYFIELD(m_iSkin, FIELD_INTEGER, "Skin"),
END_DATADESC()

enum
{
	SCHED_ZOMBIE_WORKER_ALERT,
};

//---------------------------------------------------------
//---------------------------------------------------------
const char *CNPC_ZombieWorker::pMoanSounds[] =
{
	//	"Zombie.Pain",
	//	"NPC_ZombieWorker.Moan1",
	"NPC_BaseZombie.Moan1",
	"NPC_BaseZombie.Moan2",
	//	"NPC_ZombieWorker.Moan3",
	"NPC_BaseZombie.Moan3",
	"NPC_BaseZombie.Moan4",
};

envelopePoint_t envZombieWorkerMoanIgnited[] =
{
	{ 1.0f, 1.0f,
	0.5f, 1.0f,
	},
	{ 1.0f, 1.0f,
	30.0f, 30.0f,
	},
	{ 0.0f, 0.0f,
	0.5f, 1.0f,
	},
};

void CNPC_ZombieWorker::Precache(void)
{
	BaseClass::Precache();

	PrecacheModel("models/_Monsters/Zombies/zombie_worker.mdl");

	PrecacheScriptSound("Zombie.FootstepRight");
	PrecacheScriptSound("Zombie.FootstepLeft");
	PrecacheScriptSound("Zombie.ScuffRight");
	PrecacheScriptSound("Zombie.ScuffLeft");
	PrecacheScriptSound("Zombie.Attack");
	PrecacheScriptSound("Zombie.AttackHit");
	PrecacheScriptSound("Zombie.AttackMiss");
	PrecacheScriptSound("Zombie.Pain");
	PrecacheScriptSound("Zombie.Die");
	PrecacheScriptSound("Zombie.Alert");
	PrecacheScriptSound("Zombie.Idle");

	PrecacheScriptSound("NPC_BaseZombie.Moan1");
	PrecacheScriptSound("NPC_BaseZombie.Moan2");
	PrecacheScriptSound("NPC_BaseZombie.Moan3");
	PrecacheScriptSound("NPC_BaseZombie.Moan4");
}

void CNPC_ZombieWorker::Spawn(void)
{
	Precache();

	m_fIsTorso = false;
	m_fIsHeadless = false;

#ifdef HL2_EPISODIC
	SetBloodColor(BLOOD_COLOR_ZOMBIE);
#else
	SetBloodColor(BLOOD_COLOR_GREEN);
#endif // HL2_EPISODIC

	m_iHealth = sk_zombie_worker_health.GetFloat();
	SetMaxHealth(m_iHealth);

	m_flFieldOfView = 0.2;

	CapabilitiesClear();
	CapabilitiesAdd(bits_CAP_WEAPON_MELEE_ATTACK1);

	BaseClass::Spawn();

	m_flSprintTime = 0.0f;
	m_flSprintRestTime = 0.0f;

	m_flNextMoanSound = CURTIME + random->RandomFloat(1.0, 4.0);

	m_nSkin = m_iSkin;
}

void CNPC_ZombieWorker::SetZombieModel(void)
{
	char *szModel = (char *)STRING(GetModelName());
	if (!szModel || !*szModel)
	{
		szModel = "models/_Monsters/Zombies/zombie_worker.mdl";
	}

	SetModel(szModel);
	SetHullType(HULL_HUMAN);

	//	SetBodygroup(ZOMBIE_BODYGROUP_HEADCRAB, !m_fIsHeadless); // always has headcrab

	SetHullSizeNormal(true);
	SetDefaultEyeOffset();
	SetActivity(ACT_IDLE);
}

void CNPC_ZombieWorker::PrescheduleThink(void)
{
	if (CURTIME > m_flNextMoanSound)
	{
		if (CanPlayMoanSound())
		{
			// Classic guy idles instead of moans.
			IdleSound();

			m_flNextMoanSound = CURTIME + random->RandomFloat(10.0, 15.0);
		}
		else
		{
			IdleSound();
			m_flNextMoanSound = CURTIME + random->RandomFloat(2.5, 5.0);
		}
	}

	BaseClass::PrescheduleThink();
}

void CNPC_ZombieWorker::OnScheduleChange(void)
{
	if (HasCondition(COND_CAN_MELEE_ATTACK1) && IsSprinting() == true)
	{
		m_flSuperFastAttackTime = CURTIME + 1.0f;
	}

	BaseClass::OnScheduleChange();
}
bool CNPC_ZombieWorker::CanRunAScriptedNPCInteraction(bool bForced)
{
	return BaseClass::CanRunAScriptedNPCInteraction(bForced);
}

int CNPC_ZombieWorker::SelectSchedule(void)
{
	if (GetHealth() <= 0)
		return BaseClass::SelectSchedule();

	return BaseClass::SelectSchedule();
}

void CNPC_ZombieWorker::BuildScheduleTestBits(void)
{
	BaseClass::BuildScheduleTestBits();
}

Activity CNPC_ZombieWorker::NPC_TranslateActivity(Activity baseAct)
{
	if (baseAct == ACT_MELEE_ATTACK1)
	{
		if (m_flSuperFastAttackTime > CURTIME)
		{
			return (Activity)ACT_ZOMBIEWORKER_ATTACK_FAST;
		}
	}

	if (baseAct == ACT_STAND)
	{
		if (entindex() % 2)
		{
			return (Activity)ACT_ZOMBIE_SLUMP_A_RISE;
		}
		else
		{
			return (Activity)ACT_ZOMBIE_SLUMP_B_RISE;
		}
	}

	return BaseClass::NPC_TranslateActivity(baseAct);
}

int CNPC_ZombieWorker::MeleeAttack1Conditions(float flDot, float flDist)
{
	int iBase = BaseClass::MeleeAttack1Conditions(flDot, flDist);


	return iBase;
}

void CNPC_ZombieWorker::Event_Killed(const CTakeDamageInfo &info)
{
	BaseClass::Event_Killed(info);
}

//-----------------------------------------------------------------------------
// Purpose:  This is a generic function (to be implemented by sub-classes) to
//			 handle specific interactions between different types of characters
//			 (For example the barnacle grabbing an NPC)
// Input  :  Constant for the type of interaction
// Output :	 true  - if sub-class has a response for the interaction
//			 false - if sub-class has no response
//-----------------------------------------------------------------------------
bool CNPC_ZombieWorker::HandleInteraction(int interactionType, void *data, CBaseCombatCharacter *sourceEnt)
{
	return BaseClass::HandleInteraction(interactionType, data, sourceEnt);
}

void CNPC_ZombieWorker::TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr)
{
	BaseClass::TraceAttack(info, vecDir, ptr);

	//Only knock grenades off their hands if it's a player doing the damage.
	if (info.GetAttacker() && info.GetAttacker()->IsNPC())
		return;
}

int CNPC_ZombieWorker::OnTakeDamage_Alive(const CTakeDamageInfo &inputInfo)
{
	CTakeDamageInfo newInfo = inputInfo;
	if (newInfo.GetDamageType() & DMG_BURN)
	{
		newInfo.ScaleDamage(0.5f);
	}

	return BaseClass::OnTakeDamage_Alive(newInfo);
}

void CNPC_ZombieWorker::Ignite(float flFlameLifetime, bool bNPCOnly, float flSize, bool bCalledByLevelDesigner)
{
	if (!IsOnFire() && IsAlive())
	{
		BaseClass::Ignite(flFlameLifetime, bNPCOnly, flSize, bCalledByLevelDesigner);

		RemoveSpawnFlags(SF_NPC_GAG);

		MoanSound(envZombieWorkerMoanIgnited, ARRAYSIZE(envZombieWorkerMoanIgnited));

		if (m_pMoanSound)
		{
			ENVELOPE_CONTROLLER.SoundChangePitch(m_pMoanSound, 100, 1.0);
			ENVELOPE_CONTROLLER.SoundChangeVolume(m_pMoanSound, 1, 1.0);
		}

	}
}

void CNPC_ZombieWorker::Extinguish()
{
	if (m_pMoanSound)
	{
		ENVELOPE_CONTROLLER.SoundChangeVolume(m_pMoanSound, 0, 2.0);
		ENVELOPE_CONTROLLER.SoundChangePitch(m_pMoanSound, 100, 2.0);
		m_flNextMoanSound = CURTIME + random->RandomFloat(2.0, 4.0);
	}

	BaseClass::Extinguish();
}

void CNPC_ZombieWorker::HandleAnimEvent(animevent_t *pEvent)
{
	BaseClass::HandleAnimEvent(pEvent);
}

// This is a copy of baseNPC IsValidEnemy determination.
// Because the base version returns false when IsOnFire(), and 
// that makes zombie workers too non-threatening in the beginning of DI,
// this override exists.
bool CNPC_ZombieWorker::IsValidEnemy(CBaseEntity *pEnemy)
{
	CAI_BaseNPC *pEnemyNPC = pEnemy->MyNPCPointer();
	if (pEnemyNPC && pEnemyNPC->CanBeAnEnemyOf(this) == false)
		return false;
	// fails to compile, it doesn't matter right now
//	if (m_hEnemyFilter.Get() != NULL && m_hEnemyFilter->PassesFilter(this, pEnemy) == false)
//		return false;

	return true;
}

bool CNPC_ZombieWorker::AllowedToSprint(void)
{
	if (IsOnFire())
		return true;

	//If you're sprinting then there's no reason to sprint again.
	if (IsSprinting())
		return false;

	int iChance = SPRINT_CHANCE_VALUE;

	CHL2_Player *pPlayer = dynamic_cast <CHL2_Player*> (AI_GetSinglePlayer());

	if (pPlayer)
	{
		if (HL2GameRules()->IsAlyxInDarknessMode() && pPlayer->FlashlightIsOn() == false)
		{
			iChance = SPRINT_CHANCE_VALUE_DARKNESS;
		}

		//Bigger chance of this happening if the player is not looking at the zombie
		if (pPlayer->FInViewCone(this) == false)
		{
			iChance *= 2;
		}
	}

	//Below 25% health they'll always sprint
	if ((GetHealth() > GetMaxHealth() * 0.5f))
	{
		if (IsStrategySlotRangeOccupied(SQUAD_SLOT_ZOMBINE_SPRINT1, SQUAD_SLOT_ZOMBINE_SPRINT2) == true)
			return false;

		if (random->RandomInt(0, 100) > iChance)
			return false;

		if (m_flSprintRestTime > CURTIME)
			return false;
	}

	float flLength = (GetEnemy()->WorldSpaceCenter() - WorldSpaceCenter()).Length();

	if (flLength > MAX_SPRINT_DISTANCE)
		return false;

	return true;
}

void CNPC_ZombieWorker::StopSprint(void)
{
	GetNavigator()->SetMovementActivity(ACT_WALK);

	m_flSprintTime = CURTIME;
	m_flSprintRestTime = m_flSprintTime + random->RandomFloat(2.5f, 5.0f);
}

void CNPC_ZombieWorker::Sprint(bool bMadSprint)
{
	if (IsSprinting())
		return;

	OccupyStrategySlotRange(SQUAD_SLOT_ZOMBINE_SPRINT1, SQUAD_SLOT_ZOMBINE_SPRINT2);
	GetNavigator()->SetMovementActivity(ACT_RUN);

	float flSprintTime = random->RandomFloat(MIN_SPRINT_TIME, MAX_SPRINT_TIME);

	m_flSprintTime = CURTIME + flSprintTime;

	//Don't sprint for this long after I'm done with this sprint run.
	m_flSprintRestTime = m_flSprintTime + random->RandomFloat(2.5f, 5.0f);

	EmitSound("Zombie.Alert");
}

void CNPC_ZombieWorker::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_WAIT_FOR_MOVEMENT_STEP:
	case TASK_WAIT_FOR_MOVEMENT:
	{
		BaseClass::RunTask(pTask);

		if (IsOnFire() && IsSprinting())
		{
			StopSprint();
		}

		//Only do this if I have an enemy
		if (GetEnemy())
		{
			if (AllowedToSprint() == true)
			{
				Sprint((GetHealth() <= GetMaxHealth() * 0.5f));
				return;
			}

			if (GetNavigator()->GetMovementActivity() != ACT_WALK)
			{
				if (IsSprinting() == false)
				{
					GetNavigator()->SetMovementActivity(ACT_WALK);
				}
			}
		}
		else
		{
			GetNavigator()->SetMovementActivity(ACT_WALK);
		}

		break;
	}
	default:
	{
		BaseClass::RunTask(pTask);
		break;
	}
	}
}

void CNPC_ZombieWorker::InputStartSprint(inputdata_t &inputdata)
{
	Sprint();
}

//-----------------------------------------------------------------------------
// Purpose: Returns a moan sound for this class of zombie.
//-----------------------------------------------------------------------------
const char *CNPC_ZombieWorker::GetMoanSound(int nSound)
{
	return pMoanSounds[nSound % ARRAYSIZE(pMoanSounds)];
}

//-----------------------------------------------------------------------------
// Purpose: Sound of a footstep
//-----------------------------------------------------------------------------
void CNPC_ZombieWorker::FootstepSound(bool fRightFoot)
{
	if (fRightFoot)
	{
		EmitSound("Zombie.FootstepRight");
	}
	else
	{
		EmitSound("Zombie.FootstepLeft");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sound of a foot sliding/scraping
//-----------------------------------------------------------------------------
void CNPC_ZombieWorker::FootscuffSound(bool fRightFoot)
{
	if (fRightFoot)
	{
		EmitSound("Zombie.ScuffRight");
	}
	else
	{
		EmitSound("Zombie.ScuffLeft");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack hit sound
//-----------------------------------------------------------------------------
void CNPC_ZombieWorker::AttackHitSound(void)
{
	EmitSound("Zombie.AttackHit");
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack miss sound
//-----------------------------------------------------------------------------
void CNPC_ZombieWorker::AttackMissSound(void)
{
	// Play a random attack miss sound
	EmitSound("Zombie.AttackMiss");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_ZombieWorker::PainSound(const CTakeDamageInfo &info)
{
	// We're constantly taking damage when we are on fire. Don't make all those noises!
	if (IsOnFire())
	{
			return;
	}

	EmitSound("Zombie.Pain");
	m_flNextMoanSound = CURTIME + RandomFloat(3, 5);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_ZombieWorker::DeathSound(const CTakeDamageInfo &info)
{
	EmitSound("Zombie.Die");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_ZombieWorker::AlertSound(void)
{
	EmitSound("Zombie.Alert");

	// Don't let a moan sound cut off the alert sound.
	m_flNextMoanSound += random->RandomFloat(2.0, 4.0);
}

//-----------------------------------------------------------------------------
// Purpose: Play a random idle sound.
//-----------------------------------------------------------------------------
void CNPC_ZombieWorker::IdleSound(void)
{
	if (GetState() == NPC_STATE_IDLE && random->RandomFloat(0, 1) == 0)
	{
		// Moan infrequently in IDLE state.
		return;
	}

	EmitSound("Zombie.Pain");
	MakeAISpookySound(360.0f);
}

//-----------------------------------------------------------------------------
// Purpose: Play a random attack sound.
//-----------------------------------------------------------------------------
void CNPC_ZombieWorker::AttackSound(void)
{
	EmitSound("Zombie.Pain");
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const char *CNPC_ZombieWorker::GetHeadcrabModel(void)
{
	return "models/_Monsters/Xen/headcrab.mdl";
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
const char *CNPC_ZombieWorker::GetLegsModel(void)
{
	return "models/zombie/zombie_soldier_legs.mdl";
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
const char *CNPC_ZombieWorker::GetTorsoModel(void)
{
	return "models/zombie/zombie_soldier_torso.mdl";
}

//---------------------------------------------------------
// Classic zombie only uses moan sound if on fire.
//---------------------------------------------------------
void CNPC_ZombieWorker::MoanSound(envelopePoint_t *pEnvelope, int iEnvelopeSize)
{
	if (IsOnFire())
	{
		BaseClass::MoanSound(pEnvelope, iEnvelopeSize);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns the classname (ie "npc_headcrab") to spawn when our headcrab bails.
//-----------------------------------------------------------------------------
const char *CNPC_ZombieWorker::GetHeadcrabClassname(void)
{
	return "npc_headcrab";
}

int CNPC_ZombieWorker::TranslateSchedule(int scheduleType)
{
	switch (scheduleType)
	{
	case SCHED_ALERT_FACE:
		return SCHED_ZOMBIE_WORKER_ALERT;
		break;
	case SCHED_MELEE_ATTACK1:
		return SCHED_ZOMBIE_MELEE_ATTACK1;
		break;
	case SCHED_FLINCH_PHYSICS:
	{
		m_flExtraRagdollTime = RandomFloat(1.0, 2.5);
		return SCHED_DI_FALL_IN_RAGDOLL_PUSH;
	}
	break;
	}

	return BaseClass::TranslateSchedule(scheduleType);
}

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC(npc_zombine, CNPC_ZombieWorker)

//Squad slots
DECLARE_SQUADSLOT(SQUAD_SLOT_ZOMBINE_SPRINT1)
DECLARE_SQUADSLOT(SQUAD_SLOT_ZOMBINE_SPRINT2)
DECLARE_ACTIVITY(ACT_ZOMBIEWORKER_ATTACK_FAST)
DECLARE_ACTIVITY(ACT_ZOM_RELEASECRAB)

//=========================================================
//=========================================================
DEFINE_SCHEDULE
(
	SCHED_ZOMBIE_WORKER_ALERT,

	"	Tasks"
	"		TASK_STOP_MOVING						0"
	"		TASK_FACE_IDEAL							0"
	"		TASK_PLAY_SEQUENCE_FACE_ENEMY		ACTIVITY:ACT_ZOM_RELEASECRAB"
	"		TASK_FACE_IDEAL							0"
	//	"		TASK_ZOMBIE_RELEASE_HEADCRAB				0"
	//	"		TASK_ZOMBIE_DIE								0"
	"	"
	"	Interrupts"
	"		COND_TASK_FAILED"
)

AI_END_CUSTOM_NPC()



