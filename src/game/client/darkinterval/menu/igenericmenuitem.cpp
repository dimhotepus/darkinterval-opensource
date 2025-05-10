#include "cbase.h"
#include "igenericmenuitem.h"
#include <vgui/Ivgui.h>
#include <vgui/ISurface.h>

#define TRANSPARENT Color(0, 0, 0, 0)

IGenericMenuItem::IGenericMenuItem(Panel *parent, const char *label, const char *command, Panel *target):
	vgui::Label(parent, "IGenericMenuItem", label)
{
	isItemActive = false;
	hoverBorder = new vgui::Panel(this);
	hoverBorder->SetMouseInputEnabled(false);
	hoverBorder->SetKeyBoardInputEnabled(false);
}

void IGenericMenuItem::SetItemActive(bool state)
{
	isItemActive = state;
}

bool IGenericMenuItem::IsItemActive()
{
	return isItemActive;
}

void IGenericMenuItem::PaintBackground()
{
	if (IsItemActive())
	{
		hoverBorder->SetBgColor(Color(200,200,125,225)/*lightRed*/);
		SetBgColor(Color(86,86,86,225)/*lightRedTrans*/);
	}
	else
	{
		hoverBorder->SetBgColor(Color(86,86,86,225)/*TRANSPARENT*/);
		SetBgColor(Color(50,50,50,100)/*TRANSPARENT*/);
	}

	BaseClass::PaintBackground();
}

void IGenericMenuItem::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	SetFont(pScheme->GetFont("NewMenuListing"));
//	lightRed = pScheme->GetColor("LightRed", TRANSPARENT);
//	lightRedTrans = pScheme->GetColor("LightRedTrans", TRANSPARENT);

	BaseClass::ApplySchemeSettings(pScheme);
}

void IGenericMenuItem::PerformLayout()
{
	// Size to the contents first - fixes
	// any alignment issues if we just
	// switched resolution to a different
	// font size
	SizeToContents();

	// If we have a parent, scale to it
	if (GetParent() && GetParent()->GetChildCount() > 0)
	{
		SetWide(GetParent()->GetWide());
		SetTall(GetParent()->GetTall() / GetParent()->GetChildCount());
	}

	BaseClass::PerformLayout();

	hoverBorder->SetPos(0, 0);
	hoverBorder->SetSize(GetWide(), GetTall() / 16); // horizontal border on top
//	hoverBorder->SetSize(GetWide() / 32, GetTall()); // vertical border on left
}

void IGenericMenuItem::OnCursorEntered()
{
	vgui::surface()->PlaySound( "ui/buttonrollover.wav" );
	SetItemActive(true);
}

void IGenericMenuItem::OnCursorExited()
{
	SetItemActive(false);
}