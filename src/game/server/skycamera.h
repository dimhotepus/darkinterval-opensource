//========================================================================//
//
// Purpose: Resource collection entity
//
// $NoKeywords: $
//=============================================================================//

#ifndef SKYCAMERA_H
#define SKYCAMERA_H

#ifdef _WIN32
#pragma once
#endif

class CSkyCamera;

//=============================================================================
//
// Sky Camera Class
//
#ifdef DARKINTERVAL
class CSkyCamera : public CPointEntity // DI change - so that we can parent it
#else
class CSkyCamera : CLogicalEntity
#endif
{
#ifdef DARKINTERVAL
	DECLARE_CLASS(CSkyCamera, CPointEntity);
#else
	DECLARE_CLASS(CSkyCamera, CLogicalEntity );
#endif
public:

	DECLARE_DATADESC();
	CSkyCamera();
	~CSkyCamera();
	virtual void Spawn( void );
#ifdef DARKINTERVAL
	virtual void ActivateCamera();
	virtual void OnRestore();
#else
	virtual void Activate();
#endif
public:
#ifdef DARKINTERVAL
	void			InputDeactivate(inputdata_t &inputdata);
	void			InputActivate(inputdata_t &inputdata);
	void			InputTurnOff3dSky(inputdata_t &inputdata);
	void			InputTurnOn3dSky(inputdata_t &inputdata);
	bool			m_bEnabled;
#endif
	sky3dparams_t	m_skyboxData;
	bool			m_bUseAngles;
	CSkyCamera		*m_pNext;
#ifdef DARKINTERVAL
	bool			m_skycamera_active_bool = true;
	bool			m_skycamera_draw3dsky_bool = true;

	void			SkyCameraThink();
#endif
};


//-----------------------------------------------------------------------------
// Retrives the current skycamera
//-----------------------------------------------------------------------------
CSkyCamera*		GetCurrentSkyCamera();
CSkyCamera*		GetSkyCameraList();


#endif // SKYCAMERA_H
