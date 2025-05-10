//========================================================================//
//
// Purpose: 
//
//=============================================================================//

#ifndef NPC_MANHACK_H
#define NPC_MANHACK_H
#ifdef _WIN32
#pragma once
#endif

#include "ai_basenpc_physicsflyer.h"
#include "sprite.h"
#include "env_spritetrail.h"
#include "player_pickup.h"

// Start with the engine off and folded up.
#define SF_MANHACK_PACKED_UP			(1 << 16)
#define SF_MANHACK_NO_DAMAGE_EFFECTS	(1 << 17)
#define SF_MANHACK_USE_AIR_NODES		(1 << 18)
#define SF_MANHACK_CARRIED				(1 << 19)	// Being carried by a metrocop
#define SF_MANHACK_NO_DANGER_SOUNDS		(1 << 20)
#ifdef DARKINTERVAL
#define SF_MANHACK_ARCADE_VERSION		(1 << 21) // different death effects, no ragdolls, and in the future, a new model
#endif
enum
{
	MANHACK_EYE_STATE_IDLE,
	MANHACK_EYE_STATE_CHASE,
	MANHACK_EYE_STATE_CHARGE,
	MANHACK_EYE_STATE_STUNNED,
};

//-----------------------------------------------------------------------------
// Attachment points.
//-----------------------------------------------------------------------------
#define	MANHACK_GIB_HEALTH				30
#define	MANHACK_INACTIVE_HEALTH			25
#define	MANHACK_MAX_SPEED				500
#define MANHACK_BURST_SPEED				650
#define MANHACK_NPC_BURST_SPEED			800

//-----------------------------------------------------------------------------
// Movement parameters.
//-----------------------------------------------------------------------------
#define MANHACK_WAYPOINT_DISTANCE		25	// Distance from waypoint that counts as arrival.

class CSprite;
class SmokeTrail;
class CSoundPatch;

//-----------------------------------------------------------------------------
// Manhack 
//-----------------------------------------------------------------------------
class CNPC_Manhack : public CNPCBaseInteractive<CAI_BasePhysicsFlyingBot>, public CDefaultPlayerPickupVPhysics
{
DECLARE_CLASS( CNPC_Manhack, CNPCBaseInteractive<CAI_BasePhysicsFlyingBot> );
DECLARE_SERVERCLASS();

public:
	CNPC_Manhack();
	~CNPC_Manhack();

	DECLARE_DATADESC();

	Class_T			Classify(void);

	void			Precache(void);
	void			Spawn(void);
	void			Activate();
#ifdef DARKINTERVAL
	bool			CreateVPhysics( void );
#else
	virtual bool	CreateVPhysics(void);
#endif
	void			PostNPCInit(void);
	void			UpdateOnRemove(void);

	void			TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr);
	int				OnTakeDamage_Alive( const CTakeDamageInfo &info );
	int				OnTakeDamage_Dying( const CTakeDamageInfo &info );
private:
	// Take damage from being thrown by a physcannon 
	void			TakeDamageFromPhyscannon(CBasePlayer *pPlayer);

	// Take damage from a vehicle: 
	void			TakeDamageFromVehicle(int index, gamevcollisionevent_t *pEvent);

	// Take damage from physics impacts
	void			TakeDamageFromPhysicsImpact(int index, gamevcollisionevent_t *pEvent);
public:
	void			Event_Dying(void);
	void			Event_Killed(const CTakeDamageInfo &info);
	bool			CorpseGib(const CTakeDamageInfo &info);
#ifdef DARKINTERVAL
	void			DeathSound( const CTakeDamageInfo &info );
#else
	virtual void	DeathSound(const CTakeDamageInfo &info);
#endif
	void			HandleAnimEvent( animevent_t *pEvent );
	bool			HandleInteraction(int interactionType, void* data, CBaseCombatCharacter* sourceEnt);
	Activity		NPC_TranslateActivity(Activity baseAct);
private:
	bool			IsFlyingActivity(Activity baseAct);
public:
	CBasePlayer		*HasPhysicsAttacker(float dt);
//private:
	// Are we being held by the physcannon?
	bool			IsHeldByPhyscannon();
//public:
private:
	bool			IsInEffectiveTargetZone(CBaseEntity *pTarget);
public:
	int				MeleeAttack1Conditions(float flDot, float flDist);
#ifdef DARKINTERVAL
	void			GatherEnemyConditions(CBaseEntity *pEnemy);
