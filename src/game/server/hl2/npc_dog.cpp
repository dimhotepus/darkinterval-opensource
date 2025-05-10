//========================================================================//
//
// Purpose: Implements d0g, the loving and caring head crushing Alyx companion.
//
//=============================================================================//

#include "cbase.h"
#include "npcevent.h"
#include "ai_baseactor.h"
#include "ai_basenpc.h"
#ifdef DARKINTERVAL
#include "ai_behavior_actbusy.h" // todo - add these behaviors.
#include "ai_behavior_assault.h"
#endif
#include "ai_behavior_follow.h"
#ifdef DARKINTERVAL
#include "ai_behavior_lead.h"
#endif
#include "ai_network.h"
#include "ai_navigator.h"
#include "ai_motor.h"
#include "ai_hull.h"
#include "beam_shared.h"
#include "collisionutils.h"
#ifdef DARKINTERVAL
#include "filters.h" // DI's Dog has a filter on what he can/t pick up
#endif
#include "npc_rollermine.h"
#include "saverestore_utlvector.h"
#include "physics_bone_follower.h"
#include "sprite.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define EFFECT_COUNT 4

extern ConVar ai_debug_avoidancebounds;

class CNPC_Dog : public CAI_BaseActor
{
public:
	DECLARE_DATADESC();
	DECLARE_CLASS( CNPC_Dog, CAI_BaseActor );
	Class_T Classify ( void );
	void Precache(void);
	void Spawn( void );
#ifdef DARKINTERVAL
	void Activate(void); // for pickup filters
#endif
	void StartTask( const Task_t *pTask );
	void HandleAnimEvent( animevent_t *pEvent );
	int	 SelectSchedule( void );

#ifdef DARKINTERVAL	// I'm tired of Dog turning around because of footsteps and gunfire.
	int	GetSoundInterests(void) {
		return ( SOUND_DANGER | SOUND_BULLET_IMPACT | SOUND_PHYSICS_DANGER );
	};
#endif

	bool FindPhysicsObject( const char *pPickupName, CBaseEntity *pIgnore = NULL );
	void RunTask( const Task_t *pTask );
#ifdef DARKINTERVAL
	void CreateBeams( bool bFake = false ); // to be able to have beams even without valid targets
#else
	void CreateBeams(void);
#endif
	void ClearBeams( void );

	void PrescheduleThink( void );

	bool CanTargetSeeMe( void );

	Vector	FacingPosition( void ) { return WorldSpaceCenter(); }
	float	GetHeadDebounce( void ) { return 0.8; } // how much of previous head turn to use

	void InputSetPickupTarget( inputdata_t &inputdata );
	void InputStartCatchThrowBehavior( inputdata_t &inputdata );
	void InputStopCatchThrowBehavior( inputdata_t &inputdata );
	void InputPlayerPickupObject( inputdata_t &inputdata );

	void InputStartWaitAndCatch( inputdata_t &inputdata );
	void InputStopWaitAndCatch( inputdata_t &inputdata );
	void InputSetThrowArcModifier( inputdata_t &inputdata );
	void InputSetThrowTarget( inputdata_t &inputdata );
	
	void InputTurnBoneFollowersOff( inputdata_t &inputdata );
	void InputTurnBoneFollowersOn( inputdata_t &inputdata );
#ifdef DARKINTERVAL
	void InputTurnOnPhysgunFx(inputdata_t &inputdata); // to be able to have beams even without valid targets
	void InputTurnOffPhysgunFx(inputdata_t &inputdata);
#endif
	void CleanCatchAndThrow( bool bClearTimers = true );
	void SetTurnActivity ( void );
	void ThrowObject( const char *pAttachmentName );
	void PickupOrCatchObject( const char *pAttachmentName );
	void PullObject( bool bMantain );
	void SetupThrowTarget( void );

	void GatherConditions( void );
#ifdef DARKINTERVAL
	Activity NPC_TranslateActivity( Activity newActivity );
#endif
	Disposition_t IRelationType( CBaseEntity *pTarget );

	int OnTakeDamage_Alive( const CTakeDamageInfo &info );

	void	MantainBoneFollowerCollisionGroups( int CollisionGroup );
	virtual void SetPlayerAvoidState( void );

protected:
	enum
	{
		COND_DOG_LOST_PHYSICS_ENTITY = BaseClass::NEXT_CONDITION,

		NEXT_CONDITION,
	};

protected:
	float m_flNextSwat;
	float m_flTimeToCatch;
	float m_flTimeToPull;
	EHANDLE m_hPhysicsEnt;
	EHANDLE m_hThrowTarget;
#ifdef DARKINTERVAL
	bool  m_force_physgun_fx_bool; // to be able to have beams even without valid targets
#endif
	int	  m_iPhysGunAttachment;
	bool  m_bDoCatchThrowBehavior;
	bool  m_bDoWaitforObjectBehavior;
	string_t m_sObjectName;

	COutputEvent	m_OnThrow;
	COutputEvent	m_OnCatch;
	COutputEvent	m_OnPickup;

	float			m_flThrowArcModifier;
	int				m_iContainerMoveType;
	float			m_flNextRouteTime;

	bool			m_bHasObject;
	bool			m_bBeamEffects;
	
	CUtlVector< CHandle <CBaseEntity> > m_hUnreachableObjects;

	// Contained Bone Follower manager
	CBoneFollowerManager m_BoneFollowerManager;

	bool CreateVPhysics( void );
	void UpdateOnRemove( void );
	void NPCThink( void );
	void Event_Killed( const CTakeDamageInfo &info );

	void CreateSprites( void );
	void ClearSprites( void );
	CHandle<CSprite> m_hGlowSprites[EFFECT_COUNT];
	CHandle<CBeam>  m_hBeams[EFFECT_COUNT]; //This is temp.

	virtual bool CreateBehaviors( void );
	CAI_FollowBehavior		m_FollowBehavior;
#ifdef DARKINTERVAL
	CAI_LeadBehavior		m_LeadBehavior;
	CAI_ActBusyBehavior		m_ActBusyBehavior;
#endif
	bool	m_bBoneFollowersActive;
#ifdef DARKINTERVAL
	void	InputSetPickupFilter(inputdata_t &input);
private:
	CHandle<CBaseFilter> m_dog_pickup_filter_handle; // to prevent Dog from picking up certain objects he can't ever pick up (stuck/scripted physics)
	string_t m_dog_pickup_filter_name_string;
#endif
protected:
	
	DEFINE_CUSTOM_AI;
};

LINK_ENTITY_TO_CLASS( npc_dog, CNPC_Dog );

#define DOG_PHYSOBJ_MOVE_TO_DIST	96
#define DOG_PULL_DISTANCE			200
#define DOG_CATCH_DISTANCE			48
#define DOG_PULL_VELOCITY_MOD		0.1f
#define DOG_PULL_ANGULARIMP_MOD		0.8f
#define DOG_PULL_TO_GUN_VEL_MOD		2.0f
#define DOG_MAX_THROW_MASS			250.0f
#define DOG_PHYSGUN_ATTACHMENT_NAME "physgun"

// These bones have physics shadows
static const char *pFollowerBoneNames[] =
{
	// head
	"Dog_Model.Eye",
	"Dog_Model.Pelvis",
};

enum
{
	SCHED_DOG_FIND_OBJECT = LAST_SHARED_SCHEDULE,
	SCHED_DOG_CATCH_OBJECT,
	SCHED_DOG_WAIT_THROW_OBJECT,
};

//=========================================================
// tasks
//=========================================================
enum 
{
	TASK_DOG_DELAY_SWAT = LAST_SHARED_TASK,
	TASK_DOG_GET_PATH_TO_PHYSOBJ,
	TASK_DOG_PICKUP_ITEM,
	TASK_DOG_LAUNCH_ITEM,
	TASK_DOG_FACE_OBJECT,
	TASK_DOG_WAIT_FOR_OBJECT,
	TASK_DOG_CATCH_OBJECT,
	TASK_DOG_WAIT_FOR_TARGET_TO_FACE,
	TASK_DOG_SETUP_THROW_TARGET,
};

int ACT_DOG_THROW;
int ACT_DOG_PICKUP;
int ACT_DOG_WAITING;
int ACT_DOG_CATCH;

