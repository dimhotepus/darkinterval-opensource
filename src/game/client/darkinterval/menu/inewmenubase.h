#ifndef INEWMENUBASE_H
#define INEWMENUBASE_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui_controls/Frame.h>
#include "GameUI/IGameUI.h"
#include "video/ivideoservices.h"
#include "igenericmenupanel.h"
#include "inewmenulist.h"

#define BLANK Color(0, 0, 0, 0)

class INewMenuBase : public vgui::Frame // It's the base of the menu which holds other elements on it.
{
	DECLARE_CLASS_SIMPLE(INewMenuBase, vgui::Frame);

	INewMenuBase(vgui::VPANEL parent);
	~INewMenuBase() {};

protected:
	virtual void OnKeyCodePressed(vgui::KeyCode code);
	virtual void OnKeyCodeTyped(vgui::KeyCode code);
	virtual void OnTick();
	virtual void OnThink();
	virtual void OnCommand(const char*);
	virtual void PerformLayout();
	virtual void ApplySchemeSettings(vgui::IScheme*);

private:
	void							SetActiveMenu(const char* newMenu);
	void							AddMenuPanels();
	vgui::Label						*gameTitle;
	vgui::Label						*gameTitleGlow;
	vgui::Label						*gameTitle2;
	vgui::Label						*gameTitle2Glow;
//	vgui::Label						*logo;
//	vgui::Label						*tag;
//	vgui::Panel						*darkBackground;
	IGameUI							*m_pGameUI;
//	CUtlVector<vgui::Panel*>		menuPanels;
	INewMenuMainList				*menuList;
	vgui::IScheme					*m_pScheme;
	int buttonWidth;

	///////////////////////////////////////////////////////////
	//vgui::DHANDLE<vgui::PropertyDialog> m_hOptionsDialog;
	///////////////////////////////////////////////////////////
};

class INewMenuVideoPanel : public vgui::EditablePanel
{
	DECLARE_CLASS_SIMPLE(INewMenuVideoPanel, vgui::EditablePanel);
public:

	INewMenuVideoPanel(unsigned int nXPos, unsigned int nYPos, unsigned int nHeight, unsigned int nWidth, bool allowAlternateMedia = true);

	virtual ~INewMenuVideoPanel(void);

	virtual void Activate(void);
	virtual void Paint(void);
	virtual void DoModal(void);
	virtual void OnClose(void);
	virtual void GetPanelPos(int &xpos, int &ypos);
	
	bool BeginPlayback(const char *pFilename, bool bLooped);

	void SetBlackBackground(bool bBlack) { m_bBlackBackground = bBlack; }

protected:

	virtual void OnTick(void) { BaseClass::OnTick(); }
	virtual void OnCommand(const char *pcCommand) { BaseClass::OnCommand(pcCommand); }
	virtual void OnVideoOver() {}

protected:
	IVideoMaterial *m_VideoMaterial;

	IMaterial		*m_pMaterial;
	int				m_nPlaybackHeight;			// Calculated to address ratio changes
	int				m_nPlaybackWidth;
	
	float			m_flU;	// U,V ranges for video on its sheet
	float			m_flV;

	bool			m_bBlackBackground;
	bool			m_bAllowAlternateMedia;
	bool			m_bLooped;
};


// Creates a VGUI panel which plays a video and executes a client command at its finish (if specified)
extern bool NewMenuVideoPanel_Create(unsigned int nXPos, unsigned int nYPos,
	unsigned int nWidth, unsigned int nHeight,
	const char *pVideoFilename, bool bLooped);

#endif //INEWMENUBASE