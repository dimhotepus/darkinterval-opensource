//========================================================================//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "fx.h"
#include "c_te_effect_dispatch.h"
#include "c_te_legacytempents.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#ifdef DARKINTERVAL // DI uses a more specific system which is meant to be more organised, easier to expand.
//-----------------------------------------------------------------------------
// Purpose: Shell Type 0 - Pistol
//-----------------------------------------------------------------------------
void Pistol_Shell_Eject_Callback( const CEffectData &data )
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if ( pRenderable )
	{
		tempents->EjectBrass( data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 0 );
	}
}

DECLARE_CLIENT_EFFECT( "EjectShell_Pistol", Pistol_Shell_Eject_Callback);

//-----------------------------------------------------------------------------
// Purpose: Shell Type 1 - Revolver
//-----------------------------------------------------------------------------
void Revolver_Shell_Eject_Callback( const CEffectData &data )
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if ( pRenderable )
	{
		tempents->EjectBrass( data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 1 );
	}
}

DECLARE_CLIENT_EFFECT( "EjectShell_Revolver", Revolver_Shell_Eject_Callback);

//-----------------------------------------------------------------------------
// Purpose: Shell Type 2 - Rifle (OICW, Sniper...)
//-----------------------------------------------------------------------------
void Rifle_Shell_Eject_Callback(const CEffectData &data)
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if (pRenderable)
	{
		tempents->EjectBrass(data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 2);
	}
}

DECLARE_CLIENT_EFFECT("EjectShell_Rifle", Rifle_Shell_Eject_Callback);

//-----------------------------------------------------------------------------
// Purpose: Shell Type 3 - Shotgun
//-----------------------------------------------------------------------------
void Shotgun_Shell_Eject_Callback( const CEffectData &data )
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if ( pRenderable )
	{
		tempents->EjectBrass( data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 3 );
	}
}

DECLARE_CLIENT_EFFECT( "EjectShell_Shotgun", Shotgun_Shell_Eject_Callback);


//-----------------------------------------------------------------------------
// Purpose: Shell Type 4 - SMG1
//-----------------------------------------------------------------------------
void SMG_Shell_Eject_Callback(const CEffectData &data)
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if (pRenderable)
	{
		tempents->EjectBrass(data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 4);
	}
}

DECLARE_CLIENT_EFFECT("EjectShell_SMG", SMG_Shell_Eject_Callback);

//-----------------------------------------------------------------------------
// Purpose: Shell Type 5 - Flare
//-----------------------------------------------------------------------------
void Flare_Shell_Eject_Callback(const CEffectData &data)
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if (pRenderable)
	{
		tempents->EjectBrass(data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 5);
	}
}

DECLARE_CLIENT_EFFECT("EjectShell_Flare", Flare_Shell_Eject_Callback);
#else
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ShellEjectCallback(const CEffectData &data)
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if ( pRenderable )
	{
		tempents->EjectBrass(data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 0);
	}
}

DECLARE_CLIENT_EFFECT("ShellEject", ShellEjectCallback);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void RifleShellEjectCallback(const CEffectData &data)
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if ( pRenderable )
	{
		tempents->EjectBrass(data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 1);
	}
}

DECLARE_CLIENT_EFFECT("RifleShellEject", RifleShellEjectCallback);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ShotgunShellEjectCallback(const CEffectData &data)
{
	// Use the gun angles to orient the shell
	IClientRenderable *pRenderable = data.GetRenderable();
	if ( pRenderable )
	{
		tempents->EjectBrass(data.m_vOrigin, data.m_vAngles, pRenderable->GetRenderAngles(), 2);
	}
}

DECLARE_CLIENT_EFFECT("ShotgunShellEject", ShotgunShellEjectCallback);
#endif // DARKINTERVAL
