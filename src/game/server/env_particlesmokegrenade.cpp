//========================================================================//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#include "cbase.h"
#include "env_particlesmokegrenade.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_SERVERCLASS_ST(ParticleSmokeGrenade, DT_ParticleSmokeGrenade)
	SendPropTime(SENDINFO(m_flSpawnTime) ),
	SendPropFloat(SENDINFO(m_FadeStartTime), 0, SPROP_NOSCALE),
	SendPropFloat(SENDINFO(m_FadeEndTime), 0, SPROP_NOSCALE),
#ifdef DARKINTERVAL
	SendPropFloat(SENDINFO(m_ExpandTime), 0, SPROP_NOSCALE),
	SendPropFloat(SENDINFO(m_VolumeEmissionRate), 0, SPROP_NOSCALE),
	SendPropVector(SENDINFO(m_VolumeColor), -1, SPROP_NOSCALE),
	SendPropVector(SENDINFO(m_VolumeDimensions), -1, SPROP_NOSCALE),
#endif
	SendPropInt(SENDINFO(m_CurrentStage), 1, SPROP_UNSIGNED),
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( env_particlesmokegrenade, ParticleSmokeGrenade );

BEGIN_DATADESC( ParticleSmokeGrenade )

	DEFINE_FIELD( m_CurrentStage, FIELD_CHARACTER ),
	DEFINE_FIELD( m_FadeStartTime, FIELD_TIME ),
	DEFINE_FIELD( m_FadeEndTime, FIELD_TIME ),
	DEFINE_FIELD( m_flSpawnTime, FIELD_TIME ),
#ifdef DARKINTERVAL
	DEFINE_FIELD( m_ExpandTime, FIELD_TIME),
	DEFINE_FIELD( m_VolumeEmissionRate, FIELD_TIME),
	DEFINE_FIELD( m_VolumeColor, FIELD_VECTOR),
	DEFINE_FIELD( m_VolumeDimensions, FIELD_VECTOR),
#endif
END_DATADESC()


ParticleSmokeGrenade::ParticleSmokeGrenade()
{
	m_CurrentStage = 0;
	m_FadeStartTime = 17;
	m_FadeEndTime = 22;
#ifdef DARKINTERVAL
	m_ExpandTime = 1.0f;
	m_VolumeEmissionRate = 60.0f;
	m_VolumeColor = Vector(0.50, 0.50, 0.50);
	m_VolumeDimensions = Vector(100, 60, 60);
#endif
	m_flSpawnTime = CURTIME;
}


// Smoke grenade particles should always transmitted to clients.  If not, a client who
// enters the PVS late will see the smoke start billowing from then, allowing better vision.
int ParticleSmokeGrenade::UpdateTransmitState( void )
{
	if ( IsEffectActive( EF_NODRAW ) )
		return SetTransmitState( FL_EDICT_DONTSEND );

	return SetTransmitState( FL_EDICT_ALWAYS );
}


void ParticleSmokeGrenade::FillVolume()
{
	m_CurrentStage = 1;
	CollisionProp()->SetCollisionBounds( Vector( -50, -50, -50 ), Vector( 50, 50, 50 ) );
}


void ParticleSmokeGrenade::SetFadeTime(float startTime, float endTime)
{
	m_FadeStartTime = startTime;
	m_FadeEndTime = endTime;
}
#ifdef DARKINTERVAL
void ParticleSmokeGrenade::SetExpandTime(float expandTime)
{
	m_ExpandTime = expandTime;
}

void ParticleSmokeGrenade::SetVolumeEmissionRate(float volumeEmission)
{
	m_VolumeEmissionRate = volumeEmission;
}

void ParticleSmokeGrenade::SetVolumeColour(Vector volumeColor)
{
	m_VolumeColor = volumeColor;
}

void ParticleSmokeGrenade::SetVolumeSize(Vector volumeDimensions)
{
	m_VolumeDimensions = volumeDimensions;
}
#endif
// Fade start and end are relative to current time
void ParticleSmokeGrenade::SetRelativeFadeTime(float startTime, float endTime)
{
	float flCurrentTime = CURTIME - m_flSpawnTime;

	m_FadeStartTime = flCurrentTime + startTime;
	m_FadeEndTime = flCurrentTime + endTime;
}
