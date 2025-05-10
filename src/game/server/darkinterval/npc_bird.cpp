#include "cbase.h"
#include "game.h"
#include "AI_BaseNPC.h"
#include "AI_Schedule.h"
#include "AI_Hull.h"
#include "AI_Hint.h"
#include "AI_Motor.h"
#include "hl2_shareddefs.h"
#include "NPCEvent.h"
#include "gib.h"
#include "AI_Interactions.h"
#include "ndebugoverlay.h"
#include "soundent.h"
#include "vstdlib/random.h"
#include "engine/ienginesound.h"
#include "movevars_shared.h"

//
// Spawnflags.
//
#define SF_BIRD_FLYING		16


#define CROW_TAKEOFF_SPEED		170
#define CROW_AIRSPEED			170 // FIXME: should be about 440, but I need to add acceleration


//
// Animation events.
//
static int AE_CROW_TAKEOFF;
static int AE_CROW_FLY;
static int AE_CROW_HOP;


//
// Custom schedules.
//
enum
{
	SCHED_BIRD_IDLE_WALK = LAST_SHARED_SCHEDULE,
	SCHED_BIRD_IDLE_FLY,

	//
	// Various levels of wanting to get away from something, selected
	// by current value of m_nMorale.
	//
	SCHED_BIRD_WALK_AWAY,
	SCHED_BIRD_RUN_AWAY,
	SCHED_BIRD_HOP_AWAY,
	SCHED_BIRD_FLY_AWAY,

	SCHED_BIRD_FLY,
	SCHED_BIRD_FLY_FAIL,
};


//
// Custom tasks.
//
enum
{
	TASK_BIRD_FIND_FLYTO_NODE = LAST_SHARED_TASK,
	//TASK_BIRD_PREPARE_TO_FLY,
	TASK_BIRD_TAKEOFF,
	//TASK_BIRD_LAND,
	TASK_BIRD_FLY,
	TASK_BIRD_FLY_TO_HINT,
	TASK_BIRD_PICK_RANDOM_GOAL,
	TASK_BIRD_PICK_EVADE_GOAL,
	TASK_BIRD_HOP,

	TASK_BIRD_FALL_TO_GROUND,
	TASK_BIRD_PREPARE_TO_FLY_RANDOM,
};


//
// Custom conditions.
//
enum
{
	COND_BIRD_ENEMY_TOO_CLOSE = LAST_SHARED_CONDITION,
	COND_BIRD_ENEMY_WAY_TOO_CLOSE,
	COND_BIRD_FORCED_FLY,
};


//
// Custom activities.
//
static int ACT_CROW_TAKEOFF;
static int ACT_CROW_SOAR;
static int ACT_CROW_LAND;


//
// Skill settings.
//
ConVar sk_bird_health("sk_bird_health", "5");
ConVar sk_bird_melee_dmg("sk_bird_melee_dmg", "0");


enum FlyState_t
{
	FlyState_Walking = 0,
	FlyState_Flying,
	FlyState_Falling,
	FlyState_Landing,
};


//-----------------------------------------------------------------------------
// The crow class.
//-----------------------------------------------------------------------------
class CNPC_Bird : public CAI_BaseNPC
{
	DECLARE_CLASS(CNPC_Bird, CAI_BaseNPC);

public:

	//
	// CBaseEntity:
	//
	virtual void Spawn(void);
	virtual void Precache(void);

	virtual Vector BodyTarget(const Vector &posSrc, bool bNoisy = true);
	virtual const Vector &WorldSpaceCenter(void) const;

	virtual int DrawDebugTextOverlays(void);

	//
	// CBaseCombatCharacter:
	//
	virtual int OnTakeDamage_Alive(const CTakeDamageInfo &info);
	virtual bool CorpseGib(const CTakeDamageInfo &info);

	virtual bool AllowedToIgnite(void) { return true; }
	virtual bool AllowedToIgniteByFlare(void) { return true; }

	//
	// CAI_BaseNPC:
	//
	virtual float MaxYawSpeed(void) { return 120.0f; }

	virtual Class_T Classify(void);
	virtual void GatherEnemyConditions(CBaseEntity *pEnemy);

	virtual void HandleAnimEvent(animevent_t *pEvent);
	virtual void PrescheduleThink(void);
	virtual int GetSoundInterests(void);
	virtual int SelectSchedule(void);
	virtual void StartTask(const Task_t *pTask);
	virtual void RunTask(const Task_t *pTask);

	virtual void OnChangeActivity(Activity eNewActivity);

	virtual bool OverrideMove(float flInterval);

	virtual bool FValidateHintType(CAI_Hint *pHint);
	virtual Activity GetHintActivity(short sHintType, Activity HintsActivity);

