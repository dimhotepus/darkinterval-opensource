//========================================================================//
//
// Purpose: The various ammo types for HL2	
//
//=============================================================================//

#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "items.h"
#include "ammodef.h"
#include "eventlist.h"
#include "npcevent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//---------------------------------------------------------
// Applies ammo quantity scale.
//---------------------------------------------------------
int ITEM_GiveAmmo( CBasePlayer *pPlayer, float flCount, const char *pszAmmoName, bool bSuppressSound = false )
{
	int iAmmoType = GetAmmoDef()->Index(pszAmmoName);
	if (iAmmoType == -1)
	{
		Msg("ERROR: Attempting to give unknown ammo type (%s)\n",pszAmmoName);
		return 0;
	}

	flCount *= g_pGameRules->GetAmmoQuantityScale(iAmmoType);

	// Don't give out less than 1 of anything.
	flCount = MAX( 1.0f, flCount );

	return pPlayer->GiveAmmo( flCount, iAmmoType, bSuppressSound );
}

// ========================================================================
//	>> BoxSRounds
// ========================================================================
class CItem_BoxSRounds : public CItem
{
public:
	DECLARE_CLASS( CItem_BoxSRounds, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/boxsrounds.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/items/boxsrounds.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_PISTOL, "Pistol"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}

			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_box_srounds, CItem_BoxSRounds);
LINK_ENTITY_TO_CLASS(item_ammo_pistol, CItem_BoxSRounds);
#ifdef DARKINTERVAL // convenient reserve names
LINK_ENTITY_TO_CLASS(ammo_pistol, CItem_BoxSRounds);
#endif
// ========================================================================
//	>> LargeBoxSRounds
// ========================================================================
class CItem_LargeBoxSRounds : public CItem
{
public:
	DECLARE_CLASS( CItem_LargeBoxSRounds, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/boxsrounds.mdl" );
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/items/boxsrounds.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_PISTOL_LARGE, "Pistol"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_large_box_srounds, CItem_LargeBoxSRounds);
LINK_ENTITY_TO_CLASS(item_ammo_pistol_large, CItem_LargeBoxSRounds);
#ifdef DARKINTERVAL // convenient reserve names
LINK_ENTITY_TO_CLASS(ammo_pistol_large, CItem_LargeBoxSRounds);
#endif
// ========================================================================
//	>> BoxMRounds
// ========================================================================
class CItem_BoxMRounds : public CItem
{
public:
	DECLARE_CLASS( CItem_BoxMRounds, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/boxmrounds.mdl");
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/items/boxmrounds.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_SMG1, "SMG1"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_box_mrounds, CItem_BoxMRounds);
LINK_ENTITY_TO_CLASS(item_ammo_smg1, CItem_BoxMRounds);
#ifdef DARKINTERVAL // convenient reserve names
LINK_ENTITY_TO_CLASS(item_ammo_smg, CItem_BoxMRounds);
LINK_ENTITY_TO_CLASS(ammo_smg, CItem_BoxMRounds);
LINK_ENTITY_TO_CLASS(ammo_smg1, CItem_BoxMRounds);
#endif
// ========================================================================
//	>> LargeBoxMRounds
// ========================================================================
class CItem_LargeBoxMRounds : public CItem
{
public:
	DECLARE_CLASS( CItem_LargeBoxMRounds, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/boxmrounds.mdl");
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/items/boxmrounds.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_SMG1_LARGE, "SMG1"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_large_box_mrounds, CItem_LargeBoxMRounds);
LINK_ENTITY_TO_CLASS(item_ammo_smg1_large, CItem_LargeBoxMRounds);
#ifdef DARKINTERVAL // convenient reserve names
LINK_ENTITY_TO_CLASS(item_ammo_smg_large, CItem_LargeBoxMRounds);
LINK_ENTITY_TO_CLASS(ammo_smg_large, CItem_LargeBoxMRounds);
LINK_ENTITY_TO_CLASS(ammo_smg1_large, CItem_LargeBoxMRounds);
#endif
// ========================================================================
//	>> BoxLRounds
// ========================================================================
class CItem_BoxLRounds : public CItem
{
public:
	DECLARE_CLASS( CItem_BoxLRounds, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/combine_rifle_cartridge01.mdl");
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/items/combine_rifle_cartridge01.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_AR2, "AR2"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}	
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_box_lrounds, CItem_BoxLRounds);
LINK_ENTITY_TO_CLASS(item_ammo_ar2, CItem_BoxLRounds);
#ifdef DARKINTERVAL // convenient reserve names
LINK_ENTITY_TO_CLASS(ammo_ar2, CItem_BoxLRounds);
#endif
// ========================================================================
//	>> LargeBoxLRounds
// ========================================================================
class CItem_LargeBoxLRounds : public CItem
{
public:
	DECLARE_CLASS( CItem_LargeBoxLRounds, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/combine_rifle_cartridge01.mdl");
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/items/combine_rifle_cartridge01.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_AR2_LARGE, "AR2"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}	
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_large_box_lrounds, CItem_LargeBoxLRounds);
LINK_ENTITY_TO_CLASS(item_ammo_ar2_large, CItem_LargeBoxLRounds);
#ifdef DARKINTERVAL // convenient reserve names
LINK_ENTITY_TO_CLASS(ammo_ar2_large, CItem_LargeBoxLRounds);
#endif
// ========================================================================
//	>> CItem_Box357Rounds
// ========================================================================
class CItem_Box357Rounds : public CItem
{
public:
	DECLARE_CLASS( CItem_Box357Rounds, CItem );