int AE_DOG_THROW;
int AE_DOG_PICKUP;
int AE_DOG_CATCH;
int AE_DOG_PICKUP_NOEFFECT;

#ifndef DARKINTERVAL // reducing amount of convars
ConVar dog_max_wait_time( "dog_max_wait_time", "7" );
ConVar dog_debug( "dog_debug", "0" );
#endif
//-----------------------------------------------------------------------------
// Classify - indicates this NPC's place in the 
// relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_Dog::Classify ( void )
{
	return	CLASS_PLAYER_ALLY_VITAL;
}
//-----------------------------------------------------------------------------
// Precache - precaches all resources this NPC needs
//-----------------------------------------------------------------------------
void CNPC_Dog::Precache(void)
{
#ifdef DARKINTERVAL
	PrecacheModel("models/_Characters/dog.mdl");
#else
	PrecacheModel("models/dog.mdl");
#endif
	PrecacheScriptSound("Weapon_PhysCannon.Launch");

	PrecacheModel("sprites/orangelight1.vmt");
	PrecacheModel("sprites/physcannon_bluelight2.vmt");
	PrecacheModel("sprites/glow04_noz.vmt");

	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CNPC_Dog::Spawn(void)
{
	m_bBoneFollowersActive = true;

	Precache();

	BaseClass::Spawn();
#ifdef DARKINTERVAL
	SetModel("models/_Characters/dog.mdl"); // DI todo - replace with script-based
#else
	SetModel("models/dog.mdl");
#endif
	SetHullType(HULL_WIDE_HUMAN);
	SetHullSizeNormal();

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_STEP);
	SetBloodColor(BLOOD_COLOR_MECH);

	m_iHealth = 999;
	m_flFieldOfView = 0.5;// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	m_NPCState = NPC_STATE_NONE;
#ifdef DARKINTERVAL
	m_takedamage = DAMAGE_YES;
#else
	m_takedamage = DAMAGE_NO;
#endif
	CapabilitiesAdd(bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS | bits_CAP_TURN_HEAD | bits_CAP_ANIMATEDFACE);
	CapabilitiesAdd(bits_CAP_FRIENDLY_DMG_IMMUNE);
#ifdef DARKINTERVAL
	CapabilitiesAdd(bits_CAP_LIVE_RAGDOLL);
#endif
	NPCInit();

	m_iPhysGunAttachment = LookupAttachment(DOG_PHYSGUN_ATTACHMENT_NAME);

	m_bDoCatchThrowBehavior = false;
	m_bDoWaitforObjectBehavior = false;
	m_bHasObject = false;
	m_bBeamEffects = true;

	m_flThrowArcModifier = 1.0f;

	m_flNextSwat = CURTIME;
	m_flNextRouteTime = CURTIME;
}

bool CNPC_Dog::CreateBehaviors( void )
{
	AddBehavior(&m_FollowBehavior );
#ifdef DARKINTERVAL
	AddBehavior(&m_ActBusyBehavior);
	AddBehavior(&m_LeadBehavior);
#endif
	return BaseClass::CreateBehaviors();
}
#ifdef DARKINTERVAL
void CNPC_Dog::Activate(void)
{
	BaseClass::Activate();

	// Get a handle to my enemy filter entity if there is one.
	if (m_dog_pickup_filter_name_string != NULL_STRING)
	{
		CBaseEntity *pFilter = gEntList.FindEntityByName(NULL, m_dog_pickup_filter_name_string);
		if (pFilter != NULL)
		{
			m_dog_pickup_filter_handle = dynamic_cast<CBaseFilter*>(pFilter);
		}
	}
}
#endif
Disposition_t CNPC_Dog::IRelationType( CBaseEntity *pTarget )
{
	if ( NPC_Rollermine_IsRollermine( pTarget ) )
	{
		if ( pTarget->HasSpawnFlags( SF_ROLLERMINE_FRIENDLY ) )
			 return D_LI;
	}

	return BaseClass::IRelationType( pTarget );
}

//---------------------------------------------------------
//---------------------------------------------------------
bool CNPC_Dog::CreateVPhysics( void )
{
	BaseClass::CreateVPhysics();

	if ( m_bBoneFollowersActive == true && !m_BoneFollowerManager.GetNumBoneFollowers() )
	{
		m_BoneFollowerManager.InitBoneFollowers( this, ARRAYSIZE(pFollowerBoneNames), pFollowerBoneNames );
	}
	return true;
}

//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Dog::UpdateOnRemove( void )
{
	m_BoneFollowerManager.DestroyBoneFollowers();
	BaseClass::UpdateOnRemove();
}

void CNPC_Dog::GatherConditions( void )
{
	if ( IsInAScript() )
	{
		ClearSenseConditions();
		return;
	}

	BaseClass::GatherConditions();
}

int	CNPC_Dog::OnTakeDamage_Alive( const CTakeDamageInfo &info )
{
	if ( IsInAScript() )
		 return 0;

	return BaseClass::OnTakeDamage_Alive( info );
}

//-----------------------------------------------------------------------------
// This function checks if Dog's collision group doesn't match his bone follower's and fixes them up.
//-----------------------------------------------------------------------------
void CNPC_Dog::MantainBoneFollowerCollisionGroups( int iCollisionGroup )
{
	if ( m_bBoneFollowersActive == false )
		return;

	physfollower_t* pBone = m_BoneFollowerManager.GetBoneFollower( 0 );

	if ( pBone && pBone->hFollower && pBone->hFollower->GetCollisionGroup() != iCollisionGroup )
	{
		for ( int i = 0; i < m_BoneFollowerManager.GetNumBoneFollowers(); i++ )
		{
			pBone = m_BoneFollowerManager.GetBoneFollower( i );

			if ( pBone && pBone->hFollower )
			{
				pBone->hFollower->SetCollisionGroup( iCollisionGroup );
			}
		}
	}
}

void CNPC_Dog::SetPlayerAvoidState( void )
{
	bool bIntersectingBoneFollowers = false;
	bool bIntersectingNPCBox = false;

	Vector vNothing;

	GetSequenceLinearMotion( GetSequence(), &vNothing );
	bool bIsMoving = ( IsMoving() || ( vNothing != vec3_origin ) );

	//If we are coming out of a script, check if we are stuck inside the player.
	if ( m_bPerformAvoidance || ( ShouldPlayerAvoid() && bIsMoving ) )
	{
		trace_t trace;
		Vector vMins, vMaxs;
		Vector vWorldMins, vWorldMaxs;
		Vector vPlayerMins, vPlayerMaxs;
		physfollower_t *pBone;
		int i;

		CBasePlayer *pLocalPlayer = AI_GetSinglePlayer();

		if ( pLocalPlayer )
		{
			vWorldMins = WorldAlignMins();
			vWorldMaxs = WorldAlignMaxs();

			vPlayerMins = pLocalPlayer->GetAbsOrigin() + pLocalPlayer->WorldAlignMins();
			vPlayerMaxs = pLocalPlayer->GetAbsOrigin() + pLocalPlayer->WorldAlignMaxs();

			// check if the player intersects the bounds of any of the bone followers
			for ( i = 0; i < m_BoneFollowerManager.GetNumBoneFollowers(); i++ )
			{
				pBone = m_BoneFollowerManager.GetBoneFollower( i );
				if ( pBone && pBone->hFollower )
				{
					pBone->hFollower->CollisionProp()->WorldSpaceSurroundingBounds( &vMins, &vMaxs );
					if ( IsBoxIntersectingBox( vMins, vMaxs, vPlayerMins, vPlayerMaxs ) )
					{
						bIntersectingBoneFollowers = true;
						break;
					}
				}
			}

			bIntersectingNPCBox = IsBoxIntersectingBox( GetAbsOrigin() + vWorldMins, GetAbsOrigin() + vWorldMaxs, vPlayerMins, vPlayerMaxs );

			if ( ai_debug_avoidancebounds.GetBool() )
			{
				int iRed = ( bIntersectingNPCBox == true ) ? 255 : 0;

				NDebugOverlay::Box( GetAbsOrigin(), vWorldMins, vWorldMaxs, iRed, 0, 255, 64, 0.1 );

				// draw the bounds of the bone followers
				for ( i = 0; i < m_BoneFollowerManager.GetNumBoneFollowers(); i++ )
				{
					pBone = m_BoneFollowerManager.GetBoneFollower( i );
					if ( pBone && pBone->hFollower )
					{
						pBone->hFollower->CollisionProp()->WorldSpaceSurroundingBounds( &vMins, &vMaxs );
						iRed = ( IsBoxIntersectingBox( vMins, vMaxs, vPlayerMins, vPlayerMaxs ) ) ? 255 : 0;

						NDebugOverlay::Box( vec3_origin, vMins, vMaxs, iRed, 0, 255, 64, 0.1 );
					}
				}
			}
		}
	}

	m_bPlayerAvoidState = ShouldPlayerAvoid();
	m_bPerformAvoidance = bIntersectingNPCBox || bIntersectingBoneFollowers;

	if ( GetCollisionGroup() == COLLISION_GROUP_NPC || GetCollisionGroup() == COLLISION_GROUP_NPC_ACTOR )
	{
		if ( bIntersectingNPCBox == true )
		{
			SetCollisionGroup( COLLISION_GROUP_NPC_ACTOR );
		}
		else
		{
			SetCollisionGroup( COLLISION_GROUP_NPC );
		}

		if ( bIntersectingBoneFollowers == true )
		{
			MantainBoneFollowerCollisionGroups( COLLISION_GROUP_NPC_ACTOR );
		}
		else
		{
			MantainBoneFollowerCollisionGroups( COLLISION_GROUP_NPC );
		}
	}
}
//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Dog::NPCThink( void )
{
	BaseClass::NPCThink();

	if ( m_hPhysicsEnt == NULL )
	{
#ifdef DARKINTERVAL
		if (!m_force_physgun_fx_bool) // to be able to have beams even without valid targets
#endif
		{
			ClearBeams();
		}
		m_bHasObject = false;
	}
	
	if ( m_bHasObject == true )
	{
		 RelaxAim();
		 PullObject( true );
	}
	
	
	// update follower bones
	m_BoneFollowerManager.UpdateBoneFollowers(this);
}

