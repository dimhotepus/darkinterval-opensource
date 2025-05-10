#ifndef IWEATHERSETTINGS_H
#define IWEATHERSETTINGS_H

#ifdef _WIN32
#pragma once
#endif

#include "vgui_controls/button.h"
#include "igenericcvarslider.h"
#include "vgui_controls/CheckButton.h"
#include "vgui_controls/KeyRepeat.h"
#include "vgui_controls/Label.h"
#include "vgui_controls/PropertyDialog.h"
#include "vgui_controls/PropertySheet.h"
#include "vgui_controls/PropertyPage.h"
#include "vgui_controls/QueryBox.h"
#include "vgui_controls/TextEntry.h"

#include "vgui/IInput.h"
#include "vgui/ilocalize.h"
#include "vgui/ISurface.h"
#include "vgui/ISystem.h"
#include "vgui/Ivgui.h"

#include "matsys_controls\colorpickerpanel.h"

#include "keyvalues.h"

//-----------------------------------------------------------------------------
// Dev tool for runtime fog adjustment
//-----------------------------------------------------------------------------
class CWeatherSettings_Fog : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CWeatherSettings_Fog, vgui::PropertyPage);
public:
	CWeatherSettings_Fog(vgui::Panel *parent);
	~CWeatherSettings_Fog() {};
	//	MESSAGE_FUNC(OnDataChanged, "ControlModified");
protected:
	void				DrawSettingsItems(void);
	void				ApplyChangesToConVar(const char *pConVarName, int value);
	virtual void		PerformLayout()
	{
		BaseClass::PerformLayout();
	}
	virtual void		OnCommand(const char *pcCommand);
	//	virtual void		OnTextChanged(Panel *pPanel, const char *pszText)
	//	{
	//		OnDataChanged();
	//	}

private:
	vgui::CheckButton *m_pOverrideLevelFog;
	vgui::CheckButton *m_pEnableMainFog;
	vgui::CheckButton *m_pEnableSkyboxFog;
	// Fog applied in the main area of the map
	IGenericCvarSlider *m_pFogMainStartSlider;
	IGenericCvarSlider *m_pFogMainEndSlider;
	IGenericCvarSlider *m_pFogMainDensitySlider;
	// Fog applied in the skybox area of the map
	IGenericCvarSlider *m_pFogSkyboxStartSlider;
	IGenericCvarSlider *m_pFogSkyboxEndSlider;
	IGenericCvarSlider *m_pFogSkyboxDensitySlider;
	// Main area fog color
	vgui::Slider *m_pMainFogRedSlider;
	vgui::Slider *m_pMainFogGreenSlider;
	vgui::Slider *m_pMainFogBlueSlider;
	CColorPickerButton *m_pMainFogColorPicker;
	// Sky fog color
	vgui::Slider *m_pSkyFogRedSlider;
	vgui::Slider *m_pSkyFogGreenSlider;
	vgui::Slider *m_pSkyFogBlueSlider;
	CColorPickerButton *m_pSkyFogColorPicker;
	// Text readouts for every element.
	// Can also be set to a value and will update the coresponding element.
	vgui::TextEntry *m_pReadout_MainFogStart;
	vgui::TextEntry *m_pReadout_MainFogEnd;
	vgui::TextEntry *m_pReadout_MainFogDensity;

	vgui::TextEntry *m_pReadout_SkyFogStart;
	vgui::TextEntry *m_pReadout_SkyFogEnd;
	vgui::TextEntry *m_pReadout_SkyFogDensity;

	vgui::TextEntry *m_pReadout_MainFogColor;
	vgui::TextEntry *m_pReadout_SkyFogColor;

	bool m_bLastVisibilityState;

	MESSAGE_FUNC(OnDataChanged, "ControlModified");
	MESSAGE_FUNC(OnSliderMoved, "SliderMoved");
	//	MESSAGE_FUNC(OnTextChanged, "TextNewLine");
	MESSAGE_FUNC_PARAMS(OnTextNewLine, "TextNewLine", data);
	MESSAGE_FUNC_PARAMS(OnTextKillFocus, "TextKillFocus", data);
};

//-----------------------------------------------------------------------------
// Dev tool for runtime HDR adjustment
//-----------------------------------------------------------------------------
class CWeatherSettings_HDR : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CWeatherSettings_HDR, vgui::PropertyPage);
public:
	CWeatherSettings_HDR(vgui::Panel *parent);
	~CWeatherSettings_HDR() {};
	//	MESSAGE_FUNC(OnDataChanged, "ControlModified");
