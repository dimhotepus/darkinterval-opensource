//========================================================================//
//
// Purpose: Dr. Kleiner, a suave ladies man leading the fight against the evil 
//			combine while constantly having to help his idiot assistant Gordon
//
//=============================================================================//

#include	"cbase.h"
#include	"ai_basenpc.h"
#include	"ai_hull.h"
#include	"npcevent.h"
#ifdef DARKINTERVAL
#include	"npc_playercompanion.h"
#else
#include	"ai_baseactor.h"
#endif
// memdbgon must be the last include file in a .cpp file!!!
#include	"tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
#ifdef DARKINTERVAL
class CNPC_Kleiner : public CNPC_PlayerCompanion
{
public:
	DECLARE_CLASS( CNPC_Kleiner, CNPC_PlayerCompanion);
	DECLARE_DATADESC();
#else
class CNPC_Kleiner : public CAI_BaseActor
{
public:
	DECLARE_CLASS(CNPC_Kleiner, CAI_BaseActor);
#endif

	void	Spawn( void );
	void	Precache( void );
	Class_T Classify ( void );
	void	HandleAnimEvent( animevent_t *pEvent );
	int		GetSoundInterests ( void );
#ifdef DARKINTERVAL
	void	UseFunc(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value); // for responses
	bool	ShouldLookForBetterWeapon() { return false; } // because he now inherits from player companion
	bool	ShouldRegenerateHealth(void) { return false; }

	COutputEvent m_OnPlayerUse; // for responses
#endif
};

LINK_ENTITY_TO_CLASS( npc_kleiner, CNPC_Kleiner );
#ifdef DARKINTERVAL
//LINK_ENTITY_TO_CLASS(monster_scientist, CNPC_Kleiner);
#endif
//-----------------------------------------------------------------------------
// Classify - indicates this NPC's place in the 
// relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_Kleiner::Classify ( void )
{
#ifdef DARKINTERVAL // don't want Kleiner to regenerate
	return	CLASS_PLAYER_ALLY;
#else
	return	CLASS_PLAYER_ALLY_VITAL;
#endif
}

//-----------------------------------------------------------------------------
// GetSoundInterests - generic NPC can't hear.
//-----------------------------------------------------------------------------
int CNPC_Kleiner::GetSoundInterests ( void )
{
	return	NULL;
}

//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CNPC_Kleiner::Spawn()
{
#ifdef DARKINTERVAL
	Precache();
	SetModel("models/_Characters/kleiner.mdl");
#else
	// Allow custom model usage (mostly for monitors)
	char *szModel = ( char * )STRING(GetModelName());
	if ( !szModel || !*szModel )
	{
		szModel = "models/kleiner.mdl";
		SetModelName(AllocPooledString(szModel));
	}

	Precache();
	SetModel(szModel);
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
#ifndef DARKINTERVAL // DI allows friendly fire
	CapabilitiesAdd( bits_CAP_FRIENDLY_DMG_IMMUNE );
	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION );
#else
	AddSpawnFlags(SF_NPC_NO_PLAYER_PUSHAWAY);
#endif
	NPCInit();
#ifdef DARKINTERVAL // for responses
	SetUse(&CNPC_Kleiner::UseFunc);
#endif
}

//-----------------------------------------------------------------------------
// Precache - precaches all resources this NPC needs
//-----------------------------------------------------------------------------
void CNPC_Kleiner::Precache()
{
#ifdef DARKINTERVAL
	PrecacheModel( "models/_Characters/kleiner.mdl" );	
#else
	PrecacheModel(STRING(GetModelName()));
#endif
	BaseClass::Precache();
}	

//-----------------------------------------------------------------------------
// HandleAnimEvent - catches the NPC-specific messages
// that occur when tagged animation frames are played.
//-----------------------------------------------------------------------------
void CNPC_Kleiner::HandleAnimEvent(animevent_t *pEvent)
{
	switch (pEvent->event)
	{
	case 1:
	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}
#ifdef DARKINTERVAL
//-----------------------------------------------------------------------------
// for responses
//-----------------------------------------------------------------------------
void CNPC_Kleiner::UseFunc(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	m_bDontUseSemaphore = true;
	SpeakIfAllowed(TLK_USE);
	m_bDontUseSemaphore = false;

	m_OnPlayerUse.FireOutput(pActivator, pCaller);
}

//-----------------------------------------------------------------------------
// Save/restore
//-----------------------------------------------------------------------------
BEGIN_DATADESC(CNPC_Kleiner)
DEFINE_OUTPUT(m_OnPlayerUse, "OnPlayerUse"), // for responses
DEFINE_USEFUNC(UseFunc),
END_DATADESC()
#endif
//-----------------------------------------------------------------------------
// AI Schedules Specific to this NPC
//-----------------------------------------------------------------------------
