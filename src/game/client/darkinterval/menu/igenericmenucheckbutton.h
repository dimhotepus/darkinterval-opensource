#include <vgui/Ivgui.h>
#include "igenericmenuitem.h"

class IGenericMenuCheckButton : public IGenericMenuItem
{
	DECLARE_CLASS_SIMPLE(IGenericMenuCheckButton, IGenericMenuItem);

	public:
		IGenericMenuCheckButton(Panel *parent, const char *label, const char *command, Panel *target, bool isChecked);
		
		virtual void PerformLayout();
		virtual void OnMousePressed(vgui::MouseCode);

	private:
		vgui::CheckButton	*check;
};