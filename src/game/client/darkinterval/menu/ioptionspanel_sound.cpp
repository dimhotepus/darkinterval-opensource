#include "cbase.h"
#include "ioptionspanel_sound.h"
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
#include "vgui/ISurface.h"
#include "igameuifuncs.h"
#include "estranged_system_caps.h"
#include "igenericcvarslider.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

//ConVar di_vgui_soundoptions("di_vgui_soundoptions", "0", FCVAR_CLIENTDLL, "Show/hide sound options tab");

//CON_COMMAND(Toggle_DI_Options_Sound, "Toggle-schmoggle")
//{
//	di_vgui_soundoptions.SetValue(!di_vgui_soundoptions.GetBool());
//};

COptionsPanel_Sound::COptionsPanel_Sound(vgui::Panel *parent) : PropertyPage(parent, NULL) //(VPANEL parent) : BaseClass(NULL, "DI_Options_Sound")
{
//	SetParent(parent);
//	SetTitle("#DI_Options_Sound_Title", true);
//	SetKeyBoardInputEnabled(true);
//	SetMouseInputEnabled(true);
//	SetProportional(false);
//	SetMinimizeButtonVisible(false);
//	SetMaximizeButtonVisible(false);
//	SetCloseButtonVisible(true);
//	SetSizeable(false);
//	SetMoveable(true);

	DrawOptionsItems();
	m_bRequireRestart = false;
	LoadControlSettingsAndUserConfig("resource/ui/di_options_sound.res");

	if (ScreenHeight() < 600)
		LoadControlSettingsAndUserConfig("resource/ui/di_options_sound_640.res");

//	ivgui()->AddTickSignal(GetVPanel(), 100);
}

void COptionsPanel_Sound::DrawOptionsItems()
{
	DrawMasterVolumeSlider();
	DrawMusicVolumeSlider();
	DrawSubtitlesLingerSlider();

	DrawConfigurationBox();
	DrawQualityBox();
	DrawSubtitlesBox();
	DrawSubtitlesLanguageBox();

	DrawSilentInBackgroundButton();
	DrawDisableTinnitusButton();
}

void COptionsPanel_Sound::DrawMasterVolumeSlider()
{
	m_pMasterVolumeSlider = new IGenericCvarSlider(this, "MasterVolumeSlider", "", 0.0f, 1.0f, "volume");
//	m_pMasterVolumeSlider->SetRange(0, 100);
	m_pMasterVolumeSlider->SetDragOnRepositionNob(true);
}

void COptionsPanel_Sound::DrawMusicVolumeSlider()
{
	m_pMusicVolumeSlider = new IGenericCvarSlider(this, "MusicVolumeSlider", "", 0.0f, 1.0f, "snd_musicvolume");
//	m_pMusicVolumeSlider->SetRange(0, 100);
	m_pMusicVolumeSlider->SetDragOnRepositionNob(true);
}

void COptionsPanel_Sound::DrawSubtitlesLingerSlider()
{
	m_pSubtitlesLingerSlider = new IGenericCvarSlider(this, "SubtitlesLingerSlider", "", 0.5f, 3.0f, "cc_linger_time");
//	m_pSubtitlesLingerSlider->SetRange(1, 6);
}

void COptionsPanel_Sound::DrawConfigurationBox()
{
	m_pConfigurationBox = new ComboBox(this, "ConfigurationBox", 6, false);
	m_pConfigurationBox->AddItem("#GameUI_Headphones", new KeyValues("ConfigurationBox", "speakers", 0));
	m_pConfigurationBox->AddItem("#GameUI_2Speakers", new KeyValues("ConfigurationBox", "speakers", 2));
	m_pConfigurationBox->AddItem("#GameUI_4Speakers", new KeyValues("ConfigurationBox", "speakers", 4));
	m_pConfigurationBox->AddItem("#GameUI_5Speakers", new KeyValues("ConfigurationBox", "speakers", 5));
	m_pConfigurationBox->AddItem("#GameUI_7Speakers", new KeyValues("ConfigurationBox", "speakers", 7));
}

void COptionsPanel_Sound::DrawQualityBox()
{
	m_pQualityBox = new ComboBox(this, "QualityBox", 6, false);
	m_pQualityBox->AddItem("#DI_Options_Sound_High", new KeyValues("QualityBox", "quality", SOUNDQUALITY_HIGH));
	m_pQualityBox->AddItem("#DI_Options_Sound_Medium", new KeyValues("QualityBox", "quality", SOUNDQUALITY_MEDIUM));
	m_pQualityBox->AddItem("#DI_Options_Sound_Low", new KeyValues("QualityBox", "quality", SOUNDQUALITY_LOW));
}

void COptionsPanel_Sound::DrawSubtitlesBox()
{
	m_pSubtitlesBox = new ComboBox(this, "Subtitles", 6, false);
	m_pSubtitlesBox->AddItem("#DI_Options_Subtitles_None", NULL);
	m_pSubtitlesBox->AddItem("#DI_Options_Subtitles_Dialogue", NULL);
	m_pSubtitlesBox->AddItem("#DI_Options_Subtitles_Full", NULL);
}

