//========================================================================//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
/*

===== item_suit.cpp ========================================================

  handling for the player's suit.
*/

#include "cbase.h"
#include "player.h"
#include "gamerules.h"
#include "items.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
#ifdef DARKINTERVAL
#define SF_SUIT_SHORTLOGON		1 << 16
#define SF_SUIT_BROKENLOGON		1 << 17
#else
#define SF_SUIT_SHORTLOGON		0x0001
#endif
class CItemSuit : public CItem
{
public:
	DECLARE_CLASS( CItemSuit, CItem );

	void Spawn( void )
	{ 
		Precache( );
#ifdef DARKINTERVAL
		SetModel( "models/_Items/hev_mk5.mdl" );
#else
		SetModel("models/items/hevsuit.mdl");
#endif
		BaseClass::Spawn( );
		
		CollisionProp()->UseTriggerBounds( false, 0 );
	}
	void Precache( void )
	{
#ifdef DARKINTERVAL
		PrecacheModel ("models/_Items/hev_mk5.mdl");
#else
		PrecacheModel ("models/items/hevsuit.mdl");
#endif
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
		if ( pPlayer->IsSuitEquipped() )
			return FALSE; // todo: maybe replace with a charge instead? or salvage modules off other HEVs?

		if ( HasSpawnFlags( SF_SUIT_SHORTLOGON) )
#ifdef DARKINTERVAL
			UTIL_EmitSoundSuit(pPlayer->edict(), "!HEV_AAx");		// short version of suit logon,
#else	
			UTIL_EmitSoundSuit(pPlayer->edict(), "!HEV_A0");
#endif
		else
		{
#ifdef DARKINTERVAL
			if ( HasSpawnFlags(SF_SUIT_BROKENLOGON ))
				UTIL_EmitSoundSuit(pPlayer->edict(), "!HEV_Broken");	// long version of suit logon
#else
			UTIL_EmitSoundSuit(pPlayer->edict(), "!HEV_AAx");
#endif
		}

		pPlayer->EquipSuit();
				
		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_suit, CItemSuit);
