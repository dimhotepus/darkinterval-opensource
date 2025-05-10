//========================================================================//
//
// Purpose: 
//
//=============================================================================//

#ifndef C_PHYSICSPROP_H
#define C_PHYSICSPROP_H
#ifdef _WIN32
#pragma once
#endif

#include "c_breakableprop.h"
#include "object_motion_blur_effect.h"
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class C_PhysicsProp : public C_BreakableProp
{
	typedef C_BreakableProp BaseClass;
public:
	DECLARE_CLIENTCLASS();

	C_PhysicsProp();
	~C_PhysicsProp();

	virtual bool OnInternalDrawModel( ClientModelRenderInfo_t *pInfo );

	CMotionBlurObject m_MotionBlurObject; // object motion blur

//	virtual int GetStainCount(void) {return m_stainCount_int;}
protected:
	// Networked vars.
	bool m_bAwake;
	bool m_bAwakeLastTime;
//	int	m_stainCount_int;
};

#endif // C_PHYSICSPROP_H 
