//========================================================================//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "ai_default.h"
#include "ai_task.h"
#include "ai_schedule.h"
#include "ai_node.h"
#include "ai_hull.h"
#include "ai_hint.h"
#include "ai_squad.h"
#include "ai_senses.h"
#include "ai_navigator.h"
#include "ai_motor.h"
#include "ai_behavior.h"
#include "ai_baseactor.h"
#include "ai_behavior_lead.h"
#include "ai_behavior_follow.h"
#include "ai_behavior_standoff.h"
#include "ai_behavior_assault.h"
#include "npc_playercompanion.h"
#include "soundent.h"
#include "game.h"
#include "npcevent.h"
#include "activitylist.h"
#include "vstdlib/random.h"
#include "engine/ienginesound.h"
#include "sceneentity.h"
#include "ai_behavior_functank.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#ifdef DARKINTERVAL
#define BARNEY_MODEL "models/_Characters/barney.mdl"
#else
#define BARNEY_MODEL "models/barney.mdl"
ConVar	sk_barney_health("sk_barney_health", "0");
#endif
//=========================================================
// Barney activities
//=========================================================

class CNPC_Barney : public CNPC_PlayerCompanion
{
public:
	DECLARE_CLASS( CNPC_Barney, CNPC_PlayerCompanion );
	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	virtual void Precache()
	{
		// Prevents a warning
		SelectModel( );
		BaseClass::Precache();

		PrecacheScriptSound( "NPC_Barney.FootstepLeft" );
		PrecacheScriptSound( "NPC_Barney.FootstepRight" );
		PrecacheScriptSound( "NPC_Barney.Die" );

		PrecacheInstancedScene( "scenes/Expressions/BarneyIdle.vcd" );
		PrecacheInstancedScene( "scenes/Expressions/BarneyAlert.vcd" );
		PrecacheInstancedScene( "scenes/Expressions/BarneyCombat.vcd" );
	}

	void	Spawn( void );
	void	SelectModel();
	Class_T Classify( void );
	void	Weapon_Equip( CBaseCombatWeapon *pWeapon );
#ifdef DARKINTERVAL
	virtual void InputUnholsterWeapon(inputdata_t &inputdata); // temp hack until better holstering and unholstering is added
#endif
	bool CreateBehaviors( void );

	void HandleAnimEvent( animevent_t *pEvent );
#ifdef DARKINTERVAL
	bool ShouldRegenerateHealth(void) { return false; } // else Barney becomes too hard to kill
#endif
	bool ShouldLookForBetterWeapon() { return false; }
#ifdef DARKINTERVAL
	int MeleeAttack2Conditions(float flDot, float flDist); // For kick/punch
#endif
	void OnChangeRunningBehavior( CAI_BehaviorBase *pOldBehavior,  CAI_BehaviorBase *pNewBehavior );

