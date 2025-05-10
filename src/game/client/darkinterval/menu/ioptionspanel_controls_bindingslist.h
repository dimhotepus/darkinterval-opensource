#ifndef IOPTIONSPANEL_CONTROLS_BINDINGSLIST_H
#define IOPTIONSPANEL_CONTROLS_BINDINGSLIST_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/Ivgui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/SectionedListPanel.h>
#include "ienginevgui.h"
#include <vgui/ISurface.h>
#include <vgui_controls/PropertyPage.h>
#include "igenericcvarslider.h"
#include <language.h>
#include "GameUI/IGameUI.h"

class COptionsPanel_Controls_BindingsList : public vgui::SectionedListPanel
{
public:
	// Construction
	COptionsPanel_Controls_BindingsList(vgui::Panel *parent, const char *listName);
	virtual			~COptionsPanel_Controls_BindingsList();

	// Start/end capturing
	virtual void	StartCaptureMode(vgui::HCursor hCursor = NULL);
	virtual void	EndCaptureMode(vgui::HCursor hCursor = NULL);
	virtual bool	IsCapturing();

	// Set which item should be associated with the prompt
	virtual void	SetItemOfInterest(int itemID);
	virtual int		GetItemOfInterest();

	virtual void	OnMousePressed(vgui::MouseCode code);
	virtual void	OnMouseDoublePressed(vgui::MouseCode code);

private:
	void ApplySchemeSettings(vgui::IScheme *pScheme);

	// Are we showing the prompt?
	bool			m_bCaptureMode;
	// If so, where?
	int				m_nClickRow;
	// Font to use for showing the prompt
	vgui::HFont		m_hFont;
	// panel used to edit
	class CInlineEditPanel *m_pInlineEditPanel;
	int m_iMouseX, m_iMouseY;

	typedef vgui::SectionedListPanel BaseClass;
};

#endif