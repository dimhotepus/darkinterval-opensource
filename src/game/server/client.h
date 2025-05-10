//========================================================================//
//
// Purpose: 
// DI changes that are not ifdef'd: 
// - removed all CS/TF/MP stuff
// $NoKeywords: $
//
//=============================================================================//
#ifndef CLIENT_H
#define CLIENT_H

#ifdef _WIN32
#pragma once
#endif

class CCommand;
class CUserCmd;
class CBasePlayer;

void ClientActive( edict_t *pEdict, bool bLoadGame );
void ClientPutInServer( edict_t *pEdict, const char *playername );
void ClientCommand( CBasePlayer *pSender, const CCommand &args );
void ClientPrecache( void );
// Game specific precaches
void ClientGamePrecache( void );
const char *GetGameDescription( void );
#endif		// CLIENT_H
