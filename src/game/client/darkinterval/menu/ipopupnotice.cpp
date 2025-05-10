#include "cbase.h"
#include "ipopupnotice.h"
#include <vgui/Ivgui.h>
#include <vgui_controls/button.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Divider.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/Tooltip.h>
#include <vgui_controls/ComboBox.h>
#include <vgui_controls/CheckButton.h>
#include <vgui_controls/URLLabel.h>
#include <vgui_controls/ImagePanel.h>
#include <vgui_controls/Slider.h>
#include "vgui/ISurface.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

using namespace vgui;

ConVar di_showpopupnoticepanel("di_showpopupnoticepanel", "0", FCVAR_CLIENTDLL|FCVAR_HIDDEN, "Displays pop-up message panel with text passed into this convar");
ConVar di_showpopupmessage("di_showpopupmessage", "Text", FCVAR_CLIENTDLL|FCVAR_HIDDEN, "Displays pop-up message panel with text passed into this convar");
ConVar di_showpopuptitle("di_showpopuptitle", "Title", FCVAR_CLIENTDLL | FCVAR_HIDDEN, "Displays pop-up message panel with text passed into this convar");
ConVar di_showpopupbutton("di_showpopupbutton", "OK", FCVAR_CLIENTDLL | FCVAR_HIDDEN, "Displays pop-up message panel with text passed into this convar");

CON_COMMAND(DisplayPopupNotice, "Toggle pop-up notice")
{
	if (args.ArgC() == 4)
	{
		di_showpopuptitle.SetValue(args[1]);
		di_showpopupmessage.SetValue(args[2]);
		di_showpopupbutton.SetValue(args[3]);
	}
	else 
	{
		di_showpopuptitle.SetValue("Uh-oh");
		di_showpopupmessage.SetValue("You forgot to set text contents in your inputs!"); // legacy compatibility
		di_showpopupbutton.SetValue("D'oh!");
	}

	di_showpopupnoticepanel.SetValue("1");
};

class CPopupNoticePanel : public Frame
{
	DECLARE_CLASS_SIMPLE(CPopupNoticePanel, Frame);

	CPopupNoticePanel(VPANEL parent);
	~CPopupNoticePanel() {};

protected:
	virtual void		OnTick(void);
	virtual void		OnCommand(const char* pcCommand);

	void				DrawPopupItems(void);
private:
	Label *m_pText;
	Button *m_pConfirmButton;
	bool m_bLastVisibilityState;
};

CPopupNoticePanel::CPopupNoticePanel(VPANEL parent) : BaseClass(NULL, "DI_PopupNotice")
{
	SetTitle(di_showpopuptitle.GetString(), true);

	SetDeleteSelfOnClose(false);
	int horCenter = ScreenWidth() / 2;
	int verCenter = ScreenHeight() / 4 - 300;
	SetBounds(horCenter + 200, verCenter + 140, 400, 280);

	m_pText = new Label(this, "PopupMessage", di_showpopupmessage.GetString());
	m_pText->SetWide(GetWide() * 0.9);
	m_pText->SetTall(GetTall() * 0.75);
	m_pText->SetPos(GetWide() * 0.1, GetTall() * 0.1);

	m_pConfirmButton = new Button(this, "PopupConfirm", di_showpopupbutton.GetString(), this, "Confirm");
	m_pConfirmButton->SetWide(GetWide() * 0.25);
	m_pConfirmButton->SetTall(GetTall() * 0.085);
	m_pConfirmButton->SetPos((GetWide() / 2 - m_pConfirmButton->GetWide() / 2), GetTall() * 0.87);

	SetKeyBoardInputEnabled(true);
	SetMouseInputEnabled(true);
	SetProportional(false);
	SetMinimizeButtonVisible(false);
	SetMaximizeButtonVisible(false);
	SetCloseButtonVisible(false);
	SetSizeable(false);
	SetMoveable(false);
	SetVisible(false);
	Activate();
	ivgui()->AddTickSignal(GetVPanel(), 100);
}

void CPopupNoticePanel::DrawPopupItems()
{
	SetTitle(di_showpopuptitle.GetString(), true);
	m_pText->SetText(di_showpopupmessage.GetString());
	m_pConfirmButton->SetText(di_showpopupbutton.GetString());
}

void CPopupNoticePanel::OnCommand(const char *command)
{
	if (!Q_strcmp(command, "Confirm"))
	{
		ConVarRef radished("di_showpopupnoticepanel");
		radished.SetValue("0");
	}

	BaseClass::OnCommand(command);
}

void CPopupNoticePanel::OnTick()
{
	bool visibleState = di_showpopupnoticepanel.GetBool();
	if (visibleState)
	{
		if (!m_bLastVisibilityState)
		{
			SetPos((ScreenWidth() / 2) - (GetWide() / 2), (ScreenHeight() / 2) - (GetTall() / 2));
			SetVisible(visibleState);
			DrawPopupItems();
			m_bLastVisibilityState = true;
		}
	}
	else
	{
		SetVisible(false);
		m_bLastVisibilityState = false;
	}

	BaseClass::OnTick();
}

class CPopupNoticePanelInterface : public IPopupNoticePanel
{
private:
	CPopupNoticePanel *PopupNoticePanel;
public:
	CPopupNoticePanelInterface()
	{
		PopupNoticePanel = NULL;
	}
	void Create(VPANEL parent)
	{
		PopupNoticePanel = new CPopupNoticePanel(parent);
	}
	void Destroy()
	{
		if (PopupNoticePanel)
		{
			PopupNoticePanel->SetParent((Panel *)NULL);
			delete PopupNoticePanel;
		}
	}
};

static CPopupNoticePanelInterface g_PopupNoticePanel;
IPopupNoticePanel* popupnoticepanel = (IPopupNoticePanel*)&g_PopupNoticePanel;