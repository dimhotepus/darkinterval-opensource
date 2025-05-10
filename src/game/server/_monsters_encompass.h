#ifndef _MONSTERS_ENCOMPASS_H
#define _MONSTERS_ENCOMPASS_H
#include "basecombatcharacter.h"
#include "npc_basescanner.h"

// this is here so env gravity vortex can have a pointer back to pstorms
class CTesla;
class CEnvGravityVortex;

class CNPC_ParticleStorm : public CNPC_BaseScanner
{
public:

	DECLARE_CLASS(CNPC_ParticleStorm, CNPC_BaseScanner);
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;
	//	DECLARE_SERVERCLASS();

	CNPC_ParticleStorm();
	~CNPC_ParticleStorm();

	void			Precache(void);
	void			Spawn(void);
	void			Activate();
	Disposition_t	IRelationType(CBaseEntity *pTarget);

	Class_T Classify() { return CLASS_COMBINE; }

	float			GetMaxSpeed(void) { return 500.0f; }
	int				GetSoundInterests(void) { return ( SOUND_WORLD | SOUND_COMBAT | SOUND_PLAYER | SOUND_DANGER ); }
	char			*GetEngineSound(void) { return "NPC_PStorm.FlyLoop"; }
	bool			FValidateHintType(CAI_Hint *pHint);
	
	float			MinGroundDist(void) { return 32.0f; }
	void			NPCThink(void);

	int				OnTakeDamage_Alive(const CTakeDamageInfo &info);
	//	void			Event_Killed(const CTakeDamageInfo &info);
	//	void			UpdateOnRemove(void);

	//	Activity		NPC_TranslateActivity(Activity eNewActivity);
	float			InnateRange1MinRange(void) { return 350.0f; }
	float			InnateRange1MaxRange(void) { return 800.0f; }
	int				RangeAttack1Conditions(float flDot, float flDist);
	//	void			GatherConditions(void);
	void			PrescheduleThink(void);
	void			LaunchProjectile(CBaseEntity *pEnemy);
	void			StartTask(const Task_t *pTask);
	void			RunTask(const Task_t *pTask);
	int				SelectSchedule(void);
	virtual int		TranslateSchedule(int scheduleType);
	void			SpeakSentence(int nSentenceType);

	void			SetPlayerIsCaught(bool isCaught)
	{
		m_player_caught_bool = isCaught;
		m_OnPlayerCaughtInVortex.FireOutput(this, this);
	};

public:
	CHandle<CEnvGravityVortex> m_vortex_controller_handle;
	CHandle<CBaseFilter>	m_vortex_filter_handle;
	string_t				m_vortex_filter_name;

private:
	virtual float	GetGoalDistance(void);
	//	virtual bool	OverridePathMove(CBaseEntity *pMoveTarget, float flInterval);
	virtual bool	OverrideMove(float flInterval);
	virtual void	MoveToAttack(float flInterval);
	virtual void	MoveExecute_Alive(float flInterval);

	void			ClampMotorForces(Vector &linear, AngularImpulse &angular);

	//	void			AttackPreShock(void);
	//	void			AttackShock(void);

	//	CAI_Sentence< CNPC_ParticleStorm > m_Sentences;
	bool			m_bNoLight;
	bool			m_bDissipating;
	float			m_flDissipateTime;
	float			m_flFormTime;
	bool			m_pstorm_dormant_bool;
	bool			m_enable_projectiles_bool;
	float			m_z_acceleration_float;
	float			m_z_threshold_float;
	float			m_vortex_inner_radius_float;
	float			m_vortex_outer_radius_float;
	bool			m_player_caught_bool;
	bool			m_enabledlights_bool;

private:

	CHandle<CTesla>	m_hPseudopodia;
	void			UpdatePseudopodia(void);
	void			BeginDissipation(void);
	void			FinishDissipation(void);
	void			InputDissipate(inputdata_t &inputdata);
	void			DynamicLightsThink(void);
#if 0
	void			CreateDynamicLights(void);
#endif
	void			KillDynamicLights(void);
	void			BeginForming(void);
	void			FinishForming(void);
	void			InputStartVortex(inputdata_t &inputdata);
	void			InputStopVortex(inputdata_t &inputdata);
	void			InputSetVortexInnerRadius(inputdata_t &inputdata);
	void			InputSetVortexOuterRadius(inputdata_t &inputdata);
	void			InputSetVortexFilter(inputdata_t &inputdata);
	void			InputForm(inputdata_t &inputdata);
	void			InputEnableProjectiles(inputdata_t &inputdata);
	void			InputDisableProjectiles(inputdata_t &inputdata);
	void			InputSetZAccelerationFactor(inputdata_t &inputdata)
	{
		m_z_acceleration_float = inputdata.value.Float();
	}
	void			InputSetZThresholdFactor(inputdata_t &inputdata)
	{
		m_z_threshold_float = inputdata.value.Float();
	}

	Vector			m_dynamiclightsColor;

	COutputEvent	m_OnFinishedForming;
	COutputEvent	m_OnFinishedDissipation;
	COutputEvent	m_OnStartVortexAction;
	COutputEvent	m_OnEndVortexAction;
	COutputEvent	m_OnPlayerCaughtInVortex;

	virtual void	StopLoopingSounds(void);

	static int attachmentParticleCenter;
	static int attachmentParticleCp1;
	static int attachmentParticleCp2;
	static int attachmentParticleCp3;
	static int attachmentParticleCp4;
	static int attachmentParticleCp5;
	static int attachmentParticleCp6;
	static int attachmentParticleCp7;
	static int attachmentParticleCp8;
	static int attachmentParticleCp9;
	static int attachmentParticleCp10;
	static int attachmentParticleCp11;
	static int attachmentParticleCp12;
	static int attachmentParticleCp13;

	/*
	private:
	NPC_FILE_INFO_HANDLE	m_hNPCFileInfo;
	const FileNPCInfo_t &CNPC_ParticleStorm::GetNPCScriptData(void) const
	{
	return *GetFileNPCInfoFromHandle(m_hNPCFileInfo);
	}
	*/

	// Custom schedules
	enum PStormSchedules
	{
		SCHED_PSTORM_ATTACK_HOVER = BaseClass::NEXT_SCHEDULE + 100,
		SCHED_PSTORM_PROJECTILE_ATTACK,
	};

	// Custom tasks
	enum PStormTasks
	{
		TASK_PSTORM_PROJECTILE_ATTACK = BaseClass::NEXT_TASK,
	};
};
#endif