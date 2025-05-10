//========================================================================//
//
// Purpose: Normal HUD mode
// DI changes that are not ifdef'd: 
// - removed all CS/TF/MP stuff
// $Workfile:     $
// $Date:         $
// $NoKeywords: $
//=============================================================================//


#include "cbase.h"
#include "clientmode_shared.h"
#include "iinput.h"
#include "view_shared.h"
#include "iviewrender.h"
#include "hud_basechat.h"
#include "weapon_selection.h"
#include <vgui/Ivgui.h>
#include <vgui/Cursor.h>
#include <vgui/IPanel.h>
#include <vgui/IInput.h>
#include "engine/ienginesound.h"
#include <keyvalues.h>
#include <vgui_controls/animationcontroller.h>
#include "vgui_int.h"
#include "hud_macros.h"
#include "particlemgr.h"
#include "c_vguiscreen.h"
#ifndef DARKINTERVAL // DI is singleplayer, so it ditched teams
#include "c_team.h"
#endif
#include "c_rumble.h"
#include "fmtstr.h"
#include "achievementmgr.h"
#include "c_playerresource.h"
#include "cam_thirdperson.h"
#include <vgui/ilocalize.h>
#include "hud_vote.h"
#include "ienginevgui.h"
#ifndef DARKINTERVAL // remove vr stuff
#include "sourcevr/isourcevirtualreality.h"
#endif
#ifdef DARKINTERVAL
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "glow_outline_effect.h"
#endif
#ifdef DARKINTERVAL
//#define NEW_LOADING_SCREENS
#endif
//Fenix: Needed for the custom background loading screens
#if defined (NEW_LOADING_SCREENS)
#include "GameUI/IGameUI.h"
#include "darkinterval/menu/iloadingbackground.h"
#endif
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define ACHIEVEMENT_ANNOUNCEMENT_MIN_TIME 10

class CHudWeaponSelection;
class CHudChat;
class CHudVote;

static vgui::HContext s_hVGuiContext = DEFAULT_VGUI_CONTEXT;

#if defined (NEW_LOADING_SCREENS)
//Fenix: Needed for the custom background loading screens
// See interface.h/.cpp for specifics:  basically this ensures that we actually Sys_UnloadModule the dll and that we don't call Sys_LoadModule 
//  over and over again.
static CDllDemandLoader g_GameUI("GameUI");
#endif

