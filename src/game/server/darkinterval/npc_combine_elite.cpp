//========= Dark Interval, 2018. ==============================================//
//
// Purpose: The elite Combine trooper. Capable of tactical camouflage.
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

class CFleshEffectTarget : public CPointEntity
{
	DECLARE_CLASS(CFleshEffectTarget, CPointEntity);

public:
	void InputSetRadius(inputdata_t &inputData);

	virtual void Spawn(void)
	{
		BaseClass::Spawn();

		AddEFlags(EFL_FORCE_CHECK_TRANSMIT);
	}

	float GetRadius(void) { return m_flRadius; }

	//private:

	CNetworkVar(float, m_flRadius);
	CNetworkVar(float, m_flScaleTime);

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(point_flesh_effect_target, CFleshEffectTarget);

BEGIN_DATADESC(CFleshEffectTarget)

DEFINE_FIELD(m_flScaleTime, FIELD_TIME),
DEFINE_KEYFIELD(m_flRadius, FIELD_FLOAT, "radius"),
DEFINE_INPUTFUNC(FIELD_VECTOR, "SetRadius", InputSetRadius),

END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CFleshEffectTarget, DT_FleshEffectTarget)
SendPropFloat(SENDINFO(m_flRadius), 0, SPROP_NOSCALE),
SendPropFloat(SENDINFO(m_flScaleTime), 0, SPROP_NOSCALE),
END_SEND_TABLE()

void CFleshEffectTarget::InputSetRadius(inputdata_t &inputData)
{
	Vector vecRadius;
	inputData.value.Vector3D(vecRadius);

	m_flRadius = vecRadius.x;
	m_flScaleTime = vecRadius.y;
}

class CNPC_CombineElite : public CNPC_CombineS
{
	DECLARE_CLASS(CNPC_CombineElite, CNPC_CombineS);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
public:
	CNPC_CombineElite();
	~CNPC_CombineElite();
	virtual void		Spawn(void);
	virtual void		Precache(void);
	void				RunAI(void);

	// Sounds, including overrides of Combine sentences
	void DeathSound(const CTakeDamageInfo &info);
	void PainSound(const CTakeDamageInfo &info);
//	void IdleSound(void);
//	void AlertSound(void);
//	void LostEnemySound(void);
//	void FoundEnemySound(void);
	void AnnounceAssault(void);
	void AnnounceEnemyType(CBaseEntity *pEnemy);
//	void AnnounceEnemyKill(CBaseEntity *pEnemy);

	virtual void		PrescheduleThink(void);
	virtual void		BuildScheduleTestBits(void);
	virtual int			SelectSchedule(void);
	virtual void		StartTask(const Task_t *pTask);
	virtual void		RunTask(const Task_t *pTask);
	virtual void		HandleAnimEvent(animevent_t *pEvent);
	virtual void		OnChangeActivity(Activity eNewActivity);
	virtual void		OnStateChange(NPC_STATE OldState, NPC_STATE NewState);
	virtual void		Event_Killed(const CTakeDamageInfo &info);
	virtual void		OnListened();

	virtual void		ClearAttackConditions(void);

	bool		m_fIsBlocking;

	virtual bool		IsLightDamage(const CTakeDamageInfo &info);
	virtual bool		IsHeavyDamage(const CTakeDamageInfo &info);

	virtual	bool		AllowedToIgnite(void) { return true; }

	Activity	NPC_TranslateActivity(Activity eNewActivity);
	int			m_iUseMarch;

	void InputEnableCamo(inputdata_t &data);
	void InputDisableCamo(inputdata_t &data);

