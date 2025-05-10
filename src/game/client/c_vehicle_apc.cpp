#include "cbase.h"
#include "c_vehicle_apc.h"
#include "ammodef.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Client-side Driveable APC Class
//

IMPLEMENT_CLIENTCLASS_DT(C_PropAPC, DT_PropAPC, CPropAPC)
	RecvPropInt(RECVINFO(m_nAmmoCount)),
	RecvPropInt(RECVINFO(m_nAmmo2Count)),
	RecvPropBool(RECVINFO(m_bHeadlightIsOn)),
END_RECV_TABLE()
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_PropAPC::C_PropAPC()
{
	headlightLeft = headlightRight = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
C_PropAPC::~C_PropAPC()
{
	if (headlightLeft) delete headlightLeft;
	if (headlightRight) delete headlightRight;
}
//-----------------------------------------------------------------------------
// Draws APC turret ammo count
//-----------------------------------------------------------------------------
int C_PropAPC::GetPrimaryAmmoType() const
{
	if (m_nAmmoCount < 0)
		return -1;

	int nAmmoType = GetAmmoDef()->Index("AirboatGun");
	return nAmmoType;
}

int C_PropAPC::GetPrimaryAmmoCount() const
{
	return m_nAmmoCount;
}

bool C_PropAPC::PrimaryAmmoUsesClips() const
{
	return false;
}

int C_PropAPC::GetPrimaryAmmoClip() const
{
	return -1;
}

//-----------------------------------------------------------------------------
// Draws APC missile ammo count
//-----------------------------------------------------------------------------
int C_PropAPC::GetSecondaryAmmoType() const
{
	if (m_nAmmo2Count < 0)
		return -1;

	int nAmmo2Type = GetAmmoDef()->Index("RPG");
	return nAmmo2Type;
}

int C_PropAPC::GetSecondaryAmmoCount() const
{
	return m_nAmmo2Count;
}

bool C_PropAPC::SecondaryAmmoUsesClips() const
{
	return false;
}

int C_PropAPC::GetSecondaryAmmoClip() const
{
	return -1;
}

void C_PropAPC::Simulate(void)
{
	// The dim light is the flashlight.
	if (m_bHeadlightIsOn)
	{
		if (headlightLeft == NULL)
		{
			// Turned on the headlight; create it.
			headlightLeft = new CHeadlightEffect;

			if (headlightLeft == NULL)	return;

			headlightLeft->TurnOn();
		}

		if (headlightRight == NULL)
		{
			// Turned on the headlight; create it.
			headlightRight = new CHeadlightEffect;

			if (headlightRight == NULL)	return;

			headlightRight->TurnOn();
		}

		QAngle vAngle;
		Vector vVector;
		Vector vecForward, vecRight, vecUp;

		int attachmentL = LookupAttachment("headlight_left");

		if (attachmentL != INVALID_PARTICLE_ATTACHMENT)
		{
			GetAttachment(attachmentL, vVector, vAngle);
			AngleVectors(vAngle, &vecForward, &vecRight, &vecUp);

			headlightLeft->UpdateLight(vVector, vecForward, vecRight, vecUp, 4096.0f);
		}
		
		int attachmentR = LookupAttachment("headlight_right");

		if (attachmentR != INVALID_PARTICLE_ATTACHMENT)
		{
			GetAttachment(attachmentR, vVector, vAngle);
			AngleVectors(vAngle, &vecForward, &vecRight, &vecUp);

			headlightRight->UpdateLight(vVector, vecForward, vecRight, vecUp, 4096.0f);
		}
	}
	else
	{
		if (headlightLeft)
		{
			// Turned off the flashlight; delete it.
			delete headlightLeft;
			headlightLeft = NULL;
		}

		if (headlightRight)
		{
			// Turned off the flashlight; delete it.
			delete headlightRight;
			headlightRight = NULL;
		}
	}
	
	BaseClass::Simulate();
}