//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Dog::Event_Killed( const CTakeDamageInfo &info )
{
	m_BoneFollowerManager.DestroyBoneFollowers();
	BaseClass::Event_Killed( info );
}

void CNPC_Dog::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();

	if ( m_hPhysicsEnt )
	{
		IPhysicsObject *pPhysObj = m_hPhysicsEnt->VPhysicsGetObject();

		if ( pPhysObj && pPhysObj->GetGameFlags() & FVPHYSICS_PLAYER_HELD )
		{
			m_hPhysicsEnt->SetOwnerEntity( NULL );
		}
	}

	if ( m_flTimeToCatch < CURTIME ) 
		 m_flTimeToCatch = 0.0f;

	
	if ( GetIdealActivity() == ACT_IDLE )
	{
		if ( m_hPhysicsEnt && m_bHasObject == true )
		{
			 SetIdealActivity( (Activity)ACT_DOG_WAITING );
		}
	}
}

int CNPC_Dog::SelectSchedule ( void )
{
	ClearCondition( COND_DOG_LOST_PHYSICS_ENTITY );

	if ( GetState() == NPC_STATE_SCRIPT || IsInAScript() )
		 return BaseClass::SelectSchedule();

	if ( BehaviorSelectSchedule() )
		return BaseClass::SelectSchedule();

	if ( m_bDoWaitforObjectBehavior == true )
	{
		if ( m_hPhysicsEnt )
			 return SCHED_DOG_CATCH_OBJECT;
	}
	
	if ( m_bDoCatchThrowBehavior == true )
	{
		if ( m_flTimeToCatch < 0.1 && m_flNextSwat <= CURTIME )
		{
			 return SCHED_DOG_FIND_OBJECT;
		}

		if ( m_flTimeToCatch > CURTIME && m_hPhysicsEnt )
			 return SCHED_DOG_CATCH_OBJECT;
	}
	else
	{
		if ( m_hPhysicsEnt )
		{
			if ( m_bHasObject == true )
			{
				return SCHED_DOG_WAIT_THROW_OBJECT;
			}
		}
	}

	return BaseClass::SelectSchedule();
}

void CNPC_Dog::PullObject( bool bMantain )
{
	if ( m_hPhysicsEnt == NULL )
	{
		TaskFail( "Ack! No Phys Object!");
		return;
	}

	IPhysicsObject *pPhysObj = m_hPhysicsEnt->VPhysicsGetObject();

	if ( pPhysObj == NULL )
	{
		TaskFail( "Pulling object with no Phys Object?!" );
		return;
	}

	if( pPhysObj->GetGameFlags() & FVPHYSICS_PLAYER_HELD )
	{
		m_bHasObject = false;
		ClearBeams();
		TaskFail("Player Grabbed Ball");
		return;
	}

	CreateBeams();

	Vector vGunPos;
	GetAttachment( m_iPhysGunAttachment, vGunPos );
	float flDistance = ( vGunPos - m_hPhysicsEnt->WorldSpaceCenter() ).Length();

	if ( bMantain == false )
	{
		if ( flDistance <= DOG_CATCH_DISTANCE )
		{
			m_hPhysicsEnt->SetOwnerEntity( this );

			GetNavigator()->StopMoving();

			//Fire Output!
			m_OnPickup.FireOutput( this, this );

			m_bHasObject = true;
			ClearBeams();
			TaskComplete();
			return;
		}
	}

	Vector vDir = ( vGunPos -  m_hPhysicsEnt->WorldSpaceCenter() );

	Vector vCurrentVel;
	float flCurrentVel;
	AngularImpulse vCurrentAI;

	pPhysObj->GetVelocity( &vCurrentVel, &vCurrentAI );
	flCurrentVel = vCurrentVel.Length();

	VectorNormalize( vCurrentVel );
	VectorNormalize( vDir );

	float flVelMod = DOG_PULL_VELOCITY_MOD;

	if ( bMantain == true )
		 flVelMod *= 2;

	vCurrentVel = vCurrentVel * flCurrentVel * flVelMod;

	vCurrentAI = vCurrentAI * DOG_PULL_ANGULARIMP_MOD;
	pPhysObj->SetVelocity( &vCurrentVel, &vCurrentAI );

	vDir = vDir * flDistance * (DOG_PULL_TO_GUN_VEL_MOD * 2);

	Vector vAngle( 0, 0, 0 );
	pPhysObj->AddVelocity( &vDir, &vAngle );
}

void CNPC_Dog::CleanCatchAndThrow( bool bClearTimers )
{
	if ( m_hPhysicsEnt )
	{
		if ( m_bHasObject == true )
		{
			IPhysicsObject *pPhysObj = m_hPhysicsEnt->VPhysicsGetObject();

			m_hPhysicsEnt->SetParent( NULL );
			m_hPhysicsEnt->SetOwnerEntity( NULL );

			Vector vGunPos;
			QAngle angGunAngles;
			GetAttachment( m_iPhysGunAttachment, vGunPos, angGunAngles );

			if ( pPhysObj )
			{
				pPhysObj->Wake();
				pPhysObj->RemoveShadowController();
				pPhysObj->SetPosition( vGunPos, angGunAngles, true );
			}
			else
			{
				Warning( "CleanCatchAndThrow:   m_hPhysicsEnt->VPhysicsGetObject == NULL!\n" );
			}

			m_hPhysicsEnt->SetMoveType( (MoveType_t)m_iContainerMoveType );

			if ( pPhysObj )
			{
				pPhysObj->RecheckCollisionFilter();
			}
	
			ClearBeams();
		}
		
		m_hPhysicsEnt = NULL;
	}

	if ( bClearTimers == true )
	{
		 m_bDoCatchThrowBehavior = false;
		 m_bDoWaitforObjectBehavior = false;
		 m_flTimeToCatch = 0.0f;
		 m_flNextSwat = 0.0f;

		 SetCondition( COND_DOG_LOST_PHYSICS_ENTITY );
	}
}

void CNPC_Dog::InputPlayerPickupObject ( inputdata_t &inputdata )
{
	if ( m_bDoWaitforObjectBehavior == true )
	{
		if ( m_hPhysicsEnt != inputdata.pCaller )
		{
			if ( m_hPhysicsEnt != NULL )
				 CleanCatchAndThrow( false );

			//Reset this cause CleanCatchAndThrow clears it.
			m_bDoWaitforObjectBehavior = true;
			m_hPhysicsEnt = inputdata.pCaller;
		}
	}
	else if ( m_bDoCatchThrowBehavior == true )
	{
		if ( m_sObjectName != NULL_STRING )
		{
			if ( m_hPhysicsEnt != inputdata.pCaller )
			{
				if ( m_hPhysicsEnt != NULL )
					 CleanCatchAndThrow( false );

				//Reset this cause CleanCatchAndThrow clears it.
				m_bDoCatchThrowBehavior = true;
				m_hPhysicsEnt = inputdata.pCaller;
			}
		}
	}
}

