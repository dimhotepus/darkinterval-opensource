#ifndef IOPTIONSPANEL_VIDEO_H
#define IOPTIONSPANEL_VIDEO_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/Ivgui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include "ienginevgui.h"
#include <vgui/ISurface.h>

#include <vgui_controls/PropertyPage.h>
#include "GameUI/IGameUI.h"

struct AAMode_t
{
	int m_nNumSamples;
	int m_nQualityLevel;
};

class COptionsPanel_Video : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COptionsPanel_Video, vgui::PropertyPage);

	COptionsPanel_Video(vgui::Panel *parent);
	~COptionsPanel_Video() {};

protected:
	void				SaveVideoSettings(void);
	void				LoadVideoSettings(void);
	virtual void		OnCommand(const char* pcCommand);
	virtual void		PerformLayout();
	void				CheckRequiresRestart(void);
	virtual void		OnApplyChanges(void);
	virtual void		OnResetData(void);
//	virtual void		OnTextChanged(Panel *pPanel, const char *pszText);

	int FindMSAAMode(int nAASamples, int nAAQuality);

	void				DrawOptionsItems(void);
	void				DrawMenuLabels(void);
	void				DrawModelDetailBox(void);
	void				DrawTextureSizeBox(void);
	void				DrawScreenSpaceReflectionsBox(void);
	void				DrawSpecularityBox(void);
	void				DrawAntialiasingBox(void);
	void				DrawFiltrationBox(void);
	void				DrawShadowDetailBox(void);
	void				DrawHDRSettingsBox(void);
	void				DrawMulticoreBox(void);
	void				DrawMotionBlurCheckButton(void);
	void				DrawColorCorrectionCheckButton(void);
	void				DrawVignetteCheckButton(void);
	void				DrawChromaticCheckButton(void);
	void				DrawDOFCheckButton(void);

private:
	void ApplyChangesToConVar(const char *pConVarName, int value);

	vgui::ComboBox *m_pModelDetailBox;
	vgui::ComboBox *m_pTextureSizeBox;
	vgui::ComboBox *m_pScreenSpaceReflectionsBox;
	vgui::ComboBox *m_pSpecularityBox;
	vgui::ComboBox *m_pAntialiasingBox;
	vgui::ComboBox *m_pFiltrationBox;
	vgui::ComboBox *m_pShadowDetailBox;
	vgui::ComboBox *m_pHDRSettingsBox;

	vgui::CheckButton *m_pMulticoreBox;
	vgui::CheckButton *m_pMotionBlurButton;
	vgui::CheckButton *m_pColorCorrectionButton;
	vgui::CheckButton *m_pVignetteButton;
	vgui::CheckButton *m_pChromaticButton;
	vgui::CheckButton *m_pDOFButton;

	bool m_bLastVisibilityState;
	bool m_bRequireRestart;
	
	int m_nSelectedMode; // -1 if we are running in a nonstandard mode

	int m_nNumAAModes;
	AAMode_t m_nAAModes[16];
	
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