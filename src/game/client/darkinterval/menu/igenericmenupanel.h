#include "cbase.h"
#include <vgui/Ivgui.h>
#include <vgui_controls/Panel.h>
#include "igenericmenubutton.h"
#include "igenericmenucheckbutton.h"
#include "igenericmenuslider.h"

#define NEW_MAIN_MENU_BASE_NAME		"NewMainMenuBase"
#define NEW_MAIN_MENU_LISTING_NAME	"NewMainMenuList"
#define NEW_OPTIONS_MENU_NAME		"NewOptionsDialogue"
#define NEW_EXIT_MENU_NAME			"NewExitDialogue"
#define NEW_START_GAME_MENU_NAME	"NewStartGameDialogue"
#define	NEW_LOAD_GAME_MENU_NAME		"NewLoadGameMenuName"
#define NEW_SAVE_GAME_MENU_NAME		"NewSaveGameMenuName"

#define TITLE_COMMAND				"ClickTitleLabel"
#define LOAD_COMMAND				"OpenLoadGameDialog"
#define SAVE_COMMAND				"OpenSaveGameDialog"
#define	OPTIONS_COMMAND				"ShowNewOptionsDialogue"
#define RESUME_COMMAND				"ResumeGame"
#define NEWGAME_COMMAND				"OpenNewGameDialog"
#define	ACHIEVE_COMMAND				"OpenAchievementsDialog"
#define	BONUS_COMMAND				"OpenBonusMapsDialog"
#define	CHANGE_COMMAND				"OpenChangeGameDialog"
#define EXIT_COMMAND				"Quit"

#define CONTINUE_COMMAND			"ContinueLastSave"
#define IMMEDIATE_START_COMMAND		"StartNewGameImmediately"

#pragma once

abstract_class IGenericMenuPanel : public vgui::Panel, public CAutoGameSystem
{
	DECLARE_CLASS_SIMPLE(IGenericMenuPanel, vgui::Panel);

public:
	IGenericMenuPanel(vgui::Panel* parent, const char* name);
	IGenericMenuItem*		AddButton(const char*, const char*, vgui::Panel*, const char* iconPath);
	IGenericMenuItem*		AddCheckButton(const char*, const char*, vgui::Panel*, bool);
	IGenericMenuItem*		AddSlider(const char*, const char*, vgui::Panel*, float);
	virtual void			SetVisible(bool visible);
	virtual void			SetAllItemsInactive();
	virtual int				GetNextVisibleItem(int);
	virtual int				GetLastVisibleItem(int);
	virtual int				GetActiveItemIndex();
	virtual void			SetBackButton(IGenericMenuItem*);
	virtual void			RemoveThisButton(const char *text);
	virtual void			RemoveButtons();
	virtual void			OnKeyCodePressed(vgui::KeyCode code);
	virtual void			OnThink();
	virtual void			OnEnterGame() {};
	virtual void			OnExitGame() {};
	virtual void			OnHideMenu() {};
	virtual void			OnShowMenu() {};
	virtual void			PerformLayout();
	virtual void			SendCommandToGameUI(const char*);
	virtual bool			HasSaveFiles();
	virtual void			OnTick();

private:
	CUtlVector<IGenericMenuItem*>	m_Buttons;
	IGenericMenuItem*				m_backItem;
	short							m_iGameState;
	bool							m_bWasVisible;
};