	COutputEvent		m_OnCamoOn;
	COutputEvent		m_OnCamoOff;

private:
	void				ThinkEngageCamo(void); // switch skin to camo, use think to push shader proxy values to make camo go from opaque-ish to nearly transparent
	void				ThinkTimeoutCamo(void); // called when going from NPC_STATE_COMBAT to _ALERT; wait N seconds, if no new enemy, launch ThinkDisengageCamo
	void				ThinkDisengageCamo(void); // push shader proxy values to client to make camo go from transparent to opaque again, after that switch to non-camo skin
	void				ThinkWeaponFrameUpdate(void); // recalculating our shooting direction more often for the sniper beam
	float				m_camostart_time;	// begin camo transition effect, either turning it on or off.
	float				m_camoend_time;	// begin camo transition effect, either turning it on or off.
	float				m_camo_damage_float;	// when hit, fog it up a little (PrescheduleThink()).
	bool m_camo_override_bool; // use this if called from an input. Don't deactive camo on exiting combat if this bool is true.
	CNetworkVar(float, m_camo_refract_value);
	CNetworkVar(float, m_camo_fogginess_value);

	CHandle <CFleshEffectTarget>  m_camo_fleshtarget_handle;

	enum
	{
		SCHED_COMBINE_ELITE_ESTABLISH_LINE_OF_FIRE = 122,
		SCHED_COMBINE_ELITE_SNIPER_FIRE,
		NEXT_SCHEDULE,
	};
};

#define AE_SOLDIER_BLOCK_PHYSICS		20 // trying to block an incoming physics object
#define ELITE_SKIN_CAMO_OFF 0
#define ELITE_SKIN_CAMO_ON 1
#define ELITE_SKIN_CAMO_AFTERSHOCK 2

extern Activity ACT_WALK_EASY;
extern Activity ACT_WALK_MARCH;

ConVar npc_combineelite_min_refract("npc_combineelite_min_refract", "0.05");
ConVar npc_combineelite_camo_engage_rate("npc_combineelite_camo_engage_rate", "0.02");
ConVar npc_combineelite_camo_disengage_rate("npc_combineelite_camo_disengage_rate", "0.01");

CNPC_CombineElite::CNPC_CombineElite()
{
	m_hNPCFileInfo = LookupNPCInfoScript("npc_combine_elite");
	m_camo_fleshtarget_handle = NULL;
}
CNPC_CombineElite::~CNPC_CombineElite()
{
	if (m_camo_fleshtarget_handle != NULL) m_camo_fleshtarget_handle = NULL;
}

void CNPC_CombineElite::Precache()
{
	const char *pModelName = GetNPCScriptData().NPC_Model_Path_char;

	SetModelName(MAKE_STRING(pModelName));
	PrecacheModel(STRING(GetModelName()));
	PrecacheScriptSound("NPC_CombineElite.CamoEngage");
	PrecacheScriptSound("NPC_CombineElite.CamoDisengage");
	BaseClass::Precache();
}

void CNPC_CombineElite::Spawn(void)
{
	Precache();
	SetModel(STRING(GetModelName()));

	BaseClass::Spawn();

	m_fIsElite = true;
	AddSpawnFlags(SF_COMBINE_NO_AR2DROP);

	SetHealth( GetNPCScriptData().NPC_Stats_Health_int );
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

	SetBodygroup(1, 1); // hide the ragdoll gun mesh for the live soldiers

	RegisterThinkContext("EngageCamoContext");
	RegisterThinkContext("TimeoutCamoContext");
	RegisterThinkContext("DisengageCamoContext");
	RegisterThinkContext("WeaponFrameUpdateContext");

	m_camostart_time = m_camoend_time = CURTIME;
	m_camo_refract_value = 0.15f;
	m_camo_fogginess_value = 1.30f;
	m_camo_damage_float = 0.0f;

	m_camo_fleshtarget_handle = (CFleshEffectTarget*)/*CBaseEntity::*/CreateNoSpawn("point_flesh_effect_target", WorldSpaceCenter(), GetAbsAngles(), this);
	m_camo_fleshtarget_handle->SetParent(this);
	DispatchSpawn(m_camo_fleshtarget_handle);
	m_camo_fleshtarget_handle->m_flRadius = 40.0f;
}

