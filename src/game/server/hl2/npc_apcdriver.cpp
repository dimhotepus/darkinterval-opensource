//========================================================================//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "npc_vehicledriver.h"
#include "vehicle_apc.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef DARKINTERVAL
#define SF_APCDRIVER_NO_ROCKET_ATTACK	1<<16
#define SF_APCDRIVER_NO_GUN_ATTACK		1<<17

#define NPC_APCDRIVER_REMEMBER_TIME		sk_apc_gun_remember_time.GetFloat()

ConVar sk_apc_gun_remember_time("sk_apc_gun_remember_time", "15");
#else
#define SF_APCDRIVER_NO_ROCKET_ATTACK	0x10000
#define SF_APCDRIVER_NO_GUN_ATTACK		0x20000

#define NPC_APCDRIVER_REMEMBER_TIME		4
#endif
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CNPC_APCDriver : public CNPC_VehicleDriver
{
	DECLARE_CLASS( CNPC_APCDriver, CNPC_VehicleDriver );
public:
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	virtual void Spawn( void );
	virtual void Activate( void );
#ifdef DARKINTERVAL
	virtual void NPCThink(void); // for thrusters usage
#endif
	virtual bool FVisible( CBaseEntity *pTarget, int traceMask, CBaseEntity **ppBlocker );
	virtual bool WeaponLOSCondition( const Vector &ownerPos, const Vector &targetPos, bool bSetConditions );
	virtual Class_T Classify ( void ) { return CLASS_COMBINE; }
	virtual void PrescheduleThink( );
	virtual Disposition_t IRelationType(CBaseEntity *pTarget);

	// AI
	virtual int RangeAttack1Conditions( float flDot, float flDist );
	virtual int RangeAttack2Conditions( float flDot, float flDist );
#ifdef DARKINTERVAL
	Activity NPC_TranslateActivity(Activity eNewActivity); // fix for models that don't have act_range_attack1 and/or 2
#endif
private:
	// Are we being carried by a dropship?
	bool IsBeingCarried();

	// Enable, disable firing
	void InputEnableFiring( inputdata_t &inputdata );
	void InputDisableFiring( inputdata_t &inputdata );

	CHandle<CPropAPC>	m_hAPC;
	float m_flTimeLastSeenEnemy;
	bool m_bFiringDisabled;
#ifdef DARKINTERVAL
	float m_nextthrusterboost_time;
#endif
};

LINK_ENTITY_TO_CLASS( npc_apcdriver, CNPC_APCDriver );

//------------------------------------------------------------------------------
// Purpose :
//------------------------------------------------------------------------------
void CNPC_APCDriver::Spawn( void )
{
	BaseClass::Spawn();

	m_flTimeLastSeenEnemy = -NPC_APCDRIVER_REMEMBER_TIME;
#ifdef DARKINTERVAL
	m_nextthrusterboost_time = 0;

	CapabilitiesClear();
	if( !HasSpawnFlags(SF_APCDRIVER_NO_GUN_ATTACK))
		CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK1);
	if (!HasSpawnFlags(SF_APCDRIVER_NO_ROCKET_ATTACK))
	{
		CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK2);
	}
#else
	CapabilitiesAdd(bits_CAP_INNATE_RANGE_ATTACK1 | bits_CAP_INNATE_RANGE_ATTACK2);
#endif
	m_bFiringDisabled = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_APCDriver::Activate( void )
{
	BaseClass::Activate();

	m_hAPC = dynamic_cast<CPropAPC*>((CBaseEntity*)m_hVehicleEntity);
	if ( !m_hAPC )
	{
		Warning( "npc_apcdriver %s couldn't find his apc named %s.\n", STRING(GetEntityName()), STRING(m_iszVehicleName) );
		UTIL_Remove( this );
		return;
	}
#ifdef DARKINTERVAL
//	m_hAPC->m_nSkin = APC_SKIN_NPCCONTROLLED; // prototyped it at first but it went unused
#endif
	SetParent( m_hAPC );
	SetAbsOrigin( m_hAPC->WorldSpaceCenter() );
	SetLocalAngles( vec3_angle );

	m_flDistTooFar = m_hAPC->MaxAttackRange();
	SetDistLook( m_hAPC->MaxAttackRange() );
}

