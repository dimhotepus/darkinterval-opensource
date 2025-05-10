#include "cbase.h"
#include <vgui/IVGui.h>

#include <vgui_controls/Frame.h>
#include "vgui_controls/ComboBox.h"
#include "vgui_controls/PropertySheet.h"
#include <vgui_controls/PropertyPage.h>
#include <vgui_controls/Label.h>

#include <vgui/IScheme.h>
#include <vgui/ILocalize.h>
#include <vgui/ISurface.h>
#include "ienginevgui.h"

#include <FileSystem.h>
#include <convar.h>

#include <KeyValues.h>
#include "igameuifuncs.h"
#include <game/client/iviewport.h>

#include "intromenu.h"
#include <networkstringtabledefs.h>
#include <cdll_client_int.h>
#include "spectatorgui.h"
#include "gamerules.h"

#include "GameUI\IGameUI.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

class VGUICustomSettings : public vgui::PropertyPage
{
public:
	DECLARE_CLASS_SIMPLE(VGUICustomSettings, vgui::PropertyPage);
public:
	VGUICustomSettings(vgui::Panel *parent);
	~VGUICustomSettings() {};
protected:
	virtual void ApplyChanges();
	virtual void ApplyChangesToConVar(const char *pConVarName, int value);
private:
	vgui::ComboBox *m_pRainOption;		// expensive rain splashes
	vgui::ComboBox *m_pGlowOption;		// helpful glow overlays for buttons, keypads, etc.
	vgui::ComboBox *m_pBumpWaterOption; // bumpmapped water splash particles

};

VGUICustomSettings::VGUICustomSettings(vgui::Panel *parent) : PropertyPage(parent, NULL)
{	
	SetVisible(true);

	Msg("Custom Settings Page Constructed\n");

	m_pRainOption = new ComboBox(this, "RainOption", 2, false);		// expensive rain splashes
	m_pRainOption->AddItem("#di_enabled", NULL);
	m_pRainOption->AddItem("#di_disabled", NULL);

	m_pGlowOption = new ComboBox(this, "GlowOption", 2, false);		// helpful glow overlays for buttons, keypads, etc.
	m_pGlowOption->AddItem("#di_enabled", NULL);
	m_pGlowOption->AddItem("#di_disabled", NULL);

	m_pBumpWaterOption = new ComboBox(this, "BumpWaterOption", 2, false); // bumpmapped water splash particles
	m_pBumpWaterOption->AddItem("#di_enabled", NULL);
	m_pBumpWaterOption->AddItem("#di_disabled", NULL);

	SetScheme(vgui::scheme()->LoadSchemeFromFile("resource/SourceScheme.res", "SourceScheme"));

	LoadControlSettings("resource/UI/CustomSettings.res");

	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);
}

void VGUICustomSettings::ApplyChangesToConVar(const char *pConVarName, int value)
{
	Assert(cvar->FindVar(pConVarName));
	char szCmd[256];
	Q_snprintf(szCmd, sizeof(szCmd), "%s %d\n", pConVarName, value);
	engine->ClientCmd_Unrestricted(szCmd);
}

void VGUICustomSettings::ApplyChanges()
{
	ApplyChangesToConVar("di_use_particle_splashes",		m_pRainOption->GetActiveItem());
	ApplyChangesToConVar("glow_outline_effect_enable",		m_pGlowOption->GetActiveItem());
	ApplyChangesToConVar("r_rainexpensive",					m_pBumpWaterOption->GetActiveItem());
}