#include "cbase.h"
#include "ioptionspanel_display.h"
#include <vgui/Ivgui.h>
#include <vgui_controls/button.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Divider.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Tooltip.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/URLLabel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Slider.h>
#include "igenericcvarslider.h"
#include "vgui/ISurface.h"
#include "igameuifuncs.h"
#include "materialsystem\materialsystem_config.h"
#include "estranged_system_caps.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
using namespace vgui;

struct RatioToAspectMode_t
{
	int anamorphic;
	float aspectRatio;
};

RatioToAspectMode_t g_RatioToAspectModes[] =
{
	{ 0,		4.0f / 3.0f },
	{ 1,		16.0f / 9.0f },
	{ 2,		16.0f / 10.0f },
	{ 3,		1.0f },
};

int GetScreenAspectMode(int width, int height)
{
	float aspectRatio = (float)width / (float)height;

	// just find the closest ratio
	float closestAspectRatioDist = 99999.0f;
	int closestAnamorphic = 0;
	for (int i = 0; i < ARRAYSIZE(g_RatioToAspectModes); i++)
	{
		float dist = fabs(g_RatioToAspectModes[i].aspectRatio - aspectRatio);
		if (dist < closestAspectRatioDist)
		{
			closestAspectRatioDist = dist;
			closestAnamorphic = g_RatioToAspectModes[i].anamorphic;
		}
	}

	return closestAnamorphic;
}

void GetResolutionName(vmode_t *mode, char *sz, int sizeofsz)
{
	/*
	if (mode->width == 1280 && mode->height == 1024)
	{
	// LCD native monitor resolution gets special case
	Q_snprintf(sz, sizeofsz, "%i x %i (LCD)", mode->width, mode->height);
	}
	else
	{
	*/
	Q_snprintf(sz, sizeofsz, "%i x %i", mode->width, mode->height);
	//} // DI change: What? This is ridiculous. What's that even supposed to mean?
	// 1280x1024 are somehow exclusive to LCDs? Maybe in 2004 (no)
	// but not now. Weird!
}

COptionsPanel_Display::COptionsPanel_Display(vgui::Panel *parent) : PropertyPage(parent, NULL) //(VPANEL parent) : BaseClass(NULL, "DI_Options_Video")
{
	DrawOptionsItems();
	m_bRequireRestart = false;
	LoadControlSettingsAndUserConfig("resource/ui/di_options_display.res");

	if (ScreenHeight() < 600)
		LoadControlSettingsAndUserConfig("resource/ui/di_options_display_640.res");

	SetBuildModeEditable(true);
	//	ivgui()->AddTickSignal(GetVPanel(), 100);
}

void COptionsPanel_Display::DrawOptionsItems()
{
	DrawScreenResolutionBox();
	DrawScreenAspectRatioBox();
	DrawWindowedModeBox();
	DrawFOVSlider();
	DrawCameraRollSlider();
	DrawBrightnessSlider();
	DrawVSyncButton();

	PrepareResolutionList();
}

void COptionsPanel_Display::DrawScreenResolutionBox()
{
	m_pScreenResolutionBox = new ComboBox(this, "ScreenResolutionBox", 8, false);
}
void COptionsPanel_Display::DrawScreenAspectRatioBox()
{
//	m_pScreenAspectRatioBox = new ComboBox(this, "ScreenAspectRatioBox", 6, false);
/*
	char pszAspectName[3][64];
	wchar_t *unicodeText = g_pVGuiLocalize->Find("#GameUI_AspectNormal");
	g_pVGuiLocalize->ConvertUnicodeToANSI(unicodeText, pszAspectName[0], 32);
	unicodeText = g_pVGuiLocalize->Find("#GameUI_AspectWide16x9");
	g_pVGuiLocalize->ConvertUnicodeToANSI(unicodeText, pszAspectName[1], 32);
	unicodeText = g_pVGuiLocalize->Find("#GameUI_AspectWide16x10");
	g_pVGuiLocalize->ConvertUnicodeToANSI(unicodeText, pszAspectName[2], 32);

	int iNormalItemID = m_pScreenAspectRatioBox->AddItem(pszAspectName[0], NULL);
	int i16x9ItemID = m_pScreenAspectRatioBox->AddItem(pszAspectName[1], NULL);
	int i16x10ItemID = m_pScreenAspectRatioBox->AddItem(pszAspectName[2], NULL);
*/
}
void COptionsPanel_Display::DrawWindowedModeBox()
{
	m_pWindowedModeBox = new ComboBox(this, "WindowedModeBox", 6, false);
	m_pWindowedModeBox->AddItem("#DI_Options_Fullscreen", NULL);
//	m_pWindowedModeBox->AddItem("#DI_Options_Borderless", NULL);
	m_pWindowedModeBox->AddItem("#DI_Options_Windowed", NULL);
}
void COptionsPanel_Display::DrawFOVSlider()
{
	m_pFOVSlider = new IGenericCvarSlider( this, "FOVSlider", "", 50.0f, 150.0f, "fov_desired", true);

	m_pFOVSlider->SetNumTicks(20);

	m_pFOVSlider->SetDragOnRepositionNob(true);

	m_pFOVSlider->SetMouseInputEnabled(true);
	
	m_pFOVText = new Label(this, "FOVText", ""); //TextEntry(this, "FOVText");
}
void COptionsPanel_Display::DrawCameraRollSlider()
{
	m_pCameraRollSlider = new IGenericCvarSlider(this, "CameraRollSlider", "", 0.0f, 2.0f, "sv_rollangle");
}
void COptionsPanel_Display::DrawBrightnessSlider()
{
	m_pBrightnessSlider = new IGenericCvarSlider(this, "GammaSlider", "", 1.6f, 2.6f, "mat_monitorgamma", false);
}
void COptionsPanel_Display::DrawVSyncButton()
{
	m_pVSyncButton = new CheckButton(this, "VSyncButton", "#DI_Options_VSync");
}