void CNPC_Dog::InputSetThrowArcModifier( inputdata_t &inputdata )
{
	m_flThrowArcModifier = inputdata.value.Float();
}

void CNPC_Dog::InputSetPickupTarget( inputdata_t &inputdata )
{
	CleanCatchAndThrow( false );
	FindPhysicsObject( inputdata.value.String() );
}

void CNPC_Dog::InputStartWaitAndCatch( inputdata_t &inputdata )
{
	CleanCatchAndThrow();
	m_bDoWaitforObjectBehavior = true;
}

void CNPC_Dog::InputStopWaitAndCatch( inputdata_t &inputdata )
{
	CleanCatchAndThrow();
}

void CNPC_Dog::InputStartCatchThrowBehavior( inputdata_t &inputdata )
{
	CleanCatchAndThrow();

	m_sObjectName = MAKE_STRING( inputdata.value.String() );
	m_bDoCatchThrowBehavior = true;

	m_flTimeToCatch = 0.0f;
	m_flNextSwat = 0.0f;

	FindPhysicsObject( inputdata.value.String() );
}

void CNPC_Dog::InputStopCatchThrowBehavior( inputdata_t &inputdata )
{
	m_bDoCatchThrowBehavior = false;

	m_flTimeToCatch = 0.0f;
	m_flNextSwat = 0.0f;
	m_sObjectName = NULL_STRING;

	CleanCatchAndThrow();
}

void CNPC_Dog::InputSetThrowTarget( inputdata_t &inputdata )
{
	m_hThrowTarget = gEntList.FindEntityByName( NULL, inputdata.value.String(), NULL, inputdata.pActivator, inputdata.pCaller );
}

void CNPC_Dog::SetTurnActivity( void )
{
	BaseClass::SetTurnActivity();

	if ( GetIdealActivity() == ACT_IDLE )
	{
		if ( m_hPhysicsEnt && m_bHasObject == true )
			 SetIdealActivity( (Activity)ACT_DOG_WAITING );
	}
}

void CNPC_Dog::ThrowObject( const char *pAttachmentName )
{
	if ( m_hPhysicsEnt )
	{
		m_bHasObject = false;

		IPhysicsObject *pPhysObj = m_hPhysicsEnt->VPhysicsGetObject();

		if ( pPhysObj )
		{
			Vector vGunPos;
			QAngle angGunAngles;

			AngularImpulse angVelocity = RandomAngularImpulse( -250 , -250 ) / pPhysObj->GetMass();

			InvalidateBoneCache();

			int iAttachment = LookupAttachment( pAttachmentName );

			if ( iAttachment == 0 )
				 iAttachment = m_iPhysGunAttachment;
			
			GetAttachment( iAttachment, vGunPos, angGunAngles );

			pPhysObj->Wake();

			if ( pPhysObj->GetShadowController() )
			{
				m_hPhysicsEnt->SetParent( NULL );
				m_hPhysicsEnt->SetMoveType( (MoveType_t)m_iContainerMoveType );
				m_hPhysicsEnt->SetOwnerEntity( this );

				pPhysObj->RemoveShadowController();
				pPhysObj->SetPosition( m_hPhysicsEnt->GetLocalOrigin(), m_hPhysicsEnt->GetLocalAngles(), true );

				pPhysObj->RecheckCollisionFilter();
				pPhysObj->RecheckContactPoints();
			}
				
			if ( m_hThrowTarget == NULL )
				 m_hThrowTarget = AI_GetSinglePlayer();

			Vector vThrowDirection;

			if ( m_hThrowTarget )
			{
				Vector vThrowOrigin = m_hThrowTarget->GetAbsOrigin();
				
				if ( m_hThrowTarget->IsPlayer() )
					 vThrowOrigin = vThrowOrigin + Vector( random->RandomFloat( -128, 128 ), random->RandomFloat( -128, 128 ), 0 );

				Vector vecToss = VecCheckToss( this, vGunPos, vThrowOrigin, m_flThrowArcModifier, 1.0f, true );

				if( vecToss == vec3_origin )
				{
					// Fix up an impossible throw so dog will at least toss the box in the target's general direction instead of dropping it.
					// Also toss it up in the air so it will fall down and break. (Just throw the box up at a 45 degree angle)
					Vector forward, up;
					GetVectors( &forward, NULL, &up );

					vecToss = forward + up;
					VectorNormalize( vecToss );

					vecToss *= pPhysObj->GetMass() * 30.0f;
				}

				vThrowDirection = vecToss + ( m_hThrowTarget->GetSmoothedVelocity() / 2 );
							
				Vector vLinearDrag;

				Vector unitVel = vThrowDirection;
				VectorNormalize( unitVel );

				float flTest = 1000 / vThrowDirection.Length();

				float flDrag = pPhysObj->CalculateLinearDrag( vThrowDirection );
				vThrowDirection = vThrowDirection + ( unitVel * ( flDrag * flDrag ) ) / flTest;
			
				pPhysObj->SetVelocity( &vThrowDirection, &angVelocity );
#ifdef DARKINTERVAL
				m_flTimeToCatch = CURTIME + 7;
#else
				m_flTimeToCatch = CURTIME + dog_max_wait_time.GetFloat();
#endif
				//Don't start pulling until the object is away from me.
				//We base the time on the throw velocity.
				m_flTimeToPull = CURTIME + ( 1000 / vThrowDirection.Length() );
			}

			//Fire Output!
			m_OnThrow.FireOutput( this, this );

			ClearBeams();
			
			if ( m_bBeamEffects == true )
			{
				EmitSound( "Weapon_PhysCannon.Launch" );
				
				CBeam *pBeam = CBeam::BeamCreate(  "sprites/orangelight1.vmt", 1.8 );

				if ( pBeam != NULL )
				{
					pBeam->PointEntInit( m_hPhysicsEnt->WorldSpaceCenter(), this );
					pBeam->SetEndAttachment( m_iPhysGunAttachment );
					pBeam->SetWidth( 6.4 );
					pBeam->SetEndWidth( 12.8 );					
					pBeam->SetBrightness( 255 );
					pBeam->SetColor( 255, 255, 255 );
					pBeam->LiveForTime( 0.2f );
					pBeam->RelinkBeam();
					pBeam->SetNoise( 2 );
				}
			
				Vector	shotDir = ( m_hPhysicsEnt->WorldSpaceCenter() - vGunPos );
				VectorNormalize( shotDir );

				CPVSFilter filter( m_hPhysicsEnt->WorldSpaceCenter() );
				te->GaussExplosion( filter, 0.0f, m_hPhysicsEnt->WorldSpaceCenter() - ( shotDir * 4.0f ), RandomVector(-1.0f, 1.0f), 0 );
			}
		}
	}
}

void CNPC_Dog::PickupOrCatchObject( const char *pAttachmentName )
{
	if ( m_hPhysicsEnt )
	{
		InvalidateBoneCache();

		int iAttachment = LookupAttachment( pAttachmentName );

		if ( iAttachment == 0 )
			 iAttachment = m_iPhysGunAttachment;
		
		// Move physobject to shadow
		IPhysicsObject *pPhysicsObject = m_hPhysicsEnt->VPhysicsGetObject();
		if ( pPhysicsObject )
		{
			pPhysicsObject->SetShadow( 1e4, 1e4, false, false );
			pPhysicsObject->UpdateShadow( GetAbsOrigin(), GetAbsAngles(), false, 0 );
		}
		
		m_iContainerMoveType = m_hPhysicsEnt->GetMoveType();
		m_hPhysicsEnt->SetMoveType( MOVETYPE_NONE );
		
		m_hPhysicsEnt->SetParent( this, iAttachment );
	
		m_hPhysicsEnt->SetLocalOrigin( vec3_origin );
		m_hPhysicsEnt->SetLocalAngles( vec3_angle );

		m_hPhysicsEnt->SetGroundEntity( NULL );
		

		if ( m_hPhysicsEnt->GetOwnerEntity() == NULL )
			 m_hPhysicsEnt->SetOwnerEntity( this );

		if ( pPhysicsObject )
			 pPhysicsObject->RecheckCollisionFilter();

		m_bHasObject = true;

		//Fire Output!
		m_OnPickup.FireOutput( this, this );
	}
}

