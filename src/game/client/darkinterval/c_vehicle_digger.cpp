#include "cbase.h"
#include "c_prop_vehicle.h"
#include "flashlighteffect.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Client-side Driveable APC Class
//
class C_Digger : public C_PropVehicleDriveable
{

	DECLARE_CLASS(C_Digger, C_PropVehicleDriveable);

public:

	DECLARE_CLIENTCLASS();
	DECLARE_INTERPOLATION();

	C_Digger();
	~C_Digger();

public:
	void Simulate(void);
private:

	CHeadlightEffect *headlightLeft;
	CHeadlightEffect *headlightRight;
	bool		m_bHeadlightIsOn;
};

IMPLEMENT_CLIENTCLASS_DT(C_Digger, DT_Digger, CDigger)
RecvPropBool(RECVINFO(m_bHeadlightIsOn)),
END_RECV_TABLE()
//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
C_Digger::C_Digger()
{
	headlightLeft = headlightRight = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Deconstructor
//-----------------------------------------------------------------------------
C_Digger::~C_Digger()
{
	if (headlightLeft) delete headlightLeft;
	if (headlightRight) delete headlightRight;
}

void C_Digger::Simulate(void)
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

		float red = 0.57f;
		float green = 0.87f;
		float blue = 0.55f;

		if (attachmentL != INVALID_PARTICLE_ATTACHMENT)
		{
			GetAttachment(attachmentL, vVector, vAngle);
			AngleVectors(vAngle, &vecForward, &vecRight, &vecUp);

			headlightLeft->UpdateLight(vVector, vecForward, vecRight, vecUp, 700, red * 0.7, green * 0.7, blue * 0.7);
		}

		int attachmentR = LookupAttachment("headlight_right");

		if (attachmentR != INVALID_PARTICLE_ATTACHMENT)
		{
			GetAttachment(attachmentR, vVector, vAngle);
			AngleVectors(vAngle, &vecForward, &vecRight, &vecUp);

			headlightRight->UpdateLight(vVector, vecForward, vecRight, vecUp, 700, red * 0.7, green * 0.7, blue * 0.7);
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