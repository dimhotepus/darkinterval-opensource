#ifndef PLAYER_CONTROL_H
#define PLAYER_CONTROL_H
#ifdef _WIN32
#pragma once
#endif
#include "basecombatcharacter.h"
#include "ai_basenpc.h"
#include "ai_basenpc_flyer_new.h"
#include "soundent.h"
#include "func_break.h"		// For materials
#include "hl2_player.h"
#include "in_buttons.h"
#include "ai_hull.h"
#include "decals.h"
#include "env_shake.h"
#include "ieffects.h"
#include "te_effect_dispatch.h"
#include "ndebugoverlay.h"
#include "engine/ienginesound.h"

#define PMANHACK_MAX_SPEED	350

CSoundPatch *m_sndEngineSound;

class CPlayer_Manhack : public CAI_BaseNPCFlyerNew
{
public:
	DECLARE_CLASS(CPlayer_Manhack, CAI_BaseNPCFlyerNew);
	CPlayer_Manhack();
	~CPlayer_Manhack();
	Class_T Classify(void) { return CLASS_MANHACK; }
	void ControlActivate(void);
	void ControlDeactivate(void);
	void Precache(void);
	void Spawn(void);
	bool CreateVPhysics(void);
	void OnRestore();

	void InputActivate(inputdata_t &inputdata);
	void InputDeactivate(inputdata_t &inputdata);
	void InputSetThrust(inputdata_t &inputdata);
	void InputSetSideThrust(inputdata_t &inputdata);

	COutputEvent		m_OnExitReached;
	COutputEvent		m_OnActivated;
	COutputEvent		m_OnDeactivated;

	void FlyThink(void);
	void CheckDamageSphere(float fRadius, Vector vFacing );
	void CheckBladeTrace(trace_t &tr);
//	void RunAI(void);
	virtual float GetMaxSpeed(void) { return PMANHACK_MAX_SPEED; }
	virtual void MoveToTarget(float flInterval, const Vector &vecMoveTarget) { return; }

	void DrawHUD(void);
	int UpdateTransmitState(void);
	bool m_bActive;

	DECLARE_DATADESC();

private:
	int				m_nSaveFOV;
	CBaseEntity		*exitTarget;
	EHANDLE			m_bladeModel;
};

#endif