//-----------------------------------------------------------------------------
// Aim the laser dot!
//-----------------------------------------------------------------------------
void CNPC_APCDriver::PrescheduleThink()
{
	BaseClass::PrescheduleThink();

	if (m_hAPC->m_lifeState == LIFE_ALIVE)
	{
		if (GetEnemy())
		{
#ifdef DARKINTERVAL
			if (!HasSpawnFlags(SF_APCDRIVER_NO_GUN_ATTACK))
#endif
				m_hAPC->AimPrimaryWeapon(GetEnemy()->BodyTarget(GetAbsOrigin(), false));

#ifdef DARKINTERVAL
			if (!HasSpawnFlags(SF_APCDRIVER_NO_ROCKET_ATTACK))
#endif
				m_hAPC->AimSecondaryWeaponAt(GetEnemy());
		}
	}
	else if (m_hAPC->m_lifeState == LIFE_DEAD)
	{
#ifdef DARKINTERVAL
	//	m_hAPC->m_nSkin = APC_SKIN_NOTCONTROLLED;
#endif
		UTIL_Remove(this);
	}
}
#ifdef DARKINTERVAL
void CNPC_APCDriver::NPCThink(void)
{
	BaseClass::NPCThink();

	if (m_hAPC->IsOverturned() && m_nextthrusterboost_time < CURTIME)
	{
		m_pVehicleInterface->NPC_Thruster();
		m_nextthrusterboost_time = CURTIME + RandomFloat(3, 6);
	}
}
#endif
//-----------------------------------------------------------------------------
// Enable, disable firing
//-----------------------------------------------------------------------------
void CNPC_APCDriver::InputEnableFiring( inputdata_t &inputdata )
{
	m_bFiringDisabled = false;
}

void CNPC_APCDriver::InputDisableFiring( inputdata_t &inputdata )
{
	m_bFiringDisabled = true;
}
	
//-----------------------------------------------------------------------------
// Purpose: Let's not hate things the APC makes
//-----------------------------------------------------------------------------
Disposition_t CNPC_APCDriver::IRelationType(CBaseEntity *pTarget)
{
	if ( pTarget == m_hAPC || (pTarget->GetOwnerEntity() == m_hAPC) )
		return D_LI;

	return BaseClass::IRelationType(pTarget);
}

//------------------------------------------------------------------------------
// Are we being carried by a dropship?
//------------------------------------------------------------------------------
bool CNPC_APCDriver::IsBeingCarried()
{
	// Inert if we're carried...
	Vector vecVelocity;
	m_hAPC->GetVelocity( &vecVelocity, NULL );
	return ( m_hAPC->GetMoveParent() != NULL ) || (fabs(vecVelocity.z) >= 15);
}

//------------------------------------------------------------------------------
// Is the enemy visible?
//------------------------------------------------------------------------------
bool CNPC_APCDriver::WeaponLOSCondition(const Vector &ownerPos, const Vector &targetPos, bool bSetConditions)
{
	if ( m_hAPC->m_lifeState != LIFE_ALIVE )
		return false;

	if ( IsBeingCarried() || m_bFiringDisabled )
		return false;

	float flTargetDist = ownerPos.DistTo( targetPos );
	if (flTargetDist > m_hAPC->MaxAttackRange())
		return false;

	return true;
}
	
