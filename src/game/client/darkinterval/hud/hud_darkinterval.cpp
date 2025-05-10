#include "cbase.h"
#include "engine/ienginesound.h" // for quickinfo
#include "hud.h"
#include "../hud_crosshair.h" // for quickinfo
#include "hud_macros.h"
#include "view.h"
#include "rendertexture.h"
#include "debugoverlay_shared.h"

#include "iclientmode.h"
#include "iclientvehicle.h"
#include "ihudlcd.h"

#include "c_basehlplayer.h"		// for weapon selection
#include "hud_history_resource.h"   // ditto
#include "input.h"				// ditto
#include "weapon_selection.h"   // ditto

#include <keyvalues.h>
#include <vgui/ISurface.h>
#include <vgui/ISystem.h>
#include <vgui_controls/animationcontroller.h>
#include "vgui_controls/Label.h"
#include <vgui/ilocalize.h>

#ifdef SIXENSE
#include "sixense/in_sixense.h"
#include "view.h"
int ScreenTransform(const Vector& point, Vector& screen);
#endif

using namespace vgui;

#include "hudelement.h"
#include "hud_numericdisplay.h"
#include "text_message.h"
#include "convar.h"
#include "ammodef.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar hud_transparency;
//ConVar hud_scale("hud_scale", "1.0");

static ConVar hud_show_damage_direction("hud_show_damage_direction", "1", FCVAR_ARCHIVE);

#define INIT_HEALTH -1
#define INIT_BAT	-1

//-----------------------------------------------------------------------------
// Purpose: Health panel when in HEV
//-----------------------------------------------------------------------------
class HUD_HealthCount : public CHudElement, public CHudNumericDisplay
{
	DECLARE_CLASS_SIMPLE(HUD_HealthCount, CHudNumericDisplay);

public:
	HUD_HealthCount(const char *pElementName);
	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual void OnThink();
	virtual void Paint();
	void MsgFunc_Damage(bf_read &msg);

private:
	CHudTexture *icon_health;
	// old variables
	int		m_iHealth;
	int		m_bitsDamage;
};

DECLARE_HUDELEMENT(HUD_HealthCount);
DECLARE_HUD_MESSAGE(HUD_HealthCount, Damage);

HUD_HealthCount::HUD_HealthCount(const char *pElementName) : CHudElement(pElementName), CHudNumericDisplay(NULL, "HudHealth")
{
	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void HUD_HealthCount::Init()
{
	HOOK_HUD_MESSAGE(HUD_HealthCount, Damage);
	Reset();
}

void HUD_HealthCount::Reset()
{
	m_iHealth = INIT_HEALTH;
	m_bitsDamage = 0;
/*
	wchar_t *tempString = g_pVGuiLocalize->Find("#Valve_Hud_HEALTH");

	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"HEALTH");
	}

	SetDisplayValue(m_iHealth);
*/
}

void HUD_HealthCount::VidInit()
{
	Reset();
}

void HUD_HealthCount::OnThink()
{
	BaseClass::OnThink();

	int newHealth = 0;
	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();
	if (local)
	{
		// Never below zero
		newHealth = MAX(local->GetHealth(), 0);
	}

	// Only update the fade if we've changed health
	if (newHealth == m_iHealth)
	{
		return;
	}

	m_iHealth = newHealth;

	if (m_iHealth >= 25)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthIncreasedAbove20");
	}
	else if (m_iHealth > 0)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthIncreasedBelow20");
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthLow");
	}

	SetDisplayValue(m_iHealth);
}

void HUD_HealthCount::Paint()
{
	if (!ShouldDraw())
		return;

	BaseClass::Paint();

	int alpha = hud_transparency.GetInt();

	if (m_iHealth > 0)
	{
		SetAlpha(alpha);
	}

	if (!icon_health)
	{
		icon_health = gHUD.GetIcon("cross");
	}

	if (!icon_health)
	{
		return;
	}

	Color	clrHealth;
	int x, y;

	float screenScale = 1.0;
	if (ScreenWidth() != 0 && ScreenHeight() != 0)
	{
		if (ScreenWidth() / ScreenHeight() > 1.7f) // 16:9
		{
			screenScale = float((float)ScreenWidth() / 1920.0f); // our hud textures are measured for 1920x1080
		}
		else if (ScreenWidth() / ScreenHeight() > 1.5f)  //1650x1080
		{
			screenScale = float((float)ScreenWidth() / 1650.0f);
		}
		else
		{
			screenScale = float((float)ScreenWidth() / 1440.0f); // the closest in 4:3 would be 1440x1080
		}
	}

	x = GetWide() / 8;
	y = GetTall() / 2 - icon_health->EffectiveHeight(screenScale) / 2;

	if (m_iHealth >= 25)
	{
		int r, g, b;
		r = GetFgColor().r(); g = GetFgColor().g(); b = GetFgColor().b();

		clrHealth.SetColor(r, g, b, alpha);
	}
	else
	{
		clrHealth.SetColor(250, 0, 0, alpha > 0 ? alpha + 50 : alpha);
	}

	icon_health->DrawSelf(x, y, icon_health->EffectiveWidth(screenScale), icon_health->EffectiveHeight(screenScale), clrHealth);
}

void HUD_HealthCount::MsgFunc_Damage(bf_read &msg)
{

	int armor = msg.ReadByte();	// armor
	int damageTaken = msg.ReadByte();	// health
	long bitsDamage = msg.ReadLong(); // damage bits
	bitsDamage; // variable still sent but not used

	Vector vecFrom;

	vecFrom.x = msg.ReadBitCoord();
	vecFrom.y = msg.ReadBitCoord();
	vecFrom.z = msg.ReadBitCoord();

	// Actually took damage?
	if (damageTaken > 0)
	{
		// start the animation
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthDamageTaken");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Health bar when wearing only the worker suit
//-----------------------------------------------------------------------------

class CHudHealthBarWorker : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudHealthBarWorker, vgui::Panel);

public:
	CHudHealthBarWorker(const char * pElementName);
	bool ShouldDraw();
	virtual void Init(void);
	virtual void Reset(void);
	virtual void Paint();

protected:
	virtual void OnThink(void);

private:
	CPanelAnimationVar(Color, m_HullColor, "HullColor", "150 150 165 200");
	CPanelAnimationVar(int, m_iHullDisabledAlpha, "HullDisabledAlpha", "50");
	CPanelAnimationVarAliasType(float, m_flBarInsetX, "BarInsetX", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarInsetY, "BarInsetY", "3", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarWidth, "BarWidth", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarHeight, "BarHeight", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkWidth, "BarChunkWidth", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkGap, "BarChunkGap", "0", "proportional_float");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HUDBarText");
	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_xpos, "text2_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_ypos, "text2_ypos", "40", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_gap, "text2_gap", "10", "proportional_float");
	int m_iHealth;

};

DECLARE_HUDELEMENT(CHudHealthBarWorker);

CHudHealthBarWorker::CHudHealthBarWorker(const char * pElementName) :
	CHudElement(pElementName), BaseClass(NULL, "HudHealthBarWorker")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD /*| HIDEHUD_ARCADE_HUD*/); // | HIDEHUD_NEEDSUIT);
}

bool CHudHealthBarWorker::ShouldDraw(void)
{
	bool bNeedsDraw = GetAlpha() > 0;
	C_BasePlayer * local = C_BasePlayer::GetLocalPlayer();
	if (local)
	{
		if (local->IsSuitEquipped()) bNeedsDraw = false;
		if (!(local->IsSuitlessHUDVisible())) bNeedsDraw = false;
		if (local->IsPlayingArcade()) bNeedsDraw = false;
	}
	return (bNeedsDraw && CHudElement::ShouldDraw());
}

void CHudHealthBarWorker::Init()
{
	Reset();
}

void CHudHealthBarWorker::Reset(void)
{
	m_iHealth = -1;
	SetBgColor(Color(255, 255, 0, 0));
	SetRoundedCorners(PANEL_ROUND_CORNER_ALL);
	SetPaintBackgroundEnabled(true);
}

void CHudHealthBarWorker::OnThink(void)
{
	BaseClass::OnThink();

	float newHealth = 0;
	C_BasePlayer * local = C_BasePlayer::GetLocalPlayer();

	if (!local)
		return;

	// Never below zero 
	newHealth = max(local->GetHealth(), 0);

	// Only update the fade if we've changed health
	if (newHealth == m_iHealth)
		return;

	m_iHealth = newHealth;

	if (m_iHealth >= 25)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthIncreasedAbove20");
	}
	else if (m_iHealth > 0)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthIncreasedBelow20");
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthLow");
	}
}

void CHudHealthBarWorker::Paint()
{
	// Get bar chunks

	int chunkCount = m_flBarWidth / (m_flBarChunkWidth + m_flBarChunkGap);
	int enabledChunks = (int)((float)chunkCount * (m_iHealth / 100.0f) + 0.0f);

	// Draw the suit power bar
	surface()->DrawSetColor(m_HullColor);

	int xpos = m_flBarInsetX, ypos = m_flBarInsetY;

	for (int i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}

	// Draw the exhausted portion of the bar.
	surface()->DrawSetColor(Color(m_HullColor[0], m_HullColor[1], m_HullColor[2], m_iHullDisabledAlpha));

	for (int i = enabledChunks; i < chunkCount; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}

	int alpha = hud_transparency.GetInt();
	SetAlpha(alpha);
}

//-----------------------------------------------------------------------------
// Purpose: Ammo bars when wearing only the worker suit
//-----------------------------------------------------------------------------

class CHudAmmoMainWorker : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudAmmoMainWorker, vgui::Panel);

public:
	CHudAmmoMainWorker(const char * pElementName);
	bool ShouldDraw();
	virtual void Init(void);
	virtual void Reset(void);
	virtual void Paint();
	void SetAmmo(int ammo, bool playAnimation);

protected:
	virtual void OnThink(void);
	void UpdateAmmoBar(C_BasePlayer *player);

private:
	CHandle< C_BaseCombatWeapon > m_hCurrentActiveWeapon;
	int		m_iAmmo;
	bool	m_warnAmmo;

	CPanelAnimationVar(Color, m_HullColor, "HullColor", "150 150 165 200");
	CPanelAnimationVar(int, m_iHullDisabledAlpha, "HullDisabledAlpha", "50");
	CPanelAnimationVarAliasType(float, m_flBarInsetX, "BarInsetX", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarInsetY, "BarInsetY", "3", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarWidth, "BarWidth", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarHeight, "BarHeight", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkWidth, "BarChunkWidth", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkGap, "BarChunkGap", "0", "proportional_float");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HUDBarText");
	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_xpos, "text2_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_ypos, "text2_ypos", "40", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_gap, "text2_gap", "10", "proportional_float");

};

DECLARE_HUDELEMENT(CHudAmmoMainWorker);

CHudAmmoMainWorker::CHudAmmoMainWorker(const char * pElementName) :
	CHudElement(pElementName), BaseClass(NULL, "HudAmmoMainWorker")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD /*| HIDEHUD_ARCADE_HUD*/); // | HIDEHUD_NEEDSUIT);
}

bool CHudAmmoMainWorker::ShouldDraw(void)
{
	bool bNeedsDraw = GetAlpha() > 0;
	C_BasePlayer * local = C_BasePlayer::GetLocalPlayer();
	if (local)
	{
		if (local->IsSuitEquipped()) bNeedsDraw = false;
		if (!(local->IsSuitlessHUDVisible())) bNeedsDraw = false;
		if (local->IsPlayingArcade()) bNeedsDraw = false;
	}
	return (bNeedsDraw && CHudElement::ShouldDraw());
}

void CHudAmmoMainWorker::Init()
{
	Reset();
}

void CHudAmmoMainWorker::Reset(void)
{
	SetBgColor(Color(255, 255, 0, 0));
	m_iAmmo = -1;
	m_warnAmmo = false;
	m_hCurrentActiveWeapon = NULL;

	C_BasePlayer * local = C_BasePlayer::GetLocalPlayer();

	if (local != NULL)	UpdateAmmoBar(local);
}

void CHudAmmoMainWorker::OnThink(void)
{
	BaseClass::OnThink();

	float newHealth = 0;
	C_BasePlayer * local = C_BasePlayer::GetLocalPlayer();

	if (!local)
		return;

	UpdateAmmoBar(local);
}

void CHudAmmoMainWorker::UpdateAmmoBar(C_BasePlayer *player)
{
	C_BaseCombatWeapon *wpn = GetActiveWeapon();
	
	if (!wpn || !player || !wpn->UsesPrimaryAmmo())
	{
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		return;
	}

	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);
	
	// get the ammo in our clip
	int ammo1 = wpn->Clip1();
	if (ammo1 < 0 )
	{
		// we don't use clip ammo, just use the total ammo count
		ammo1 = player->GetAmmoCount(wpn->GetPrimaryAmmoType());
	}
	
	if (wpn == m_hCurrentActiveWeapon)
	{
		// same weapon, just update counts
		SetAmmo(ammo1, true);
	}
	else
	{
		// diferent weapon, change without triggering
		SetAmmo(ammo1, false);

		// update whether or not we show the total ammo display
		if (wpn->UsesClipsForAmmo1())
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponUsesClips");
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponDoesNotUseClips");
		}

		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponChanged");
		m_hCurrentActiveWeapon = wpn;
	}
}

void CHudAmmoMainWorker::SetAmmo(int ammo, bool playAnimation)
{
	if (ammo != m_iAmmo)
	{
		if (ammo == 0)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoEmpty");
		}
		else if (ammo < m_iAmmo)
		{
			// ammo has decreased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoDecreased");
		}
		else
		{
			// ammunition has increased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoIncreased");
		}

		m_iAmmo = ammo;
	}
}

void CHudAmmoMainWorker::Paint()
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (player == NULL)
		return;

	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if (pWeapon == NULL)
		return;

	int alpha = hud_transparency.GetInt();
	SetAlpha(alpha);

	// Check our ammo for a warning
	int	ammo = pWeapon->Clip1();
	if (ammo != m_iAmmo)
	{
		m_iAmmo = ammo;

		// Find how far through the current clip we are
		float ammoPerc = (float)ammo / (float)pWeapon->GetMaxClip1();

		// Warn if we're below a certain percentage of our clip's size
		if ((pWeapon->GetMaxClip1() > 1) && (ammoPerc <= (1.0f - CLIP_PERC_THRESHOLD)))
		{
			if (m_warnAmmo == false)
			{
				m_warnAmmo = true;
			}
		}
		else
		{
			m_warnAmmo = false;
		}
	}

	float ammoPerc;

	if (pWeapon->GetMaxClip1() <= 0)
	{
		ammoPerc = 0.0f;
	}
	else
	{
		ammoPerc = 1.0f - ((float)ammo / (float)pWeapon->GetMaxClip1());
		ammoPerc = clamp(ammoPerc, 0.0f, 1.0f);
	}

	Color ammoColor = m_warnAmmo ? gHUD.m_clrCaution : m_HullColor;
	/*
	// Draw the suit power bar

	int xpos = m_flBarInsetX, ypos = m_flBarInsetY;

	surface()->DrawSetColor(ammoColor);
	gHUD.DrawProgressBar(xpos, ypos, m_flBarWidth, m_flBarHeight, ammoPerc, m_HullColor, CHud::HUDPB_HORIZONTAL);

	// Draw the exhausted portion of the bar.
	surface()->DrawSetColor(Color(m_HullColor[0], m_HullColor[1], m_HullColor[2], m_iHullDisabledAlpha));
	surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarWidth, ypos + m_flBarHeight);
	*/

	// Get bar chunks
	int chunkCount = (float)pWeapon->GetMaxClip1();
	int enabledChunks = (int)((float)chunkCount * ((float)ammo / (float)pWeapon->GetMaxClip1()) + 0.0f);

	m_flBarChunkWidth = m_flBarWidth / chunkCount - m_flBarChunkGap;

	// Draw the suit power bar
	surface()->DrawSetColor(m_HullColor);

	int xpos = m_flBarInsetX, ypos = m_flBarInsetY;

	for (int i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}

	// Draw the exhausted portion of the bar.
	surface()->DrawSetColor(Color(m_HullColor[0], m_HullColor[1], m_HullColor[2], m_iHullDisabledAlpha));

	for (int i = enabledChunks; i < chunkCount; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}
}



class CHudAmmoReserveWorker : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudAmmoReserveWorker, vgui::Panel);

public:
	CHudAmmoReserveWorker(const char * pElementName);
	bool ShouldDraw();
	virtual void Init(void);
	virtual void Reset(void);
	virtual void Paint();
	void SetAmmo(int ammo, bool playAnimation);

protected:
	virtual void OnThink(void);
	void UpdateAmmoBar(C_BasePlayer *player);

private:
	CHandle< C_BaseCombatWeapon > m_hCurrentActiveWeapon;
	int		m_iAmmo;
	bool	m_warnAmmo;

	CPanelAnimationVar(Color, m_HullColor, "HullColor", "150 150 165 200");
	CPanelAnimationVar(int, m_iHullDisabledAlpha, "HullDisabledAlpha", "50");
	CPanelAnimationVarAliasType(float, m_flBarInsetX, "BarInsetX", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarInsetY, "BarInsetY", "3", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarWidth, "BarWidth", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarHeight, "BarHeight", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkWidth, "BarChunkWidth", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkGap, "BarChunkGap", "0", "proportional_float");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HUDBarText");
	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_xpos, "text2_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_ypos, "text2_ypos", "40", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_gap, "text2_gap", "10", "proportional_float");

};

DECLARE_HUDELEMENT(CHudAmmoReserveWorker);

CHudAmmoReserveWorker::CHudAmmoReserveWorker(const char * pElementName) :
	CHudElement(pElementName), BaseClass(NULL, "HudAmmoReserveWorker")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD /*| HIDEHUD_ARCADE_HUD*/); // | HIDEHUD_NEEDSUIT);
}

bool CHudAmmoReserveWorker::ShouldDraw(void)
{
	bool bNeedsDraw = GetAlpha() > 0;
	C_BasePlayer * local = C_BasePlayer::GetLocalPlayer();
	if (local)
	{
		if (local->IsSuitEquipped()) bNeedsDraw = false;
		if (!(local->IsSuitlessHUDVisible())) bNeedsDraw = false;
		if (local->IsPlayingArcade()) bNeedsDraw = false;
	}
	return (bNeedsDraw && CHudElement::ShouldDraw());
}

void CHudAmmoReserveWorker::Init()
{
	Reset();
}

void CHudAmmoReserveWorker::Reset(void)
{
	SetBgColor(Color(255, 255, 0, 0));
	m_iAmmo = -1;
	m_warnAmmo = false;
	m_hCurrentActiveWeapon = NULL;

	C_BasePlayer * local = C_BasePlayer::GetLocalPlayer();

	if (local != NULL)	UpdateAmmoBar(local);
}

void CHudAmmoReserveWorker::OnThink(void)
{
	BaseClass::OnThink();

	float newHealth = 0;
	C_BasePlayer * local = C_BasePlayer::GetLocalPlayer();

	if (!local)
		return;

	UpdateAmmoBar(local);
}

void CHudAmmoReserveWorker::UpdateAmmoBar(C_BasePlayer *player)
{
	C_BaseCombatWeapon *wpn = GetActiveWeapon();

	if (!wpn || !player || !wpn->UsesPrimaryAmmo())
	{
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		return;
	}

	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);

	// get the ammo in our clip
	int ammo1 = wpn->Clip1();
	if (ammo1 < 0)
	{
		// we don't use clip ammo, just use the total ammo count
		ammo1 = player->GetAmmoCount(wpn->GetPrimaryAmmoType());
	}
	else
	{
		// we use clip ammo, so the second ammo is the total ammo
		ammo1 = player->GetAmmoCount(wpn->GetPrimaryAmmoType());
	}

	if (wpn == m_hCurrentActiveWeapon)
	{
		// same weapon, just update counts
		SetAmmo(ammo1, true);
	}
	else
	{
		// diferent weapon, change without triggering
		SetAmmo(ammo1, false);

		// update whether or not we show the total ammo display
		if (wpn->UsesClipsForAmmo1())
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponUsesClips");
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponDoesNotUseClips");
		}

		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponChanged");
		m_hCurrentActiveWeapon = wpn;
	}
}

