#ifndef IOPTIONSPANEL_CONTROLS_H
#define IOPTIONSPANEL_CONTROLS_H
#ifdef _WIN32
#pragma once
#endif

#include <vgui/Ivgui.h>
#include <vgui_controls/Frame.h>
#include <vgui_controls/Label.h>
#include <vgui_controls/SectionedListPanel.h>
#include "ienginevgui.h"
#include <vgui/ISurface.h>
#include "ioptionspanel_controls_bindingslist.h"
#include <vgui_controls/PropertyPage.h>
#include "igenericcvarslider.h"
#include <language.h>
#include "GameUI/IGameUI.h"

class COptionsPanel_Controls : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(COptionsPanel_Controls, vgui::PropertyPage);

	COptionsPanel_Controls(vgui::Panel *parent);
	~COptionsPanel_Controls() 
	{
		DeleteSavedBindings();
	};

protected:
	void				SaveControlsSettings(void);
	void				LoadControlsSettings(void);
	virtual void		PerformLayout();
	virtual void		OnApplyChanges(void);
	virtual void		OnResetData(void);
//	virtual void		OnTextChanged(Panel *pPanel, const char *pszText);
	virtual void		OnKeyCodePressed(vgui::KeyCode code);
	virtual void		OnThink();
	// Trap row selection message
	MESSAGE_FUNC_INT(ItemSelected, "ItemSelected", itemID);

	void				DrawOptionsItems(void);
	
private:
	void				ApplyChangesToConVar(const char *pConVarName, int value);

	IGenericCvarSlider	*m_pMouseSensitivity;
	vgui::Label			*m_pSensitivityText;
	vgui::CheckButton	*m_pMouseInverseXButton;
	vgui::CheckButton	*m_pMouseInverseYButton;
	vgui::CheckButton	*m_pMouseRawButton;
	vgui::CheckButton	*m_pConsoleEnabledButton;
	vgui::CheckButton	*m_pWeaponFastSwitchButton;
	vgui::CheckButton	*m_pMouseSmoothingButton;
	COptionsPanel_Controls_BindingsList *m_pKeybordBingingsList;

	bool m_bLastVisibilityState;

//	MESSAGE_FUNC(OnDataChanged, "ControlModified");
	MESSAGE_FUNC(OnControlModified, "ControlModified");
	MESSAGE_FUNC(OnTextChanged, "TextChanged")
	{
		OnControlModified();
	}
	MESSAGE_FUNC(OnCheckButtonChecked, "CheckButtonChecked")
	{
		OnControlModified();
	}

	void Finish(ButtonCode_t code);

	//-----------------------------------------------------------------------------
	// Purpose: Used for saving engine keybindings in case user hits cancel button
	//-----------------------------------------------------------------------------
	struct KeyBinding
	{
		char *binding;
	};
	
	virtual void	OnCommand(const char *command);

	// Tell engine to bind/unbind a key
	void			BindKey(const char *key, const char *binding);
	void			UnbindKey(const char *key);
	virtual void	OnKeyCodeTyped(vgui::KeyCode code);

	// Save/restore/cleanup engine's current bindings ( for handling cancel button )
	void			SaveCurrentBindings(void);
	void			DeleteSavedBindings(void);

	// Get column 0 action descriptions for all keys
	void			ParseActionDescriptions(void);

	// Populate list of actions with current engine keybindings
	void			FillInCurrentBindings(void);
	// Remove all current bindings from list of bindings
	void			ClearBindItems(void);
	// Fill in bindings with mod-specified defaults
	void			FillInDefaultBindings(void);
	// Copy bindings out of list and set them in the engine
	void			ApplyAllBindings(void);

	// Bind a key to the item
	void			AddBinding(KeyValues *item, const char *keyname);

	// Remove all instances of a key from all bindings
	void			RemoveKeyFromBindItems(KeyValues *org_item, const char *key);

	// Find item by binding name
	KeyValues *GetItemForBinding(const char *binding);

	vgui::Button *m_pSetBindingButton;
	vgui::Button *m_pClearBindingButton;

	// List of saved bindings for the keys
	KeyBinding m_Bindings[BUTTON_CODE_LAST];

	// List of all the keys that need to have their binding removed
	CUtlVector<CUtlSymbol> m_KeysToUnbind;
};
#endif