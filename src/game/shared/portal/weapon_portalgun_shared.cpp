#include "cbase.h"
#ifdef PORTAL_COMPILE
#include "weapon_portalgun_shared.h"
#include "npcevent.h"
#include "in_buttons.h"
#include "rumble_shared.h"

#include "prop_portal_shared.h"

#ifdef CLIENT_DLL
	#define CWeaponPortalgun C_WeaponPortalgun
#endif //#ifdef CLIENT_DLL


acttable_t	CWeaponPortalgun::m_acttable[] = 
{
	{ ACT_RANGE_ATTACK1,			ACT_RANGE_ATTACK_AR2,			true },
	{ ACT_RELOAD,					ACT_RELOAD_SMG1,				true },
	{ ACT_IDLE,						ACT_IDLE_SMG1,					true },
	{ ACT_IDLE_ANGRY,				ACT_IDLE_ANGRY_SMG1,			true },
	{ ACT_WALK,						ACT_WALK_RIFLE,					true },
	// Readiness activities (not aiming)
	{ ACT_IDLE_RELAXED,				ACT_IDLE_SMG1_RELAXED,			false },//never aims
	{ ACT_IDLE_STIMULATED,			ACT_IDLE_SMG1_STIMULATED,		false },
	{ ACT_IDLE_AGITATED,			ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_RELAXED,				ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_STIMULATED,			ACT_WALK_RIFLE_STIMULATED,		false },
	{ ACT_WALK_AGITATED,			ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_RELAXED,				ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_STIMULATED,			ACT_RUN_RIFLE_STIMULATED,		false },
	{ ACT_RUN_AGITATED,				ACT_RUN_AIM_RIFLE,				false },//always aims
																			// Readiness activities (aiming)
	{ ACT_IDLE_AIM_RELAXED,			ACT_IDLE_SMG1_RELAXED,			false },//never aims	
	{ ACT_IDLE_AIM_STIMULATED,		ACT_IDLE_AIM_RIFLE_STIMULATED,	false },
	{ ACT_IDLE_AIM_AGITATED,		ACT_IDLE_ANGRY_SMG1,			false },//always aims

	{ ACT_WALK_AIM_RELAXED,			ACT_WALK_RIFLE_RELAXED,			false },//never aims
	{ ACT_WALK_AIM_STIMULATED,		ACT_WALK_AIM_RIFLE_STIMULATED,	false },
	{ ACT_WALK_AIM_AGITATED,		ACT_WALK_AIM_RIFLE,				false },//always aims

	{ ACT_RUN_AIM_RELAXED,			ACT_RUN_RIFLE_RELAXED,			false },//never aims
	{ ACT_RUN_AIM_STIMULATED,		ACT_RUN_AIM_RIFLE_STIMULATED,	false },
	{ ACT_RUN_AIM_AGITATED,			ACT_RUN_AIM_RIFLE,				false },//always aims
																			//End readiness activities
	{ ACT_WALK_AIM,					ACT_WALK_AIM_RIFLE,				true },
	{ ACT_WALK_CROUCH,				ACT_WALK_CROUCH_RIFLE,			true },
	{ ACT_WALK_CROUCH_AIM,			ACT_WALK_CROUCH_AIM_RIFLE,		true },
	{ ACT_RUN,						ACT_RUN_RIFLE,					true },
	{ ACT_RUN_AIM,					ACT_RUN_AIM_RIFLE,				true },
	{ ACT_RUN_CROUCH,				ACT_RUN_CROUCH_RIFLE,			true },
	{ ACT_RUN_CROUCH_AIM,			ACT_RUN_CROUCH_AIM_RIFLE,		true },
	{ ACT_GESTURE_RANGE_ATTACK1,	ACT_GESTURE_RANGE_ATTACK_AR2,	false },
	{ ACT_COVER_LOW,				ACT_COVER_SMG1_LOW,				false },
	{ ACT_RANGE_AIM_LOW,			ACT_RANGE_AIM_AR2_LOW,			false },
	{ ACT_RANGE_ATTACK1_LOW,		ACT_RANGE_ATTACK_SMG1_LOW,		true },
	{ ACT_RELOAD_LOW,				ACT_RELOAD_SMG1_LOW,			false },
	{ ACT_GESTURE_RELOAD,			ACT_GESTURE_RELOAD_SMG1,		true },
	{ ACT_RANGE_ATTACK2,			ACT_RANGE_ATTACK_AR2_GRENADE,	true },
};

