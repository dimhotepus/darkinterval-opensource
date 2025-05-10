//====== Copyright © 1996-2005, Valve Corporation, All rights reserved. =======
//
// Purpose: 
//
//=============================================================================

#include "cbase.h"

// NOTE: This has to be the last file included!
#include "tier0/memdbgon.h"


//extern bool  g_bDOFEnabled;
extern float g_flDOFNearBlurDepth;
extern float g_flDOFNearFocusDepth;
extern float g_flDOFFarFocusDepth;
extern float g_flDOFFarBlurDepth;
extern float g_flDOFNearBlurRadius;
extern float g_flDOFFarBlurRadius;

EHANDLE g_hDOFControllerInUse = NULL;

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_EnvDOFController : public C_BaseEntity
{
	DECLARE_CLASS( C_EnvDOFController, C_BaseEntity );
public:
	DECLARE_CLIENTCLASS();

	C_EnvDOFController();
	~C_EnvDOFController();
	virtual void	OnDataChanged( DataUpdateType_t updateType );

private:
	bool  m_bDOFEnabled;
	float m_flNearBlurDepth;
	float m_flNearFocusDepth;
	float m_flFarFocusDepth;
	float m_flFarBlurDepth;
	float m_flNearBlurRadius;
	float m_flFarBlurRadius;

private:
	C_EnvDOFController( const C_EnvDOFController & );
};

IMPLEMENT_CLIENTCLASS_DT( C_EnvDOFController, DT_EnvDOFController, CEnvDOFController )
	RecvPropInt( RECVINFO(m_bDOFEnabled) ),
	RecvPropFloat( RECVINFO(m_flNearBlurDepth) ),
	RecvPropFloat( RECVINFO(m_flNearFocusDepth) ),
	RecvPropFloat( RECVINFO(m_flFarFocusDepth) ),
	RecvPropFloat( RECVINFO(m_flFarBlurDepth) ),
	RecvPropFloat( RECVINFO(m_flNearBlurRadius) ),
	RecvPropFloat( RECVINFO(m_flFarBlurRadius) )
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_EnvDOFController::C_EnvDOFController( void )
:	m_bDOFEnabled( true ),
	m_flNearBlurDepth( 0.0f ),
	m_flNearFocusDepth( 0.0f ),
	m_flFarFocusDepth( 0.0f ),
	m_flFarBlurDepth( 0.0f ),
	m_flNearBlurRadius( 0.0f ),		// no near blur by default
	m_flFarBlurRadius( 0.0f )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_EnvDOFController::~C_EnvDOFController( void )
{
	if ( g_hDOFControllerInUse == this )
	{
//		g_bDOFEnabled = false;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ConVar mat_dof_zoom_scaling("mat_dof_zoom_scaling", "1");
void C_EnvDOFController::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );
	
	// Compute the dof scaling based on player zoom value
	float flFactor = 1.0f;
	C_BasePlayer *pLocal = C_BasePlayer::GetLocalPlayer();
	if (pLocal)
	{
		flFactor = pLocal->GetFOVDistanceAdjustFactor();
	}

//	g_bDOFEnabled = m_bDOFEnabled && ( ( m_flNearBlurRadius > 0.0f ) || ( m_flFarBlurRadius > 0.0f ) );
	g_flDOFNearBlurDepth	= m_flNearBlurDepth;
	g_flDOFNearFocusDepth	= m_flNearFocusDepth;
	g_flDOFFarFocusDepth	= m_flFarFocusDepth;
	g_flDOFFarBlurDepth		= m_flFarBlurDepth;
	g_flDOFNearBlurRadius	= m_flNearBlurRadius;
	if (mat_dof_zoom_scaling.GetBool())
		g_flDOFFarBlurRadius = m_flFarBlurRadius * flFactor;
	else
		g_flDOFFarBlurRadius = m_flFarBlurRadius;

	g_hDOFControllerInUse = this;
}