protected:
	void				DrawSettingsItems(void);
	virtual void		PerformLayout()
	{
		BaseClass::PerformLayout();
	}
	virtual void		OnCommand(const char *pcCommand);

private:
	vgui::CheckButton *m_pOverrideHDR;
	vgui::CheckButton *m_pEnableMainFog;
	vgui::CheckButton *m_pEnableSkyboxFog;
	// Your three main settings for HDR
	IGenericCvarSlider *m_pHDRMinSlider;
	IGenericCvarSlider *m_pHDRMaxSlider;
	IGenericCvarSlider *m_pBloomSlider;
	// Text readouts for every element.
	// Can also be set to a value and will update the coresponding element.
	vgui::TextEntry *m_pReadout_HDRMin;
	vgui::TextEntry *m_pReadout_HDRMax;
	vgui::TextEntry *m_pReadout_Bloom;

	bool m_bLastVisibilityState;

	MESSAGE_FUNC(OnDataChanged, "ControlModified");
	MESSAGE_FUNC(OnSliderMoved, "SliderMoved");
	MESSAGE_FUNC(OnTextChanged, "TextNewLine");
};

//-----------------------------------------------------------------------------
// Dev tool for runtime DOF adjustment
//-----------------------------------------------------------------------------
class CWeatherSettings_DoF : public vgui::PropertyPage
{
	DECLARE_CLASS_SIMPLE(CWeatherSettings_DoF, vgui::PropertyPage);
public:
	CWeatherSettings_DoF(vgui::Panel *parent);
	~CWeatherSettings_DoF() {};
	//	MESSAGE_FUNC(OnDataChanged, "ControlModified");
protected:
	void				DrawSettingsItems(void);
	virtual void		PerformLayout()
	{
		BaseClass::PerformLayout();
	}
	virtual void		OnCommand(const char *pcCommand);

private:
	vgui::CheckButton *m_pEnableDoF;
	// Your main settings for DoF
	IGenericCvarSlider *m_pDoFFarFocusDepthSlider;
	IGenericCvarSlider *m_pDoFFarBlurDepthSlider;
	IGenericCvarSlider *m_pDoFFarBlurRadiusSlider;

	IGenericCvarSlider *m_pDoFNearFocusDepthSlider;
	IGenericCvarSlider *m_pDoFNearBlurDepthSlider;
	IGenericCvarSlider *m_pDoFNearBlurRadiusSlider;
	// Text readouts for every element.
	// Can also be set to a value and will update the coresponding element.
	vgui::TextEntry *m_pReadout_DoFFarFocusDepth;
	vgui::TextEntry *m_pReadout_DoFFarBlurDepth;
	vgui::TextEntry *m_pReadout_DoFFarBlurRadius;
	vgui::TextEntry *m_pReadout_DoFNearFocusDepth;
	vgui::TextEntry *m_pReadout_DoFNearBlurDepth;
	vgui::TextEntry *m_pReadout_DoFNearBlurRadius;

	bool m_bLastVisibilityState;

	MESSAGE_FUNC(OnDataChanged, "ControlModified");
	MESSAGE_FUNC(OnSliderMoved, "SliderMoved");
	MESSAGE_FUNC(OnTextChanged, "TextNewLine");
};

//-----------------------------------------------------------------------------
// Purpose: Holds all setting pages for the weather configurator dialog
// (currently fog and DoF settings)
//-----------------------------------------------------------------------------
class CWeatherSettingsDialog : public vgui::PropertyDialog
{
	DECLARE_CLASS_SIMPLE(CWeatherSettingsDialog, vgui::PropertyDialog);

public:
	CWeatherSettingsDialog(vgui::Panel *parent);
	~CWeatherSettingsDialog();

	void Run();
	virtual void Activate();
	virtual void OnTick();

	MESSAGE_FUNC(OnGameUIHidden, "GameUIHidden");
private:
	class CWeatherSettings_Fog *m_pFogSettingsPanel;
	class CWeatherSettings_HDR *m_pHDRSettingsPanel;
	class CWeatherSettings_DoF *m_pDoFSettingsPanel;
};

#endif //IWEATHERSETTINGS_H