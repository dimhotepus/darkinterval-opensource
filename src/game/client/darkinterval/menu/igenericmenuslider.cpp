#include "cbase.h"
#include "igenericmenuslider.h"
#include <vgui/Ivgui.h>
#include <vgui/ISurface.h>
#include <vgui_controls/Slider.h>

IGenericMenuSlider::IGenericMenuSlider(Panel *parent, const char *label, const char *command, Panel *target, float value):
	IGenericMenuItem(parent, label, command, target)
{
	SetText("");
	slide = new vgui::Slider(this, "IGenericMenuSlider");
	slide->SetRange(0, 100);
	slide->SetValue(value);
	slide->SetTickCaptions("Hi", "Lo");
}

void IGenericMenuSlider::PerformLayout()
{
	slide->SetSize(GetWide(), GetTall());
}