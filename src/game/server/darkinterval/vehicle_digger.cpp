#include "cbase.h"
#include "trace.h"
#include "in_buttons.h"
#include "rumble_shared.h"
#include "vehicle_base.h"
#include "smoke_trail.h"
#include "soundent.h"
#include "soundenvelope.h"

#include "tier0/memdbgon.h"

static float blade_radius = 25;

class CDiggerFourWheelServerVehicle : public CFourWheelServerVehicle
{
	typedef CFourWheelServerVehicle BaseClass;
};

// Crusher damage-dealing spheres

// Debug sphere
class diggerBlade : public CBaseAnimating
{
	DECLARE_CLASS(diggerBlade, CBaseAnimating);
public:
	virtual bool OverridePropdata() { return true; }
	
	void Precache(void)
	{
		PrecacheModel(STRING(GetModelName()));
	}

	void Spawn(void)
	{
		BaseClass::Spawn();
		Precache();
		SetModel(STRING(GetModelName()));
		SetCollisionGroup(COLLISION_GROUP_INTERACTIVE);
		SetRenderMode(kRenderTransAdd);
		SetRenderColor(255, 25, 25, 0);
		m_takedamage = DAMAGE_NO;
		m_iHealth = m_iMaxHealth = 10000;
		CreateVPhysics();
	}

	bool CreateVPhysics(void)
	{
		SetSolid(SOLID_BBOX);
		IPhysicsObject *pPhysicsObject = VPhysicsInitShadow(false, false);

		if (!pPhysicsObject)
		{
			SetSolid(SOLID_NONE);
			SetMoveType(MOVETYPE_NONE);
			Warning("ERROR!: Can't create physics object for %s\n", STRING(GetModelName()));
		}

		return true;
	}
};

LINK_ENTITY_TO_CLASS(digger_blade, diggerBlade);
//-----------------------------------------------------------------------------
// Driveable Digger vehicle for Dark Interval
//-----------------------------------------------------------------------------
class CDigger : public CPropVehicleDriveable
{
	DECLARE_CLASS(CDigger, CPropVehicleDriveable);
	DECLARE_SERVERCLASS();
public:
	DECLARE_DATADESC();
	CDigger(void);
	// CBaseEntity
	virtual void	Precache(void);
	virtual void	Spawn(void);
	virtual void	Activate();
	virtual void	UpdateOnRemove(void);
	virtual void	OnRestore(void);
	void			Think(void);

	// CPropVehicle
	virtual void	CreateServerVehicle(void);
	virtual void	DriveVehicle(float flFrameTime, CUserCmd *ucmd, int iButtonsDown, int iButtonsReleased);
	virtual void	ProcessMovement(CBasePlayer *pPlayer, CMoveData *pMoveData);
	virtual Class_T	ClassifyPassenger(CBaseCombatCharacter *pPassenger, Class_T defaultClassification);
	virtual int		OnTakeDamage(const CTakeDamageInfo &info);
	virtual float	PassengerDamageModifier(const CTakeDamageInfo &info);
		
	virtual Vector	EyePosition();				// position of eyes
	Vector			BodyTarget(const Vector &posSrc, bool bNoisy);

	virtual void	Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	virtual void	EnterVehicle(CBaseCombatCharacter *pPassenger);
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
	CNetworkVar(bool, m_bHeadlightIsOn);

private:
	// Danger sounds made by the APC
	float	m_flDangerSoundTime;

	// handbrake after the fact to keep vehicles from rolling
	float	m_flHandbrakeTime;
	bool	m_bInitialHandbrake;
		
	// Rock crusher rotor setup
	float			m_crusherPosition;
	bool			m_crusherGoingUp;
	float			m_crusherNextExtendTime;
	bool			m_crusherRotating;
	float			m_crusherNextRotateTime;
	float			m_crusherRotateStartTime;
	float			m_crusherRotateStopTime;
	CSoundPatch		*m_crusherRotateSound;
	int				m_crusherRotateAmount;

	void			MoveCrusher(bool extend);
	void			RotateCrusher(void);
	void			StopCrusher(void);
	void			TraceCrusherDamage(void);

	CHandle <diggerBlade> bladePhysics;
//	CHandle <diggerBladeSphere> bladeSphereR;

	COutputEvent	m_OnDamaged;
	COutputEvent	m_OnDeath;
};

