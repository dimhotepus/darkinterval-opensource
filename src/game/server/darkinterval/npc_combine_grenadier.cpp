//========= Dark Interval, 2021. ==============================================//
//
// Purpose: The grenadier
//
//=============================================================================//

#include "cbase.h"
#include "ai_hull.h"
#include "ai_memory.h"
#include "ai_motor.h"
#include "ai_squadslot.h"
#include "ammodef.h"
#include "bitstring.h"
#include "engine/ienginesound.h"
#include "env_explosion.h"
#include "game.h"
#include "gameweaponmanager.h"
#include "hl2_gamerules.h"
#include "hl2/hl2_player.h"
#include "ieffects.h"
#include "ndebugoverlay.h"
#include "npcevent.h"
#include "npc_combine_soldier.h"
#include "darkinterval\npc_parse.h"
#include "soundent.h"
#include "sprite.h"
#include "soundenvelope.h"
#include "te_effect_dispatch.h"
#include "vehicle_base.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CNPC_CombineGrenadier : public CNPC_CombineS
{
	DECLARE_CLASS(CNPC_CombineGrenadier, CNPC_CombineS);
	DECLARE_DATADESC();
public:
	CNPC_CombineGrenadier();
	~CNPC_CombineGrenadier();
	virtual void		Spawn(void);
	virtual void		Precache(void);
	void				RunAI(void);
	virtual void		DeathSound(const CTakeDamageInfo &info);
	virtual void		PrescheduleThink(void);
	virtual void		BuildScheduleTestBits(void);
	virtual int			SelectSchedule(void);
	virtual void		StartTask(const Task_t *pTask);
	virtual void		RunTask(const Task_t *pTask);
	virtual float		GetHitgroupDamageMultiplier(int iHitGroup, const CTakeDamageInfo &info);
	virtual void		HandleAnimEvent(animevent_t *pEvent);
	virtual void		OnChangeActivity(Activity eNewActivity);
	virtual void		Event_Killed(const CTakeDamageInfo &info);
	virtual void		OnListened();

	bool				m_fIsBlocking;

	virtual bool		IsLightDamage(const CTakeDamageInfo &info);
	virtual bool		IsHeavyDamage(const CTakeDamageInfo &info);

	virtual	bool		AllowedToIgnite(void) { return false; } // grenadiers are fire-proof

	Activity			NPC_TranslateActivity(Activity eNewActivity);
	int					m_iUseMarch;

private:
//	enum
//	{
//		SCHED_COMBINE_ELITE_ESTABLISH_LINE_OF_FIRE = 122,
//		SCHED_COMBINE_ELITE_SNIPER_FIRE,
//		NEXT_SCHEDULE,
//	};
};

#define AE_SOLDIER_BLOCK_PHYSICS		20 // trying to block an incoming physics object
#define ELITE_SKIN_CAMO_OFF 0
#define ELITE_SKIN_CAMO_ON 1
#define ELITE_SKIN_CAMO_AFTERSHOCK 2

extern Activity ACT_WALK_EASY;
extern Activity ACT_WALK_MARCH;

CNPC_CombineGrenadier::CNPC_CombineGrenadier()
{
	m_hNPCFileInfo = LookupNPCInfoScript("npc_combine_grenadier");
}
CNPC_CombineGrenadier::~CNPC_CombineGrenadier()
{
}

void CNPC_CombineGrenadier::Precache()
{
	const char *pModelName = GetNPCScriptData().NPC_Model_Path_char;

	SetModelName(MAKE_STRING(pModelName));
	PrecacheModel(STRING(GetModelName()));

	BaseClass::Precache();
}

