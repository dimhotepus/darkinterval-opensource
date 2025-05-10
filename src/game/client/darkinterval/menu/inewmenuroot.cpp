#include "cbase.h"
#include "inewmenuroot.h"
#include <vgui/Ivgui.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/ImagePanel.h>
#include "vgui/ISurface.h"
#include <vgui/ISystem.h>
#include "inewmenubase.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class NewMainMenuInterface : public INewMenuRoot
{
private:
	INewMenuBase *newMainMenuBase;
public:
	NewMainMenuInterface()
	{
		newMainMenuBase = NULL;
	}
	void Create(vgui::VPANEL parent)
	{
	//	NewMenuVideoPanel_Create(0, 0, ScreenWidth(), ScreenHeight(), "media/bg_01.bik", true);
		newMainMenuBase = new INewMenuBase(parent);
	}
	void Destroy()
	{
		if (newMainMenuBase)
		{
			newMainMenuBase->SetParent((vgui::Panel*)NULL);
			delete newMainMenuBase;
		}
	}
};

static NewMainMenuInterface g_NewMainMenuBase;
INewMenuRoot* newmainmenubase = &g_NewMainMenuBase;