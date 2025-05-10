//========================================================================//
//
// Purpose: 
// DI changes that are not ifdef'd: 
// - removed all CS/TF/MP stuff
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "vgui_int.h"
#include "ienginevgui.h"
#include "itextmessage.h"
#include "vguicenterprint.h"
#include "iloadingdisc.h"
#include "ifpspanel.h"
#include "imessagechars.h"
#include "inetgraphpanel.h"
#include "idebugoverlaypanel.h"
#include <vgui/ISurface.h>
#include <vgui/Ivgui.h>
#include <vgui/IInput.h>
#include "tier0/vprof.h"
#include "iclientmode.h"
#include <vgui_controls/Panel.h>
#include <keyvalues.h>
#include "filesystem.h"
#include "matsys_controls/matsyscontrols.h"
#ifdef DARKINTERVAL // custom DI panels
#include "darkinterval\menu\ipopupnotice.h"
#include "darkinterval\menu\ioptionsmaindialogue.h"
#include "darkinterval\menu\ioptionspanel_video.h"
#include "darkinterval\menu\ioptionspanel_sound.h"
#include "darkinterval\menu\inewmenuroot.h"
#include "darkinterval\menu\iweathersettings.h"
#endif
#ifdef SIXENSE
#include "sixense/in_sixense.h"
#endif

using namespace vgui;

void MP3Player_Create( vgui::VPANEL parent );
void MP3Player_Destroy();
#ifdef DARKINTERVAL
void WSUI_Create(vgui::VPANEL parent); // the fog/hdr/dof ui panel
void WSUI_Destroy();

void OverrideGameUI(); // the main menu override
#endif
#include <vgui/IInputInternal.h>
vgui::IInputInternal *g_InputInternal = NULL;

#include <vgui_controls/Controls.h>

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

void GetVGUICursorPos( int& x, int& y )
{
	vgui::input()->GetCursorPos(x, y);
}

void SetVGUICursorPos( int x, int y )
{
	if ( !g_bTextMode )
	{
		vgui::input()->SetCursorPos(x, y);
	}
}

class CHudTextureHandleProperty : public vgui::IPanelAnimationPropertyConverter
{
public:
	virtual void GetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		CHudTextureHandle *pHandle = ( CHudTextureHandle * )data;

		// lookup texture name for id
		if ( pHandle->Get() )
		{
			kv->SetString( entry->name(), pHandle->Get()->szShortName );
		}
		else
		{
			kv->SetString( entry->name(), "" );
		}
	}
	
	virtual void SetData( Panel *panel, KeyValues *kv, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );
		
		CHudTextureHandle *pHandle = ( CHudTextureHandle * )data;

		const char *texturename = kv->GetString( entry->name() );
		if ( texturename && texturename[ 0 ] )
		{
			CHudTexture *currentTexture = gHUD.GetIcon( texturename );
			pHandle->Set( currentTexture );
		}
		else
		{
			pHandle->Set( NULL );
		}
	}

	virtual void InitFromDefault( Panel *panel, PanelAnimationMapEntry *entry )
	{
		void *data = ( void * )( (*entry->m_pfnLookup)( panel ) );

		CHudTextureHandle *pHandle = ( CHudTextureHandle * )data;

		const char *texturename = entry->defaultvalue();
		if ( texturename && texturename[ 0 ] )
		{
			CHudTexture *currentTexture = gHUD.GetIcon( texturename );
			pHandle->Set( currentTexture );
		}
		else
		{
			pHandle->Set( NULL );
		}
	}
};

static CHudTextureHandleProperty textureHandleConverter;

static void VGui_VideoMode_AdjustForModeChange( void )
{
	// Kill all our panels. We need to do this in case any of them
	//	have pointers to objects (eg: iborders) that will get freed
	//	when schemes get destroyed and recreated (eg: mode change).
	netgraphpanel->Destroy();
	debugoverlaypanel->Destroy();
#if defined( TRACK_BLOCKING_IO )
	iopanel->Destroy();
#endif
	fps->Destroy();
	messagechars->Destroy();
	loadingdisc->Destroy();

	// Recreate our panels.
	VPANEL gameToolParent = enginevgui->GetPanel( PANEL_CLIENTDLL_TOOLS );
	VPANEL toolParent = enginevgui->GetPanel( PANEL_TOOLS );
#if defined( TRACK_BLOCKING_IO )
	VPANEL gameDLLPanel = enginevgui->GetPanel( PANEL_GAMEDLL );
#endif

	loadingdisc->Create( gameToolParent );
	messagechars->Create( gameToolParent );

	// Debugging or related tool
	fps->Create( toolParent );
#if defined( TRACK_BLOCKING_IO )
	iopanel->Create( gameDLLPanel );
#endif
	netgraphpanel->Create( toolParent );
	debugoverlaypanel->Create( gameToolParent );
}

