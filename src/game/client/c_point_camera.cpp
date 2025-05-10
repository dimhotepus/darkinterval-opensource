//========================================================================//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#include "cbase.h"
#include "c_point_camera.h"
#include "toolframework/itoolframework.h"
#include "toolframework_client.h"
#include "tier1/keyvalues.h"


// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

IMPLEMENT_CLIENTCLASS_DT( C_PointCamera, DT_PointCamera, CPointCamera )
	RecvPropFloat( RECVINFO( m_FOV ) ), 
	RecvPropFloat( RECVINFO( m_Resolution ) ), 
	RecvPropInt( RECVINFO( m_bFogEnable ) ),
	RecvPropInt( RECVINFO( m_FogColor ) ),
	RecvPropFloat( RECVINFO( m_flFogStart ) ), 
	RecvPropFloat( RECVINFO( m_flFogEnd ) ), 
	RecvPropFloat( RECVINFO( m_flFogMaxDensity ) ), 
	RecvPropInt( RECVINFO( m_bActive ) ),
	RecvPropInt( RECVINFO( m_bUseScreenAspectRatio ) ),
#ifdef DARKINTERVAL
	//// Mapbase
	RecvPropString(RECVINFO(m_iszRenderTarget)),
	////
#endif
END_RECV_TABLE()

C_EntityClassList<C_PointCamera> g_PointCameraList;
template<> C_PointCamera *C_EntityClassList<C_PointCamera>::m_pClassList = NULL;

C_PointCamera* GetPointCameraList()
{
	return g_PointCameraList.m_pClassList;
}

C_PointCamera::C_PointCamera()
{
	m_bActive = false;
	m_bFogEnable = false;
	//// Mapbase
	m_iszRenderTarget[0] = '\0';
	////
	g_PointCameraList.Insert( this );
}

C_PointCamera::~C_PointCamera()
{
	g_PointCameraList.Remove( this );
}

void C_PointCamera::OnDataChanged(DataUpdateType_t type)
{
#ifdef DARKINTERVAL
	//// Mapbase
	// Reset render texture
	m_pRenderTarget = NULL;
	////
#endif
	return BaseClass::OnDataChanged(type);
}

bool C_PointCamera::ShouldDraw()
{
	return false;
}

float C_PointCamera::GetFOV()
{
	return m_FOV;
}

float C_PointCamera::GetResolution()
{
	return m_Resolution;
}

bool C_PointCamera::IsFogEnabled()
{
	return m_bFogEnable;
}

void C_PointCamera::GetFogColor( unsigned char &r, unsigned char &g, unsigned char &b )
{
	r = m_FogColor.r;
	g = m_FogColor.g;
	b = m_FogColor.b;
}

float C_PointCamera::GetFogStart()
{
	return m_flFogStart;
}

float C_PointCamera::GetFogEnd()
{
	return m_flFogEnd;
}

float C_PointCamera::GetFogMaxDensity()
{
	return m_flFogMaxDensity;
}

bool C_PointCamera::IsActive()
{
	return m_bActive;
}


void C_PointCamera::GetToolRecordingState( KeyValues *msg )
{
	BaseClass::GetToolRecordingState( msg );

	unsigned char r, g, b;
	static MonitorRecordingState_t state;
	state.m_bActive = IsActive() && !IsDormant();
	state.m_flFOV = GetFOV();
	state.m_bFogEnabled = IsFogEnabled();
	state.m_flFogStart = GetFogStart();
	state.m_flFogEnd = GetFogEnd();
	GetFogColor( r, g, b );
	state.m_FogColor.SetColor( r, g, b, 255 );
					  
	msg->SetPtr( "monitor", &state );
}
#ifdef DARKINTERVAL
//// Mapbase
extern ITexture *GetCameraTexture( void );
extern void AddReleaseFunc(void);

ITexture *C_PointCamera::RenderTarget()
{
	if (m_iszRenderTarget[0] != '\0')
	{
		if (!m_pRenderTarget)
		{
			// We don't use a CTextureReference for this because we don't want to shut down the texture on removal/change
			m_pRenderTarget = materials->FindTexture(m_iszRenderTarget, TEXTURE_GROUP_RENDER_TARGET);
		}

		if (m_pRenderTarget)
			return m_pRenderTarget;
	}

	return GetCameraTexture();
}
////
#endif