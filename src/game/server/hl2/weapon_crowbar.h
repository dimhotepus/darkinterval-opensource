//========================================================================//
//
// Purpose:
//
//=============================================================================//

#ifndef WEAPON_CROWBAR_H
#define WEAPON_CROWBAR_H
#ifdef DARKINTERVAL
//=========================================================
// CBaseHLBludgeonWeapon 
//=========================================================
class CPlayerCrowbar : public CBaseHLCombatWeapon
{
	DECLARE_CLASS(CPlayerCrowbar, CBaseHLCombatWeapon);
public:
	CPlayerCrowbar();

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();
	
	virtual	void	Spawn(void);
	virtual	void	Precache(void);
	virtual void	UpdateViewModel(void);

	//Attack functions
	virtual	void	PrimaryAttack(void);
	virtual	void	SecondaryAttack(void);

	virtual void	ItemPostFrame(void);

	//Functions to select animation sequences 
	virtual Activity	GetPrimaryAttackActivity(void) { return	ACT_VM_HITCENTER; }
	virtual Activity	GetSecondaryAttackActivity(void) { return	ACT_VM_HITCENTER2; }

	virtual	float	GetFireRate(void) { return	0.75f; }
	virtual float	GetRange(void) { return	90.0f; }
	virtual	float	GetDamageForActivity(Activity hitActivity);

	virtual int		CapabilitiesGet(void);
	virtual	int		WeaponMeleeAttack1Condition(float flDot, float flDist);

	virtual void	Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator);
	void			AddViewKick(void);

protected:
	virtual	void	ImpactEffect(trace_t &trace);

private:

	trace_t			saveHitTrace;
	Activity		saveHitActivity;
	bool			saveHitIsSecondary;

	bool			ImpactWater(const Vector &start, const Vector &end);
	void			Swing(int bIsSecondary);
	void			Hit(trace_t &traceHit, Activity nHitActivity, bool bIsSecondary);
	Activity		ChooseIntersectionPointAndActivity(trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner);
};
#else
#include "basebludgeonweapon.h"

#if defined( _WIN32 )
#pragma once
#endif

#define	CROWBAR_RANGE	90.0f
#define	CROWBAR_REFIRE	0.75f

//-----------------------------------------------------------------------------
// CWeaponCrowbar
//-----------------------------------------------------------------------------

class CWeaponCrowbar : public CBaseHLBludgeonWeapon
{
public:
	DECLARE_CLASS(CWeaponCrowbar, CBaseHLBludgeonWeapon);

	DECLARE_SERVERCLASS();
	DECLARE_ACTTABLE();

	CWeaponCrowbar();

	void Precache(void)
	{
		BaseClass::Precache();
		PrecacheScriptSound("DI_Crowbar.Kill");
	}

	float		GetRange( void )		{	return	CROWBAR_RANGE;	}
	float		GetFireRate( void )		{	return	CROWBAR_REFIRE;	}

	void		AddViewKick( void );
	float		GetDamageForActivity( Activity hitActivity );
	
	virtual int WeaponMeleeAttack1Condition( float flDot, float flDist );
	void		MeleeAttack(bool strong = true);
	void		PrimaryAttack(void) { MeleeAttack(true); }
	void		SecondaryAttack( void )	{ MeleeAttack(false); }

	virtual void DefaultTouch(CBaseEntity *pOther)
	{
		BaseClass::DefaultTouch(pOther);

		CBasePlayer *pPlayer = ToBasePlayer(pOther);
		if (pPlayer)
		{
			UTIL_HudHintText(pPlayer, "%+attack% SWING (HIGH DAMAGE) %+attack2% HIT (MEDIUM DAMAGE)");
		}
	}

	void BreakWeapon(void)
	{
		const char *name = "weapon_broken";
		SetName(AllocPooledString(name));
		GetOwner()->Weapon_Drop(this, NULL, NULL);
		UTIL_Remove(this);
	}

	// Animation event
	virtual void Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator );

private:
	// Animation event handlers
	void HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator );
};
#endif
#endif // WEAPON_CROWBAR_H