void CHudAmmoReserveWorker::SetAmmo(int ammo, bool playAnimation)
{
	if (ammo != m_iAmmo)
	{
		if (ammo == 0)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoEmpty");
		}
		else if (ammo < m_iAmmo)
		{
			// ammo has decreased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoDecreased");
		}
		else
		{
			// ammunition has increased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoIncreased");
		}

		m_iAmmo = ammo;
	}
}

void CHudAmmoReserveWorker::Paint()
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (player == NULL)
		return;

	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if (pWeapon == NULL)
		return;

	int alpha = hud_transparency.GetInt();
	SetAlpha(alpha);

	// Check our ammo for a warning
	int	ammo = player->GetAmmoCount(pWeapon->GetPrimaryAmmoType());

	if (ammo != m_iAmmo)
	{
		m_iAmmo = ammo;

		// Find how far through the current clip we are
		float ammoPerc = (float)ammo / (float)GetAmmoDef()->MaxCarry(pWeapon->GetPrimaryAmmoType());
	}

	float ammoPerc;

	if (player->GetAmmoCount(pWeapon->GetPrimaryAmmoType()) <= 0)
	{
		ammoPerc = 0.0f;
	}
	else
	{
		ammoPerc = (float)ammo / (float)GetAmmoDef()->MaxCarry(pWeapon->GetPrimaryAmmoType());
		ammoPerc = clamp(ammoPerc, 0.0f, 1.0f);
	}

	Color ammoColor = m_warnAmmo ? gHUD.m_clrCaution : m_HullColor;

	// Draw the suit power bar
	/*
	int xpos = m_flBarInsetX, ypos = m_flBarInsetY;

	if (ammoPerc > 0)
	{
	surface()->DrawSetColor(ammoColor);
	gHUD.DrawProgressBar(xpos, ypos, m_flBarWidth, m_flBarHeight, ammoPerc, m_HullColor, CHud::HUDPB_HORIZONTAL);
	}

	// Draw the exhausted portion of the bar.
	surface()->DrawSetColor(Color(m_HullColor[0], m_HullColor[1], m_HullColor[2], m_iHullDisabledAlpha));
	surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarWidth, ypos + m_flBarHeight);
	*/

	// Get bar chunks
	int chunkCount = GetAmmoDef()->MaxCarry(pWeapon->GetPrimaryAmmoType());
	int enabledChunks = (int)(chunkCount * ammoPerc);

	m_flBarChunkWidth = m_flBarWidth / chunkCount; // -m_flBarChunkGap;

												   // Draw the suit power bar
	surface()->DrawSetColor(m_HullColor);

	int xpos = m_flBarInsetX, ypos = m_flBarInsetY;

	for (int i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth/* + m_flBarChunkGap*/);
	}

	// Draw the exhausted portion of the bar.
	surface()->DrawSetColor(Color(m_HullColor[0], m_HullColor[1], m_HullColor[2], m_iHullDisabledAlpha));

	for (int i = enabledChunks; i < chunkCount; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth/* + m_flBarChunkGap*/);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Health bar inside Arcade games
//-----------------------------------------------------------------------------

class CHudHealthBarArcade : public CHudElement, public vgui::Panel
{

	DECLARE_CLASS_SIMPLE(CHudHealthBarArcade, vgui::Panel);

public:
	CHudHealthBarArcade(const char * pElementName);

	virtual void Init(void);
	virtual void Reset(void);
	virtual void Paint();

protected:
	virtual void OnThink(void);

private:
	CPanelAnimationVar(Color, m_HullColor, "HullColor", "50 255 0 255");
	CPanelAnimationVar(int, m_iHullDisabledAlpha, "HullDisabledAlpha", "50");
	CPanelAnimationVarAliasType(float, m_flBarInsetX, "BarInsetX", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarInsetY, "BarInsetY", "3", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarWidth, "BarWidth", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarHeight, "BarHeight", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkWidth, "BarChunkWidth", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flBarChunkGap, "BarChunkGap", "0", "proportional_float");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HUDBarText");
	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_xpos, "text2_xpos", "8", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_ypos, "text2_ypos", "40", "proportional_float");
	CPanelAnimationVarAliasType(float, text2_gap, "text2_gap", "10", "proportional_float");
	float m_iHealth;

};

DECLARE_HUDELEMENT(CHudHealthBarArcade);

CHudHealthBarArcade::CHudHealthBarArcade(const char * pElementName) :
	CHudElement(pElementName), BaseClass(NULL, "HudHealthBarArcade")
{
	vgui::Panel * pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_ARCADE_HUD); // | HIDEHUD_NEEDSUIT);
}

void CHudHealthBarArcade::Init()
{
	Reset();
}

void CHudHealthBarArcade::Reset(void)
{
	m_iHealth = -1;
	SetBgColor(Color(255, 255, 0, 0));
}

void CHudHealthBarArcade::OnThink(void)
{
	BaseClass::OnThink();

	float newHealth = 0;
	C_BasePlayer * local = C_BasePlayer::GetLocalPlayer();

	if (!local)
		return;

	// Never below zero 
	newHealth = max(local->GetHealth(), 0);

	// DevMsg("Shield at is at: %f\n",newShield);
	// Only update the fade if we've changed health
	if (newHealth == m_iHealth)
		return;

	m_iHealth = newHealth;
}

void CHudHealthBarArcade::Paint()
{
	// Get bar chunks

	int chunkCount = m_flBarWidth / (m_flBarChunkWidth + m_flBarChunkGap);
	int enabledChunks = (int)((float)chunkCount * (m_iHealth / 100.0f) + 0.0f);

	// Draw the suit power bar
	surface()->DrawSetColor(m_HullColor);

	int xpos = m_flBarInsetX, ypos = m_flBarInsetY;

	for (int i = 0; i < enabledChunks; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}

	// Draw the exhausted portion of the bar.
	surface()->DrawSetColor(Color(m_HullColor[0], m_HullColor[1], m_HullColor[2], m_iHullDisabledAlpha));

	for (int i = enabledChunks; i < chunkCount; i++)
	{
		surface()->DrawFilledRect(xpos, ypos, xpos + m_flBarChunkWidth, ypos + m_flBarHeight);
		xpos += (m_flBarChunkWidth + m_flBarChunkGap);
	}
}

//-----------------------------------------------------------------------------
// Purpose: HEV suit power
//-----------------------------------------------------------------------------
class HUD_ArmourCount : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(HUD_ArmourCount, CHudNumericDisplay);

public:
	HUD_ArmourCount(const char *pElementName);
	void Init(void);
	void Reset(void);
	void VidInit(void);
	void OnThink(void);
	void MsgFunc_Battery(bf_read &msg);
	bool ShouldDraw();
	void Paint();

private:
	CHudTexture		*icon_suit_empty;
	CHudTexture		*icon_suit_full;
	int		m_iBat;
	int		m_iNewBat;
};

DECLARE_HUDELEMENT(HUD_ArmourCount);
DECLARE_HUD_MESSAGE(HUD_ArmourCount, Battery);

HUD_ArmourCount::HUD_ArmourCount(const char *pElementName) : BaseClass(NULL, "HudSuit"), CHudElement(pElementName)
{
	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_NEEDSUIT);
}

void HUD_ArmourCount::Init(void)
{
	HOOK_HUD_MESSAGE(HUD_ArmourCount, Battery);
	Reset();
	m_iBat = INIT_BAT;
	m_iNewBat = 0;
}

void HUD_ArmourCount::Reset(void)
{
//	SetLabelText(g_pVGuiLocalize->Find("#Valve_Hud_SUIT"));
	SetDisplayValue(m_iBat);
}

void HUD_ArmourCount::VidInit(void)
{
	Reset();
}

bool HUD_ArmourCount::ShouldDraw(void)
{
	bool bNeedsDraw = (m_iBat != m_iNewBat) || (GetAlpha() > 0);

	return (bNeedsDraw && CHudElement::ShouldDraw());
}

void HUD_ArmourCount::OnThink(void)
{
	BaseClass::OnThink();

	if (m_iBat == m_iNewBat)
		return;

	if (!m_iNewBat)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SuitPowerZero");
	}
	else if (m_iNewBat < m_iBat)
	{
		// battery power has decreased, so play the damaged animation
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SuitDamageTaken");

		// play an extra animation if we're super low
		if (m_iNewBat < 20)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SuitArmorLow");
		}
	}
	else
	{
		// battery power has increased (if we had no previous armor, or if we just loaded the game, don't use alert state)
		if (m_iBat == INIT_BAT || m_iBat == 0 || m_iNewBat >= 20)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SuitPowerIncreasedAbove20");
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("SuitPowerIncreasedBelow20");
		}
	}

	m_iBat = m_iNewBat;

	SetDisplayValue(m_iBat);
}

void HUD_ArmourCount::Paint()
{
	Color	clrHealth;
//	int		a;
	int		x;
	int		y;

	BaseClass::Paint();
	
	int alpha = hud_transparency.GetInt();

	if (m_iBat > 0)
	{
		SetAlpha(alpha);
	}

	if (!icon_suit_empty)
	{
		icon_suit_empty = gHUD.GetIcon("suit_empty");
	}

	if (!icon_suit_full)
	{
		icon_suit_full = gHUD.GetIcon("suit_full");
	}

	if (!icon_suit_empty || !icon_suit_full)
	{
		return;
	}

	float screenScale = 1.0;
	if (ScreenWidth() != 0 && ScreenHeight() != 0)
	{
		if (ScreenWidth() / ScreenHeight() > 1.34f) // 16:9
		{
			screenScale = float((float)ScreenWidth() / 1920.0f); // our hud textures are measured for 1920x1080
		}
		else // 4:3 or 5:4
		{
			screenScale = float((float)ScreenWidth() / 1440.0f); // the closest in 4:3 would be 1440x1080
		}
	}
	int nFontHeight = GetTall();
	int nIconWidth = icon_suit_empty->EffectiveWidth(screenScale);

	x = GetWide() / 8;
	y = GetTall() / 2 - icon_suit_empty->EffectiveHeight(screenScale) / 4;

//	x = GetWide() / 5;
//	y = GetTall() / 2 - icon_suit_empty->Height() / 2;

	int iOffset = icon_suit_empty->EffectiveHeight(screenScale) / 6;
	
	int r, g, b;
	r = GetFgColor().r(); g = GetFgColor().g(); b = GetFgColor().b();
	clrHealth.SetColor(r, g, b, alpha);
		
	icon_suit_empty->DrawSelf(x, y - iOffset, icon_suit_empty->EffectiveWidth(screenScale), icon_suit_empty->EffectiveHeight(screenScale), clrHealth);
	
	if (m_iBat > 0)
	{
		int nSuitOffset = icon_suit_empty->EffectiveHeight(screenScale) * ((float)(100 - (MIN(100, m_iBat))) * 0.01);	// battery can go from 0 to 100 so * 0.01 goes from 0 to 1
		icon_suit_full->DrawSelfCropped(x, y - iOffset + nSuitOffset, 0, nSuitOffset, icon_suit_full->EffectiveWidth(screenScale), icon_suit_full->EffectiveHeight(screenScale) - nSuitOffset, clrHealth);
	//	icon_suit_full->DrawSelf(x, y - iOffset + nSuitOffset, icon_suit_full->EffectiveWidth(screenScale), icon_suit_full->EffectiveHeight(screenScale) - nSuitOffset, clrHealth);

	}
}

void HUD_ArmourCount::MsgFunc_Battery(bf_read &msg)
{
	m_iNewBat = msg.ReadShort();
}

//-----------------------------------------------------------------------------
// Purpose: Vehicle health panel
//-----------------------------------------------------------------------------
class HUD_VehicleIntegrityCount : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(HUD_VehicleIntegrityCount, CHudNumericDisplay);

public:
	HUD_VehicleIntegrityCount(const char *pElementName);
	void Init(void);
	void Reset(void);
	void VidInit(void);
	void OnThink(void);
	void MsgFunc_VehicleIntegrity(bf_read &msg);
	bool ShouldDraw();
	void Paint();

private:
	CHudTexture		*icon_vehicle;
	int		m_iVehicleIntegrity;
	int		m_iNewVehicleIntegrity;

	CPanelAnimationVarAliasType(float, vehicle_xpos, "vehicle_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, vehicle_ypos, "vehicle_ypos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, vehicle_width, "vehicle_width", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, vehicle_height, "vehicle_height", "0", "proportional_float");
};

DECLARE_HUDELEMENT(HUD_VehicleIntegrityCount);
DECLARE_HUD_MESSAGE(HUD_VehicleIntegrityCount, VehicleIntegrity);

HUD_VehicleIntegrityCount::HUD_VehicleIntegrityCount(const char *pElementName) : BaseClass(NULL, "HudVehicleIntegrity"), CHudElement(pElementName)
{
	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_NEEDSUIT | HIDEHUD_VEHICLE_CROSSHAIR );
}

void HUD_VehicleIntegrityCount::Init(void)
{
	HOOK_HUD_MESSAGE(HUD_VehicleIntegrityCount, VehicleIntegrity);
	Reset();
	m_iVehicleIntegrity = INIT_BAT;
	m_iNewVehicleIntegrity = 0;
}

void HUD_VehicleIntegrityCount::Reset(void)
{
	//	SetLabelText(g_pVGuiLocalize->Find("#Valve_Hud_SUIT"));
	SetDisplayValue(m_iVehicleIntegrity);
}

void HUD_VehicleIntegrityCount::VidInit(void)
{
	Reset();
}

bool HUD_VehicleIntegrityCount::ShouldDraw(void)
{
	bool bNeedsDraw = ((m_iVehicleIntegrity != m_iNewVehicleIntegrity) || m_iVehicleIntegrity != 0);
	
	return (bNeedsDraw && CHudElement::ShouldDraw());
}

void HUD_VehicleIntegrityCount::OnThink(void)
{
	BaseClass::OnThink();

	if (m_iVehicleIntegrity == m_iNewVehicleIntegrity)
		return;

	if (m_iNewVehicleIntegrity <= 0)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("VehicleArmorZero");
	}
	else if (m_iNewVehicleIntegrity < m_iVehicleIntegrity)
	{
		// battery power has decreased, so play the damaged animation
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("VehicleDamageTaken");

		// play an extra animation if we're super low
		if (m_iVehicleIntegrity < 100)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("VehicleArmorLow");
		}
	}
	else
	{
		// battery power has increased (if we had no previous armor, or if we just loaded the game, don't use alert state)
		if (m_iVehicleIntegrity == INIT_BAT || m_iVehicleIntegrity == 0 || m_iNewVehicleIntegrity >= 20)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("VehicleArmorIncreasedAbove20");
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("VehicleArmorIncreasedBelow20");
		}
	}

	m_iVehicleIntegrity = m_iNewVehicleIntegrity;

	SetDisplayValue(m_iVehicleIntegrity);
}

void HUD_VehicleIntegrityCount::Paint()
{
//	if (!ShouldDraw())
//		return;

	Color	clrHealth;

	BaseClass::Paint();

	int alpha = hud_transparency.GetInt();
	
	if (m_iVehicleIntegrity > 0 && m_iNewVehicleIntegrity != -1)
	{
		SetAlpha(alpha);
	}
	else
	{
		SetAlpha(0);
	}

	if (!icon_vehicle)
	{
		icon_vehicle = gHUD.GetIcon("vehicle");
	}
	
	if (!icon_vehicle )
	{
		return;
	}

	// x = GetWide() / 8;
	// y = GetTall() / 2 - icon_vehicle->Height() / 4;

	//	x = GetWide() / 5;
	//	y = GetTall() / 2 - icon_suit_empty->Height() / 2;

	// int iOffset = icon_vehicle->Height() / 6;

	int r, g, b;
	r = GetFgColor().r(); g = GetFgColor().g(); b = GetFgColor().b();
	clrHealth.SetColor(r, g, b, alpha);

	int x, y;
	GetPos(x, y); 
	
	float screenScale = 1.0;
	if (ScreenWidth() != 0 && ScreenHeight() != 0)
	{
		if (ScreenWidth() / ScreenHeight() > 1.34f) // 16:9
		{
			screenScale = float((float)ScreenWidth() / 1920.0f); // our hud textures are measured for 1920x1080
		}
		else // 4:3 or 5:4
		{
			screenScale = float((float)ScreenWidth() / 1440.0f); // the closest in 4:3 would be 1440x1080
		}
	}

	icon_vehicle->DrawSelf((GetWide() / 8) + vehicle_xpos, (GetTall() / 2) + vehicle_ypos, 
		(int)(vehicle_width), (int)(vehicle_height), clrHealth);

//	if (m_iVehicleIntegrity > 0)
//	{
//		int nOffset = icon_vehicle->Height() * ((float)(500 - (MIN(500, m_iVehicleIntegrity))) * 0.05);	// battery can go from 0 to 100 so * 0.01 goes from 0 to 1
//		icon_suit_full->DrawSelfCropped(x, y - iOffset + nSuitOffset, 0, nSuitOffset, icon_suit_full->Width(), icon_suit_full->Height() - nSuitOffset, clrHealth);
//	}
}

void HUD_VehicleIntegrityCount::MsgFunc_VehicleIntegrity(bf_read &msg)
{
	m_iNewVehicleIntegrity = msg.ReadShort();
}

//-----------------------------------------------------------------------------
// Purpose: Damage "tiles" - little icons showing the type of damage recieved
//-----------------------------------------------------------------------------
#define DMG_IMAGE_LIFE		2	// seconds that image is up
#define NUM_DMG_TYPES		9

typedef struct
{
	float		flExpire;
	float		flBaseline;
	int			x;
	int			y;
	CHudTexture	*icon;
	long		fFlags;
} damagetile_t;

class CHudDamageTiles : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudDamageTiles, vgui::Panel);

public:
	CHudDamageTiles(const char *pElementName);

	void	Init(void);
	void	VidInit(void);
	void	Reset(void);
	bool	ShouldDraw(void);

	// Handler for our message
	void	MsgFunc_Damage(bf_read &msg);

private:
	void	Paint();
	void	ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	CHudTexture	*DamageTileIcon(int i);
	long		DamageTileFlags(int i);
	void		UpdateTiles(long bits);

private:
	long			m_bitsDamage;
	damagetile_t	m_dmgTileInfo[NUM_DMG_TYPES];
};


DECLARE_HUDELEMENT(CHudDamageTiles);
DECLARE_HUD_MESSAGE(CHudDamageTiles, Damage);