//-----------------------------------------------------------------------------
// HandleAnimEvent - catches the NPC-specific messages
// that occur when tagged animation frames are played.
//-----------------------------------------------------------------------------
void CNPC_Dog::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_DOG_THROW )
	{
		ThrowObject( pEvent->options );
		return;
	}

	if ( pEvent->event == AE_DOG_PICKUP || pEvent->event == AE_DOG_CATCH || pEvent->event == AE_DOG_PICKUP_NOEFFECT )
	{
		if ( pEvent->event == AE_DOG_PICKUP_NOEFFECT )
			 m_bBeamEffects = false;
		else
			 m_bBeamEffects = true;

		PickupOrCatchObject( pEvent->options );
		return;
	}

	BaseClass::HandleAnimEvent( pEvent );
}

void CNPC_Dog::ClearBeams( void )
{
	ClearSprites();
	
	// Turn off sprites
	for ( int i = 0; i < EFFECT_COUNT; i++ )
	{
		if ( m_hBeams[i] != NULL )
		{
			UTIL_Remove( m_hBeams[i] );
			m_hBeams[i] = NULL;
		}
	}
}

void CNPC_Dog::ClearSprites( void )
{
	// Turn off sprites
	for ( int i = 0; i < EFFECT_COUNT; i++ )
	{
		if ( m_hGlowSprites[i] != NULL )
		{
			UTIL_Remove( m_hGlowSprites[i] );
			m_hGlowSprites[i] = NULL;
		}
	}
}

void CNPC_Dog::CreateSprites( void )
{
	//Create the glow sprites
	for ( int i = 0; i < EFFECT_COUNT; i++ )
	{
		if ( m_hGlowSprites[i] )
			continue;

		const char *attachNames[] = 
		{
			"physgun",
			"thumb",
			"pinky",
			"index",
		};

		m_hGlowSprites[i] = CSprite::SpriteCreate( "sprites/glow04_noz.vmt", GetAbsOrigin(), false );

		m_hGlowSprites[i]->SetAttachment( this, LookupAttachment( attachNames[i] ) );
		m_hGlowSprites[i]->SetTransparency( kRenderGlow, 255, 128, 0, 64, kRenderFxNoDissipation );
		m_hGlowSprites[i]->SetBrightness( 255, 0.2f );
		m_hGlowSprites[i]->SetScale( 0.55f, 0.2f );
	}
}
#ifdef DARKINTERVAL
void CNPC_Dog::CreateBeams( bool bFake )
#else
void CNPC_Dog::CreateBeams( void )
#endif
{
	if ( m_bBeamEffects == false || (!m_hPhysicsEnt && !bFake) )
	{
		ClearBeams();
		return;
	}

	CreateSprites();

	for ( int i = 0; i < EFFECT_COUNT; i++ )
	{
		if ( m_hBeams[i] )
			continue;

		const char *attachNames[] = 
		{
			"physgun",
			"thumb",
			"pinky",
			"index",
		};

		m_hBeams[i] = CBeam::BeamCreate( "sprites/physcannon_bluelight2.vmt", 5.0 );
#ifdef DARKINTERVAL
		if (bFake)
		{
			m_hBeams[i]->EntsInit( this, this);
			m_hBeams[i]->SetStartAttachment(LookupAttachment(attachNames[0]));
		}
		else
#endif
		{
			m_hBeams[i]->EntsInit(m_hPhysicsEnt, this);
		}
		m_hBeams[i]->SetEndAttachment( LookupAttachment( attachNames[i] ) );
		m_hBeams[i]->SetBrightness( 255 );
		m_hBeams[i]->SetColor( 255, 255, 255 );
		m_hBeams[i]->SetNoise( 5.5 );
		m_hBeams[i]->SetRenderMode( kRenderTransAdd );
	}

}

bool CNPC_Dog::FindPhysicsObject( const char *pPickupName, CBaseEntity *pIgnore )
{
	CBaseEntity		*pEnt = NULL;
	CBaseEntity		*pNearest = NULL;
	float			flDist;
	IPhysicsObject	*pPhysObj = NULL;
	float			flNearestDist = 99999;

	if ( pPickupName != NULL && strlen( pPickupName ) > 0 )
	{
		pEnt = gEntList.FindEntityByName( NULL, pPickupName );
		
		if ( m_hUnreachableObjects.Find( pEnt ) == -1  )
		{
			m_bHasObject = false;
			m_hPhysicsEnt = pEnt;
			return true;
		}
	}
	
	while ( ( pEnt = gEntList.FindEntityByClassname( pEnt, "prop_physics" ) ) != NULL )
	{
		//We don't want this one.
		if ( pEnt == pIgnore )
			 continue;

		if ( m_hUnreachableObjects.Find( pEnt ) != -1 )
			 continue;
#ifdef DARKINTERVAL
		if (m_dog_pickup_filter_handle.Get() && !(m_dog_pickup_filter_handle.Get()->PassesFilter(this, pEnt)))
			continue;
#endif
		pPhysObj = pEnt->VPhysicsGetObject();

		if( pPhysObj == NULL )
			continue;

		if ( pPhysObj->GetMass() > DOG_MAX_THROW_MASS )
			 continue;
		
		Vector center = pEnt->WorldSpaceCenter();
		flDist = UTIL_DistApprox2D( GetAbsOrigin(), center );

		vcollide_t *pCollide = modelinfo->GetVCollide( pEnt->GetModelIndex() );

		if ( pCollide == NULL )
			 continue;

		if ( pPhysObj->GetGameFlags() & FVPHYSICS_PLAYER_HELD )
			 continue;

		if ( pPhysObj->IsMoveable() == false )
			 continue;

		if ( pEnt->GetCollisionGroup() == COLLISION_GROUP_DEBRIS || 
			 pEnt->GetCollisionGroup() == COLLISION_GROUP_INTERACTIVE_DEBRIS )
			 continue;

		if ( center.z > EyePosition().z )
			 continue;

		if ( flDist >= flNearestDist )
			 continue;

		if ( FVisible( pEnt ) == false )
			 continue;
		
		pNearest = pEnt;
		flNearestDist = flDist;
	}

	m_bHasObject = false;
	m_hPhysicsEnt = pNearest;
#ifndef DARKINTERVAL
	if ( dog_debug.GetBool() == true )
	{
		if ( pNearest )
			 NDebugOverlay::Box( pNearest->WorldSpaceCenter(), pNearest->CollisionProp()->OBBMins(), pNearest->CollisionProp()->OBBMaxs(), 255, 0, 255, true, 3 );
	}
#endif
	if( m_hPhysicsEnt == NULL )
	{
		return false;
	}
	else
	{
		return true;
	}
}

//-----------------------------------------------------------------------------
// Can me enemy see me? 
//-----------------------------------------------------------------------------
bool CNPC_Dog::CanTargetSeeMe( void )
{
	CBaseEntity *pEntity = m_hThrowTarget;

	if ( pEntity )
	{
		if ( pEntity->IsPlayer() == false )
			return true;

		CBasePlayer *pPlayer = dynamic_cast<CBasePlayer*>( pEntity );

		if ( pPlayer )
		{
			if ( m_hPhysicsEnt )
			{
				if ( pPlayer->FVisible( m_hPhysicsEnt ) == false )
					return false;
			}
			
			if ( pPlayer->FInViewCone( this ) )
			{
				return true;
			}
		}
	}

	return false;
}


