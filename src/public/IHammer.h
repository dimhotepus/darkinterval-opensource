//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: The application object.
//
//=============================================================================//

#ifndef IHAMMER_H
#define IHAMMER_H

#include "appframework/IAppSystem.h"

typedef struct tagMSG MSG;


class IStudioDataCache;

#define SLE //// SLE NEW - holds all the additions and changes done in SLE

//#define HAMMER2013_PORT_KEYBINDS //// SLE NEW - custom keybinds (from sdk-2013-hammer)
//#define HAMMER2013_PORT_SAVE_ON_CRASH //// SLE NEW - ported from sdk-2013-hammer - autosave on crash
#define HAMMER2013_PORT_SPRITES_DEPTH //// SLE NEW - ported from sdk-2013-hammer - don't disable depth test on sprites in editor // wip
#define HAMMER2013_PORT_TBROWSER_TRANSPARENCY //// SLE NEW - ported from sdk-2013-hammer - display transparent textures in texture browser
//#define HAMMER2013_PORT_TBROWSER_NEWCACHE //// SLE NEW - ported from sdk-2013-hammer - display transparent textures in texture browser
//#define HAMMER2013_PORT_MISSING_CHECKER //// SLE NEW - ported from sdk-2013-hammer - display missing texture as emo checkerboard
#define HAMMER2013_PORT_FMOD
//#define HAMMER2013_PORT_CORDONS
#define HAMMER2013_PORT_PROXIES //// SLE NEW - ported from sdk-2013-hammer - material proxies, expanded
//#define SLE_2D_BACKGROUNDS //// SLE NEW - background images
//-----------------------------------------------------------------------------
// Return values for RequestNewConfig
//-----------------------------------------------------------------------------
enum RequestRetval_t
{
	REQUEST_OK = 0,
	REQUEST_QUIT
};


//-----------------------------------------------------------------------------
// Interface used to drive hammer
//-----------------------------------------------------------------------------
#define INTERFACEVERSION_HAMMER	"Hammer001"
class IHammer : public IAppSystem
{
public:
	virtual bool HammerPreTranslateMessage( MSG * pMsg ) = 0;
	virtual bool HammerIsIdleMessage( MSG * pMsg ) = 0;
	virtual bool HammerOnIdle( long count ) = 0;

	virtual void RunFrame() = 0;

	// Returns the mod and the game to initially start up
	virtual const char *GetDefaultMod() = 0;
	virtual const char *GetDefaultGame() = 0;

	virtual bool InitSessionGameConfig( const char *szGameDir ) = 0;

	// Request a new config from hammer's config system
	virtual RequestRetval_t RequestNewConfig() = 0;

	// Returns the full path to the mod and the game to initially start up
	virtual const char *GetDefaultModFullPath() = 0;

	virtual int MainLoop() = 0;
};	

#endif // IHAMMER_H