	void Precache( void )
	{
		PrecacheModel ("models/items/357ammo.mdl");
	}
	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/357ammo.mdl");
		BaseClass::Spawn( );
	}

	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_357, "357"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}	
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_ammo_357, CItem_Box357Rounds);
#ifdef DARKINTERVAL // convenient reserve names
LINK_ENTITY_TO_CLASS(item_ammo_revolver, CItem_Box357Rounds);
LINK_ENTITY_TO_CLASS(ammo_357, CItem_Box357Rounds);
LINK_ENTITY_TO_CLASS(ammo_revolver, CItem_Box357Rounds);
#endif
// ========================================================================
//	>> CItem_LargeBox357Rounds
// ========================================================================
class CItem_LargeBox357Rounds : public CItem
{
public:
	DECLARE_CLASS( CItem_LargeBox357Rounds, CItem );

	void Precache( void )
	{
		PrecacheModel ("models/items/357ammobox.mdl");
	}
	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/357ammobox.mdl");
		BaseClass::Spawn( );
	}

	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_357_LARGE, "357"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_ammo_357_large, CItem_LargeBox357Rounds);
#ifdef DARKINTERVAL // convenient reserve names
LINK_ENTITY_TO_CLASS(item_ammo_revolver_large, CItem_LargeBox357Rounds);
LINK_ENTITY_TO_CLASS(ammo_357_large, CItem_LargeBox357Rounds);
LINK_ENTITY_TO_CLASS(ammo_revolver_large, CItem_LargeBox357Rounds);
#endif
#ifdef DARKINTERVAL
// ========================================================================
//	>> Revolver Round (single)
// ========================================================================
class CItem_RoundRevolver : public CItem
{
public:
	DECLARE_CLASS(CItem_RoundRevolver, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/_Weapons/revolver_ammo_single.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/_Weapons/revolver_ammo_single.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, 1, "357"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_round_357, CItem_RoundRevolver);
LINK_ENTITY_TO_CLASS(item_357_round, CItem_RoundRevolver);
LINK_ENTITY_TO_CLASS(item_revolver_round, CItem_RoundRevolver);
LINK_ENTITY_TO_CLASS(round_revolver, CItem_RoundRevolver);
LINK_ENTITY_TO_CLASS(item_round_revolver, CItem_RoundRevolver);
LINK_ENTITY_TO_CLASS(revolver_round, CItem_RoundRevolver);
#endif
// ========================================================================
//	>> CItem_BoxXBowRounds
// ========================================================================
class CItem_BoxXBowRounds : public CItem
{
public:
	DECLARE_CLASS( CItem_BoxXBowRounds, CItem );

	void Precache( void )
	{
		PrecacheModel ("models/items/crossbowrounds.mdl");
	}

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/crossbowrounds.mdl");
		BaseClass::Spawn( );
	}

	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_CROSSBOW, "XBowBolt" ))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}	
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_ammo_crossbow, CItem_BoxXBowRounds);
#ifdef DARKINTERVAL // convenient reserve names
LINK_ENTITY_TO_CLASS(ammo_crossbow, CItem_BoxXBowRounds);
#endif
// ========================================================================
//	>> FlareRound
// ========================================================================
class CItem_FlareRound : public CItem
{
public:
	DECLARE_CLASS( CItem_FlareRound, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/flare.mdl");
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/items/flare.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
#ifdef DARKINTERVAL
		if (ITEM_GiveAmmo( pPlayer, 1, "Flaregun"))
#else
		if (ITEM_GiveAmmo( pPlayer, 1, "FlareRound" ))
#endif
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_flare_round, CItem_FlareRound);
#ifdef DARKINTERVAL // convenient reserve names
LINK_ENTITY_TO_CLASS(item_ammo_flareround, CItem_FlareRound);
LINK_ENTITY_TO_CLASS(item_ammo_flare_round, CItem_FlareRound);
LINK_ENTITY_TO_CLASS(item_ammo_flare, CItem_FlareRound);
LINK_ENTITY_TO_CLASS(ammo_flare, CItem_FlareRound);
#endif
// ========================================================================
//	>> BoxFlareRounds
// ========================================================================
#define SIZE_BOX_FLARE_ROUNDS 5

class CItem_BoxFlareRounds : public CItem
{
public:
	DECLARE_CLASS( CItem_BoxFlareRounds, CItem );

