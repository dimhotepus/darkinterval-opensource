//========================================================================//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef GRENADE_FRAG_H
#define GRENADE_FRAG_H
#pragma once

class CBaseGrenade;
struct edict_t;
#ifdef DARKINTERVAL
enum
{
	FRAGGRENADE_TYPE_DEFAULT,
	FRAGGRENADE_TYPE_SMOKE,
};
#endif
CBaseGrenade *Fraggrenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, bool combineSpawned
#ifdef DARKINTERVAL
	, int grenadeType = FRAGGRENADE_TYPE_DEFAULT, float nextBlipTime = CURTIME 
#endif
);
bool	Fraggrenade_WasPunted( const CBaseEntity *pEntity );
bool	Fraggrenade_WasCreatedByCombine( const CBaseEntity *pEntity );

#endif // GRENADE_FRAG_H
