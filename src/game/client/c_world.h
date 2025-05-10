//========================================================================//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#if !defined( C_WORLD_H )
#define C_WORLD_H
#ifdef _WIN32
#pragma once
#endif

#include "c_baseentity.h"

#if defined( CLIENT_DLL )
#define CWorld C_World
#endif

class C_World : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_World, C_BaseEntity );
	DECLARE_CLIENTCLASS();

	C_World( void );
	~C_World( void );
	
	// Override the factory create/delete functions since the world is a singleton.
	virtual bool Init( int entnum, int iSerialNum );
	virtual void Release();

	virtual void Precache();
	virtual void Spawn();

	// Don't worry about adding the world to the collision list; it's already there
	virtual CollideType_t	GetCollideType( void )	{ return ENTITY_SHOULD_NOT_COLLIDE; }

	virtual void OnDataChanged( DataUpdateType_t updateType );
	virtual void PreDataUpdate( DataUpdateType_t updateType );

	float GetWaveHeight() const;
	const char *GetDetailSpriteMaterial() const;
#ifdef DARKINTERVAL
	float GetDetailDist() const { return m_flDetailDist; }
#endif
public:
	enum
	{
		MAX_DETAIL_SPRITE_MATERIAL_NAME_LENGTH = 256,
	};

	float	m_flWaveHeight;
	Vector	m_WorldMins;
	Vector	m_WorldMaxs;
	bool	m_bStartDark;
#ifdef DARKINTERVAL
	Vector	cStartDarkColor; // customisable start fade
	float   m_flStartDarkTime;
#endif
	float	m_flMaxOccludeeArea;
	float	m_flMinOccluderArea;
	float	m_flMinPropScreenSpaceWidth;
	float	m_flMaxPropScreenSpaceWidth;
	bool	m_bColdWorld;
#ifdef DARKINTERVAL // DI's per-level optimisation settings
	bool	m_bRainExpensive;
	bool	m_bForceCheapShadows;
	bool	m_bForceCheapWater;
	float   m_flDetailDist;

	Vector m_SnowDir;  // control dynamic snow from world settings // todo: move to some env_weather_controller entity, or even func_precipitation
	float m_SnowDot;
	float GetSnowDot() const { return m_SnowDot; }
#endif
private:
	void	RegisterSharedActivities( void );
	char	m_iszDetailSpriteMaterial[MAX_DETAIL_SPRITE_MATERIAL_NAME_LENGTH];
};
#ifndef DARKINTERVAL // obsolete, not used by anything
inline float C_World::GetWaveHeight() const
{
	return m_flWaveHeight;
}
#endif
inline const char *C_World::GetDetailSpriteMaterial() const
{
	return m_iszDetailSpriteMaterial;
}

void ClientWorldFactoryInit();
void ClientWorldFactoryShutdown();
C_World* GetClientWorldEntity();

#endif // C_WORLD_H