CHudDamageTiles::CHudDamageTiles(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudDamageTiles")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void CHudDamageTiles::Init(void)
{
	HOOK_HUD_MESSAGE(CHudDamageTiles, Damage);
}


void CHudDamageTiles::VidInit(void)
{
	Reset();
}

void CHudDamageTiles::Reset(void)
{
	m_bitsDamage = 0;
	for (int i = 0; i < NUM_DMG_TYPES; i++)
	{
		m_dmgTileInfo[i].flExpire = 0;
		m_dmgTileInfo[i].flBaseline = 0;
		m_dmgTileInfo[i].x = 0;
		m_dmgTileInfo[i].y = 0;
		m_dmgTileInfo[i].icon = DamageTileIcon(i);
		m_dmgTileInfo[i].fFlags = DamageTileFlags(i);
	}
}


CHudTexture *CHudDamageTiles::DamageTileIcon(int i)
{
	switch (i)
	{
	case 0:
	{
		return gHUD.GetIcon("dmg_poison");
	}
		break;
	case 1:
	{
		return gHUD.GetIcon("dmg_chem");
	}
		break;
	case 2:
	{
		return gHUD.GetIcon("dmg_cold");
	}
		break;
	case 3:
	{
		return gHUD.GetIcon("dmg_drown");
	}
		break;
	case 4:
	{
		return gHUD.GetIcon("dmg_heat");
	}
		break;
	case 5:
	{
		return gHUD.GetIcon("dmg_bio");
	}
		break;
	case 6:
	{
		return gHUD.GetIcon("dmg_rad");
	}
		break;
	case 7:
	{
		return gHUD.GetIcon("dmg_shock");
	}
		break;
	case 8:
	{
		return gHUD.GetIcon("dmg_sonic");
	}
		break;
	default:
		return gHUD.GetIcon("dmg_sonic");
		break;
	}
}

long CHudDamageTiles::DamageTileFlags(int i)
{
	switch (i)
	{
	case 0:
	{
		return DMG_POISON;
	}
		break;
	case 1:
	{
		return DMG_ACID;
	}
		break;
	case 2:
	{
		return DMG_FREEZE;
	}
		break;
	case 3:
	{
		return DMG_DROWN;
	}
		break;
	case 4:
	{
		return DMG_BURN | DMG_SLOWBURN;
	}
		break;
	case 5:
	{
		return DMG_NERVEGAS;
	}
		break;
	case 6:
	{
		return DMG_RADIATION;
	}
		break;
	case 7:
	{
		return DMG_SHOCK;
	}
		break;
	case 8:
	{
		return DMG_SONIC;
	}
		break;
	default:
		return DMG_GENERIC;
		break;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Message handler for Damage message
//-----------------------------------------------------------------------------
void CHudDamageTiles::MsgFunc_Damage(bf_read &msg)
{
	msg.ReadByte();	// armor
	msg.ReadByte();	// health
	long bitsDamage = msg.ReadLong(); // damage bits

	UpdateTiles(bitsDamage);
}

bool CHudDamageTiles::ShouldDraw(void)
{
	if (!CHudElement::ShouldDraw())
		return false;

	if (!m_bitsDamage)
		return false;

	return true;
}

void CHudDamageTiles::Paint(void)
{
	int				r, g, b, a;
	damagetile_t	*pDmgTile;
	Color			clrTile;

	r = GetFgColor().r(); g = GetFgColor().g(); b = GetFgColor().b();
//	(gHUD.m_clrNormal).GetColor(r, g, b, nUnused);
	a = (int)(fabs(sin(CURTIME * 2)) * 256.0);
	clrTile.SetColor(r, g, b, a);

	int i;
	// Draw all the items
	for (i = 0; i < NUM_DMG_TYPES; i++)
	{
		pDmgTile = &m_dmgTileInfo[i];
		if (m_bitsDamage & pDmgTile->fFlags)
		{
			if (pDmgTile->icon)
			{
				(pDmgTile->icon)->DrawSelf(pDmgTile->x, pDmgTile->y, clrTile);
			}
		}
	}

	// check for bits that should be expired
	for (i = 0; i < NUM_DMG_TYPES; i++)
	{
		pDmgTile = &m_dmgTileInfo[i];

		if (m_bitsDamage & pDmgTile->fFlags)
		{
			pDmgTile->flExpire = min(CURTIME + DMG_IMAGE_LIFE, pDmgTile->flExpire);

			if (pDmgTile->flExpire <= CURTIME		// when the time has expired
				&& a < 40)										// and the flash is at the low point of the cycle
			{
				pDmgTile->flExpire = 0;

				int x = pDmgTile->x;
				pDmgTile->x = 0;
				pDmgTile->y = 0;

				m_bitsDamage &= ~pDmgTile->fFlags;  // clear the bits

													// move everyone else to the left
				for (int j = 0; j < NUM_DMG_TYPES; j++)
				{
					damagetile_t *pDmgTileIter = &m_dmgTileInfo[j];
					if ((pDmgTileIter->x) && (pDmgTileIter->x < x) && (pDmgTileIter->icon))
						pDmgTileIter->x -= (pDmgTileIter->icon)->Width();
				}
			}
		}
	}
}

void CHudDamageTiles::UpdateTiles(long bitsDamage)
{
	damagetile_t *pDmgTile;

	// Which types are new?
	long bitsOn = ~m_bitsDamage & bitsDamage;

	for (int i = 0; i < NUM_DMG_TYPES; i++)
	{
		pDmgTile = &m_dmgTileInfo[i];

		// Is this one already on?
		if (m_bitsDamage & pDmgTile->fFlags)
		{
			pDmgTile->flExpire = CURTIME + DMG_IMAGE_LIFE; // extend the duration
			if (!pDmgTile->flBaseline)
			{
				pDmgTile->flBaseline = CURTIME;
			}
		}

		// Are we just turning it on?
		if (bitsOn & pDmgTile->fFlags)
		{
			// put this one at the bottom
			if (pDmgTile->icon)
			{
				pDmgTile->x = (pDmgTile->icon)->Width() / 4;
				pDmgTile->y = (pDmgTile->icon)->Height() / 4;
			}
			pDmgTile->flExpire = CURTIME + DMG_IMAGE_LIFE;

			// move everyone else to the right
			for (int j = 0; j < NUM_DMG_TYPES; j++)
			{
				if (j == i)
					continue;

				pDmgTile = &m_dmgTileInfo[j];
				if (pDmgTile->x && pDmgTile->icon)
					pDmgTile->x += (pDmgTile->icon)->Width();

			}
			pDmgTile = &m_dmgTileInfo[i];
		}
	}

	// damage bits are only turned on here;  they are turned off when the draw time has expired (in Paint())
	m_bitsDamage |= bitsDamage;
}

void CHudDamageTiles::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);
}

class CHudRadialDamage : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudRadialDamage, vgui::Panel);
public:
	CHudRadialDamage(const char *pElementName);
	void Init(void);
	void VidInit(void);
	void Reset(void);
	virtual bool ShouldDraw(void);

	// Handler for our message
	void MsgFunc_Damage(bf_read &msg);

private:
	virtual void OnThink();
	virtual void Paint();
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	// Painting
	void GetDamagePosition(const Vector &vecDelta, float flRadius, float *xpos, float *ypos, float *flRotation);
	void DrawDamageIndicator(int x0, int y0, int x1, int y1, float alpha, float flRotation);

private:
	// Indication times

	CPanelAnimationVarAliasType(float, m_flMinimumWidth, "MinimumWidth", "28", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flMaximumWidth, "MaximumWidth", "32", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flMinimumHeight, "MinimumHeight", "28", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flMaximumHeight, "MaximumHeight", "32", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flStartRadius, "StartRadius", "36", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flEndRadius, "EndRadius", "40", "proportional_float");

	CPanelAnimationVar(float, m_iDmgMarkerScale, "DamageMarkerScale", "2");
	CPanelAnimationVar(float, m_flMinimumTime, "MinimumTime", "1");
	CPanelAnimationVar(float, m_flMaximumTime, "MaximumTime", "2");
	CPanelAnimationVar(float, m_flTravelTime, "TravelTime", ".1");
	CPanelAnimationVar(float, m_flFadeOutPercentage, "FadeOutPercentage", "0.7");
	CPanelAnimationVar(float, m_flNoise, "Noise", "0.1");

	// List of damages we've taken
	struct damage_t
	{
		int		iScale;
		float	flLifeTime;
		float	flStartTime;
		Vector	vecDelta;	// Damage origin relative to the player
		Vector	vecWorldPos; // original position from where the damage was dealt
	};
	CUtlVector<damage_t>	m_vecDamages;

	CMaterialReference m_WhiteAdditiveMaterial;
};