//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Dog::RunTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{

	case TASK_DOG_PICKUP_ITEM:
	{
		 PullObject( false );
	}
	break;

	case TASK_DOG_GET_PATH_TO_PHYSOBJ:
		{
			//Check this cause our object might have been deleted.
			if ( m_hPhysicsEnt == NULL )
				 FindPhysicsObject( NULL );

			//And if we still can't find anything, then just go away.
			if ( m_hPhysicsEnt == NULL )
			{
				TaskFail( "Can't find an object I like!" );
				return;
			}
	
			IPhysicsObject *pPhysicsObject = m_hPhysicsEnt->VPhysicsGetObject();
			
			Vector vecGoalPos;
			Vector vecDir;

			vecDir = GetLocalOrigin() - m_hPhysicsEnt->WorldSpaceCenter();
			VectorNormalize(vecDir);
			vecDir.z = 0;
		
			if ( m_hPhysicsEnt->GetOwnerEntity() == NULL )
				 m_hPhysicsEnt->SetOwnerEntity( this );
		
			if ( pPhysicsObject )
				 pPhysicsObject->RecheckCollisionFilter();

			vecGoalPos = m_hPhysicsEnt->WorldSpaceCenter() + (vecDir * DOG_PHYSOBJ_MOVE_TO_DIST );

			bool bBuiltRoute = false;

			//If I'm near my goal, then just walk to it.
			Activity aActivity = ACT_RUN;

			if ( ( vecGoalPos - GetLocalOrigin() ).Length() <= 128 )
				 aActivity = ACT_WALK;

			bBuiltRoute = GetNavigator()->SetGoal( AI_NavGoal_t( vecGoalPos, aActivity ), AIN_NO_PATH_TASK_FAIL );

			if ( bBuiltRoute == true )
				 TaskComplete();
			else
			{
				m_flTimeToCatch = CURTIME + 0.1;
				m_flNextRouteTime = CURTIME + 0.3;
				m_flNextSwat = CURTIME + 0.1;

				if ( m_hUnreachableObjects.Find( m_hPhysicsEnt ) == -1 )
					 m_hUnreachableObjects.AddToTail( m_hPhysicsEnt );
								
				m_hPhysicsEnt = NULL;

				GetNavigator()->ClearGoal();
			}
		}
		break;

	case TASK_WAIT:
	{
		if ( IsWaitFinished() )
		{
			TaskComplete();
		}

		if ( m_hPhysicsEnt )
		{
			if ( m_bHasObject == false )
			{
				GetMotor()->SetIdealYawToTarget( m_hPhysicsEnt->GetAbsOrigin() );
				GetMotor()->UpdateYaw();
			}
		}

		break;
	}

	case TASK_DOG_LAUNCH_ITEM:
		if( IsActivityFinished() )
		{
			if ( m_hPhysicsEnt )
			{
				m_hPhysicsEnt->SetOwnerEntity( NULL );
			}

			TaskComplete();
		}
		break;

	case TASK_DOG_WAIT_FOR_TARGET_TO_FACE:
	{
		if ( CanTargetSeeMe() )
			 TaskComplete();
	}
		break;

	case TASK_WAIT_FOR_MOVEMENT:
		{
			if ( GetState() == NPC_STATE_SCRIPT || IsInAScript() )
			{
			  	 BaseClass::RunTask( pTask );
				 return;
			}

			if ( m_hPhysicsEnt != NULL )
			{
				IPhysicsObject *pPhysObj = m_hPhysicsEnt->VPhysicsGetObject();
					
				if ( !pPhysObj )
				{
					Warning( "npc_dog TASK_WAIT_FOR_MOVEMENT with NULL m_hPhysicsEnt->VPhysicsGetObject\n" );
				}

				if ( pPhysObj && pPhysObj->GetGameFlags() & FVPHYSICS_PLAYER_HELD )
					 TaskFail( "Player picked it up!" );

				//If the object is moving then my old goal might not be valid
				//cancel the schedule and make it restart again in a bit.
				if ( pPhysObj && pPhysObj->IsAsleep() == false && GetNavigator()->IsGoalActive() == false )
				{
					Vector vecGoalPos;
					Vector vecDir;
				
					vecDir = GetLocalOrigin() - m_hPhysicsEnt->WorldSpaceCenter();
					VectorNormalize(vecDir);
					vecDir.z = 0;
									
					vecGoalPos = m_hPhysicsEnt->WorldSpaceCenter() + (vecDir * DOG_PHYSOBJ_MOVE_TO_DIST );

					GetNavigator()->ClearGoal();

					float flDistance = (vecGoalPos - GetLocalOrigin()).Length();

					//If I'm near my goal, then just walk to it.
					Activity aActivity = ACT_RUN;

					if ( ( vecGoalPos - GetLocalOrigin() ).Length() <= 128 )
						 aActivity = ACT_WALK;

				    GetNavigator()->SetGoal( AI_NavGoal_t( vecGoalPos, aActivity ), AIN_NO_PATH_TASK_FAIL );

					if ( flDistance <= DOG_PHYSOBJ_MOVE_TO_DIST )
					{
						TaskComplete();
						GetNavigator()->StopMoving();
					}
				}
			}
			
			BaseClass::RunTask( pTask );
		}
		break;

	case TASK_DOG_WAIT_FOR_OBJECT:
		{
			if ( m_hPhysicsEnt != NULL )
			{
				if ( FVisible( m_hPhysicsEnt ) == false )
				{
					m_flTimeToCatch = 0.0f;
					ClearBeams();
					TaskFail( "Lost sight of the object!" );
					m_hPhysicsEnt->SetOwnerEntity( NULL );
					return;
				}

				m_hPhysicsEnt->SetOwnerEntity( this );

				Vector vForward;
				AngleVectors( GetAbsAngles(), &vForward );


				Vector vGunPos;
				GetAttachment( m_iPhysGunAttachment, vGunPos );

				Vector vToObject = m_hPhysicsEnt->WorldSpaceCenter() - vGunPos;
				float flDistance = vToObject.Length();

				VectorNormalize( vToObject );

				SetAim( m_hPhysicsEnt->WorldSpaceCenter() - GetAbsOrigin() );

				CBasePlayer *pPlayer = AI_GetSinglePlayer();

				float flDistanceToPlayer = flDistance;

				if ( pPlayer )
				{
					flDistanceToPlayer = (pPlayer->GetAbsOrigin() - m_hPhysicsEnt->WorldSpaceCenter()).Length();
				}
			
				IPhysicsObject *pPhysObj = m_hPhysicsEnt->VPhysicsGetObject();
				if ( !pPhysObj )
				{
					Warning( "npc_dog:  TASK_DOG_WAIT_FOR_OBJECT with m_hPhysicsEnt->VPhysicsGetObject == NULL\n" );
				}
					
				if ( pPhysObj && !( pPhysObj->GetGameFlags() & FVPHYSICS_PLAYER_HELD ) && flDistanceToPlayer > ( flDistance * 2 ) )
				{
					if ( m_flTimeToPull <= CURTIME )
					{
						Vector vCurrentVel;
						float flCurrentVel;
						AngularImpulse vCurrentAI;

						pPhysObj->GetVelocity( &vCurrentVel, &vCurrentAI );

						flCurrentVel = vCurrentVel.Length();
						VectorNormalize( vCurrentVel );

						if ( pPhysObj && flDistance <= DOG_PULL_DISTANCE )
						{
							Vector vDir = ( vGunPos -  m_hPhysicsEnt->WorldSpaceCenter() );
								
							VectorNormalize( vDir );

							vCurrentVel = vCurrentVel * ( flCurrentVel * DOG_PULL_VELOCITY_MOD );

							vCurrentAI = vCurrentAI * DOG_PULL_ANGULARIMP_MOD;
							pPhysObj->SetVelocity( &vCurrentVel, &vCurrentAI );

							vDir = vDir * flDistance * DOG_PULL_TO_GUN_VEL_MOD;

							Vector vAngle( 0, 0, 0 );
							pPhysObj->AddVelocity( &vDir, &vAngle );
							
							CreateBeams();
						}
					
						float flDot = DotProduct( vCurrentVel, vForward );

						if ( flDistance >= DOG_PULL_DISTANCE && flDistance <= ( DOG_PULL_DISTANCE * 2 ) && flDot > -0.3 )
						{
							if ( pPhysObj->IsAsleep() == false && !( pPhysObj->GetGameFlags() & FVPHYSICS_PLAYER_HELD ) )
							{
								Vector vecGoalPos;
								Vector vecDir;

								vecDir = GetLocalOrigin() - m_hPhysicsEnt->WorldSpaceCenter();
								VectorNormalize(vecDir);
								vecDir.z = 0;
												
								vecGoalPos = m_hPhysicsEnt->WorldSpaceCenter() + (vecDir * DOG_PHYSOBJ_MOVE_TO_DIST );

								GetNavigator()->ClearGoal();

								//If I'm near my goal, then just walk to it.
								Activity aActivity = ACT_RUN;

								if ( ( vecGoalPos - GetLocalOrigin() ).Length() <= 128 )
									 aActivity = ACT_WALK;
									 
								GetNavigator()->SetGoal( AI_NavGoal_t( vecGoalPos, aActivity ),  AIN_NO_PATH_TASK_FAIL );
							}
						}
					}
				}


				float flDirDot = DotProduct( vToObject, vForward );

				if ( flDirDot < 0.2 )
				{
					GetMotor()->SetIdealYawToTarget( m_hPhysicsEnt->GetAbsOrigin() );
					GetMotor()->UpdateYaw();
				}

				if ( m_flTimeToCatch < CURTIME && m_bDoWaitforObjectBehavior == false ) 
				{
					m_hPhysicsEnt->SetOwnerEntity( NULL );
					m_flTimeToCatch = 0.0f;
					ClearBeams();
					TaskFail( "Done waiting!" );
				}
				else if ( pPhysObj && ( flDistance <= DOG_CATCH_DISTANCE && !( pPhysObj->GetGameFlags() & FVPHYSICS_PLAYER_HELD ) ) )
				{
					AngularImpulse vZero( 0, 0, 0 );
					pPhysObj->SetVelocity( &vec3_origin, &vZero );

					GetNavigator()->StopMoving();

					//Fire Output!
					m_OnCatch.FireOutput( this, this );
					m_bHasObject = true;
					ClearBeams();
					TaskComplete();
				}
			}
			else
			{
				GetNavigator()->StopMoving();

				ClearBeams();
				TaskFail("No Physics Object!");
			}
			
		}
		break;

	case TASK_DOG_CATCH_OBJECT:
		if( IsActivityFinished() )
		{
			m_flTimeToCatch = 0.0f;
			TaskComplete();
		}
		break;
	default:
		BaseClass::RunTask( pTask );
		break;
	}
}

