//========================================================================//
//
// Purpose: 
//
//=============================================================================//

#ifndef NPC_COMBINES_H
#define NPC_COMBINES_H
#ifdef _WIN32
#pragma once
#endif

#include "npc_combine.h"

//=========================================================
//	>> CNPC_CombineS
//=========================================================
class CNPC_CombineS : public CNPC_Combine
{
	DECLARE_CLASS( CNPC_CombineS, CNPC_Combine );
	DECLARE_DATADESC();

public: 
	void		Spawn( void );
	void		Precache( void );
	void		DeathSound( const CTakeDamageInfo &info );
	void		PrescheduleThink( void );
	void		BuildScheduleTestBits( void );
	int			SelectSchedule ( void );
#ifdef DARKINTERVAL
protected:
	virtual		Activity GetFlinchActivity(bool bHeavyDamage, bool bGesture);
public:
	void		PlayFlinchGesture(void);
#endif
	float		GetHitgroupDamageMultiplier( int iHitGroup, const CTakeDamageInfo &info );
	void		TraceAttack(const CTakeDamageInfo &info, const Vector &vecDir, trace_t *ptr );
	void		HandleAnimEvent( animevent_t *pEvent );
	void		OnChangeActivity( Activity eNewActivity );
	void		Event_Killed( const CTakeDamageInfo &info );
	void		OnListened();

	void		ClearAttackConditions( void );
#ifdef DARKINTERVAL
	WeaponProficiency_t CalcWeaponProficiency(CBaseCombatWeapon *pWeapon);
#endif
	bool		m_fIsBlocking;

	bool		IsLightDamage( const CTakeDamageInfo &info );
	bool		IsHeavyDamage( const CTakeDamageInfo &info );

	virtual	bool		AllowedToIgnite( void ) { return true; }

private:
	bool		ShouldHitPlayer( const Vector &targetDir, float targetDist );
#ifdef DARKINTERVAL // flinch code, taken from metrocops
	float		m_flLastPhysicsFlinchTime;
	float		m_flLastDamageFlinchTime;
	float		m_flLastHitYaw;
#endif
#if HL2_EPISODIC
public:
	Activity	NPC_TranslateActivity( Activity eNewActivity );
#ifndef DARKINTERVAL
protected:
#endif
	int			m_iUseMarch;
#endif

};

#endif // NPC_COMBINES_H
