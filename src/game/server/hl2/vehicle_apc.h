//========================================================================//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef VEHICLE_APC_H
#define VEHICLE_APC_H

#ifdef _WIN32
#pragma once
#endif

#include "vehicle_base.h"
#include "smoke_trail.h"

//-----------------------------------------------------------------------------
// Purpose: Four wheel physics vehicle server vehicle with weaponry
//-----------------------------------------------------------------------------
#ifdef DARKINTERVAL
class CParticleSystem;
#define NUM_WHEEL_EFFECTS	2 // front wheel dust/splashes
#define APC_SKIN_NOTCONTROLLED 0 // unused now
#define APC_SKIN_NPCCONTROLLED 1
#endif
class CAPCFourWheelServerVehicle : public CFourWheelServerVehicle
{
	typedef CFourWheelServerVehicle BaseClass;
// IServerVehicle
public:
	bool		NPC_HasPrimaryWeapon( void ) { return true; }
	void		NPC_AimPrimaryWeapon( Vector vecTarget );
	bool		NPC_HasSecondaryWeapon( void ) { return true; }
	void		NPC_AimSecondaryWeapon( Vector vecTarget ); 
#ifdef DARKINTERVAL // DI wants the player to get out and grab them manually
	bool		VehicleCanPickupItems(void) { return false; }
#endif
	// Weaponry
	void		Weapon_PrimaryRanges( float *flMinRange, float *flMaxRange );
	void		Weapon_SecondaryRanges( float *flMinRange, float *flMaxRange );
	float		Weapon_PrimaryCanFireAt( void );		// Return the time at which this vehicle's primary weapon can fire again
	float		Weapon_SecondaryCanFireAt( void );		// Return the time at which this vehicle's secondary weapon can fire again
};


//-----------------------------------------------------------------------------
// A driveable vehicle with a gun that shoots wherever the driver looks.
//-----------------------------------------------------------------------------
class CPropAPC : public CPropVehicleDriveable
{
	DECLARE_CLASS( CPropAPC, CPropVehicleDriveable );
	DECLARE_SERVERCLASS();

public:
#ifdef DARKINTERVAL
	CPropAPC();
	~CPropAPC();
#endif
	// CBaseEntity
	virtual void Precache( void );
	void	Think( void );
	virtual void Spawn(void);
	virtual void Activate();
	virtual void UpdateOnRemove( void );
	virtual void OnRestore( void );

	// CPropVehicle
	virtual void	CreateServerVehicle( void );
	virtual void	DriveVehicle( float flFrameTime, CUserCmd *ucmd, int iButtonsDown, int iButtonsReleased );
	virtual void	ProcessMovement( CBasePlayer *pPlayer, CMoveData *pMoveData );
	virtual Class_T	ClassifyPassenger( CBaseCombatCharacter *pPassenger, Class_T defaultClassification );
	virtual int		OnTakeDamage( const CTakeDamageInfo &inputInfo);
	virtual float	PassengerDamageModifier( const CTakeDamageInfo &inputInfo);
#ifdef DARKINTERVAL
	// APC passengers do not directly receive damage from blasts or radiation damage
	virtual bool PassengerShouldReceiveDamage(CTakeDamageInfo &info)
	{
		return (info.GetDamageType() & (DMG_RADIATION | DMG_BLAST | DMG_CRUSH | DMG_ACID | DMG_FREEZE )) == 0;
	}
#endif
	// Weaponry
	const Vector	&GetPrimaryGunOrigin( void );
	void			AimPrimaryWeapon( const Vector &vecForward );
	void			AimSecondaryWeaponAt( CBaseEntity *pTarget );
	float			PrimaryWeaponFireTime( void ) { return m_flMachineGunTime; }
	float			SecondaryWeaponFireTime( void ) { return m_flRocketTime; }
	float			MaxAttackRange() const;
	bool			IsInPrimaryFiringCone() const { return m_bInFiringCone; }
#ifdef DARKINTERVAL
	// Recharges the ammo when not firing
	void			RechargeAmmo();
	// Removes the ammo...
	void			RemoveAmmo(float flAmmoAmount);
	// Do the right thing for the gun
	void			UpdateGunState(CUserCmd *ucmd);

	// APC-specific HUD stuff
	void			SetupAPCInterface(void); // poseparameters, vgui (if applicable)...
	int				DrawDebugTextOverlays(int nOffset);

	// APC self repair
	void			AutoRepairThink(void);
	int				m_auto_repair_goal_hull_amount_int; // when damaged, self repair will kick in, and determine how much HP it needs to reach the nearest rounded value
#endif
	// Muzzle flashes
#ifndef DARKINTERVAL // replaced with a particle system
	const char		*GetTracerType( void ) ;
#else
	virtual void	MakeTracer(const Vector &vecTracerSrc, const trace_t &tr, int iTracerType);
#endif
	void			DoImpactEffect( trace_t &tr, int nDamageType );
	void			DoMuzzleFlash( void );

	virtual Vector	EyePosition( );				// position of eyes
	Vector			BodyTarget( const Vector &posSrc, bool bNoisy );
		
	virtual void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );
#ifdef DARKINTERVAL
	void			InputShowHudHint(inputdata_t &inputdata);

	virtual void	EnterVehicle(CBaseCombatCharacter *pPassenger);
	virtual void	PreExitVehicle(CBaseCombatCharacter *pPlayer, int nRole);
	virtual void	ExitVehicle(int nRole);

	virtual bool	AllowBlockedExit(CBasePlayer *pPlayer, int nRole) 
	{ 
		return false; 
	}

	virtual bool	CanExitVehicle(CBaseEntity *pEntity) 
	{
		return (!m_bEnterAnimOn && !m_bExitAnimOn && !m_bLocked && (m_nSpeed <= 100.0f));
	}

