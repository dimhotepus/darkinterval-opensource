#include "cbase.h"
#include "igenericmenucheckbutton.h"
#include <vgui_controls/CheckButton.h>
#include <vgui/Ivgui.h>
#include <vgui/ISurface.h>

#define TRANSPARENT Color(0, 0, 0, 0)

IGenericMenuCheckButton::IGenericMenuCheckButton(Panel *parent, const char *label, const char *command, Panel *target, bool checked):
	IGenericMenuItem(parent, label, command, target)
{
	check = new vgui::CheckButton(this, "CheckButton", "");
	check->SetSelected(checked);
	check->SetKeyBoardInputEnabled(false);
	check->SetMouseInputEnabled(false);
}

void IGenericMenuCheckButton::PerformLayout()
{
	BaseClass::PerformLayout();

	check->SetSize(GetTall(), GetTall());
	SetTextInset(GetTall() / 1.5, 0);
}

void IGenericMenuCheckButton::OnMousePressed(vgui::MouseCode code)
{
	BaseClass::OnMousePressed(code);

	check->SetSelected(!check->IsSelected());
}