#include "igenericmenuscroller.h"

class IGenericMenuScrollerLoadGame : public IGenericMenuScroller
{
	DECLARE_CLASS_SIMPLE(IGenericMenuScrollerLoadGame, IGenericMenuScroller);

	IGenericMenuScrollerLoadGame(vgui::VPANEL parent);

	protected:
		virtual void OnSave();
		virtual void OnCommand(const char *command);

	private:
		void AddSavedGames();
};