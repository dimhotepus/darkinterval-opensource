#include "cbase.h"
#include "igenericmenuscrollerloadgame.h"
#include "filesystem.h"
#include "fmtstr.h"

IGenericMenuScrollerLoadGame::IGenericMenuScrollerLoadGame(vgui::VPANEL parent) : BaseClass(parent, "LOAD GAME")
{
	AddSavedGames();
}

void IGenericMenuScrollerLoadGame::OnSave()
{
	AddSavedGames();
}

void IGenericMenuScrollerLoadGame::AddSavedGames()
{
	ClearItems();

	const char *SAVE_PATH = "./SAVE/";
	FileFindHandle_t fh;
	char path[256];
	Q_snprintf(path, sizeof(path), "%s*.sav", SAVE_PATH);

	char const *fn = g_pFullFileSystem->FindFirstEx(path, "MOD", &fh);
	if (!fn) return;

	char save_name[64];
	do
	{
		Q_StripExtension(fn, save_name, sizeof(save_name));

		AddItem(new vgui::Button(NULL, "", save_name, this, save_name));

		fn = g_pFullFileSystem->FindNext(fh);
	}
	while(fn);

	g_pFullFileSystem->FindClose(fh);
}

void IGenericMenuScrollerLoadGame::OnCommand(const char *command)
{
	BaseClass::OnCommand(command);

	CFmtStr console_command;
	console_command.sprintf("progress_enable \n load %s", command);

	engine->ClientCmd(console_command.Access());
}