ConVar vehicle_digger_spin_rate("vehicle_digger_spin_rate", "10");
ConVar vehicle_digger_extend_speed("vehicle_digger_extend_speed", "0.05f");
ConVar vehicle_digger_crusher_dmg("vehicle_digger_crusher_dmg", "50");

CDigger::CDigger(void)
{
//	bladeSphereL = bladeSphereR = NULL;
	bladePhysics = NULL;
	m_bHasGun = false;
	m_crusherGoingUp = m_crusherRotating = false;
	m_crusherNextRotateTime = m_crusherNextExtendTime = m_crusherRotateStartTime = m_crusherRotateStopTime = 0;
}

void CDigger::Precache(void)
{
	PrecacheScriptSound("PropAPC.Headlight_on");
	PrecacheScriptSound("PropAPC.Headlight_off");
	PrecacheScriptSound("digger_grinder_start");
	PrecacheScriptSound("digger_grinder_stop");
	PrecacheScriptSound("digger_grinder_loop");
	PrecacheScriptSound("digger_extend_up");
	PrecacheScriptSound("digger_extend_down");
	PrecacheScriptSound("digger_crush_rock");
	PrecacheScriptSound("digger_crush_flesh");
	BaseClass::Precache();
}

void CDigger::Spawn(void)
{
	// Setup vehicle as a real-wheels car.
	SetVehicleType(VEHICLE_TYPE_CAR_WHEELS);

	BaseClass::Spawn();
	SetBlocksLOS(true);
	m_iHealth = m_iMaxHealth = 10000;
	SetCycle(0);
	m_lifeState = LIFE_ALIVE;

	m_flHandbrakeTime = CURTIME + 0.1;
	m_bInitialHandbrake = false;

	// Reset the gun to a default pose.
	SetPoseParameter("digger_crusher_extend", 0);
	SetPoseParameter("digger_crusher_rotate", 0);
	
	m_bHasGun = false;

	Vector bladePos;
	GetAttachment(LookupAttachment("digger_blades"), bladePos);
	bladePhysics = (diggerBlade*)CBaseEntity::CreateNoSpawn("digger_blade", bladePos, GetAbsAngles(), this); 
	bladePhysics->SetAbsOrigin(bladePos);
	bladePhysics->SetModelName(MAKE_STRING("models/_Tech/digger_blades.mdl"));
	bladePhysics->SetParent(this, LookupAttachment("digger_blades"));
	DispatchSpawn(bladePhysics);
	bladePhysics->Activate();
	bladePhysics->SetSolidFlags(FSOLID_NOT_SOLID);
/*
	Vector bladeL, bladeR;
	GetAttachment(LookupAttachment("digger_crusher_blade_L"), bladeL);
	GetAttachment(LookupAttachment("digger_crusher_blade_R"), bladeR);
	bladeSphereL = (diggerBladeSphere*)CBaseEntity::CreateNoSpawn("digger_blade", bladeL, GetAbsAngles(), this);
	bladeSphereR = (diggerBladeSphere*)CBaseEntity::CreateNoSpawn("digger_blade", bladeR, GetAbsAngles(), this);
	bladeSphereL->SetAbsOrigin(bladeL); 
	bladeSphereR->SetAbsOrigin(bladeR);
	bladeSphereL->SetModelName(MAKE_STRING("models/props_c17/barrel_regular.mdl"));
	bladeSphereR->SetModelName(MAKE_STRING("models/props_c17/barrel_regular.mdl"));
	bladeSphereL->SetParent(this, LookupAttachment("digger_crusher_blade_L"));
	bladeSphereR->SetParent(this, LookupAttachment("digger_crusher_blade_R"));
	DispatchSpawn(bladeSphereL);
	DispatchSpawn(bladeSphereR);
	bladeSphereL->Activate();
	bladeSphereR->Activate();
	bladeSphereL->SetSolidFlags(FSOLID_NOT_SOLID);
	bladeSphereR->SetSolidFlags(FSOLID_NOT_SOLID);
*/
}

void CDigger::Activate()
{
	BaseClass::Activate();

	CBaseServerVehicle *pServerVehicle = dynamic_cast<CBaseServerVehicle *>(GetServerVehicle());
	if (pServerVehicle)
	{
		if (pServerVehicle->GetPassenger())
		{
			// If a jeep comes back from a save game with a driver, make sure the engine rumble starts up.
			pServerVehicle->StartEngineRumble();
		}
	}
}

