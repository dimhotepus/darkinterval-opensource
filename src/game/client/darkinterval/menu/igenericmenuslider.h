#include <vgui/Ivgui.h>
#include "igenericmenuitem.h"

class IGenericMenuSlider : public IGenericMenuItem
{
	DECLARE_CLASS_SIMPLE(IGenericMenuSlider, IGenericMenuItem);

	public:
		IGenericMenuSlider(Panel *parent, const char *label, const char *command, Panel *target, float value);

		virtual void PerformLayout();

	private:
		vgui::Slider	*slide;
};