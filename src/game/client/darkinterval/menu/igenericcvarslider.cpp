#include "cbase.h"
#include "igenericcvarslider.h"
#include <stdio.h>
//#include "EngineInterface.h"
#include "tier1/keyvalues.h"
#include "tier1/convar.h"
#include <vgui/Ivgui.h>
#include <vgui_controls/propertypage.h>

#define CVARSLIDER_SCALE_FACTOR 100.0f

// memdbgon must be the last include file in a .cpp file!!!
#include <tier0/memdbgon.h>

using namespace vgui;

DECLARE_BUILD_FACTORY(IGenericCvarSlider);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
IGenericCvarSlider::IGenericCvarSlider(Panel *parent, const char *name) : Slider(parent, name)
{
	SetupSlider(0, 1, "", false);
	m_bCreatedInCode = false;

	AddActionSignalTarget(this);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
IGenericCvarSlider::IGenericCvarSlider(Panel *parent, const char *panelName, char const *caption,
	float minValue, float maxValue, char const *cvarname, bool bAllowOutOfRange)
	: Slider(parent, panelName)
{
	AddActionSignalTarget(this);

	SetupSlider(minValue, maxValue, cvarname, bAllowOutOfRange);

	// For backwards compatability. Ignore .res file settings for forced setup sliders.
	m_bCreatedInCode = true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void IGenericCvarSlider::SetupSlider(float minValue, float maxValue, const char *cvarname, bool bAllowOutOfRange)
{
	m_flMinValue = minValue;
	m_flMaxValue = maxValue;

	// scale by CVARSLIDER_SCALE_FACTOR
	SetRange((int)(CVARSLIDER_SCALE_FACTOR * minValue), (int)(CVARSLIDER_SCALE_FACTOR * maxValue));

	char szMin[32];
	char szMax[32];

	Q_snprintf(szMin, sizeof(szMin), "%.2f", minValue);
	Q_snprintf(szMax, sizeof(szMax), "%.2f", maxValue);

	SetTickCaptions(szMin, szMax);

	Q_strncpy(m_szCvarName, cvarname, sizeof(m_szCvarName));

	m_bModifiedOnce = false;
	m_bAllowOutOfRange = bAllowOutOfRange;

	// Set slider to current value
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
IGenericCvarSlider::~IGenericCvarSlider()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void IGenericCvarSlider::ApplySettings(KeyValues *inResourceData)
{
	BaseClass::ApplySettings(inResourceData);

	if (!m_bCreatedInCode)
	{
		float minValue = inResourceData->GetFloat("minvalue", 0);
		float maxValue = inResourceData->GetFloat("maxvalue", 1);
		const char *cvarname = inResourceData->GetString("cvar_name", "");
		bool bAllowOutOfRange = inResourceData->GetInt("allowoutofrange", 0) != 0;
		SetupSlider(minValue, maxValue, cvarname, bAllowOutOfRange);

		if (GetParent())
		{
			// HACK: If our parent is a property page, we want the dialog containing it
			if (dynamic_cast<vgui::PropertyPage*>(GetParent()) && GetParent()->GetParent())
			{
				GetParent()->GetParent()->AddActionSignalTarget(this);
			}
			else
			{
				GetParent()->AddActionSignalTarget(this);
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Get control settings for editing
//-----------------------------------------------------------------------------
void IGenericCvarSlider::GetSettings(KeyValues *outResourceData)
{
	BaseClass::GetSettings(outResourceData);

	if (!m_bCreatedInCode)
	{
		outResourceData->SetFloat("minvalue", m_flMinValue);
		outResourceData->SetFloat("maxvalue", m_flMaxValue);
		outResourceData->SetString("cvar_name", m_szCvarName);
		outResourceData->SetInt("allowoutofrange", m_bAllowOutOfRange);
	}
}

void IGenericCvarSlider::SetCVarName(char const *cvarname)
{
	Q_strncpy(m_szCvarName, cvarname, sizeof(m_szCvarName));

	m_bModifiedOnce = false;

	// Set slider to current value
	Reset();
}

void IGenericCvarSlider::SetMinMaxValues(float minValue, float maxValue, bool bSetTickDisplay)
{
	SetRange((int)(CVARSLIDER_SCALE_FACTOR * minValue), (int)(CVARSLIDER_SCALE_FACTOR * maxValue));

	if (bSetTickDisplay)
	{
		char szMin[32];
		char szMax[32];

		Q_snprintf(szMin, sizeof(szMin), "%.2f", minValue);
		Q_snprintf(szMax, sizeof(szMax), "%.2f", maxValue);


		SetTickCaptions(szMin, szMax);
	}

	// Set slider to current value
	Reset();
}

void IGenericCvarSlider::SetTickColor(Color color)
{
	m_TickColor = color;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void IGenericCvarSlider::Paint()
{
	// Get engine's current value
	//	float curvalue = engine->pfnGetCvarFloat( m_szCvarName );
	ConVarRef var(m_szCvarName);
	if (!var.IsValid())
		return;
	float curvalue = var.GetFloat();

	// did it get changed from under us?
	if (curvalue != m_fStartValue)
	{
		int val = (int)(CVARSLIDER_SCALE_FACTOR * curvalue);
		m_fStartValue = curvalue;
		m_fCurrentValue = curvalue;

		SetValue(val);
		m_iStartValue = GetValue();
	}

	BaseClass::Paint();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void IGenericCvarSlider::ApplyChanges()
{
	if (m_bModifiedOnce)
	{
		m_iStartValue = GetValue();
		if (m_bAllowOutOfRange)
		{
			m_fStartValue = m_fCurrentValue;
		}
		else
		{
			m_fStartValue = (float)m_iStartValue / CVARSLIDER_SCALE_FACTOR;
		}

		//engine->Cvar_SetValue( m_szCvarName, m_fStartValue );
		ConVarRef var(m_szCvarName);
		var.SetValue((float)m_fStartValue);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
float IGenericCvarSlider::GetSliderValue()
{
	if (m_bAllowOutOfRange)
	{
		return m_fCurrentValue;
	}
	else
	{
		return ((float)GetValue()) / CVARSLIDER_SCALE_FACTOR;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void IGenericCvarSlider::SetSliderValue(float fValue)
{
	int nVal = (int)(CVARSLIDER_SCALE_FACTOR * fValue);
	SetValue(nVal, false);

	// remember this slider value
	m_iLastSliderValue = GetValue();

	if (m_fCurrentValue != fValue)
	{
		m_fCurrentValue = fValue;
		m_bModifiedOnce = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void IGenericCvarSlider::Reset()
{
	// Set slider to current value
	//	m_fStartValue = engine->pfnGetCvarFloat( m_szCvarName );
	ConVarRef var(m_szCvarName);
	if (!var.IsValid())
	{
		m_fCurrentValue = m_fStartValue = 0.0f;
		SetValue(0);
		m_iStartValue = GetValue();
		m_iLastSliderValue = m_iStartValue;
		return;
	}
	float m_fStartValue = var.GetFloat();
	m_fCurrentValue = m_fStartValue;

	int value = (int)(CVARSLIDER_SCALE_FACTOR * m_fStartValue);
	SetValue(value);

	m_iStartValue = GetValue();
	m_iLastSliderValue = m_iStartValue;

}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool IGenericCvarSlider::HasBeenModified()
{
	if (GetValue() != m_iStartValue)
	{
		m_bModifiedOnce = true;
	}

	return m_bModifiedOnce;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : position - 
//-----------------------------------------------------------------------------
void IGenericCvarSlider::OnSliderMoved()
{
	if (HasBeenModified())
	{
		if (m_iLastSliderValue != GetValue())
		{
			m_iLastSliderValue = GetValue();
			m_fCurrentValue = ((float)m_iLastSliderValue) / CVARSLIDER_SCALE_FACTOR;
		}

		// tell parent that we've been modified
		PostActionSignal(new KeyValues("ControlModified"));
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void IGenericCvarSlider::OnApplyChanges(void)
{
	if (!m_bCreatedInCode)
	{
		ApplyChanges();
	}
}