void CDigger::UpdateOnRemove(void)
{
	if (bladePhysics != NULL) UTIL_Remove(bladePhysics);
//	if (bladeSphereL != NULL) UTIL_Remove(bladeSphereL);
//	if (bladeSphereR != NULL) UTIL_Remove(bladeSphereR);
	BaseClass::UpdateOnRemove();
}

void CDigger::CreateServerVehicle(void)
{
	// Create our armed server vehicle
	m_pServerVehicle = new CDiggerFourWheelServerVehicle();
	m_pServerVehicle->SetVehicle(this);
}

Class_T	CDigger::ClassifyPassenger(CBaseCombatCharacter *pPassenger, Class_T defaultClassification)
{
	return CLASS_COMBINE;
}

float CDigger::PassengerDamageModifier(const CTakeDamageInfo &info)
{
	CTakeDamageInfo DmgInfo = info;

	// protect the player from physical impacts (that can happen FROM digger pushing stuff with its blades)
	if ((DmgInfo.GetDamageType() & DMG_CLUB) || (DmgInfo.GetDamageType() & DMG_GENERIC))
		return (0);

	// Accept everything else by default
	return 1.0;
}

Vector CDigger::EyePosition()
{
	Vector vecEyePosition;
	CollisionProp()->NormalizedToWorldSpace(Vector(0.5, 0.5, 1.0), &vecEyePosition);
	return vecEyePosition;
}

Vector CDigger::BodyTarget(const Vector &posSrc, bool bNoisy)
{
	Vector	shotPos;
	matrix3x4_t	matrix;

	int eyeAttachmentIndex = LookupAttachment("cannon_muzzle"); // FIXME
	GetAttachment(eyeAttachmentIndex, matrix);
	MatrixGetColumn(matrix, 3, shotPos);

	if (bNoisy)
	{
		shotPos[0] += random->RandomFloat(-8.0f, 8.0f);
		shotPos[1] += random->RandomFloat(-8.0f, 8.0f);
		shotPos[2] += random->RandomFloat(-8.0f, 8.0f);
	}

	return shotPos;
}

int CDigger::OnTakeDamage(const CTakeDamageInfo &info)
{
	if (m_iHealth == 0)
		return 0;

	m_OnDamaged.FireOutput(info.GetAttacker(), this);

	return 1;
}

void CDigger::ProcessMovement(CBasePlayer *pPlayer, CMoveData *pMoveData)
{
	BaseClass::ProcessMovement(pPlayer, pMoveData);

	if (m_flDangerSoundTime > CURTIME)
		return;

	QAngle vehicleAngles = GetLocalAngles();
	Vector vecStart = GetAbsOrigin();
	Vector vecDir, vecRight;

	GetVectors(&vecDir, &vecRight, NULL);

	GetVectors(&vecDir, NULL, NULL);

	const float soundDuration = 0.25;
	float speed = m_VehiclePhysics.GetHLSpeed();

	// Make danger sounds ahead of the jeep
	if (fabs(speed) > 120)
	{
		Vector	vecSpot;

		float steering = m_VehiclePhysics.GetSteering();
		if (steering != 0)
		{
			if (speed > 0)
			{
				vecDir += vecRight * steering * 0.5;
			}
			else
			{
				vecDir -= vecRight * steering * 0.5;
			}
			VectorNormalize(vecDir);
		}
		const float radius = speed * 0.4;

		// 0.3 seconds ahead of the jeep
		vecSpot = vecStart + vecDir * (speed * 1.1f);
		CSoundEnt::InsertSound(SOUND_DANGER | SOUND_CONTEXT_PLAYER_VEHICLE, vecSpot, radius, soundDuration, this, 0);
		CSoundEnt::InsertSound(SOUND_PHYSICS_DANGER | SOUND_CONTEXT_PLAYER_VEHICLE, vecSpot, radius, soundDuration, this, 1);
	}

	// Make engine sounds even when we're not going fast.
	CSoundEnt::InsertSound(SOUND_PLAYER | SOUND_CONTEXT_PLAYER_VEHICLE, GetAbsOrigin(), 800, soundDuration, this, 0);
	m_flDangerSoundTime = CURTIME + 0.1;
}

