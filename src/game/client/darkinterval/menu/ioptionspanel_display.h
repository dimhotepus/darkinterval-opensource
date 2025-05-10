#ifndef IOPTIONSPANEL_DISPLAY_H
#define IOPTIONSPANEL_DISPLAY_H
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

class COptionsPanel_Display : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COptionsPanel_Display, vgui::PropertyPage);

	COptionsPanel_Display(vgui::Panel *parent);
	~COptionsPanel_Display() {};

protected:
	void				SaveDisplaySettings(void);
	void				LoadDisplaySettings(void);
	virtual void		PerformLayout();
	virtual void		OnCommand(const char* pcCommand);
	void				CheckRequiresRestart(void);
	virtual void		OnApplyChanges(void);
	virtual void		OnResetData(void);
//	virtual void		OnTick();
//	virtual void		OnTextChanged(Panel *pPanel, const char *pszText);
	
	void				DrawOptionsItems(void);
	void				DrawScreenResolutionBox(void);
	void				DrawScreenAspectRatioBox(void);
	void				DrawWindowedModeBox(void);
	void				DrawFOVSlider(void);
	void				DrawCameraRollSlider(void);
	void				DrawBrightnessSlider(void);
	void				DrawVSyncButton(void);

private:
	void ApplyChangesToConVar(const char *pConVarName, int value);

	vgui::ComboBox *m_pScreenResolutionBox;
//	vgui::ComboBox *m_pScreenAspectRatioBox;
	vgui::ComboBox *m_pWindowedModeBox;
//	vgui::TextEntry *m_pFOVText;
	vgui::Label *m_pFOVText;
	IGenericCvarSlider *m_pFOVSlider;
	IGenericCvarSlider *m_pCameraRollSlider;
	IGenericCvarSlider *m_pBrightnessSlider;
	vgui::CheckButton *m_pVSyncButton;

	bool m_bLastVisibilityState;
	bool m_bRequireRestart;

	void PrepareResolutionList();
	void SetCurrentResolutionComboItem();
	int m_nSelectedMode; // -1 if we are running in a nonstandard mode

public:
//	MESSAGE_FUNC(OnDataChanged, "ControlModified");
	MESSAGE_FUNC(OnControlModified, "ControlModified");
	MESSAGE_FUNC_PTR_CHARPTR(OnTextChanged, "TextChanged", panel, text);
	MESSAGE_FUNC(OnCheckButtonChecked, "CheckButtonChecked")
	{
		OnControlModified();
	}
};

#endif