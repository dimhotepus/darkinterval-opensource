//========================================================================//
//
// Purpose: The G-Man, misunderstood servant of the people
//
// $NoKeywords: $
//
//=============================================================================//


//-----------------------------------------------------------------------------
// Generic NPC - purely for scripted sequence work.
//-----------------------------------------------------------------------------
#include "cbase.h"
#include "npcevent.h"
#include "ai_basenpc.h"
#include "ai_hull.h"
#include "ai_behavior_follow.h"
#include "ai_playerally.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef DARKINTERVAL
ConVar npc_gman_transform_threshold("npc_gman_transform_threshold", "0.1");
#endif
//-----------------------------------------------------------------------------
// NPC's Anim Events Go Here
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CNPC_GMan : public CAI_PlayerAlly
{
public:
	DECLARE_CLASS( CNPC_GMan, CAI_PlayerAlly );
	DECLARE_DATADESC();

	void	Spawn( void );
	void	Precache( void );
	Class_T Classify ( void );
	void	HandleAnimEvent( animevent_t *pEvent );
	virtual Disposition_t IRelationType(CBaseEntity *pTarget);
	int		GetSoundInterests ( void );
	bool	CreateBehaviors(void);
	int		SelectSchedule( void );
#ifdef DARKINTERVAL
	void	InputTransform(inputdata_t &inputdata)
	{
		SetThink(&CNPC_GMan::TransformThink);
		SetNextThink(CURTIME);
	}

	void TransformThink()
	{
		SetModelScale(GetModelScale() - 0.05f);
		if (GetModelScale() <= npc_gman_transform_threshold.GetFloat())
		{
			m_OnFinishTransform.FireOutput(this, this);
			SetThink(NULL);
			SetNextThink(CURTIME + 0.1f);
		}
		SetNextThink(CURTIME + 0.1f);
	}
#endif
private:
	CAI_FollowBehavior	m_FollowBehavior;
#ifdef DARKINTERVAL
	COutputEvent m_OnFinishTransform;
#endif
};

LINK_ENTITY_TO_CLASS( npc_gman, CNPC_GMan );

//-----------------------------------------------------------------------------
// Classify - indicates this NPC's place in the 
// relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_GMan::Classify ( void )
{
	return CLASS_PLAYER_ALLY_VITAL;
}



//-----------------------------------------------------------------------------
// HandleAnimEvent - catches the NPC-specific messages
// that occur when tagged animation frames are played.
//-----------------------------------------------------------------------------
void CNPC_GMan::HandleAnimEvent( animevent_t *pEvent )
{
	switch( pEvent->event )
	{
	case 1:
	default:
		BaseClass::HandleAnimEvent( pEvent );
		break;
	}
}

//-----------------------------------------------------------------------------
// GetSoundInterests - generic NPC can't hear.
//-----------------------------------------------------------------------------
int CNPC_GMan::GetSoundInterests ( void )
{
	return NULL;
}

//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CNPC_GMan::Spawn()
{
#ifdef DARKINTERVAL
	char *szModel = (char *)STRING(GetModelName());
	if (!szModel || !*szModel)
	{
		szModel = "models/_Characters/gman.mdl";
		SetModelName(AllocPooledString(szModel));
	}

	Precache();
	BaseClass::Spawn();
	SetModel(szModel);
#else
	Precache();

	BaseClass::Spawn();

	SetModel( "models/gman.mdl" );

#endif
	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );
	SetBloodColor( BLOOD_COLOR_RED );
#ifdef DARKINTERVAL
	m_iHealth			= 100;
#else
	m_iHealth			= 8;
#endif
	m_flFieldOfView		= 0.5;// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	m_NPCState			= NPC_STATE_NONE;
	SetImpactEnergyScale( 0.0f ); // no physics damage on the gman
	
	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS | bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD );
	CapabilitiesAdd( bits_CAP_FRIENDLY_DMG_IMMUNE );
#ifdef DARKINTERVAL
	CapabilitiesAdd(bits_CAP_LIVE_RAGDOLL);
#endif
#ifndef DARKINTERVAL
	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL );
#endif
	NPCInit();
}

//-----------------------------------------------------------------------------
// Precache - precaches all resources this NPC needs
//-----------------------------------------------------------------------------
void CNPC_GMan::Precache()
{
#ifdef DARKINTERVAL
	PrecacheModel(STRING(GetModelName()));
#else
	PrecacheModel( "models/gman.mdl" );
#endif
	BaseClass::Precache();
}	

//-----------------------------------------------------------------------------
// The G-Man isn't scared of anything.
//-----------------------------------------------------------------------------
Disposition_t CNPC_GMan::IRelationType(CBaseEntity *pTarget)
{
	return D_NU;
}

//=========================================================
// Purpose:
//=========================================================
bool CNPC_GMan::CreateBehaviors()
{
	AddBehavior( &m_FollowBehavior );
	
	return BaseClass::CreateBehaviors();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
int CNPC_GMan::SelectSchedule( void )
{
	if ( !BehaviorSelectSchedule() )
	{
	}

	return BaseClass::SelectSchedule();
}

//-----------------------------------------------------------------------------
// Save/restore
//-----------------------------------------------------------------------------
BEGIN_DATADESC(CNPC_GMan)
// (auto saved by AI)
//	DEFINE_FIELD( m_FollowBehavior, FIELD_EMBEDDED ),	(auto saved by AI)
#ifdef DARKINTERVAL
DEFINE_THINKFUNC(TransformThink),
DEFINE_INPUTFUNC(FIELD_VOID, "Transform", InputTransform),
DEFINE_OUTPUT(m_OnFinishTransform, "OnFinishTransform"),
#endif
END_DATADESC()

//-----------------------------------------------------------------------------
// AI Schedules Specific to this NPC
//-----------------------------------------------------------------------------
