#ifndef IOPTIONSPANEL_SOUND_H
#define IOPTIONSPANEL_SOUND_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/Ivgui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include "ienginevgui.h"
#include <vgui/ISurface.h>

#include <vgui_controls/PropertyPage.h>
#include "igenericcvarslider.h"
#include <language.h>
#include "GameUI/IGameUI.h"

enum SoundQuality_e
{
	SOUNDQUALITY_LOW,
	SOUNDQUALITY_MEDIUM,
	SOUNDQUALITY_HIGH,
};

class COptionsPanel_Sound : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COptionsPanel_Sound, vgui::PropertyPage);

	COptionsPanel_Sound(vgui::Panel *parent);
	~COptionsPanel_Sound() {};

protected:
	void				SaveSoundSettings(void);
	void				LoadSoundSettings(void);
	virtual void		OnCommand(const char* pcCommand);
	virtual void		PerformLayout();
	void				CheckRequiresRestart(void);
	virtual void		OnApplyChanges(void);
	virtual void		OnResetData(void);
//	virtual void		OnTextChanged(Panel *pPanel, const char *pszText);

	void				DrawOptionsItems(void);

	void				DrawMasterVolumeSlider(void);
	void				DrawMusicVolumeSlider(void);
	void				DrawConfigurationBox(void);
	void				DrawQualityBox(void);
	void				DrawSubtitlesBox(void);
	void				DrawSubtitlesLanguageBox(void);
	void				DrawSubtitlesLingerSlider(void);
	void				DrawSilentInBackgroundButton(void);
	void				DrawDisableTinnitusButton(void);

private:
	void				ApplyChangesToConVar(const char *pConVarName, int value);

	IGenericCvarSlider *m_pMasterVolumeSlider;
	IGenericCvarSlider *m_pMusicVolumeSlider;
	IGenericCvarSlider *m_pSubtitlesLingerSlider;

	vgui::ComboBox *m_pConfigurationBox;
	vgui::ComboBox *m_pQualityBox;
	vgui::ComboBox *m_pSubtitlesBox;
	vgui::ComboBox *m_pSubtitlesLanguageBox;

	vgui::CheckButton *m_pSilentInBackgroundButton;
	vgui::CheckButton *m_pDisableTinnitusButton;
	bool m_bLastVisibilityState;
	bool m_bRequireRestart;

//	MESSAGE_FUNC(OnDataChanged, "DataChanged");
	MESSAGE_FUNC(OnControlModified, "ControlModified");
	MESSAGE_FUNC(OnTextChanged, "TextChanged");
	MESSAGE_FUNC(OnCheckButtonChecked, "CheckButtonChecked")
	{
		OnControlModified();
	}
};
#endif