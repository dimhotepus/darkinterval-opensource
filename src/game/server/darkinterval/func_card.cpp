//========================================================================//
// DI NEW: simpler version of areaportal window that doesn't affect visibility
// at all, all it does is controls opacity of another brush model, so we can
// have things appear from far away and fade out when we get closer to them - 
// - ideal for fake fog cards and things like that.
//=============================================================================//

#include "cbase.h"
#include "func_card.h"
#include "entitylist.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(func_card, CFuncCard);


IMPLEMENT_SERVERCLASS_ST(CFuncCard, DT_FuncCard)
SendPropFloat(SENDINFO(m_flFadeDist), 0, SPROP_NOSCALE),
SendPropFloat(SENDINFO(m_flFadeStartDist), 0, SPROP_NOSCALE),
SendPropBool(SENDINFO(m_bUseFOV)),
SendPropFloat(SENDINFO(m_flTranslucencyLimit), 0, SPROP_NOSCALE),

SendPropModelIndex(SENDINFO(m_iBackgroundModelIndex)),
END_SEND_TABLE()


BEGIN_DATADESC(CFuncCard)

DEFINE_KEYFIELD(m_flFadeStartDist, FIELD_FLOAT, "FadeStartDist"),
DEFINE_KEYFIELD(m_flFadeDist, FIELD_FLOAT, "FadeDist"),
DEFINE_KEYFIELD(m_flTranslucencyLimit, FIELD_FLOAT, "TranslucencyLimit"),
DEFINE_KEYFIELD(m_iBackgroundBModelName, FIELD_STRING, "BackgroundBModel"),
DEFINE_KEYFIELD(m_bUseFOV, FIELD_BOOLEAN, "UseFOV"),

DEFINE_INPUTFUNC(FIELD_FLOAT, "SetFadeStartDistance", InputSetFadeStartDistance),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetFadeEndDistance", InputSetFadeEndDistance),

END_DATADESC()

CFuncCard::CFuncCard()
{
	m_iBackgroundModelIndex = -1;
}

CFuncCard::~CFuncCard()
{
}

void CFuncCard::Spawn()
{
	Precache();
}

void CFuncCard::Activate()
{
	BaseClass::Activate();

	// Find our background model.
	CBaseEntity *pBackground = gEntList.FindEntityByName(NULL, m_iBackgroundBModelName);
	if (pBackground)
	{
		m_iBackgroundModelIndex = modelinfo->GetModelIndex(STRING(pBackground->GetModelName()));
		pBackground->AddEffects(EF_NODRAW); // we will draw for it.
	}

	// Find our target and steal its bmodel.
	CBaseEntity *pTarget = gEntList.FindEntityByName(NULL, m_target);
	if (pTarget)
	{
		SetModel(STRING(pTarget->GetModelName()));
		SetAbsOrigin(pTarget->GetAbsOrigin());
		pTarget->AddEffects(EF_NODRAW); // we will draw for it.
	}
}

/*
bool CFuncCard::IsWindowOpen(const Vector &vOrigin, float fovDistanceAdjustFactor)
{
	float flDist = CollisionProp()->CalcDistanceFromPoint(vOrigin);
	flDist *= fovDistanceAdjustFactor;
	return (flDist <= (m_flFadeDist + FADE_DIST_BUFFER));
}

bool CFuncCard::UpdateVisibility(const Vector &vOrigin, float fovDistanceAdjustFactor, bool &bIsOpenOnClient)
{
	if (IsWindowOpen(vOrigin, fovDistanceAdjustFactor))
	{
		return BaseClass::UpdateVisibility(vOrigin, fovDistanceAdjustFactor, bIsOpenOnClient);
	}
	else
	{
		bIsOpenOnClient = false;
		return false;
	}
}
*/

//-----------------------------------------------------------------------------
// Purpose: Changes the fade start distance 
// Input: float distance in inches
//-----------------------------------------------------------------------------
void CFuncCard::InputSetFadeStartDistance(inputdata_t &inputdata)
{
	m_flFadeStartDist = inputdata.value.Float();
}

//-----------------------------------------------------------------------------
// Purpose: Changes the fade end distance
// Input: float distance in inches
//-----------------------------------------------------------------------------
void CFuncCard::InputSetFadeEndDistance(inputdata_t &inputdata)
{
	m_flFadeDist = inputdata.value.Float();
}