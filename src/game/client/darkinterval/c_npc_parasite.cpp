//========= Copyright © 2017, Dark Interval.==================================//
//
// Purpose: 
//
//============================================================================//

#include "cbase.h"
#include "c_ai_basenpc.h"
#include "dlight.h"
#include "iefx.h"
class C_NPC_Parasite : public C_AI_BaseNPC
{
	DECLARE_CLASS(C_NPC_Parasite, C_AI_BaseNPC)
public:
	DECLARE_CLIENTCLASS();

	C_NPC_Parasite(void);

	void	Think(void);

private:
	C_NPC_Parasite(const C_NPC_Parasite &);
};

IMPLEMENT_CLIENTCLASS_DT(C_NPC_Parasite, DT_NPC_Parasite, CNPC_Parasite)
END_RECV_TABLE()

// Constructor

C_NPC_Parasite::C_NPC_Parasite()
{
}

void C_NPC_Parasite::Think(void)
{
/*
	dlight_t *dl = effects->CL_AllocDlight(index);

	dl->origin = GetAbsOrigin() + Vector(0, 0, 0);

	dl->color.r = 50;
	dl->color.g = 75;
	dl->color.b = 250;
	dl->color.exponent = 1.75;

	dl->radius = RandomFloat(60.0f, 75.0f);
	dl->die = CURTIME + 0.2f;
*/
}