DECLARE_HUDELEMENT(CHudRadialDamage);
DECLARE_HUD_MESSAGE(CHudRadialDamage, Damage);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CHudRadialDamage::CHudRadialDamage(const char *pElementName) :
	CHudElement(pElementName), BaseClass(NULL, "HudRadialDamage")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_HEALTH);

	m_WhiteAdditiveMaterial.Init("sprites/radial_damage", TEXTURE_GROUP_VGUI); // ("VGUI/damageindicator", TEXTURE_GROUP_VGUI);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudRadialDamage::Init(void)
{
	HOOK_HUD_MESSAGE(CHudRadialDamage, Damage);
	Reset();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudRadialDamage::Reset(void)
{
	m_vecDamages.Purge();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudRadialDamage::VidInit(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudRadialDamage::OnThink()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
bool CHudRadialDamage::ShouldDraw(void)
{
	if (!CHudElement::ShouldDraw())
		return false;

	// Don't draw if we don't have any damage to indicate
	if (!m_vecDamages.Count())
		return false;
	
	// If player opted not to, don't draw
	if (!hud_show_damage_direction.GetBool())
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Convert a damage position in world units to the screen's units
//-----------------------------------------------------------------------------
void CHudRadialDamage::GetDamagePosition(const Vector &vecDelta, float flRadius, float *xpos, float *ypos, float *flRotation)
{
	// Player Data
	Vector playerPosition = MainViewOrigin();
	QAngle playerAngles = MainViewAngles();

	Vector forward, right, up(0, 0, 1);
	AngleVectors(playerAngles, &forward, &right, NULL);
	forward.z = 0;
	VectorNormalize(forward);
	VectorNormalize(right);
	CrossProduct(up, forward, right);
	float front = DotProduct(vecDelta, forward);
	float side = DotProduct(vecDelta, right);
	*xpos = flRadius * -side;
	*ypos = flRadius * -front;

	// Get the rotation (yaw)
	*flRotation = atan2(*xpos, *ypos) + M_PI;
	*flRotation *= 180 / M_PI;

	float yawRadians = -(*flRotation) * M_PI / 180.0f;
	float ca = cos(yawRadians);
	float sa = sin(yawRadians);

	// Rotate it around the circle
	*xpos = (int)((ScreenWidth() / 2) + (flRadius * sa));
	*ypos = (int)((ScreenHeight() / 2) - (flRadius * ca));
}

//-----------------------------------------------------------------------------
// Purpose: Draw a single damage indicator
//-----------------------------------------------------------------------------
void CHudRadialDamage::DrawDamageIndicator(int x0, int y0, int x1, int y1, float alpha, float flRotation)
{
	CMatRenderContextPtr pRenderContext(materials);
	IMesh *pMesh = pRenderContext->GetDynamicMesh(true, NULL, NULL, m_WhiteAdditiveMaterial);


	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	if (false) //(pPlayer->IsInAVehicle())
	{
		int iSize = m_vecDamages.Count();
		for (int i = iSize - 1; i >= 0; i--)
		{
			int xpos, ypos;
			float xpos1, ypos1;
			GetVectorInScreenSpace(m_vecDamages[i].vecWorldPos, xpos, ypos);
			xpos1 = (float)(xpos / ScreenWidth());
			ypos1 = (float)(ypos / ScreenHeight());
		//	xpos = 0; ypos = 0.2;
			NDebugOverlay::ScreenText(xpos1, ypos1, "[ ]", 255, 50, 50, 200, 1.0f);
			
			CMeshBuilder meshBuilder;
			meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);

			int iAlpha = alpha * hud_transparency.GetInt();
			meshBuilder.Color4ub(255, 255, 255, iAlpha);
			meshBuilder.TexCoord2f(0, 0, 0);
			meshBuilder.Position3f(xpos - 0.1f, ypos - 0.1f, 0);
			meshBuilder.AdvanceVertex();

			meshBuilder.Color4ub(255, 255, 255, iAlpha);
			meshBuilder.TexCoord2f(0, 1, 0);
			meshBuilder.Position3f(xpos - 0.1f, ypos + 0.1f, 0);
			meshBuilder.AdvanceVertex();

			meshBuilder.Color4ub(255, 255, 255, iAlpha);
			meshBuilder.TexCoord2f(0, 1, 1);
			meshBuilder.Position3f(xpos + 0.1f, ypos + 0.1f, 0);
			meshBuilder.AdvanceVertex();

			meshBuilder.Color4ub(255, 255, 255, iAlpha);
			meshBuilder.TexCoord2f(0, 0, 1);
			meshBuilder.Position3f(xpos + 0.1f, ypos - 0.1f, 0);
			meshBuilder.AdvanceVertex();

			meshBuilder.End();
			pMesh->Draw();

		//	surface()->DrawFilledRectFade(xpos - 8, ypos - 8, xpos + 8, ypos + 8, 100, 0, false);
		}
	}
	else
	{
		// Get the corners, since they're being rotated
		int wide = x1 - x0;
		int tall = y1 - y0;
		Vector2D vecCorners[4];
		Vector2D center(x0 + (wide * 0.5f), y0 + (tall * 0.5f));
		float yawRadians = -flRotation * M_PI / 180.0f;
		Vector2D axis[2];
		axis[0].x = cos(yawRadians);
		axis[0].y = sin(yawRadians);
		axis[1].x = -axis[0].y;
		axis[1].y = axis[0].x;
		Vector2DMA(center, -0.5f * wide, axis[0], vecCorners[0]);
		Vector2DMA(vecCorners[0], -0.5f * tall, axis[1], vecCorners[0]);
		Vector2DMA(vecCorners[0], wide, axis[0], vecCorners[1]);
		Vector2DMA(vecCorners[1], tall, axis[1], vecCorners[2]);
		Vector2DMA(vecCorners[0], tall, axis[1], vecCorners[3]);

		// Draw the sucker
		CMeshBuilder meshBuilder;
		meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);

		int iAlpha = alpha * 255;
		meshBuilder.Color4ub(255, 255, 255, iAlpha);
		meshBuilder.TexCoord2f(0, 0, 0);
		meshBuilder.Position3f(vecCorners[0].x, vecCorners[0].y, 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub(255, 255, 255, iAlpha);
		meshBuilder.TexCoord2f(0, 1, 0);
		meshBuilder.Position3f(vecCorners[1].x, vecCorners[1].y, 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub(255, 255, 255, iAlpha);
		meshBuilder.TexCoord2f(0, 1, 1);
		meshBuilder.Position3f(vecCorners[2].x, vecCorners[2].y, 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.Color4ub(255, 255, 255, iAlpha);
		meshBuilder.TexCoord2f(0, 0, 1);
		meshBuilder.Position3f(vecCorners[3].x, vecCorners[3].y, 0);
		meshBuilder.AdvanceVertex();

		meshBuilder.End();
		pMesh->Draw();
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudRadialDamage::Paint()
{
	// Iterate backwards, because we might remove them as we go
	int iSize = m_vecDamages.Count();
	for (int i = iSize - 1; i >= 0; i--)
	{
		// Scale size to the damage
		int clampedDamage = m_iDmgMarkerScale;

		int iWidth = RemapVal(clampedDamage, 0, m_iDmgMarkerScale, m_flMinimumWidth, m_flMaximumWidth) * 0.5;
		int iHeight = RemapVal(clampedDamage, 0, m_iDmgMarkerScale, m_flMinimumHeight, m_flMaximumHeight) * 0.5;

		// Find the place to draw it
		float xpos, ypos;
		float flRotation;
		float flTimeSinceStart = (CURTIME - m_vecDamages[i].flStartTime);
		float flRadius = RemapVal(min(flTimeSinceStart, m_flTravelTime), 0, m_flTravelTime, m_flStartRadius, m_flEndRadius);
		GetDamagePosition(m_vecDamages[i].vecDelta, flRadius, &xpos, &ypos, &flRotation);

		// Calculate life left
		float flLifeLeft = (m_vecDamages[i].flLifeTime - CURTIME);
		if (flLifeLeft > 0)
		{
			float flPercent = flTimeSinceStart / (m_vecDamages[i].flLifeTime - m_vecDamages[i].flStartTime);
			float alpha;
			if (flPercent <= m_flFadeOutPercentage)
			{
				alpha = 1.0;
			}
			else
			{
				alpha = 1.0 - RemapVal(flPercent, m_flFadeOutPercentage, 1.0, 0.0, 1.0);
			}
			DrawDamageIndicator(xpos - iWidth, ypos - iHeight, xpos + iWidth, ypos + iHeight, alpha, flRotation);
		}
		else
		{
			m_vecDamages.Remove(i);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Message handler for Damage message
//-----------------------------------------------------------------------------
void CHudRadialDamage::MsgFunc_Damage(bf_read &msg)
{
	int armor = msg.ReadByte();	// armor
	int damageTaken = msg.ReadByte();	// health
	long bitsDamage = msg.ReadLong(); // damage bits

	// ignore these kinds of damage
	if (bitsDamage & DMG_DROWN) return;
	if (bitsDamage & DMG_FALL) return;
	if (bitsDamage & DMG_RADIATION) return;
	if (bitsDamage & DMG_DROWNRECOVER) return;

	damage_t damage;
	damage.iScale = damageTaken + armor;
	if (damage.iScale > m_iDmgMarkerScale)
	{
		damage.iScale = m_iDmgMarkerScale;
	}
	Vector vecOrigin;

	vecOrigin.x = msg.ReadFloat();
	vecOrigin.y = msg.ReadFloat();
	vecOrigin.z = msg.ReadFloat();

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	// ignore damage without direction
	if (vecOrigin == vec3_origin)
		return;

	// ignore 0 damage, this can happen to time-based damages and show the indicator some time after taking damage
	if (damageTaken <= 0)
		return;

	damage.flStartTime = CURTIME;
	damage.flLifeTime = CURTIME + RemapVal(damage.iScale, 0, m_iDmgMarkerScale, m_flMinimumTime, m_flMaximumTime);

	if (vecOrigin == vec3_origin)
	{
		vecOrigin = MainViewOrigin();
	}

	damage.vecWorldPos = vecOrigin;
	damage.vecDelta = (vecOrigin - MainViewOrigin());
	VectorNormalize(damage.vecDelta);

	// Add some noise
	damage.vecDelta[0] += random->RandomFloat(-m_flNoise, m_flNoise);
	damage.vecDelta[1] += random->RandomFloat(-m_flNoise, m_flNoise);
	damage.vecDelta[2] += random->RandomFloat(-m_flNoise, m_flNoise);
	VectorNormalize(damage.vecDelta);

	m_vecDamages.AddToTail(damage);
}

//-----------------------------------------------------------------------------
// Purpose: hud scheme settings
//-----------------------------------------------------------------------------
void CHudRadialDamage::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);

	// set our size
	int screenWide, screenTall;
	int x, y;
	GetPos(x, y);
	GetHudSize(screenWide, screenTall);
	SetBounds(0, y, screenWide, screenTall - y);
}

//-----------------------------------------------------------------------------
// Purpose: Radio Transmission HUD Panel
//-----------------------------------------------------------------------------
class CHudSuitRadio : public vgui::Panel, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudSuitRadio, vgui::Panel);

public:
	CHudSuitRadio(const char *pElementName);

	void Init();
	void Reset();
	void MsgFunc_SuitRadio(bf_read &msg);
	void FireGameEvent(IGameEvent * event);
	bool ShouldDraw();
	void Paint();

	void LocalizeAndDisplay(const char *pszHudTxtMsg, const char *szRawString);

	virtual void PerformLayout();

protected:
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);
	virtual void OnThink();

	string_t m_hud_label_string;
protected:
	vgui::HFont m_hFont;
	Color		m_bgColor;
	vgui::Label *m_pLabel;
	float m_hud_lifetime_float;

private:
	CHudTexture		*radio_border;
	CHudTexture		*radio_graph;
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HudSuitRadio");
	CPanelAnimationVar(Color, m_TextColor, "TextColor", "FgColor");
	CPanelAnimationVarAliasType(float, border_xpos, "border_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, border_ypos, "border_ypos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, graph_xpos, "graph_xpos", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, graph_ypos, "graph_ypos", "2", "proportional_float");
	CPanelAnimationVarAliasType(float, text_xpos, "text_xpos", "0", "proportional_float");
	CPanelAnimationVarAliasType(float, text_ypos, "text_ypos", "0", "proportional_float");
//	CPanelAnimationVarAliasType(float, text_ygap, "text_ygap", "14", "proportional_float");
};

DECLARE_HUDELEMENT(CHudSuitRadio);
DECLARE_HUD_MESSAGE(CHudSuitRadio, SuitRadio);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudSuitRadio::CHudSuitRadio(const char *pElementName) : BaseClass(NULL, "HudSuitRadio"), CHudElement(pElementName)
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	SetVisible(false);
	m_pLabel = new vgui::Label(this, "HudSuitRadioLabel", "");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSuitRadio::Init()
{
	HOOK_HUD_MESSAGE(CHudSuitRadio, SuitRadio);
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSuitRadio::Reset()
{
	m_hud_lifetime_float = 0;
//	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HudRadioHide");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudSuitRadio::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);

	SetFgColor(GetSchemeColor("HintMessageFg", pScheme));
//	m_hFont = pScheme->GetFont("HudHintText", true);
	if (m_pLabel)
	{
	//	int a;
	//	a = hud_transparency.GetInt();
		m_pLabel->SetPinCorner(PIN_TOPLEFT, 0, 0);
		m_pLabel->SetBgColor((Color(m_TextColor[0], m_TextColor[1], m_TextColor[2], 0)));
		m_pLabel->SetFgColor((Color(m_TextColor[0], m_TextColor[1], m_TextColor[2], 0)));
		m_pLabel->SetPaintBackgroundType(2);
		m_pLabel->SetPaintBackgroundEnabled(false);
		m_pLabel->SetPos(text_xpos, text_ypos);
		m_pLabel->SetSize(GetWide(), GetTall() / 2);		// Start tiny, it'll grow.
		m_pLabel->SetFont(m_hTextFont);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Resizes the label
//-----------------------------------------------------------------------------
void CHudSuitRadio::PerformLayout()
{
	BaseClass::PerformLayout();
}

//-----------------------------------------------------------------------------
// Purpose: Updates the label color each frame
//-----------------------------------------------------------------------------
void CHudSuitRadio::OnThink()
{
	if (CURTIME > m_hud_lifetime_float)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HudRadioHide");
	}

	if (m_pLabel)
	{
		m_pLabel->SetBgColor(GetBgColor());
		m_pLabel->SetFgColor(GetFgColor());
		m_pLabel->SetAlpha(GetAlpha() * (hud_transparency.GetInt() / 255.0f));
	}
	
	SetPaintBackgroundType(GetAlpha() == 0 ? 0 : 2);
	SetPaintBackgroundEnabled(GetAlpha() > 0);
}

bool CHudSuitRadio::ShouldDraw(void)
{
	if (!CHudElement::ShouldDraw())
		return false;

	if ( GetAlpha() <= 0)
		return false;

	if (m_hud_lifetime_float == 0)
		return false;

	return true;
}

void CHudSuitRadio::Paint()
{
	BaseClass::Paint();

	SetAlpha(hud_transparency.GetInt());

	if (!radio_border)
	{
		radio_border = gHUD.GetIcon("radio_border");
	}

	if (!radio_graph)
	{
		radio_graph = gHUD.GetIcon("radio_graph");
	}

	if (!radio_border || !radio_graph)
	{
		return;
	}

	int a;
	a = GetAlpha(); //hud_transparency.GetInt();

	radio_border->DrawSelf(border_xpos, border_ypos, GetFgColor());//Color(m_TextColor[0], m_TextColor[1], m_TextColor[2], a));
	radio_graph->DrawSelf(graph_xpos, graph_ypos, GetFgColor());//Color(m_TextColor[0], m_TextColor[1], m_TextColor[2], a));
}


//-----------------------------------------------------------------------------
// Purpose: Activates the hint display
//-----------------------------------------------------------------------------
void CHudSuitRadio::MsgFunc_SuitRadio(bf_read &msg)
{
	C_BasePlayer *pLocalPlayer = C_BasePlayer::GetLocalPlayer();
	if (pLocalPlayer)
	{
		// Read the string(s)
		char szString[255];
		float lifetime;
		msg.ReadByte();
		lifetime = msg.ReadFloat();
		msg.ReadString(szString, sizeof(szString));
		if (lifetime > 0)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HudRadioShow");
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HudRadioHide");
		}
		if (m_pLabel)
		{
			m_pLabel->SetText(g_pVGuiLocalize->Find(szString));
		}
		m_hud_lifetime_float = (szString == NULL_STRING) ? 0 : CURTIME + lifetime;

		pLocalPlayer->EmitSound("Hud.Hint");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Activates the hint display upon receiving a hint
//-----------------------------------------------------------------------------
void CHudSuitRadio::FireGameEvent(IGameEvent * event)
{
}

//-----------------------------------------------------------------------------
// Purpose: Localize, display, and animate the hud element
//-----------------------------------------------------------------------------
void CHudSuitRadio::LocalizeAndDisplay(const char *pszHudTxtMsg, const char *szRawString)
{
	// make it visible
	if (true)
	{
		SetVisible(true);
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HudRadioShow");
	}
	else
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HudRadioHide");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Displays current ammunition level
//-----------------------------------------------------------------------------
class CHudAmmo : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudAmmo, CHudNumericDisplay);

public:
	CHudAmmo(const char *pElementName);
	void Init(void);
	void VidInit(void);
	void Reset();

	void SetAmmo(int ammo, bool playAnimation);
	void SetAmmo2(int ammo2, bool playAnimation);
	virtual void Paint(void);

protected:
	virtual void OnThink();

	void UpdateAmmoDisplays();
	void UpdatePlayerAmmo(C_BasePlayer *player);
	void UpdateVehicleAmmo(C_BasePlayer *player, IClientVehicle *pVehicle);

private:
	CHandle< C_BaseCombatWeapon > m_hCurrentActiveWeapon;
	CHandle< C_BaseEntity > m_hCurrentVehicle;
	int		m_iAmmo;
	int		m_iAmmo2;
	CHudTexture *m_iconPrimaryAmmo;
};

DECLARE_HUDELEMENT(CHudAmmo);

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudAmmo::CHudAmmo(const char *pElementName) : BaseClass(NULL, "HudAmmo"), CHudElement(pElementName)
{
	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT | HIDEHUD_WEAPONSELECTION);

	hudlcd->SetGlobalStat("(ammo_primary)", "0");
	hudlcd->SetGlobalStat("(ammo_secondary)", "0");
	hudlcd->SetGlobalStat("(weapon_print_name)", "");
	hudlcd->SetGlobalStat("(weapon_name)", "");
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudAmmo::Init(void)
{
	m_iAmmo = -1;
	m_iAmmo2 = -1;

	m_iconPrimaryAmmo = NULL;
#ifdef DARKINTERVAL
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
#endif
	wchar_t *tempString = g_pVGuiLocalize->Find("#Valve_Hud_AMMO");

	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"AMMO");
	}
#ifdef DARKINTERVAL // like in retail build, there's no "AMMO" label
	if (player && !player->IsSuitEquipped()) SetLabelText(L" ");  //tempString = g_pVGuiLocalize->Find("#DI_NullString");
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudAmmo::VidInit(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: Resets hud after save/restore
//-----------------------------------------------------------------------------
void CHudAmmo::Reset()
{
	BaseClass::Reset();

	m_hCurrentActiveWeapon = NULL;
	m_hCurrentVehicle = NULL;
	m_iAmmo = 0;
	m_iAmmo2 = 0;

	UpdateAmmoDisplays();
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get ammo info from the weapon
//-----------------------------------------------------------------------------
void CHudAmmo::UpdatePlayerAmmo(C_BasePlayer *player)
{
	// Clear out the vehicle entity
	m_hCurrentVehicle = NULL;

	C_BaseCombatWeapon *wpn = GetActiveWeapon();

	hudlcd->SetGlobalStat("(weapon_print_name)", wpn ? wpn->GetPrintName() : " ");
	hudlcd->SetGlobalStat("(weapon_name)", wpn ? wpn->GetName() : " ");

	if (!wpn || !player || !wpn->UsesPrimaryAmmo())
	{
		hudlcd->SetGlobalStat("(ammo_primary)", "n/a");
		hudlcd->SetGlobalStat("(ammo_secondary)", "n/a");

		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		return;
	}
#ifdef DARKINTERVAL
	if (!player->IsSuitEquipped())
	{
		hudlcd->SetGlobalStat("(ammo_primary)", "n/a");
	}
#endif
	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(
#ifdef DARKINTERVAL
		player->IsSuitEquipped()
#else
		true
#endif
	);

	// Get our icons for the ammo types
#ifdef DARKINTERVAL
	if (player->IsSuitEquipped())
#endif
	{
		m_iconPrimaryAmmo = gWR.GetAmmoIconFromWeapon(wpn->GetPrimaryAmmoType());
	}
	// get the ammo in our clip
	int ammo1 = wpn->Clip1();
	int ammo2;
	if (ammo1 < 0)
	{
		// we don't use clip ammo, just use the total ammo count
		ammo1 = player->GetAmmoCount(wpn->GetPrimaryAmmoType());
		ammo2 = 0;
	}
	else
	{
		// we use clip ammo, so the second ammo is the total ammo
		ammo2 = player->GetAmmoCount(wpn->GetPrimaryAmmoType());
	}

	hudlcd->SetGlobalStat("(ammo_primary)", VarArgs("%d", ammo1));
	hudlcd->SetGlobalStat("(ammo_secondary)", VarArgs("%d", ammo2));

	if (wpn == m_hCurrentActiveWeapon)
	{
		// same weapon, just update counts
		SetAmmo(ammo1, true);
		SetAmmo2(ammo2, true);
	}
	else
	{
		// diferent weapon, change without triggering
		SetAmmo(ammo1, false);
		SetAmmo2(ammo2, false);

		// update whether or not we show the total ammo display
		if (wpn->UsesClipsForAmmo1())
		{
			SetShouldDisplaySecondaryValue(true);
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponUsesClips");
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponDoesNotUseClips");
			SetShouldDisplaySecondaryValue(false);
		}

		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponChanged");
		m_hCurrentActiveWeapon = wpn;
	}
#ifdef DARKINTERVAL
	if (!player->IsSuitEquipped()) g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoEmpty");
#endif
}

void CHudAmmo::UpdateVehicleAmmo(C_BasePlayer *player, IClientVehicle *pVehicle)
{
	m_hCurrentActiveWeapon = NULL;
	CBaseEntity *pVehicleEnt = pVehicle->GetVehicleEnt();

	if (!pVehicleEnt || pVehicle->GetPrimaryAmmoType() < 0)
	{
		SetPaintEnabled(false);
		SetPaintBackgroundEnabled(false);
		return;
	}

	SetPaintEnabled(true);
	SetPaintBackgroundEnabled(true);

	// get the ammo in our clip
	int ammo1 = pVehicle->GetPrimaryAmmoClip();
	int ammo2;
	if (ammo1 < 0)
	{
		// we don't use clip ammo, just use the total ammo count
		ammo1 = pVehicle->GetPrimaryAmmoCount();
		ammo2 = 0;
	}
	else
	{
		// we use clip ammo, so the second ammo is the total ammo
		ammo2 = pVehicle->GetPrimaryAmmoCount();
	}

	if (pVehicleEnt == m_hCurrentVehicle)
	{
		// same weapon, just update counts
		SetAmmo(ammo1, true);
		SetAmmo2(ammo2, true);
	}
	else
	{
		// diferent weapon, change without triggering
		SetAmmo(ammo1, false);
		SetAmmo2(ammo2, false);

		// update whether or not we show the total ammo display
		if (pVehicle->PrimaryAmmoUsesClips())
		{
			SetShouldDisplaySecondaryValue(true);
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponUsesClips");
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponDoesNotUseClips");
			SetShouldDisplaySecondaryValue(false);
		}

		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponChanged");
		m_hCurrentVehicle = pVehicleEnt;
	}
}

//-----------------------------------------------------------------------------
// Purpose: called every frame to get ammo info from the weapon
//-----------------------------------------------------------------------------
void CHudAmmo::OnThink()
{
	UpdateAmmoDisplays();
}

//-----------------------------------------------------------------------------
// Purpose: updates the ammo display counts
//-----------------------------------------------------------------------------
void CHudAmmo::UpdateAmmoDisplays()
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	IClientVehicle *pVehicle = player ? player->GetVehicle() : NULL;

	if (!pVehicle)
	{
		UpdatePlayerAmmo(player);
	}
	else
	{
		UpdateVehicleAmmo(player, pVehicle);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Updates ammo display
//-----------------------------------------------------------------------------
void CHudAmmo::SetAmmo(int ammo, bool playAnimation)
{
	if (ammo != m_iAmmo)
	{
		if (ammo == 0)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoEmpty");
		}
		else if (ammo < m_iAmmo)
		{
			// ammo has decreased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoDecreased");
		}
		else
		{
			// ammunition has increased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoIncreased");
		}

		m_iAmmo = ammo;
	}

	SetDisplayValue(ammo);
}

//-----------------------------------------------------------------------------
// Purpose: Updates 2nd ammo display
//-----------------------------------------------------------------------------
void CHudAmmo::SetAmmo2(int ammo2, bool playAnimation)
{
	if (ammo2 != m_iAmmo2)
	{
		if (ammo2 == 0)
		{
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Ammo2Empty");
		}
		else if (ammo2 < m_iAmmo2)
		{
			// ammo has decreased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Ammo2Decreased");
		}
		else
		{
			// ammunition has increased
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("Ammo2Increased");
		}

		m_iAmmo2 = ammo2;
	}

	SetSecondaryValue(ammo2);
}

//-----------------------------------------------------------------------------
// Purpose: We add an icon into the 
//-----------------------------------------------------------------------------
void CHudAmmo::Paint(void)
{
	BaseClass::Paint();

	if (m_hCurrentVehicle == NULL && m_iconPrimaryAmmo)
	{
		int nLabelHeight;
		int nLabelWidth;
		surface()->GetTextSize(m_hTextFont, m_LabelText, nLabelWidth, nLabelHeight);

		// Figure out where we're going to put this
		int x = text_xpos + (nLabelWidth - m_iconPrimaryAmmo->Width()) / 2;
		int y = text_ypos - (nLabelHeight + (m_iconPrimaryAmmo->Height() / 2));

		m_iconPrimaryAmmo->DrawSelf(x, y, GetFgColor());
	}
#ifdef DARKINTERVAL
	int alpha = hud_transparency.GetInt();
	SetAlpha(alpha);
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Displays the secondary ammunition level
//-----------------------------------------------------------------------------
class CHudSecondaryAmmo : public CHudNumericDisplay, public CHudElement
{
	DECLARE_CLASS_SIMPLE(CHudSecondaryAmmo, CHudNumericDisplay);

public:
	CHudSecondaryAmmo(const char *pElementName) : BaseClass(NULL, "HudAmmoSecondary"), CHudElement(pElementName)
	{
		m_iAmmo = -1;

		SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_WEAPONSELECTION | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT 
#ifdef DARKINTERVAL
			| HIDEHUD_INVEHICLE
#endif
		);
	}

	void Init(void)
	{
		C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
		wchar_t *tempString = g_pVGuiLocalize->Find("#Valve_Hud_AMMO_ALT");
		if (tempString)
		{
			SetLabelText(tempString);
		}
		else
		{
			SetLabelText(L"ALT");
		}
#ifdef DARKINTERVAL // like the retail build, there's no label
		if (player && !player->IsSuitEquipped()) SetLabelText(L" ");  //tempString = g_pVGuiLocalize->Find("#DI_NullString");
#endif
	}

	void SetAmmo(int ammo)
	{
		if (ammo != m_iAmmo)
		{
			if (ammo == 0)
			{
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoSecondaryEmpty");
			}
			else if (ammo < m_iAmmo)
			{
				// ammo has decreased
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoSecondaryDecreased");
			}
			else
			{
				// ammunition has increased
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("AmmoSecondaryIncreased");
			}

			m_iAmmo = ammo;
		}
		SetDisplayValue(ammo);
	}

	void Reset()
	{
		// hud reset, update ammo state
		BaseClass::Reset();
		m_iAmmo = 0;
		m_hCurrentActiveWeapon = NULL;
		SetAlpha(0);
		UpdateAmmoState();
	}
#ifdef DARKINTERVAL
	virtual bool ShouldDraw(void)
	{
		bool bNeedsDraw = false;
		C_BasePlayer * local = C_BasePlayer::GetLocalPlayer();
		if (local)
		{
			//	if (GetAlpha() <= 0) bNeedsDraw = false;
			C_BaseCombatWeapon *wpn = GetActiveWeapon();
			if (wpn && wpn->UsesSecondaryAmmo()) bNeedsDraw = true;
		}
		return (bNeedsDraw && CHudElement::ShouldDraw());
	}
#endif
	virtual void Paint(void)
	{
#ifdef DARKINTERVAL
		if (!ShouldDraw())
			return;
#endif
		BaseClass::Paint();
#ifndef DARKINTERVAL
		if (m_iconSecondaryAmmo)
		{
		int nLabelHeight;
		int nLabelWidth;
		surface()->GetTextSize(m_hTextFont, m_LabelText, nLabelWidth, nLabelHeight);

		// Figure out where we're going to put this
		int x = text_xpos + (nLabelWidth - m_iconSecondaryAmmo->Width()) / 2;
		int y = text_ypos - (nLabelHeight + (m_iconSecondaryAmmo->Height() / 2));

		m_iconSecondaryAmmo->DrawSelf(x, y, GetFgColor());
		}
#endif
#ifdef DARKINTERVAL
		int alpha = hud_transparency.GetInt();
		SetAlpha(alpha);
#endif
	}

protected:

	virtual void OnThink()
	{
#ifndef DARKINTERVAL
		// set whether or not the panel draws based on if we have a weapon that supports secondary ammo
		C_BaseCombatWeapon *wpn = GetActiveWeapon();
		C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
		IClientVehicle *pVehicle = player ? player->GetVehicle() : NULL;
		if (!wpn || !player || pVehicle)
		{
			m_hCurrentActiveWeapon = NULL;
			SetPaintEnabled(false);
			SetPaintBackgroundEnabled(false);
			return;
		}
		else
		{
			SetPaintEnabled(true);
			SetPaintBackgroundEnabled(true);
		}
#endif
		UpdateAmmoState();
	}

	void UpdateAmmoState()
	{
		C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
		C_BaseCombatWeapon *wpn = GetActiveWeapon();
#ifdef DARKINTERVAL
		if (!player) return;
		if (!wpn) return;
#endif
		if (player && wpn && wpn->UsesSecondaryAmmo())
		{
			SetAmmo(player->GetAmmoCount(wpn->GetSecondaryAmmoType())); 
#ifdef DARKINTERVAL
			SetPaintEnabled(true);
			SetPaintBackgroundEnabled(true);
#endif
		}
		else
		{
			SetPaintEnabled(false);
			SetPaintBackgroundEnabled(false);
		}

		if (m_hCurrentActiveWeapon != wpn)
		{
			if (wpn->UsesSecondaryAmmo())
			{
				// we've changed to a weapon that uses secondary ammo
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponUsesSecondaryAmmo");
			}
			else
			{
				// we've changed away from a weapon that uses secondary ammo
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponDoesNotUseSecondaryAmmo");
			}
			m_hCurrentActiveWeapon = wpn;

			// Get the icon we should be displaying
#ifndef DARKINTERVAL
			m_iconSecondaryAmmo = wpn->UsesSecondaryAmmo() ? NULL : gWR.GetAmmoIconFromWeapon(m_hCurrentActiveWeapon->GetSecondaryAmmoType());
#endif
		}
	}

private:
	CHandle< C_BaseCombatWeapon > m_hCurrentActiveWeapon;
#ifdef DARKINTERVAL
	CHudTexture *m_iconSecondaryAmmo; // DI doesn't use it
#endif
	int		m_iAmmo;
};

DECLARE_HUDELEMENT(CHudSecondaryAmmo);

//-----------------------------------------------------------------------------
// Purpose: Displays the quickinfo panels around the crosshair
//-----------------------------------------------------------------------------

#define	HEALTH_WARNING_THRESHOLD	25

static ConVar	hud_quickinfo("hud_quickinfo", "1", FCVAR_ARCHIVE);

extern ConVar crosshair;

#define QUICKINFO_EVENT_DURATION	1.0f
#define	QUICKINFO_BRIGHTNESS_FULL	255
#define	QUICKINFO_BRIGHTNESS_DIM	64
#define	QUICKINFO_FADE_IN_TIME		0.5f
#define QUICKINFO_FADE_OUT_TIME		2.0f

class CHUDQuickInfo : public CHudElement, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHUDQuickInfo, vgui::Panel);
public:
	CHUDQuickInfo(const char *pElementName);
	void Init(void);
	void VidInit(void);
	bool ShouldDraw(void);
	virtual void OnThink();
	virtual void Paint();

	virtual void ApplySchemeSettings(IScheme *scheme);
private:

	void	DrawWarning(int x, int y, CHudTexture *icon, float &time);
	void	UpdateEventTime(void);
	bool	EventTimeElapsed(void);

	int		m_lastAmmo;
	int		m_lastHealth;

	float	m_ammoFade;
	float	m_healthFade;

	bool	m_warnAmmo;
	bool	m_warnHealth;

	bool	m_bFadedOut;

	bool	m_bDimmed;			// Whether or not we are dimmed down
	float	m_flLastEventTime;	// Last active event (controls dimmed state)

	CHudTexture	*m_icon_c;

	CHudTexture	*m_icon_rbn;	// right bracket
	CHudTexture	*m_icon_lbn;	// left bracket

	CHudTexture	*m_icon_rb;		// right bracket, full
	CHudTexture	*m_icon_lb;		// left bracket, full
	CHudTexture	*m_icon_rbe;	// right bracket, empty
	CHudTexture	*m_icon_lbe;	// left bracket, empty
#ifdef DARKINTERVAL // for worker suit, grey HUD
	CPanelAnimationVar(Color, m_HullColor, "HullColor", "150 150 165 200");
#endif
};

DECLARE_HUDELEMENT(CHUDQuickInfo);

CHUDQuickInfo::CHUDQuickInfo(const char *pElementName) :
	CHudElement(pElementName), BaseClass(NULL, "HUDQuickInfo")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);

	SetHiddenBits(HIDEHUD_CROSSHAIR);
}

void CHUDQuickInfo::ApplySchemeSettings(IScheme *scheme)
{
	BaseClass::ApplySchemeSettings(scheme);

	SetPaintBackgroundEnabled(false);
	SetForceStereoRenderToFrameBuffer(true);
}

void CHUDQuickInfo::Init(void)
{
	m_ammoFade = 0.0f;
	m_healthFade = 0.0f;

	m_lastAmmo = 0;
	m_lastHealth = 100;

	m_warnAmmo = false;
	m_warnHealth = false;

	m_bFadedOut = false;
	m_bDimmed = false;
	m_flLastEventTime = 0.0f;
}

void CHUDQuickInfo::VidInit(void)
{
	Init();

	m_icon_c = gHUD.GetIcon("crosshair");
	m_icon_rb = gHUD.GetIcon("crosshair_right_full");
	m_icon_lb = gHUD.GetIcon("crosshair_left_full");
	m_icon_rbe = gHUD.GetIcon("crosshair_right_empty");
	m_icon_lbe = gHUD.GetIcon("crosshair_left_empty");
	m_icon_rbn = gHUD.GetIcon("crosshair_right");
	m_icon_lbn = gHUD.GetIcon("crosshair_left");
}

void CHUDQuickInfo::DrawWarning(int x, int y, CHudTexture *icon, float &time)
{
	float scale = (int)(fabs(sin(CURTIME*8.0f)) * 128.0);

	// Only fade out at the low point of our blink
	if (time <= (gpGlobals->frametime * 200.0f))
	{
		if (scale < 40)
		{
			time = 0.0f;
			return;
		}
		else
		{
			// Counteract the offset below to survive another frame
			time += (gpGlobals->frametime * 200.0f);
		}
	}

	// Update our time
	time -= (gpGlobals->frametime * 200.0f);
	Color caution = gHUD.m_clrCaution;
	caution[3] = scale * 255;

	icon->DrawSelf(x, y, caution);
}

//-----------------------------------------------------------------------------
// Purpose: Save CPU cycles by letting the HUD system early cull
// costly traversal.  Called per frame, return true if thinking and 
// painting need to occur.
//-----------------------------------------------------------------------------
bool CHUDQuickInfo::ShouldDraw(void)
{
	if (!m_icon_c || !m_icon_rb || !m_icon_rbe || !m_icon_lb || !m_icon_lbe)
		return false;

	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (player == NULL)
		return false;
#ifdef DARKINTERVAL
	if (player->IsSuitEquipped() == false && player->IsSuitlessHUDVisible() == false) return false;
#endif
	if (!crosshair.GetBool())
		return false;

	return (CHudElement::ShouldDraw() && !engine->IsDrawingLoadingImage());
}

//-----------------------------------------------------------------------------
// Purpose: Checks if the hud element needs to fade out
//-----------------------------------------------------------------------------
void CHUDQuickInfo::OnThink()
{
	BaseClass::OnThink();

	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (player == NULL)
		return;

	// see if we should fade in/out
	bool bFadeOut = player->IsZoomed();

	// check if the state has changed
	if (m_bFadedOut != bFadeOut)
	{
		m_bFadedOut = bFadeOut;

		m_bDimmed = false;

		if (bFadeOut)
		{
			g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "Alpha", 0.0f, 0.0f, 0.25f, vgui::AnimationController::INTERPOLATOR_LINEAR);
		}
		else
		{
			g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "Alpha", QUICKINFO_BRIGHTNESS_FULL, 0.0f, QUICKINFO_FADE_IN_TIME, vgui::AnimationController::INTERPOLATOR_LINEAR);
		}
	}
	else if (!m_bFadedOut)
	{
		// If we're dormant, fade out
		if (EventTimeElapsed())
		{
			if (!m_bDimmed)
			{
				m_bDimmed = true;
				g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "Alpha", QUICKINFO_BRIGHTNESS_DIM, 0.0f, QUICKINFO_FADE_OUT_TIME, vgui::AnimationController::INTERPOLATOR_LINEAR);
			}
		}
		else if (m_bDimmed)
		{
			// Fade back up, we're active
			m_bDimmed = false;
			g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "Alpha", QUICKINFO_BRIGHTNESS_FULL, 0.0f, QUICKINFO_FADE_IN_TIME, vgui::AnimationController::INTERPOLATOR_LINEAR);
		}
	}
}

