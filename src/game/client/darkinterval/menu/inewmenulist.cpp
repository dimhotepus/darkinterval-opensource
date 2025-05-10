#include "cbase.h"
#include "inewmenulist.h"
#include <vgui/ilocalize.h>
#include <vgui/Ivgui.h>
#include <vgui/ISurface.h>
#include "saverestore.h" // for "continue" button
#include "savegame_version.h"

#define CLOSED_BETA_0

INewMenuMainList::INewMenuMainList(vgui::Panel *parent) : IGenericMenuPanel(parent, "NewMainMenuList")
{
//	if( !continueGameButton)
	if (HasSaveFiles())
	{
		continueGameButton = AddButton("#DI_NewMenu_ContinueGame", CONTINUE_COMMAND, this, "vgui/menu/continue");
	}
//	if (!resumeGameButton)
	resumeGameButton =	AddButton("#DI_NewMenu_ResumeGame", RESUME_COMMAND, this, "vgui/menu/resume");
//	if (!newGameButton)
	newGameButton	 =	AddButton("#DI_NewMenu_NewGame",	NEWGAME_COMMAND, this, "vgui/menu/newgame");
//	newGameButton	 = AddButton("#DI_NewMenu_NewGame",		BONUS_COMMAND, this, "vgui/menu/newgame");
//	if (!saveGameButton)
	saveGameButton	 =	AddButton("#DI_NewMenu_SaveGame",	SAVE_COMMAND, this, "vgui/menu/load");
//	if (!loadGameButton)
	loadGameButton	 =	AddButton("#DI_NewMenu_LoadGame",	LOAD_COMMAND, this, "vgui/menu/load");
//	if (!optionsButton)
	optionsButton	 =	AddButton("#DI_NewMenu_OptionsMenu",OPTIONS_COMMAND, this, "vgui/menu/options");
//	if (!achieveButton)
//	achieveButton	 =  AddButton("#DI_NewMenu_Achievements", ACHIEVE_COMMAND, this, "vgui/menu/achievements");
//	if (!bonusButton)
//	bonusButton		 =  AddButton("#GameUI_BonusMaps",		BONUS_COMMAND, this, "vgui/menu/bonus");
	if (!exitButton)
	exitButton		 =  AddButton("#DI_NewMenu_Quit",		EXIT_COMMAND, this, "vgui/menu/quit");

//	if (saveGameButton)
//		saveGameButton->SetVisible(false);
//	if (loadGameButton)
//		loadGameButton->SetVisible(HasSaveFiles());
//	if (continueGameButton)
//		continueGameButton->SetVisible(HasSaveFiles());
}

void INewMenuMainList::OnCommand(const char* command)
{
	if (!Q_strcmp(command, CONTINUE_COMMAND))
	{
		if (HasSaveFiles())
		{
			// do stuff to load latest save
			engine->ClientCmd_Unrestricted("load autosave\n");
			////
		}
	}
	if (!Q_strcmp(command, NEWGAME_COMMAND))
#ifdef CLOSED_BETA_1 // if in closed beta, pressing 'new game' immediately launches the first map.
	{
		if( !engine->IsInGame())
			engine->ClientCmd_Unrestricted("progress_enable\nexec chapter0\n");
	}		
#else
	{
		if (!m_hNewGameDialog.Get())
		{
			m_hNewGameDialog = new CNewGameDialogue(this);
			m_hNewGameDialog->Activate();
		}
	}
#endif
	if (!Q_strcmp(command, RESUME_COMMAND))	SendCommandToGameUI(RESUME_COMMAND);
	if (!Q_strcmp(command, SAVE_COMMAND))	SendCommandToGameUI(SAVE_COMMAND);
	if (!Q_strcmp(command, LOAD_COMMAND))	SendCommandToGameUI(LOAD_COMMAND);
	if (!Q_strcmp(command, EXIT_COMMAND))	SendCommandToGameUI(EXIT_COMMAND);
	if (!Q_strcmp(command, ACHIEVE_COMMAND)) SendCommandToGameUI(ACHIEVE_COMMAND);
	if (!Q_strcmp(command, BONUS_COMMAND)) SendCommandToGameUI(BONUS_COMMAND);
	if (!Q_strcmp(command, CHANGE_COMMAND)) SendCommandToGameUI(CHANGE_COMMAND);
	if (!Q_strcmp(command, OPTIONS_COMMAND))
	{
		if (!m_hOptionsDialog.Get())
		{
			m_hOptionsDialog = new CNewOptionsDialog(this->GetParent());
			int x;
			int y; 
			int ww; 
			int wt; 
			int wide = 0; 
			int tall = 0;
			vgui::surface()->GetWorkspaceBounds(x, y, ww, wt);
		//	m_hOptionsDialog->GetSize(wide, tall);
			m_hOptionsDialog->SetPos(x + ((ww - wide) / 2), y + ((wt - tall) / 2));
			m_hOptionsDialog->Activate();
		}
	}
}