protected:
	CNetworkVar(int, m_nAmmoCount);
	CNetworkVar(int, m_nAmmo2Count);
	CNetworkVar(bool, m_bHeadlightIsOn);
	float m_flDoorCloseTime;
	int m_iNumberOfEntries;
#endif // DARKINTERVAL
private:
	enum
	{
		MAX_SMOKE_TRAILS = 4,
		MAX_EXPLOSIONS = 4,
	};

	void			DoThrusters(void);

	// Should we trigger a damage effect?
	bool ShouldTriggerDamageEffect( int nPrevHealth, int nEffectCount, const CTakeDamageInfo &info) const;

	// Add a smoke trail since we've taken more damage
	void AddSmokeTrail( const Vector &vecPos );

	// Creates the breakable husk of an attack chopper
	void CreateChopperHusk();

	// Pow!
	void ExplodeAndThrowChunk( const Vector &vecExplosionPos );

	void Event_Killed( const CTakeDamageInfo &info );

	// Purpose: 
	void GetRocketShootPosition( Vector *pPosition );

	void FireMachineGun( void );
#ifdef DARKINTERVAL
	void FireNPCRocket(void);
	void FirePlayerRocket(void);

	// Give or take health (hull integrity) // DI TODO: promote for all practical vehicles?
	void InputSetHealth(inputdata_t &inputdata);
	void InputSetHealthFraction(inputdata_t &inputdata);
	void InputAddHealth(inputdata_t &inputdata);
	void InputRemoveHealth(inputdata_t &inputdata);
	void InputRequestHealth(inputdata_t &inputdata);
#else
	void FireRocket(void);
#endif
	// Death volley 
	void FireDying( );

	// Create a corpse 
	void CreateCorpse( );

	// Blows da shizzle up
	void InputDestroy( inputdata_t &inputdata );
	void InputFireMissileAt( inputdata_t &inputdata );

	void CreateAPCLaserDot( void );

	virtual bool ShouldAttractAutoAim( CBaseEntity *pAimingEnt );

	// Danger sounds made by the APC
	float	m_flDangerSoundTime;

	// handbrake after the fact to keep vehicles from rolling
	float	m_flHandbrakeTime;
	bool	m_bInitialHandbrake;

	// Damage effects
	int		m_nSmokeTrailCount;
#ifdef DARKINTERVAL
	// Wheel Effects
	void	UpdateWheelFx(void);
	CHandle< CParticleSystem > m_hWheelDust[NUM_WHEEL_EFFECTS];
	CHandle< CParticleSystem > m_hWheelWater[NUM_WHEEL_EFFECTS];
	float		m_WheelFxTime[NUM_WHEEL_EFFECTS];
#endif
	// Machine gun attacks
	int		m_nMachineGunMuzzleAttachment;
	int		m_nMachineGunBaseAttachment;
	float	m_flMachineGunTime;
	int		m_iMachineGunBurstLeft;
	Vector	m_vecBarrelPos;
	float	m_aimYaw;
	float	m_aimPitch;
	bool	m_bInFiringCone;
#ifdef DARKINTERVAL
	// For accumulating accuracy penalty + allowing to shoot precisely in "semi-auto"
	float	m_accuracy_penalty_float;

	float	m_flRechargeTime;
	float	m_flChargeRemainder;
	float	m_flDrainRemainder;
	int		m_nGunState;
	int		m_nRocketState;
	float	m_flRocketStartAimTime;
	float	m_flRocketAimingTime;
	float	m_flRocketReloadTime;
	float	m_flThrustersTime;
	enum
	{
		GUN_STATE_IDLE = 0,
		GUN_STATE_FIRING,
	};

	enum
	{
		ROCKET_STATE_IDLE = 0,
		ROCKET_STATE_AIMING,
		ROCKET_STATE_READY,
		ROCKET_STATE_RELOAD,
	};
#endif // DARKINTERVAL
	// Rocket attacks
	EHANDLE	m_hLaserDot;
	EHANDLE m_hRocketTarget;
	float   m_flLaserTargetTime;
	int		m_iRocketSalvoLeft;
	float	m_flRocketTime;
	int		m_nRocketAttachment;
	int		m_nRocketSide;
	EHANDLE m_hSpecificRocketTarget;
	string_t m_strMissileHint;

	COutputEvent m_OnDeath;
	COutputEvent m_OnFiredMissile;
	COutputEvent m_OnDamaged;
	COutputEvent m_OnDamagedByPlayer;
#ifdef DARKINTERVAL // for repair stations
	COutputEvent m_OnFullHealth;
	COutputInt m_outputHealth_int;

	CSimTimer m_NextDenyTurretSoundTime;
	CSimTimer m_NextDenyRocketSoundTime;

	// Indicator sprites
	CSprite	*m_turretAmmoSprite; // sprite on side of the turret, orange when ammo < 50, red when < 15
	CSprite *m_cabinStandbySprite; // sprite to the right of the monitor, turns on when no driven and door open
	CSprite *m_repairStateSprite; // sprite to the right of the monitor, turns on when in repair mech
	CSimpleSimTimer	m_hullBlink_timer;
#endif
	DECLARE_DATADESC();
#ifdef DARKINTERVAL
protected:
	// Using these static variables for poseparameters
	// to save some memory. 
	static int m_poseparam_door, m_poseparam_headlight, m_poseparam_handbrake, m_poseparam_thruster, m_poseparam_gun_pitch, m_poseparam_gun_yaw;
	/*static*/ bool m_sbStaticPoseParamsLoaded;
	virtual void PopulatePoseParameters(void);
#endif
};

#endif // VEHICLE_APC_H

