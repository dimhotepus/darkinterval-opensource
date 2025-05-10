//========================================================================//
//
// Purpose: 
//
//===========================================================================//

#include "cbase.h"
#include "func_brush.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class CFuncReflectiveGlass : public CFuncBrush
{
	DECLARE_DATADESC();
	DECLARE_CLASS( CFuncReflectiveGlass, CFuncBrush );
	DECLARE_SERVERCLASS();
#ifdef DARKINTERVAL
public:
	void Spawn(void);
#endif
};
// automatically hooks in the system's callbacks
BEGIN_DATADESC(CFuncReflectiveGlass)
END_DATADESC()

LINK_ENTITY_TO_CLASS( func_reflective_glass, CFuncReflectiveGlass );

IMPLEMENT_SERVERCLASS_ST(CFuncReflectiveGlass, DT_FuncReflectiveGlass)
END_SEND_TABLE()
#ifdef DARKINTERVAL
//ConVar r_reflectiveglass_use_distance_cull("r_reflectiveglass_use_distance_cull", "0");
void CFuncReflectiveGlass::Spawn(void)
{
	BaseClass::Spawn();
	SetNextThink(TICK_NEVER_THINK);
}
#endif