//========================================================================//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef C_POINTCAMERA_H
#define C_POINTCAMERA_H
#ifdef _WIN32
#pragma once
#endif

#include "c_baseentity.h"
#include "basetypes.h"

class C_PointCamera : public C_BaseEntity
{
public:
	DECLARE_CLASS( C_PointCamera, C_BaseEntity );
	DECLARE_CLIENTCLASS();

public:
	C_PointCamera();
	~C_PointCamera();

	bool IsActive();
	
	// C_BaseEntity.
#ifdef DARKINTERVAL
	virtual void	OnDataChanged(DataUpdateType_t type);
#endif
	virtual bool	ShouldDraw();

	float			GetFOV();
	float			GetResolution();
	bool			IsFogEnabled();
	void			GetFogColor( unsigned char &r, unsigned char &g, unsigned char &b );
	float			GetFogStart();
	float			GetFogMaxDensity();
	float			GetFogEnd();
	bool			UseScreenAspectRatio() const { return m_bUseScreenAspectRatio; }
#ifdef DARKINTERVAL
	//// Mapbase
	ITexture		*RenderTarget();
	////
#endif
	virtual void	GetToolRecordingState( KeyValues *msg );

private:
	float m_FOV;
	float m_Resolution;
	bool m_bFogEnable;
	color32 m_FogColor;
	float m_flFogStart;
	float m_flFogEnd;
	float m_flFogMaxDensity;
	bool m_bActive;
	bool m_bUseScreenAspectRatio;
#ifdef DARKINTERVAL
	//// Mapbase
	ITexture *m_pRenderTarget;
	char m_iszRenderTarget[64];
	////
#endif
public:
	C_PointCamera	*m_pNext;
};

C_PointCamera *GetPointCameraList();

#endif // C_POINTCAMERA_H
