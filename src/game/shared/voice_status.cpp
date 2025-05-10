//========================================================================//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "basetypes.h"
#include "hud.h"
#include <string.h>
#include <stdio.h>
#include "voice_status.h"
#include "r_efx.h"
#include <vgui_controls/TextImage.h>
#include <vgui/MouseCode.h>
#include "cdll_client_int.h"
#include "hud_macros.h"
#include "c_playerresource.h"
#include "cliententitylist.h"
#include "c_baseplayer.h"
#include "materialsystem/imesh.h"
#include "view.h"
#include "convar.h"
#include <vgui_controls/Controls.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include "vgui_bitmapimage.h"
#include "materialsystem/imaterial.h"
#include "tier0/dbg.h"
#include "cdll_int.h"
#include <vgui/IPanel.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
using namespace vgui;
#define VOICE_MODEL_INTERVAL		0.3
#define SQUELCHOSCILLATE_PER_SECOND	2.0f
#ifndef DARKINTERVAL // doesn't use it because singleplayer
extern int cam_thirdperson;
ConVar voice_modenable( "voice_modenable", "1", FCVAR_ARCHIVE | FCVAR_CLIENTCMD_CAN_EXECUTE, "Enable/disable voice in this mod." );
ConVar voice_clientdebug( "voice_clientdebug", "0" );
#endif
// ---------------------------------------------------------------------- //
// The voice manager for the client.
// ---------------------------------------------------------------------- //
static CVoiceStatus *g_VoiceStatus = NULL;

CVoiceStatus* GetClientVoiceMgr()
{
	if ( !g_VoiceStatus )
	{
		ClientVoiceMgr_Init();
	}

	return g_VoiceStatus;
}

void ClientVoiceMgr_Init()
{
	if ( g_VoiceStatus )
		return;

	g_VoiceStatus = new CVoiceStatus();
}

void ClientVoiceMgr_Shutdown()
{
	delete g_VoiceStatus;
	g_VoiceStatus = NULL;
}

// ---------------------------------------------------------------------- //
// CVoiceStatus.
// ---------------------------------------------------------------------- //

static CVoiceStatus *g_pInternalVoiceStatus = NULL;

void __MsgFunc_VoiceMask(bf_read &msg)
{
	if(g_pInternalVoiceStatus)
		g_pInternalVoiceStatus->HandleVoiceMaskMsg(msg);
}

void __MsgFunc_RequestState(bf_read &msg)
{
	if(g_pInternalVoiceStatus)
		g_pInternalVoiceStatus->HandleReqStateMsg(msg);
}


// ---------------------------------------------------------------------- //
// CVoiceStatus.
// ---------------------------------------------------------------------- //

CVoiceStatus::CVoiceStatus()
{
	m_nControlSize = 0;
	m_bBanMgrInitialized = false;
	m_LastUpdateServerState = 0;

	m_bTalking = m_bServerAcked = false;

#ifdef VOICE_VOX_ENABLE
	m_bAboveThresholdTimer.Invalidate();
#endif // VOICE_VOX_ENABLE

	m_bServerModEnable = -1;

	m_pHeadLabelMaterial = NULL;

	m_bHeadLabelsDisabled = false;
}


CVoiceStatus::~CVoiceStatus()
{
	if ( m_pHeadLabelMaterial )
	{
		m_pHeadLabelMaterial->DecrementReferenceCount();
	}

	g_pInternalVoiceStatus = NULL;			
#ifndef DARKINTERVAL
	const char *pGameDir = engine->GetGameDirectory();
	if( pGameDir )
	{
		if(m_bBanMgrInitialized)
		{
			m_BanMgr.SaveState( pGameDir );
		}
	}
#endif
}

int CVoiceStatus::Init(
	IVoiceStatusHelper *pHelper,
	VPANEL pParentPanel)
{
#ifndef DARKINTERVAL // doesn't use it because singleplayer
	const char *pGameDir = engine->GetGameDirectory();
	if( pGameDir )
	{
		m_BanMgr.Init( pGameDir );
		m_bBanMgrInitialized = true;
	}

	Assert(!g_pInternalVoiceStatus);
	g_pInternalVoiceStatus = this;


	m_pHeadLabelMaterial = materials->FindMaterial( "voice/icntlk_pl", TEXTURE_GROUP_VGUI );
	m_pHeadLabelMaterial->IncrementReferenceCount();

	m_bInSquelchMode = false;

	m_pHelper = pHelper;
	m_pParentPanel = pParentPanel;

	HOOK_MESSAGE(VoiceMask);
	HOOK_MESSAGE(RequestState);
#endif
	return 1;
}