void CNPC_CombineElite::RunAI(void)
{
	BaseClass::RunAI();

	if (/*HasCondition(COND_DI_IS_IN_LIVE_RAGDOLL) ||*/ m_lifeState == LIFE_DYING || m_lifeState == LIFE_DEAD)
	{
		SetBodygroup(1, 0); // unhide the ragdoll gun mesh
		// note: don't do it when life ragdolling - we don't hide the weapon itself in live ragdolls, 
		// assuming the soldier is capable of grasping the weapon in that state.
		// if we do end up hiding the weapons then we can reenable the condition above.
	}

	if (m_NPCState == NPC_STATE_COMBAT && !IsMoving()) SetNextThink(CURTIME + 0.03f);
	else  SetNextThink(CURTIME + 0.1f);
	
}
void CNPC_CombineElite::ThinkEngageCamo(void)
{
	if (CURTIME > m_camoend_time)
	{
	//	g_pEffects->Sparks(GetAbsOrigin() + Vector(0, 0, 32));
		SetContextThink(NULL, 0, "EngageCamoContext");
	}
	
	if (CURTIME < m_camostart_time + 1.0f)
	{
		m_camo_fleshtarget_handle->m_flRadius -= 2.0f;
		if (m_camo_fleshtarget_handle->GetRadius() < 1.0f)
		{
			m_nSkin = ELITE_SKIN_CAMO_AFTERSHOCK;
			m_camo_fleshtarget_handle->m_flRadius = 1.0f;
		}
	}

	if (CURTIME > m_camostart_time + 0.5f)
	{
		m_camo_refract_value -= npc_combineelite_camo_engage_rate.GetFloat();
		m_camo_fogginess_value += npc_combineelite_camo_engage_rate.GetFloat();
	}

	if (m_camo_refract_value < npc_combineelite_min_refract.GetFloat()) m_camo_refract_value = npc_combineelite_min_refract.GetFloat();
	if (m_camo_fogginess_value > 1.30f) m_camo_fogginess_value = 1.30f;
	
	SetNextThink(CURTIME + 0.01f, "EngageCamoContext");
}

void CNPC_CombineElite::ThinkTimeoutCamo(void)
{
	if (CURTIME > GetLastEnemyTime() + 2.0f || m_camo_override_bool )
	{
	//	g_pEffects->Sparks(GetAbsOrigin() + Vector(0, 0, 32));	
		EmitSound("NPC_CombineElite.CamoDisengage");
		m_camostart_time = CURTIME;
		m_camoend_time = CURTIME + 1.0f;
		m_nSkin = ELITE_SKIN_CAMO_AFTERSHOCK; // should already be this, but just in case
		SetContextThink(&CNPC_CombineElite::ThinkDisengageCamo, CURTIME + 0.1f, "DisengageCamoContext");
		SetContextThink(NULL, 0, "TimeoutCamoContext");
	}

	SetNextThink(CURTIME + 0.05f, "TimeoutCamoContext");
}

void CNPC_CombineElite::ThinkDisengageCamo(void)
{
	m_camo_refract_value += npc_combineelite_camo_disengage_rate.GetFloat();
	m_camo_fogginess_value -= npc_combineelite_camo_disengage_rate.GetFloat();

	if (CURTIME > m_camoend_time - 0.55f)
	{
		m_nSkin = ELITE_SKIN_CAMO_ON;
		m_camo_fleshtarget_handle->m_flRadius += 1.0f; 
		if (m_camo_fleshtarget_handle->GetRadius() > 40.0f)	m_camo_fleshtarget_handle->m_flRadius = 40.0f;
	}

	if (m_camo_refract_value > 4.0f) m_camo_refract_value = 4.0f;
	if (m_camo_fogginess_value < -0.5f) m_camo_fogginess_value = 0.5f;

	if (CURTIME > m_camoend_time)
	{
		m_camo_fleshtarget_handle->m_flRadius = 40.0f;
		m_nSkin = ELITE_SKIN_CAMO_OFF;
	//	g_pEffects->Sparks(GetAbsOrigin() + Vector(0, 0, 32));
		SetContextThink(NULL, 0, "DisengageCamoContext");
		m_OnCamoOff.FireOutput(this, this);
	}
	
	SetNextThink(CURTIME + 0.01f, "DisengageCamoContext");
}

