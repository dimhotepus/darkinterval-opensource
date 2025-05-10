//#include "BasePanel.h"
#include "cbase.h"
#include "ioptionsmaindialogue.h"

#include "vgui_controls/button.h"
#include "vgui_controls/CheckButton.h"
#include "vgui_controls/PropertySheet.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/QueryBox.h"

#include "vgui/ilocalize.h"
#include "vgui/ISurface.h"
#include "vgui/ISystem.h"
#include "vgui/Ivgui.h"

#include "keyvalues.h"

#include "ioptionspanel_game.h"
#include "ioptionspanel_controls.h"
#include "ioptionspanel_display.h"
#include "ioptionspanel_sound.h"
#include "ioptionspanel_video.h"

using namespace vgui;

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

//-----------------------------------------------------------------------------
// Purpose: Basic help dialog
//-----------------------------------------------------------------------------
CNewOptionsDialog::CNewOptionsDialog(vgui::Panel *parent) : PropertyDialog(parent, "NewOptionsDialogue")
{
//	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/darkinterval_menuscheme.res", "Scheme"));

	SetDeleteSelfOnClose(true);
//	int horCenter = (ScreenWidth() / 2) -300;
//	int verCenter = (ScreenHeight() / 2) -300;
//	SetBounds(horCenter - 300, verCenter - 300, 600, 600);
	int horCenter = ScreenWidth() / 2;
	int verCenter = ScreenHeight() / 4;
	if( ScreenHeight() >= 600 )
		SetBounds(horCenter + ScreenWidth() / 5, verCenter, 600, 600 );
	else
	{
		SetBounds(horCenter + ScreenWidth() / 5, verCenter, 600, 480);
	}
	SetSizeable(true);
	SetRoundedCorners(0);

	SetTitle("#GameUI_Options", true);

	m_pNewOptionsGame = new COptionsPanel_Game(this);
	AddPage(m_pNewOptionsGame, "#DI_NewMenu_GameOptions_General");
	m_pNewOptionsControls = new COptionsPanel_Controls(this);
	AddPage(m_pNewOptionsControls, "#DI_NewMenu_GameOptions_Controls");
	m_pNewOptionsSound = new COptionsPanel_Sound(this);
	AddPage(m_pNewOptionsSound, "#DI_NewMenu_GameOptions_Sound");
	m_pNewOptionsDisplay = new COptionsPanel_Display(this);
	AddPage(m_pNewOptionsDisplay, "#DI_NewMenu_GameOptions_Display");
	m_pNewOptionsVideo = new COptionsPanel_Video(this);
	AddPage(m_pNewOptionsVideo, "#DI_NewMenu_GameOptions_Video");
	
	SetApplyButtonVisible(true);
	GetPropertySheet()->SetTabWidth(84);
//	LoadControlSettings("resource/ui/di_options_maindialogue.res");
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
CNewOptionsDialog::~CNewOptionsDialog()
{
}

//-----------------------------------------------------------------------------
// Purpose: Brings the dialog to the fore
//-----------------------------------------------------------------------------
void CNewOptionsDialog::Activate()
{
	BaseClass::Activate();
	EnableApplyButton(false);

	SetPos((ScreenWidth() / 2) - (GetWide() / 2), (ScreenHeight() / 2) - (GetTall() / 2));
}

//-----------------------------------------------------------------------------
// Purpose: Opens the dialog
//-----------------------------------------------------------------------------
void CNewOptionsDialog::Run()
{
	SetTitle("#GameUI_Options", true);
	Activate();
}

//-----------------------------------------------------------------------------
// Purpose: Called when the GameUI is hidden
//-----------------------------------------------------------------------------
void CNewOptionsDialog::OnGameUIHidden()
{
	// tell our children about it
	for (int i = 0; i < GetChildCount(); i++)
	{
		Panel *pChild = GetChild(i);
		if (pChild)
		{
			PostMessage(pChild, new KeyValues("GameUIHidden"));
		}
	}
}
