//========================================================================//
//
// Purpose: Dr. Breen, the oft maligned genius, heroically saving humanity from 
//			its own worst enemy, itself.
//=============================================================================//


//-----------------------------------------------------------------------------
// Generic NPC - purely for scripted sequence work.
//-----------------------------------------------------------------------------
#include	"cbase.h"
#include	"ai_baseactor.h"
#include	"ai_basenpc.h"
#include	"ai_hull.h"
#include	"npcevent.h"
#ifdef DARKINTERVAL
#include	"npc_talker.h"
#endif
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Spawnflags
#define SF_BREEN_BACKGROUND_TALK		( 1 << 16 )		// 65536 

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
#ifdef DARKINTERVAL
class CNPC_Breen : public CNPCSimpleTalker
{
public:
	DECLARE_CLASS( CNPC_Breen, CNPCSimpleTalker);
#else
class CNPC_Breen : public CAI_BaseActor
{
public:
	DECLARE_CLASS(CNPC_Breen, CAI_BaseActor);
#endif
	DECLARE_DATADESC();
	void	Spawn( void );
	void	Precache( void );
	Class_T Classify ( void );
	void	HandleAnimEvent( animevent_t *pEvent );
	int		GetSoundInterests ( void );
	bool	UseSemaphore( void );
#ifdef DARKINTERVAL
	virtual void	Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	int 	SelectSchedule();
	void	RunAI(void);
#endif
};

LINK_ENTITY_TO_CLASS( npc_breen, CNPC_Breen );

//-----------------------------------------------------------------------------
// Classify - indicates this NPC's place in the 
// relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_Breen::Classify ( void )
{
#ifdef DARKINTERVAL
	return	CLASS_PLAYER_ALLY;
#else
	return	CLASS_NONE;
#endif
}

//-----------------------------------------------------------------------------
// GetSoundInterests - generic NPC can't hear.
//-----------------------------------------------------------------------------
int CNPC_Breen::GetSoundInterests ( void )
{
#ifdef DARKINTERVAL // А я могу (с) 
	return SOUND_BULLET_IMPACT | SOUND_COMBAT | SOUND_DANGER | SOUND_BUGBAIT | SOUND_GARBAGE | SOUND_CARCASS;
#else
	return	NULL;
#endif
}

//-----------------------------------------------------------------------------
// Precache - precaches all resources this NPC needs
//-----------------------------------------------------------------------------
void CNPC_Breen::Precache()
{
#ifdef DARKINTERVAL
	PrecacheModel("models/_Characters/Breen.mdl");
#else
	PrecacheModel(STRING(GetModelName()));
#endif
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CNPC_Breen::Spawn()
{
#ifdef DARKINTERVAL
	Precache();
	SetModel( "models/_Characters/Breen.mdl" );

#else
	// Breen is allowed to use multiple models, because he has a torso version for monitors.
	// He defaults to his normal model.
	char *szModel = ( char * )STRING(GetModelName());
	if ( !szModel || !*szModel )
	{
		szModel = "models/breen.mdl";
		SetModelName(AllocPooledString(szModel));
	}

	Precache();
#endif
	BaseClass::Spawn();

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );
	SetBloodColor( BLOOD_COLOR_RED );
	m_iHealth			= 8;
	m_flFieldOfView		= 0.5;// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	m_NPCState			= NPC_STATE_NONE;
	
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS | bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD );
#ifndef DARKINTERVAL
	CapabilitiesAdd( bits_CAP_FRIENDLY_DMG_IMMUNE );
	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION );
#else
	CapabilitiesAdd(bits_CAP_LIVE_RAGDOLL);
#endif
	NPCInit();
#ifdef DARKINTERVAL
	SetUse(&CNPC_Breen::Use);
#endif
#ifdef DARKINTERVAL // event broadcasting test
	IGameEvent *event = gameeventmanager->CreateEvent("di_event_test");
	if ( event )
	{
		gameeventmanager->FireEvent(event);
	}
#endif
}