	void Spawn( void )
	{ 
		Precache( );
#ifdef DARKINTERVAL
		SetModel( "models/_Weapons/flarebox.mdl");
#else
		SetModel("models/items/boxflares.mdl");
#endif
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
#ifdef DARKINTERVAL
		PrecacheModel ("models/_Weapons/flarebox.mdl");
#else
		PrecacheModel ("models/items/boxflares.mdl");
#endif
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
#ifdef DARKINTERVAL
		if (ITEM_GiveAmmo( pPlayer, SIZE_BOX_FLARE_ROUNDS, "Flaregun"))
#else
		if ( ITEM_GiveAmmo(pPlayer, SIZE_BOX_FLARE_ROUNDS, "FlareRound") )
#endif
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_box_flare_rounds, CItem_BoxFlareRounds);
#ifdef DARKINTERVAL // convenient reserve names
LINK_ENTITY_TO_CLASS(item_ammo_flarebox, CItem_BoxFlareRounds);
LINK_ENTITY_TO_CLASS(item_ammo_flare_box, CItem_BoxFlareRounds);
LINK_ENTITY_TO_CLASS(ammo_flares, CItem_BoxFlareRounds);
#endif
// ========================================================================
// RPG Round
// ========================================================================
class CItem_RPG_Round : public CItem
{
public:
	DECLARE_CLASS( CItem_RPG_Round, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/weapons/w_missile_closed.mdl");
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/weapons/w_missile_closed.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_RPG_ROUND, "RPG_Round"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}	
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS( item_ml_grenade, CItem_RPG_Round );
LINK_ENTITY_TO_CLASS( item_rpg_round, CItem_RPG_Round );
#ifdef DARKINTERVAL // convenient reserve names
LINK_ENTITY_TO_CLASS( item_ammo_rpg, CItem_RPG_Round);
LINK_ENTITY_TO_CLASS( ammo_rpg, CItem_RPG_Round);
#endif
// ========================================================================
//	>> AR2_Grenade
// ========================================================================
class CItem_AR2_Grenade : public CItem
{
public:
	DECLARE_CLASS( CItem_AR2_Grenade, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/ar2_grenade.mdl");
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/items/ar2_grenade.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_SMG1_GRENADE, "SMG1_Grenade"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}	
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_ar2_grenade, CItem_AR2_Grenade);
LINK_ENTITY_TO_CLASS(item_ammo_smg1_grenade, CItem_AR2_Grenade);
#ifdef DARKINTERVAL // convenient reserve names
LINK_ENTITY_TO_CLASS( ammo_smg1_grenade, CItem_AR2_Grenade);
LINK_ENTITY_TO_CLASS(ammo_smg_grenade, CItem_AR2_Grenade);
#endif
// ========================================================================
//	>> BoxSniperRounds
// ========================================================================
#define SIZE_BOX_SNIPER_ROUNDS 10

class CItem_BoxSniperRounds : public CItem
{
public:
	DECLARE_CLASS( CItem_BoxSniperRounds, CItem );

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/boxsniperrounds.mdl");
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
		PrecacheModel ("models/items/boxsniperrounds.mdl");
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_BOX_SNIPER_ROUNDS, "SniperRound"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}	
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_box_sniper_rounds, CItem_BoxSniperRounds);

// ========================================================================
//	>> Shotgun Shell (single)
// ========================================================================
#ifdef DARKINTERVAL
class CItem_RoundBuckshot : public CItem
{
public:
	DECLARE_CLASS(CItem_RoundBuckshot, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/_Weapons/shotgun_ammo_single.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/_Weapons/shotgun_ammo_single.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, 1, "Buckshot"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_round_buckshot, CItem_RoundBuckshot);
LINK_ENTITY_TO_CLASS(item_buckshot_round, CItem_RoundBuckshot);
LINK_ENTITY_TO_CLASS(item_shotgun_round, CItem_RoundBuckshot);
LINK_ENTITY_TO_CLASS(round_shotgun, CItem_RoundBuckshot);
LINK_ENTITY_TO_CLASS(item_round_shotgun, CItem_RoundBuckshot);
LINK_ENTITY_TO_CLASS(shotgun_round, CItem_RoundBuckshot);
#endif // DARKINTERVAL
class CItem_BoxBuckshot : public CItem
{
public:
	DECLARE_CLASS( CItem_BoxBuckshot, CItem );