void CNPC_CombineGrenadier::Spawn(void)
{
	Precache();
	SetModel(STRING(GetModelName()));

	BaseClass::Spawn();

	SetHealth(GetNPCScriptData().NPC_Stats_Health_int);
	SetMaxHealth(GetNPCScriptData().NPC_Stats_MaxHealth_int);

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	SetHullType(Hull_t(GetNPCScriptData().NPC_Stats_HullType_int));
	SetHullSizeNormal();
	m_flFieldOfView = GetNPCScriptData().NPC_Stats_FOV_float;

	CapabilitiesClear();

	if ((GetNPCScriptData().NPC_Capable_Jump_boolean) == true) CapabilitiesAdd(bits_CAP_MOVE_JUMP);
	if ((GetNPCScriptData().NPC_Capable_Climb_boolean) == true) CapabilitiesAdd(bits_CAP_MOVE_CLIMB);
	if ((GetNPCScriptData().NPC_Capable_Doors_boolean) == true) CapabilitiesAdd(bits_CAP_DOORS_GROUP);
	if ((GetNPCScriptData().NPC_Capable_MeleeAttack1_boolean) == true) CapabilitiesAdd(bits_CAP_INNATE_MELEE_ATTACK1);
	if ((GetNPCScriptData().NPC_Capable_MeleeAttack2_boolean) == true) CapabilitiesAdd(bits_CAP_INNATE_MELEE_ATTACK2);
	if ((GetNPCScriptData().NPC_Capable_InnateRangeAttack1_boolean) == true) CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK1);
	if ((GetNPCScriptData().NPC_Capable_InnateRangeAttack1_boolean) == true) CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK2);
	if ((GetNPCScriptData().NPC_Capable_WeaponRangeAttack1_boolean) == true) CapabilitiesAdd(bits_CAP_WEAPON_RANGE_ATTACK1);
	if ((GetNPCScriptData().NPC_Capable_WeaponRangeAttack2_boolean) == true) CapabilitiesAdd(bits_CAP_WEAPON_RANGE_ATTACK2);

	CapabilitiesAdd(bits_CAP_MOVE_GROUND);
	CapabilitiesAdd(bits_CAP_MOVE_SHOOT);
	CapabilitiesAdd(bits_CAP_ANIMATEDFACE);

	m_shouldpatrol_boolean = GetNPCScriptData().NPC_Behavior_PatrolAfterSpawn_boolean;

	SetModelScale(RandomFloat(GetNPCScriptData().NPC_Model_ScaleMin_float, GetNPCScriptData().NPC_Model_ScaleMax_float));

	m_nSkin = RandomInt(GetNPCScriptData().NPC_Model_SkinMin_int, GetNPCScriptData().NPC_Model_SkinMax_int);
}

void CNPC_CombineGrenadier::RunAI(void)
{
	BaseClass::RunAI();
}

void CNPC_CombineGrenadier::DeathSound(const CTakeDamageInfo &info)
{
	// NOTE: The response system deals with this at the moment
	if (GetFlags() & FL_DISSOLVING)
		return;

	GetSentences()->Speak("COMBINE_DIE", SENTENCE_PRIORITY_INVALID, SENTENCE_CRITERIA_ALWAYS);
}

void CNPC_CombineGrenadier::PrescheduleThink(void)
{
	BaseClass::PrescheduleThink();
}

void CNPC_CombineGrenadier::BuildScheduleTestBits(void)
{
	//Interrupt any schedule with physics danger (as long as I'm not moving or already trying to block)
	if (m_flGroundSpeed == 0.0 && !IsCurSchedule(SCHED_FLINCH_PHYSICS))
	{
		SetCustomInterruptCondition(COND_HEAR_PHYSICS_DANGER);
	}

	BaseClass::BuildScheduleTestBits();
}

int CNPC_CombineGrenadier::SelectSchedule(void)
{
	return BaseClass::SelectSchedule();
}

void CNPC_CombineGrenadier::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
	{
		m_nShots = GetActiveWeapon()->GetRandomBurst();
		m_flShotDelay = GetActiveWeapon()->GetFireRate();

		m_flNextAttack = CURTIME + m_flShotDelay - 0.1;
		ResetIdealActivity(ACT_RANGE_ATTACK1);
		m_last_attack_time = CURTIME;
	}
	break;

	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