IMPLEMENT_ACTTABLE(CWeaponPortalgun);


//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CWeaponPortalgun::CWeaponPortalgun( void )
{
	m_reloads_singly_boolean = true;

	// TODO: specify these in hammer instead of assuming every gun has blue chip
	m_bCanFirePortal1 = true;
	m_bCanFirePortal2 = false;

	m_iLastFiredPortal = 0;
	m_fCanPlacePortal1OnThisSurface = 1.0f;
	m_fCanPlacePortal2OnThisSurface = 1.0f;

	m_fMinRange1	= 0.0f;
	m_fMaxRange1	= MAX_TRACE_LENGTH;
	m_fMinRange2	= 0.0f;
	m_fMaxRange2	= MAX_TRACE_LENGTH;

	m_EffectState	= EFFECT_NONE;
}

void CWeaponPortalgun::Precache()
{
	BaseClass::Precache();

	PrecacheModel( PORTALGUN_BEAM_SPRITE );
	PrecacheModel( PORTALGUN_BEAM_SPRITE_NOZ );

	PrecacheModel( "models/portals/portal1.mdl" );
	PrecacheModel( "models/portals/portal2.mdl" );

	PrecacheScriptSound( "Portal.ambient_loop" );

	PrecacheScriptSound( "Portal.open_blue" );
	PrecacheScriptSound( "Portal.open_red" );
	PrecacheScriptSound( "Portal.close_blue" );
	PrecacheScriptSound( "Portal.close_red" );
	PrecacheScriptSound( "Portal.fizzle_moved" );
	PrecacheScriptSound( "Portal.fizzle_invalid_surface" );
	PrecacheScriptSound( "Weapon_Portalgun.powerup" );
	PrecacheScriptSound( "Weapon_PhysCannon.HoldSound" );

#ifndef CLIENT_DLL
//	PrecacheParticleSystem( "portal_1_projectile_stream" );
//	PrecacheParticleSystem( "portal_1_projectile_stream_pedestal" );
//	PrecacheParticleSystem( "portal_2_projectile_stream" );
//	PrecacheParticleSystem( "portal_2_projectile_stream_pedestal" );
//	PrecacheParticleSystem( "portal_1_charge" );
//	PrecacheParticleSystem( "portal_2_charge" );
#endif
}

PRECACHE_WEAPON_REGISTER(weapon_portalgun);

bool CWeaponPortalgun::ShouldDrawCrosshair( void )
{
	return true;//( m_fCanPlacePortal1OnThisSurface > 0.5f || m_fCanPlacePortal2OnThisSurface > 0.5f );
}

