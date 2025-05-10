#ifndef POINT_SPOTLIGHT_H
#define POINT_SPOTLIGHT_H
#include "beam_shared.h"
#include "spotlightend.h"
#endif // POINT_SPOTLIGHT_h

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CPointSpotlight : public CPointEntity
{
	DECLARE_CLASS(CPointSpotlight, CPointEntity);
public:
	DECLARE_DATADESC();

	CPointSpotlight();

	void	Precache(void);
	void	Spawn(void);
	virtual void Activate();

	virtual void OnEntityEvent(EntityEvent_t event, void *pEventData);
	
private:
	int 	UpdateTransmitState();
	void	SpotlightThink(void);
	void	SpotlightUpdate(void);
	Vector	SpotlightCurrentPos(void);
#ifdef DARKINTERVAL
	void	SpotlightCreate(const char* pSpotlightTextureName);  // to allow custom textures for the beam
#else
	void	SpotlightCreate(void);
#endif
	void	SpotlightDestroy(void);
#ifdef DARKINTERVAL // to allow custom textures for the beam
	CNetworkString(szTextureName, MAX_PATH);
	bool	KeyValue(const char *szKeyValueName, const char* szKeyValue);
#endif
	// ------------------------------
	//  Inputs
	// ------------------------------
	void InputLightOn(inputdata_t &inputdata);
	void InputLightOff(inputdata_t &inputdata);
#ifdef DARKINTERVAL
	void InputSetBeamWidth(inputdata_t &inputdata);
#endif
	// Creates the efficient spotlight 
	void CreateEfficientSpotlight();

	// Computes render info for a spotlight
	void ComputeRenderInfo();

private:
	bool	m_bSpotlightOn;
	bool	m_bEfficientSpotlight;
#ifdef DARKINTERVAL
	bool	m_bIgnoreSolid; // allows spotlights that sink into the prop/brush to not be disabled
#endif
	Vector	m_vSpotlightTargetPos;
	Vector	m_vSpotlightCurrentPos;
	Vector	m_vSpotlightDir;
	int		m_nHaloSprite;
	CHandle<CBeam>			m_hSpotlight;
	CHandle<CSpotlightEnd>	m_hSpotlightTarget;

	float	m_flSpotlightMaxLength;
	float	m_flSpotlightCurLength;
	float	m_flSpotlightGoalWidth;
	float	m_flHDRColorScale;
	int		m_nMinDXLevel;
public:
	COutputEvent m_OnOn, m_OnOff;     ///< output fires when turned on, off
};