BitmapImage* vgui_LoadMaterial( vgui::VPANEL pParent, const char *pFilename )
{
	return new BitmapImage( pParent, pFilename );
}

void CVoiceStatus::VidInit()
{
}

void CVoiceStatus::Frame(double frametime)
{
	// check server banned players once per second
	if (CURTIME - m_LastUpdateServerState > 1)
	{
		UpdateServerState(false);
	}
}

float g_flHeadOffset = 35;
float g_flHeadIconSize = 8;

void CVoiceStatus::SetHeadLabelOffset( float offset )
{
	g_flHeadOffset = offset;
}

float CVoiceStatus::GetHeadLabelOffset( void ) const
{
	return g_flHeadOffset;
}

void CVoiceStatus::DrawHeadLabels()
{
#ifndef DARKINTERVAL
	if (m_bHeadLabelsDisabled)
		return;

	if (GameRules() && (GameRules()->ShouldDrawHeadLabels() == false))
		return;

	if (!m_pHeadLabelMaterial)
		return;

	CMatRenderContextPtr pRenderContext(materials);

	for (int i = 0; i < VOICE_MAX_PLAYERS; i++)
	{
		if (!m_VoicePlayers[i])
			continue;

		IClientNetworkable *pClient = cl_entitylist->GetClientEntity(i + 1);

		// Don't show an icon if the player is not in our PVS.
		if (!pClient || pClient->IsDormant())
			continue;

		C_BasePlayer *pPlayer = dynamic_cast<C_BasePlayer*>(pClient);
		if (!pPlayer)
			continue;

		// Don't show an icon for dead or spectating players (ie: invisible entities).
		if (pPlayer->IsPlayerDead())
			continue;

		// Place it 20 units above his head.
		Vector vOrigin = pPlayer->WorldSpaceCenter();
		vOrigin.z += g_flHeadOffset;


		// Align it so it never points up or down.
		Vector vUp(0, 0, 1);
		Vector vRight = CurrentViewRight();
		if (fabs(vRight.z) > 0.95)	// don't draw it edge-on
			continue;

		vRight.z = 0;
		VectorNormalize(vRight);


		float flSize = g_flHeadIconSize;

		pRenderContext->Bind(pPlayer->GetHeadLabelMaterial());
		IMesh *pMesh = pRenderContext->GetDynamicMesh();
		CMeshBuilder meshBuilder;
		meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);

		meshBuilder.Color3f(1.0, 1.0, 1.0);
		meshBuilder.TexCoord2f(0, 0, 0);
		meshBuilder.Position3fv((vOrigin + (vRight * -flSize) + (vUp * flSize)).Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.Color3f(1.0, 1.0, 1.0);
		meshBuilder.TexCoord2f(0, 1, 0);
		meshBuilder.Position3fv((vOrigin + (vRight * flSize) + (vUp * flSize)).Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.Color3f(1.0, 1.0, 1.0);
		meshBuilder.TexCoord2f(0, 1, 1);
		meshBuilder.Position3fv((vOrigin + (vRight * flSize) + (vUp * -flSize)).Base());
		meshBuilder.AdvanceVertex();

		meshBuilder.Color3f(1.0, 1.0, 1.0);
		meshBuilder.TexCoord2f(0, 0, 1);
		meshBuilder.Position3fv((vOrigin + (vRight * -flSize) + (vUp * -flSize)).Base());
		meshBuilder.AdvanceVertex();
		meshBuilder.End();
		pMesh->Draw();
	}
#else
	return;
#endif
}

void CVoiceStatus::UpdateSpeakerStatus(int entindex, bool bTalking)
{
#ifndef DARKINTERVAL
	if(!m_pParentPanel)
		return;

	if( voice_clientdebug.GetInt() )
	{
		Msg( "CVoiceStatus::UpdateSpeakerStatus: ent %d talking = %d\n", entindex, bTalking );
	}

	// Is it the local player talking?
	if( entindex == -1 )
	{
		m_bTalking = !!bTalking;
		if( bTalking )
		{
			// Enable voice for them automatically if they try to talk.
			engine->ClientCmd( "voice_modenable 1" );
		}
	}
	else if( entindex == -2 )
	{
		m_bServerAcked = !!bTalking;
	}
#ifdef VOICE_VOX_ENABLE
	else if( entindex == -3 )
	{
		if ( bTalking )
		{
			const float AboveThresholdMinDuration = 0.5f;
			m_bAboveThresholdTimer.Start( AboveThresholdMinDuration );
		}
	}
#endif // VOICE_VOX_ENABLE
	else if(entindex > 0 && entindex <= VOICE_MAX_PLAYERS)
	{
		int iClient = entindex - 1;
		if(iClient < 0)
			return;

		if(bTalking)
		{
			m_VoicePlayers[iClient] = true;
			m_VoiceEnabledPlayers[iClient] = true;
		}
		else
		{
			m_VoicePlayers[iClient] = false;
		}
	}
#else
	return;
#endif
}


void CVoiceStatus::UpdateServerState(bool bForce)
{
#ifndef DARKINTERVAL
	// Can't do anything when we're not in a level.
	if( !g_bLevelInitialized )
	{
		if( voice_clientdebug.GetInt() )
		{
			Msg( "CVoiceStatus::UpdateServerState: g_bLevelInitialized\n" );
		}

		return;
	}
	
	int bCVarModEnable = !!voice_modenable.GetInt();
	if(bForce || m_bServerModEnable != bCVarModEnable)
	{
		m_bServerModEnable = bCVarModEnable;

		char str[256];
		Q_snprintf(str, sizeof(str), "VModEnable %d", m_bServerModEnable);
		engine->ServerCmd(str);

		if( voice_clientdebug.GetInt() )
		{
			Msg( "CVoiceStatus::UpdateServerState: Sending '%s'\n", str );
		}
	}

	char str[2048];
	Q_strncpy(str,"vban",sizeof(str));
	bool bChange = false;

	for(unsigned long dw=0; dw < VOICE_MAX_PLAYERS_DW; dw++)
	{	
		unsigned long serverBanMask = 0;
		unsigned long banMask = 0;
		for(unsigned long i=0; i < 32; i++)
		{
			int playerIndex = ( dw * 32 + i );
			if ( playerIndex >= MAX_PLAYERS )
				break;

			player_info_t pi;

			if ( !engine->GetPlayerInfo( i+1, &pi ) )
				continue;

			if ( m_BanMgr.GetPlayerBan( pi.guid ) )
			{
				banMask |= 1 << i;
			}

			if ( m_ServerBannedPlayers[playerIndex] )
			{
				serverBanMask |= 1 << i;
			}
		}

		if ( serverBanMask != banMask )
		{
			bChange = true;
		}

		// Ok, the server needs to be updated.
		char numStr[512];
		Q_snprintf(numStr, sizeof(numStr), " %lx", banMask);
		Q_strncat(str, numStr, sizeof(str), COPY_ALL_CHARACTERS);
	}

	if(bChange || bForce)
	{
		if( voice_clientdebug.GetInt() )
		{
			Msg( "CVoiceStatus::UpdateServerState: Sending '%s'\n", str );
		}

		engine->ServerCmd( str, false );	// Tell the server..
	}
	else
	{
		if( voice_clientdebug.GetInt() )
		{
			Msg( "CVoiceStatus::UpdateServerState: no change\n" );
		}
	}
	
	m_LastUpdateServerState = CURTIME;
#else
	return;
#endif
}

void CVoiceStatus::HandleVoiceMaskMsg(bf_read &msg)
{
#ifndef DARKINTERVAL
	unsigned int dw;
	for(dw=0; dw < VOICE_MAX_PLAYERS_DW; dw++)
	{
		m_AudiblePlayers.SetDWord(dw, (unsigned long)msg.ReadLong());
		m_ServerBannedPlayers.SetDWord(dw, (unsigned long)msg.ReadLong());

		if( voice_clientdebug.GetInt())
		{
			Msg("CVoiceStatus::HandleVoiceMaskMsg\n");
			Msg("    - m_AudiblePlayers[%d] = %u\n", dw, m_AudiblePlayers.GetDWord(dw));
			Msg("    - m_ServerBannedPlayers[%d] = %u\n", dw, m_ServerBannedPlayers.GetDWord(dw));
		}
	}

	m_bServerModEnable = msg.ReadByte();
#else
	return;
#endif
}

void CVoiceStatus::HandleReqStateMsg(bf_read &msg)
{
#ifndef DARKINTERVAL
	if(voice_clientdebug.GetInt())
	{
		Msg("CVoiceStatus::HandleReqStateMsg\n");
	}

	UpdateServerState(true);	
#else
	return;
#endif
}

void CVoiceStatus::StartSquelchMode()
{
#ifndef DARKINTERVAL
	if(m_bInSquelchMode)
		return;

	m_bInSquelchMode = true;
	m_pHelper->UpdateCursorState();
#else
	return;
#endif
}

void CVoiceStatus::StopSquelchMode()
{
#ifndef DARKINTERVAL
	m_bInSquelchMode = false;
	m_pHelper->UpdateCursorState();
#else
	return;
#endif
}

bool CVoiceStatus::IsInSquelchMode()
{
#ifndef DARKINTERVAL
	return m_bInSquelchMode;
#else
	return false;
#endif
}

void SetOrUpdateBounds( 
	vgui::Panel *pPanel, 
	int left, int top, int wide, int tall, 
	bool bOnlyUpdateBounds, int &topCoord, int &bottomCoord )
{
	if ( bOnlyUpdateBounds )
	{
		if ( top < topCoord )
			topCoord = top;

		if ( (top+tall) >= bottomCoord )
			bottomCoord = top+tall;
	}
	else
	{
		pPanel->SetBounds( left, top, wide, tall );
	}
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the target client has been banned
// Input  : playerID - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CVoiceStatus::IsPlayerBlocked(int iPlayer)
{
#ifndef DARKINTERVAL
	player_info_t pi;

	if ( !engine->GetPlayerInfo( iPlayer, &pi ) )
		return false;

	return m_BanMgr.GetPlayerBan( pi.guid );
#else
	return false;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the player can't hear the other client due to game rules (eg. the other team)
// Input  : playerID - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CVoiceStatus::IsPlayerAudible(int iPlayer)
{
#ifndef DARKINTERVAL
	return !!m_AudiblePlayers[iPlayer-1];
#else
	return false;
#endif
}


//-----------------------------------------------------------------------------
// returns true if the player is currently speaking
//-----------------------------------------------------------------------------
bool CVoiceStatus::IsPlayerSpeaking(int iPlayerIndex)
{
#ifndef DARKINTERVAL
	return m_VoiceEnabledPlayers[iPlayerIndex - 1] != 0;
#else
	return false;
#endif
}

//-----------------------------------------------------------------------------
// returns true if the local player is attempting to speak
//-----------------------------------------------------------------------------
bool CVoiceStatus::IsLocalPlayerSpeaking( void )
{
#ifndef DARKINTERVAL
#ifdef VOICE_VOX_ENABLE
	if ( voice_vox.GetBool() )
	{
		if ( m_bAboveThresholdTimer.IsElapsed() == true )
		{
			return false;
		}
	}
#endif // VOICE_VOX_ENABLE

	return m_bTalking;
#else
	return false;
#endif
}


//-----------------------------------------------------------------------------
// Purpose: blocks/unblocks the target client from being heard
// Input  : playerID - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
void CVoiceStatus::SetPlayerBlockedState(int iPlayer, bool blocked)
{
#ifndef DARKINTERVAL
	if (voice_clientdebug.GetInt())
	{
		Msg( "CVoiceStatus::SetPlayerBlockedState part 1\n" );
	}

	player_info_t pi;
	if ( !engine->GetPlayerInfo( iPlayer, &pi ) )
#endif
		return;
#ifndef DARKINTERVAL
	if (voice_clientdebug.GetInt())
	{
		Msg( "CVoiceStatus::SetPlayerBlockedState part 2\n" );
	}

	// Squelch or (try to) unsquelch this player.
	if (voice_clientdebug.GetInt())
	{
		Msg("CVoiceStatus::SetPlayerBlockedState: setting player %d ban to %d\n", iPlayer, !m_BanMgr.GetPlayerBan(pi.guid));
	}

	m_BanMgr.SetPlayerBan(pi.guid, !m_BanMgr.GetPlayerBan(pi.guid));
	UpdateServerState(false);
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CVoiceStatus::SetHeadLabelMaterial( const char *pszMaterial )
{
	return;
#ifndef DARKINTERVAL
	if ( m_pHeadLabelMaterial )
	{
		m_pHeadLabelMaterial->DecrementReferenceCount();
		m_pHeadLabelMaterial = NULL;
	}

	m_pHeadLabelMaterial = materials->FindMaterial( pszMaterial, TEXTURE_GROUP_VGUI );
	m_pHeadLabelMaterial->IncrementReferenceCount();
#endif
}