#include "cbase.h"
#include <vgui/Ivgui.h>
#include <vgui_controls/Panel.h>
#include "igenericmenupanel.h"
#include "ioptionsmaindialogue.h"
#include "inewgamedialogue.h"
#include "igamesystem.h"
#include "iweathersettings.h"

class INewMenuMainList : public IGenericMenuPanel
{
	DECLARE_CLASS_SIMPLE(INewMenuMainList, IGenericMenuPanel);

public:
	INewMenuMainList(vgui::Panel* parent);
	virtual void OnEnterGame();
	virtual void OnExitGame();
	virtual void OnCommand(const char*);

private:
	vgui::Panel		*continueGameButton;
	vgui::Panel		*resumeGameButton;
	vgui::Panel		*newGameButton;
	vgui::Panel		*saveGameButton;
	vgui::Panel		*loadGameButton;
	vgui::Panel		*optionsButton;
	vgui::Panel		*achieveButton;
	vgui::Panel		*bonusButton;
	vgui::Panel		*exitButton;

	vgui::DHANDLE<vgui::Frame> m_hNewGameDialog;
	vgui::DHANDLE<vgui::PropertyDialog> m_hOptionsDialog;
//	vgui::DHANDLE<vgui::PropertyDialog> m_hUnused;
//	int buttonCount;
};