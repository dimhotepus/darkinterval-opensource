#include "cbase.h"
#include "ioptionspanel_game.h"
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

#define LOCATOR_MODES 2

COptionsPanel_Game::COptionsPanel_Game(vgui::Panel *parent) : PropertyPage(parent, NULL) //(VPANEL parent) : BaseClass(NULL, "DI_Options_Video")
{
	DrawOptionsItems();
	LoadControlSettingsAndUserConfig("resource/ui/di_options_game.res");

	if( ScreenHeight() < 600 )
		LoadControlSettingsAndUserConfig("resource/ui/di_options_game_640.res");

	SetBuildModeEditable(true);
}

void COptionsPanel_Game::DrawOptionsItems()
{
	DrawCrosshairButton();
	DrawQuickHelpButton();
	DrawRadialDamageButton();
	DrawPropTransparencyButton();
//	DrawFriendliFireBox();
//	DrawLocatorBox();
//	DrawHardcoreBox();
	DrawDifficultyBox();
	DrawHudTransparency();
	DrawStickyHudButton();
	DrawItemBlinkButton();
	DrawAutoReloadButton();
}

void COptionsPanel_Game::DrawCrosshairButton() 
{
	m_pCrosshairButton = new CheckButton(this, "CrosshairButton", "#DI_Options_Crosshair");
	if(m_pCrosshairButton) m_pCrosshairButton->GetTooltip()->SetText("#DI_Options_Crosshair_TT");
}
void COptionsPanel_Game::DrawQuickHelpButton() 
{
	m_pQuickHelpButton = new CheckButton(this, "QuickHelpButton", "#DI_Options_QuickHelp");
	if (m_pQuickHelpButton) m_pQuickHelpButton->GetTooltip()->SetText("#DI_Options_QuickHelp_TT");
}
void COptionsPanel_Game::DrawRadialDamageButton()
{
	m_pRadialDamageButton = new CheckButton(this, "RadialDamageButton", "#DI_Options_RadialDamage");
	if (m_pRadialDamageButton) m_pRadialDamageButton->GetTooltip()->SetText("#DI_Options_RadialDamage_TT");
}
void COptionsPanel_Game::DrawPropTransparencyButton()
{
	m_pPropTransparencyButton = new CheckButton(this, "PropTransparencyButton", "#DI_Options_PropTransparency");
	if (m_pPropTransparencyButton) m_pPropTransparencyButton->GetTooltip()->SetText("#DI_Options_PropTransparency_TT");
}
void COptionsPanel_Game::DrawFriendliFireBox() 
{
	m_pFriendlyFireBox = new ComboBox(this, "FriendlyFireBox", 6, false);
	m_pFriendlyFireBox->AddItem("#DI_Options_FriendlyFire_None", NULL);
	m_pFriendlyFireBox->AddItem("#DI_Options_FriendlyFire_NonVital", NULL);
	m_pFriendlyFireBox->AddItem("#DI_Options_FriendlyFire_All", NULL);
}
void COptionsPanel_Game::DrawHudTransparency()
{
	m_pHudTransparencySlider = new IGenericCvarSlider(this, "HudTransparencySlider", "", 1, 255, "hud_transparency", false);
}
void COptionsPanel_Game::DrawStickyHudButton()
{
	m_pStickyHudButton = new CheckButton(this, "StickyHudButton", "#DI_Options_StickyHud");
	if (m_pStickyHudButton) m_pStickyHudButton->GetTooltip()->SetText("#DI_Options_StickyHud_TT");
}
void COptionsPanel_Game::DrawLocatorBox() 
{
	m_pLocatorBox = new ComboBox(this, "LocatorBox", 6, false);
	m_pLocatorBox->AddItem("#DI_Options_Locator_Disabled", NULL);
//	m_pLocatorBox->AddItem("#DI_Options_Locator_PressHotkey", NULL);
	m_pLocatorBox->AddItem("#DI_Options_Locator_HoldHotkey", NULL);
	m_pLocatorBox->AddItem("#DI_Options_Locator_PartOfZoom", NULL);

//	m_pLocatorLifetimeSlider = new IGenericCvarSlider(this, "LocatorLifetimeSlider", "", 1.0f, 15.0f, "di_locator_lifetime", false);
}
void COptionsPanel_Game::DrawHardcoreBox() 
{
	m_pHardcoreBox = new ComboBox(this, "HardcoreBox", 4, false);
	m_pHardcoreBox->AddItem("#DI_Options_Hardcore_NoThanks", NULL);
	m_pHardcoreBox->AddItem("#DI_Options_Hardcore_YesPlease", NULL);
}
void COptionsPanel_Game::DrawDifficultyBox()
{
	m_pDifficultyBox = new ComboBox(this, "DifficultyBox", 4, false);
	m_pDifficultyBox->AddItem("#DI_Options_Difficulty_Easy", NULL);
	m_pDifficultyBox->AddItem("#DI_Options_Difficulty_Normal", NULL);
	m_pDifficultyBox->AddItem("#DI_Options_Difficulty_Hard", NULL);
	if (m_pDifficultyBox) m_pDifficultyBox->GetTooltip()->SetText("#DI_Options_Difficulty_TT");
}
void COptionsPanel_Game::DrawItemBlinkButton()
{
	m_pItemBlinkButton = new CheckButton(this, "ItemBlinkButton", "#DI_Options_ItemBlink");
	if (m_pItemBlinkButton) m_pItemBlinkButton->GetTooltip()->SetText("#DI_Options_ItemBlink_TT");
}