	virtual void PainSound(const CTakeDamageInfo &info);
	virtual void DeathSound(const CTakeDamageInfo &info);
	virtual void IdleSound(void);
	virtual void AlertSound(void);
	virtual void StopLoopingSounds(void);

	void InputFlyAway(inputdata_t &inputdata);

	DEFINE_CUSTOM_AI;
	DECLARE_DATADESC();

protected:

	void SetFlyingState(FlyState_t eState);
	inline bool IsFlying(void) const { return GetNavType() == NAV_FLY; }

	void Takeoff(CBaseEntity *pGoalEnt);
	void FlapSound(void);

	void MoveCrowFly(float flInterval);
	bool Probe(const Vector &vecMoveDir, float flSpeed, Vector &vecDeflect);

	float m_flGroundIdleMoveTime;

	float m_flEnemyDist;		// Distance to GetEnemy(), cached in GatherEnemyConditions.
	int m_nMorale;				// Used to determine which avoidance schedule to pick. Degrades as I pick avoidance schedules.

	bool m_bReachedMoveGoal;

	float m_flHopStartZ;		// Our Z coordinate when we started a hop. Used to check for accidentally hopping off things.
};

LINK_ENTITY_TO_CLASS(npc_bird, CNPC_Bird);

BEGIN_DATADESC(CNPC_Bird)

DEFINE_FIELD( m_flGroundIdleMoveTime, FIELD_TIME),
DEFINE_FIELD( m_flHopStartZ, FIELD_FLOAT),
DEFINE_FIELD( m_flEnemyDist, FIELD_FLOAT),
DEFINE_FIELD( m_nMorale, FIELD_INTEGER),
DEFINE_FIELD( m_bReachedMoveGoal, FIELD_BOOLEAN),

// Inputs
DEFINE_INPUTFUNC( FIELD_VOID, "FlyAway", InputFlyAway),

END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bird::Spawn(void)
{
	BaseClass::Spawn();

	Precache();

	SetModel("models/crow.mdl");
	m_iHealth = sk_bird_health.GetFloat();

	SetHullType(HULL_TINY);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_STEP);

	m_flFieldOfView = VIEW_FIELD_FULL;
	SetViewOffset(Vector(6, 0, 11));		// Position of the eyes relative to NPC's origin.

	m_flGroundIdleMoveTime = CURTIME + random->RandomFloat(0.0f, 5.0f);

	SetBloodColor(BLOOD_COLOR_RED);
	m_NPCState = NPC_STATE_NONE;

	m_nMorale = random->RandomInt(0, 12);

	SetCollisionGroup(HL2COLLISION_GROUP_CROW);

	CapabilitiesClear();

	bool bFlying = ((m_spawnflags & SF_BIRD_FLYING) != 0);
	SetFlyingState(bFlying ? FlyState_Flying : FlyState_Walking);

	// We don't mind zombies so much. They smell good!
	AddClassRelationship(CLASS_ZOMBIE, D_NU, 0);

	NPCInit();
}


