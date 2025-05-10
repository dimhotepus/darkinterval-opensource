#include <vgui/Ivgui.h>
#include <vgui_controls/Label.h>
#include "igenericmenuitem.h"
#include <vgui_controls/ImagePanel.h>

class IGenericMenuButton : public IGenericMenuItem
{
	DECLARE_CLASS_SIMPLE(IGenericMenuButton, IGenericMenuItem);

	public:
		IGenericMenuButton(Panel *parent, const char *label, const char *command, Panel *target, const char *iconPath);

		virtual void OnMouseReleased(vgui::MouseCode);
		virtual void OnKeyCodePressed(vgui::KeyCode);
		virtual void PerformLayout();

	private:
#if 0
		vgui::ImagePanel	*icon;
#endif
		vgui::Panel			*signalTarget;
		const char			*signalCommand;
};