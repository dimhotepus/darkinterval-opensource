#include "cbase.h"
#include <vgui/Ivgui.h>
#include <vgui_controls/Panel.h>
#include <vgui_controls/Label.h>
#include "igenericmenupanel.h"
#include "ienginevgui.h"
#include "filesystem.h"
#include "igenericmenuutil.h"

#define MENU_BUTTONS_NUM 10

IGenericMenuPanel::IGenericMenuPanel(Panel *parent, const char* name) : vgui::Panel(parent, name)
{
	SetKeyBoardInputEnabled(true);
	m_iGameState = -1;
	m_backItem = NULL;

	m_bWasVisible = false;

	vgui::ivgui()->AddTickSignal(GetVPanel(), 100);
}

IGenericMenuItem* IGenericMenuPanel::AddButton(const char *text, const char *command, vgui::Panel *signalTarget, const char *iconPath)
{
	IGenericMenuButton *button = new IGenericMenuButton(this, text, command, signalTarget, iconPath);
	m_Buttons.AddToTail(button);
	PerformLayout();
	return button;
}

IGenericMenuItem* IGenericMenuPanel::AddCheckButton(const char *text, const char *command, vgui::Panel *signalTarget, bool checked)
{
	IGenericMenuCheckButton *checkButton = new IGenericMenuCheckButton(this, text, command, signalTarget, checked);
	m_Buttons.AddToTail(checkButton);
	PerformLayout();
	return checkButton;
}

IGenericMenuItem* IGenericMenuPanel::AddSlider(const char *text, const char *command, vgui::Panel *signalTarget, float value)
{
	IGenericMenuSlider *slider = new IGenericMenuSlider(this, text, command, signalTarget, value);
	m_Buttons.AddToTail(slider);
	PerformLayout();
	return slider;
}

void IGenericMenuPanel::OnTick()
{
	BaseClass::OnTick();

	if (!IGenericMenuUtil::IsGameUIVisible() && m_bWasVisible)
	{
		OnHideMenu();
		m_bWasVisible = false;
	}
}

void IGenericMenuPanel::RemoveThisButton(const char *text)
{
	for (int i = 0; i < m_Buttons.Count(); i++)
	{
		if (m_Buttons.Element(i) && !Q_strcmp(m_Buttons.Element(i)->GetName(), text))
		{
			m_Buttons.Remove(i);
		}
	}
	PerformLayout();
}

void IGenericMenuPanel::RemoveButtons()
{
	for (int i = 0; i < m_Buttons.Count(); i++)
	{
		m_Buttons.Element(i)->DeletePanel();
	}

	m_Buttons.RemoveAll();
	PerformLayout();
}

bool IGenericMenuPanel::HasSaveFiles()
{
	const char *SAVE_PATH = "./SAVE/";
	FileFindHandle_t fh;
	char path[256];
	Q_snprintf(path, sizeof(path), "%s*.sav", SAVE_PATH);

	char const *fn = g_pFullFileSystem->FindFirstEx(path, "MOD", &fh);
	g_pFullFileSystem->FindClose(fh);

	return fn;
}

void IGenericMenuPanel::OnKeyCodePressed(vgui::KeyCode code)
{
	int activeItem = GetActiveItemIndex();
	SetAllItemsInactive();

	int index = 0;

	switch (code)
	{
	case KEY_DOWN:
	case KEY_XBUTTON_DOWN:
	case KEY_XSTICK1_DOWN:
		index = GetNextVisibleItem(activeItem);
		m_Buttons.Element(index)->SetItemActive(true);
		break;
	case KEY_UP:
	case KEY_XBUTTON_UP:
	case KEY_XSTICK1_UP:
		index = GetLastVisibleItem(activeItem);
		m_Buttons.Element(index)->SetItemActive(true);
		break;
	case KEY_XBUTTON_B:
		if (m_backItem != NULL)
		{
			m_backItem->SetItemActive(true);
		}
		break;
	default:
		if (activeItem > -1) m_Buttons.Element(activeItem)->OnKeyCodePressed(code);
	}
}

void IGenericMenuPanel::SetBackButton(IGenericMenuItem* menuItem)
{
	m_backItem = menuItem;
}

int IGenericMenuPanel::GetLastVisibleItem(int start)
{
	for (int i = start - 1; i > -1; i--)
	{
		if (m_Buttons.Element(i)->IsVisible()) return i;
	}

	return GetLastVisibleItem(m_Buttons.Count());
}

int IGenericMenuPanel::GetNextVisibleItem(int start)
{
	for (int i = start + 1; i < m_Buttons.Count(); i++)
	{
		if (m_Buttons.Element(i)->IsVisible()) return i;
	}

	return GetNextVisibleItem(-1);
}

void IGenericMenuPanel::SetAllItemsInactive()
{
	for (int i = 0; i < m_Buttons.Count(); i++)
	{
		m_Buttons.Element(i)->SetItemActive(false);
	}
}

int IGenericMenuPanel::GetActiveItemIndex()
{
	for (int i = 0; i < m_Buttons.Count(); i++)
	{
		IGenericMenuItem *child = m_Buttons.Element(i);
		if (child->IsItemActive() && child->IsVisible()) return i;
	}

	return -1;
}

void IGenericMenuPanel::OnThink()
{
	short gameState = engine->IsLevelMainMenuBackground() + engine->IsInGame();

	if (gameState != m_iGameState || m_iGameState < 0)
	{
		if (gameState == 1)
		{
			OnEnterGame();
		}
		else
		{
			OnExitGame();
		}
	}

	if (!m_bWasVisible)
	{
		OnShowMenu();
		m_bWasVisible = true;
	}

	m_iGameState = gameState;
}

void IGenericMenuPanel::SendCommandToGameUI(const char *command)
{
	IGenericMenuUtil::CloseGameFrames(GetVPanel());

	vgui::ivgui()->PostMessage(IGenericMenuUtil::GetBaseGameUIPanel(), new KeyValues("command", "command", command), GetVPanel());
}

void IGenericMenuPanel::SetVisible(bool visible)
{
	BaseClass::SetVisible(visible);

	int index = GetNextVisibleItem(-1);
	m_Buttons.Element(index)->SetItemActive(true);
}

#define MAX_BUTTON_COUNT 8

void IGenericMenuPanel::PerformLayout()
{
	if (GetTall() <= 0 || GetChildCount() <= 0) return;

	int buttonCount = 0;

	for (int i = 0; i < m_Buttons.Count(); i++)
	{
		if (!m_Buttons.Element(i)) continue;
		vgui::Panel *child = m_Buttons.Element(i);
		if (!child) continue;
		if (!child->IsVisible()) continue;
		buttonCount++;
	}
	
	if (buttonCount == 0) return;

	int buttonHeight = (ScreenHeight() * 0.5) / MAX_BUTTON_COUNT; // GetTall() / buttonCount;
	int buttonPadding = 0;
	for (int i = 0; i < buttonCount; i++)
	{
		vgui::Panel *child = m_Buttons.Element(i);
		child->SetPos(0, i * buttonHeight + i * buttonPadding);
		child->SetSize(GetWide(), buttonHeight);
	}

	SetSize(GetWide(), buttonCount * buttonHeight + buttonPadding * buttonCount); // Make sure the total height of this list
																				  // is the total height of its elements.

	int posX, posY, parentSizeX, parentSizeY;
	GetPos(posX, posY);
	GetParent()->GetSize(parentSizeX, parentSizeY);
	SetPos(posX, parentSizeY - GetTall()); // move us on the parent to fit our entire height, align our bottom with parent's bottom.
}