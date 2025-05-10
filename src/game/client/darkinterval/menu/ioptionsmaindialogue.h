//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef IOPTIONSMAINDIALOGUE_H
#define IOPTIONSMAINDIALOGUE_H
#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/PropertyDialog.h"
#include "vgui_controls/KeyRepeat.h"

//-----------------------------------------------------------------------------
// Purpose: Holds all the game option pages
//-----------------------------------------------------------------------------
class CNewOptionsDialog : public vgui::PropertyDialog
{
	DECLARE_CLASS_SIMPLE(CNewOptionsDialog, vgui::PropertyDialog);

public:
	CNewOptionsDialog(vgui::Panel *parent);
	~CNewOptionsDialog();

	void Run();
	virtual void Activate();

	MESSAGE_FUNC(OnGameUIHidden, "GameUIHidden");	// called when the GameUI is hidden

private:
	class COptionsPanel_Game *m_pNewOptionsGame;
	class COptionsPanel_Sound *m_pNewOptionsSound;
	class COptionsPanel_Display *m_pNewOptionsDisplay;
	class COptionsPanel_Video *m_pNewOptionsVideo;
	class COptionsPanel_Controls *m_pNewOptionsControls;
};

//#define OPTIONS_MAX_NUM_ITEMS 15
//struct OptionData_t;

#endif // OPTIONSDIALOG_H