//-----------------------------------------------------------------------------
// HandleAnimEvent - catches the NPC-specific messages
// that occur when tagged animation frames are played.
//-----------------------------------------------------------------------------
void CNPC_Breen::HandleAnimEvent(animevent_t *pEvent)
{
	switch ( pEvent->event )
	{
		case 1:
		default:
			BaseClass::HandleAnimEvent(pEvent);
			break;
	}
}

bool CNPC_Breen::UseSemaphore(void)
{
#ifdef DARKINTERVAL
	return true;
#else
	if ( HasSpawnFlags( SF_BREEN_BACKGROUND_TALK ) )
		return false;
	return BaseClass::UseSemaphore();
#endif
}

#ifdef DARKINTERVAL // Thinks
void CNPC_Breen::RunAI(void)
{
	BaseClass::RunAI();
	SetNextThink(CURTIME + 0.1f);
}

void CNPC_Breen::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	// Don't allow use during a scripted_sentence
	if (m_useTime > CURTIME)
		return;

	if (pCaller == NULL || !pCaller->IsPlayer())
		return;
	
	if (!IsInAScript() && m_NPCState != NPC_STATE_SCRIPT)
	{
		if (false)
		{
			// First, try to speak the +USE concept
			if (IsOkToSpeakInResponseToPlayer())
			{
				if (!Speak(TLK_HELLO))
				{
					// If we haven't said hi, say that first
					if (!SpokeConcept(TLK_HELLO))
					{
						Speak(TLK_HELLO);
					}
					else
					{
						Speak(TLK_IDLE);
					}
				}
				else
				{
					// Don't say hi after you've said your +USE speech
					SetSpokeConcept(TLK_HELLO, NULL);
				}
			}
		}
		else
		{
			if (!m_FollowBehavior.GetFollowTarget() && IsInterruptable())
			{
				LimitFollowers(pCaller, 2);

				if (m_afMemory & bits_MEMORY_PROVOKED)
				{
					Speak("BREEN_DECLINEFOLLOW");
				}
				else
				{
					// If we haven't said hi, say that first
					if (!SpokeConcept(TLK_HELLO))
					{
						Speak(TLK_HELLO);
					}
					else
					{
						Speak(TLK_STARTFOLLOW);
					}
					SetSpokeConcept(TLK_HELLO, NULL);
					StartFollowing(pCaller);
				}
			}
			else
			{
				Speak(TLK_STOPFOLLOW);
				StopFollowing();
			}
		}
	}
}

int CNPC_Breen::SelectSchedule()
{
	if (HasCondition(COND_HEAR_DANGER))
	{
		CSound *pSound;
		pSound = GetBestSound(SOUND_DANGER);

		ASSERT(pSound != NULL);

		if (pSound && (pSound->m_iType & SOUND_DANGER))
		{
			if (!(pSound->SoundContext() & (SOUND_CONTEXT_MORTAR | SOUND_CONTEXT_FROM_SNIPER)) || IsOkToCombatSpeak())
				SpeakIfAllowed(TLK_DANGER);

		//	if (HasCondition(COND_PC_SAFE_FROM_MORTAR))
		//	{
		//		// Just duck and cover if far away from the explosion, or in cover.
		//		return SCHED_COWER;
		//	}
#if 1
			else if (pSound && (pSound->m_iType & SOUND_CONTEXT_FROM_SNIPER))
			{
				return SCHED_COWER;
			}
#endif

			return SCHED_TAKE_COVER_FROM_BEST_SOUND;
		}
	}

//	if (HasCondition(COND_LIGHT_DAMAGE)|| HasCondition(COND_HEAVY_DAMAGE)|| HasCondition(COND_REPEATED_DAMAGE))
//		return SCHED_RUN_RANDOM;
	
	if (HasCondition(COND_HEAR_MOVE_AWAY))
		return SCHED_MOVE_AWAY;
	
	return BaseClass::SelectSchedule();
}
#endif // DARKINTERVAL
//-----------------------------------------------------------------------------
// Save/restore
//-----------------------------------------------------------------------------
#ifdef DARKINTERVAL
BEGIN_DATADESC(CNPC_Breen)
// Function Pointers
DEFINE_USEFUNC(Use),
END_DATADESC()
#endif
//-----------------------------------------------------------------------------
// AI Schedules Specific to this NPC
//-----------------------------------------------------------------------------

