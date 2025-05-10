#ifndef ENV_GRAVITY_VORTEX_H
#define ENV_GRAVITY_VORTEX_H
#include "cbase.h"
#include "basegrenade_shared.h"

// This is used to pull player and debris inside the pstorm.
// Debris gets removed, player reaches the center and enters a special trigger.
enum VortexFlags_t
{
	VORTEX_PROP_PULL	= 0x00000001,
	VORTEX_PROP_KILL	= 0x00000002,
	VORTEX_NPC_PULL		= 0x00000004,
	VORTEX_NPC_KILL		= 0x00000008,
	VORTEX_PLAYER_PULL	= 0x00000010,
	VORTEX_PLAYER_KILL	= 0x00000020,
	VORTEX_STOP_REMOVE	= 0x00000040,
};

class CEnvGravityVortex : public CBaseAnimating
{
	DECLARE_CLASS(CEnvGravityVortex, CBaseAnimating);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
public:

	CEnvGravityVortex(void) : m_end_time(10.0f), m_inner_radius_float(64), m_outer_radius_float(512), 
		m_strength_float(256), m_mass_float(0.0f), m_pull_gravity_scale_float(1.0f)
	{
		m_vortex_flags_bit |= VORTEX_PROP_PULL | VORTEX_PROP_KILL;
	}
	float	GetConsumedMass(void) const;

	static CEnvGravityVortex *Create(const Vector &origin, float innerRadius, float outerRadius, float strength, float duration, CBaseEntity *pOwner);

	void	Precache(void);
	void	Spawn(void);

	void	SetVortexInnerRadius(float inner) { m_inner_radius_float = inner; };
	void	SetVortexOuterRadius(float outer) { m_outer_radius_float = outer; };
	void	SetVortexDuration(float duration) { m_duration_float = duration; };
	void	SetVortexStrength(float strength) { m_strength_float = strength; };
	void	SetVortexMaxMass(float mass)	  { m_maxmass_float = mass; };
	void	SetVortexDensePropLimit(int limit)	  { m_dense_prop_limit_int = limit; };
	void	SetVortexPlayerPull(bool enable)
	{ 
		if ( enable ) m_vortex_flags_bit |= VORTEX_PLAYER_PULL;
		else m_vortex_flags_bit &= ~VORTEX_PLAYER_PULL;
	};
	void	SetVortexPlayerKill(bool enable)
	{
		if ( enable ) m_vortex_flags_bit |= VORTEX_PLAYER_KILL;
		else m_vortex_flags_bit &= ~VORTEX_PLAYER_KILL;
	};
	void	SetVortexPropPull(bool enable)
	{
		if ( enable ) m_vortex_flags_bit |= VORTEX_PROP_PULL;
		else m_vortex_flags_bit &= ~VORTEX_PROP_PULL;
	};
	void	SetVortexPropKill(bool enable)
	{
		if ( enable ) m_vortex_flags_bit |= VORTEX_PROP_KILL;
		else m_vortex_flags_bit &= ~VORTEX_PROP_KILL;
	};
	void	SetVortexNPCPull(bool enable)
	{
		if ( enable ) m_vortex_flags_bit |= VORTEX_NPC_PULL;
		else m_vortex_flags_bit &= ~VORTEX_NPC_PULL;
	};
	void	SetVortexNPCKill(bool enable)
	{
		if ( enable ) m_vortex_flags_bit |= VORTEX_NPC_KILL;
		else m_vortex_flags_bit &= ~VORTEX_NPC_KILL;
	};
	void	SetVortexRemoveOnStop(bool enable)
	{
		if ( enable ) m_vortex_flags_bit |= VORTEX_STOP_REMOVE;
		else m_vortex_flags_bit &= ~VORTEX_STOP_REMOVE;
	};
	void	SetVortexGravityScale(float scale) { m_pull_gravity_scale_float = scale; };

	CHandle<CBaseFilter> m_filter_handle;
	void	SetVortexPullFilter(CBaseFilter *filter) { m_filter_handle = filter; };
private:
	void	InputTurnOn(inputdata_t &inputdata);
	void	InputTurnOff(inputdata_t &inputdata);
	void	InputSetVortexInnerRadius(inputdata_t &inputdata);
	void	InputSetVortexOuterRadius(inputdata_t &inputdata);
	void	InputSetVortexDuration(inputdata_t &inputdata);
	void	InputSetVortexStrength(inputdata_t &inputdata);
	void	InputSetVortexMaxMass(inputdata_t &inputdata);
	void	InputSetVortexPlayerPull(inputdata_t &inputdata);
	void	InputSetVortexPlayerKill(inputdata_t &inputdata);
	void	InputSetVortexPropPull(inputdata_t &inputdata);
	void	InputSetVortexPropKill(inputdata_t &inputdata);
	void	InputSetVortexNPCPull(inputdata_t &inputdata);
	void	InputSetVortexNPCKill(inputdata_t &inputdata);
	void	InputSetVortexRemoveOnStop(inputdata_t &inputdata);
	void	InputSetVortexDensePropLimit(inputdata_t &inputdata);
	void	InputSetVortexGravityScale(inputdata_t &inputdata);

	void	ConsumeEntity(CBaseEntity *pEnt);
	void	PullPlayersInRange(void);
	bool	KillNPCInRange(CBaseEntity *pVictim, IPhysicsObject **pPhysObj);
	void	CreateDenseProp(void);
	void	PullThink(void);
	void	StartPull(const Vector &origin, float innerRadius, float outerRadius, float strength, float duration);
	void	StopPull(bool doFx);

	float	m_mass_float;		// Mass consumed by the vortex
	float	m_maxmass_float;		// Mass at which dense prop should be produced
	float	m_end_time;	// Time when the vortex will stop functioning
	float	m_inner_radius_float;	// Inner radius of the vortex, where things disappear
	float	m_outer_radius_float;	// Area of effect for the vortex
	float	m_strength_float;	// Pulling strength of the vortex
	float	m_duration_float;	// used when spawned by a mapper
	bool	m_enabled_bool; // used when spawned by a mapper
	int		m_dense_prop_limit_int; // at 0, it will never be produced, at -1, it'll be produced whenever the mass goes over the threshold, then gets reset
	int		m_vortex_flags_bit; // controls what types of objects the vortex can pull in and destroy
	bool	m_negate_inner_forces_bool = 1; // if 1, objects that cross the inner radius experience the opposite pull force
	float	m_pull_gravity_scale_float; // scaled pulled objects' gravity by this factor
};

#endif // ENV_GRAVITY_VORTEX_H