	void Spawn( void )
	{ 
		Precache( );
#ifdef DARKINTERVAL
		SetModel( "models/_Weapons/shotgun_ammo_strip.mdl");
#else
		SetModel("models/items/boxbuckshot.mdl");
#endif
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
#ifdef DARKINTERVAL
		PrecacheModel ("models/_Weapons/shotgun_ammo_strip.mdl");
#else
		PrecacheModel ("models/items/boxbuckshot.mdl");
#endif
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_BUCKSHOT, "Buckshot"))
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_box_buckshot, CItem_BoxBuckshot);
#ifdef DARKINTERVAL // convenient reserve names
LINK_ENTITY_TO_CLASS(item_ammo_shotgun, CItem_BoxBuckshot);
LINK_ENTITY_TO_CLASS(ammo_shotgun, CItem_BoxBuckshot);
#endif
#ifdef DARKINTERVAL
// ========================================================================
//	>> Box of shells
// ========================================================================
class CItem_BoxBuckshotLarge : public CItem
{
public:
	DECLARE_CLASS(CItem_BoxBuckshotLarge, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/Items/boxbuckshot.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/Items/boxbuckshot.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_BUCKSHOT_LARGE, "Buckshot"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_box_buckshot_large, CItem_BoxBuckshotLarge);
LINK_ENTITY_TO_CLASS(item_shotgun_large, CItem_BoxBuckshotLarge);
LINK_ENTITY_TO_CLASS(ammo_shotgun_large, CItem_BoxBuckshotLarge);
#endif
// ========================================================================
//	>> CItem_AR2AltFireRound
// ========================================================================
class CItem_AR2AltFireRound : public CItem
{
public:
	DECLARE_CLASS( CItem_AR2AltFireRound, CItem );

	void Precache( void )
	{
		PrecacheParticleSystem( "combineball" );
		PrecacheModel ("models/items/combine_rifle_ammo01.mdl");
	}

	void Spawn( void )
	{ 
		Precache( );
		SetModel( "models/items/combine_rifle_ammo01.mdl");
		BaseClass::Spawn( );
	}