//-----------------------------------------------------------------------------
// Purpose: Returns this monster's classification in the relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_Bird::Classify(void)
{
	return(CLASS_EARTH_FAUNA);
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pEnemy - 
//-----------------------------------------------------------------------------
void CNPC_Bird::GatherEnemyConditions(CBaseEntity *pEnemy)
{
	m_flEnemyDist = (GetLocalOrigin() - pEnemy->GetLocalOrigin()).Length();

	if (m_flEnemyDist < 256)
	{
		SetCondition(COND_BIRD_ENEMY_WAY_TOO_CLOSE);
	}

	if (m_flEnemyDist < 512)
	{
		SetCondition(COND_BIRD_ENEMY_TOO_CLOSE);
	}

	BaseClass::GatherEnemyConditions(pEnemy);
}


//-----------------------------------------------------------------------------
// Purpose: Returns the real center of the monster. The bounding box is much larger
//			than the actual creature so this is needed for targetting.
//-----------------------------------------------------------------------------
const Vector &CNPC_Bird::WorldSpaceCenter(void) const
{
	Vector &vecResult = AllocTempVector();
	vecResult = GetAbsOrigin();
	vecResult.z += 6;
	return vecResult;
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : posSrc - 
// Output : Vector
//-----------------------------------------------------------------------------
Vector CNPC_Bird::BodyTarget(const Vector &posSrc, bool bNoisy)
{
	return WorldSpaceCenter();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bird::StopLoopingSounds(void)
{
	//
	// Stop whatever flap sound might be playing.
	//
	StopSound("NPC_Crow.Flap");
	BaseClass::StopLoopingSounds();
}


//-----------------------------------------------------------------------------
// Purpose: Catches the monster-specific messages that occur when tagged
//			animation frames are played.
// Input  : pEvent - 
//-----------------------------------------------------------------------------
void CNPC_Bird::HandleAnimEvent(animevent_t *pEvent)
{
	if(pEvent->event == AE_CROW_TAKEOFF)
	{
		Takeoff(GetHintNode());
		return;
	}

	if (pEvent->event == AE_CROW_HOP)
	{
		RemoveFlag(FL_ONGROUND);

		//
		// Take him off ground so engine doesn't instantly reset FL_ONGROUND.
		//
		UTIL_SetOrigin(this, GetLocalOrigin() + Vector(0, 0, 1));

		//
		// How fast does the crow need to travel to reach the hop goal given gravity?
		//
		float flHopDistance = (m_vSavePosition - GetLocalOrigin()).Length();
		float gravity = sv_gravity.GetFloat();
		if (gravity <= 1)
		{
			gravity = 1;
		}

		float height = 0.25 * flHopDistance;
		float speed = sqrt(2 * gravity * height);
		float time = speed / gravity;

		//
		// Scale the sideways velocity to get there at the right time
		//
		Vector vecJumpDir = m_vSavePosition - GetLocalOrigin();
		vecJumpDir = vecJumpDir / time;

		//
		// Speed to offset gravity at the desired height.
		//
		vecJumpDir.z = speed;

		//
		// Don't jump too far/fast.
		//
		float distance = vecJumpDir.Length();
		if (distance > 650)
		{
			vecJumpDir = vecJumpDir * (650.0 / distance);
		}

		m_nMorale -= random->RandomInt(1, 6);
		if (m_nMorale <= 0)
		{
			m_nMorale = 0;
		}

		// Play a hop flap sound.
		EmitSound("NPC_Crow.Hop");

		SetAbsVelocity(vecJumpDir);
		return;
	}

	if (pEvent->event == AE_CROW_FLY)
	{
		//
		// Start flying.
		//
		SetActivity(ACT_FLY);
		return;
	}

	CAI_BaseNPC::HandleAnimEvent(pEvent);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : eNewActivity - 
//-----------------------------------------------------------------------------
void CNPC_Bird::OnChangeActivity(Activity eNewActivity)
{
	//	if ( eNewActivity == ACT_FLY )
	//	{
	//		m_flGroundSpeed = CROW_AIRSPEED;
	//	}
	//
	bool fRandomize = false;
	if (eNewActivity == ACT_FLY)
	{
		fRandomize = true;
	}

	BaseClass::OnChangeActivity(eNewActivity);
	if (fRandomize)
	{
	 SetCycle( random->RandomFloat(0.0, 0.75));
	}
}


//-----------------------------------------------------------------------------
// Purpose: Input handler that makes the crow fly away.
//-----------------------------------------------------------------------------
void CNPC_Bird::InputFlyAway(inputdata_t &inputdata)
{
	SetCondition(COND_BIRD_FORCED_FLY);
}


//-----------------------------------------------------------------------------
// Purpose: Handles all flight movement because we don't ever build paths when
//			when we are flying.
// Input  : flInterval - Seconds to simulate.
//-----------------------------------------------------------------------------
bool CNPC_Bird::OverrideMove(float flInterval)
{
	if (IsFlying())
	{
		if (m_bReachedMoveGoal)
		{
			SetIdealActivity((Activity)ACT_CROW_LAND);
			SetFlyingState(FlyState_Landing);
			TaskMovementComplete();
		}
		else
		{
			MoveCrowFly(flInterval);
		}
		return true;
	}
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: Handles all flight movement.
// Input  : flInterval - Seconds to simulate.
//-----------------------------------------------------------------------------
void CNPC_Bird::MoveCrowFly(float flInterval)
{
	//
	// Bound interval so we don't get ludicrous motion when debugging
	// or when framerate drops catastrophically.  
	//
	if (flInterval > 1.0)
	{
		flInterval = 1.0;
	}

	//
	// Determine the goal of our movement.
	//
	Vector vecMoveGoal;
	if (GetHintNode() != NULL)
	{
		// Fly towards the hint.
		vecMoveGoal = GetHintNode()->GetLocalOrigin();
	}
	else
	{
		// No movement goal.
		vecMoveGoal = GetLocalOrigin();
		SetAbsVelocity(vec3_origin);
		return;
	}

	//
	// Fly towards the movement goal.
	//
	Vector vecMoveDir = (vecMoveGoal - GetLocalOrigin());
	Vector vForward;
	AngleVectors(GetAbsAngles(), &vForward);

	float flDistance = vecMoveDir.Length();
	if (flDistance < CROW_AIRSPEED * flInterval)
	{
		m_bReachedMoveGoal = true;
	}

	//
	// Look to see if we are going to hit anything.
	//
	VectorNormalize(vecMoveDir);
	Vector vecDeflect;
	if (Probe(vecMoveDir, CROW_AIRSPEED, vecDeflect))
	{
		vecMoveDir = vecDeflect;
	}

	SetAbsVelocity(vecMoveDir * CROW_AIRSPEED);
	
	if (GetAbsVelocity().Length() > 0 && GetNavigator()->CurWaypointIsGoal() && flDistance < CROW_AIRSPEED * 2)
	{
		SetIdealActivity((Activity)ACT_CROW_LAND);
	}

	//
	// Try to face our flight direction.
	//
	GetMotor()->SetIdealYawAndUpdate(vecMoveDir, AI_KEEP_YAW_SPEED);
	
	//Bank and set angles.
	Vector vRight;
	QAngle vRollAngle;

	VectorAngles(vForward, vRollAngle);
	vRollAngle.z = 0;

	AngleVectors(vRollAngle, NULL, &vRight, NULL);

	float flRoll = DotProduct(vRight, vecMoveDir) * 45;
	flRoll = clamp(flRoll, -45, 45);

	vRollAngle[ROLL] = flRoll;
	SetAbsAngles(vRollAngle);
}


//-----------------------------------------------------------------------------
// Purpose: Looks ahead to see if we are going to hit something. If we are, a
//			recommended avoidance path is returned.
// Input  : vecMoveDir - 
//			flSpeed - 
//			vecDeflect - 
// Output : Returns true if we hit something and need to deflect our course,
//			false if all is well.
//-----------------------------------------------------------------------------
bool CNPC_Bird::Probe(const Vector &vecMoveDir, float flSpeed, Vector &vecDeflect)
{
	//
	// Look 2 seconds ahead.
	//
	trace_t tr;
	AI_TraceHull(GetAbsOrigin(), GetAbsOrigin() + vecMoveDir * flSpeed * 0.5, GetHullMins(), GetHullMaxs(), MASK_NPCSOLID, this, HL2COLLISION_GROUP_CROW, &tr);
	if (tr.fraction < 1.0f)
	{
		//
		// If we hit something, deflect flight path parallel to surface hit.
		//
		Vector vecUp;
		CrossProduct(vecMoveDir, tr.plane.normal, vecUp);
		CrossProduct(tr.plane.normal, vecUp, vecDeflect);
		VectorNormalize(vecDeflect);
		return true;
	}

	vecDeflect = vec3_origin;
	return false;
}


//-----------------------------------------------------------------------------
// Purpose: Switches between flying mode and ground mode.
//-----------------------------------------------------------------------------
void CNPC_Bird::SetFlyingState(FlyState_t eState)
{
	if (eState == FlyState_Flying)
	{
		// Flying
		RemoveFlag(FL_ONGROUND);
		AddFlag(FL_FLY);
		SetNavType(NAV_FLY);
		CapabilitiesRemove(bits_CAP_MOVE_GROUND);
		CapabilitiesAdd(bits_CAP_MOVE_FLY);
		SetMoveType(MOVETYPE_STEP);
	}
	else if (eState == FlyState_Walking)
	{
		// Walking
		RemoveFlag(FL_FLY);
		SetNavType(NAV_GROUND);
		CapabilitiesRemove(bits_CAP_MOVE_FLY);
		CapabilitiesAdd(bits_CAP_MOVE_GROUND);
		SetMoveType(MOVETYPE_STEP);
	}
	else
	{
		// Falling
		RemoveFlag(FL_FLY);
		SetNavType(NAV_GROUND);
		CapabilitiesRemove(bits_CAP_MOVE_FLY);
		CapabilitiesAdd(bits_CAP_MOVE_GROUND);
		SetMoveType(MOVETYPE_STEP);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Performs a takeoff. Called via an animation event at the moment
//			our feet leave the ground.
// Input  : pGoalEnt - The entity that we are going to fly toward.
//-----------------------------------------------------------------------------
void CNPC_Bird::Takeoff(CBaseEntity *pGoalEnt)
{
	if (pGoalEnt != NULL)
	{
		SetFlyingState(FlyState_Flying);

		//
		// Lift us off ground so engine doesn't instantly reset FL_ONGROUND.
		//
		UTIL_SetOrigin(this, GetLocalOrigin() + Vector(0, 0, 1));

		//
		// Fly straight at the goal entity at our maximum airspeed.
		//
		Vector vecMoveDir = pGoalEnt->GetLocalOrigin() - GetLocalOrigin();
		VectorNormalize(vecMoveDir);
		SetAbsVelocity(vecMoveDir * CROW_TAKEOFF_SPEED);
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bird::PrescheduleThink(void)
{
	BaseClass::PrescheduleThink();
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pTask - 
//-----------------------------------------------------------------------------
void CNPC_Bird::StartTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
		//
		// This task enables us to build a path that requires flight.
		//
		//		case TASK_BIRD_PREPARE_TO_FLY:
		//		{
		//			SetFlyingState( FlyState_Flying );
		//			TaskComplete();
		//			break;
		//		}

	case TASK_BIRD_TAKEOFF:
	{
		if (random->RandomInt(1, 4) < 4)
		{
			AlertSound();
		}

		FlapSound();

		SetIdealActivity((Activity)ACT_CROW_TAKEOFF);
		break;
	}

	case TASK_BIRD_FLY_TO_HINT:
	{
		break;
	}

	case TASK_BIRD_PICK_EVADE_GOAL:
	{
		if (GetEnemy() != NULL)
		{
			//
			// Get our enemy's position in x/y.
			//
			Vector vecEnemyOrigin = GetEnemy()->GetLocalOrigin();
			vecEnemyOrigin.z = GetLocalOrigin().z;

			//
			// Pick a hop goal a random distance along a vector away from our enemy.
			//
			m_vSavePosition = GetLocalOrigin() - vecEnemyOrigin;
			VectorNormalize(m_vSavePosition);
			m_vSavePosition = GetLocalOrigin() + m_vSavePosition * (32 + random->RandomInt(0, 32));

			GetMotor()->SetIdealYawToTarget(m_vSavePosition);
			TaskComplete();
		}
		else
		{
			TaskFail("No enemy");
		}
		break;
	}

	case TASK_BIRD_FALL_TO_GROUND:
	{
		SetFlyingState(FlyState_Falling);
		break;
	}

	case TASK_FIND_HINTNODE:
	{
		// Overloaded because we search over a greater distance.
		if (!GetHintNode())
		{
			CAI_Hint *pHint = CAI_HintManager::FindHint(this, HINT_CROW_FLYTO_POINT, bits_HINT_NODE_NEAREST | bits_HINT_NOT_CLOSE_TO_ENEMY, 5000);
			SetHintNode(pHint);		
		}

		if (GetHintNode())
		{
			m_bReachedMoveGoal = false;
			TaskComplete();
		}
		else
		{
			TaskFail(FAIL_NO_HINT_NODE);
		}
		break;
	}

	//
	// We have failed to fly normally. Pick a random "up" direction and fly that way.
	//
	case TASK_BIRD_FLY:
	{
		float flYaw = UTIL_AngleMod(random->RandomInt(-180, 180));

		Vector vecNewVelocity(cos(DEG2RAD(flYaw)), sin(DEG2RAD(flYaw)), random->RandomFloat(0.1f, 0.5f));
		vecNewVelocity *= CROW_AIRSPEED;
		SetAbsVelocity(vecNewVelocity);

		SetIdealActivity(ACT_FLY);
		break;
	}

	case TASK_BIRD_PICK_RANDOM_GOAL:
	{
		m_vSavePosition = GetLocalOrigin() + Vector(random->RandomFloat(-48.0f, 48.0f), random->RandomFloat(-48.0f, 48.0f), 0);
		TaskComplete();
		break;
	}

	case TASK_BIRD_HOP:
	{
		SetIdealActivity(ACT_HOP);
		m_flHopStartZ = GetLocalOrigin().z;
		break;
	}

	default:
	{
		BaseClass::StartTask(pTask);
	}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pTask - 
//-----------------------------------------------------------------------------
void CNPC_Bird::RunTask(const Task_t *pTask)
{
	switch (pTask->iTask)
	{
	case TASK_BIRD_TAKEOFF:
	{
		if (GetHintNode() != NULL)
		{
			GetMotor()->SetIdealYawToTargetAndUpdate(GetAbsOrigin() + GetHintNode()->GetAbsOrigin(), AI_KEEP_YAW_SPEED);
		}

		if (IsActivityFinished())
		{
			TaskComplete();
			SetIdealActivity(ACT_FLY);
		}

		break;
	}

	case TASK_BIRD_FLY_TO_HINT:
	{
		if (m_bReachedMoveGoal)
		{
			SetHintNode(NULL);
			TaskComplete();
		}
		break;
	}

	case TASK_BIRD_HOP:
	{
		if (IsActivityFinished())
		{
			TaskComplete();
			SetIdealActivity(ACT_IDLE);
		}

		if ((GetAbsOrigin().z < m_flHopStartZ) && (!(GetFlags() & FL_ONGROUND)))
		{
			//
			// We've hopped off of something! See if we're going to fall very far.
			//
			trace_t tr;
			AI_TraceLine(GetAbsOrigin(), GetAbsOrigin() + Vector(0, 0, -32), MASK_SOLID, this, HL2COLLISION_GROUP_CROW, &tr);
			if (tr.fraction == 1.0f)
			{
				//
				// We're falling! Better fly away. SelectSchedule will check ONGROUND and do the right thing.
				//
				TaskComplete();
			}
			else
			{
				//
				// We'll be okay. Don't check again unless what we're hopping onto moves
				// out from under us.
				//
				m_flHopStartZ = GetAbsOrigin().z - (32 * tr.fraction);
			}
		}

		break;
	}

	//
	// Face the direction we are flying.
	//
	case TASK_BIRD_FLY:
	{
		GetMotor()->SetIdealYawToTargetAndUpdate(GetAbsOrigin() + GetAbsVelocity(), AI_KEEP_YAW_SPEED);

		break;
	}

	case TASK_BIRD_FALL_TO_GROUND:
	{
		if (GetFlags() & FL_ONGROUND)
		{
			SetFlyingState(FlyState_Walking);
			TaskComplete();
		}
		break;
	}

	default:
	{
		CAI_BaseNPC::RunTask(pTask);
	}
	}
}


//------------------------------------------------------------------------------
// Purpose: Override to do crow specific gibs.
// Output : Returns true to gib, false to not gib.
//-----------------------------------------------------------------------------
bool CNPC_Bird::CorpseGib(const CTakeDamageInfo &info)
{
	EmitSound("NPC_Crow.Gib");

	// TODO: crow gibs?
	//CGib::SpawnSpecificGibs( this, CROW_GIB_COUNT, 300, 400, "models/gibs/crow_gibs.mdl");

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CNPC_Bird::FValidateHintType(CAI_Hint *pHint)
{
	return(pHint->HintType() == HINT_CROW_FLYTO_POINT);
}


//-----------------------------------------------------------------------------
// Purpose: Returns the activity for the given hint type.
// Input  : sHintType - 
//-----------------------------------------------------------------------------
Activity CNPC_Bird::GetHintActivity(short sHintType, Activity HintsActivity)
{
	if (sHintType == HINT_CROW_FLYTO_POINT)
	{
		return ACT_FLY;
	}

	return BaseClass::GetHintActivity(sHintType, HintsActivity);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pevInflictor - 
//			pevAttacker - 
//			flDamage - 
//			bitsDamageType - 
//-----------------------------------------------------------------------------
int CNPC_Bird::OnTakeDamage_Alive(const CTakeDamageInfo &info)
{
	// TODO: spew a feather or two
	return CAI_BaseNPC::OnTakeDamage_Alive(info);
}


//-----------------------------------------------------------------------------
// Purpose: Returns the best new schedule for this NPC based on current conditions.
//-----------------------------------------------------------------------------
int CNPC_Bird::SelectSchedule(void)
{
	//
	// If we're flying, just find somewhere to fly to.
	//
	if (IsFlying())
	{
		return SCHED_BIRD_IDLE_FLY;
	}

	//
	// If we were told to fly away via our FlyAway input, do so ASAP.
	//
	if (HasCondition(COND_BIRD_FORCED_FLY))
	{
		ClearCondition(COND_BIRD_FORCED_FLY);
		return SCHED_BIRD_FLY_AWAY;
	}

	//
	// If we're not flying but we're not on the ground, start flying.
	// Maybe we hopped off of something? Don't do this immediately upon
	// because we may be falling to the ground on spawn.
	//
	if (!(GetFlags() & FL_ONGROUND) && (CURTIME > 2.0))
	{
		return SCHED_BIRD_FLY_AWAY;
	}

	//
	// If we heard a gunshot or have taken damage, fly away.
	//
	if (HasCondition(COND_HEAR_DANGER) || HasCondition(COND_HEAR_COMBAT) ||
		HasCondition(COND_LIGHT_DAMAGE) || HasCondition(COND_HEAVY_DAMAGE))
	{
		return SCHED_BIRD_FLY_AWAY;
	}

	//
	// If someone we hate is getting WAY too close for comfort, fly away.
	//
	if (HasCondition(COND_BIRD_ENEMY_WAY_TOO_CLOSE))
	{
		ClearCondition(COND_BIRD_ENEMY_WAY_TOO_CLOSE);

		m_nMorale = 0;
		return SCHED_BIRD_FLY_AWAY;
	}

	//
	// If someone we hate is getting a little too close for comfort, avoid them.
	//
	if (HasCondition(COND_BIRD_ENEMY_TOO_CLOSE))
	{
		ClearCondition(COND_BIRD_ENEMY_TOO_CLOSE);

		if (m_flEnemyDist > 400)
		{
			return SCHED_BIRD_WALK_AWAY;
		}
		else if (m_flEnemyDist > 300)
		{
			m_nMorale -= 1;
			return SCHED_BIRD_RUN_AWAY;
		}
		else
		{
			m_nMorale -= 2;
			return SCHED_BIRD_HOP_AWAY;
		}
	}

	switch (m_NPCState)
	{
	case NPC_STATE_IDLE:
	case NPC_STATE_ALERT:
	case NPC_STATE_COMBAT:
	{
		if (!IsFlying())
		{
			//
			// If we are hanging out on the ground, see if it is time to pick a new place to walk to.
			//
			if (CURTIME > m_flGroundIdleMoveTime)
			{
				m_flGroundIdleMoveTime = CURTIME + random->RandomFloat(10.0f, 20.0f);
				return SCHED_BIRD_IDLE_WALK;
			}

			return SCHED_BIRD_IDLE_WALK;
		}

		// TODO: need idle flying behaviors!
	}
	}

	return BaseClass::SelectSchedule();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Bird::Precache(void)
{
	BaseClass::Precache();

	engine->PrecacheModel("models/crow.mdl");
}


//-----------------------------------------------------------------------------
// Purpose: Sounds.
//-----------------------------------------------------------------------------
void CNPC_Bird::IdleSound(void)
{
	EmitSound("NPC_Crow.Idle");
}


void CNPC_Bird::AlertSound(void)
{
	EmitSound("NPC_Crow.Alert");
}


void CNPC_Bird::PainSound(const CTakeDamageInfo &info)
{
	EmitSound("NPC_Crow.Pain");
}


void CNPC_Bird::DeathSound(const CTakeDamageInfo &info)
{
	EmitSound("NPC_Crow.Die");
}


void CNPC_Bird::FlapSound(void)
{
	EmitSound("NPC_Crow.Flap");
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CNPC_Bird::DrawDebugTextOverlays(void)
{
	int nOffset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT)
	{
		char tempstr[512];
		Q_snprintf(tempstr, sizeof(tempstr), "morale: %d", m_nMorale);
		NDebugOverlay::EntityText(entindex(), nOffset, tempstr, 0);
		nOffset++;

		if (GetEnemy() != NULL)
		{
			Q_snprintf(tempstr, sizeof(tempstr), "enemy (dist): %s (%g)", GetEnemy()->GetClassname(), (double)m_flEnemyDist);
			NDebugOverlay::EntityText(entindex(), nOffset, tempstr, 0);
			nOffset++;
		}
	}

	return nOffset;
}


//-----------------------------------------------------------------------------
// Purpose: Determines which sounds the crow cares about.
//-----------------------------------------------------------------------------
int CNPC_Bird::GetSoundInterests(void)
{
	return	SOUND_WORLD | SOUND_COMBAT | SOUND_PLAYER | SOUND_DANGER;
}


//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC(npc_BIRD, CNPC_Bird)

DECLARE_TASK(TASK_BIRD_FIND_FLYTO_NODE)
//DECLARE_TASK( TASK_BIRD_PREPARE_TO_FLY )
DECLARE_TASK(TASK_BIRD_TAKEOFF)
DECLARE_TASK(TASK_BIRD_FLY)
DECLARE_TASK(TASK_BIRD_FLY_TO_HINT)
DECLARE_TASK(TASK_BIRD_PICK_RANDOM_GOAL)
DECLARE_TASK(TASK_BIRD_HOP)
DECLARE_TASK(TASK_BIRD_PICK_EVADE_GOAL)

// experiment
DECLARE_TASK(TASK_BIRD_FALL_TO_GROUND)
DECLARE_TASK(TASK_BIRD_PREPARE_TO_FLY_RANDOM)

DECLARE_ACTIVITY(ACT_CROW_TAKEOFF)
DECLARE_ACTIVITY(ACT_CROW_LAND)

DECLARE_CONDITION(COND_BIRD_ENEMY_TOO_CLOSE)
DECLARE_CONDITION(COND_BIRD_ENEMY_WAY_TOO_CLOSE)
DECLARE_CONDITION(COND_BIRD_FORCED_FLY)

DECLARE_ANIMEVENT(AE_CROW_HOP)
DECLARE_ANIMEVENT(AE_CROW_FLY)
DECLARE_ANIMEVENT(AE_CROW_TAKEOFF)
//=========================================================
DEFINE_SCHEDULE
(
	SCHED_BIRD_IDLE_WALK,

	"	Tasks"
	"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_IDLE_STAND"
	"		TASK_BIRD_PICK_RANDOM_GOAL		0"
	"		TASK_GET_PATH_TO_SAVEPOSITION	0"
	"		TASK_WALK_PATH					0"
	"		TASK_WAIT_FOR_MOVEMENT			0"
	"		TASK_WAIT_PVS					0"
	"		"
	"	Interrupts"
	"		COND_BIRD_FORCED_FLY"
	"		COND_BIRD_ENEMY_TOO_CLOSE"
	"		COND_NEW_ENEMY"
	"		COND_HEAVY_DAMAGE"
	"		COND_LIGHT_DAMAGE"
	"		COND_HEAVY_DAMAGE"
	"		COND_HEAR_DANGER"
	"		COND_HEAR_COMBAT"
)

//=========================================================
DEFINE_SCHEDULE
(
	SCHED_BIRD_WALK_AWAY,

	"	Tasks"
	"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_BIRD_FLY_AWAY"
	"		TASK_BIRD_PICK_EVADE_GOAL		0"
	"		TASK_GET_PATH_TO_SAVEPOSITION	0"
	"		TASK_WALK_PATH					0"
	"		TASK_WAIT_FOR_MOVEMENT			0"
	"		"
	"	Interrupts"
	"		COND_BIRD_FORCED_FLY"
	"		COND_BIRD_ENEMY_WAY_TOO_CLOSE"
	"		COND_NEW_ENEMY"
	"		COND_HEAVY_DAMAGE"
	"		COND_LIGHT_DAMAGE"
	"		COND_HEAVY_DAMAGE"
	"		COND_HEAR_DANGER"
	"		COND_HEAR_COMBAT"
)

//=========================================================
DEFINE_SCHEDULE
(
	SCHED_BIRD_RUN_AWAY,

	"	Tasks"
	"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_BIRD_FLY_AWAY"
	"		TASK_BIRD_PICK_EVADE_GOAL		0"
	"		TASK_GET_PATH_TO_SAVEPOSITION	0"
	"		TASK_RUN_PATH					0"
	"		TASK_WAIT_FOR_MOVEMENT			0"
	"		"
	"	Interrupts"
	"		COND_BIRD_FORCED_FLY"
	"		COND_BIRD_ENEMY_WAY_TOO_CLOSE"
	"		COND_NEW_ENEMY"
	"		COND_HEAVY_DAMAGE"
	"		COND_LIGHT_DAMAGE"
	"		COND_HEAVY_DAMAGE"
	"		COND_HEAR_DANGER"
	"		COND_HEAR_COMBAT"
)

//=========================================================
DEFINE_SCHEDULE
(
	SCHED_BIRD_HOP_AWAY,

	"	Tasks"
	"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_BIRD_FLY_AWAY"
	"		TASK_STOP_MOVING				0"
	"		TASK_BIRD_PICK_EVADE_GOAL		0"
	"		TASK_FACE_IDEAL					0"
	"		TASK_BIRD_HOP					0"
	"	"
	"	Interrupts"
	"		COND_BIRD_FORCED_FLY"
	"		COND_HEAVY_DAMAGE"
	"		COND_LIGHT_DAMAGE"
	"		COND_HEAVY_DAMAGE"
	"		COND_HEAR_DANGER"
	"		COND_HEAR_COMBAT"
)

//=========================================================
DEFINE_SCHEDULE
(
	SCHED_BIRD_IDLE_FLY,

	"	Tasks"
	"		TASK_WAIT_RANDOM				0.1"
	"		TASK_FIND_HINTNODE				0"
	"		TASK_BIRD_FLY_TO_HINT			0"
	"		"
	"	Interrupts"
)

//=========================================================
DEFINE_SCHEDULE
(
	SCHED_BIRD_FLY_AWAY,

	"	Tasks"
	"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_BIRD_FLY_FAIL"
	"		TASK_STOP_MOVING				0"
	"		TASK_FIND_HINTNODE				0"
	"		TASK_BIRD_TAKEOFF				0"
	"		TASK_BIRD_FLY_TO_HINT			0"
	"		TASK_WAIT_INDEFINITE			0"
	"	"
	"	Interrupts"
)

//=========================================================
DEFINE_SCHEDULE
(
	SCHED_BIRD_FLY,

	"	Tasks"
	"		TASK_SET_FAIL_SCHEDULE			SCHEDULE:SCHED_BIRD_FLY_FAIL"
	"		TASK_STOP_MOVING				0"
	"		TASK_BIRD_TAKEOFF				0"
	"		TASK_BIRD_FLY					0"
	"	"
	"	Interrupts"
)

//=========================================================
DEFINE_SCHEDULE
(
	SCHED_BIRD_FLY_FAIL,

	"	Tasks"
	"		TASK_BIRD_FALL_TO_GROUND		0"
	"		TASK_SET_SCHEDULE				SCHEDULE:SCHED_BIRD_HOP_AWAY"
	"	"
	"	Interrupts"
)

AI_END_CUSTOM_NPC()
