//========================================================================//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "gametrace.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

bool CGameTrace::DidHitWorld() const
{
	return m_pEnt == ClientEntityList().GetBaseEntity( 0 );
}


bool CGameTrace::DidHitNonWorldEntity() const
{
	return m_pEnt != NULL && !DidHitWorld();
}
#ifdef DARKINTERVAL // add a couple useful traces. For example, used by the rain code.
bool CGameTrace::DidHitNodrawSurface() const
{
	return m_pEnt != NULL && ( surface.flags & SURF_NODRAW );
}

bool CGameTrace::DidHitSkybox() const
{
	return m_pEnt != NULL && ( surface.flags & ( SURF_SKY | SURF_SKY2D ));
}
#endif
int CGameTrace::GetEntityIndex() const
{
	if ( m_pEnt )
		return m_pEnt->entindex();
	else
		return -1;
}