	bool MyTouch( CBasePlayer *pPlayer )
	{
		if (ITEM_GiveAmmo( pPlayer, SIZE_AMMO_AR2_ALTFIRE, "AR2AltFire" ) )
		{
			if ( g_pGameRules->ItemShouldRespawn( this ) == GR_ITEM_RESPAWN_NO )
			{
				UTIL_Remove(this);	
			}
			return true;
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS( item_ammo_ar2_altfire, CItem_AR2AltFireRound );
#ifdef DARKINTERVAL // convenient reserve names
LINK_ENTITY_TO_CLASS( item_ammo_energyball, CItem_AR2AltFireRound);
LINK_ENTITY_TO_CLASS( item_ammo_energy_ball, CItem_AR2AltFireRound);
LINK_ENTITY_TO_CLASS( ammo_ar2_altfire, CItem_AR2AltFireRound)
LINK_ENTITY_TO_CLASS( ammo_energyball, CItem_AR2AltFireRound);
LINK_ENTITY_TO_CLASS( ammo_energy_ball, CItem_AR2AltFireRound);
#endif
#ifdef DARKINTERVAL
// ========================================================================
//	>> OICW Ammo
// ========================================================================
class CItem_OICW_Mag : public CItem
{
public:
	DECLARE_CLASS(CItem_OICW_Mag, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/_Weapons/oicw_ammo_mag.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/_Weapons/oicw_ammo_mag.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_OICW_MAG, "OICW"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_oicw_ammo, CItem_OICW_Mag);
LINK_ENTITY_TO_CLASS(item_ammo_oicw, CItem_OICW_Mag);
LINK_ENTITY_TO_CLASS(item_ammo_oicw_mag, CItem_OICW_Mag);
LINK_ENTITY_TO_CLASS(ammo_oicw, CItem_OICW_Mag);
LINK_ENTITY_TO_CLASS(ammo_oicw_mag, CItem_OICW_Mag);

class CItem_OICW_Box : public CItem
{
public:
	DECLARE_CLASS(CItem_OICW_Box, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/_Weapons/oicw_ammo_box.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/_Weapons/oicw_ammo_box.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_OICW_BOX, "OICW"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_box_oicw, CItem_OICW_Box);
LINK_ENTITY_TO_CLASS(item_oicw_box, CItem_OICW_Box);
LINK_ENTITY_TO_CLASS(item_ammo_oicw_box, CItem_OICW_Box);
LINK_ENTITY_TO_CLASS(ammo_oicw_box, CItem_OICW_Box);

// ========================================================================
//	>> OICW Grenades Ammo
// ========================================================================
class CItem_BoxOICWGrenades : public CItem
{
public:
	DECLARE_CLASS(CItem_BoxOICWGrenades, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/_Weapons/oicw_ammo_grenade_box.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/_Weapons/oicw_ammo_grenade_box.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, 6, "OICW_Grenade"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_ammo_oicw_grenades, CItem_BoxOICWGrenades);
//LINK_ENTITY_TO_CLASS(item_ammo_oicw_altfire, CItem_BoxOICWGrenades);
LINK_ENTITY_TO_CLASS(item_oicw_grenades, CItem_BoxOICWGrenades);
//LINK_ENTITY_TO_CLASS(item_oicw_altfire, CItem_BoxOICWGrenades);
//LINK_ENTITY_TO_CLASS(ammo_oicw_grenade, CItem_BoxOICWGrenades);
LINK_ENTITY_TO_CLASS(ammo_oicw_grenades, CItem_BoxOICWGrenades);
//LINK_ENTITY_TO_CLASS(ammo_oicw_altfire, CItem_BoxOICWGrenades);

// ========================================================================
//	>> OICW Single Grenade Ammo
// ========================================================================
class CItem_BoxOICWGrenade : public CItem
{
public:
	DECLARE_CLASS(CItem_BoxOICWGrenade, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/_Weapons/oicw_ammo_grenade.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/_Weapons/oicw_ammo_grenade.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, 1, "OICW_Grenade"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_ammo_oicw_grenade, CItem_BoxOICWGrenade);
LINK_ENTITY_TO_CLASS(item_ammo_oicw_altfire, CItem_BoxOICWGrenade);
LINK_ENTITY_TO_CLASS(item_oicw_altfire, CItem_BoxOICWGrenade);
LINK_ENTITY_TO_CLASS(item_oicw_grenade, CItem_BoxOICWGrenade);
LINK_ENTITY_TO_CLASS(ammo_oicw_grenade, CItem_BoxOICWGrenade);
LINK_ENTITY_TO_CLASS(ammo_oicw_altfire, CItem_BoxOICWGrenade);


// ========================================================================
//	>> Tau Rifle Ammo
// ========================================================================
class CItem_Tau_Ammo : public CItem
{
public:
	DECLARE_CLASS(CItem_Tau_Ammo, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/_Weapons/tau_ammo.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/_Weapons/tau_ammo.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_TAU, "TauRound"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_tau_ammo, CItem_Tau_Ammo);
LINK_ENTITY_TO_CLASS(item_ammo_tau, CItem_Tau_Ammo);
LINK_ENTITY_TO_CLASS(ammo_tau, CItem_Tau_Ammo);
LINK_ENTITY_TO_CLASS(item_gauss_ammo, CItem_Tau_Ammo);
LINK_ENTITY_TO_CLASS(item_ammo_gauss, CItem_Tau_Ammo);
LINK_ENTITY_TO_CLASS(ammo_gauss, CItem_Tau_Ammo);

class CItem_Tau_Ammo_Box : public CItem
{
public:
	DECLARE_CLASS(CItem_Tau_Ammo_Box, CItem);

	void Spawn(void)
	{
		Precache();
		SetModel("models/_Weapons/tau_ammo_box.mdl");
		BaseClass::Spawn();
	}
	void Precache(void)
	{
		PrecacheModel("models/_Weapons/tau_ammo_box.mdl");
	}
	bool MyTouch(CBasePlayer *pPlayer)
	{
		if (ITEM_GiveAmmo(pPlayer, SIZE_AMMO_TAU_BOX, "TauRound"))
		{
			if (g_pGameRules->ItemShouldRespawn(this) == GR_ITEM_RESPAWN_NO)
			{
				UTIL_Remove(this);
			}
			return true;
		}
		return false;
	}
};
LINK_ENTITY_TO_CLASS(item_tau_ammo_box, CItem_Tau_Ammo_Box);
LINK_ENTITY_TO_CLASS(item_ammo_tau_box, CItem_Tau_Ammo_Box);
LINK_ENTITY_TO_CLASS(ammo_tau_box, CItem_Tau_Ammo_Box);
LINK_ENTITY_TO_CLASS(item_gauss_ammo_box, CItem_Tau_Ammo_Box);
LINK_ENTITY_TO_CLASS(item_ammo_gauss_box, CItem_Tau_Ammo_Box);
LINK_ENTITY_TO_CLASS(ammo_gauss_box, CItem_Tau_Ammo_Box);
#endif // DARKINTERVAL
// ==================================================================
// Ammo crate which will supply infinite ammo of the specified type
// ==================================================================

// Ammo types
enum
{
	AMMOCRATE_SMALL_ROUNDS,
	AMMOCRATE_MEDIUM_ROUNDS,
	AMMOCRATE_LARGE_ROUNDS,
	AMMOCRATE_RPG_ROUNDS,
	AMMOCRATE_BUCKSHOT,
	AMMOCRATE_GRENADES,
	AMMOCRATE_357,
	AMMOCRATE_CROSSBOW,
	AMMOCRATE_AR2_ALTFIRE,
	AMMOCRATE_SMG_ALTFIRE,
#ifdef DARKINTERVAL
	AMMOCRATE_OICW,
	AMMOCRATE_RESERVE1,
	AMMOCRATE_RESERVE2,
	AMMOCRATE_RESERVE3,
	AMMOCRATE_RESERVE4,
	AMMOCRATE_RESERVE5,
	AMMOCRATE_EMPTY,
#endif
	NUM_AMMO_CRATE_TYPES,
};

// Ammo crate

class CItem_AmmoCrate : public CBaseAnimating
{
public:
	DECLARE_CLASS( CItem_AmmoCrate, CBaseAnimating );

	void	Spawn( void );
	void	Precache( void );
	bool	CreateVPhysics( void );

	virtual void HandleAnimEvent( animevent_t *pEvent );

	void	SetupCrate( void );
	void	OnRestore( void );

	//FIXME: May not want to have this used in a radius
	int		ObjectCaps( void ) { return (BaseClass::ObjectCaps() | (FCAP_IMPULSE_USE|FCAP_USE_IN_RADIUS)); };
	void	Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	void	InputKill( inputdata_t &data );
	void	CrateThink( void );
	
	virtual int OnTakeDamage( const CTakeDamageInfo &info );

protected:

	int		m_nAmmoType;
	int		m_nAmmoIndex;

	static const char *m_lpzModelNames[NUM_AMMO_CRATE_TYPES];
	static const char *m_lpzAmmoNames[NUM_AMMO_CRATE_TYPES];
	static int m_nAmmoAmounts[NUM_AMMO_CRATE_TYPES];
	static const char *m_pGiveWeapon[NUM_AMMO_CRATE_TYPES];

	float	m_flCloseTime;
	COutputEvent	m_OnUsed;
	CHandle< CBasePlayer > m_hActivator;

	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS( item_ammo_crate, CItem_AmmoCrate );

BEGIN_DATADESC( CItem_AmmoCrate )

	DEFINE_KEYFIELD( m_nAmmoType,	FIELD_INTEGER, "AmmoType" ),	

	DEFINE_FIELD( m_flCloseTime, FIELD_FLOAT ),
	DEFINE_FIELD( m_hActivator, FIELD_EHANDLE ),

	// These can be recreated
	//DEFINE_FIELD( m_nAmmoIndex,		FIELD_INTEGER ),
	//DEFINE_FIELD( m_lpzModelNames,	FIELD_ ),
	//DEFINE_FIELD( m_lpzAmmoNames,	FIELD_ ),
	//DEFINE_FIELD( m_nAmmoAmounts,	FIELD_INTEGER ),

	DEFINE_OUTPUT( m_OnUsed, "OnUsed" ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Kill", InputKill ),

	DEFINE_THINKFUNC( CrateThink ),

END_DATADESC()

//-----------------------------------------------------------------------------
// Animation events.
//-----------------------------------------------------------------------------

// Models names
const char *CItem_AmmoCrate::m_lpzModelNames[NUM_AMMO_CRATE_TYPES] =
{
	"models/items/ammocrate_pistol.mdl",	// Small rounds
	"models/items/ammocrate_smg1.mdl",		// Medium rounds
	"models/items/ammocrate_ar2.mdl",		// Large rounds
	"models/items/ammocrate_rockets.mdl",	// RPG rounds
	"models/items/ammocrate_buckshot.mdl",	// Buckshot
	"models/items/ammocrate_grenade.mdl",	// Grenades
#ifdef DARKINTERVAL
	"models/items/ammocrate_357.mdl",		// 357
#else
	"models/items/ammocrate_smg1.mdl",		// 357
#endif
	"models/items/ammocrate_smg1.mdl",		// Crossbow
	
	//FIXME: This model is incorrect!
	"models/items/ammocrate_ar2.mdl",		// Combine Ball 
	"models/items/ammocrate_smg2.mdl",	    // smg grenade
#ifdef DARKINTERVAL
	"models/items/ammocrate_oicw.mdl",	    // OICW rounds

	//TEMP: reserve ammo crates for the future
	"models/items/ammocrate_pistol.mdl",		// 
	"models/items/ammocrate_pistol.mdl",	    // 
	"models/items/ammocrate_pistol.mdl",	    //
	"models/items/ammocrate_pistol.mdl",	    // 
	"models/items/ammocrate_pistol.mdl",	    // 

	// Empty crate
	"models/items/ammocrate_empty.mdl",			// meant to look like AR2 crate
#endif
};

// Ammo type names
const char *CItem_AmmoCrate::m_lpzAmmoNames[NUM_AMMO_CRATE_TYPES] =
{
	"Pistol",		
	"SMG1",			
	"AR2",			
	"RPG_Round",	
	"Buckshot",		
	"Grenade",
	"357",
	"XBowBolt",
	"AR2AltFire",
	"SMG1_Grenade",
#ifdef DARKINTERVAL
	"OICW",
	// reserve crates
	"Pistol",
	"Pistol",
	"Pistol",
	"Pistol",
	"Pistol",
	//
	"Pistol",
#endif
};

// Ammo amount given per +use
int CItem_AmmoCrate::m_nAmmoAmounts[NUM_AMMO_CRATE_TYPES] =
{
#ifdef DARKINTERVAL
	300,	// Pistol
	300,	// SMG1
	300,	// AR2
	5,		// RPG rounds
	120,	// Buckshot
	8,		// Grenades
	50,		// 357
	50,		// Crossbow
	5,		// AR2 alt-fire
	5,
	80, // oicw
	// reserve crates
	10,
	10,
	10,
	10,
	10,
	// empty crate
	0,
#else
	300,	// Pistol
	300,	// SMG1
	300,	// AR2
	3,		// RPG rounds
	120,	// Buckshot
	5,		// Grenades
	50,		// 357
	50,		// Crossbow
	3,		// AR2 alt-fire
	5,
#endif
};

const char *CItem_AmmoCrate::m_pGiveWeapon[NUM_AMMO_CRATE_TYPES] =
{
	NULL,	// Pistol
	NULL,	// SMG1
	NULL,	// AR2
	NULL,		// RPG rounds
	NULL,	// Buckshot
	"weapon_frag",		// Grenades
	NULL,		// 357
	NULL,		// Crossbow
	NULL,		// AR2 alt-fire
	NULL,		// SMG alt-fire
#ifdef DARKINTERVAL
	NULL,		// oicw
	// reserve crates
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	// empty
	NULL,
#endif
};

#define	AMMO_CRATE_CLOSE_DELAY	1.5f

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::Spawn( void )
{
	Precache();

	BaseClass::Spawn();

	SetModel( STRING( GetModelName() ) );
	SetMoveType( MOVETYPE_NONE );
	SetSolid( SOLID_VPHYSICS );
	CreateVPhysics();

	ResetSequence( LookupSequence( "Idle" ) );
	SetBodygroup( 1, true );

	m_flCloseTime = CURTIME;
	m_flAnimTime = CURTIME;
	m_flPlaybackRate = 0.0;
	SetCycle( 0 );

	m_takedamage = DAMAGE_EVENTS_ONLY;

}

//------------------------------------------------------------------------------
// Purpose:
//------------------------------------------------------------------------------
bool CItem_AmmoCrate::CreateVPhysics( void )
{
	return ( VPhysicsInitStatic() != NULL );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::Precache( void )
{
	SetupCrate();
	PrecacheModel( STRING( GetModelName() ) );

	PrecacheScriptSound( "AmmoCrate.Open" );
	PrecacheScriptSound( "AmmoCrate.Close" );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::SetupCrate( void )
{
	SetModelName( AllocPooledString( m_lpzModelNames[m_nAmmoType] ) );
	
	m_nAmmoIndex = GetAmmoDef()->Index( m_lpzAmmoNames[m_nAmmoType] );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::OnRestore( void )
{
	BaseClass::OnRestore();

	// Restore our internal state
	SetupCrate();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pActivator - 
//			*pCaller - 
//			useType - 
//			value - 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::Use( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBasePlayer *pPlayer = ToBasePlayer( pActivator );

	if ( pPlayer == NULL )
		return;

	m_OnUsed.FireOutput( pActivator, this );

	int iSequence = LookupSequence( "Open" );

	// See if we're not opening already
	if ( GetSequence() != iSequence )
	{
		Vector mins, maxs;
		trace_t tr;

		CollisionProp()->WorldSpaceAABB( &mins, &maxs );

		Vector vOrigin = GetAbsOrigin();
		vOrigin.z += ( maxs.z - mins.z );
		mins = (mins - GetAbsOrigin()) * 0.2f;
		maxs = (maxs - GetAbsOrigin()) * 0.2f;
		mins.z = ( GetAbsOrigin().z - vOrigin.z );  
		
		UTIL_TraceHull( vOrigin, vOrigin, mins, maxs, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr );

		if ( tr.startsolid || tr.allsolid )
			 return;
			
		m_hActivator = pPlayer;

		// Animate!
		ResetSequence( iSequence );

		// Make sound
		CPASAttenuationFilter sndFilter( this, "AmmoCrate.Open" );
		EmitSound( sndFilter, entindex(), "AmmoCrate.Open" );

		// Start thinking to make it return
		SetThink( &CItem_AmmoCrate::CrateThink );
		SetNextThink( CURTIME + 0.1f );
	}

	// Don't close again for two seconds
	m_flCloseTime = CURTIME + AMMO_CRATE_CLOSE_DELAY;
}

//-----------------------------------------------------------------------------
// Purpose: allows the crate to open up when hit by a crowbar
//-----------------------------------------------------------------------------
int CItem_AmmoCrate::OnTakeDamage( const CTakeDamageInfo &info )
{
	// if it's the player hitting us with a crowbar, open up
	CBasePlayer *player = ToBasePlayer(info.GetAttacker());
	if (player)
	{
		CBaseCombatWeapon *weapon = player->GetActiveWeapon();

		if (weapon && !stricmp(weapon->GetName(), "weapon_crowbar"))
		{
			// play the normal use sound
			player->EmitSound( "HL2Player.Use" );
			// open the crate
			Use(info.GetAttacker(), info.GetAttacker(), USE_TOGGLE, 0.0f);
		}
	}

	// don't actually take any damage
	return 0;
}


//-----------------------------------------------------------------------------
// Purpose: Catches the monster-specific messages that occur when tagged
//			animation frames are played.
// Input  : *pEvent - 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::HandleAnimEvent( animevent_t *pEvent )
{
	if ( pEvent->event == AE_AMMOCRATE_PICKUP_AMMO )
	{
		if ( m_hActivator )
		{
			if ( m_pGiveWeapon[m_nAmmoType] && !m_hActivator->Weapon_OwnsThisType( m_pGiveWeapon[m_nAmmoType] ) )
			{
				CBaseEntity *pEntity = CreateEntityByName( m_pGiveWeapon[m_nAmmoType] );
				CBaseCombatWeapon *pWeapon = dynamic_cast<CBaseCombatWeapon*>(pEntity);
				if ( pWeapon )
				{
					pWeapon->SetAbsOrigin( m_hActivator->GetAbsOrigin() );
					pWeapon->m_iPrimaryAmmoType = 0;
					pWeapon->m_iSecondaryAmmoType = 0;
					pWeapon->Spawn();
					if ( !m_hActivator->BumpWeapon( pWeapon ) )
					{
						UTIL_Remove( pEntity );
					}
					else
					{
						SetBodygroup( 1, false );
					}
				}
			}

			if ( m_hActivator->GiveAmmo( m_nAmmoAmounts[m_nAmmoType], m_nAmmoIndex ) != 0 )
			{
				SetBodygroup( 1, false );
			}
			m_hActivator = NULL;
		}
		return;
	}

	BaseClass::HandleAnimEvent( pEvent );
}

	
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::CrateThink( void )
{
	StudioFrameAdvance();
	DispatchAnimEvents( this );

	SetNextThink( CURTIME + 0.1f );

	// Start closing if we're not already
	if ( GetSequence() != LookupSequence( "Close" ) )
	{
		// Not ready to close?
		if ( m_flCloseTime <= CURTIME )
		{
			m_hActivator = NULL;

			ResetSequence( LookupSequence( "Close" ) );
		}
	}
	else
	{
		// See if we're fully closed
		if ( IsSequenceFinished() )
		{
			// Stop thinking
			SetThink( NULL );
			CPASAttenuationFilter sndFilter( this, "AmmoCrate.Close" );
			EmitSound( sndFilter, entindex(), "AmmoCrate.Close" );

			// FIXME: We're resetting the sequence here
			// but setting Think to NULL will cause this to never have
			// StudioFrameAdvance called. What are the consequences of that?
			ResetSequence( LookupSequence( "Idle" ) );
			SetBodygroup( 1, true );
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CItem_AmmoCrate::InputKill( inputdata_t &data )
{
	UTIL_Remove( this );
}
