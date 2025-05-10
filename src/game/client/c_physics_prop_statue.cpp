//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "cbase.h"
#include "c_physics_prop_statue.h"
#include "debugoverlay_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT(C_StatueProp, DT_StatueProp, CStatueProp)
	RecvPropEHandle( RECVINFO( m_hInitBaseAnimating ) ),
	RecvPropBool( RECVINFO( m_bShatter ) ),
	RecvPropInt( RECVINFO( m_nShatterFlags ) ),
	RecvPropVector( RECVINFO( m_vShatterPosition ) ),
	RecvPropVector( RECVINFO( m_vShatterForce ) ),
END_RECV_TABLE()

C_StatueProp::C_StatueProp()
{
	m_bShattered = false;
}

C_StatueProp::~C_StatueProp()
{
}

void C_StatueProp::Spawn( void )
{
	BaseClass::Spawn();

	m_EntClientFlags |= ENTCLIENTFLAG_DONTUSEIK;
}

void C_StatueProp::ComputeWorldSpaceSurroundingBox( Vector *pVecWorldMins, Vector *pVecWorldMaxs )
{
	CBaseAnimating *pBaseAnimating = m_hInitBaseAnimating;

	if ( pBaseAnimating )
	{
		pBaseAnimating->CollisionProp()->WorldSpaceSurroundingBounds( pVecWorldMins, pVecWorldMaxs );
	}
}


void C_StatueProp::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	Vector vecSurroundMins, vecSurroundMaxs;
	vecSurroundMins = CollisionProp()->OBBMins();
	vecSurroundMaxs = CollisionProp()->OBBMaxs();

	if (m_bShatter && !m_bShattered)
	{
		m_bShattered = true;

		float xSize = vecSurroundMaxs.x - vecSurroundMins.x;
		float ySize = vecSurroundMaxs.y - vecSurroundMins.y;
		float flSize = (xSize + ySize) / 218.0f; // this is the size of the drone BBox
		if (flSize < 0.5f)
			flSize = 0.5f;
		else if (flSize > 2.5f)
			flSize = 2.5f;

		//NDebugOverlay::Cross3D( m_vShatterPosition, 16, 255, 0, 0, true, 30 );
		CUtlReference<CNewParticleEffect> pEffect;
		pEffect = ParticleProp()->Create("freeze_statue_shatter", PATTACH_ABSORIGIN_FOLLOW, -1, m_vShatterPosition - GetAbsOrigin());
		pEffect->SetControlPoint(1, Vector(flSize, 0, 0));
	}
}