void CNPC_CombineElite::ThinkWeaponFrameUpdate(void)
{
	Weapon_FrameUpdate();
	float next = 0.1f;
	if (m_NPCState == NPC_STATE_COMBAT) next = 0.03f;
	SetNextThink(CURTIME + next, "WeaponFrameUpdateContext");
}

void CNPC_CombineElite::OnStateChange(NPC_STATE OldState, NPC_STATE NewState)
{
	switch (NewState)
	{
	case NPC_STATE_COMBAT:
	{
		SetIdealActivity(ACT_SIGNAL_ADVANCE);
		if (!m_camo_override_bool)
		{
			EmitSound("NPC_CombineElite.CamoEngage");
			m_nSkin = ELITE_SKIN_CAMO_ON;
			m_camostart_time = CURTIME;
			m_camoend_time = CURTIME + 3.0f;
			m_camo_refract_value = 1.0f;
			m_camo_fogginess_value = 0.0f;
			m_camo_fleshtarget_handle->m_flRadius = 40.0f;
			m_camo_damage_float = 0.0f;
			RemoveAllDecals();
			SetContextThink(NULL, 0, "DisengageCamoContext");
			SetContextThink(NULL, 0, "TimeoutCamoContext");
			SetContextThink(&CNPC_CombineElite::ThinkEngageCamo, CURTIME + 0.02f, "EngageCamoContext");
			SetContextThink(&CNPC_CombineElite::ThinkWeaponFrameUpdate, CURTIME + 0.03f, "WeaponFrameUpdateContext");
			m_OnCamoOn.FireOutput(this, this);
		}
	}
	break;
	default:
		if (OldState == NPC_STATE_COMBAT )
		{
		//	g_pEffects->Sparks(GetAbsOrigin() + Vector(0, 0, 32));
			m_flLastEnemyTime = CURTIME;
			if (!m_camo_override_bool)
			{
				SetContextThink(&CNPC_CombineElite::ThinkTimeoutCamo, CURTIME + 0.1f, "TimeoutCamoContext");
				SetContextThink(&CNPC_CombineElite::ThinkWeaponFrameUpdate, CURTIME + 0.1f, "WeaponFrameUpdateContext");
			}
		}
		break;
	}
}

void CNPC_CombineElite::DeathSound(const CTakeDamageInfo &info)
{
	// NOTE: The response system deals with this at the moment
	if (GetFlags() & FL_DISSOLVING)
		return;

	DispatchResponse("TLK_DEATH");

//	GetSentences()->Speak("COMBINE_ELITE_DIE", SENTENCE_PRIORITY_INVALID, SENTENCE_CRITERIA_ALWAYS);
}

void CNPC_CombineElite::PainSound(const CTakeDamageInfo &info)
{
	// NOTE: The response system deals with this at the moment
	if (GetFlags() & FL_DISSOLVING)
		return;

	if (CURTIME > m_flNextPainSoundTime)
	{
		DispatchResponse("TLK_WOUND");
		m_flNextPainSoundTime = CURTIME + 2;
	}
}

void CNPC_CombineElite::AnnounceAssault(void)
{
	GetSentences()->Speak("COMBINE_ELITE_REFIND_ENEMY", SENTENCE_PRIORITY_HIGH);
}

void CNPC_CombineElite::AnnounceEnemyType(CBaseEntity *pEnemy)
{
	DispatchResponse("TLK_STARTCOMBAT");
}
//-----------------------------------------------------------------------------
// Purpose: Soldiers use CAN_RANGE_ATTACK2 to indicate whether they can throw
//			a grenade. Because they check only every half-second or so, this
//			condition must persist until it is updated again by the code
//			that determines whether a grenade can be thrown, so prevent the 
//			base class from clearing it out. (sjb)
//-----------------------------------------------------------------------------
void CNPC_CombineElite::ClearAttackConditions()
{
	bool fCanRangeAttack2 = HasCondition(COND_CAN_RANGE_ATTACK2);

	// Call the base class.
	BaseClass::ClearAttackConditions();

	if (fCanRangeAttack2)
	{
		// We don't allow the base class to clear this condition because we
		// don't sense for it every frame.
		SetCondition(COND_CAN_RANGE_ATTACK2);
	}
}