#else
	virtual void	GatherEnemyConditions(CBaseEntity *pEnemy);
#endif
	void			GatherConditions();
	
	void			OnStateChange(NPC_STATE OldState, NPC_STATE NewState);
#ifdef DARKINTERVAL
	int				TranslateSchedule(int scheduleType);
#else
	virtual int		TranslateSchedule(int scheduleType);
#endif
	void			PrescheduleThink(void);

	void			StartTask(const Task_t *pTask);
	void			RunTask(const Task_t *pTask);

	// INPCInteractive Functions
#ifdef DARKINTERVAL
	bool			CanInteractWith(CAI_BaseNPC *pUser) { return false; } // Disabled for now (sjb)
	bool			HasBeenInteractedWith() { return m_bHackedByAlyx; }
	void			NotifyInteraction(CAI_BaseNPC *pUser)
#else
	virtual bool	CanInteractWith(CAI_BaseNPC *pUser) { return false; } // Disabled for now (sjb)
	virtual	bool	HasBeenInteractedWith() { return m_bHackedByAlyx; }
	virtual void	NotifyInteraction(CAI_BaseNPC *pUser)
#endif
	{
		// Turn the sprites off and on again so their colors will change.
		KillSprites(0.0f);
		m_bHackedByAlyx = true;
		StartEye();
	}

	void			InputDisableSwarm(inputdata_t &inputdata);
	void			InputUnpack(inputdata_t &inputdata);
#ifdef DARKINTERVAL
	void			InputPowerdown(inputdata_t &inputdata);	
#else
	virtual void	InputPowerdown(inputdata_t &inputdata)
	{
		m_iHealth = 0;
	}
#endif
	void			TranslateNavGoal(CBaseEntity *pEnemy, Vector &chasePosition);
	float			GetDefaultNavGoalTolerance();
	bool			OverrideMove(float flInterval);
	void			MoveToTarget(float flInterval, const Vector &MoveTarget);
	void			MoveExecute_Alive(float flInterval);
	void			MoveExecute_Dead(float flInterval);
	int				MoveCollisionMask(void);
#ifdef DARKINTERVAL
	void			ClampMotorForces(Vector &linear, AngularImpulse &angular);
#else
	virtual void	ClampMotorForces(Vector &linear, AngularImpulse &angular);
#endif
private:
	void			MaintainGroundHeight(void);
	void			StartLoitering(const Vector &vecLoiterPosition);
	void			StopLoitering() { m_vecLoiterPosition = vec3_invalid; m_fTimeNextLoiterPulse = CURTIME; }
	bool			IsLoitering() { return m_vecLoiterPosition != vec3_invalid; }
	void			Loiter();
public:
	float			ManhackMaxSpeed(void);
	float			GetMaxEnginePower();

	void			VPhysicsShadowCollision(int index, gamevcollisionevent_t *pEvent);
	void			VPhysicsCollision(int index, gamevcollisionevent_t *pEvent);
	unsigned int	PhysicsSolidMaskForEntity(void) const;

	void			PlayFlySound(void);
#ifdef DARKINTERVAL
	void			StopLoopingSounds(void);
#else
	virtual void	StopLoopingSounds(void);
#endif
	void			BladesInit();
	void			SoundInit(void);
	void			StartEye(void);
private:
	void			ShowHostile(bool hostile = true);
public:
	void			CreateSmokeTrail();
	void			DestroySmokeTrail();
	void			KillSprites(float flDelay);
	void			TurnHeadRandomly( float flInterval );
	void			CrashTouch( CBaseEntity *pOther );
	void			StartEngine( bool fStartSound );
	void			CheckCollisions(float flInterval);
	void			SpinBlades(float flInterval);
	void			HitPhysicsObject(CBaseEntity *pOther);
	void			Slice(CBaseEntity *pHitEntity, float flInterval, trace_t &tr);
	void			Bump(CBaseEntity *pHitEntity, float flInterval, trace_t &tr);
	void			Splash(const Vector &vecSplashPos);

private:
	// Computes the slice bounce velocity
	void			ComputeSliceBounceVelocity(CBaseEntity *pHitEntity, trace_t &tr);
	void			StartBurst(const Vector &vecDirection);
	void			StopBurst(bool bInterruptSchedule = false);
	void			UpdatePanels(void);
	void			SetEyeState(int state);

