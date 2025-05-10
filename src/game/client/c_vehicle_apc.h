#ifndef C_VEHICLE_APC_H
#define C_VEHICLE_APC_H
#pragma once

#include "cbase.h"
#include "c_prop_vehicle.h"
#include "flashlighteffect.h"

// Client-side Driveable APC Class
//
class C_PropAPC : public C_PropVehicleDriveable
{
	DECLARE_CLASS(C_PropAPC, C_PropVehicleDriveable);

public:

	DECLARE_CLIENTCLASS();
	DECLARE_INTERPOLATION();

	C_PropAPC();
	~C_PropAPC();

public:
	void Simulate(void);
	virtual int GetPrimaryAmmoType() const;
	virtual int GetPrimaryAmmoClip() const;
	virtual bool PrimaryAmmoUsesClips() const;
	virtual int GetPrimaryAmmoCount() const;
	virtual int GetSecondaryAmmoType() const;
	virtual int GetSecondaryAmmoClip() const;
	virtual bool SecondaryAmmoUsesClips() const;
	virtual int GetSecondaryAmmoCount() const;
private:

	CHeadlightEffect *headlightLeft;
	CHeadlightEffect *headlightRight;
	int			m_nAmmoCount;
	int			m_nAmmo2Count;
	bool		m_bHeadlightIsOn;
};

#endif //C_VEHICLE_APC_H