	float GetHitgroupDamageMultiplier(int iHitGroup, const CTakeDamageInfo &info);
	void DeathSound( const CTakeDamageInfo &info );
	void GatherConditions();
	void UseFunc( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	CAI_FuncTankBehavior		m_FuncTankBehavior;
	COutputEvent				m_OnPlayerUse;

	DEFINE_CUSTOM_AI;
};


LINK_ENTITY_TO_CLASS( npc_barney, CNPC_Barney );
#ifdef DARKINTERVAL
//LINK_ENTITY_TO_CLASS(monster_barney, CNPC_Barney);
#endif

IMPLEMENT_SERVERCLASS_ST(CNPC_Barney, DT_NPC_Barney)
END_SEND_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Barney::SelectModel()
{
	SetModelName( AllocPooledString( BARNEY_MODEL ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Barney::Spawn( void )
{
	Precache();
#ifdef DARKINTERVAL
	m_iHealth = 100;
#endif
	m_iszIdleExpression = MAKE_STRING("scenes/Expressions/BarneyIdle.vcd");
	m_iszAlertExpression = MAKE_STRING("scenes/Expressions/BarneyAlert.vcd");
	m_iszCombatExpression = MAKE_STRING("scenes/Expressions/BarneyCombat.vcd");

	BaseClass::Spawn();
#ifndef DARKINTERVAL
	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION );
#else
	CapabilitiesAdd(bits_CAP_INNATE_MELEE_ATTACK1| bits_CAP_INNATE_MELEE_ATTACK2);

	AddSpawnFlags(SF_NPC_NO_PLAYER_PUSHAWAY);
#endif
	NPCInit();

	SetUse( &CNPC_Barney::UseFunc );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : 
//-----------------------------------------------------------------------------
Class_T	CNPC_Barney::Classify( void )
{
	return	CLASS_PLAYER_ALLY_VITAL;
}
#ifdef DARKINTERVAL
//-----------------------------------------------------------------------------
// Purpose: For combine melee attack (kick/hit)
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CNPC_Barney::MeleeAttack2Conditions(float flDot, float flDist)
{
	// Only kick the player
	if (GetEnemy() && GetEnemy()->IsPlayer() == false)
		return COND_NONE;

	if (flDist > 64)
	{
		return COND_NONE; // COND_TOO_FAR_TO_ATTACK;
	}
	else if (flDot < 0.7)
	{
		return COND_NONE; // COND_NOT_FACING_ATTACK;
	}

	// Check Z
	if (GetEnemy() && fabs(GetEnemy()->GetAbsOrigin().z - GetAbsOrigin().z) > 64)
		return COND_NONE;

	// Make sure not trying to kick through a window or something. 
	trace_t tr;
	Vector vecSrc, vecEnd;

	vecSrc = WorldSpaceCenter();
	vecEnd = GetEnemy()->WorldSpaceCenter();

	AI_TraceLine(vecSrc, vecEnd, MASK_SHOT, this, COLLISION_GROUP_NONE, &tr);
	if (tr.m_pEnt != GetEnemy())
	{
		return COND_NONE;
	}

	return COND_CAN_MELEE_ATTACK2;
}
#endif
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Barney::Weapon_Equip( CBaseCombatWeapon *pWeapon )
{
	BaseClass::Weapon_Equip( pWeapon );

	if( hl2_episodic.GetBool() && FClassnameIs( pWeapon, "weapon_ar2" ) )
	{
		// Allow Barney to defend himself at point-blank range in c17_05.
		pWeapon->m_fMinRange1 = 0.0f;
	}
#ifdef DARKINTERVAL
	if (FClassnameIs(pWeapon, "weapon_pistol"))
	{
		pWeapon->m_fMinRange1 = 0.0f;
	}
#endif
}
#ifdef DARKINTERVAL
void CNPC_Barney::InputUnholsterWeapon(inputdata_t &inputdata) // temp hack until better holstering and unholstering is added
{
	if (IsWeaponHolstered())
	{
		if (UnholsterWeapon() != -1)
		{
			if (GetActiveWeapon())
			{
				GetActiveWeapon()->Deploy();
			}
			return;
		}
		else
		{
			Warning("NPC %s is told to unholster weapon, but lacking proper unholster animations!\n", GetDebugName());
			// Deploy the first weapon you can find
			for (int i = 0; i < WeaponCount(); i++)
			{
				if (GetWeapon(i))
				{
					SetActiveWeapon(GetWeapon(i));

					GetActiveWeapon()->Deploy();

					// Refill the clip
					if (GetActiveWeapon()->UsesClipsForAmmo1())
					{
						GetActiveWeapon()->m_iClip1 = GetActiveWeapon()->GetMaxClip1();
					}

					// Make sure we don't try to reload while we're unholstering
					ClearCondition(COND_LOW_PRIMARY_AMMO);
					ClearCondition(COND_NO_PRIMARY_AMMO);
					ClearCondition(COND_NO_SECONDARY_AMMO);
				}
			}
		}

		SetDesiredWeaponState(DESIREDWEAPONSTATE_UNHOLSTERED);
	}
}
#endif
//---------------------------------------------------------
//---------------------------------------------------------
void CNPC_Barney::HandleAnimEvent( animevent_t *pEvent )
{
	switch( pEvent->event )
	{
	case NPC_EVENT_LEFTFOOT:
		{
			EmitSound( "NPC_Barney.FootstepLeft", pEvent->eventtime );
		}
		break;
	case NPC_EVENT_RIGHTFOOT:
		{
			EmitSound( "NPC_Barney.FootstepRight", pEvent->eventtime );
		}
		break;
#ifdef DARKINTERVAL
	case 1000:
		{
			CBaseEntity *pHurt = CheckTraceHullAttack(50, -Vector(16, 16, 32), Vector(16, 16, 32), 15, DMG_CLUB, 4.0f, false);
			if (pHurt)
			{
				Vector vecForceDir = (pHurt->WorldSpaceCenter() - WorldSpaceCenter());

				if (pHurt->IsPlayer())
				{
					CBasePlayer *pPlayer = ToBasePlayer(pHurt);
					if (pPlayer != NULL)
					{
						//Kick the player angles
						pPlayer->ViewPunch(QAngle(RandomInt(-3, -3), 0, 10));

						Vector	dir = pHurt->GetAbsOrigin() - GetAbsOrigin();
						VectorNormalize(dir);
						QAngle angles;
						VectorAngles(dir, angles);
						Vector forward, right;
						AngleVectors(angles, &forward, &right, NULL);

						//If not on ground, then don't make them fly!
						if (!(pHurt->GetFlags() & FL_ONGROUND))
							forward.z = 0.0f;

						//Push the target back
						pHurt->ApplyAbsVelocityImpulse(forward * 700.0f);

						// Force the player to drop anyting they were holding
						pPlayer->ForceDropOfCarriedPhysObjects();
					}
				}
				else
				{
				}
				// Play a random attack hit sound
				EmitSound("NPC_Metropolice.Shove");
			}
		}
		break;
#endif
	default:
		BaseClass::HandleAnimEvent( pEvent );
		break;
	}
}

//------------------------------------------------------------------------------
// Make Barney not take extra headshot damage, make him more resistant
//------------------------------------------------------------------------------

float CNPC_Barney::GetHitgroupDamageMultiplier(int iHitGroup, const CTakeDamageInfo &info)
{
	switch (iHitGroup)
	{
	case HITGROUP_HEAD:
	case HITGROUP_CHEST:
	case HITGROUP_STOMACH:
	{
		return 1.0f;
	}
	}

	return BaseClass::GetHitgroupDamageMultiplier(iHitGroup, info);
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void CNPC_Barney::DeathSound( const CTakeDamageInfo &info )
{
	// Sentences don't play on dead NPCs
	SentenceStop();

	EmitSound( "npc_barney.die" );

}

bool CNPC_Barney::CreateBehaviors( void )
{
	BaseClass::CreateBehaviors();
	AddBehavior( &m_FuncTankBehavior );

	return true;
}

void CNPC_Barney::OnChangeRunningBehavior( CAI_BehaviorBase *pOldBehavior,  CAI_BehaviorBase *pNewBehavior )
{
	if ( pNewBehavior == &m_FuncTankBehavior )
	{
		m_bReadinessCapable = false;
	}
	else if ( pOldBehavior == &m_FuncTankBehavior )
	{
		m_bReadinessCapable = IsReadinessCapable();
	}

	BaseClass::OnChangeRunningBehavior( pOldBehavior, pNewBehavior );
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Barney::GatherConditions()
{
	BaseClass::GatherConditions();

	// Handle speech AI. Don't do AI speech if we're in scripts unless permitted by the EnableSpeakWhileScripting input.
	if ( m_NPCState == NPC_STATE_IDLE || m_NPCState == NPC_STATE_ALERT || m_NPCState == NPC_STATE_COMBAT ||
		( ( m_NPCState == NPC_STATE_SCRIPT ) && CanSpeakWhileScripting() ) )
	{
		DoCustomSpeechAI();
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Barney::UseFunc( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	m_bDontUseSemaphore = true;
	SpeakIfAllowed( TLK_USE );
	m_bDontUseSemaphore = false;

	m_OnPlayerUse.FireOutput( pActivator, pCaller );
}

//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CNPC_Barney)
	//	m_FuncTankBehavior
	DEFINE_OUTPUT(m_OnPlayerUse, "OnPlayerUse"),
	DEFINE_USEFUNC(UseFunc),
#ifdef DARKINTERVAL
	DEFINE_INPUTFUNC(FIELD_VOID, "UnholsterWeapon", InputUnholsterWeapon), // temp hack until better holstering and unholstering is added
#endif
END_DATADESC()

//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------

AI_BEGIN_CUSTOM_NPC( npc_barney, CNPC_Barney )

AI_END_CUSTOM_NPC()