void CHUDQuickInfo::Paint()
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (player == NULL)
		return;

	C_BaseCombatWeapon *pWeapon = GetActiveWeapon();
	if (pWeapon == NULL)
		return;
#ifdef DARKINTERVAL
	int alpha = hud_transparency.GetInt();
	SetAlpha(alpha);
#endif
	float fX, fY;
	bool bBehindCamera = false;
	CHudCrosshair::GetDrawPosition(&fX, &fY, &bBehindCamera);

	// if the crosshair is behind the camera, don't draw it
	if (bBehindCamera)
		return;

	int		xCenter = (int)fX;
	int		yCenter = (int)fY - m_icon_lb->Height() / 2;

	float	scalar = 138.0f / 255.0f;

	// Check our health for a warning
	int	health = player->GetHealth();
	if (health != m_lastHealth)
	{
		UpdateEventTime();
		m_lastHealth = health;

		if (health <= HEALTH_WARNING_THRESHOLD)
		{
			if (m_warnHealth == false)
			{
				m_healthFade = 255;
				m_warnHealth = true;

				CLocalPlayerFilter filter;
				C_BaseEntity::EmitSound(filter, SOUND_FROM_LOCAL_PLAYER, "HUDQuickInfo.LowHealth");
			}
		}
		else
		{
			m_warnHealth = false;
		}
	}

	// Check our ammo for a warning
	int	ammo = pWeapon->Clip1();
	if (ammo != m_lastAmmo)
	{
		UpdateEventTime();
		m_lastAmmo = ammo;

		// Find how far through the current clip we are
		float ammoPerc = (float)ammo / (float)pWeapon->GetMaxClip1();

		// Warn if we're below a certain percentage of our clip's size
		if ((pWeapon->GetMaxClip1() > 1) && (ammoPerc <= (1.0f - CLIP_PERC_THRESHOLD)))
		{
			if (m_warnAmmo == false)
			{
				m_ammoFade = 255;
				m_warnAmmo = true;

				CLocalPlayerFilter filter;
				C_BaseEntity::EmitSound(filter, SOUND_FROM_LOCAL_PLAYER, "HUDQuickInfo.LowAmmo");
			}
		}
		else
		{
			m_warnAmmo = false;
		}
	}
#ifdef DARKINTERVAL // for worker suit, grey HUD
	Color clrNormal = player->IsSuitEquipped() ? gHUD.m_clrNormal : m_HullColor;
#else
	Color clrNormal = gHUD.m_clrNormal;
#endif
	clrNormal[3] = 255 * scalar;
	m_icon_c->DrawSelf(xCenter, yCenter, clrNormal);
#ifdef DARKINTERVAL
	if (hud_transparency.GetInt() == 0) return;
#endif
	if (!hud_quickinfo.GetInt())
	{
#ifdef DARKINTERVAL // restored from retail code
		// no quickinfo, just draw the small versions of the crosshairs
		clrNormal[3] = 196;
		m_icon_lbn->DrawSelf(xCenter - (m_icon_lbn->Width() * 2), yCenter, clrNormal);
		m_icon_rbn->DrawSelf(xCenter + m_icon_rbn->Width(), yCenter, clrNormal);
#endif
		return;
	}

	int	sinScale = (int)(fabs(sin(CURTIME*8.0f)) * 128.0f);

	// Update our health
	if (m_healthFade > 0.0f)
	{
		DrawWarning(xCenter - (m_icon_lb->Width() * 2), yCenter, m_icon_lb, m_healthFade);
	}
	else
	{
		float healthPerc = (float)health / 100.0f;
		healthPerc = clamp(healthPerc, 0.0f, 1.0f);

		Color healthColor = m_warnHealth ? gHUD.m_clrCaution : clrNormal;

		if (m_warnHealth)
		{
			healthColor[3] = 255 * sinScale;
		}
		else
		{
			healthColor[3] = 255 * scalar;
		}

		gHUD.DrawIconProgressBar(xCenter - (m_icon_lb->Width() * 2), yCenter, m_icon_lb, m_icon_lbe, (1.0f - healthPerc), healthColor, CHud::HUDPB_VERTICAL);
	}

	// Update our ammo
	if (m_ammoFade > 0.0f)
	{
		DrawWarning(xCenter + m_icon_rb->Width(), yCenter, m_icon_rb, m_ammoFade);
	}
	else
	{
		float ammoPerc;

		if (pWeapon->GetMaxClip1() <= 0)
		{
			ammoPerc = 0.0f;
		}
		else
		{
			ammoPerc = 1.0f - ((float)ammo / (float)pWeapon->GetMaxClip1());
			ammoPerc = clamp(ammoPerc, 0.0f, 1.0f);
		}

		Color ammoColor = m_warnAmmo ? gHUD.m_clrCaution : clrNormal;

		if (m_warnAmmo)
		{
			ammoColor[3] = 255 * sinScale;
		}
		else
		{
			ammoColor[3] = 255 * scalar;
		}

		gHUD.DrawIconProgressBar(xCenter + m_icon_rb->Width(), yCenter, m_icon_rb, m_icon_rbe, ammoPerc, ammoColor, CHud::HUDPB_VERTICAL);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHUDQuickInfo::UpdateEventTime(void)
{
	m_flLastEventTime = CURTIME;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool CHUDQuickInfo::EventTimeElapsed(void)
{
	if ((CURTIME - m_flLastEventTime) > QUICKINFO_EVENT_DURATION)
		return true;

	return false;
}

//-----------------------------------------------------------------------------
// Purpose: Displays the weapon selection HUD system
//-----------------------------------------------------------------------------
ConVar hud_showemptyweaponslots("hud_showemptyweaponslots", "0", FCVAR_ARCHIVE, "Shows slots for missing weapons when recieving weapons out of order");
#ifdef DARKINTERVAL
ConVar hud_stickyweaponselection("hud_stickyweaponselection", "0", FCVAR_ARCHIVE, "If true, the weapon selection HUD will not fade away, requiring to click or escape to close it");
extern ConVar hud_transparency;
#endif
#define SELECTION_TIMEOUT_THRESHOLD		0.5f	// Seconds
#define SELECTION_FADEOUT_TIME			0.75f

#define PLUS_DISPLAY_TIMEOUT			0.5f	// Seconds
#define PLUS_FADEOUT_TIME				0.75f

#define FASTSWITCH_DISPLAY_TIMEOUT		1.5f
#define FASTSWITCH_FADEOUT_TIME			1.5f

#define CAROUSEL_SMALL_DISPLAY_ALPHA	200.0f
#define FASTSWITCH_SMALL_DISPLAY_ALPHA	160.0f

#define MAX_CAROUSEL_SLOTS				5
#ifdef DARKINTERVAL
#define WEAPON_PICKUP_EFFECT			1
#define WEAPON_AMMO_BARS				1
#endif
//-----------------------------------------------------------------------------
// Purpose: hl2 weapon selection hud element
//-----------------------------------------------------------------------------
class CHudWeaponSelection : public CBaseHudWeaponSelection, public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(CHudWeaponSelection, vgui::Panel);

public:
	CHudWeaponSelection(const char *pElementName);

	virtual bool ShouldDraw();
	virtual void OnWeaponPickup(C_BaseCombatWeapon *pWeapon);

	virtual void CycleToNextWeapon(void);
	virtual void CycleToPrevWeapon(void);

	virtual C_BaseCombatWeapon *GetWeaponInSlot(int iSlot, int iSlotPos);
	virtual void SelectWeaponSlot(int iSlot);

	virtual C_BaseCombatWeapon	*GetSelectedWeapon(void)
	{
		return m_hSelectedWeapon;
	}

	virtual void OpenSelection(void);
	virtual void HideSelection(void);

	virtual void LevelInit();

protected:
	virtual void OnThink();
	virtual void Paint();
	virtual void ApplySchemeSettings(vgui::IScheme *pScheme);

	virtual bool IsWeaponSelectable()
	{
		if (IsInSelectionMode())
			return true;

		return false;
	}

	virtual void SetWeaponSelected()
	{
		CBaseHudWeaponSelection::SetWeaponSelected();

		switch (hud_fastswitch.GetInt())
		{
		case HUDTYPE_FASTSWITCH:
		case HUDTYPE_CAROUSEL:
			ActivateFastswitchWeaponDisplay(GetSelectedWeapon());
			break;
		case HUDTYPE_PLUS:
			ActivateWeaponHighlight(GetSelectedWeapon());
			break;
		default:
			// do nothing
			break;
		}
	}

private:
	C_BaseCombatWeapon *FindNextWeaponInWeaponSelection(int iCurrentSlot, int iCurrentPosition);
	C_BaseCombatWeapon *FindPrevWeaponInWeaponSelection(int iCurrentSlot, int iCurrentPosition);

	void DrawLargeWeaponBox(C_BaseCombatWeapon *pWeapon, bool bSelected, int x, int y, int wide, int tall, Color color, float alpha, int number);
	void ActivateFastswitchWeaponDisplay(C_BaseCombatWeapon *pWeapon);
	void ActivateWeaponHighlight(C_BaseCombatWeapon *pWeapon);
	float GetWeaponBoxAlpha(bool bSelected);
	int GetLastPosInSlot(int iSlot) const;

	void FastWeaponSwitch(int iWeaponSlot);
	void PlusTypeFastWeaponSwitch(int iWeaponSlot);

	virtual	void SetSelectedWeapon(C_BaseCombatWeapon *pWeapon)
	{
		m_hSelectedWeapon = pWeapon;
	}

	virtual	void SetSelectedSlot(int slot)
	{
		m_iSelectedSlot = slot;
	}

	void SetSelectedSlideDir(int dir)
	{
		m_iSelectedSlideDir = dir;
	}

	void DrawBox(int x, int y, int wide, int tall, Color color, float normalizedAlpha, int number);

	CPanelAnimationVar(vgui::HFont, m_hNumberFont, "NumberFont", "HudSelectionNumbers");
	CPanelAnimationVar(vgui::HFont, m_hTextFont, "TextFont", "HudSelectionText");
	CPanelAnimationVar(float, m_flBlur, "Blur", "0");

	CPanelAnimationVarAliasType(float, m_flSmallBoxSize, "SmallBoxSize", "32", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flLargeBoxWide, "LargeBoxWide", "108", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flLargeBoxTall, "LargeBoxTall", "72", "proportional_float");

	CPanelAnimationVarAliasType(float, m_flMediumBoxWide, "MediumBoxWide", "75", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flMediumBoxTall, "MediumBoxTall", "50", "proportional_float");

	CPanelAnimationVarAliasType(float, m_flBoxGap, "BoxGap", "12", "proportional_float");

	CPanelAnimationVarAliasType(float, m_flSelectionNumberXPos, "SelectionNumberXPos", "4", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flSelectionNumberYPos, "SelectionNumberYPos", "4", "proportional_float");
#if WEAPON_PICKUP_EFFECT
	CPanelAnimationVarAliasType(float, m_flIconXPos, "IconXPos", "16", "proportional_float");
	CPanelAnimationVarAliasType(float, m_flIconYPos, "IconYPos", "8", "proportional_float");
#endif
	CPanelAnimationVarAliasType(float, m_flTextYPos, "TextYPos", "54", "proportional_float");

	CPanelAnimationVar(float, m_flAlphaOverride, "Alpha", "0");
	CPanelAnimationVar(float, m_flSelectionAlphaOverride, "SelectionAlpha", "0");

	CPanelAnimationVar(Color, m_TextColor, "TextColor", "SelectionTextFg");
	CPanelAnimationVar(Color, m_NumberColor, "NumberColor", "SelectionNumberFg");
	CPanelAnimationVar(Color, m_EmptyBoxColor, "EmptyBoxColor", "SelectionEmptyBoxBg");
	CPanelAnimationVar(Color, m_BoxColor, "BoxColor", "SelectionBoxBg");
	CPanelAnimationVar(Color, m_SelectedBoxColor, "SelectedBoxColor", "SelectionSelectedBoxBg");
	CPanelAnimationVar(Color, m_SelectedFgColor, "SelectedFgColor", "FgColor");
	CPanelAnimationVar(Color, m_BrightBoxColor, "SelectedFgColor", "BgColor");
#if WEAPON_PICKUP_EFFECT
	CPanelAnimationVar(float, m_flWeaponPickupGrowTime, "SelectionGrowTime", "1.0");
#endif
	CPanelAnimationVar(float, m_flTextScan, "TextScan", "1.0");

	bool m_bFadingOut;

	// fastswitch weapon display
	struct WeaponBox_t
	{
		int m_iSlot;
		int m_iSlotPos;
	};
	CUtlVector<WeaponBox_t>	m_WeaponBoxes;
	int						m_iSelectedWeaponBox;
	int						m_iSelectedSlideDir;
	int						m_iSelectedBoxPosition;
	int						m_iSelectedSlot;
	C_BaseCombatWeapon		*m_pLastWeapon;
	CPanelAnimationVar(float, m_flHorizWeaponSelectOffsetPoint, "WeaponBoxOffset", "0");
#if WEAPON_PICKUP_EFFECT
	CHandle<C_BaseCombatWeapon> m_hLastPickedUpWeapon;
	float m_flPickupStartTime;
	float m_flWeaponPickupDisplayTime;
#endif
#if WEAPON_AMMO_BARS
	void DrawAmmoBar(C_BaseCombatWeapon *pWeapon, int x, int y, int nWidth, int nHeight, int gap);
	int DrawBar(int x, int y, int width, int height, float f);

#endif
};

DECLARE_HUDELEMENT(CHudWeaponSelection);

using namespace vgui;

//-----------------------------------------------------------------------------
// Purpose: Constructor
//-----------------------------------------------------------------------------
CHudWeaponSelection::CHudWeaponSelection(const char *pElementName) : CBaseHudWeaponSelection(pElementName), BaseClass(NULL, "HudWeaponSelection")
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
	m_bFadingOut = false;
#if WEAPON_PICKUP_EFFECT
	// get the sequence time from the animation file
	m_flWeaponPickupDisplayTime = g_pClientMode->GetViewportAnimationController()->GetAnimationSequenceLength("WeaponPickup");
#endif
}