void COptionsPanel_Display::PerformLayout()
{
	BaseClass::PerformLayout();
}

void COptionsPanel_Display::CheckRequiresRestart()
{
	m_bRequireRestart = false;
}

void COptionsPanel_Display::OnControlModified()
{
	char szCmd[256];
	Q_snprintf(szCmd, sizeof(szCmd), "fov %i\n", (int)m_pFOVSlider->GetSliderValue());
	engine->ClientCmd_Unrestricted(szCmd);

	char buf[64];
	Q_snprintf(buf, sizeof(buf), " %i", (int)m_pFOVSlider->GetSliderValue());
	m_pFOVText->SetText(buf);

	m_pBrightnessSlider->ApplyChanges();
	m_pCameraRollSlider->ApplyChanges();
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}

void COptionsPanel_Display::OnApplyChanges()
{
	SaveDisplaySettings();
}

void COptionsPanel_Display::ApplyChangesToConVar(const char *pConVarName, int value)
{
	Assert(cvar->FindVar(pConVarName));
	char szCmd[256];
	Q_snprintf(szCmd, sizeof(szCmd), "%s %d\n", pConVarName, value);
	engine->ClientCmd_Unrestricted(szCmd);
}

void COptionsPanel_Display::LoadDisplaySettings()
{
	ConVarRef vsync("mat_vsync");
	m_pVSyncButton->SetSelected(vsync.GetBool());

	char buf[64];
	Q_snprintf(buf, sizeof(buf), " %i", (int)m_pFOVSlider->GetSliderValue());
	m_pFOVText->SetText(buf);

	const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();
/*
	int iAspectMode = GetScreenAspectMode(config.m_VideoMode.m_Width, config.m_VideoMode.m_Height);
	switch (iAspectMode)
	{
	default:
	case 0:
		m_pScreenAspectRatioBox->ActivateItem(0);
		break;
	case 1:
		m_pScreenAspectRatioBox->ActivateItem(1);
		break;
	case 2:
		m_pScreenAspectRatioBox->ActivateItem(2);
		break;
	}
*/
	// reset UI elements
	m_pWindowedModeBox->ActivateItem(config.Windowed() ? 1 : 0);

	PrepareResolutionList();
	SetCurrentResolutionComboItem();
}

void COptionsPanel_Display::SaveDisplaySettings()
{
	// resolution
	char sz[16];
//	if (m_nSelectedMode == -1)
//	{
		m_pScreenResolutionBox->GetText(sz, 16);
//	}
//	else
//	{
//		m_pScreenResolutionBox->GetItemText(m_nSelectedMode, sz, 256);
//	}

	int width = 0, height = 0;
	sscanf(sz, "%i x %i", &width, &height);

	// windowed
	bool windowed = (m_pWindowedModeBox->GetActiveItem() > 0) ? true : false;

	// make sure there is a change
	const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();
	if (config.m_VideoMode.m_Width != width
		|| config.m_VideoMode.m_Height != height
		|| config.Windowed() != windowed)
	{
		// set mode
		char szCmd[64];
		Q_snprintf(szCmd, sizeof(szCmd), "mat_setvideomode %i %i %i\n", width, height, windowed ? 1 : 0);
		engine->ClientCmd_Unrestricted(szCmd);
	}

	Msg("%i %i\n", width, height);

	ApplyChangesToConVar("mat_vsync", m_pVSyncButton->IsSelected());
	
	m_pFOVSlider->SetSliderValue((int)m_pFOVSlider->GetSliderValue());
	m_pFOVSlider->ApplyChanges();
	m_pBrightnessSlider->ApplyChanges();
	m_pCameraRollSlider->ApplyChanges();

	// apply changes
	engine->ClientCmd_Unrestricted("mat_savechanges\n");
}

