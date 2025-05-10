#ifndef IOPTIONSPANEL_GAME_H
#define IOPTIONSPANEL_GAME_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/Ivgui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include "ienginevgui.h"
#include <vgui/ISurface.h>
#include "igenericcvarslider.h"
#include <vgui_controls/PropertyPage.h>
#include "GameUI/IGameUI.h"

class COptionsPanel_Game : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COptionsPanel_Game, vgui::PropertyPage);

	COptionsPanel_Game(vgui::Panel *parent);
	~COptionsPanel_Game() {};

protected:
	void				SaveGameSettings(void);
	void				LoadGameSettings(void);
	virtual void		PerformLayout();
	virtual void		OnCommand(const char* pcCommand);
	virtual void		OnApplyChanges(void);
	virtual void		OnResetData(void);
//	virtual void		OnTextChanged(Panel *pPanel, const char *pszText);

	void				DrawOptionsItems(void);
	void				DrawCrosshairButton(void);
	void				DrawQuickHelpButton(void);
	void				DrawRadialDamageButton(void);
	void				DrawPropTransparencyButton(void);
	void				DrawHudTransparency(void);
	void				DrawStickyHudButton(void);
	void				DrawFriendliFireBox(void);
	void				DrawLocatorBox(void);
	void				DrawHardcoreBox(void);
	void				DrawDifficultyBox(void);
	void				DrawItemBlinkButton(void);
	void				DrawAutoReloadButton(void);

private:
	void ApplyChangesToConVar(const char *pConVarName, int value);

	vgui::CheckButton *m_pCrosshairButton;
	vgui::CheckButton *m_pQuickHelpButton;
	vgui::CheckButton *m_pRadialDamageButton;
	vgui::CheckButton *m_pPropTransparencyButton;
	vgui::ComboBox *m_pFriendlyFireBox;
	vgui::ComboBox *m_pLocatorBox;
//	IGenericCvarSlider *m_pLocatorLifetimeSlider;
	IGenericCvarSlider *m_pHudTransparencySlider;
	vgui::CheckButton *m_pStickyHudButton;
	vgui::ComboBox *m_pHardcoreBox;
	vgui::ComboBox *m_pDifficultyBox;
	vgui::CheckButton *m_pItemBlinkButton;
	vgui::CheckButton *m_pAutoReloadButton;


//	IGenericCvarSlider *m_pFOVSlider;
//	IGenericCvarSlider *m_pCameraRollSlider;
//	IGenericCvarSlider *m_pBrightnessSlider;

	bool m_bLastVisibilityState;

public:
//	MESSAGE_FUNC(OnDataChanged, "ControlModified");
	MESSAGE_FUNC(OnControlModified, "ControlModified")
	{
		PostActionSignal(new KeyValues("ApplyButtonEnable"));
	}
	MESSAGE_FUNC(OnTextChanged, "TextChanged")
	{
		OnControlModified();
	}
	MESSAGE_FUNC(OnCheckButtonChecked, "CheckButtonChecked")
	{
		OnControlModified();
	}
};

#endif