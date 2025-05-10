//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//

#ifndef IGENERICCVARSLIDER_H
#define IGENERICCVARSLIDER_H
#ifdef _WIN32
#pragma once
#endif
#include "cbase.h"
#include <vgui_controls/Slider.h>
#include "vgui_controls/TextEntry.h"
#include "keyvalues.h"

class IGenericCvarSlider : public vgui::Slider
{
	DECLARE_CLASS_SIMPLE(IGenericCvarSlider, vgui::Slider);

public:

	IGenericCvarSlider(vgui::Panel *parent, const char *panelName);
	IGenericCvarSlider(vgui::Panel *parent, const char *panelName, char const *caption,
		float minValue, float maxValue, char const *cvarname, bool bAllowOutOfRange = false);
	~IGenericCvarSlider();

	void			SetupSlider(float minValue, float maxValue, const char *cvarname, bool bAllowOutOfRange);

	void			SetCVarName(char const *cvarname);
	void			SetMinMaxValues(float minValue, float maxValue, bool bSetTickdisplay = true);
	void			SetTickColor(Color color);

	virtual void	Paint();

	virtual void	ApplySettings(KeyValues *inResourceData);
	virtual void	GetSettings(KeyValues *outResourceData);

	void			ApplyChanges();
	float			GetSliderValue();
	void            SetSliderValue(float fValue);
	void            Reset();
	bool            HasBeenModified();

private:
	MESSAGE_FUNC(OnSliderMoved, "SliderMoved");
	MESSAGE_FUNC(OnApplyChanges, "ApplyChanges");

	bool        m_bAllowOutOfRange;
	bool        m_bModifiedOnce;
	float       m_fStartValue;
	int         m_iStartValue;
	int         m_iLastSliderValue;
	float       m_fCurrentValue;
	char		m_szCvarName[64];

	bool		m_bCreatedInCode;
	float		m_flMinValue;
	float		m_flMaxValue;
};

#endif // REPLACECVARSLIDER_H