void CDigger::Think(void)
{
	BaseClass::Think();

	//	CBasePlayer	*pPlayer = UTIL_GetLocalPlayer();

	SetSimulationTime(CURTIME);
	SetNextThink(CURTIME);
	SetAnimatedEveryTick(true);

	if (!m_bInitialHandbrake)	// after initial timer expires, set the handbrake
	{
		m_bInitialHandbrake = true;
		m_VehiclePhysics.SetHandbrake(true);
		m_VehiclePhysics.Think();
	}

	// FIXME: Slam the crosshair every think -- if we don't do this it disappears randomly, never to return.
	if ((m_hPlayer.Get() != NULL) && !(m_bEnterAnimOn || m_bExitAnimOn))
	{
		m_hPlayer->m_Local.m_iHideHUD &= ~HIDEHUD_VEHICLE_CROSSHAIR;
	}
	
	StudioFrameAdvance();

	if (IsSequenceFinished() && (m_bExitAnimOn || m_bEnterAnimOn))
	{
		if (m_bEnterAnimOn)
		{
			m_VehiclePhysics.ReleaseHandbrake();
			StartEngine();

			// HACKHACK: This forces the jeep to play a sound when it gets entered underwater
			if (m_VehiclePhysics.IsEngineDisabled())
			{
				CBaseServerVehicle *pServerVehicle = dynamic_cast<CBaseServerVehicle *>(GetServerVehicle());
				if (pServerVehicle)
				{
					pServerVehicle->SoundStartDisabled();
				}
			}

			// The first few time we get into the jeep, print the jeep help
			//	if (m_iNumberOfEntries < hud_jeephint_numentries.GetInt())
			//	{
			//		g_EventQueue.AddEvent(this, "ShowHudHint", 1.5f, this, this);
			//	}
		}
		
		// Reset on exit anim
		GetServerVehicle()->HandleEntryExitFinish(m_bExitAnimOn, m_bExitAnimOn);
	}

	if (m_debugOverlays & OVERLAY_NPC_KILL_BIT)
	{
		CTakeDamageInfo info(this, this, m_iHealth, DMG_BLAST);
		info.SetDamagePosition(WorldSpaceCenter());
		info.SetDamageForce(Vector(0, 0, 1));
		TakeDamage(info);
	}

	if (m_crusherRotating)
	{
		float spin_time = CURTIME - m_crusherRotateStartTime;
		if (spin_time > 1.0f) spin_time = 1.0f;
		float spin_rate = vehicle_digger_spin_rate.GetInt() * spin_time;
		m_crusherRotateAmount += spin_rate;
		SetPoseParameter("digger_crusher_rotate", m_crusherRotateAmount);
		TraceCrusherDamage();
	}
	else
	{
		float stop_time = m_crusherRotateStopTime - CURTIME;
		if (stop_time < 0.0f) stop_time = 0.0f;
		float stop_rate = vehicle_digger_spin_rate.GetInt() * stop_time;
		m_crusherRotateAmount += stop_rate;
		SetPoseParameter("digger_crusher_rotate", m_crusherRotateAmount);
	}

	if (m_crusherGoingUp)
	{
		m_crusherPosition += vehicle_digger_extend_speed.GetFloat();
	}
	else
	{
		m_crusherPosition -= vehicle_digger_extend_speed.GetFloat();
	}

	if (m_crusherPosition < 0) m_crusherPosition = 0;
	if (m_crusherPosition > 1) m_crusherPosition = 1;

	// Since this poseparameter doesn't wrap, we can always send in the value
	SetPoseParameter("digger_crusher_extend", m_crusherPosition);

//	if (bladeSphereL != NULL)
//	{
	//	if (!(bladeSphereL->IsSolidFlagSet(FSOLID_NOT_SOLID)))
	//		NDebugOverlay::Sphere(bladeSphereL->WorldSpaceCenter(), blade_radius, 255, 100, 100, false, 0.05f);
//	}

//	if (bladeSphereR != NULL)
//	{
	//	if (!(bladeSphereR->IsSolidFlagSet(FSOLID_NOT_SOLID)))
	//		NDebugOverlay::Sphere(bladeSphereR->WorldSpaceCenter(), blade_radius, 255, 100, 100, false, 0.05f);
//	}
}

void CDigger::MoveCrusher(bool extend)
{
	if (extend)
	{
		if (m_crusherNextExtendTime > CURTIME)
			return;

		m_crusherGoingUp = true;
		return;
	}
	else
	{
		m_crusherGoingUp = false;
		m_crusherNextExtendTime = CURTIME + 0.5f;
	}
}

