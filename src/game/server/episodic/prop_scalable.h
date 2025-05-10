#ifndef PROP_SCALABLE_H
#define PROP_SCALABLE_H
#pragma once

#include "cbase.h"
#include "baseentity.h"

class CPropScalable : public CBaseAnimating
{
public:
	DECLARE_CLASS(CPropScalable, CBaseAnimating);
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CPropScalable();

	virtual void Spawn(void);
	virtual void Precache(void);

	CNetworkVar(float, m_flScaleX);
	CNetworkVar(float, m_flScaleY);
	CNetworkVar(float, m_flScaleZ);

	CNetworkVar(float, m_flLerpTimeX);
	CNetworkVar(float, m_flLerpTimeY);
	CNetworkVar(float, m_flLerpTimeZ);

	CNetworkVar(float, m_flGoalTimeX);
	CNetworkVar(float, m_flGoalTimeY);
	CNetworkVar(float, m_flGoalTimeZ);

	void InputSetScaleX(inputdata_t &inputdata);
	void InputSetScaleY(inputdata_t &inputdata);
	void InputSetScaleZ(inputdata_t &inputdata);
};
#endif