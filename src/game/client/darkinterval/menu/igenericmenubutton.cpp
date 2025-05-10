#include "cbase.h"
#include "igenericmenubutton.h"
#include <vgui/Ivgui.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Label.h>

IGenericMenuButton::IGenericMenuButton(Panel *parent, const char *label, const char *command, Panel *target, const char *iconPath):
	IGenericMenuItem(parent, label, command, target)
{
#if 0
	icon = new vgui::ImagePanel(this, "IGenericMenuItemIcon");
	icon->SetImage(iconPath);
	icon->SetShouldScaleImage(true);
	icon->SetAlpha(128);
	icon->SetMouseInputEnabled(false);
	icon->SetKeyBoardInputEnabled(false);
#endif
	signalTarget = target;
	signalCommand = command;
}

void IGenericMenuButton::OnMouseReleased(vgui::MouseCode code)
{
	BaseClass::OnKeyCodePressed(code);
	
	vgui::surface()->PlaySound("ui/buttonclick.wav");

	PostMessage(signalTarget, new KeyValues("command", "command", signalCommand));
}

void IGenericMenuButton::OnKeyCodePressed(vgui::KeyCode code)
{
	BaseClass::OnKeyCodePressed(code);

	if (code == KEY_ENTER || code == JOYSTICK_FIRST || code == KEY_XBUTTON_B)
	{
		vgui::surface()->PlaySound("ui/buttonclick.wav");

		PostMessage(signalTarget, new KeyValues("command", "command", signalCommand));
	}
}

void IGenericMenuButton::PerformLayout()
{
	BaseClass::PerformLayout();

	SetTextInset(GetTall(), 0);
#if 0
	icon->SetPos(0, 0);
	icon->SetSize(GetWide(), GetTall());
#endif
}