void CDigger::RotateCrusher(void)
{
	//Don't fire again if it's been too soon
	if (m_crusherNextRotateTime > CURTIME)
		return;

	//See if we're starting a charge
	if (m_crusherRotating == false)
	{
		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "digger_grinder_start", NULL);
		m_crusherRotateStartTime = CURTIME;
		m_crusherRotating = true;

		//Start charging sound
		m_crusherRotateSound = (CSoundEnvelopeController::GetController()).SoundCreate(filter, entindex(), CHAN_STATIC, "digger_grinder_loop", ATTN_NORM);

		if (m_hPlayer)
		{
			m_hPlayer->RumbleEffect(RUMBLE_FLAT_LEFT, (int)(0.1 * 100), RUMBLE_FLAG_RESTART | RUMBLE_FLAG_LOOP | RUMBLE_FLAG_INITIAL_SCALE);
		}

		assert(m_crusherRotateSound != NULL);
		if (m_crusherRotateSound != NULL)
		{
			(CSoundEnvelopeController::GetController()).Play(m_crusherRotateSound, 1.0f, 100, CURTIME + 1.0f);
		}

		if (bladePhysics != NULL) bladePhysics->RemoveSolidFlags(FSOLID_NOT_SOLID);
/*
		if (bladeSphereL != NULL)
		{
			bladeSphereL->RemoveSolidFlags(FSOLID_NOT_SOLID);
		}

		if (bladeSphereR != NULL)
		{
			bladeSphereR->RemoveSolidFlags(FSOLID_NOT_SOLID);
		}
*/
		return;
	}
	else
	{
		float rotateAmount = (CURTIME - m_crusherRotateStartTime) / 0.5f;
		if (rotateAmount > 1.0f)
		{
			rotateAmount = 1.0f;
		}

		float rumble = rotateAmount * 0.5f;

		if (m_hPlayer)
		{
			m_hPlayer->RumbleEffect(RUMBLE_FLAT_LEFT, (int)(rumble * 100), RUMBLE_FLAG_UPDATE_SCALE);
		}
	}

	//TODO: Add muzzle effect?

	//TODO: Check for overcharge and have the weapon simply fire or instead "decharge"?
}

void CDigger::StopCrusher(void)
{
	if (m_crusherRotating)
	{
		CPASAttenuationFilter filter(this);
		EmitSound(filter, entindex(), "digger_grinder_stop", NULL);
	}
	m_crusherRotating = false;
	m_crusherNextRotateTime = CURTIME + 1.1f;

	if (m_crusherRotateSound != NULL)
	{
		(CSoundEnvelopeController::GetController()).SoundFadeOut(m_crusherRotateSound, 1.0f);
	}

	if (m_hPlayer)
	{
		m_hPlayer->RumbleEffect(RUMBLE_FLAT_LEFT, 0, RUMBLE_FLAG_STOP);
	}

	if (bladePhysics != NULL) bladePhysics->SetSolidFlags(FSOLID_NOT_SOLID);
/*
	if (bladeSphereL != NULL)
	{
		bladeSphereL->SetSolidFlags(FSOLID_NOT_SOLID);
	}

	if (bladeSphereR != NULL)
	{
		bladeSphereR->SetSolidFlags(FSOLID_NOT_SOLID);
	}
*/
}

void CDigger::TraceCrusherDamage(void)
{
	if (!GetDriver()) return;
	Vector bladeL, bladeR;
	GetAttachment(LookupAttachment("digger_crusher_blade_L"), bladeL);
	GetAttachment(LookupAttachment("digger_crusher_blade_R"), bladeR);

	trace_t tr;
	CTraceFilterSkipTwoEntities filter(this, bladePhysics, COLLISION_GROUP_NONE);	
	UTIL_TraceHull(bladeL, bladeR,
		-Vector(0, blade_radius, blade_radius ), 
		Vector(0, blade_radius, blade_radius),
		MASK_SHOT, &filter, &tr);

//	NDebugOverlay::SweptBox(tr.startpos, tr.endpos,
//		-Vector(0, blade_radius, blade_radius),
//		Vector(0, blade_radius, blade_radius),
//		GetAbsAngles(), 255, 50, 50, 50, 0.05f);

	if (tr.fraction != 1.0)
	{
		if (tr.m_pEnt && tr.m_pEnt->m_takedamage == DAMAGE_YES)
		{
			CTakeDamageInfo cInfo;
			cInfo.SetDamageType(DMG_CRUSH | DMG_SLASH | DMG_ALWAYSGIB);
			cInfo.SetDamage(vehicle_digger_crusher_dmg.GetFloat());
			cInfo.ScaleDamageForce(10.0f);
			cInfo.SetAttacker(GetDriver());
			cInfo.SetInflictor(GetDriver());
			ClearMultiDamage();
			tr.m_pEnt->DispatchTraceAttack(cInfo, GetAbsOrigin() - tr.m_pEnt->GetAbsOrigin(), &tr);
		}
	}
}