void CNPC_CombineGrenadier::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
	{
		AutoMovement();

		Vector vecEnemyLKP = GetEnemyLKP();
		if (!FInAimCone(vecEnemyLKP))
		{
			GetMotor()->SetIdealYawToTargetAndUpdate(vecEnemyLKP, AI_KEEP_YAW_SPEED);
		}
		else
		{
			GetMotor()->SetIdealYawAndUpdate(GetMotor()->GetIdealYaw(), AI_KEEP_YAW_SPEED);
		}
		if (CURTIME >= m_flNextAttack)
		{
			if (IsActivityFinished())
			{
				if (--m_nShots > 0)
				{
					// DevMsg("ACT_RANGE_ATTACK1\n");
					ResetIdealActivity(ACT_RANGE_ATTACK1);
					m_last_attack_time = CURTIME;
					m_flNextAttack = CURTIME + m_flShotDelay;
				}
				else
				{
					// DevMsg("TASK_RANGE_ATTACK1 complete\n");
					m_flNextAttack = GetShotRegulator()->NextShotTime();
					TaskComplete();
				}
			}
		}
		else
		{
			// DevMsg("Wait\n");
		}
	}
	break;
	default:
	{
		BaseClass::RunTask(pTask);
		break;
	}
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float CNPC_CombineGrenadier::GetHitgroupDamageMultiplier(int iHitGroup, const CTakeDamageInfo &info)
{
	switch (iHitGroup)
	{
	case HITGROUP_HEAD:
	{
		// Soldiers take double headshot damage
		return 1.4f;
	}
	}

	return BaseClass::GetHitgroupDamageMultiplier(iHitGroup, info);
}


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_CombineGrenadier::HandleAnimEvent(animevent_t *pEvent)
{
	switch (pEvent->event)
	{
	case AE_SOLDIER_BLOCK_PHYSICS:
		m_fIsBlocking = true;
		break;

	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

void CNPC_CombineGrenadier::OnChangeActivity(Activity eNewActivity)
{
	// Any new sequence stops us blocking.
	m_fIsBlocking = false;

	BaseClass::OnChangeActivity(eNewActivity);

	// Give each trooper a varied look for his march. Done here because if you do it earlier (eg Spawn, StartTask), the
	// pose param gets overwritten.
	if (m_iUseMarch)
	{
		SetPoseParameter("casual", RandomFloat());
	}
}

void CNPC_CombineGrenadier::OnListened()
{
	BaseClass::OnListened();
}

void CNPC_CombineGrenadier::Event_Killed(const CTakeDamageInfo &info)
{
	BaseClass::Event_Killed(info);
}

bool CNPC_CombineGrenadier::IsLightDamage(const CTakeDamageInfo &info)
{
	return BaseClass::IsLightDamage(info);
}

bool CNPC_CombineGrenadier::IsHeavyDamage(const CTakeDamageInfo &info)
{
	// Combine considers AR2 fire to be heavy damage
	if (info.GetAmmoType() == GetAmmoDef()->Index("AR2"))
		return true;

	// Combine considers OICW fire to be heavy damage
	if (info.GetAmmoType() == GetAmmoDef()->Index("OICW"))
		return true;

	// 357 rounds are heavy damage
	if (info.GetAmmoType() == GetAmmoDef()->Index("357"))
		return true;

	// Rollermine shocks
	if ((info.GetDamageType() & DMG_SHOCK))
	{
		return true;
	}

	return BaseClass::IsHeavyDamage(info);
}

/*
int CNPC_CombineGrenadier::TranslateSchedule(int scheduleType)
{
switch (scheduleType)
{
case SCHED_ESTABLISH_LINE_OF_FIRE:
{
// always assume standing
// Stand();

if (CanAltFireEnemy(true) && OccupyStrategySlot(SQUAD_SLOT_SPECIAL_ATTACK))
{
// If an elite in the squad could fire a combine ball at the player's last known position,
// do so!
return SCHED_COMBINE_ELITE_SNIPER_FIRE;
}

return SCHED_COMBINE_ELITE_ESTABLISH_LINE_OF_FIRE;
}
break;
case SCHED_RANGE_ATTACK1:
{
if (HasCondition(COND_NO_PRIMARY_AMMO) || HasCondition(COND_LOW_PRIMARY_AMMO))
{
// Ditch the strategy slot for attacking (which we just reserved!)
VacateStrategySlot();
return TranslateSchedule(SCHED_HIDE_AND_RELOAD);
}

if (CanAltFireEnemy(true) && OccupyStrategySlot(SQUAD_SLOT_SPECIAL_ATTACK))
{
// Since I'm holding this squadslot, no one else can try right now. If I die before the shot
// goes off, I won't have affected anyone else's ability to use this attack at their nearest
// convenience.
return SCHED_COMBINE_ELITE_SNIPER_FIRE;
}

if (IsCrouching() || (CrouchIsDesired() && !HasCondition(COND_HEAVY_DAMAGE)))
{
// See if we can crouch and shoot
if (GetEnemy() != NULL)
{
float dist = (GetLocalOrigin() - GetEnemy()->GetLocalOrigin()).Length();

// only crouch if they are relatively far away
if (dist > 350)
{
// try crouching
Crouch();

Vector targetPos = GetEnemy()->BodyTarget(GetActiveWeapon()->GetLocalOrigin());

// if we can't see it crouched, stand up
if (!WeaponLOSCondition(GetLocalOrigin(), targetPos, false))
{
Stand();
}
}
}
}
else
{
// always assume standing
Stand();
}

//	return SCHED_COMBINE_RANGE_ATTACK1;
}
}

return BaseClass::TranslateSchedule(scheduleType);
}
*/

Activity CNPC_CombineGrenadier::NPC_TranslateActivity(Activity eNewActivity)
{
	if (m_iUseMarch && eNewActivity == ACT_WALK)
	{
		eNewActivity = ACT_WALK_MARCH;
	}

	return BaseClass::NPC_TranslateActivity(eNewActivity);
}

BEGIN_DATADESC(CNPC_CombineGrenadier)
DEFINE_KEYFIELD(m_iUseMarch, FIELD_INTEGER, "usemarch"),
END_DATADESC()

LINK_ENTITY_TO_CLASS(npc_combine_grenadier, CNPC_CombineGrenadier);
PRECACHE_REGISTER(npc_combine_grenadier);