void INewMenuMainList::OnEnterGame()
{
	RemoveButtons();
	////////////
	resumeGameButton = AddButton("#DI_NewMenu_ResumeGame", RESUME_COMMAND, this, "vgui/menu/resume");
	newGameButton = AddButton("#DI_NewMenu_NewGame", NEWGAME_COMMAND, this, "vgui/menu/newgame");
//	newGameButton = AddButton("#DI_NewMenu_NewGame", BONUS_COMMAND, this, "vgui/menu/new");
	saveGameButton = AddButton("#DI_NewMenu_SaveGame", SAVE_COMMAND, this, "vgui/menu/load");
	loadGameButton = AddButton("#DI_NewMenu_LoadGame", LOAD_COMMAND, this, "vgui/menu/load");
	optionsButton = AddButton("#DI_NewMenu_OptionsMenu", OPTIONS_COMMAND, this, "vgui/menu/options");
//	achieveButton = AddButton("#DI_NewMenu_Achievements", ACHIEVE_COMMAND, this, "vgui/menu/achievements");
//	bonusButton = AddButton("#GameUI_BonusMaps", BONUS_COMMAND, this, "vgui/menu/bonus");
	exitButton = AddButton("#DI_NewMenu_Quit", EXIT_COMMAND, this, "");

#ifdef CLOSED_BETA_1
	newGameButton->SetVisible(true);
	if (continueGameButton) continueGameButton->SetVisible(false);
#else
	newGameButton->SetVisible(true);
	if (continueGameButton) continueGameButton->SetVisible(false);
#endif

	saveGameButton->SetVisible(true);
	loadGameButton->SetVisible(true);
	optionsButton->SetVisible(true);
//	achieveButton->SetVisible(true);
//	bonusButton->SetVisible(true);
	exitButton->SetVisible(true);
	////////////

	// shut down open dialogues, or they can hang around awkwardly with game un-paused
	if (m_hNewGameDialog.Get())
		m_hNewGameDialog->Close();

	if( m_hOptionsDialog.Get())
		m_hOptionsDialog->Close();

	InvalidateLayout(true);
	PerformLayout();
}

void INewMenuMainList::OnExitGame()
{
	RemoveButtons();
	////////////
	if (HasSaveFiles())
	{
		continueGameButton = AddButton("#DI_NewMenu_ContinueGame", CONTINUE_COMMAND, this, "vgui/menu/back");
	}
	newGameButton = AddButton("#DI_NewMenu_NewGame", NEWGAME_COMMAND, this, "vgui/menu/newgame");
//	newGameButton = AddButton("#DI_NewMenu_NewGame", BONUS_COMMAND, this, "vgui/menu/new");
	loadGameButton = AddButton("#DI_NewMenu_LoadGame", LOAD_COMMAND, this, "vgui/menu/load");
	optionsButton = AddButton("#DI_NewMenu_OptionsMenu", OPTIONS_COMMAND, this, "vgui/menu/options");
//	achieveButton = AddButton("#DI_NewMenu_Achievements", ACHIEVE_COMMAND, this, "vgui/menu/achievements");
//	bonusButton = AddButton("#GameUI_BonusMaps", BONUS_COMMAND, this, "vgui/menu/bonus");
	exitButton = AddButton("#DI_NewMenu_Quit", EXIT_COMMAND, this, "");

	if( newGameButton)
		newGameButton->SetVisible(true);
	if (saveGameButton)
		saveGameButton->SetVisible(true);
	if (loadGameButton)
		loadGameButton->SetVisible(true);
	if (continueGameButton)
		continueGameButton->SetVisible(true);
	if (optionsButton)
		optionsButton->SetVisible(true);
//	if (achieveButton)
//		achieveButton->SetVisible(true);
//	if (bonusButton)
//		bonusButton->SetVisible(true);
	if (exitButton)
		exitButton->SetVisible(true);
	////////////

	if (m_hOptionsDialog.Get())
		m_hOptionsDialog->Close();

	InvalidateLayout(true);
	PerformLayout();
}