//-----------------------------------------------------------------------------
// Is the enemy visible?
//-----------------------------------------------------------------------------
bool CNPC_APCDriver::FVisible( CBaseEntity *pTarget, int traceMask, CBaseEntity **ppBlocker )
{
	if ( m_hAPC->m_lifeState != LIFE_ALIVE )
		return false;

	if ( IsBeingCarried() || m_bFiringDisabled )
		return false;

	float flTargetDist = GetAbsOrigin().DistTo( pTarget->GetAbsOrigin() );
	if (flTargetDist > m_hAPC->MaxAttackRange())
		return false;

	bool bVisible = m_hAPC->FVisible( pTarget, traceMask, ppBlocker );
	if ( bVisible && (pTarget == GetEnemy()) )
	{
		m_flTimeLastSeenEnemy = CURTIME;
	}

	if ( pTarget->IsPlayer() || pTarget->Classify() == CLASS_BULLSEYE )
	{
		if (!bVisible)
		{
			if ( ( CURTIME - m_flTimeLastSeenEnemy ) <= NPC_APCDRIVER_REMEMBER_TIME )
				return true;
		}
	}

	return bVisible;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int CNPC_APCDriver::RangeAttack1Conditions( float flDot, float flDist )
{
	if ( HasSpawnFlags(SF_APCDRIVER_NO_GUN_ATTACK) )
		return COND_NONE;

	if ( m_hAPC->m_lifeState != LIFE_ALIVE )
		return COND_NONE;

	if ( IsBeingCarried() || m_bFiringDisabled )
		return COND_NONE;

	if ( !HasCondition( COND_SEE_ENEMY ) )
		return COND_NONE;

	if ( !m_hAPC->IsInPrimaryFiringCone() )
		return COND_NONE;

	// Vehicle not ready to fire again yet?
	if ( m_pVehicleInterface->Weapon_PrimaryCanFireAt() > CURTIME + 0.1f )
		return COND_NONE;

	float flMinDist, flMaxDist;
	m_pVehicleInterface->Weapon_PrimaryRanges( &flMinDist, &flMaxDist );

	if (flDist < flMinDist)
		return COND_NONE;

	if (flDist > flMaxDist)
		return COND_NONE;

	return COND_CAN_RANGE_ATTACK1;
}

int CNPC_APCDriver::RangeAttack2Conditions( float flDot, float flDist )
{
	if ( HasSpawnFlags(SF_APCDRIVER_NO_ROCKET_ATTACK) )
		return COND_NONE;

	if ( m_hAPC->m_lifeState != LIFE_ALIVE )
		return COND_NONE;

	if ( IsBeingCarried() || m_bFiringDisabled )
		return COND_NONE;

	if ( !HasCondition( COND_SEE_ENEMY ) )
		return COND_NONE;

	// Vehicle not ready to fire again yet?
	if ( m_pVehicleInterface->Weapon_SecondaryCanFireAt() > CURTIME + 0.1f )
		return COND_NONE;

	float flMinDist, flMaxDist;
	m_pVehicleInterface->Weapon_SecondaryRanges( &flMinDist, &flMaxDist );

	if (flDist < flMinDist)
		return COND_NONE;

	if (flDist > flMaxDist)
		return COND_NONE;

	return COND_CAN_RANGE_ATTACK2;
}
#ifdef DARKINTERVAL
// the original APC driver relies on using a special model,
// edited rollermine, that has bogus range attacks 1 and 2
// anims. Replacing it with another model (which is done
// for visible drivers in DI) potentially breaks attacks.
// This is where this translate activity comes into play.
Activity CNPC_APCDriver::NPC_TranslateActivity(Activity eNewActivity)
{
	if (eNewActivity == ACT_RANGE_ATTACK1) return ACT_IDLE;
	if (eNewActivity == ACT_RANGE_ATTACK2) return ACT_IDLE;

	return BaseClass::NPC_TranslateActivity(eNewActivity);
}
#endif
BEGIN_DATADESC(CNPC_APCDriver)

//DEFINE_FIELD( m_hAPC, FIELD_EHANDLE ),
DEFINE_FIELD(m_bFiringDisabled, FIELD_BOOLEAN),
DEFINE_FIELD(m_flTimeLastSeenEnemy, FIELD_TIME),
#ifdef DARKINTERVAL
DEFINE_FIELD(m_nextthrusterboost_time, FIELD_TIME),
#endif
DEFINE_INPUTFUNC(FIELD_VOID, "EnableFiring", InputEnableFiring),
DEFINE_INPUTFUNC(FIELD_VOID, "DisableFiring", InputDisableFiring),

END_DATADESC()
//-----------------------------------------------------------------------------
//
// Schedules
//
//-----------------------------------------------------------------------------
AI_BEGIN_CUSTOM_NPC( npc_apcdriver, CNPC_APCDriver )
	
AI_END_CUSTOM_NPC()