void COptionsPanel_Sound::DrawSubtitlesLanguageBox()
{
	m_pSubtitlesLanguageBox = new ComboBox(this, "SubtitlesLanguage", 4, false);
	m_pSubtitlesLanguageBox->AddItem("#DI_Options_Subtitles_English", NULL);
	m_pSubtitlesLanguageBox->AddItem("#DI_Options_Subtitles_Russian", NULL);
	// Todo: automatically find which languages we have and add them to subtitle choices
}

void COptionsPanel_Sound::DrawSilentInBackgroundButton()
{
	m_pSilentInBackgroundButton = new CheckButton(this, "SilentInBackgroundButton", "#DI_Options_SilentInBackground");
	if (m_pSilentInBackgroundButton) m_pSilentInBackgroundButton->GetTooltip()->SetText("#DI_Options_SilentInBackground_TT");
}

void COptionsPanel_Sound::DrawDisableTinnitusButton()
{
	m_pDisableTinnitusButton = new CheckButton(this, "DisableTinnitusButton", "#DI_Options_DisableTinnitus");
	if(m_pDisableTinnitusButton ) m_pDisableTinnitusButton->GetTooltip()->SetText("#DI_Options_DisableTinnitus_TT");
}

void COptionsPanel_Sound::PerformLayout()
{
	BaseClass::PerformLayout();
}

void COptionsPanel_Sound::CheckRequiresRestart()
{
	m_bRequireRestart = false;
}

void COptionsPanel_Sound::OnControlModified()
{
	m_pMasterVolumeSlider->ApplyChanges();
	m_pMusicVolumeSlider->ApplyChanges();
	m_pSubtitlesLingerSlider->ApplyChanges();
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}

void COptionsPanel_Sound::OnTextChanged()
{
	PostActionSignal(new KeyValues("ApplyButtonEnable"));
}

/*
void COptionsPanel_Sound::OnDataChanged(Panel *pPanel, const char *pszText)
{
	OnControlModified();
}
*/

void COptionsPanel_Sound::OnApplyChanges()
{
	SaveSoundSettings();
}

void COptionsPanel_Sound::ApplyChangesToConVar(const char *pConVarName, int value)
{
	Assert(cvar->FindVar(pConVarName));
	char szCmd[256];
	Q_snprintf(szCmd, sizeof(szCmd), "%s %d\n", pConVarName, value);
	engine->ClientCmd_Unrestricted(szCmd);
}

void COptionsPanel_Sound::LoadSoundSettings()
{
//	ConVarRef mastervolume("volume");
//	m_pMasterVolumeSlider->SetValue((int)(mastervolume.GetFloat() * 100), false);

//	ConVarRef musicvolume("Snd_MusicVolume");
//	m_pMusicVolumeSlider->SetValue((int)(musicvolume.GetFloat() * 100), false);

//	ConVarRef linger("cc_linger_time");
//	m_pSubtitlesLingerSlider->SetValue((int)(linger.GetFloat() * 2));

	m_pMasterVolumeSlider->Reset();
	m_pMusicVolumeSlider->Reset();
	m_pSubtitlesLingerSlider->Reset();

	ConVarRef snd_surround_speakers("Snd_Surround_Speakers");
	if( snd_surround_speakers.GetInt() == 0)
		m_pConfigurationBox->ActivateItem(0);
	else if (snd_surround_speakers.GetInt() == 2)
		m_pConfigurationBox->ActivateItem(1);
	else if (snd_surround_speakers.GetInt() == 4)
		m_pConfigurationBox->ActivateItem(2);
	else if (snd_surround_speakers.GetInt() == 5)
		m_pConfigurationBox->ActivateItem(3);
	else if (snd_surround_speakers.GetInt() == 7)
		m_pConfigurationBox->ActivateItem(4);

	/*
	int speakers = snd_surround_speakers.GetInt();
	{
		for (int itemID = 0; itemID < m_pConfigurationBox->GetItemCount(); itemID++)
		{
			KeyValues *kv = m_pConfigurationBox->GetItemUserData(itemID);
			if (kv && kv->GetInt("speakers") == speakers)
			{
				m_pConfigurationBox->ActivateItem(itemID);
			}
		}
	}
	*/
	// sound quality is made up from several cvars
	ConVarRef Snd_PitchQuality("Snd_PitchQuality");
	ConVarRef dsp_slow_cpu("dsp_slow_cpu");
	int quality = SOUNDQUALITY_LOW;
	if (dsp_slow_cpu.GetBool() == false)
	{
		quality = SOUNDQUALITY_MEDIUM;
	}
	if (Snd_PitchQuality.GetBool())
	{
		quality = SOUNDQUALITY_HIGH;
	}
	// find the item in the list and activate it
	{
		for (int itemID = 0; itemID < m_pQualityBox->GetItemCount(); itemID++)
		{
			KeyValues *kv = m_pQualityBox->GetItemUserData(itemID);
			if (kv && kv->GetInt("quality") == quality)
			{
				m_pQualityBox->ActivateItem(itemID);
			}
		}
	}

	ConVarRef closecaption("closecaption");
	ConVarRef cc_subtitles("cc_subtitles");
	if (closecaption.GetBool())
	{
		if (cc_subtitles.GetBool())
		{
			m_pSubtitlesBox->ActivateItem(1);
		}
		else
		{
			m_pSubtitlesBox->ActivateItem(2);
		}
	}
	else
	{
		m_pSubtitlesBox->ActivateItem(0);
	}

	ConVarRef lang("cc_lang");

	if (!Q_strcmp(lang.GetString(), "russian"))
	{
		m_pSubtitlesLanguageBox->ActivateItem(1);
	}
	else
	{
		m_pSubtitlesLanguageBox->ActivateItem(0);
	}

	ConVarRef silent("snd_mute_losefocus");
	m_pSilentInBackgroundButton->SetSelected(silent.GetBool());

	ConVarRef notinnitus("snd_disable_tinnitus");
	m_pDisableTinnitusButton->SetSelected(notinnitus.GetBool());
}