//-----------------------------------------------------------------------------
// Purpose: Override so only reload one shell at a time
// Input  :
// Output :
//-----------------------------------------------------------------------------
bool CWeaponPortalgun::Reload( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Play finish reload anim and fill clip
// Input  :
// Output :
//-----------------------------------------------------------------------------
void CWeaponPortalgun::FillClip( void )
{
	CBaseCombatCharacter *pOwner  = GetOwner();
	
	if ( pOwner == NULL )
		return;

	// Add them to the clip
	if ( pOwner->GetAmmoCount( m_iPrimaryAmmoType ) > 0 )
	{
		if ( Clip1() < GetMaxClip1() )
		{
			m_iClip1++;
			pOwner->RemoveAmmo( 1, m_iPrimaryAmmoType );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponPortalgun::DryFire( void )
{
	WeaponSound(EMPTY);
	SendWeaponAnim( ACT_VM_DRYFIRE );
	
	m_flNextPrimaryAttack = CURTIME + SequenceDuration();
}

void CWeaponPortalgun::SetCanFirePortal1( bool bCanFire /*= true*/ )
{
	m_bCanFirePortal1 = bCanFire;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;
/*
	if ( !m_bOpenProngs )
	{
		DoEffect( EFFECT_HOLDING );
		DoEffect( EFFECT_READY );
	}
*/
	// TODO: Remove muzzle flash when there's an upgrade animation
	pOwner->DoMuzzleFlash();

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = CURTIME + 0.25f;
	m_flNextSecondaryAttack = CURTIME + 0.25f;

	// player "shoot" animation
	pOwner->SetAnimation( PLAYER_ATTACK1 );

	pOwner->ViewPunch( QAngle( random->RandomFloat( -1, -0.5f ), random->RandomFloat( -1, 1 ), 0 ) );

	EmitSound( "Weapon_Portalgun.powerup" );
}

void CWeaponPortalgun::SetCanFirePortal2( bool bCanFire /*= true*/ )
{
	m_bCanFirePortal2 = bCanFire;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner == NULL )
		return;
/*
	if ( !m_bOpenProngs )
	{
		DoEffect( EFFECT_HOLDING );
		DoEffect( EFFECT_READY );
	}
*/
	// TODO: Remove muzzle flash when there's an upgrade animation
	pOwner->DoMuzzleFlash();

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = CURTIME + 0.5f;
	m_flNextSecondaryAttack = CURTIME + 0.5f;

	// player "shoot" animation
	pOwner->SetAnimation( PLAYER_ATTACK1 );

	pOwner->ViewPunch( QAngle( random->RandomFloat( -1, -0.5f ), random->RandomFloat( -1, 1 ), 0 ) );

	EmitSound( "Weapon_Portalgun.powerup" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponPortalgun::PrimaryAttack( void )
{
	if ( !CanFirePortal1() )
		return;

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
	{
		return;
	}

#ifndef CLIENT_DLL
	inputdata_t inputdata;
	inputdata.pActivator = this;
	inputdata.pCaller = this;
	inputdata.value;//null
	FirePortal1( inputdata );
	m_OnFiredPortal1.FireOutput( pPlayer, this );

	pPlayer->RumbleEffect( RUMBLE_PORTALGUN_LEFT, 0, RUMBLE_FLAGS_NONE );
#endif

	pPlayer->DoMuzzleFlash();

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = CURTIME + 0.5f;//SequenceDuration();
	m_flNextSecondaryAttack = CURTIME + 0.5f;//SequenceDuration();

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	pPlayer->ViewPunch( QAngle( random->RandomFloat( -1, -0.5f ), random->RandomFloat( -1, 1 ), 0 ) );
}

//-----------------------------------------------------------------------------
// Purpose: 
//
//
//-----------------------------------------------------------------------------
void CWeaponPortalgun::SecondaryAttack( void )
{
	if ( !CanFirePortal2() )
		return;

	// Only the player fires this way so we can cast
	CBasePlayer *pPlayer = ToBasePlayer( GetOwner() );

	if (!pPlayer)
	{
		return;
	}

#ifndef CLIENT_DLL
	inputdata_t inputdata;
	inputdata.pActivator = this;
	inputdata.pCaller = this;
	inputdata.value;//null
	FirePortal2( inputdata );
	m_OnFiredPortal2.FireOutput( pPlayer, this );
	pPlayer->RumbleEffect( RUMBLE_PORTALGUN_RIGHT, 0, RUMBLE_FLAGS_NONE );
#endif

	pPlayer->DoMuzzleFlash();

	// Don't fire again until fire animation has completed
	m_flNextPrimaryAttack = CURTIME + 0.5f;//SequenceDuration();
	m_flNextSecondaryAttack = CURTIME + 0.5f;//SequenceDuration();

	// player "shoot" animation
	pPlayer->SetAnimation( PLAYER_ATTACK1 );

	pPlayer->ViewPunch( QAngle( random->RandomFloat( -1, -0.5f ), random->RandomFloat( -1, 1 ), 0 ) );
}

void CWeaponPortalgun::DelayAttack( float fDelay )
{
	m_flNextPrimaryAttack = CURTIME + fDelay;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CWeaponPortalgun::ItemHolsterFrame( void )
{
	// Must be player held
	if ( GetOwner() && GetOwner()->IsPlayer() == false )
		return;

	// We can't be active
	if ( GetOwner()->GetActiveWeapon() == this )
		return;
#ifdef DARKINTERVAL
	if (sk_auto_reload_enabled.GetBool())
	{
#endif
		// If it's been longer than three seconds, reload
		if ((CURTIME - m_flHolsterTime) > sk_auto_reload_time.GetFloat())
		{
			// Reset the timer
			m_flHolsterTime = CURTIME;

			if (GetOwner() == NULL)
				return;

			if (m_iClip1 == GetMaxClip1())
				return;

			// Just load the clip with no animations
			int ammoFill = min((GetMaxClip1() - m_iClip1), GetOwner()->GetAmmoCount(GetPrimaryAmmoType()));

			GetOwner()->RemoveAmmo(ammoFill, GetPrimaryAmmoType());
			m_iClip1 += ammoFill;
		}
#ifdef DARKINTERVAL
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponPortalgun::Holster( CBaseCombatWeapon *pSwitchingTo )
{
//	DestroyEffects();

	return BaseClass::Holster( pSwitchingTo );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CWeaponPortalgun::Deploy( void )
{
//	DoEffect( EFFECT_READY );

	bool bReturn = BaseClass::Deploy();

	m_flNextSecondaryAttack = m_flNextPrimaryAttack = CURTIME;

	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );

	if ( pOwner )
	{
		pOwner->SetNextAttack( CURTIME );

#ifndef CLIENT_DLL
		if( GameRules()->IsMultiplayer() )
		{
			m_iPortalLinkageGroupID = pOwner->entindex();

			Assert( (m_iPortalLinkageGroupID >= 0) && (m_iPortalLinkageGroupID < 256) );
		}
#endif
	}

	return bReturn;
}

void CWeaponPortalgun::WeaponIdle( void )
{
	//See if we should idle high or low
	if ( WeaponShouldBeLowered() )
	{
		// Move to lowered position if we're not there yet
		if ( GetActivity() != ACT_VM_IDLE_LOWERED && GetActivity() != ACT_VM_IDLE_TO_LOWERED 
			&& GetActivity() != ACT_TRANSITION )
		{
			SendWeaponAnim( ACT_VM_IDLE_LOWERED );
		}
		else if ( HasWeaponIdleTimeElapsed() )
		{
			// Keep idling low
			SendWeaponAnim( ACT_VM_IDLE_LOWERED );
		}
	}
	else
	{
		// See if we need to raise immediately
		if ( m_flRaiseTime < CURTIME && GetActivity() == ACT_VM_IDLE_LOWERED ) 
		{
			SendWeaponAnim( ACT_VM_IDLE );
		}
		else if ( HasWeaponIdleTimeElapsed() ) 
		{
			SendWeaponAnim( ACT_VM_IDLE );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Restore
//-----------------------------------------------------------------------------
void CWeaponPortalgun::OnRestore()
{
	BaseClass::OnRestore();
/*
	// Portalgun effects disappear through level transition, so
	//  just recreate any effects here
	if ( m_EffectState != EFFECT_NONE )
	{
		DoEffect( m_EffectState, NULL );
	}
*/
}


//-----------------------------------------------------------------------------
// On Remove
//-----------------------------------------------------------------------------
void CWeaponPortalgun::UpdateOnRemove(void)
{
//	DestroyEffects();
	BaseClass::UpdateOnRemove();
}
#endif