public:
#ifdef DARKINTERVAL
	Vector			BodyTarget( const Vector &posSrc, bool bNoisy = true ) { return WorldSpaceCenter(); }
	float			GetHeadTurnRate(void) { return 45.0f; } // Degrees per second
#else 
	virtual Vector	BodyTarget(const Vector &posSrc, bool bNoisy = true) { return WorldSpaceCenter(); }
	virtual float	GetHeadTurnRate(void) { return 45.0f; } // Degrees per second
#endif
	void			Ignite(float flFlameLifetime, bool bNPCOnly, float flSize, bool bCalledByLevelDesigner) { return; } // DI todo - consider allowing them to be ignited?
	bool			ShouldGib(const CTakeDamageInfo &info)
	{
		return (m_bGib);
	}	
#ifdef DARKINTERVAL
	COutputEvent	m_OnUnpacked;
#endif
	// 	CDefaultPlayerPickupVPhysics
#ifdef DARKINTERVAL
	void			OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );
	void			OnPhysGunDrop( CBasePlayer *pPhysGunUser, PhysGunDrop_t Reason );
#else
	virtual void	OnPhysGunPickup(CBasePlayer *pPhysGunUser, PhysGunPickup_t reason);
	virtual void	OnPhysGunDrop(CBasePlayer *pPhysGunUser, PhysGunDrop_t Reason);
#endif
	DEFINE_CUSTOM_AI;

private:
	//
	// Movement variables.
	//

	Vector			m_vForceVelocity;		// Someone forced me to move

	Vector			m_vTargetBanking;

	Vector			m_vForceMoveTarget;		// Will fly here
	float			m_fForceMoveTime;		// If time is less than this
	Vector			m_vSwarmMoveTarget;		// Will fly here
	float			m_fSwarmMoveTime;		// If time is less than this
	float			m_fEnginePowerScale;	// scale all thrust by this amount (usually 1.0!)

	float			m_flNextEngineSoundTime;
	float			m_flEngineStallTime;

	float			m_flNextBurstTime;
	float			m_flBurstDuration;
	Vector			m_vecBurstDirection;

	float			m_flWaterSuspendTime;
	int				m_nLastSpinSound;

	// physics influence
	CHandle<CBasePlayer>	m_hPhysicsAttacker;
	float					m_flLastPhysicsInfluenceTime;

	// Death
	float			m_fSparkTime;
	float			m_fSmokeTime;

	bool			m_bDirtyPitch; // indicates whether we want the sound pitch updated.(sjb)
	bool			m_bShowingHostile;

	bool			m_bBladesActive;
	bool			m_bIgnoreClipbrushes;

	float			m_flBladeSpeed;

	CSprite			*m_pEyeGlow;
	CSprite			*m_pLightGlow;
	
	CHandle<SmokeTrail>	m_hSmokeTrail;

	int				m_iPanel1;
	int				m_iPanel2;
	int				m_iPanel3;
	int				m_iPanel4;

	int				m_nLastWaterLevel;
	bool			m_bDoSwarmBehavior;
	bool			m_bGib;

	bool			m_bHeld;
	bool			m_bHackedByAlyx;
	Vector			m_vecLoiterPosition;
	float			m_fTimeNextLoiterPulse;

	float			m_flBumpSuppressTime;

	CNetworkVar( int,	m_nEnginePitch1 );
	CNetworkVar( int,	m_nEnginePitch2 );
	CNetworkVar( float,	m_flEnginePitch1Time );
	CNetworkVar( float,	m_flEnginePitch2Time );

public:
	enum
	{
		COND_MANHACK_START_ATTACK = LAST_SHARED_CONDITION,	// We are able to do a bombing run on the current enemy.
	};

	enum
	{
		SCHED_MANHACK_ATTACK_HOVER = LAST_SHARED_SCHEDULE,
		SCHED_MANHACK_DEPLOY,
		SCHED_MANHACK_REGROUP,
		SCHED_MANHACK_SWARM_IDLE,
		SCHED_MANHACK_SWARM,
		SCHED_MANHACK_SWARM_FAILURE,
	};

	enum
	{
		TASK_MANHACK_HOVER = LAST_SHARED_TASK,
		TASK_MANHACK_UNPACK,
		TASK_MANHACK_FIND_SQUAD_CENTER,
		TASK_MANHACK_FIND_SQUAD_MEMBER,
		TASK_MANHACK_MOVEAT_SAVEPOSITION,
	};
};

#endif	//NPC_MANHACK_H