void CNPC_Dog::SetupThrowTarget( void )
{
	if ( m_hThrowTarget == NULL )
	{
		m_hThrowTarget = AI_GetSinglePlayer();
	}

	SetTarget( m_hThrowTarget );
}

//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Dog::StartTask( const Task_t *pTask )
{
	switch( pTask->iTask )
	{

	case TASK_DOG_SETUP_THROW_TARGET:
		{
			SetupThrowTarget();
			TaskComplete();
		}
		break;
	case TASK_DOG_GET_PATH_TO_PHYSOBJ:
		{
			FindPhysicsObject( STRING( m_sObjectName ) );

			if ( m_hPhysicsEnt == NULL )
			{
				 FindPhysicsObject( NULL );
				 return;
			}

			IPhysicsObject *pPhysicsObject = m_hPhysicsEnt->VPhysicsGetObject();
			
			Vector vecGoalPos;
			Vector vecDir;

			vecDir = GetLocalOrigin() - m_hPhysicsEnt->WorldSpaceCenter();
			VectorNormalize(vecDir);
			vecDir.z = 0;
		
			if ( m_hPhysicsEnt->GetOwnerEntity() == NULL )
				 m_hPhysicsEnt->SetOwnerEntity( this );
		
			if ( pPhysicsObject )
				 pPhysicsObject->RecheckCollisionFilter();

			vecGoalPos = m_hPhysicsEnt->WorldSpaceCenter() + (vecDir * DOG_PHYSOBJ_MOVE_TO_DIST );

			//If I'm near my goal, then just walk to it.
			Activity aActivity = ACT_RUN;

			if ( ( vecGoalPos - GetLocalOrigin() ).Length() <= 128 )
				 aActivity = ACT_WALK;

			if ( GetNavigator()->SetGoal( AI_NavGoal_t( vecGoalPos, aActivity ), AIN_NO_PATH_TASK_FAIL ) == false )
			{
				 if ( m_hUnreachableObjects.Find( m_hPhysicsEnt ) == -1 )
					  m_hUnreachableObjects.AddToTail( m_hPhysicsEnt );
					
				 FindPhysicsObject( NULL, m_hPhysicsEnt );

				 m_flTimeToCatch = CURTIME + 0.1;
				 m_flNextRouteTime = CURTIME + 0.3;
				 m_flNextSwat = CURTIME + 0.1;

				 GetNavigator()->ClearGoal();
			}
			else
			{
				TaskComplete();
			}
		}
		break;

	case TASK_DOG_FACE_OBJECT:
		{
			if( m_hPhysicsEnt == NULL )
			{
				// Physics Object is gone! Probably was an explosive 
				// or something else broke it.
				TaskFail("Physics ent NULL");
				return;
			}

			Vector vecDir;

			vecDir = m_hPhysicsEnt->WorldSpaceCenter() - GetLocalOrigin();
			VectorNormalize(vecDir);

			GetMotor()->SetIdealYaw( UTIL_VecToYaw( vecDir ) );
			TaskComplete();
		}
		break;
		
	case TASK_DOG_PICKUP_ITEM:
		{
			if( m_hPhysicsEnt == NULL )
			{
				// Physics Object is gone! Probably was an explosive 
				// or something else broke it.
				TaskFail("Physics ent NULL");
				return;
			}
			else
			{
				SetIdealActivity( (Activity)ACT_DOG_PICKUP );
			}
		}

		break;
		
	case TASK_DOG_LAUNCH_ITEM:
		{
			if( m_hPhysicsEnt == NULL )
			{
				// Physics Object is gone! Probably was an explosive 
				// or something else broke it.
				TaskFail("Physics ent NULL");
				return;
			}
			else
			{
				if ( m_hPhysicsEnt == NULL || m_bHasObject == false )
				{
					 TaskFail( "Don't have the item!" );
					 return;
				}

				SetIdealActivity( (Activity)ACT_DOG_THROW );
			}
		}

		break;

	case TASK_DOG_WAIT_FOR_TARGET_TO_FACE:
		{
			if ( CanTargetSeeMe() )
				 TaskComplete();
		}
		break;

	case TASK_DOG_WAIT_FOR_OBJECT:
		{
			SetIdealActivity( (Activity)ACT_DOG_WAITING );
		}
		break;

	case TASK_DOG_CATCH_OBJECT:
	{
		SetIdealActivity( (Activity)ACT_DOG_CATCH  );
	}
	break;
			
	case TASK_DOG_DELAY_SWAT:
		m_flNextSwat = CURTIME + pTask->flTaskData;

		if ( m_hThrowTarget == NULL )
			m_hThrowTarget = AI_GetSinglePlayer();

		TaskComplete();
		break;

	default:
		BaseClass::StartTask( pTask );
	}
}

void CNPC_Dog::InputTurnBoneFollowersOff( inputdata_t &inputdata )
{
	if ( m_bBoneFollowersActive )
	{
		m_bBoneFollowersActive = false;
		m_BoneFollowerManager.DestroyBoneFollowers();
	}

}

void CNPC_Dog::InputTurnBoneFollowersOn( inputdata_t &inputdata )
{
	if ( !m_bBoneFollowersActive )
	{
		m_bBoneFollowersActive = true;
		m_BoneFollowerManager.InitBoneFollowers( this, ARRAYSIZE(pFollowerBoneNames), pFollowerBoneNames );
	}
}
#ifdef DARKINTERVAL
void CNPC_Dog::InputTurnOnPhysgunFx(inputdata_t &inputdata) // to be able to have beams even without valid targets
{
	m_force_physgun_fx_bool = true;
	CreateBeams(true);
}

void CNPC_Dog::InputTurnOffPhysgunFx(inputdata_t &inputdata) // to be able to have beams even without valid targets
{
	ClearBeams();
	m_force_physgun_fx_bool = false;
}

void CNPC_Dog::InputSetPickupFilter(inputdata_t &inputdata)
{
	// Get a handle to my enemy filter entity if there is one.
	CBaseEntity *pFilter = gEntList.FindEntityByName(NULL, inputdata.value.StringID());
	m_dog_pickup_filter_handle = dynamic_cast<CBaseFilter*>(pFilter);
}