void COptionsPanel_Game::DrawAutoReloadButton()
{
	m_pAutoReloadButton = new CheckButton(this, "AutoReloadButton", "#DI_Options_AutoReload");
	if (m_pAutoReloadButton) m_pAutoReloadButton->GetTooltip()->SetText("#DI_Options_AutoReload_TT");
}

void COptionsPanel_Game::PerformLayout()
{
	BaseClass::PerformLayout();

//	m_pLocatorLifetimeSlider->SetEnabled(m_pLocatorBox->GetActiveItem() != 1);
}

/*
void COptionsPanel_Game::OnDataChanged()
{
	m_pHudTransparencySlider->ApplyChanges();
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}
*/

void COptionsPanel_Game::OnApplyChanges()
{
	SaveGameSettings();
}

void COptionsPanel_Game::ApplyChangesToConVar(const char *pConVarName, int value)
{
	Assert(cvar->FindVar(pConVarName));
	char szCmd[256];
	Q_snprintf(szCmd, sizeof(szCmd), "%s %d\n", pConVarName, value);
	engine->ClientCmd_Unrestricted(szCmd);
}

void COptionsPanel_Game::LoadGameSettings()
{
	ConVarRef crosshair("crosshair");
	m_pCrosshairButton->SetSelected(crosshair.GetBool());

	ConVarRef hud_quickinfo("hud_quickinfo");
	m_pQuickHelpButton->SetSelected(hud_quickinfo.GetBool());

	ConVarRef hud_radial("hud_show_damage_direction");
	m_pRadialDamageButton->SetSelected(hud_radial.GetBool());
/*
	ConVarRef locator_mode("hud_locator_displaymode");
	m_pLocatorBox->ActivateItem(locator_mode.GetInt());
	if( locator_mode.GetInt() < 0 || locator_mode.GetInt() > LOCATOR_MODES )
		m_pLocatorBox->ActivateItem(LOCATOR_MODES);
*/
	ConVarRef skill("skill");
	m_pDifficultyBox->ActivateItem(skill.GetInt() - 1); // skill starts at 1, enum starts at 0

	ConVarRef prop_transparency("di_pickup_prop_transparency");
	m_pPropTransparencyButton->SetSelected(prop_transparency.GetBool());

	m_pHudTransparencySlider->Reset();

	ConVarRef item_blink("item_blink");
	m_pItemBlinkButton->SetSelected(item_blink.GetBool());

	ConVarRef sk_auto_reload_enabled("sk_auto_reload_enabled");
	m_pAutoReloadButton->SetSelected(sk_auto_reload_enabled.GetBool());

	ConVarRef hud_stickyweaponselection("hud_stickyweaponselection");
	m_pStickyHudButton->SetSelected(hud_stickyweaponselection.GetBool());
}

void COptionsPanel_Game::SaveGameSettings()
{
	ApplyChangesToConVar("crosshair", m_pCrosshairButton->IsSelected());
	ApplyChangesToConVar("hud_quickinfo", m_pQuickHelpButton->IsSelected());
	ApplyChangesToConVar("hud_show_damage_direction", m_pRadialDamageButton->IsSelected());
//	ApplyChangesToConVar("hud_locator_displaymode", m_pLocatorBox->GetActiveItem());
	ApplyChangesToConVar("di_pickup_prop_transparency", m_pPropTransparencyButton->IsSelected());
	ApplyChangesToConVar("item_blink", m_pItemBlinkButton->IsSelected());
	ApplyChangesToConVar("skill", m_pDifficultyBox->GetActiveItem() + 1); // skill starts at 1, enum starts at 0
	ApplyChangesToConVar("sk_auto_reload_enabled", m_pAutoReloadButton->IsSelected());
	m_pHudTransparencySlider->ApplyChanges();
	ApplyChangesToConVar("hud_stickyweaponselection", m_pStickyHudButton->IsSelected());
}

void COptionsPanel_Game::OnCommand(const char *command)
{
	if (!Q_strcmp(command, "cancel") || !Q_strcmp(command, "Close"))
	{
		SetVisible(false);
	}
	else if (!Q_strcmp(command, "save"))
	{
		SaveGameSettings();
		SetVisible(false);
	}
	else if (!Q_strcmp(command, "apply"))
	{
		SaveGameSettings();
	}

	BaseClass::OnCommand(command);
}

void COptionsPanel_Game::OnResetData()
{
	LoadGameSettings();
}

/*
void COptionsPanel_Game::OnTextChanged(Panel *pPanel, const char *pszText)
{
	OnDataChanged();
}
*/