ConVar cl_drawhud( "cl_drawhud", "1", FCVAR_NOTIFY, "Enable the rendering of the hud" );
#ifndef DARKINTERVAL // reducing amount of convars
ConVar hud_takesshots( "hud_takesshots", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Auto-save a scoreboard screenshot at the end of a map." );
ConVar hud_freezecamhide( "hud_freezecamhide", "0", FCVAR_CLIENTDLL | FCVAR_ARCHIVE, "Hide the HUD during freeze-cam" );
#endif
ConVar cl_show_num_particle_systems( "cl_show_num_particle_systems", "0", FCVAR_CLIENTDLL, "Display the number of active particle systems." );

extern ConVar v_viewmodel_fov;
extern ConVar voice_modenable;

extern bool IsInCommentaryMode( void );

#if defined (NEW_LOADING_SCREENS)
//Fenix: Needed for the custom background loading screens
CMapLoadBG *pPanelBg;
#endif

#ifdef VOICE_VOX_ENABLE
void VoxCallback( IConVar *var, const char *oldString, float oldFloat )
{
	if ( engine && engine->IsConnected() )
	{
		ConVarRef voice_vox( var->GetName() );
		if ( voice_vox.GetBool() && voice_modenable.GetBool() )
		{
			engine->ClientCmd_Unrestricted( "voicerecord_toggle on\n" );
		}
		else
		{
			engine->ClientCmd_Unrestricted( "voicerecord_toggle off\n" );
		}
	}
}
ConVar voice_vox( "voice_vox", "0", FCVAR_ARCHIVE, "Voice chat uses a vox-style always on", true, 0, true, 1, VoxCallback );

// --------------------------------------------------------------------------------- //
// CVoxManager.
// --------------------------------------------------------------------------------- //
class CVoxManager : public CAutoGameSystem
{
public:
	CVoxManager() : CAutoGameSystem( "VoxManager" )
	{
	}

	virtual void LevelInitPostEntity( void )
	{
		if ( voice_vox.GetBool() && voice_modenable.GetBool() )
		{
			engine->ClientCmd_Unrestricted( "voicerecord_toggle on\n" );
		}
	}

	virtual void LevelShutdownPreEntity( void )
	{
		if ( voice_vox.GetBool() )
		{
			engine->ClientCmd_Unrestricted( "voicerecord_toggle off\n" );
		}
	}
};

static CVoxManager s_VoxManager;
// --------------------------------------------------------------------------------- //
#endif // VOICE_VOX_ENABLE

CON_COMMAND( hud_reloadscheme, "Reloads hud layout and animation scripts." )
{
	ClientModeShared *mode = ( ClientModeShared * )GetClientModeNormal();
	if ( !mode )
		return;

	mode->ReloadScheme();
}

static void __MsgFunc_Rumble( bf_read &msg )
{
	unsigned char waveformIndex;
	unsigned char rumbleData;
	unsigned char rumbleFlags;

	waveformIndex = msg.ReadByte();
	rumbleData = msg.ReadByte();
	rumbleFlags = msg.ReadByte();

	RumbleEffect( waveformIndex, rumbleData, rumbleFlags );
}

static void __MsgFunc_VGUIMenu( bf_read &msg )
{
	char panelname[2048]; 
	
	msg.ReadString( panelname, sizeof(panelname) );

	bool  bShow = msg.ReadByte()!=0;
	
	IViewPortPanel *viewport = gViewPortInterface->FindPanelByName( panelname );

	if ( !viewport )
	{
		// DevMsg("VGUIMenu: couldn't find panel '%s'.\n", panelname );
		return;
	}

	int count = msg.ReadByte();

	if ( count > 0 )
	{
		KeyValues *keys = new KeyValues("data");
		//Msg( "MsgFunc_VGUIMenu:\n" );

		for ( int i=0; i<count; i++)
		{
			char name[255];
			char data[255];

			msg.ReadString( name, sizeof(name) );
			msg.ReadString( data, sizeof(data) );
			//Msg( "  %s <- '%s'\n", name, data );

			keys->SetString( name, data );
		}

		// !KLUDGE! Whitelist of URL protocols formats for MOTD
		if (
			!V_stricmp( panelname, PANEL_INFO ) // MOTD
			&& keys->GetInt( "type", 0 ) == 2 // URL message type
		) {
			const char *pszURL = keys->GetString( "msg", "" );
			if ( Q_strncmp( pszURL, "http://", 7 ) != 0 && Q_strncmp( pszURL, "https://", 8 ) != 0 && Q_stricmp( pszURL, "about:blank" ) != 0 )
			{
				Warning( "Blocking MOTD URL '%s'; must begin with 'http://' or 'https://' or be about:blank\n", pszURL );
				keys->deleteThis();
				return;
			}
		}

		viewport->SetData( keys );

		keys->deleteThis();
	}
#ifndef DARKINTERVAL
	// is the server telling us to show the scoreboard (at the end of a map)?
	if ( Q_stricmp( panelname, "scores" ) == 0 )
	{
		if ( hud_takesshots.GetBool() == true )
		{
			gHUD.SetScreenShotTime( CURTIME + 1.0 ); // take a screenshot in 1 second
		}
	}
#endif
	// is the server trying to show an MOTD panel? Check that it's allowed right now.
	ClientModeShared *mode = ( ClientModeShared * )GetClientModeNormal();
	if ( Q_stricmp( panelname, PANEL_INFO ) == 0 && mode )
	{
		if ( !mode->IsInfoPanelAllowed() )
		{
			return;
		}
		else
		{
			mode->InfoPanelDisplayed();
		}
	}

	gViewPortInterface->ShowPanel( viewport, bShow );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ClientModeShared::ClientModeShared()
{
	m_pViewport = NULL;
	m_pChatElement = NULL;
	m_pWeaponSelection = NULL;
	m_nRootSize[ 0 ] = m_nRootSize[ 1 ] = -1;

#if defined (NEW_LOADING_SCREENS)
	//Fenix: Needed for the custom background loading screens
	pPanelBg = NULL;
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
ClientModeShared::~ClientModeShared()
{
	delete m_pViewport; 
}

void ClientModeShared::ReloadScheme( void )
{
	m_pViewport->ReloadScheme( "resource/ClientScheme.res" );
	ClearKeyValuesCache();
}

//----------------------------------------------------------------------------
// Purpose: Let the client mode set some vgui conditions
//-----------------------------------------------------------------------------
void	ClientModeShared::ComputeVguiResConditions( KeyValues *pkvConditions ) 
{
#ifndef DARKINTERVAL // remove vr stuff
	if (UseVR())
	{
		pkvConditions->FindKey("if_vr", true);
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::Init()
{
	m_pChatElement = ( CBaseHudChat * )GET_HUDELEMENT( CHudChat );
	Assert( m_pChatElement );

	m_pWeaponSelection = ( CBaseHudWeaponSelection * )GET_HUDELEMENT( CHudWeaponSelection );
	Assert( m_pWeaponSelection );

	KeyValuesAD pConditions( "conditions" );
	ComputeVguiResConditions( pConditions );

	// Derived ClientMode class must make sure m_Viewport is instantiated
	Assert( m_pViewport );
	m_pViewport->LoadControlSettings( "scripts/HudLayout.res", NULL, NULL, pConditions );

	ListenForGameEvent( "player_connect" );
	ListenForGameEvent( "player_disconnect" );
	ListenForGameEvent( "player_team" );
	ListenForGameEvent( "server_cvar" );
	ListenForGameEvent( "player_changename" );
	ListenForGameEvent( "teamplay_broadcast_audio" );
	ListenForGameEvent( "achievement_earned" );
	
	m_CursorNone = vgui::dc_none;

	HOOK_MESSAGE( VGUIMenu );
	HOOK_MESSAGE( Rumble );

#if defined (NEW_LOADING_SCREENS)
	//Fenix: Custom background loading screens - Injects the custom panel at the loading screen
	CreateInterfaceFn gameUIFactory = g_GameUI.GetFactory();
	if (gameUIFactory)
	{
		IGameUI *pGameUI = (IGameUI *)gameUIFactory(GAMEUI_INTERFACE_VERSION, NULL);
		if (pGameUI)
		{
			// insert custom loading panel for the loading dialog
			pPanelBg = new CMapLoadBG("Background");
			if (pPanelBg)
			{
				pPanelBg->InvalidateLayout(false, true);
				pPanelBg->SetVisible(false);
				pPanelBg->MakePopup(false);
				pGameUI->SetLoadingBackgroundDialog(pPanelBg->GetVPanel());
			}
		}
	}
#endif
}

void ClientModeShared::InitViewport()
{
}

void ClientModeShared::VGui_Shutdown()
{
	delete m_pViewport;
	m_pViewport = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::Shutdown()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : frametime - 
//			*cmd - 
//-----------------------------------------------------------------------------
bool ClientModeShared::CreateMove( float flInputSampleTime, CUserCmd *cmd )
{
	// Let the player override the view.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if(!pPlayer)
		return true;

	// Let the player at it
	return pPlayer->CreateMove( flInputSampleTime, cmd );
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pSetup - 
//-----------------------------------------------------------------------------
void ClientModeShared::OverrideView( CViewSetup *pSetup )
{
	QAngle camAngles;

	// Let the player override the view.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if(!pPlayer)
		return;

	pPlayer->OverrideView( pSetup );

	if( ::input->CAM_IsThirdPerson() )
	{
		Vector cam_ofs = g_ThirdPersonManager.GetCameraOffsetAngles();
		Vector cam_ofs_distance = g_ThirdPersonManager.GetFinalCameraOffset();

		cam_ofs_distance *= g_ThirdPersonManager.GetDistanceFraction();

		camAngles[ PITCH ] = cam_ofs[ PITCH ];
		camAngles[ YAW ] = cam_ofs[ YAW ];
		camAngles[ ROLL ] = 0;

		Vector camForward, camRight, camUp;
		

		if ( g_ThirdPersonManager.IsOverridingThirdPerson() == false )
		{
			engine->GetViewAngles( camAngles );
		}
			
		// get the forward vector
		AngleVectors( camAngles, &camForward, &camRight, &camUp );
	
		VectorMA( pSetup->origin, -cam_ofs_distance[0], camForward, pSetup->origin );
		VectorMA( pSetup->origin, cam_ofs_distance[1], camRight, pSetup->origin );
		VectorMA( pSetup->origin, cam_ofs_distance[2], camUp, pSetup->origin );

		// Override angles from third person camera
		VectorCopy( camAngles, pSetup->angles );
	}
	else if (::input->CAM_IsOrthographic())
	{
		pSetup->m_bOrtho = true;
		float w, h;
		::input->CAM_OrthographicSize( w, h );
		w *= 0.5f;
		h *= 0.5f;
		pSetup->m_OrthoLeft   = -w;
		pSetup->m_OrthoTop    = -h;
		pSetup->m_OrthoRight  = w;
		pSetup->m_OrthoBottom = h;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool ClientModeShared::ShouldDrawEntity(C_BaseEntity *pEnt)
{
	return true;
}

bool ClientModeShared::ShouldDrawParticles( )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Allow weapons to override mouse input (for binoculars)
//-----------------------------------------------------------------------------
void ClientModeShared::OverrideMouseInput( float *x, float *y )
{
	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if ( pWeapon )
	{
		pWeapon->OverrideMouseInput( x, y );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool ClientModeShared::ShouldDrawViewModel()
{
	return true;
}

bool ClientModeShared::ShouldDrawDetailObjects( )
{
	return true;
}
#ifndef DARKINTERVAL // remove vr stuff
//-----------------------------------------------------------------------------
// Purpose: Returns true if VR mode should black out everything outside the HUD.
//			This is used for things like sniper scopes and full screen UI
//-----------------------------------------------------------------------------
bool ClientModeShared::ShouldBlackoutAroundHUD()
{
	return enginevgui->IsGameUIVisible();
}

//-----------------------------------------------------------------------------
// Purpose: Allows the client mode to override mouse control stuff in sourcevr
//-----------------------------------------------------------------------------
HeadtrackMovementMode_t ClientModeShared::ShouldOverrideHeadtrackControl()
{
	return HMM_NOOVERRIDE;
}
#endif
//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool ClientModeShared::ShouldDrawCrosshair( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Don't draw the current view entity if we are using the fake viewmodel instead
//-----------------------------------------------------------------------------
bool ClientModeShared::ShouldDrawLocalPlayer( C_BasePlayer *pPlayer )
{
	if ( ( pPlayer->index == render->GetViewEntity() ) && !C_BasePlayer::ShouldDrawLocalPlayer() )
		return false;

	return true;
}


//-----------------------------------------------------------------------------
// Purpose: The mode can choose to not draw fog
//-----------------------------------------------------------------------------
bool ClientModeShared::ShouldDrawFog( void )
{
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::AdjustEngineViewport( int& x, int& y, int& width, int& height )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::PreRender( CViewSetup *pSetup )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::PostRender()
{
	// Let the particle manager simulate things that haven't been simulated.
	ParticleMgr()->PostRender();
}

void ClientModeShared::PostRenderVGui()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::Update()
{
	if ( m_pViewport->IsVisible() != cl_drawhud.GetBool() )
	{
		m_pViewport->SetVisible( cl_drawhud.GetBool() );
	}

	UpdateRumbleEffects();

	if ( cl_show_num_particle_systems.GetBool() )
	{
		int nCount = 0;

		for ( int i = 0; i < g_pParticleSystemMgr->GetParticleSystemCount(); i++ )
		{
			const char *pParticleSystemName = g_pParticleSystemMgr->GetParticleSystemNameFromIndex(i);
			CParticleSystemDefinition *pParticleSystem = g_pParticleSystemMgr->FindParticleSystem( pParticleSystemName );
			if ( !pParticleSystem )
				continue;

			for ( CParticleCollection *pCurCollection = pParticleSystem->FirstCollection();
				  pCurCollection != NULL;
				  pCurCollection = pCurCollection->GetNextCollectionUsingSameDef() )
			{
				++nCount;
			}
		}

		engine->Con_NPrintf( 0, "# Active particle systems: %i", nCount );
	}
}

//-----------------------------------------------------------------------------
// This processes all input before SV Move messages are sent
//-----------------------------------------------------------------------------

void ClientModeShared::ProcessInput(bool bActive)
{
	gHUD.ProcessInput( bActive );
}

//-----------------------------------------------------------------------------
// Purpose: We've received a keypress from the engine. Return 1 if the engine is allowed to handle it.
//-----------------------------------------------------------------------------
int	ClientModeShared::KeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
{
	if ( engine->Con_IsVisible() )
		return 1;
	
	// Should we start typing a message?
	if ( pszCurrentBinding &&
		( Q_strcmp( pszCurrentBinding, "messagemode" ) == 0 ||
		  Q_strcmp( pszCurrentBinding, "say" ) == 0 ) )
	{
		if ( down )
		{
			StartMessageMode( MM_SAY );
		}
		return 0;
	}
	else if ( pszCurrentBinding &&
				( Q_strcmp( pszCurrentBinding, "messagemode2" ) == 0 ||
				  Q_strcmp( pszCurrentBinding, "say_team" ) == 0 ) )
	{
		if ( down )
		{
			StartMessageMode( MM_SAY_TEAM );
		}
		return 0;
	}
	
	// If we're voting...
#ifdef VOTING_ENABLED
	CHudVote *pHudVote = GET_HUDELEMENT( CHudVote );
	if ( pHudVote && pHudVote->IsVisible() )
	{
		if ( !pHudVote->KeyInput( down, keynum, pszCurrentBinding ) )
		{
			return 0;
		}
	}
#endif

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	// if ingame spectator mode, let spectator input intercept key event here
	if( pPlayer &&
		( pPlayer->GetObserverMode() > OBS_MODE_DEATHCAM ) &&
		!HandleSpectatorKeyInput( down, keynum, pszCurrentBinding ) )
	{
		return 0;
	}

	// Let game-specific hud elements get a crack at the key input
	if ( !HudElementKeyInput( down, keynum, pszCurrentBinding ) )
	{
		return 0;
	}

	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if ( pWeapon )
	{
		return pWeapon->KeyInput( down, keynum, pszCurrentBinding );
	}

	return 1;
}

//-----------------------------------------------------------------------------
// Purpose: See if spectator input occurred. Return 0 if the key is swallowed.
//-----------------------------------------------------------------------------
int ClientModeShared::HandleSpectatorKeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
{
	return 0;
}

//-----------------------------------------------------------------------------
// Purpose: See if hud elements want key input. Return 0 if the key is swallowed
//-----------------------------------------------------------------------------
int ClientModeShared::HudElementKeyInput( int down, ButtonCode_t keynum, const char *pszCurrentBinding )
{
	if ( m_pWeaponSelection )
	{
		if ( !m_pWeaponSelection->KeyInput( down, keynum, pszCurrentBinding ) )
		{
			return 0;
		}
	}
	
	return 1;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool ClientModeShared::DoPostScreenSpaceEffects( const CViewSetup *pSetup )
{
#ifdef DARKINTERVAL // todo: why isn't this routed like the rest of postprocess, should put it there
	g_GlowObjectManager.RenderGlowEffects(pSetup, 0);
#endif
	return true;
}
//-----------------------------------------------------------------------------
// Purpose: 
// Output : vgui::Panel
//-----------------------------------------------------------------------------
vgui::Panel *ClientModeShared::GetMessagePanel()
{
	if ( m_pChatElement && m_pChatElement->GetInputPanel() && m_pChatElement->GetInputPanel()->IsVisible() )
		return m_pChatElement->GetInputPanel();

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: The player has started to type a message
//-----------------------------------------------------------------------------
void ClientModeShared::StartMessageMode( int iMessageModeType )
{
	// Can only show chat UI in multiplayer!!!
	if ( gpGlobals->maxClients == 1 )
	{
		return;
	}
	if ( m_pChatElement )
	{
		m_pChatElement->StartMessageMode( iMessageModeType );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *newmap - 
//-----------------------------------------------------------------------------
void ClientModeShared::LevelInit( const char *newmap )
{
	m_pViewport->GetAnimationController()->StartAnimationSequence("LevelInit");

	// Tell the Chat Interface
	if ( m_pChatElement )
	{
		m_pChatElement->LevelInit( newmap );
	}

	// we have to fake this event clientside, because clients connect after that
	IGameEvent *event = gameeventmanager->CreateEvent( "game_newmap" );
	if ( event )
	{
		event->SetString("mapname", newmap );
		gameeventmanager->FireEventClientSide( event );
	}

	// Create a vgui context for all of the in-game vgui panels...
	if ( s_hVGuiContext == DEFAULT_VGUI_CONTEXT )
	{
		s_hVGuiContext = vgui::ivgui()->CreateContext();
	}

	// Reset any player explosion/shock effects
	CLocalPlayerFilter filter;
	enginesound->SetPlayerDSP( filter, 0, true );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void ClientModeShared::LevelShutdown( void )
{
	// Reset the third person camera so we don't crash
	g_ThirdPersonManager.Init();
#ifdef DARKINTERVAL // not sure why this was added here...
	engine->GetGamestatsData();
#endif
	if ( m_pChatElement )
	{
		m_pChatElement->LevelShutdown();
	}
	if ( s_hVGuiContext != DEFAULT_VGUI_CONTEXT )
	{
		vgui::ivgui()->DestroyContext( s_hVGuiContext );
 		s_hVGuiContext = DEFAULT_VGUI_CONTEXT;
	}

	// Reset any player explosion/shock effects
	CLocalPlayerFilter filter;
	enginesound->SetPlayerDSP( filter, 0, true );
}


void ClientModeShared::Enable()
{
	vgui::VPANEL pRoot = VGui_GetClientDLLRootPanel();;

	// Add our viewport to the root panel.
	if( pRoot != 0 )
	{
		m_pViewport->SetParent( pRoot );
	}

	// All hud elements should be proportional
	// This sets that flag on the viewport and all child panels
	m_pViewport->SetProportional( true );

	m_pViewport->SetCursor( m_CursorNone );
	vgui::surface()->SetCursor( m_CursorNone );

	m_pViewport->SetVisible( true );
	if ( m_pViewport->IsKeyBoardInputEnabled() )
	{
		m_pViewport->RequestFocus();
	}

	Layout();
}


void ClientModeShared::Disable()
{
	vgui::VPANEL pRoot = VGui_GetClientDLLRootPanel();;

	// Remove our viewport from the root panel.
	if( pRoot != 0 )
	{
		m_pViewport->SetParent( (vgui::VPANEL)NULL );
	}

	m_pViewport->SetVisible( false );
}


void ClientModeShared::Layout()
{
	vgui::VPANEL pRoot = VGui_GetClientDLLRootPanel();
	int wide, tall;

	// Make the viewport fill the root panel.
	if( pRoot != 0 )
	{
		vgui::ipanel()->GetSize(pRoot, wide, tall);

		bool changed = wide != m_nRootSize[ 0 ] || tall != m_nRootSize[ 1 ];
		m_nRootSize[ 0 ] = wide;
		m_nRootSize[ 1 ] = tall;

		m_pViewport->SetBounds(0, 0, wide, tall);
		if ( changed )
		{
			ReloadScheme();
		}
	}
}

float ClientModeShared::GetViewModelFOV( void )
{
	return v_viewmodel_fov.GetFloat();
}

class CHudChat;

bool PlayerNameNotSetYet( const char *pszName )
{
	if ( pszName && pszName[0] )
	{
		// Don't show "unconnected" if we haven't got the players name yet
		if ( Q_strnicmp(pszName,"unconnected",11) == 0 )
			return true;
		if ( Q_strnicmp(pszName,"NULLNAME",11) == 0 )
			return true;
	}

	return false;
}

void ClientModeShared::FireGameEvent( IGameEvent *event )
{
	CBaseHudChat *hudChat = (CBaseHudChat *)GET_HUDELEMENT( CHudChat );

	const char *eventname = event->GetName();

	if ( Q_strcmp( "player_connect", eventname ) == 0 )
	{
		if ( !hudChat )
			return;
		if ( PlayerNameNotSetYet(event->GetString("name")) )
			return;

		if ( !IsInCommentaryMode() )
		{
			wchar_t wszLocalized[100];
			wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
			g_pVGuiLocalize->ConvertANSIToUnicode( event->GetString("name"), wszPlayerName, sizeof(wszPlayerName) );
			g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_player_joined_game" ), 1, wszPlayerName );

			char szLocalized[100];
			g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );

			hudChat->Printf( CHAT_FILTER_JOINLEAVE, "%s", szLocalized );
		}
	}
	else if ( Q_strcmp( "player_disconnect", eventname ) == 0 )
	{
		C_BasePlayer *pPlayer = USERID2PLAYER( event->GetInt("userid") );

		if ( !hudChat || !pPlayer )
			return;
		if ( PlayerNameNotSetYet(event->GetString("name")) )
			return;

		if ( !IsInCommentaryMode() )
		{
			wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
			g_pVGuiLocalize->ConvertANSIToUnicode( pPlayer->GetPlayerName(), wszPlayerName, sizeof(wszPlayerName) );

			wchar_t wszReason[64];
			const char *pszReason = event->GetString( "reason" );
			if ( pszReason && ( pszReason[0] == '#' ) && g_pVGuiLocalize->Find( pszReason ) )
			{
				V_wcsncpy( wszReason, g_pVGuiLocalize->Find( pszReason ), sizeof( wszReason ) );
			}
			else
			{
				g_pVGuiLocalize->ConvertANSIToUnicode( pszReason, wszReason, sizeof(wszReason) );
			}

			wchar_t wszLocalized[100];
			if (IsPC())
			{
				g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_player_left_game" ), 2, wszPlayerName, wszReason );
			}
			else
			{
				g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_player_left_game" ), 1, wszPlayerName );
			}

			char szLocalized[100];
			g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );

			hudChat->Printf( CHAT_FILTER_JOINLEAVE, "%s", szLocalized );
		}
	}
#ifndef DARKINTERVAL // DI is singleplayer, so it ditched teams
	else if (Q_strcmp("player_team", eventname) == 0)
	{
		C_BasePlayer *pPlayer = USERID2PLAYER(event->GetInt("userid"));
		if (!hudChat)
			return;

		bool bDisconnected = event->GetBool("disconnect");

		if (bDisconnected)
			return;

		int team = event->GetInt("team");
		bool bAutoTeamed = event->GetInt("autoteam", false);
		bool bSilent = event->GetInt("silent", false);

		const char *pszName = event->GetString("name");
		if (PlayerNameNotSetYet(pszName))
			return;

		if (!bSilent)
		{
			wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
			g_pVGuiLocalize->ConvertANSIToUnicode(pszName, wszPlayerName, sizeof(wszPlayerName));

			wchar_t wszTeam[64];
			C_Team *pTeam = GetGlobalTeam(team);
			if (pTeam)
			{
				g_pVGuiLocalize->ConvertANSIToUnicode(pTeam->Get_Name(), wszTeam, sizeof(wszTeam));
			}
			else
			{
				_snwprintf(wszTeam, sizeof(wszTeam) / sizeof(wchar_t), L"%d", team);
			}

			if (!IsInCommentaryMode())
			{
				wchar_t wszLocalized[100];
				if (bAutoTeamed)
				{
					g_pVGuiLocalize->ConstructString(wszLocalized, sizeof(wszLocalized), g_pVGuiLocalize->Find("#game_player_joined_autoteam"), 2, wszPlayerName, wszTeam);
				}
				else
				{
					g_pVGuiLocalize->ConstructString(wszLocalized, sizeof(wszLocalized), g_pVGuiLocalize->Find("#game_player_joined_team"), 2, wszPlayerName, wszTeam);
				}

				char szLocalized[100];
				g_pVGuiLocalize->ConvertUnicodeToANSI(wszLocalized, szLocalized, sizeof(szLocalized));

				hudChat->Printf(CHAT_FILTER_TEAMCHANGE, "%s", szLocalized);
			}
		}

		if (pPlayer && pPlayer->IsLocalPlayer())
		{
			// that's me
			pPlayer->TeamChange(team);
		}
	}
#endif // DARKINTERVAL
	else if ( Q_strcmp( "player_changename", eventname ) == 0 )
	{
		if ( !hudChat )
			return;

		const char *pszOldName = event->GetString("oldname");
		if ( PlayerNameNotSetYet(pszOldName) )
			return;

		wchar_t wszOldName[MAX_PLAYER_NAME_LENGTH];
		g_pVGuiLocalize->ConvertANSIToUnicode( pszOldName, wszOldName, sizeof(wszOldName) );

		wchar_t wszNewName[MAX_PLAYER_NAME_LENGTH];
		g_pVGuiLocalize->ConvertANSIToUnicode( event->GetString( "newname" ), wszNewName, sizeof(wszNewName) );

		wchar_t wszLocalized[100];
		g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_player_changed_name" ), 2, wszOldName, wszNewName );

		char szLocalized[100];
		g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );

		hudChat->Printf( CHAT_FILTER_NAMECHANGE, "%s", szLocalized );
	}
#ifndef DARKINTERVAL // DI is singleplayer, so it ditched teams
	else if (Q_strcmp("teamplay_broadcast_audio", eventname) == 0)
	{
		int team = event->GetInt("team");

		bool bValidTeam = false;

		if ((GetLocalTeam() && GetLocalTeam()->GetTeamNumber() == team))
		{
			bValidTeam = true;
		}

		//If we're in the spectator team then we should be getting whatever messages the person I'm spectating gets.
		if (bValidTeam == false)
		{
			CBasePlayer *pSpectatorTarget = UTIL_PlayerByIndex(GetSpectatorTarget());

			if (pSpectatorTarget && (GetSpectatorMode() == OBS_MODE_IN_EYE || GetSpectatorMode() == OBS_MODE_CHASE))
			{
				if (pSpectatorTarget->GetTeamNumber() == team)
				{
					bValidTeam = true;
				}
			}
		}

		if (team == 0 && GetLocalTeam() > 0)
		{
			bValidTeam = false;
		}

		if (team == 255)
		{
			bValidTeam = true;
		}

		if (bValidTeam == true)
		{
			EmitSound_t et;
			et.m_pSoundName = event->GetString("sound");
			et.m_nFlags = event->GetInt("additional_flags");

			CLocalPlayerFilter filter;
			C_BaseEntity::EmitSound(filter, SOUND_FROM_LOCAL_PLAYER, et);
		}
	}
#endif // DARKINTERVAL
	else if ( Q_strcmp( "server_cvar", eventname ) == 0 )
	{
		if ( !IsInCommentaryMode() )
		{
			wchar_t wszCvarName[64];
			g_pVGuiLocalize->ConvertANSIToUnicode( event->GetString("cvarname"), wszCvarName, sizeof(wszCvarName) );

			wchar_t wszCvarValue[64];
			g_pVGuiLocalize->ConvertANSIToUnicode( event->GetString("cvarvalue"), wszCvarValue, sizeof(wszCvarValue) );

			wchar_t wszLocalized[256];
			g_pVGuiLocalize->ConstructString( wszLocalized, sizeof( wszLocalized ), g_pVGuiLocalize->Find( "#game_server_cvar_changed" ), 2, wszCvarName, wszCvarValue );

			char szLocalized[256];
			g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalized, szLocalized, sizeof(szLocalized) );

			hudChat->Printf( CHAT_FILTER_SERVERMSG, "%s", szLocalized );
		}
	}
	else if ( Q_strcmp( "achievement_earned", eventname ) == 0 )
	{
		int iPlayerIndex = event->GetInt( "player" );
		C_BasePlayer *pPlayer = UTIL_PlayerByIndex( iPlayerIndex );
		int iAchievement = event->GetInt( "achievement" );

		if ( !hudChat || !pPlayer )
			return;

		if ( !IsInCommentaryMode() )
		{
			CAchievementMgr *pAchievementMgr = dynamic_cast<CAchievementMgr *>( engine->GetAchievementMgr() );
			if ( !pAchievementMgr )
				return;

			IAchievement *pAchievement = pAchievementMgr->GetAchievementByID( iAchievement );
			if ( pAchievement )
			{
				if ( !pPlayer->IsDormant() && pPlayer->ShouldAnnounceAchievement() )
				{
					pPlayer->SetNextAchievementAnnounceTime( CURTIME + ACHIEVEMENT_ANNOUNCEMENT_MIN_TIME );
					
					pPlayer->OnAchievementAchieved( iAchievement );
				}

				if ( g_PR )
				{
					wchar_t wszPlayerName[MAX_PLAYER_NAME_LENGTH];
					g_pVGuiLocalize->ConvertANSIToUnicode( g_PR->GetPlayerName( iPlayerIndex ), wszPlayerName, sizeof( wszPlayerName ) );

					const wchar_t *pchLocalizedAchievement = ACHIEVEMENT_LOCALIZED_NAME_FROM_STR( pAchievement->GetName() );
					if ( pchLocalizedAchievement )
					{
						wchar_t wszLocalizedString[128];
						g_pVGuiLocalize->ConstructString( wszLocalizedString, sizeof( wszLocalizedString ), g_pVGuiLocalize->Find( "#Achievement_Earned" ), 2, wszPlayerName, pchLocalizedAchievement );

						char szLocalized[128];
						g_pVGuiLocalize->ConvertUnicodeToANSI( wszLocalizedString, szLocalized, sizeof( szLocalized ) );

						hudChat->ChatPrintf( iPlayerIndex, CHAT_FILTER_SERVERMSG, "%s", szLocalized );
					}
				}
			}
		}
	}
	
	else
	{
		DevMsg( 2, "Unhandled GameEvent in ClientModeShared::FireGameEvent - %s\n", event->GetName()  );
	}
}

//-----------------------------------------------------------------------------
// In-game VGUI context 
//-----------------------------------------------------------------------------
void ClientModeShared::ActivateInGameVGuiContext( vgui::Panel *pPanel )
{
	vgui::ivgui()->AssociatePanelWithContext( s_hVGuiContext, pPanel->GetVPanel() );
	vgui::ivgui()->ActivateContext( s_hVGuiContext );
}

void ClientModeShared::DeactivateInGameVGuiContext()
{
	vgui::ivgui()->ActivateContext( DEFAULT_VGUI_CONTEXT );
}