Activity CNPC_Dog::NPC_TranslateActivity(Activity newActivity)
{
	newActivity = BaseClass::NPC_TranslateActivity(newActivity);

	// Make it so Dog doesn't try to run up to Eli in ch04_eli, it looks janky
	if (GetRunningBehavior() == &m_FollowBehavior && m_FollowBehavior.GetFollowTarget())
	{
		if (newActivity == ACT_RUN)
			return ACT_WALK;
	}
	
	return newActivity;
}
#endif
BEGIN_DATADESC(CNPC_Dog)
	DEFINE_EMBEDDED(m_BoneFollowerManager),
	//	m_FollowBehavior
	DEFINE_FIELD(m_flNextSwat, FIELD_TIME),
	DEFINE_FIELD(m_flTimeToCatch, FIELD_TIME),
	DEFINE_FIELD(m_flTimeToPull, FIELD_TIME),
	DEFINE_FIELD(m_hPhysicsEnt, FIELD_EHANDLE),
	DEFINE_FIELD(m_hThrowTarget, FIELD_EHANDLE),
	DEFINE_FIELD(m_iPhysGunAttachment, FIELD_INTEGER),
	DEFINE_FIELD(m_bDoCatchThrowBehavior, FIELD_BOOLEAN),
	DEFINE_FIELD(m_bDoWaitforObjectBehavior, FIELD_BOOLEAN),
	DEFINE_FIELD(m_sObjectName, FIELD_STRING),
	DEFINE_FIELD(m_flThrowArcModifier, FIELD_FLOAT),
	DEFINE_FIELD(m_flNextRouteTime, FIELD_TIME),
	DEFINE_FIELD(m_bHasObject, FIELD_BOOLEAN),
	DEFINE_FIELD(m_iContainerMoveType, FIELD_INTEGER),
	DEFINE_FIELD(m_bBeamEffects, FIELD_BOOLEAN),
	DEFINE_FIELD(m_bBoneFollowersActive, FIELD_BOOLEAN),
	#ifdef DARKINTERVAL
	DEFINE_FIELD(m_force_physgun_fx_bool, FIELD_BOOLEAN), // to be able to have beams even without valid targets
	#endif
	DEFINE_UTLVECTOR(m_hUnreachableObjects, FIELD_EHANDLE),
	#ifdef DARKINTERVAL
	DEFINE_FIELD(m_dog_pickup_filter_handle, FIELD_EHANDLE),
	DEFINE_KEYFIELD(m_dog_pickup_filter_name_string, FIELD_STRING, "pickupfilter"),
	#endif
	DEFINE_AUTO_ARRAY(m_hGlowSprites, FIELD_EHANDLE),
	DEFINE_AUTO_ARRAY(m_hBeams, FIELD_EHANDLE),
	DEFINE_INPUTFUNC(FIELD_STRING, "SetPickupTarget", InputSetPickupTarget),
	DEFINE_INPUTFUNC(FIELD_STRING, "StartCatchThrowBehavior", InputStartCatchThrowBehavior),
	DEFINE_INPUTFUNC(FIELD_STRING, "StopCatchThrowBehavior", InputStopCatchThrowBehavior),
	DEFINE_INPUTFUNC(FIELD_VOID, "PlayerPickupObject", InputPlayerPickupObject),
	DEFINE_INPUTFUNC(FIELD_VOID, "StartWaitAndCatch", InputStartWaitAndCatch),
	DEFINE_INPUTFUNC(FIELD_VOID, "StopWaitAndCatch", InputStopWaitAndCatch),
	DEFINE_INPUTFUNC(FIELD_FLOAT, "SetThrowArcModifier", InputSetThrowArcModifier),
	DEFINE_INPUTFUNC(FIELD_STRING, "SetThrowTarget", InputSetThrowTarget),
	DEFINE_INPUTFUNC(FIELD_VOID, "TurnBoneFollowersOff", InputTurnBoneFollowersOff),
	DEFINE_INPUTFUNC(FIELD_VOID, "TurnBoneFollowersOn", InputTurnBoneFollowersOn),
	#ifdef DARKINTERVAL
	DEFINE_INPUTFUNC(FIELD_VOID, "TurnOnPhysgunFx", InputTurnOnPhysgunFx), // to be able to have beams even without valid targets
	DEFINE_INPUTFUNC(FIELD_VOID, "TurnOffPhysgunFx", InputTurnOffPhysgunFx),
	#endif
	DEFINE_OUTPUT(m_OnThrow, "OnDogThrow"),
	DEFINE_OUTPUT(m_OnCatch, "OnDogCatch"),
	DEFINE_OUTPUT(m_OnPickup, "OnDogPickup"),
END_DATADESC()

AI_BEGIN_CUSTOM_NPC( npc_dog, CNPC_Dog )

	DECLARE_USES_SCHEDULE_PROVIDER( CAI_FollowBehavior )

	DECLARE_ACTIVITY( ACT_DOG_THROW )
	DECLARE_ACTIVITY( ACT_DOG_PICKUP )
	DECLARE_ACTIVITY( ACT_DOG_WAITING )
	DECLARE_ACTIVITY( ACT_DOG_CATCH )
	
	DECLARE_CONDITION( COND_DOG_LOST_PHYSICS_ENTITY )

	DECLARE_TASK( TASK_DOG_DELAY_SWAT )
	DECLARE_TASK( TASK_DOG_GET_PATH_TO_PHYSOBJ )
	DECLARE_TASK( TASK_DOG_LAUNCH_ITEM )
	DECLARE_TASK( TASK_DOG_PICKUP_ITEM )
	DECLARE_TASK( TASK_DOG_FACE_OBJECT )
	DECLARE_TASK( TASK_DOG_WAIT_FOR_OBJECT )
	DECLARE_TASK( TASK_DOG_CATCH_OBJECT )

	DECLARE_TASK( TASK_DOG_WAIT_FOR_TARGET_TO_FACE )
	DECLARE_TASK( TASK_DOG_SETUP_THROW_TARGET )
		
	DECLARE_ANIMEVENT( AE_DOG_THROW )
	DECLARE_ANIMEVENT( AE_DOG_PICKUP )
	DECLARE_ANIMEVENT( AE_DOG_CATCH )
	DECLARE_ANIMEVENT( AE_DOG_PICKUP_NOEFFECT )
	

	DEFINE_SCHEDULE
	(
		SCHED_DOG_FIND_OBJECT,

		"	Tasks"
		"		TASK_DOG_DELAY_SWAT					3"
		"		TASK_DOG_GET_PATH_TO_PHYSOBJ		0"
		"		TASK_RUN_PATH						0"
		"		TASK_WAIT_FOR_MOVEMENT				0"
		"		TASK_DOG_FACE_OBJECT				0"
		"		TASK_FACE_IDEAL						0"
		"		TASK_DOG_PICKUP_ITEM				0"
		"		TASK_DOG_SETUP_THROW_TARGET			0"
		"		TASK_FACE_TARGET					0.5"
		"		TASK_DOG_WAIT_FOR_TARGET_TO_FACE	0"
		"		TASK_DOG_LAUNCH_ITEM				0"
		""
		"	Interrupts"
		"		COND_DOG_LOST_PHYSICS_ENTITY"
	)

	DEFINE_SCHEDULE
	(
		SCHED_DOG_WAIT_THROW_OBJECT,
		"	Tasks"
		"		TASK_DOG_SETUP_THROW_TARGET			0"
		"		TASK_FACE_TARGET					0.5"
		"		TASK_DOG_WAIT_FOR_TARGET_TO_FACE	0"
		"		TASK_DOG_LAUNCH_ITEM				0"
		""
		"	Interrupts"
		"		COND_DOG_LOST_PHYSICS_ENTITY"
	)

	DEFINE_SCHEDULE
	(
		SCHED_DOG_CATCH_OBJECT,

		"	Tasks"
		"		TASK_DOG_WAIT_FOR_OBJECT			0"
		"		TASK_DOG_CATCH_OBJECT				0"
		"		TASK_FACE_PLAYER					0.5"
		"		TASK_DOG_WAIT_FOR_TARGET_TO_FACE	0"
		"		TASK_DOG_LAUNCH_ITEM				0"
		"		TASK_SET_FAIL_SCHEDULE				SCHEDULE:SCHED_IDLE_STAND"
		""
		"	Interrupts"
		"		COND_DOG_LOST_PHYSICS_ENTITY"
	)

AI_END_CUSTOM_NPC()
