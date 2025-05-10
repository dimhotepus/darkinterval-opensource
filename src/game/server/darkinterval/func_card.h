//========================================================================//
// DI NEW: simpler version of areaportal window that doesn't affect visibility
// at all, all it does is controls opacity of another brush model, so we can
// have things appear from far away and fade out when we get closer to them - 
// - ideal for fake fog cards and things like that.
//=============================================================================//

#ifndef FUNC_CARD_H
#define FUNC_CARD_H
#ifdef _WIN32
#pragma once
#endif

#include "baseentity.h"
#include "utllinkedlist.h"

class CFuncCard : public CBaseEntity
{
public:
	DECLARE_CLASS(CFuncCard, CBaseEntity);

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	CFuncCard();
	~CFuncCard();


	// Overrides.
public:

	virtual void	Spawn();
	virtual void	Activate();

public:

	CNetworkVar(float, m_flFadeStartDist);	// Distance at which it starts fading (when <= this, alpha=m_flTranslucencyLimit).
	CNetworkVar(float, m_flFadeDist);		// Distance at which it becomes solid.
	CNetworkVar(bool, m_bUseFOV);			// let player fov affect the fading - when using zoom

											// 0-1 value - minimum translucency it's allowed to get to.
	CNetworkVar(float, m_flTranslucencyLimit);

	string_t 		m_iBackgroundBModelName;	// string name of background bmodel
	CNetworkVar(int, m_iBackgroundModelIndex);

	//Input handlers
	void InputSetFadeStartDistance(inputdata_t &inputdata);
	void InputSetFadeEndDistance(inputdata_t &inputdata);
};



#endif // FUNC_CARD_H