void COptionsPanel_Sound::SaveSoundSettings()
{
	m_pMasterVolumeSlider->ApplyChanges();
	m_pMusicVolumeSlider->ApplyChanges();
	m_pSubtitlesLingerSlider->ApplyChanges();

	if (m_pSilentInBackgroundButton) ApplyChangesToConVar("snd_mute_losefocus", m_pSilentInBackgroundButton->IsSelected());
	if (m_pDisableTinnitusButton) ApplyChangesToConVar("snd_disable_tinnitus", m_pDisableTinnitusButton->IsSelected());

	int closecaption_value = 0;

	ConVarRef cc_subtitles("cc_subtitles");
	switch (m_pSubtitlesBox->GetActiveItem())
	{
	default:
	case 0:
		closecaption_value = 0;
		cc_subtitles.SetValue(0);
		break;
	case 1:
		closecaption_value = 1;
		cc_subtitles.SetValue(1);
		break;
	case 2:
		closecaption_value = 1;
		cc_subtitles.SetValue(0);
		break;
	}

	char cmd[64];
	Q_snprintf(cmd, sizeof(cmd), "closecaption %i\n", closecaption_value);
	engine->ClientCmd_Unrestricted(cmd);

	ConVarRef snd_surround_speakers("Snd_Surround_Speakers");
	int speakers = 0;
	switch (m_pConfigurationBox->GetActiveItem())
	{
	case 0:
		speakers = 0;
		break;
	case 1: 
		speakers = 2;
		break;
	case 2: 
		speakers = 4;
		break;
	case 3: 
		speakers = 5;
		break;
	case 4: 
		speakers = 7;
		break;
	default: 
		speakers = 0;
		break;
	}

	snd_surround_speakers.SetValue(speakers);

	// quality
	ConVarRef Snd_PitchQuality("Snd_PitchQuality");
	ConVarRef dsp_slow_cpu("dsp_slow_cpu");
	int quality = m_pQualityBox->GetActiveItemUserData()->GetInt("quality");
	switch (quality)
	{
	case SOUNDQUALITY_LOW:
		dsp_slow_cpu.SetValue(true);
		Snd_PitchQuality.SetValue(false);
		break;
	case SOUNDQUALITY_MEDIUM:
		dsp_slow_cpu.SetValue(false);
		Snd_PitchQuality.SetValue(false);
		break;
	default:
		Assert("Undefined sound quality setting.");
	case SOUNDQUALITY_HIGH:
		dsp_slow_cpu.SetValue(false);
		Snd_PitchQuality.SetValue(true);
		break;
	};

	// headphones at high quality get enhanced stereo turned on
	ConVarRef dsp_enhance_stereo("dsp_enhance_stereo");
	if (speakers == 0 && quality == SOUNDQUALITY_HIGH)
	{
		dsp_enhance_stereo.SetValue(1);
	}
	else
	{
		dsp_enhance_stereo.SetValue(0);
	}

	char lang[16];

	switch (m_pSubtitlesLanguageBox->GetActiveItem())
	{
	default:
	case 0:
		sprintf(lang, "english");
		break;
	case 1:
		sprintf(lang, "russian");
		break;
	}

	char langcmd[64];
	Q_snprintf(langcmd, sizeof(langcmd), "cc_lang %s\n", lang);
	ConVarRef currentlang("cc_lang");
	if( Q_strcmp(currentlang.GetString(), lang) != 0)
		engine->ClientCmd_Unrestricted(langcmd);
}

void COptionsPanel_Sound::OnCommand(const char *command)
{
	if (!Q_strcmp(command, "cancel") || !Q_strcmp(command, "Close"))
	{
		SetVisible(false);
	//	di_vgui_soundoptions.SetValue("0");
	}
	else if (!Q_strcmp(command, "save"))
	{
		SaveSoundSettings();
		SetVisible(false);
	//	di_vgui_soundoptions.SetValue("0");
	}
	else if (!Q_strcmp(command, "apply"))
	{
		SaveSoundSettings();
	}

	BaseClass::OnCommand(command);
}
void COptionsPanel_Sound::OnResetData()
{
	LoadSoundSettings();
}