//-----------------------------------------------------------------------------
// Purpose: sets up display for showing weapon pickup
//-----------------------------------------------------------------------------
void CHudWeaponSelection::OnWeaponPickup(C_BaseCombatWeapon *pWeapon)
{
#if WEAPON_PICKUP_EFFECT
	if (!IsInSelectionMode())
	{
		// show the weapon pickup animation, but only if we're not in weapon selection mode
		m_hLastPickedUpWeapon = pWeapon;
		m_flPickupStartTime = CURTIME;

		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponPickup");
	}
	//	return;
#endif
	// add to pickup history
	CHudHistoryResource *pHudHR = GET_HUDELEMENT(CHudHistoryResource);
	if (pHudHR)
	{
		pHudHR->AddToHistory(pWeapon);
	}
}

//-----------------------------------------------------------------------------
// Purpose: updates animation status
//-----------------------------------------------------------------------------
void CHudWeaponSelection::OnThink(void)
{
#if WEAPON_PICKUP_EFFECT
	if (m_hLastPickedUpWeapon)
	{
		// check to see if we should clear out the weapon display
		if (m_flPickupStartTime + m_flWeaponPickupDisplayTime < CURTIME)
		{
			m_hLastPickedUpWeapon = NULL;
		}
	}
	if (hud_stickyweaponselection.GetBool())
		return;
#endif
#ifdef DARKINTERVAL
	if (!hud_stickyweaponselection.GetBool())
#endif
	{
		float flSelectionTimeout = SELECTION_TIMEOUT_THRESHOLD;
		float flSelectionFadeoutTime = SELECTION_FADEOUT_TIME;
		if (hud_fastswitch.GetBool())
		{
			flSelectionTimeout = FASTSWITCH_DISPLAY_TIMEOUT;
			flSelectionFadeoutTime = FASTSWITCH_FADEOUT_TIME;
		}

		// Time out after awhile of inactivity
		if ((CURTIME - m_flSelectionTime) > flSelectionTimeout)
		{
			if (!m_bFadingOut)
			{
				// start fading out
				g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("FadeOutWeaponSelectionMenu");
				m_bFadingOut = true;
			}
			else if (CURTIME - m_flSelectionTime > flSelectionTimeout + flSelectionFadeoutTime)
			{
				// finished fade, close
				HideSelection();
			}
		}
		else if (m_bFadingOut)
		{
			// stop us fading out, show the animation again
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("OpenWeaponSelectionMenu");
			m_bFadingOut = false;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: returns true if the panel should draw
//-----------------------------------------------------------------------------
bool CHudWeaponSelection::ShouldDraw()
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
	{
		if (IsInSelectionMode())
		{
			HideSelection();
		}
		return false;
	}
#if WEAPON_PICKUP_EFFECT
	if (m_hLastPickedUpWeapon)
	{
		// Always draw weapon pickups, except when we don't have a suit
		if (!pPlayer->IsSuitEquipped())
			return false;

		return true;
	}
#endif
	bool bret = CBaseHudWeaponSelection::ShouldDraw();
	if (!bret)
		return false;

	// draw weapon selection a little longer if in fastswitch so we can see what we've selected
	if (hud_fastswitch.GetBool() && (CURTIME - m_flSelectionTime) < (FASTSWITCH_DISPLAY_TIMEOUT + FASTSWITCH_FADEOUT_TIME))
		return true;

	return (m_bSelectionVisible) ? true : false;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CHudWeaponSelection::LevelInit()
{
	CHudElement::LevelInit();

	m_iSelectedWeaponBox = -1;
	m_iSelectedSlideDir = 0;
	m_pLastWeapon = NULL;
#ifdef DARKINTERVAL
	m_flPickupStartTime = CURTIME;
#endif
}


//-----------------------------------------------------------------------------
// Purpose: starts animating the center of the draw point to the newly selected weapon
//-----------------------------------------------------------------------------
void CHudWeaponSelection::ActivateFastswitchWeaponDisplay(C_BaseCombatWeapon *pSelectedWeapon)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	// make sure all our configuration data is read
	MakeReadyForUse();

	m_WeaponBoxes.RemoveAll();
	m_iSelectedWeaponBox = 0;

	// find out where our selected weapon is in the full list
	int cWeapons = 0;
	int iLastSelectedWeaponBox = -1;
	for (int i = 0; i < MAX_WEAPON_SLOTS; i++)
	{
		for (int slotpos = 0; slotpos < MAX_WEAPON_POSITIONS; slotpos++)
		{
			C_BaseCombatWeapon *pWeapon = GetWeaponInSlot(i, slotpos);
			if (!pWeapon)
				continue;

			WeaponBox_t box = { i, slotpos };
			m_WeaponBoxes.AddToTail(box);

			if (pWeapon == pSelectedWeapon)
			{
				m_iSelectedWeaponBox = cWeapons;
			}
			if (pWeapon == m_pLastWeapon)
			{
				iLastSelectedWeaponBox = cWeapons;
			}
			cWeapons++;
		}
	}

	if (iLastSelectedWeaponBox == -1)
	{
		// unexpected failure, no last weapon to scroll from, default to snap behavior
		m_pLastWeapon = NULL;
	}

	// calculate where we would have to start drawing for this weapon to slide into center
	float flStart, flStop, flTime;
	if (!m_pLastWeapon || m_iSelectedSlideDir == 0 || m_flHorizWeaponSelectOffsetPoint != 0)
	{
		// no previous weapon or weapon selected directly or selection during slide, snap to exact position
		m_pLastWeapon = pSelectedWeapon;
		flStart = flStop = flTime = 0;
	}
	else
	{
		// offset display for a scroll
		// causing selected weapon to slide into position
		// scroll direction based on user's "previous" or "next" selection
		int numIcons = 0;
		int start = iLastSelectedWeaponBox;
		for (int i = 0; i<cWeapons; i++)
		{
			// count icons in direction of slide to destination
			if (start == m_iSelectedWeaponBox)
				break;
			if (m_iSelectedSlideDir < 0)
			{
				start--;
			}
			else
			{
				start++;
			}
			// handle wraparound in either direction
			start = (start + cWeapons) % cWeapons;
			numIcons++;
		}

		flStart = numIcons * (m_flLargeBoxWide + m_flBoxGap);
		if (m_iSelectedSlideDir < 0)
			flStart *= -1;
		flStop = 0;

		// shorten duration for scrolling when desired weapon is farther away
		// otherwise a large skip in the same duration causes the scroll to fly too fast
		flTime = numIcons * 0.20f;
		if (numIcons > 1)
			flTime *= 0.5f;
	}
	m_flHorizWeaponSelectOffsetPoint = flStart;
	g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "WeaponBoxOffset", flStop, 0, flTime, AnimationController::INTERPOLATOR_LINEAR);

	// start the highlight after the scroll completes
	m_flBlur = 7.f;
	g_pClientMode->GetViewportAnimationController()->RunAnimationCommand(this, "Blur", 0, flTime, 0.75f, AnimationController::INTERPOLATOR_DEACCEL);
}

//-----------------------------------------------------------------------------
// Purpose: starts animating the highlight for the selected weapon
//-----------------------------------------------------------------------------
void CHudWeaponSelection::ActivateWeaponHighlight(C_BaseCombatWeapon *pSelectedWeapon)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	// make sure all our configuration data is read
	MakeReadyForUse();

	C_BaseCombatWeapon *pWeapon = GetWeaponInSlot(m_iSelectedSlot, m_iSelectedBoxPosition);
	if (!pWeapon)
		return;

	// start the highlight after the scroll completes
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("WeaponHighlight");
}

//-----------------------------------------------------------------------------
// Purpose: returns an (per frame animating) alpha value for different weapon boxes
//-----------------------------------------------------------------------------
float CHudWeaponSelection::GetWeaponBoxAlpha(bool bSelected)
{
	float alpha;
	if (bSelected)
	{
		alpha = m_flSelectionAlphaOverride;
	}
	else
	{
		alpha = m_flSelectionAlphaOverride * (m_flAlphaOverride / 255.0f);
	}
#ifdef DARKINTERVAL
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		alpha = 0;

	if (!pPlayer->IsSuitEquipped())
		alpha = 0;
#endif
	return alpha;
}


//-------------------------------------------------------------------------
// Purpose: draws the selection area
//-------------------------------------------------------------------------
void CHudWeaponSelection::Paint()
{
	int width;
	int xpos;
	int ypos;

	if (!ShouldDraw())
		return;

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;
#ifdef DARKINTERVAL
	int alpha = hud_transparency.GetInt();
	SetAlpha(alpha);
#endif
	// find and display our current selection
	C_BaseCombatWeapon *pSelectedWeapon = NULL;
	switch (hud_fastswitch.GetInt())
	{
	case HUDTYPE_FASTSWITCH:
	case HUDTYPE_CAROUSEL:
		pSelectedWeapon = pPlayer->GetActiveWeapon();
		break;
	default:
		pSelectedWeapon = GetSelectedWeapon();
		break;
	}
#if WEAPON_PICKUP_EFFECT
	// override with the weapon we just picked up
	if (m_hLastPickedUpWeapon)
	{
		pSelectedWeapon = m_hLastPickedUpWeapon;
	}
#endif	
	if (!pSelectedWeapon)
		return;

	bool bPushedViewport = false;
	if (hud_fastswitch.GetInt() == HUDTYPE_FASTSWITCH || hud_fastswitch.GetInt() == HUDTYPE_PLUS)
	{
		CMatRenderContextPtr pRenderContext(materials);
		if (pRenderContext->GetRenderTarget())
		{
			surface()->PushFullscreenViewport();
			bPushedViewport = true;
		}
	}

	// interpolate the selected box size between the small box size and the large box size
	// interpolation has been removed since there is no weapon pickup animation anymore, so it's all at the largest size
	float percentageDone;
#if WEAPON_PICKUP_EFFECT
	percentageDone = min(1.0f, (CURTIME - m_flPickupStartTime) / m_flWeaponPickupGrowTime);
#else
	percentageDone = 1.0f;
#endif
	int largeBoxWide = m_flSmallBoxSize + ((m_flLargeBoxWide - m_flSmallBoxSize) * percentageDone);
	int largeBoxTall = m_flSmallBoxSize + ((m_flLargeBoxTall - m_flSmallBoxSize) * percentageDone);
	Color selectedColor;
	for (int i = 0; i < 4; i++)
	{
		selectedColor[i] = m_BoxColor[i] + ((m_SelectedBoxColor[i] - m_BoxColor[i]) * percentageDone);
	}

	switch (hud_fastswitch.GetInt())
	{
	case HUDTYPE_CAROUSEL:
	{
		// carousel style - flat line of items
		ypos = 0;
		if (m_iSelectedWeaponBox == -1 || m_WeaponBoxes.Count() <= 1)
		{
			// nothing to do
			return;
		}
		else if (m_WeaponBoxes.Count() < MAX_CAROUSEL_SLOTS)
		{
			// draw the selected weapon as a 1 of n style
			width = (m_WeaponBoxes.Count() - 1) * (m_flLargeBoxWide + m_flBoxGap) + m_flLargeBoxWide;
			xpos = (GetWide() - width) / 2;
			for (int i = 0; i<m_WeaponBoxes.Count(); i++)
			{
				C_BaseCombatWeapon *pWeapon = GetWeaponInSlot(m_WeaponBoxes[i].m_iSlot, m_WeaponBoxes[i].m_iSlotPos);
				if (!pWeapon)
					break;
#ifdef DARKINTERVAL
				float alpha = pPlayer->IsSuitEquipped() ? GetWeaponBoxAlpha(i == m_iSelectedWeaponBox) : 0;
#else
				float alpha = GetWeaponBoxAlpha(i == m_iSelectedWeaponBox);
#endif
				if (i == m_iSelectedWeaponBox)
				{
					// draw selected in highlighted style
					DrawLargeWeaponBox(pWeapon, true, xpos, ypos, m_flLargeBoxWide, m_flLargeBoxTall, selectedColor, alpha, -1);
				}
				else
				{
					DrawLargeWeaponBox(pWeapon, false, xpos, ypos, m_flLargeBoxWide, m_flLargeBoxTall / 1.5f, m_BoxColor, alpha, -1);
				}

				xpos += (m_flLargeBoxWide + m_flBoxGap);
			}
		}
		else
		{
			// draw the selected weapon in the center, as a continuous scrolling carosuel
			// draw at center the current selected and all items to its right
			xpos = GetWide() / 2 + m_flHorizWeaponSelectOffsetPoint - largeBoxWide / 2;
			int i = m_iSelectedWeaponBox;
			while (1)
			{
				C_BaseCombatWeapon *pWeapon = GetWeaponInSlot(m_WeaponBoxes[i].m_iSlot, m_WeaponBoxes[i].m_iSlotPos);
				if (!pWeapon)
					break;

				float alpha;
				if (i == m_iSelectedWeaponBox && !m_flHorizWeaponSelectOffsetPoint)
				{
					// draw selected in highlighted style
					alpha = GetWeaponBoxAlpha(true);
					DrawLargeWeaponBox(pWeapon, true, xpos, ypos, largeBoxWide, largeBoxTall, selectedColor, alpha, -1);
				}
				else
				{
					alpha = GetWeaponBoxAlpha(false);
					DrawLargeWeaponBox(pWeapon, false, xpos, ypos, largeBoxWide, largeBoxTall / 1.5f, m_BoxColor, alpha, -1);
				}

				// advance until past edge
				xpos += (largeBoxWide + m_flBoxGap);
				if (xpos >= GetWide())
					break;

				++i;
				if (i >= m_WeaponBoxes.Count())
				{
					// wraparound
					i = 0;
				}
			}

			// draw all items left of center
			xpos = GetWide() / 2 + m_flHorizWeaponSelectOffsetPoint - (3 * largeBoxWide / 2 + m_flBoxGap);
			i = m_iSelectedWeaponBox - 1;
			while (1)
			{
				if (i < 0)
				{
					// wraparound
					i = m_WeaponBoxes.Count() - 1;
				}

				C_BaseCombatWeapon *pWeapon = GetWeaponInSlot(m_WeaponBoxes[i].m_iSlot, m_WeaponBoxes[i].m_iSlotPos);
				if (!pWeapon)
					break;

				float alpha;
				if (i == m_iSelectedWeaponBox && !m_flHorizWeaponSelectOffsetPoint)
				{
					// draw selected in highlighted style
					alpha = GetWeaponBoxAlpha(true);
					DrawLargeWeaponBox(pWeapon, true, xpos, ypos, largeBoxWide, largeBoxTall, selectedColor, alpha, -1);
				}
				else
				{
					alpha = GetWeaponBoxAlpha(false);
					DrawLargeWeaponBox(pWeapon, false, xpos, ypos, largeBoxWide, largeBoxTall / 1.5f, m_BoxColor, alpha, -1);
				}

				// retreat until past edge
				xpos -= (largeBoxWide + m_flBoxGap);
				if (xpos + largeBoxWide <= 0)
					break;

				--i;
			}
		}
	}
	break;

	case HUDTYPE_PLUS:
	{
		float fCenterX, fCenterY;
		bool bBehindCamera = false;
		CHudCrosshair::GetDrawPosition(&fCenterX, &fCenterY, &bBehindCamera);

		// if the crosshair is behind the camera, don't draw it
		if (bBehindCamera)
			return;

		// bucket style
		int screenCenterX = (int)fCenterX;
		int screenCenterY = (int)fCenterY - 15; // Height isn't quite screen height, so adjust for center alignment

												// Modifiers for the four directions. Used to change the x and y offsets
												// of each box based on which bucket we're drawing. Bucket directions are
												// 0 = UP, 1 = RIGHT, 2 = DOWN, 3 = LEFT
		int xModifiers[] = { 0, 1, 0, -1, -1, 1 };
		int yModifiers[] = { -1, 0, 1, 0, 1, 1 };

		// Draw the four buckets
		for (int i = 0; i < MAX_WEAPON_SLOTS; ++i)
		{
			// Set the top left corner so the first box would be centered in the screen.
			int xPos = screenCenterX - (m_flMediumBoxWide / 2);
			int yPos = screenCenterY - (m_flMediumBoxTall / 2);

			// Find out how many positions to draw - an empty position should still
			// be drawn if there is an active weapon in any slots past it.
			int lastSlotPos = -1;
			for (int slotPos = 0; slotPos < MAX_WEAPON_POSITIONS; ++slotPos)
			{
				C_BaseCombatWeapon *pWeapon = GetWeaponInSlot(i, slotPos);
				if (pWeapon)
				{
					lastSlotPos = slotPos;
				}
			}

			// Draw the weapons in this bucket
			for (int slotPos = 0; slotPos <= lastSlotPos; ++slotPos)
			{
				// Offset the box position
				xPos += (m_flMediumBoxWide + 5) * xModifiers[i];
				yPos += (m_flMediumBoxTall + 5) * yModifiers[i];

				int boxWide = m_flMediumBoxWide;
				int boxTall = m_flMediumBoxTall;
				int x = xPos;
				int y = yPos;

				C_BaseCombatWeapon *pWeapon = GetWeaponInSlot(i, slotPos);
				bool selectedWeapon = false;
				if (i == m_iSelectedSlot && slotPos == m_iSelectedBoxPosition)
				{
					// This is a bit of a misnomer... we really are asking "Is this the selected slot"?
					selectedWeapon = true;
				}

				// Draw the box with the appropriate icon
				DrawLargeWeaponBox(pWeapon,
					selectedWeapon,
					x,
					y,
					boxWide,
					boxTall,
					selectedWeapon ? selectedColor : m_BoxColor,
					GetWeaponBoxAlpha(selectedWeapon),
					-1);
			}
		}
	}
	break;

	case HUDTYPE_BUCKETS:
	{
		// bucket style
		width = (MAX_WEAPON_SLOTS - 1) * (m_flSmallBoxSize + m_flBoxGap) + largeBoxWide;
		xpos = (GetWide() - width) / 2;
		ypos = 0;

		int iActiveSlot = (pSelectedWeapon ? pSelectedWeapon->GetSlot() : -1);

		// draw the bucket set
		// iterate over all the weapon slots
		for (int i = 0; i < MAX_WEAPON_SLOTS; i++)
		{
			if (i == iActiveSlot)
			{
				bool bDrawBucketNumber = true;
				int iLastPos = GetLastPosInSlot(i);

				for (int slotpos = 0; slotpos <= iLastPos; slotpos++)
				{
					C_BaseCombatWeapon *pWeapon = GetWeaponInSlot(i, slotpos);
					if (!pWeapon)
					{
						if (!hud_showemptyweaponslots.GetBool())
							continue;
						DrawBox(xpos, ypos, largeBoxWide, largeBoxTall, m_EmptyBoxColor, m_flAlphaOverride, bDrawBucketNumber ? i + 1 : -1);
					}
					else
					{
						bool bSelected = (pWeapon == pSelectedWeapon);
						DrawLargeWeaponBox(pWeapon,
							bSelected,
							xpos,
							ypos,
							largeBoxWide,
							largeBoxTall,
							bSelected ? selectedColor : m_BoxColor,
							GetWeaponBoxAlpha(bSelected),
							bDrawBucketNumber ? i + 1 : -1);
					}

					// move down to the next bucket
					ypos += (largeBoxTall + m_flBoxGap);
					bDrawBucketNumber = false;
				}

				xpos += largeBoxWide;
			}
			else
			{
				// check to see if there is a weapons in this bucket
				if (GetFirstPos(i))
				{
					// draw has weapon in slot
					DrawBox(xpos, ypos, m_flSmallBoxSize, m_flSmallBoxSize, m_BoxColor, m_flAlphaOverride, i + 1);
				}
				else
				{
					// draw empty slot
					DrawBox(xpos, ypos, m_flSmallBoxSize, m_flSmallBoxSize, m_EmptyBoxColor, m_flAlphaOverride, -1);
				}

				xpos += m_flSmallBoxSize;
			}

			// reset position
			ypos = 0;
			xpos += m_flBoxGap;
		}
	}
	break;

	default:
	{
		// do nothing
	}
	break;
	}

	if (bPushedViewport)
	{
		surface()->PopFullscreenViewport();
	}
}


