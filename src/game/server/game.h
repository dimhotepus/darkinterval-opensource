//========================================================================//
//
// Purpose: 
// DI changes that are not ifdef'd: 
// - removed all CS/TF/MP stuff
// $NoKeywords: $
//
//=============================================================================//

#ifndef GAME_H
#define GAME_H


#include "globals.h"

extern void GameDLLInit( void );

extern ConVar	displaysoundlist;

extern ConVar	suitvolume;

// Engine Cvars
extern const ConVar *g_pDeveloper;
#endif		// GAME_H