void CNPC_CombineElite::PrescheduleThink(void)
{
	BaseClass::PrescheduleThink();

	// when in camo and being hit, fog it up a little for a moment

	if (m_nSkin == ELITE_SKIN_CAMO_AFTERSHOCK && CURTIME > m_camoend_time)
	{
		if (CURTIME < m_flLastDamageTime + 0.15f)
		{
			m_camo_damage_float = 0.7f;
		}
		else
		{
			m_camo_damage_float -= 0.15f;
		}

		if (m_camo_damage_float < 0.0f) m_camo_damage_float = 0.0f;

		m_camo_fogginess_value = 1.30f;
		m_camo_fogginess_value -= m_camo_damage_float;

		if (CURTIME < m_last_attack_time + 0.5f)
		{
			m_camo_fogginess_value = 5.0f;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Allows for modification of the interrupt mask for the current schedule.
//			In the most cases the base implementation should be called first.
//-----------------------------------------------------------------------------
void CNPC_CombineElite::BuildScheduleTestBits(void)
{
	//Interrupt any schedule with physics danger (as long as I'm not moving or already trying to block)
	if (m_flGroundSpeed == 0.0 && !IsCurSchedule(SCHED_FLINCH_PHYSICS))
	{
		SetCustomInterruptCondition(COND_HEAR_PHYSICS_DANGER);
	}

	if (IsCurSchedule(SCHED_RANGE_ATTACK1) && GetActiveWeapon() && GetActiveWeapon()->ClassMatches("weapon_sniper"))
	{
		ClearCustomInterruptCondition(COND_LIGHT_DAMAGE);
		ClearCustomInterruptCondition(COND_REPEATED_DAMAGE);
	}

	BaseClass::BuildScheduleTestBits();
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CNPC_CombineElite::SelectSchedule(void)
{
	return BaseClass::SelectSchedule();
}

void CNPC_CombineElite::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_RANGE_ATTACK1:
	{
		m_nShots = GetActiveWeapon()->GetRandomBurst();
		m_flShotDelay = GetActiveWeapon()->GetFireRate();

		m_flNextAttack = CURTIME + m_flShotDelay -0.1;
		ResetIdealActivity( ACT_RANGE_ATTACK1 );
		m_last_attack_time = CURTIME;
	}
	break;
	
	default:
		BaseClass::StartTask(pTask);
		break;
	}
}

void CNPC_CombineElite::RunTask(const Task_t *pTask)
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
		if ( CURTIME >= m_flNextAttack )
		{
		if ( IsActivityFinished() )
		{
		if (--m_nShots > 0)
		{
		// DevMsg("ACT_RANGE_ATTACK1\n");
		ResetIdealActivity( ACT_RANGE_ATTACK1 );
		m_last_attack_time = CURTIME;
		//NDebugOverlay::Cross3D(EyePosition(), 8, 255, 125, 175, true, 0.1f);
		m_flNextAttack = CURTIME + m_flShotDelay;
		}
		else
		{
		// DevMsg("TASK_RANGE_ATTACK1 complete\n");
		//NDebugOverlay::Cross3D(EyePosition(), 8, 125, 175, 255, true, 0.1f);
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
void CNPC_CombineElite::HandleAnimEvent(animevent_t *pEvent)
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

void CNPC_CombineElite::OnChangeActivity(Activity eNewActivity)
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

void CNPC_CombineElite::OnListened()
{
	BaseClass::OnListened();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
void CNPC_CombineElite::Event_Killed(const CTakeDamageInfo &info)
{
	SetBodygroup(1, 0); // unhide the ragdoll gun mesh
	SetContextThink(NULL, 0, "EngageCamo");
	SetContextThink(NULL, 0, "TimeoutCamo");
	SetContextThink(NULL, 0, "DisengageCamo");
	EmitSound("NPC_CombineElite.CamoDisengage");
	m_nSkin = ELITE_SKIN_CAMO_OFF;
	BaseClass::Event_Killed(info);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_CombineElite::IsLightDamage(const CTakeDamageInfo &info)
{
	return BaseClass::IsLightDamage(info);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &info - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CNPC_CombineElite::IsHeavyDamage(const CTakeDamageInfo &info)
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

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
/*
int CNPC_CombineElite::TranslateSchedule(int scheduleType)
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

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
Activity CNPC_CombineElite::NPC_TranslateActivity(Activity eNewActivity)
{
	// If the special ep2_outland_05 "use march" flag is set, use the more casual marching anim.
	if (m_iUseMarch && eNewActivity == ACT_WALK)
	{
		eNewActivity = ACT_WALK_MARCH;
	}

	return BaseClass::NPC_TranslateActivity(eNewActivity);
}

void CNPC_CombineElite::InputEnableCamo(inputdata_t &data)
{
	m_camo_override_bool = true;
	EmitSound("NPC_CombineElite.CamoEngage");
	m_nSkin = ELITE_SKIN_CAMO_ON;
	m_camostart_time = CURTIME;
	m_camoend_time = FLT_MAX;
	m_camo_refract_value = 1.0f;
	m_camo_fogginess_value = 0.0f;
	m_camo_fleshtarget_handle->m_flRadius = 40.0f;
	m_camo_damage_float = 0.0f;
	RemoveAllDecals();
	SetContextThink(NULL, 0, "DisengageCamoContext");
	SetContextThink(NULL, 0, "TimeoutCamoContext");
	SetContextThink(&CNPC_CombineElite::ThinkEngageCamo, CURTIME + 0.02f, "EngageCamoContext");
	SetContextThink(&CNPC_CombineElite::ThinkWeaponFrameUpdate, CURTIME + 0.03f, "WeaponFrameUpdateContext");
}

void CNPC_CombineElite::InputDisableCamo(inputdata_t &data)
{
	m_camo_override_bool = false;
	SetContextThink(&CNPC_CombineElite::ThinkTimeoutCamo, CURTIME + 0.1f, "TimeoutCamoContext");
	SetContextThink(&CNPC_CombineElite::ThinkWeaponFrameUpdate, CURTIME + 0.1f, "WeaponFrameUpdateContext");

}

BEGIN_DATADESC(CNPC_CombineElite)
DEFINE_KEYFIELD(m_iUseMarch, FIELD_INTEGER, "usemarch"),
DEFINE_FIELD(m_camostart_time, FIELD_TIME),
DEFINE_FIELD(m_camoend_time, FIELD_TIME),
DEFINE_FIELD(m_camo_damage_float, FIELD_FLOAT),
DEFINE_FIELD(m_camo_refract_value, FIELD_FLOAT),
DEFINE_FIELD(m_camo_fogginess_value, FIELD_FLOAT),
DEFINE_FIELD(m_camo_fleshtarget_handle, FIELD_EHANDLE),
DEFINE_FIELD(m_camo_override_bool, FIELD_BOOLEAN),
DEFINE_THINKFUNC(ThinkEngageCamo),
DEFINE_THINKFUNC(ThinkTimeoutCamo),
DEFINE_THINKFUNC(ThinkDisengageCamo),
DEFINE_THINKFUNC(ThinkWeaponFrameUpdate),
DEFINE_INPUTFUNC(FIELD_VOID, "CamoOn", InputEnableCamo),
DEFINE_INPUTFUNC(FIELD_VOID, "CamoOff", InputDisableCamo),
DEFINE_OUTPUT(m_OnCamoOn, "OnCamoOn"),
DEFINE_OUTPUT(m_OnCamoOff, "OnCamoOff"),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CNPC_CombineElite, DT_NPC_CombineElite)
SendPropFloat(SENDINFO(m_camo_refract_value), 0, SPROP_NOSCALE),
SendPropFloat(SENDINFO(m_camo_fogginess_value), 0, SPROP_NOSCALE),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(npc_combine_elite, CNPC_CombineElite);
PRECACHE_REGISTER(npc_combine_elite);