//-----------------------------------------------------------------------------
// Purpose: draws a single weapon selection box
//-----------------------------------------------------------------------------
void CHudWeaponSelection::DrawLargeWeaponBox(C_BaseCombatWeapon *pWeapon, bool bSelected, int xpos, int ypos, int boxWide, int boxTall, Color selectedColor, float alpha, int number)
{
	Color col = bSelected ? m_SelectedFgColor : GetFgColor();
#ifdef DARKINTERVAL
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer->IsSuitEquipped())
		col[3] = 0;
#endif
	switch (hud_fastswitch.GetInt())
	{
	case HUDTYPE_BUCKETS:
	{
		// draw box for selected weapon
		DrawBox(xpos, ypos, boxWide, boxTall, selectedColor, alpha, number);

		// draw icon
		col[3] *= (alpha / 255.0f);
		if (pWeapon->GetSpriteActive())
		{
			// find the center of the box to draw in
			int iconWidth = pWeapon->GetSpriteActive()->Width();
			int iconHeight = pWeapon->GetSpriteActive()->Height();

			int x_offs = (boxWide - iconWidth) / 2;

			int y_offs;
			if (bSelected && hud_fastswitch.GetInt() != 0)
			{
				// place the icon aligned with the non-selected version
				y_offs = (boxTall / 1.5f - iconHeight) / 2;
			}
			else
			{
				y_offs = (boxTall - iconHeight) / 2;
			}

			if (!pWeapon->CanBeSelected())
			{
				// unselectable weapon, display as such
				col = Color(255, 0, 0, col[3]);
			}
			else if (bSelected)
			{
				// currently selected weapon, display brighter
				col[3] = alpha;

				// draw an active version over the top
				pWeapon->GetSpriteActive()->DrawSelf(xpos + x_offs, ypos + y_offs, col);
			}

			// draw the inactive version
			pWeapon->GetSpriteInactive()->DrawSelf(xpos + x_offs, ypos + y_offs, col);
#if WEAPON_AMMO_BARS
			DrawAmmoBar(pWeapon, xpos + boxWide * 0.15, ypos + boxTall * 0.1, boxWide * 0.3, 4, boxWide * 0.1); // Draw Ammo Bar
#endif
		}
	}
	break;

	case HUDTYPE_PLUS:
	case HUDTYPE_CAROUSEL:
	{
		if (!pWeapon)
		{
			// draw red box for an empty bubble
			if (bSelected)
			{
				selectedColor.SetColor(255, 0, 0, 40);
			}

			DrawBox(xpos, ypos, boxWide, boxTall, selectedColor, alpha, number);
			return;
		}
		else
		{
			// draw box for selected weapon
			DrawBox(xpos, ypos, boxWide, boxTall, selectedColor, alpha, number);
		}

		int iconWidth;
		int	iconHeight;
		int	x_offs;
		int	y_offs;

		// draw icon
		col[3] *= (alpha / 255.0f);

		if (pWeapon->GetSpriteInactive())
		{
			iconWidth = pWeapon->GetSpriteInactive()->Width();
			iconHeight = pWeapon->GetSpriteInactive()->Height();

			x_offs = (boxWide - iconWidth) / 2;
			if (bSelected && HUDTYPE_CAROUSEL == hud_fastswitch.GetInt())
			{
				// place the icon aligned with the non-selected version
				y_offs = (boxTall / 1.5f - iconHeight) / 2;
			}
			else
			{
				y_offs = (boxTall - iconHeight) / 2;
			}

			if (!pWeapon->CanBeSelected())
			{
				// unselectable weapon, display as such
				col = Color(255, 0, 0, col[3]);
			}

			// draw the inactive version
			pWeapon->GetSpriteInactive()->DrawSelf(xpos + x_offs, ypos + y_offs, iconWidth, iconHeight, col);
		}

		if (bSelected && pWeapon->GetSpriteActive())
		{
			// find the center of the box to draw in
			iconWidth = pWeapon->GetSpriteActive()->Width();
			iconHeight = pWeapon->GetSpriteActive()->Height();

			x_offs = (boxWide - iconWidth) / 2;
			if (HUDTYPE_CAROUSEL == hud_fastswitch.GetInt())
			{
				// place the icon aligned with the non-selected version
				y_offs = (boxTall / 1.5f - iconHeight) / 2;
			}
			else
			{
				y_offs = (boxTall - iconHeight) / 2;
			}

			col[3] = 255;
			for (float fl = m_flBlur; fl > 0.0f; fl -= 1.0f)
			{
				if (fl >= 1.0f)
				{
					pWeapon->GetSpriteActive()->DrawSelf(xpos + x_offs, ypos + y_offs, col);
				}
				else
				{
					// draw a percentage of the last one
					col[3] *= fl;
					pWeapon->GetSpriteActive()->DrawSelf(xpos + x_offs, ypos + y_offs, col);
				}
			}
		}
	}
	break;

	default:
	{
		// do nothing
	}
	break;
	}

	if (HUDTYPE_PLUS == hud_fastswitch.GetInt())
	{
		// No text in plus bucket method
		return;
	}

	// draw text
	col = m_TextColor;
#ifdef DARKINTERVAL
	if (!pPlayer->IsSuitEquipped())
	{
		col[0] = col[1] = col[2] = 130;
		col[3] = 230;
	}
#endif
	const FileWeaponInfo_t &weaponInfo = pWeapon->GetWpnData();

	if (bSelected)
	{
		wchar_t text[128];
		wchar_t *tempString = g_pVGuiLocalize->Find(weaponInfo.szPrintName);
#ifdef WEAPON_PICKUP_EFFECT
		wchar_t *acqString = g_pVGuiLocalize->Find("#HL2_Acquired");
#endif

		// setup our localized string
		if (tempString)
		{
#ifdef WIN32
#ifdef WEAPON_PICKUP_EFFECT
			if (m_hLastPickedUpWeapon)
			{
				_snwprintf(text, sizeof(text) / sizeof(wchar_t) - 1, L"%s: %s", acqString, tempString);
			}
			else
#endif
				_snwprintf(text, sizeof(text) / sizeof(wchar_t) - 1, L"%s", tempString);
#else
			_snwprintf(text, sizeof(text) / sizeof(wchar_t) - 1, L"%S", tempString);
#endif
			text[sizeof(text) / sizeof(wchar_t) - 1] = 0;
		}
		else
		{
			// string wasn't found by g_pVGuiLocalize->Find()
			g_pVGuiLocalize->ConvertANSIToUnicode(weaponInfo.szPrintName, text, sizeof(text));
		}

		surface()->DrawSetTextColor(col);
		surface()->DrawSetTextFont(m_hTextFont);

		// count the position
		int slen = 0, charCount = 0, maxslen = 0;
		int firstslen = 0;
		{
			for (wchar_t *pch = text; *pch != 0; pch++)
			{
				if (*pch == '\n')
				{
					// newline character, drop to the next line
					if (slen > maxslen)
					{
						maxslen = slen;
					}
					if (!firstslen)
					{
						firstslen = slen;
					}

					slen = 0;
				}
				else if (*pch == '\r')
				{
					// do nothing
				}
				else
				{
					slen += surface()->GetCharacterWidth(m_hTextFont, *pch);
					charCount++;
				}
			}
		}
		if (slen > maxslen)
		{
			maxslen = slen;
		}
		if (!firstslen)
		{
			firstslen = maxslen;
		}

		int tx = xpos + ((m_flLargeBoxWide - firstslen) / 2);
		int ty = ypos + (int)m_flTextYPos;
		surface()->DrawSetTextPos(tx, ty);
		// adjust the charCount by the scan amount
		charCount *= m_flTextScan;
		for (wchar_t *pch = text; charCount > 0; pch++)
		{
			if (*pch == '\n')
			{
				// newline character, move to the next line
				surface()->DrawSetTextPos(xpos + ((boxWide - slen) / 2), ty + (surface()->GetFontTall(m_hTextFont) * 1.1f));
			}
			else if (*pch == '\r')
			{
				// do nothing
			}
			else
			{
				surface()->DrawUnicodeChar(*pch);
				charCount--;
			}
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: draws a selection box
//-----------------------------------------------------------------------------
void CHudWeaponSelection::DrawBox(int x, int y, int wide, int tall, Color color, float normalizedAlpha, int number)
{
#ifdef DARKINTERVAL
	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (!pPlayer) return;
	BaseClass::DrawBox(x, y, wide, tall, color, normalizedAlpha / 255.0f);
#endif
	// draw the number
	if (number >= 0)
	{
		Color numberColor = m_NumberColor;
#ifdef DARKINTERVAL
		if (!pPlayer->IsSuitEquipped())
		{
			numberColor[3] = 0;
		}
		else
#endif
		{
			numberColor[3] *= normalizedAlpha / 255.0f;
		}
		surface()->DrawSetTextColor(numberColor);
		surface()->DrawSetTextFont(m_hNumberFont);
		wchar_t wch = '0' + number;
		surface()->DrawSetTextPos(x + m_flSelectionNumberXPos, y + m_flSelectionNumberYPos);
		surface()->DrawUnicodeChar(wch);
	}
}

//-----------------------------------------------------------------------------
// Purpose: hud scheme settings
//-----------------------------------------------------------------------------
void CHudWeaponSelection::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(false);

	// set our size
	int screenWide, screenTall;
	int x, y;
	GetPos(x, y);
	GetHudSize(screenWide, screenTall);

	if (hud_fastswitch.GetInt() == HUDTYPE_CAROUSEL)
	{
		// need bounds to be exact width for proper clipping during scroll 
		int width = MAX_CAROUSEL_SLOTS*m_flLargeBoxWide + (MAX_CAROUSEL_SLOTS - 1)*m_flBoxGap;
		SetBounds((screenWide - width) / 2, y, width, screenTall - y);
	}
	else
	{
		SetBounds(x, y, screenWide - x, screenTall - y);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Opens weapon selection control
//-----------------------------------------------------------------------------
void CHudWeaponSelection::OpenSelection(void)
{
	Assert(!IsInSelectionMode());
#ifdef WEAPON_PICKUP_EFFECT
	m_hLastPickedUpWeapon = NULL;
#endif
	CBaseHudWeaponSelection::OpenSelection();
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("OpenWeaponSelectionMenu");
	m_iSelectedBoxPosition = 0;
	m_iSelectedSlot = -1;
}

//-----------------------------------------------------------------------------
// Purpose: Closes weapon selection control immediately
//-----------------------------------------------------------------------------
void CHudWeaponSelection::HideSelection(void)
{
	CBaseHudWeaponSelection::HideSelection();
	g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("CloseWeaponSelectionMenu");
	m_bFadingOut = false;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the next available weapon item in the weapon selection
//-----------------------------------------------------------------------------
C_BaseCombatWeapon *CHudWeaponSelection::FindNextWeaponInWeaponSelection(int iCurrentSlot, int iCurrentPosition)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return NULL;

	C_BaseCombatWeapon *pNextWeapon = NULL;

	// search all the weapons looking for the closest next
	int iLowestNextSlot = MAX_WEAPON_SLOTS;
	int iLowestNextPosition = MAX_WEAPON_POSITIONS;
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		C_BaseCombatWeapon *pWeapon = pPlayer->GetWeapon(i);
		if (!pWeapon)
			continue;

		if (CanBeSelectedInHUD(pWeapon))
		{
			int weaponSlot = pWeapon->GetSlot(), weaponPosition = pWeapon->GetPosition();

			// see if this weapon is further ahead in the selection list
			if (weaponSlot > iCurrentSlot || (weaponSlot == iCurrentSlot && weaponPosition > iCurrentPosition))
			{
				// see if this weapon is closer than the current lowest
				if (weaponSlot < iLowestNextSlot || (weaponSlot == iLowestNextSlot && weaponPosition < iLowestNextPosition))
				{
					iLowestNextSlot = weaponSlot;
					iLowestNextPosition = weaponPosition;
					pNextWeapon = pWeapon;
				}
			}
		}
	}

	return pNextWeapon;
}

//-----------------------------------------------------------------------------
// Purpose: Returns the prior available weapon item in the weapon selection
//-----------------------------------------------------------------------------
C_BaseCombatWeapon *CHudWeaponSelection::FindPrevWeaponInWeaponSelection(int iCurrentSlot, int iCurrentPosition)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return NULL;

	C_BaseCombatWeapon *pPrevWeapon = NULL;

	// search all the weapons looking for the closest next
	int iLowestPrevSlot = -1;
	int iLowestPrevPosition = -1;
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		C_BaseCombatWeapon *pWeapon = pPlayer->GetWeapon(i);
		if (!pWeapon)
			continue;

		if (CanBeSelectedInHUD(pWeapon))
		{
			int weaponSlot = pWeapon->GetSlot(), weaponPosition = pWeapon->GetPosition();

			// see if this weapon is further ahead in the selection list
			if (weaponSlot < iCurrentSlot || (weaponSlot == iCurrentSlot && weaponPosition < iCurrentPosition))
			{
				// see if this weapon is closer than the current lowest
				if (weaponSlot > iLowestPrevSlot || (weaponSlot == iLowestPrevSlot && weaponPosition > iLowestPrevPosition))
				{
					iLowestPrevSlot = weaponSlot;
					iLowestPrevPosition = weaponPosition;
					pPrevWeapon = pWeapon;
				}
			}
		}
	}

	return pPrevWeapon;
}

//-----------------------------------------------------------------------------
// Purpose: Moves the selection to the next item in the menu
//-----------------------------------------------------------------------------
void CHudWeaponSelection::CycleToNextWeapon(void)
{
	// Get the local player.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	m_pLastWeapon = pPlayer->GetActiveWeapon();

	C_BaseCombatWeapon *pNextWeapon = NULL;
	if (IsInSelectionMode())
	{
		// find the next selection spot
		C_BaseCombatWeapon *pWeapon = GetSelectedWeapon();
		if (!pWeapon)
			return;

		pNextWeapon = FindNextWeaponInWeaponSelection(pWeapon->GetSlot(), pWeapon->GetPosition());
	}
	else
	{
		// open selection at the current place
		pNextWeapon = pPlayer->GetActiveWeapon();
		if (pNextWeapon)
		{
			pNextWeapon = FindNextWeaponInWeaponSelection(pNextWeapon->GetSlot(), pNextWeapon->GetPosition());
		}
	}

	if (!pNextWeapon)
	{
		// wrap around back to start
		pNextWeapon = FindNextWeaponInWeaponSelection(-1, -1);
	}

	if (pNextWeapon)
	{
		SetSelectedWeapon(pNextWeapon);
		SetSelectedSlideDir(1);

		if (!IsInSelectionMode())
		{
			OpenSelection();
		}

		// Play the "cycle to next weapon" sound
		pPlayer->EmitSound("Player.WeaponSelectionMoveSlot");
	}
}

//-----------------------------------------------------------------------------
// Purpose: Moves the selection to the previous item in the menu
//-----------------------------------------------------------------------------
void CHudWeaponSelection::CycleToPrevWeapon(void)
{
	// Get the local player.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	m_pLastWeapon = pPlayer->GetActiveWeapon();

	C_BaseCombatWeapon *pNextWeapon = NULL;
	if (IsInSelectionMode())
	{
		// find the next selection spot
		C_BaseCombatWeapon *pWeapon = GetSelectedWeapon();
		if (!pWeapon)
			return;

		pNextWeapon = FindPrevWeaponInWeaponSelection(pWeapon->GetSlot(), pWeapon->GetPosition());
	}
	else
	{
		// open selection at the current place
		pNextWeapon = pPlayer->GetActiveWeapon();
		if (pNextWeapon)
		{
			pNextWeapon = FindPrevWeaponInWeaponSelection(pNextWeapon->GetSlot(), pNextWeapon->GetPosition());
		}
	}

	if (!pNextWeapon)
	{
		// wrap around back to end of weapon list
		pNextWeapon = FindPrevWeaponInWeaponSelection(MAX_WEAPON_SLOTS, MAX_WEAPON_POSITIONS);
	}

	if (pNextWeapon)
	{
		SetSelectedWeapon(pNextWeapon);
		SetSelectedSlideDir(-1);

		if (!IsInSelectionMode())
		{
			OpenSelection();
		}

		// Play the "cycle to next weapon" sound
		pPlayer->EmitSound("Player.WeaponSelectionMoveSlot");
	}
}

//-----------------------------------------------------------------------------
// Purpose: returns the # of the last weapon in the specified slot
//-----------------------------------------------------------------------------
int CHudWeaponSelection::GetLastPosInSlot(int iSlot) const
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	int iMaxSlotPos;

	if (!player)
		return -1;

	iMaxSlotPos = -1;
	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		C_BaseCombatWeapon *pWeapon = player->GetWeapon(i);

		if (pWeapon == NULL)
			continue;

		if (pWeapon->GetSlot() == iSlot && pWeapon->GetPosition() > iMaxSlotPos)
			iMaxSlotPos = pWeapon->GetPosition();
	}

	return iMaxSlotPos;
}

//-----------------------------------------------------------------------------
// Purpose: returns the weapon in the specified slot
//-----------------------------------------------------------------------------
C_BaseCombatWeapon *CHudWeaponSelection::GetWeaponInSlot(int iSlot, int iSlotPos)
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if (!player)
		return NULL;

	for (int i = 0; i < MAX_WEAPONS; i++)
	{
		C_BaseCombatWeapon *pWeapon = player->GetWeapon(i);

		if (pWeapon == NULL)
			continue;

		if (pWeapon->GetSlot() == iSlot && pWeapon->GetPosition() == iSlotPos)
			return pWeapon;
	}

	return NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Opens the next weapon in the slot