void COptionsPanel_Display::OnCommand(const char *command)
{
	if (!Q_strcmp(command, "cancel") || !Q_strcmp(command, "Close"))
	{
		SetVisible(false);
		//	di_vgui_advancedvideooptions.SetValue("0");
	}
	else if (!Q_strcmp(command, "save"))
	{
		SaveDisplaySettings();
		SetVisible(false);
		//	di_vgui_advancedvideooptions.SetValue("0");
	}
	else if (!Q_strcmp(command, "apply"))
	{
		SaveDisplaySettings();
	}

	BaseClass::OnCommand(command);
}

void COptionsPanel_Display::OnResetData()
{
	m_bRequireRestart = false;
	LoadDisplaySettings();
}

void COptionsPanel_Display::OnTextChanged(Panel *pPanel, const char *pszText)
{
	if (pPanel == m_pScreenResolutionBox)
	{
		const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();

		m_nSelectedMode = m_pScreenResolutionBox->GetActiveItem();

		int w = 0, h = 0;
		sscanf(pszText, "%i x %i", &w, &h);
		if (config.m_VideoMode.m_Width != w || config.m_VideoMode.m_Height != h)
		{
			OnControlModified();
		}
	}
//	else if (pPanel == m_pScreenAspectRatioBox)
//	{
//		PrepareResolutionList();
//	}
	else if (pPanel == m_pWindowedModeBox)
	{
		PrepareResolutionList();
		OnControlModified();
	}

	OnControlModified();
}

void COptionsPanel_Display::PrepareResolutionList()
{
	// get the currently selected resolution
	char sz[16];
	m_pScreenResolutionBox->GetText(sz, 16);
	int currentWidth = 0, currentHeight = 0;
	sscanf(sz, "%i x %i", &currentWidth, &currentHeight);

	// Clean up before filling the info again.
	m_pScreenResolutionBox->DeleteAllItems();
//	m_pScreenAspectRatioBox->SetItemEnabled(1, false);
//	m_pScreenAspectRatioBox->SetItemEnabled(2, false);

	// get full video mode list
	vmode_t *plist = NULL;
	int count = 0;
	gameuifuncs->GetVideoModes(&plist, &count);

	const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();

//	bool bWindowed = (m_pWindowedModeBox->GetActiveItem() > 0);
	int desktopWidth, desktopHeight;
	gameuifuncs->GetDesktopResolution(desktopWidth, desktopHeight);

	// iterate all the video modes adding them to the dropdown
//	bool bFoundWidescreen = false;
	int selectedItemID = -1;
	for (int i = 0; i < count; i++, plist++)
	{
		char sz[16];
		GetResolutionName(plist, sz, sizeof(sz));

		// don't show modes bigger than the desktop for windowed mode
	//	if (bWindowed && (plist->width > desktopWidth || plist->height > desktopHeight))
	//		continue;

		int itemID = -1;

//		int iAspectMode = GetScreenAspectMode(plist->width, plist->height);
//		if (iAspectMode > 0)
//		{
//			m_pScreenAspectRatioBox->SetItemEnabled(iAspectMode, true);
//			bFoundWidescreen = true;
//		}

		// filter the list for those matching the current aspect
//		if (iAspectMode == m_pScreenAspectRatioBox->GetActiveItem())
//		{
			itemID = m_pScreenResolutionBox->AddItem(sz, NULL);
//		}

		// try and find the best match for the resolution to be selected
		if (plist->width == currentWidth && plist->height == currentHeight)
		{
			selectedItemID = itemID;
		}
		else if (selectedItemID == -1 && plist->width == config.m_VideoMode.m_Width && plist->height == config.m_VideoMode.m_Height)
		{
			selectedItemID = itemID;
		}
	}

	// disable ratio selection if we can't display widescreen.
//	m_pScreenAspectRatioBox->SetEnabled(bFoundWidescreen);

	m_nSelectedMode = selectedItemID;

	if (selectedItemID != -1)
	{
		m_pScreenResolutionBox->ActivateItem(selectedItemID);
	}
	else
	{
		char sz[16];
		sprintf(sz, "%d x %d", config.m_VideoMode.m_Width, config.m_VideoMode.m_Height);
		m_pScreenResolutionBox->SetText(sz);
	}
}

void COptionsPanel_Display::SetCurrentResolutionComboItem()
{
	vmode_t *plist = NULL;
	int count = 0;
	gameuifuncs->GetVideoModes(&plist, &count);

	const MaterialSystem_Config_t &config = materials->GetCurrentConfigForVideoCard();

	int resolution = -1;
	for (int i = 0; i < count; i++, plist++)
	{
		if (plist->width == config.m_VideoMode.m_Width &&
			plist->height == config.m_VideoMode.m_Height)
		{
			resolution = i;
			break;
		}
	}

	if (resolution != -1)
	{
		char sz[16];
		GetResolutionName(plist, sz, sizeof(sz));
		m_pScreenResolutionBox->SetText(sz);
	}
}