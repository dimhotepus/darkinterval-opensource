//========================================================================//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//

#ifndef PARTICLE_SMOKEGRENADE_H
#define PARTICLE_SMOKEGRENADE_H


#include "baseparticleentity.h"


#define PARTICLESMOKEGRENADE_ENTITYNAME	"env_particlesmokegrenade"


class ParticleSmokeGrenade : public CBaseParticleEntity
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS( ParticleSmokeGrenade, CBaseParticleEntity );
	DECLARE_SERVERCLASS();

						ParticleSmokeGrenade();

	virtual int			UpdateTransmitState( void );

public:

	// Tell the client entity to start filling the volume.
	void				FillVolume();

	// Set the times it fades out at.
	void				SetFadeTime(float startTime, float endTime);
#ifdef DARKINTERVAL
	// Set the expansion time (originally clamped at 1 second or less)
	void				SetExpandTime(float expandTime);

	// Set the colour to be passed onto clientside
	void				SetVolumeColour(Vector volumeColor);

	// Regulate the size (given as full dimensions, diameter + height)
	void				SetVolumeSize(Vector volumeDimensions);

	// Set emission rate to be passed onto clientside
	void				SetVolumeEmissionRate(float volumeEmission);
#endif
	// Set time to fade out relative to current time
	void				SetRelativeFadeTime(float startTime, float endTime);


public:
	
	// Stage 0 (default): make a smoke trail that follows the entity it's following.
	// Stage 1          : fill a volume with smoke.
	CNetworkVar( unsigned char, m_CurrentStage );

	CNetworkVar( float, m_flSpawnTime );

	// When to fade in and out.
	CNetworkVar( float, m_FadeStartTime );
	CNetworkVar( float, m_FadeEndTime );
	CNetworkVar( float, m_ExpandTime);

	// Colour, size and emission rate
	CNetworkVar( Vector, m_VolumeColor);
	CNetworkVar( Vector, m_VolumeDimensions);
	CNetworkVar( float,	m_VolumeEmissionRate);
};


#endif