//-----------------------------------------------------------------------------
void CHudWeaponSelection::FastWeaponSwitch(int iWeaponSlot)
{
	// get the slot the player's weapon is in
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	m_pLastWeapon = NULL;

	// see where we should start selection
	int iPosition = -1;
	C_BaseCombatWeapon *pActiveWeapon = pPlayer->GetActiveWeapon();
	if (pActiveWeapon && pActiveWeapon->GetSlot() == iWeaponSlot)
	{
		// start after this weapon
		iPosition = pActiveWeapon->GetPosition();
	}

	C_BaseCombatWeapon *pNextWeapon = NULL;

	// search for the weapon after the current one
	pNextWeapon = FindNextWeaponInWeaponSelection(iWeaponSlot, iPosition);
	// make sure it's in the same bucket
	if (!pNextWeapon || pNextWeapon->GetSlot() != iWeaponSlot)
	{
		// just look for any weapon in this slot
		pNextWeapon = FindNextWeaponInWeaponSelection(iWeaponSlot, -1);
	}

	// see if we found a weapon that's different from the current and in the selected slot
	if (pNextWeapon && pNextWeapon != pActiveWeapon && pNextWeapon->GetSlot() == iWeaponSlot)
	{
		// select the new weapon
		::input->MakeWeaponSelection(pNextWeapon);
	}
	else if (pNextWeapon != pActiveWeapon)
	{
		// error sound
		pPlayer->EmitSound("Player.DenyWeaponSelection");
	}

	if (HUDTYPE_CAROUSEL != hud_fastswitch.GetInt())
	{
		// kill any fastswitch display
		m_flSelectionTime = 0.0f;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Opens the next weapon in the slot
//-----------------------------------------------------------------------------
void CHudWeaponSelection::PlusTypeFastWeaponSwitch(int iWeaponSlot)
{
	// get the slot the player's weapon is in
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	m_pLastWeapon = NULL;
	int newSlot = m_iSelectedSlot;

	// Changing slot number does not necessarily mean we need to change the slot - the player could be
	// scrolling through the same slot but in the opposite direction. Slot pairs are 0,2 and 1,3 - so
	// compare the 0 bits to see if we're within a pair. Otherwise, reset the box to the zero position.
	if (-1 == m_iSelectedSlot || ((m_iSelectedSlot ^ iWeaponSlot) & 1))
	{
		// Changing vertical/horizontal direction. Reset the selected box position to zero.
		m_iSelectedBoxPosition = 0;
		m_iSelectedSlot = iWeaponSlot;
	}
	else
	{
		// Still in the same horizontal/vertical direction. Determine which way we're moving in the slot.
		int increment = 1;
		if (m_iSelectedSlot != iWeaponSlot)
		{
			// Decrementing within the slot. If we're at the zero position in this slot, 
			// jump to the zero position of the opposite slot. This also counts as our increment.
			increment = -1;
			if (0 == m_iSelectedBoxPosition)
			{
				newSlot = (m_iSelectedSlot + 2) % 4;
				increment = 0;
			}
		}

		// Find out of the box position is at the end of the slot
		int lastSlotPos = -1;
		for (int slotPos = 0; slotPos < MAX_WEAPON_POSITIONS; ++slotPos)
		{
			C_BaseCombatWeapon *pWeapon = GetWeaponInSlot(newSlot, slotPos);
			if (pWeapon)
			{
				lastSlotPos = slotPos;
			}
		}

		// Increment/Decrement the selected box position
		if (m_iSelectedBoxPosition + increment <= lastSlotPos)
		{
			m_iSelectedBoxPosition += increment;
			m_iSelectedSlot = newSlot;
		}
		else
		{
			// error sound
			pPlayer->EmitSound("Player.DenyWeaponSelection");
			return;
		}
	}

	// Select the weapon in this position
	bool bWeaponSelected = false;
	C_BaseCombatWeapon *pActiveWeapon = pPlayer->GetActiveWeapon();
	C_BaseCombatWeapon *pWeapon = GetWeaponInSlot(m_iSelectedSlot, m_iSelectedBoxPosition);
	if (pWeapon)
	{
		if (pWeapon != pActiveWeapon)
		{
			// Select the new weapon
			::input->MakeWeaponSelection(pWeapon);
			SetSelectedWeapon(pWeapon);
			bWeaponSelected = true;
		}
	}

	if (!bWeaponSelected)
	{
		// Still need to set this to make hud display appear
		SetSelectedWeapon(pPlayer->GetActiveWeapon());
	}
}

//-----------------------------------------------------------------------------
// Purpose: Moves selection to the specified slot
//-----------------------------------------------------------------------------
void CHudWeaponSelection::SelectWeaponSlot(int iSlot)
{
	// iSlot is one higher than it should be, since it's the number key, not the 0-based index into the weapons
	--iSlot;

	// Get the local player.
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	// Don't try and read past our possible number of slots
	if (iSlot >= MAX_WEAPON_SLOTS)
		return;

	// Make sure the player's allowed to switch weapons
	if (pPlayer->IsAllowedToSwitchWeapons() == false)
		return;

	switch (hud_fastswitch.GetInt())
	{
	case HUDTYPE_FASTSWITCH:
	case HUDTYPE_CAROUSEL:
	{
		FastWeaponSwitch(iSlot);
		return;
	}

	case HUDTYPE_PLUS:
	{
		if (!IsInSelectionMode())
		{
			// open the weapon selection
			OpenSelection();
		}

		PlusTypeFastWeaponSwitch(iSlot);
		ActivateWeaponHighlight(GetSelectedWeapon());
	}
	break;

	case HUDTYPE_BUCKETS:
	{
		int slotPos = 0;
		C_BaseCombatWeapon *pActiveWeapon = GetSelectedWeapon();

		// start later in the list
		if (IsInSelectionMode() && pActiveWeapon && pActiveWeapon->GetSlot() == iSlot)
		{
			slotPos = pActiveWeapon->GetPosition() + 1;
		}

		// find the weapon in this slot
		pActiveWeapon = GetNextActivePos(iSlot, slotPos);
		if (!pActiveWeapon)
		{
			pActiveWeapon = GetNextActivePos(iSlot, 0);
		}

		if (pActiveWeapon != NULL)
		{
			if (!IsInSelectionMode())
			{
				// open the weapon selection
				OpenSelection();
			}

			// Mark the change
			SetSelectedWeapon(pActiveWeapon);
			SetSelectedSlideDir(0);
		}
	}

	default:
	{
		// do nothing
	}
	break;
	}

	pPlayer->EmitSound("Player.WeaponSelectionMoveSlot");
}

#if WEAPON_AMMO_BARS
void CHudWeaponSelection::DrawAmmoBar(C_BaseCombatWeapon *pWeapon, int x, int y, int nWidth, int nHeight, int gap)
{
	if (pWeapon->GetPrimaryAmmoType() != -1)
	{
		C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

		if (pPlayer && !pPlayer->IsSuitEquipped())
			return; // only enable bars in HEV

		int nAmmoCount = pPlayer->GetAmmoCount(pWeapon->GetPrimaryAmmoType());
		//if (!nAmmoCount)
		//	return; // we want to draw completely empty bars if there's no ammo reserve, feels odd otherwise

		float flPct = (float)nAmmoCount / (float)GetAmmoDef()->MaxCarry(pWeapon->GetPrimaryAmmoType());

		x = DrawBar(x, y, nWidth, nHeight, flPct);

		// Do we have secondary ammo too?
		if (pWeapon->GetSecondaryAmmoType() != -1)
		{
			flPct = (float)pPlayer->GetAmmoCount(pWeapon->GetSecondaryAmmoType()) / (float)GetAmmoDef()->MaxCarry(pWeapon->GetSecondaryAmmoType());

			x += gap;

			DrawBar(x, y, nWidth, nHeight, flPct);
		}
	}
}

int CHudWeaponSelection::DrawBar(int x, int y, int nWidth, int nHeight, float flPct)
{
	Color clrBar;
	int r, g, b, a;

	if (flPct < 0)
	{
		flPct = 0;
	}
	else if (flPct > 1)
	{
		flPct = 1;
	}

	if (flPct)
	{
		int nBarWidth = flPct * nWidth;

		// Always show at least one pixel if we have ammo.
		if (nBarWidth <= 0)
			nBarWidth = 1;

		m_SelectedFgColor.GetColor(r, g, b, a);

		clrBar.SetColor(r, g, b, 255);
		vgui::surface()->DrawSetColor(clrBar);
		vgui::surface()->DrawFilledRect(x, y, x + nBarWidth, y + nHeight);

		x += nBarWidth;
		nWidth -= nBarWidth;
	}

	m_BrightBoxColor.GetColor(r, g, b, a);

	clrBar.SetColor(r, g, b, 128);
	vgui::surface()->DrawSetColor(clrBar);
	vgui::surface()->DrawFilledRect(x, y, x + nWidth, y + nHeight);

	return (x + nWidth);
}
#endif // WEAPON AMMO BARS

#if 0
class HUD_OldStyleNumeric : public vgui::Panel
{
	DECLARE_CLASS_SIMPLE(HUD_OldStyleNumeric, vgui::Panel);

public:
	HUD_OldStyleNumeric(vgui::Panel *parent, const char *name);
	void	VidInit(void);

protected:
	int		DrawHudNumber(int x, int y, int iNumber, Color &clrDraw);
	int		GetNumberFontHeight(void);
	int		GetNumberFontWidth(void);

private:
	CHudTexture *icon_digits[10];
};

HUD_OldStyleNumeric::HUD_OldStyleNumeric(vgui::Panel *parent, const char *name) : BaseClass(parent, name)
{
	vgui::Panel *pParent = g_pClientMode->GetViewport();
	SetParent(pParent);
}


void HUD_OldStyleNumeric::VidInit(void)
{
	for (int i = 0; i < 10; i++)
	{
		char szNumString[10];

		sprintf(szNumString, "number_%d", i);
		icon_digits[i] = gHUD.GetIcon(szNumString);
	}
}

int HUD_OldStyleNumeric::GetNumberFontHeight(void)
{
	if (icon_digits[0])
	{
		return icon_digits[0]->Height();
	}
	else
	{
		return 0;
	}
}

int HUD_OldStyleNumeric::GetNumberFontWidth(void)
{
	if (icon_digits[0])
	{
		return icon_digits[0]->Width();
	}
	else
	{
		return 0;
	}
}

int HUD_OldStyleNumeric::DrawHudNumber(int x, int y, int iNumber, Color &clrDraw)
{
	int iWidth = GetNumberFontWidth();
	int k;

	if (iNumber > 0)
	{
		// SPR_Draw 100's
		if (iNumber >= 100)
		{
			k = iNumber / 100;
			icon_digits[k]->DrawSelf(x, y, clrDraw);
			x += iWidth;
		}
		else
		{
			x += iWidth;
		}

		// SPR_Draw 10's
		if (iNumber >= 10)
		{
			k = (iNumber % 100) / 10;
			icon_digits[k]->DrawSelf(x, y, clrDraw);
			x += iWidth;
		}
		else
		{
			x += iWidth;
		}

		// SPR_Draw ones
		k = iNumber % 10;
		icon_digits[k]->DrawSelf(x, y, clrDraw);
		x += iWidth;
	}
	else
	{
		// SPR_Draw 100's
		x += iWidth;

		// SPR_Draw 10's
		x += iWidth;

		// SPR_Draw ones
		k = 0;
		icon_digits[k]->DrawSelf(x, y, clrDraw);
		x += iWidth;
	}

	return x;
}

#define FADE_TIME	100
#define MIN_ALPHA	100	

extern ConVar hud_transparency;

//-----------------------------------------------------------------------------
// Purpose: Health panel
//-----------------------------------------------------------------------------
#define INIT_HEALTH -1

class HUD_HealthMeter : public CHudElement, public HUD_OldStyleNumeric
{
	DECLARE_CLASS_SIMPLE(HUD_HealthMeter, HUD_OldStyleNumeric);

public:
	HUD_HealthMeter(const char *pElementName); 
	virtual void Init(void);
	virtual void VidInit(void);
	virtual void Reset(void);
	virtual void OnThink();
	void MsgFunc_Damage(bf_read &msg);

private:
	virtual void Paint();
	void	ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	CHudTexture *icon_cross;
	int		m_iHealth;
	float	m_flFade;
	int		m_bitsDamage;
};

DECLARE_HUDELEMENT(HUD_HealthMeter);
DECLARE_HUD_MESSAGE(HUD_HealthMeter, Damage);

HUD_HealthMeter::HUD_HealthMeter(const char *pElementName) : CHudElement(pElementName), HUD_OldStyleNumeric(NULL, "HudHealthDI")
{
	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_PLAYERDEAD | HIDEHUD_NEEDSUIT);
}

void HUD_HealthMeter::Init()
{
	HOOK_HUD_MESSAGE(HUD_HealthMeter, Damage);
	Reset();
}

void HUD_HealthMeter::VidInit()
{
	Reset();
	BaseClass::VidInit();
}

void HUD_HealthMeter::Reset()
{
	m_iHealth = INIT_HEALTH;
	m_flFade = 0;
	m_bitsDamage = 0;

	/*
	wchar_t *tempString = g_pVGuiLocalize->Find("#Valve_Hud_HEALTH");

	if (tempString)
	{
		SetLabelText(tempString);
	}
	else
	{
		SetLabelText(L"HEALTH");
	}

	SetDisplayValue(m_iHealth);
	*/
}

void HUD_HealthMeter::OnThink()
{
	int newHealth = 0;
	C_BasePlayer *local = C_BasePlayer::GetLocalPlayer();
	if (local)
	{
		// Never below zero
		newHealth = MAX(local->GetHealth(), 0);
	}
	
	// Only update the fade if we've changed health
	if (newHealth == m_iHealth)
	{
		return;
	}

	m_flFade = FADE_TIME;
	m_iHealth = newHealth;
		
	if (m_iHealth >= 20)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthIncreasedAbove20");
	}
	else if (m_iHealth > 0)
	{
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthIncreasedBelow20");
		g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthLow");
	}
}

void HUD_HealthMeter::Paint()
{
//	if (!ShouldDraw())
//		return;

	Color	clrHealth;
	int		a;
	int		x;
	int		y;

	BaseClass::Paint();

	if (!icon_cross)
	{
		icon_cross = gHUD.GetIcon("cross");
	}

	if (!icon_cross)
	{
		return;
	}

	if (m_iHealth > 0)
	{
		int alpha = hud_transparency.GetInt();
		SetAlpha(alpha);
	}

	// Has health changed? Flash the health #
	if (m_flFade)
	{
		m_flFade -= (gpGlobals->frametime * 20);
		if (m_flFade <= 0)
		{
			a = MIN_ALPHA;
			m_flFade = 0;
		}
		else
		{
			// Fade the health number back to dim
			a = MIN_ALPHA + (m_flFade / FADE_TIME) * 128;
		}
	}
	else
	{
		a = MIN_ALPHA;
	}

	// If health is getting low, make it bright red
	if (m_iHealth <= 15)
		a = 255;

	if (m_iHealth > 25)
	{
		int r, g, b, nUnused;

		(gHUD.m_clrYellowish).GetColor(r, g, b, nUnused);
		clrHealth.SetColor(r, g, b, a);
	}
	else
	{
		clrHealth.SetColor(250, 0, 0, a);
	}

	int nFontWidth = GetNumberFontWidth();
	int nFontHeight = GetNumberFontHeight();
	int nCrossWidth = icon_cross->Width();

	x = nCrossWidth / 2;
	y = GetTall() - (nFontHeight * 1.5f);

	icon_cross->DrawSelf(x, y, clrHealth);

	x = nCrossWidth + (nFontWidth / 2);

	x = DrawHudNumber(x, y, m_iHealth, clrHealth);

	x += nFontWidth / 2;

	int iHeight = nFontHeight;
	int iWidth = nFontWidth / 10;

	clrHealth.SetColor(255, 160, 0, a);
	vgui::surface()->DrawSetColor(clrHealth);
	vgui::surface()->DrawFilledRect(x, y, x + iWidth, y + iHeight);
}

void HUD_HealthMeter::MsgFunc_Damage( bf_read &msg )
{
	msg.ReadByte();	// armor
	msg.ReadByte();	// health
	msg.ReadLong();	// damage bits

	Vector vecFrom;

	vecFrom.x = msg.ReadBitCoord();
	vecFrom.y = msg.ReadBitCoord();
	vecFrom.z = msg.ReadBitCoord();

	/*
	int armor = msg.ReadByte();	// armor
	int damageTaken = msg.ReadByte();	// health
	long bitsDamage = msg.ReadLong(); // damage bits
	bitsDamage; // variable still sent but not used

	Vector vecFrom;

	vecFrom.x = msg.ReadBitCoord();
	vecFrom.y = msg.ReadBitCoord();
	vecFrom.z = msg.ReadBitCoord();

	// Actually took damage?
	if (damageTaken > 0 || armor > 0)
	{
		if (damageTaken > 0)
		{
			// start the animation
			g_pClientMode->GetViewportAnimationController()->StartAnimationSequence("HealthDamageTaken");
		}
	}
	*/
}

void HUD_HealthMeter::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(true);
}

//-----------------------------------------------------------------------------
// Purpose: Armour panel
//-----------------------------------------------------------------------------
class HUD_SuitMeter : public CHudElement, public HUD_OldStyleNumeric
{
	DECLARE_CLASS_SIMPLE(HUD_SuitMeter, HUD_OldStyleNumeric);

public:
	HUD_SuitMeter(const char *pElementName);

	virtual void	Init(void);
	virtual void	Reset(void);
	virtual void	VidInit(void);
	void			MsgFunc_Battery(bf_read &msg);

private:
	virtual void	Paint(void);
	void	ApplySchemeSettings(vgui::IScheme *pScheme);

private:
	CHudTexture		*icon_suit_empty;
	CHudTexture		*icon_suit_full;
	int				m_iBattery;
	float			m_flFade;
};

DECLARE_HUDELEMENT(HUD_SuitMeter);
DECLARE_HUD_MESSAGE(HUD_SuitMeter, Battery);

HUD_SuitMeter::HUD_SuitMeter(const char *pElementName) : CHudElement(pElementName), BaseClass(NULL, "HudSuitDI")
{
	SetHiddenBits(HIDEHUD_HEALTH | HIDEHUD_NEEDSUIT);
}

void HUD_SuitMeter::Init()
{
	HOOK_HUD_MESSAGE(HUD_SuitMeter, Battery);
	Reset();
}

void HUD_SuitMeter::Reset()
{
	m_iBattery = 0;
	m_flFade = 0;
}

void HUD_SuitMeter::VidInit()
{
	Reset();
	BaseClass::VidInit();
}

void HUD_SuitMeter::Paint()
{
	Color	clrHealth;
	int		a;
	int		x;
	int		y;

	BaseClass::Paint();

	if (!icon_suit_empty)
	{
		icon_suit_empty = gHUD.GetIcon("suit_empty");
	}

	if (!icon_suit_full)
	{
		icon_suit_full = gHUD.GetIcon("suit_full");
	}

	if (!icon_suit_empty || !icon_suit_full)
	{
		return;
	}

	// Has health changed? Flash the health #
	if (m_flFade)
	{
		if (m_flFade > FADE_TIME)
			m_flFade = FADE_TIME;

		m_flFade -= (gpGlobals->frametime * 20);
		if (m_flFade <= 0)
		{
			a = 128;
			m_flFade = 0;
		}
		else
		{
			// Fade the health number back to dim
			a = MIN_ALPHA + (m_flFade / FADE_TIME) * 128;
		}
	}
	else
	{
		a = MIN_ALPHA;
	}

	int r, g, b, nUnused;
	(gHUD.m_clrYellowish).GetColor(r, g, b, nUnused);
	clrHealth.SetColor(r, g, b, a);

	int nFontWidth = GetNumberFontWidth();
	int nFontHeight = GetNumberFontHeight();

	int nHudElemWidth, nHudElemHeight;
	GetSize(nHudElemWidth, nHudElemHeight);

	int iOffset = icon_suit_empty->Height() / 6;

	x = icon_suit_empty->Width() / 2; //nHudElemWidth / 5;

	y = GetTall() - (nFontHeight * 1.5f);

	icon_suit_empty->DrawSelf(x, y - iOffset, clrHealth);

	if (m_iBattery > 0)
	{
		int nSuitOffset = icon_suit_full->Height() * ((float)(100 - (MIN(100, m_iBattery))) * 0.01);	// battery can go from 0 to 100 so * 0.01 goes from 0 to 1
		icon_suit_full->DrawSelfCropped(x, y - iOffset + nSuitOffset, 0, nSuitOffset, icon_suit_full->Width(), icon_suit_full->Height() - nSuitOffset, clrHealth);
	}

	x += icon_suit_empty->Width() + (nFontWidth / 2);
	DrawHudNumber(x, y, m_iBattery, clrHealth);

	if (m_iBattery > 0)
	{
		int alpha = hud_transparency.GetInt();
		SetAlpha(alpha);
	}
}

void HUD_SuitMeter::MsgFunc_Battery(bf_read &msg)
{
	int x = msg.ReadShort();

	if (x != m_iBattery)
	{
		m_flFade = FADE_TIME;
		m_iBattery = x;
	}
}

void HUD_SuitMeter::ApplySchemeSettings(vgui::IScheme *pScheme)
{
	BaseClass::ApplySchemeSettings(pScheme);
	SetPaintBackgroundEnabled(true);
}
#endif