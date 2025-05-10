//========= Copyright, Valve Corporation, All rights reserved. ============//
//
// Purpose:
//
//=============================================================================//

#ifndef HUD_LOCATOR_H
#define HUD_LOCATOR_H
#ifdef _WIN32
#pragma once
#endif

#include "hudelement.h"
#include <vgui_controls/Panel.h>
#include "hl2_vehicle_radar.h"
#ifdef DARKINTERVAL
class CLocatorContact
{
public:
	int		m_iType;
	EHANDLE	m_pEnt;

	CLocatorContact();
};
#endif
//-----------------------------------------------------------------------------
// Purpose: Shows positions of objects relative to the player.
//-----------------------------------------------------------------------------
class CHudLocator : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudLocator, vgui::Panel);

public:
	CHudLocator(const char *pElementName);
#ifndef DARKINTERVAL
	virtual ~CHudLocator(void);
#endif
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	void VidInit(void);
	bool ShouldDraw();
#ifdef DARKINTERVAL
	void OnThink(void);

	void AddLocatorContact(EHANDLE hEnt, int iType);
	int FindLocatorContact(EHANDLE hEnt);
	void MaintainLocatorContacts();
	void ClearAllLocatorContacts() { m_iNumlocatorContacts = 0; }
#endif
protected:
	void FillRect(int x, int y, int w, int h);
	float LocatorXPositionForYawDiff(float yawDiff);
	void DrawGraduations(float flYawPlayerFacing);
	virtual void Paint();

private:
	void Reset(void);

	int m_textureID_IconPointOfInterest;
	int m_textureID_IconOutpost;
	int m_textureID_IconMainObjective;
//	int m_textureID_IconBigEnemy;
//	int m_textureID_IconRadiation;
#ifndef DARKINTERVAL
	int m_textureID_IconJalopy;
	int m_textureID_IconBigTick;
	int m_textureID_IconSmallTick;
#endif
	Vector			m_vecLocation;
#ifdef DARKINTERVAL
	CLocatorContact	m_locatorContacts[LOCATOR_MAX_CONTACTS];
	int				m_iNumlocatorContacts;

	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "Default");
#endif
};
#ifdef DARKINTERVAL
extern CHudLocator *GetHudLocator();
#endif
#endif // HUD_LOCATOR_H
