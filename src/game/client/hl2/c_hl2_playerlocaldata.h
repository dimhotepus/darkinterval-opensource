//========================================================================//
//
// Purpose: 
//
// $Workfile:     $
// $NoKeywords: $
//=============================================================================//

#if !defined( C_HL2_PLAYERLOCALDATA_H )
#define C_HL2_PLAYERLOCALDATA_H
#ifdef _WIN32
#pragma once
#endif

#include "dt_recv.h"
#ifdef DARKINTERVAL
#include "hl2_vehicle_radar.h"
#endif
#include "hl2/hl_movedata.h"

EXTERN_RECV_TABLE( DT_HL2Local );

class C_HL2PlayerLocalData
{
public:
	DECLARE_PREDICTABLE();
	DECLARE_CLASS_NOBASE( C_HL2PlayerLocalData );
	DECLARE_EMBEDDED_NETWORKVAR();

	C_HL2PlayerLocalData();

	float	m_flSuitPower;
	bool	m_bZooming;
#ifdef DARKINTERVAL
	int		m_nZoomType; // DI NEW
#endif
	int		m_bitsActiveDevices;
	int		m_iSquadMemberCount;
	int		m_iSquadMedicCount;
	bool	m_fSquadInFollowMode;
	bool	m_bWeaponLowered;
	EHANDLE m_hAutoAimTarget;
	Vector	m_vecAutoAimPoint;
	bool	m_bDisplayReticle;
	bool	m_bStickyAutoAim;
	bool	m_bAutoAimTarget;
#ifdef HL2_EPISODIC
	float	m_flFlashBattery;
	Vector	m_vecLocatorOrigin;
#ifdef DARKINTERVAL // TE120--
	int m_iNumLocatorContacts;
	CHandle< EHANDLE > m_locatorEnt[LOCATOR_MAX_CONTACTS];
	float	m_flTapePos[LOCATOR_MAX_CONTACTS];
	int m_iLocatorContactType[LOCATOR_MAX_CONTACTS];
#endif
#endif
#ifdef DARKINTERVAL
	bool	m_flashlight_force_flicker_bool;
#endif

	// Ladder related data
	EHANDLE			m_hLadder;
	LadderMove_t	m_LadderMove;
};


#endif
