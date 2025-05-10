//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//
//=============================================================================//
/*

===== item_longjump.cpp ========================================================

  handling for the longjump module
*/

#include "cbase.h"
#include "player.h"
//#include "weapons.h"
#include "gamerules.h"
#include "items.h"

class CItemLongJump : public CItem
{
public:
	DECLARE_CLASS( CItemLongJump, CItem );

	void Spawn( void )
	{ 
		Precache( );
#ifdef DARKINTERVAL
		SetModel( "models/_Items/hev_module_a.mdl" );
#else
		SetModel("models/w_longjump.mdl");
#endif
		BaseClass::Spawn( );
	}
	void Precache( void )
	{
#ifdef DARKINTERVAL
		PrecacheModel ("models/_Items/hev_module_a.mdl");
		PrecacheScriptSound("ItemLongJump.Touch");
#endif
	}
	bool MyTouch( CBasePlayer *pPlayer )
	{
#ifdef DARKINTERVAL
		if ( pPlayer->IsLongJumpEquipped() )
#else
		if ( pPlayer->m_fLongJump )
#endif
		{
			return FALSE;
		}

		if ( pPlayer->IsSuitEquipped() )
		{
#ifdef DARKINTERVAL
			EmitSound("ItemLongJump.Touch");
			pPlayer->EquipLongJump();// player now has longjump module
#else
			pPlayer->m_fLongJump = TRUE;// player now has longjump module

			CSingleUserRecipientFilter user(pPlayer);
			user.MakeReliable();

			UserMessageBegin(user, "ItemPickup");
			WRITE_STRING(STRING(pev->classname));
			MessageEnd();

			UTIL_EmitSoundSuit(pPlayer->edict(), "!HEV_A1");	// Play the longjump sound UNDONE: Kelly? correct sound?
#endif
			return true;		
		}
		return false;
	}
};

LINK_ENTITY_TO_CLASS( item_longjump, CItemLongJump );