static void VGui_OneTimeInit()
{
	static bool initialized = false;
	if ( initialized )
		return;
	initialized = true;

	vgui::Panel::AddPropertyConverter( "CHudTextureHandle", &textureHandleConverter );


    g_pMaterialSystem->AddModeChangeCallBack( &VGui_VideoMode_AdjustForModeChange );
}

bool VGui_Startup( CreateInterfaceFn appSystemFactory )
{
	if ( !vgui::VGui_InitInterfacesList( "CLIENT", &appSystemFactory, 1 ) )
		return false;

	if ( !vgui::VGui_InitMatSysInterfacesList( "CLIENT", &appSystemFactory, 1 ) )
		return false;

	g_InputInternal = (IInputInternal *)appSystemFactory( VGUI_INPUTINTERNAL_INTERFACE_VERSION,  NULL );
	if ( !g_InputInternal )
	{
		return false; // c_vguiscreen.cpp needs this!
	}

	VGui_OneTimeInit();

	// Create any root panels for .dll
	VGUI_CreateClientDLLRootPanel();

	// Make sure we have a panel
	VPANEL root = VGui_GetClientDLLRootPanel();
	if ( !root )
	{
		return false;
	}
	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void VGui_CreateGlobalPanels( void )
{
	VPANEL gameToolParent = enginevgui->GetPanel( PANEL_CLIENTDLL_TOOLS );
	VPANEL toolParent = enginevgui->GetPanel( PANEL_TOOLS );
#ifdef DARKINTERVAL
	VPANEL GameUiDll = enginevgui->GetPanel(PANEL_GAMEUIDLL);
#endif
#if defined( TRACK_BLOCKING_IO )
	VPANEL gameDLLPanel = enginevgui->GetPanel( PANEL_GAMEDLL );
#endif
	// Part of game
	internalCenterPrint->Create( gameToolParent );
	loadingdisc->Create( gameToolParent );
	messagechars->Create( gameToolParent );
	
	// Debugging or related tool
	fps->Create( toolParent );
#if defined( TRACK_BLOCKING_IO )
	iopanel->Create( gameDLLPanel );
#endif
	netgraphpanel->Create( toolParent );
	debugoverlaypanel->Create( gameToolParent );
#ifdef SIXENSE
	g_pSixenseInput->CreateGUI( gameToolParent );
#endif	
#ifdef DARKINTERVAL
	//	loadingtip->Create(gameToolParent); // unused loading tip
	popupnoticepanel->Create(GameUiDll); // used for warnings such as when using the Arcade machines
	newmainmenubase->Create(GameUiDll); // the new main menu
	WSUI_Create(toolParent); // the fog/hdr/dof ui panel
#endif
	MP3Player_Create(toolParent);
}

void VGui_Shutdown()
{
	VGUI_DestroyClientDLLRootPanel();
	
	netgraphpanel->Destroy();
	debugoverlaypanel->Destroy();
#if defined( TRACK_BLOCKING_IO )
	iopanel->Destroy();
#endif
	fps->Destroy();

	messagechars->Destroy();
	loadingdisc->Destroy();
	internalCenterPrint->Destroy();

#ifdef DARKINTERVAL
	//	loadingtip->Destroy();
	popupnoticepanel->Destroy();
	newmainmenubase->Destroy();
#endif
	if ( g_pClientMode )
	{
		g_pClientMode->VGui_Shutdown();
	}

#ifdef DARKINTERVAL
	WSUI_Destroy();
#endif
	MP3Player_Destroy();

	// Make sure anything "marked for deletion"
	//  actually gets deleted before this dll goes away
	vgui::ivgui()->RunFrame();
}
#ifdef DARKINTERVAL
static ConVar cl_showloadingimage("cl_showloadingimage", "1", 0, "Show the 'LOADING' image when game is paused.");
#endif
static ConVar cl_showpausedimage( "cl_showpausedimage", "1", 0, "Show the 'Paused' image when game is paused." );

//-----------------------------------------------------------------------------
// Things to do before rendering vgui stuff...
//-----------------------------------------------------------------------------
void VGui_PreRender()
{
	VPROF( "VGui_PreRender" );
	tmZone( TELEMETRY_LEVEL0, TMZF_NONE, "%s", __FUNCTION__ );

	// 360 does not use these plaques
	if ( IsPC() )
	{
		loadingdisc->SetLoadingVisible(
#ifdef DARKINTERVAL
			cl_showloadingimage.GetBool() && 
#endif
			engine->IsDrawingLoadingImage() && !engine->IsPlayingDemo() );

		loadingdisc->SetPausedVisible( !enginevgui->IsGameUIVisible() && cl_showpausedimage.GetBool() && engine->IsPaused() && !engine->IsTakingScreenshot() && !engine->IsPlayingDemo() );
#ifdef DARKINTERVAL
		//	loadingtip->SetLoadingVisible(engine->IsDrawingLoadingImage() && !engine->IsPlayingDemo());
		//	loadingtip->SetPausedVisible(!enginevgui->IsGameUIVisible() && cl_showpausedimage.GetBool() && engine->IsPaused() && !engine->IsTakingScreenshot() && !engine->IsPlayingDemo());
#endif
	}
}

void VGui_PostRender()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : cl_panelanimation - 
//-----------------------------------------------------------------------------
CON_COMMAND( cl_panelanimation, "Shows panel animation variables: <panelname | blank for all panels>." )
{
	if ( args.ArgC() == 2 )
	{
		PanelAnimationDumpVars( args[1] );
	}
	else
	{
		PanelAnimationDumpVars( NULL );
	}
}

void GetHudSize( int& w, int &h )
{
	vgui::surface()->GetScreenSize( w, h );

	VPANEL hudParent = enginevgui->GetPanel( PANEL_CLIENTDLL );
	if ( hudParent )
	{
		vgui::ipanel()->GetSize( hudParent, w, h );
	}
}
#ifdef DARKINTERVAL
/*
CON_COMMAND(
	test_listinfoforpanel,  //The actual console command that we'll type in the console
	"Show infos for panel, including childrens listing, and HPanel#. If no params specified, displays info about root panel. : <HPanel#>"
)
{
	VPANEL panel;

	if (args.ArgC() == 2)
	{
		//parse the HPanel #
		panel = vgui::ivgui()->HandleToPanel(Q_atoi(args.ArgV()[1]));
	}
	else
	{
		//Panel 1 is always the static panel (parent of gameuidll) at least it seams to be...
		panel = vgui::ivgui()->HandleToPanel(1);
	}

	GetInfoAboutPanel(panel);
}

void GetInfoAboutPanel(VPANEL panel)
{
	if (panel != 0)
	{
		int NbChilds = vgui::ipanel()->GetChildCount(panel); //Get our ammount of children VPANELs

		DevMsg("PanelName: %s, Class: %s, HPanel# : %d, NbChildrens: %d, IsVisible: %d\n",
			vgui::ipanel()->GetName(panel),
			vgui::ipanel()->GetClassName(panel),
			vgui::ivgui()->PanelToHandle(panel),
			NbChilds,
			vgui::ipanel()->IsVisible(panel)
		);

		ListChilds(NbChilds, panel);
	}
}

void ListChilds(int nbchilds, VPANEL panel)
{
	for (int i = 0; i < nbchilds; ++i)
	{
		VPANEL tmppanel = vgui::ipanel()->GetChild(panel, i); //Get the child of panel, at index i
		DevMsg("- %d : ", i);
		int NbChilds = vgui::ipanel()->GetChildCount(tmppanel); //Get the ammount of child from panel's child VPANEL

		DevMsg("PanelName: %s, Class: %s, HPanel# : %d, NbChildrens: %d, IsVisible: %d\n",
			vgui::ipanel()->GetName(tmppanel),
			vgui::ipanel()->GetClassName(tmppanel),
			vgui::ivgui()->PanelToHandle(tmppanel),
			NbChilds,
			vgui::ipanel()->IsVisible(tmppanel)
		);
	}
}
*/
#endif // DARKINTERVAL