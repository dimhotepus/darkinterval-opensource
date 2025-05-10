//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Hud locator element, helps direct the player to objects in the world
//
//=============================================================================//

#include "cbase.h"
#include "hudelement.h"
#include "hud_numericdisplay.h"
#include <vgui_controls/Panel.h>
#include "hud.h"
#include "hud_suitpower.h"
#include "hud_macros.h"
#include "iclientmode.h"
#include <vgui_controls/AnimationController.h>
#include <vgui/ISurface.h>
#include "c_basehlplayer.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PIPBOY_MATERIAL_NEUTRAL			"vgui/icons/icon_jalopy"
#define PIPBOY_MATERIAL_HOSTILE			"vgui/icons/icon_jalopy"
#define PIPBOY_MATERIAL_BIG_TICK		"vgui/icons/tick_long"
#define PIPBOY_MATERIAL_SMALL_TICK		"vgui/icons/tick_short"

ConVar hud_pipboy_alpha("hud_locator_alpha", "230");
ConVar hud_pipboy_fov("hud_locator_fov", "180");

//-----------------------------------------------------------------------------
// Purpose: Shows positions of objects relative to the player.
//-----------------------------------------------------------------------------
class CHudPipboy : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudPipboy, vgui::Panel);

public:
	CHudPipboy(const char *pElementName);
	virtual ~CHudPipboy(void);

	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	void VidInit(void);
	bool ShouldDraw();

protected:
	void FillRect(int x, int y, int w, int h);
	float LocatorXPositionForYawDiff(float yawDiff);
	void DrawGraduations(float flYawPlayerFacing);
	virtual void Paint();

private:
	void Reset(void);

	int m_textureID_IconNeutral;
	int m_textureID_IconHostile;
	int m_textureID_IconBigTick;
	int m_textureID_IconSmallTick;

	Vector			m_vecLocation;
};

using namespace vgui;

#ifdef HL2_EPISODIC
DECLARE_HUDELEMENT(CHudPipboy);
#endif 

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudPipboy::CHudPipboy(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudPipboy")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);

	m_textureID_IconNeutral = -1;
	m_textureID_IconHostile = -1;
	m_textureID_IconSmallTick = -1;
	m_textureID_IconBigTick = -1;
}

CHudPipboy::~CHudPipboy(void)
{
	if (vgui::surface())
	{
		if (m_textureID_IconNeutral != -1)
		{
			vgui::surface()->DestroyTextureID(m_textureID_IconNeutral);
			m_textureID_IconNeutral = -1;
		}

		if (m_textureID_IconHostile != -1)
		{
			vgui::surface()->DestroyTextureID(m_textureID_IconHostile);
			m_textureID_IconHostile = -1;
		}

		if (m_textureID_IconSmallTick != -1)
		{
			vgui::surface()->DestroyTextureID(m_textureID_IconSmallTick);
			m_textureID_IconSmallTick = -1;
		}

		if (m_textureID_IconBigTick != -1)
		{
			vgui::surface()->DestroyTextureID(m_textureID_IconBigTick);
			m_textureID_IconBigTick = -1;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pScheme - 
//-----------------------------------------------------------------------------
void CHudPipboy::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CHudPipboy::VidInit(void)
{
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
bool CHudPipboy::ShouldDraw(void)
{
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return false;

	if (pPlayer->GetVehicle())
		return false;

	if (pPlayer->m_HL2Local.m_vecLocatorOrigin == vec3_invalid)
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Start with our background off
//-----------------------------------------------------------------------------
void CHudPipboy::Reset(void)
{
	m_vecLocation = Vector(0, 0, 0);
}

//-----------------------------------------------------------------------------
// Purpose: Make it a bit more convenient to do a filled rect.
//-----------------------------------------------------------------------------
void CHudPipboy::FillRect(int x, int y, int w, int h)
{
	int panel_x, panel_y, panel_w, panel_h;
	GetBounds(panel_x, panel_y, panel_w, panel_h);
	vgui::surface()->DrawFilledRect(x, y, x + w, y + h);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
float CHudPipboy::LocatorXPositionForYawDiff(float yawDiff)
{
	float fov = hud_pipboy_fov.GetFloat() / 2;
	float remappedAngle = RemapVal(yawDiff, -fov, fov, -90, 90);
	float cosine = sin(DEG2RAD(remappedAngle));
	int element_wide = GetWide();

	float position = (element_wide >> 1) + ((element_wide >> 1) * cosine);

	return position;
}

//-----------------------------------------------------------------------------
// Draw the tickmarks on the locator
//-----------------------------------------------------------------------------
#define NUM_GRADUATIONS	16.0f
void CHudPipboy::DrawGraduations(float flYawPlayerFacing)
{
	int icon_wide, icon_tall;
	int xPos, yPos;
	float fov = hud_pipboy_fov.GetFloat() / 2;

	if (m_textureID_IconBigTick == -1)
	{
		m_textureID_IconBigTick = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(m_textureID_IconBigTick, PIPBOY_MATERIAL_BIG_TICK, true, false);
	}

	if (m_textureID_IconSmallTick == -1)
	{
		m_textureID_IconSmallTick = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(m_textureID_IconSmallTick, PIPBOY_MATERIAL_SMALL_TICK, true, false);
	}

	int element_tall = GetTall();		// Height of the VGUI element

	surface()->DrawSetColor(255, 255, 255, 255);

	// Tick Icons

	float angleStep = 360.0f / NUM_GRADUATIONS;
	bool tallLine = true;

	for (float angle = -180; angle <= 180; angle += angleStep)
	{
		yPos = (element_tall >> 1);

		if (tallLine)
		{
			vgui::surface()->DrawSetTexture(m_textureID_IconBigTick);
			vgui::surface()->DrawGetTextureSize(m_textureID_IconBigTick, icon_wide, icon_tall);
			tallLine = false;
		}
		else
		{
			vgui::surface()->DrawSetTexture(m_textureID_IconSmallTick);
			vgui::surface()->DrawGetTextureSize(m_textureID_IconSmallTick, icon_wide, icon_tall);
			tallLine = true;
		}

		float flDiff = UTIL_AngleDiff(flYawPlayerFacing, angle);

		if (fabs(flDiff) > fov)
			continue;

		float xPosition = LocatorXPositionForYawDiff(flDiff);

		xPos = (int)xPosition;
		xPos -= (icon_wide >> 1);

		vgui::surface()->DrawTexturedRect(xPos, yPos, xPos + icon_wide, yPos + icon_tall);
	}
}

//-----------------------------------------------------------------------------
// Purpose: draws the locator icons on the VGUI element.
//-----------------------------------------------------------------------------
void CHudPipboy::Paint()
{
	if (m_textureID_IconNeutral == -1)
	{
		m_textureID_IconNeutral = vgui::surface()->CreateNewTextureID();
		vgui::surface()->DrawSetTextureFile(m_textureID_IconNeutral, PIPBOY_MATERIAL_NEUTRAL, true, false);
	}

	int alpha = hud_pipboy_alpha.GetInt();

	SetAlpha(alpha);

	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	if (pPlayer->m_HL2Local.m_vecLocatorOrigin == vec3_origin)
		return;

	int element_tall = GetTall();		// Height of the VGUI element

	float fov = (hud_pipboy_fov.GetFloat()) / 2.0f;

	// Compute the relative position of objects we're tracking
	// We'll need the player's yaw for comparison.
	float flYawPlayerForward = pPlayer->EyeAngles().y;

	// Copy this value out of the member variable in case we decide to expand this
	// feature later and want to iterate some kind of list. 
}