void CDigger::DriveVehicle(float flFrameTime, CUserCmd *ucmd, int iButtonsDown, int iButtonsReleased)
{
	switch (m_lifeState)
	{
	case LIFE_ALIVE:
	{
		if (ucmd->impulse == 100)
		{
			if (m_bHeadlightIsOn)
				EmitSound("PropAPC.Headlight_off");
			else
				EmitSound("PropAPC.Headlight_on");
			m_bHeadlightIsOn ^= true;
		}

		int iButtons = ucmd->buttons;

		if (iButtons & IN_ATTACK2)
		{
			MoveCrusher(true);
		}
		
		if (iButtons & IN_ATTACK)
		{
			RotateCrusher();
		}

		// If we've released our primary button, lower the rotor
		if ((iButtonsReleased & IN_ATTACK2) /*&& (m_crusherGoingUp)*/)
		{
			MoveCrusher(false);
		}

		// If we've released our secondary button, stop the rotor
		if ((iButtonsReleased & IN_ATTACK) && (m_crusherRotating))
		{
			m_crusherRotateStopTime = CURTIME + 1.0f;
			StopCrusher();
		}
	}
	break;

	case LIFE_DYING:
		break;

	case LIFE_DEAD:
		return;
	}

	BaseClass::DriveVehicle(flFrameTime, ucmd, iButtonsDown, iButtonsReleased);
}

void CDigger::Use(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	BaseClass::Use(pActivator, pCaller, useType, value);
}

void CDigger::EnterVehicle(CBaseCombatCharacter *pPassenger)
{
	CBasePlayer *pPlayer = ToBasePlayer(pPassenger);
	if (!pPlayer)
		return;

	// Play the engine start sound.
	float flDuration;
	EmitSound("Airboat_engine_start", 0.0, &flDuration);
	m_VehiclePhysics.TurnOn();

	// Start playing the engine's idle sound as the startup sound finishes.
	//	m_flEngineIdleTime = CURTIME + flDuration - 0.1;

	BaseClass::EnterVehicle(pPassenger);
}

void CDigger::ExitVehicle(int nRole)
{
	StopCrusher();
	m_crusherGoingUp = false;
	m_crusherNextExtendTime = CURTIME + 0.5f;
	m_crusherRotating = false;
	//	HeadlightTurnOff();
	BaseClass::ExitVehicle(nRole);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CDigger::OnRestore(void)
{
	BaseClass::OnRestore();
	IServerVehicle *pServerVehicle = GetServerVehicle();
	if (pServerVehicle != NULL)
	{
		// Restore the passenger information we're holding on to
		pServerVehicle->RestorePassengerInfo();
	}
}

BEGIN_DATADESC(CDigger)
DEFINE_FIELD(m_crusherPosition, FIELD_FLOAT),
DEFINE_FIELD(m_crusherGoingUp, FIELD_BOOLEAN),
DEFINE_FIELD(m_crusherNextExtendTime, FIELD_TIME),
DEFINE_FIELD(m_crusherRotating, FIELD_BOOLEAN),
DEFINE_FIELD(m_crusherNextRotateTime, FIELD_TIME),
DEFINE_FIELD(m_crusherRotateStartTime, FIELD_TIME),
DEFINE_SOUNDPATCH(m_crusherRotateSound),
DEFINE_FIELD(bladePhysics, FIELD_EHANDLE),
//DEFINE_FIELD(bladeSphereL, FIELD_EHANDLE),
//DEFINE_FIELD(bladeSphereR, FIELD_EHANDLE),
DEFINE_FIELD(m_crusherRotateAmount, FIELD_INTEGER),
DEFINE_FIELD(m_flDangerSoundTime, FIELD_TIME),
DEFINE_FIELD(m_flHandbrakeTime, FIELD_TIME),
DEFINE_FIELD(m_bInitialHandbrake, FIELD_BOOLEAN),
DEFINE_FIELD(m_bHeadlightIsOn, FIELD_BOOLEAN),
DEFINE_OUTPUT(m_OnDamaged, "OnDamaged"),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CDigger, DT_Digger)
	SendPropBool(SENDINFO(m_bHeadlightIsOn)),
END_SEND_TABLE();

LINK_ENTITY_TO_CLASS(prop_vehicle_digger, CDigger);