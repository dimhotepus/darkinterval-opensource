#include "cbase.h"
#include "ioptionspanel_controls_bindingslist.h"
#include <vgui/IInput.h>
#include <vgui/IScheme.h>
#include <vgui/ISurface.h>
#include <vgui/Ivgui.h>
#include <vgui/Cursor.h>
#include <keyvalues.h>


// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: panel used for inline editing of key bindings
//-----------------------------------------------------------------------------
class CInlineEditPanel : public vgui::Panel
{
public:
	CInlineEditPanel() : vgui::Panel(NULL, "InlineEditPanel")
	{
	}

	virtual void Paint()
	{
		int x = 0, y = 0, wide, tall;
		GetSize(wide, tall);

		// Draw a white rectangle around that cell
		vgui::surface()->DrawSetColor(255, 165, 0, 255);
		vgui::surface()->DrawFilledRect(x, y, x + wide, y + tall);
	}

	virtual void OnKeyCodeTyped(KeyCode code)
	{
		// forward up
		if (GetParent())
		{
			GetParent()->OnKeyCodeTyped(code);
		}
	}

	virtual void ApplySchemeSettings(IScheme *pScheme)
	{
		Panel::ApplySchemeSettings(pScheme);
		SetBorder(pScheme->GetBorder("DepressedButtonBorder"));
	}

	void OnMousePressed(vgui::MouseCode code)
	{
		// forward up mouse pressed messages to be handled by the key options
		if (GetParent())
		{
			GetParent()->OnMousePressed(code);
		}
	}
};

//-----------------------------------------------------------------------------
// Purpose: Construction
//-----------------------------------------------------------------------------
COptionsPanel_Controls_BindingsList::COptionsPanel_Controls_BindingsList(vgui::Panel *parent, const char *listName) : vgui::SectionedListPanel(parent, listName)
{
	m_bCaptureMode = false;
	m_nClickRow = 0;
	m_pInlineEditPanel = new CInlineEditPanel();
	m_hFont = INVALID_FONT;
}

//-----------------------------------------------------------------------------
// Purpose: Destructor
//-----------------------------------------------------------------------------
COptionsPanel_Controls_BindingsList::~COptionsPanel_Controls_BindingsList()
{
	m_pInlineEditPanel->MarkForDeletion();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void COptionsPanel_Controls_BindingsList::ApplySchemeSettings(IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	m_hFont = pScheme->GetFont("Default", IsProportional());
}

//-----------------------------------------------------------------------------
// Purpose: Start capture prompt display
//-----------------------------------------------------------------------------
void COptionsPanel_Controls_BindingsList::StartCaptureMode(HCursor hCursor)
{
	m_bCaptureMode = true;
	EnterEditMode(m_nClickRow, 1, m_pInlineEditPanel);
	input()->SetMouseFocus(m_pInlineEditPanel->GetVPanel());
	input()->SetMouseCapture(m_pInlineEditPanel->GetVPanel());

	engine->StartKeyTrapMode();

	if (hCursor)
	{
		m_pInlineEditPanel->SetCursor(hCursor);

		// save off the cursor position so we can restore it
		vgui::input()->GetCursorPos(m_iMouseX, m_iMouseY);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Finish capture prompt display
//-----------------------------------------------------------------------------
void COptionsPanel_Controls_BindingsList::EndCaptureMode(HCursor hCursor)
{
	m_bCaptureMode = false;
	input()->SetMouseCapture(NULL);
	LeaveEditMode();
	RequestFocus();
	input()->SetMouseFocus(GetVPanel());
	if (hCursor)
	{
		m_pInlineEditPanel->SetCursor(hCursor);
		surface()->SetCursor(hCursor);
		if (hCursor != dc_none)
		{
			vgui::input()->SetCursorPos(m_iMouseX, m_iMouseY);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Set active row column
//-----------------------------------------------------------------------------
void COptionsPanel_Controls_BindingsList::SetItemOfInterest(int itemID)
{
	m_nClickRow = itemID;
}

//-----------------------------------------------------------------------------
// Purpose: Retrieve row, column of interest
//-----------------------------------------------------------------------------
int COptionsPanel_Controls_BindingsList::GetItemOfInterest()
{
	return m_nClickRow;
}

//-----------------------------------------------------------------------------
// Purpose: returns true if we're currently waiting to capture a key
//-----------------------------------------------------------------------------
bool COptionsPanel_Controls_BindingsList::IsCapturing(void)
{
	return m_bCaptureMode;
}

//-----------------------------------------------------------------------------
// Purpose: Forwards mouse pressed message up to keyboard page when in capture
//-----------------------------------------------------------------------------
void COptionsPanel_Controls_BindingsList::OnMousePressed(vgui::MouseCode code)
{
	if (IsCapturing())
	{
		// forward up mouse pressed messages to be handled by the key options
		if (GetParent())
		{
			GetParent()->OnMousePressed(code);
		}
	}
	else
	{
		BaseClass::OnMousePressed(code);
	}
}


//-----------------------------------------------------------------------------
// Purpose: input handler
//-----------------------------------------------------------------------------
void COptionsPanel_Controls_BindingsList::OnMouseDoublePressed(vgui::MouseCode code)
{
	if (IsItemIDValid(GetSelectedItem()))
	{
		// enter capture mode
		OnKeyCodePressed(KEY_ENTER);
	}
	else
	{
		BaseClass::OnMouseDoublePressed(code);
	}
}
