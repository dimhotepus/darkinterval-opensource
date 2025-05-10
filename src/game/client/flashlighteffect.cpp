//========================================================================//
//
// Purpose: 
//
//===========================================================================//

#include "cbase.h"
#include "flashlighteffect.h"
#include "dlight.h"
#include "iefx.h"
#include "iviewrender.h"
#include "view.h"
#include "engine/ivdebugoverlay.h"
#include "tier0/vprof.h"
#include "tier1/keyvalues.h"
#include "toolframework_client.h"

#ifdef HL2_CLIENT_DLL
#include "c_basehlplayer.h"
#endif // HL2_CLIENT_DLL

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#ifdef DARKINTERVAL
extern ConVar r_flashlight_resolution; // depthres likes to default/not save, so we use a separate one
#else
extern ConVar r_flashlightdepthres;
#endif
void r_newflashlightCallback_f(IConVar *pConVar, const char *pOldString, float flOldValue);
#ifdef DARKINTERVAL
static ConVar r_flashlight_episodic("r_flashlight_projectedlight", "1", FCVAR_NONE, "", r_newflashlightCallback_f);
static ConVar r_flashlight_fov("r_flashlight_fov", "65.0"); // fov also likes to get defaulted, so use a separate one
static ConVar r_flashlight_shadows("r_flashlight_shadows", "0", FCVAR_ARCHIVE, "");

ConVar r_flashlight_force_flicker("r_flashlight_force_flicker", "0");
#else // reducing amount of convars
static ConVar r_newflashlight("r_newflashlight", "1", FCVAR_CHEAT, "", r_newflashlightCallback_f);
static ConVar r_swingflashlight("r_swingflashlight", "1", FCVAR_CHEAT);
static ConVar r_flashlightlockposition("r_flashlightlockposition", "0", FCVAR_CHEAT);
static ConVar r_flashlightfov("r_flashlightfov", "45.0", FCVAR_CHEAT);
static ConVar r_flashlightoffsetx("r_flashlightoffsetx", "10.0", FCVAR_CHEAT);
static ConVar r_flashlightoffsety("r_flashlightoffsety", "-20.0", FCVAR_CHEAT);
static ConVar r_flashlightoffsetz("r_flashlightoffsetz", "24.0", FCVAR_CHEAT);
static ConVar r_flashlightnear("r_flashlightnear", "4.0", FCVAR_CHEAT);
static ConVar r_flashlightfar("r_flashlightfar", "750.0", FCVAR_CHEAT);
static ConVar r_flashlightconstant("r_flashlightconstant", "0.0", FCVAR_CHEAT);
static ConVar r_flashlightlinear("r_flashlightlinear", "100.0", FCVAR_CHEAT);
static ConVar r_flashlightquadratic("r_flashlightquadratic", "0.0", FCVAR_CHEAT);
static ConVar r_flashlightvisualizetrace("r_flashlightvisualizetrace", "0", FCVAR_CHEAT);
static ConVar r_flashlightambient("r_flashlightambient", "0.0", FCVAR_CHEAT);
static ConVar r_flashlightshadowatten("r_flashlightshadowatten", "0.35", FCVAR_CHEAT);
static ConVar r_flashlightladderdist("r_flashlightladderdist", "40.0", FCVAR_CHEAT);

#endif
#ifdef DARKINTERVAL // from Mapbase
extern ConVarRef mat_slopescaledepthbias_shadowmap;
extern ConVarRef mat_depthbias_shadowmap;
#else
static ConVar mat_slopescaledepthbias_shadowmap("mat_slopescaledepthbias_shadowmap", "16", FCVAR_NONE);
static ConVar mat_depthbias_shadowmap("mat_depthbias_shadowmap", "0.00005", FCVAR_NONE);
#endif

#ifndef DARKINTERVAL // the "feeler" is when the flashlight traces in front of it and gets shorter/wider when you're close to walls
#define USE_FLASHLIGHT_FEELER 1 // DI doesn't want that to happen, personal preference
#else
#define USE_FLASHLIGHT_FEELER 0
#endif

void r_newflashlightCallback_f(IConVar *pConVar, const char *pOldString, float flOldValue)
{
	if (engine->GetDXSupportLevel() < 70) // todo - this is meaningless, it can't go below 80 in Orange Box branch
	{
		r_flashlight_episodic.SetValue(0);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nEntIndex - The m_nEntIndex of the client entity that is creating us.
//			vecPos - The position of the light emitter.
//			vecDir - The direction of the light emission.
//-----------------------------------------------------------------------------
CFlashlightEffect::CFlashlightEffect(int nEntIndex, const char *pszFlashlightTexture)
{
	m_FlashlightHandle = CLIENTSHADOW_INVALID_HANDLE;
	m_nEntIndex = nEntIndex;

	m_bIsOn = false;
	m_pPointLight = NULL;
	if (engine->GetDXSupportLevel() < 70)
	{
		r_flashlight_episodic.SetValue(0);
	}
#ifdef DARKINTERVAL
	if (pszFlashlightTexture == NULL || !*pszFlashlightTexture)
#endif
	{
		if (g_pMaterialSystemHardwareConfig->SupportsBorderColor())
		{
			pszFlashlightTexture = "effects/flashlight_border";
		}
		else
		{
			pszFlashlightTexture = "effects/flashlight001";
		}
	}
#ifdef DARKINTERVAL
	m_FlashlightTexture.Init(pszFlashlightTexture, TEXTURE_GROUP_OTHER, true);
#endif
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CFlashlightEffect::~CFlashlightEffect()
{
	LightOff();
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFlashlightEffect::TurnOn()
{
	m_bIsOn = true;
	m_flDistMod = 1.0f;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFlashlightEffect::TurnOff()
{
	if (m_bIsOn)
	{
		m_bIsOn = false;
		LightOff();
	}
}

// Custom trace filter that skips the player and the view model.
// If we don't do this, we'll end up having the light right in front of us all
// the time.
class CTraceFilterSkipPlayerAndViewModel : public CTraceFilter
{
public:
	virtual bool ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask)
	{
		C_BaseEntity *pEntity = EntityFromEntityHandle(pServerEntity);
		if (!pEntity)
			return true;

		if ((dynamic_cast<C_BaseViewModel *>(pEntity) != NULL) ||
			(dynamic_cast<C_BasePlayer *>(pEntity) != NULL) ||
			pEntity->GetCollisionGroup() == COLLISION_GROUP_DEBRIS ||
			pEntity->GetCollisionGroup() == COLLISION_GROUP_INTERACTIVE_DEBRIS)
		{
			return false;
		}

		return true;
	}
};

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
#ifdef DARKINTERVAL
void CFlashlightEffect::UpdateLightNew(const Vector &vecPos, const Vector &vecForward, const Vector &vecRight, const Vector &vecUp, bool bUseOffset)
#else
void CFlashlightEffect::UpdateLightNew(const Vector &vecPos, const Vector &vecForward, const Vector &vecRight, const Vector &vecUp)
#endif

{
	VPROF_BUDGET("CFlashlightEffect::UpdateLightNew", VPROF_BUDGETGROUP_SHADOW_DEPTH_TEXTURING);

	FlashlightState_t state;

#if USE_FLASHLIGHT_FEELER // DI REMOVE: unpleasant oscillations of the flashlight when near things
	// We will lock some of the flashlight params if player is on a ladder, to prevent oscillations due to the trace-rays
	bool bPlayerOnLadder = (C_BasePlayer::GetLocalPlayer()->GetMoveType() == MOVETYPE_LADDER);

	const float flEpsilon = 0.1f;			// Offset flashlight position along vecUp
	const float flDistCutoff = 128.0f;
	const float flDistDrag = 0.2;

	CTraceFilterSkipPlayerAndViewModel traceFilter;
#ifdef DARKINTERVAL
	float flOffsetY = bUseOffset ? -20.0f : 0.0f;
#else
	float flOffsetY = r_flashlightoffsety.GetFloat();
#endif
	if (false)
	{
		// This projects the view direction backwards, attempting to raise the vertical
		// offset of the flashlight, but only when the player is looking down.
		Vector vecSwingLight = vecPos + vecForward * -12.0f;
		if (vecSwingLight.z > vecPos.z)
		{
			flOffsetY += (vecSwingLight.z - vecPos.z);
		}
	}

	Vector vOrigin = vecPos + flOffsetY * vecUp;

	// Not on ladder...trace a hull
#ifdef DARKINTERVAL
	if (!bPlayerOnLadder && !bUseOffset)
#else
	if ( !bPlayerOnLadder )
#endif
	{
		trace_t pmOriginTrace;
		UTIL_TraceHull(vecPos, vOrigin, Vector(-4, -4, -4), Vector(4, 4, 4), MASK_SOLID & ~(CONTENTS_HITBOX), &traceFilter, &pmOriginTrace);

		if (pmOriginTrace.DidHit())
		{
			vOrigin = vecPos;
		}
	}
	else // on ladder...skip the above hull trace
	{
		vOrigin = vecPos;
	}

	// Now do a trace along the flashlight direction to ensure there is nothing within range to pull back from
	int iMask = MASK_OPAQUE_AND_NPCS;
	iMask &= ~CONTENTS_HITBOX;
	iMask |= CONTENTS_WINDOW;
#ifdef DARKINTERVAL
	Vector vTarget = vecPos + vecForward * 1500;
#else
	Vector vTarget = vecPos + vecForward * r_flashlightfar.GetFloat();
#endif
	// Work with these local copies of the basis for the rest of the function
	Vector vDir = vTarget - vOrigin;
	Vector vRight = vecRight;
	Vector vUp = vecUp;
	VectorNormalize(vDir);
	VectorNormalize(vRight);
	VectorNormalize(vUp);

	// Orthonormalize the basis, since the flashlight texture projection will require this later...
	vUp -= DotProduct(vDir, vUp) * vDir;
	VectorNormalize(vUp);
	vRight -= DotProduct(vDir, vRight) * vDir;
	VectorNormalize(vRight);
	vRight -= DotProduct(vUp, vRight) * vUp;
	VectorNormalize(vRight);

	AssertFloatEquals(DotProduct(vDir, vRight), 0.0f, 1e-3);
	AssertFloatEquals(DotProduct(vDir, vUp), 0.0f, 1e-3);
	AssertFloatEquals(DotProduct(vRight, vUp), 0.0f, 1e-3);

	trace_t pmDirectionTrace;
	UTIL_TraceHull(vOrigin, vTarget, Vector(-4, -4, -4), Vector(4, 4, 4), iMask, &traceFilter, &pmDirectionTrace);

#ifdef DARKINTERVAL
	if (false)
#else
	if ( r_flashlightvisualizetrace.GetBool() == true )
#endif
	{
		debugoverlay->AddBoxOverlay(pmDirectionTrace.endpos, Vector(-4, -4, -4), Vector(4, 4, 4), QAngle(0, 0, 0), 0, 0, 255, 16, 0);
		debugoverlay->AddLineOverlay(vOrigin, pmDirectionTrace.endpos, 255, 0, 0, false, 0);
	}

	float flDist = (pmDirectionTrace.endpos - vOrigin).Length();
	if (flDist < flDistCutoff)
	{
		// We have an intersection with our cutoff range
		// Determine how far to pull back, then trace to see if we are clear
#ifdef DARKINTERVAL
		float flPullBackDist = bPlayerOnLadder ? 40.0f : flDistCutoff - flDist;	// Fixed pull-back distance if on ladder
#else
		float flPullBackDist = bPlayerOnLadder ? r_flashlightladderdist.GetFloat() : flDistCutoff - flDist;
#endif
		m_flDistMod = Lerp(flDistDrag, m_flDistMod, flPullBackDist);

		if (!bPlayerOnLadder)
		{
			trace_t pmBackTrace;
			UTIL_TraceHull(vOrigin, vOrigin - vDir*(flPullBackDist - flEpsilon), Vector(-4, -4, -4), Vector(4, 4, 4), iMask, &traceFilter, &pmBackTrace);
			if (pmBackTrace.DidHit())
			{
				// We have an intersection behind us as well, so limit our m_flDistMod
				float flMaxDist = (pmBackTrace.endpos - vOrigin).Length() - flEpsilon;
				if (m_flDistMod > flMaxDist)
					m_flDistMod = flMaxDist;
			}
		}
	}
	else
	{
		m_flDistMod = Lerp(flDistDrag, m_flDistMod, 0.0f);
	}
	vOrigin = vOrigin - vDir * m_flDistMod;
#else
//	float flOffsetY = bUseOffset -20.0f;
	float flOffsetY = -4.0f;

	if (false)
	{
		// This projects the view direction backwards, attempting to raise the vertical
		// offset of the flashlight, but only when the player is looking down.
		Vector vecSwingLight = vecPos + vecForward * -12.0f;
		if (vecSwingLight.z > vecPos.z)
		{
			flOffsetY += (vecSwingLight.z - vecPos.z);
		}
	}

	Vector vOrigin = vecPos + flOffsetY * vecUp;
	Vector vTarget = vecPos + vecForward * 1500;
	Vector vDir = vTarget - vOrigin;
	// Work with these local copies of the basis for the rest of the function
	Vector vRight = vecRight;
	Vector vUp = vecUp;
	VectorNormalize(vDir);
	VectorNormalize(vRight);
	VectorNormalize(vUp);
	// Orthonormalize the basis, since the flashlight texture projection will require this later...
	vUp -= DotProduct(vDir, vUp) * vDir;
	VectorNormalize(vUp);
	vRight -= DotProduct(vDir, vRight) * vDir;
	VectorNormalize(vRight);
	vRight -= DotProduct(vUp, vRight) * vUp;
	VectorNormalize(vRight);
#endif

	state.m_vecLightOrigin = vOrigin;

	BasisToQuaternion(vDir, vRight, vUp, state.m_quatOrientation);
#ifdef DARKINTERVAL
	state.m_fQuadraticAtten = 0.0;
#else
	state.m_fQuadraticAtten = r_flashlightquadratic.GetFloat();
#endif
	bool bFlicker = false;

	C_BaseHLPlayer *pPlayer = (C_BaseHLPlayer *)C_BasePlayer::GetLocalPlayer();
	if (pPlayer)
	{
		float flBatteryPower = (pPlayer->m_HL2Local.m_flFlashBattery >= 0.0f) ? (pPlayer->m_HL2Local.m_flFlashBattery) : pPlayer->m_HL2Local.m_flSuitPower;
#ifdef DARKINTERVAL
		if (flBatteryPower <= 10.0f || pPlayer->m_HL2Local.m_flashlight_force_flicker_bool || r_flashlight_force_flicker.GetBool())
#else
		if (flBatteryPower <= 10.0f )
#endif
		{
			float flScale;
			if (flBatteryPower >= 0.0f)
			{
				flScale = (flBatteryPower <= 4.5f) ? SimpleSplineRemapVal(flBatteryPower, 4.5f, 0.0f, 1.0f, 0.0f) : 1.0f;
			}
			else
			{
				flScale = SimpleSplineRemapVal(flBatteryPower, 10.0f, 4.8f, 1.0f, 0.0f);
			}

			flScale = clamp(flScale, 0.0f, 1.0f);
#ifdef DARKINTERVAL
			if (flScale < 0.35f || pPlayer->m_HL2Local.m_flashlight_force_flicker_bool || r_flashlight_force_flicker.GetBool())
#else
			if (flScale < 0.35f)
#endif
			{
				float flFlicker = cosf(CURTIME * 6.0f) * sinf(CURTIME * 15.0f);

				if (flFlicker > 0.25f && flFlicker < 0.75f)
				{
					// On
#ifdef DARKINTERVAL
					state.m_fLinearAtten = 200.0 * flScale;
#else
					state.m_fLinearAtten = r_flashlightlinear.GetFloat() * flScale;
#endif
				}
				else
				{
					// Off
					state.m_fLinearAtten = 0.0f;
				}
			}
			else
			{
				float flNoise = cosf(CURTIME * 7.0f) * sinf(CURTIME * 25.0f);
#ifdef DARKINTERVAL
				state.m_fLinearAtten = 200.0 * flScale + 1.5f * flNoise;
#else
				state.m_fLinearAtten = r_flashlightlinear.GetFloat() * flScale + 1.5f * flNoise;
#endif
			}
#ifdef DARKINTERVAL
			state.m_fHorizontalFOVDegrees = r_flashlight_fov.GetFloat() - (16.0f * (1.0f - flScale));
			state.m_fVerticalFOVDegrees = r_flashlight_fov.GetFloat() - (16.0f * (1.0f - flScale));
#else
			state.m_fHorizontalFOVDegrees = r_flashlightfov.GetFloat() - ( 16.0f * ( 1.0f - flScale ) );
			state.m_fVerticalFOVDegrees = r_flashlightfov.GetFloat() - ( 16.0f * ( 1.0f - flScale ) );
#endif
			bFlicker = true;
		}
	}

	if (bFlicker == false)
	{
#ifdef DARKINTERVAL
		state.m_fLinearAtten = 200.0;
		state.m_fHorizontalFOVDegrees = r_flashlight_fov.GetFloat();
		state.m_fVerticalFOVDegrees = r_flashlight_fov.GetFloat();
#else
		state.m_fLinearAtten = r_flashlightlinear.GetFloat();
		state.m_fHorizontalFOVDegrees = r_flashlightfov.GetFloat();
		state.m_fVerticalFOVDegrees = r_flashlightfov.GetFloat();
#endif
	}
#ifdef DARKINTERVAL
	state.m_fConstantAtten = 0.0;
	state.m_Color[0] = CBasePlayer::GetLocalPlayer()->IsSuitEquipped() ? 1.0f : 0.75;
	state.m_Color[1] = CBasePlayer::GetLocalPlayer()->IsSuitEquipped() ? 1.0f : 0.72;
	state.m_Color[2] = CBasePlayer::GetLocalPlayer()->IsSuitEquipped() ? 1.0f : 0.5;
	state.m_Color[3] = 0.0f;

	state.m_NearZ = 1.0f;
	
	state.m_FarZ = CBasePlayer::GetLocalPlayer()->IsSuitEquipped() ?
		1500 : 1300 ;

	state.m_bEnableShadows = r_flashlight_shadows.GetBool();
	state.m_nShadowMapResolution = r_flashlight_resolution.GetInt();
#else
	state.m_fConstantAtten = r_flashlightconstant.GetFloat();
	state.m_Color[ 0 ] = 1.0f;
	state.m_Color[ 1 ] = 1.0f;
	state.m_Color[ 2 ] = 1.0f;
	state.m_Color[ 3 ] = r_flashlightambient.GetFloat();
	state.m_NearZ = r_flashlightnear.GetFloat() + m_flDistMod;	// Push near plane out so that we don't clip the world when the flashlight pulls back 
	state.m_FarZ = r_flashlightfar.GetFloat();
	state.m_bEnableShadows = r_flashlightdepthtexture.GetBool();
	state.m_flShadowMapResolution = r_flashlightdepthres.GetInt();
#endif // DARKINTERVAL
	state.m_pSpotlightTexture = m_FlashlightTexture;
	state.m_nSpotlightTextureFrame = 0;
#ifdef DARKINTERVAL
	state.m_flShadowAtten = 0.0f; // todo - consider 0.35f like the convar?
#else
	state.m_flShadowAtten = r_flashlightshadowatten.GetFloat();
#endif
	state.m_flShadowSlopeScaleDepthBias = mat_slopescaledepthbias_shadowmap.GetFloat();
	state.m_flShadowDepthBias = mat_depthbias_shadowmap.GetFloat();

	if (m_FlashlightHandle == CLIENTSHADOW_INVALID_HANDLE)
	{
		m_FlashlightHandle = g_pClientShadowMgr->CreateFlashlight(state);
	}
	else
	{
#ifndef DARKINTERVAL
		if ( !r_flashlightlockposition.GetBool() )
#endif
			g_pClientShadowMgr->UpdateFlashlightState(m_FlashlightHandle, state);
	}

	g_pClientShadowMgr->UpdateProjectedTexture(m_FlashlightHandle, true);
	
	// Kill the old flashlight method if we have one.
	LightOffOld();

#ifndef NO_TOOLFRAMEWORK
	if (clienttools->IsInRecordingMode())
	{
		KeyValues *msg = new KeyValues("FlashlightState");
		msg->SetFloat("time", CURTIME);
		msg->SetInt("entindex", m_nEntIndex);
		msg->SetInt("flashlightHandle", m_FlashlightHandle);
		msg->SetPtr("flashlightState", &state);
		ToolFramework_PostToolMessage(HTOOLHANDLE_INVALID, msg);
		msg->deleteThis();
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
void CFlashlightEffect::UpdateLightOld(const Vector &vecPos, const Vector &vecDir, int nDistance)
{
	if (!m_pPointLight || (m_pPointLight->key != m_nEntIndex))
	{
		// Set up the environment light
		m_pPointLight = effects->CL_AllocDlight(m_nEntIndex);
		m_pPointLight->flags = 0.0f;
#ifndef DARKINTERVAL
		m_pPointLight->radius = 128;
#else
		m_pPointLight->radius = 80;
#endif
	}

	// For bumped lighting
	VectorCopy(vecDir, m_pPointLight->m_Direction);

	Vector end;
	end = vecPos + nDistance * vecDir;

	// Trace a line outward, skipping the player model and the view model.
	trace_t pm;
	CTraceFilterSkipPlayerAndViewModel traceFilter;
	UTIL_TraceLine(vecPos, end, MASK_ALL, &traceFilter, &pm);
	VectorCopy(pm.endpos, m_pPointLight->origin);

	float falloff = pm.fraction * nDistance;

	if (falloff < 500)
		falloff = 1.0;
	else
		falloff = 500.0 / falloff;

	falloff *= falloff;
#ifdef DARKINTERVAL
//	Vector vecColor;
//	GetColorForSurface(&pm, &vecColor);
//	Msg("vecColor is %i %i %i\n", (int)vecColor.x, (int)vecColor.y, (int)vecColor.z);
//	m_pPointLight->radius = 128;
	m_pPointLight->color.r = 220 /** vecColor.x * falloff*/;
	m_pPointLight->color.g = 215 /** vecColor.y * falloff*/;
	m_pPointLight->color.b = 200 /** vecColor.x * falloff*/;
	m_pPointLight->color.exponent = 0;
#else
	m_pPointLight->radius = 80;
	m_pPointLight->color.r = m_pPointLight->color.g = m_pPointLight->color.b = 255 * falloff;
#endif
	// Make it live for a bit
	m_pPointLight->die = CURTIME + 0.2f;

	// Update list of surfaces we influence
	render->TouchLight(m_pPointLight);

	// kill the new flashlight if we have one
	LightOffNew();
}

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
#ifdef DARKINTERVAL
void CFlashlightEffect::UpdateLight(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance, bool bUseOffset)
#else
void CFlashlightEffect::UpdateLight(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance)
#endif
{
	if (!m_bIsOn)
	{
		return;
	}
#ifdef DARKINTERVAL
	if (r_flashlight_episodic.GetBool())
	{
		UpdateLightNew(vecPos, vecDir, vecRight, vecUp, bUseOffset);
	}
#else
	if ( r_newflashlight.GetBool() )
	{
		UpdateLightNew(vecPos, vecDir, vecRight, vecUp);
	}
#endif
	else
	{
		UpdateLightOld(vecPos, vecDir, nDistance);
	}	
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFlashlightEffect::LightOffNew()
{
#ifndef NO_TOOLFRAMEWORK
	if (clienttools->IsInRecordingMode())
	{
		KeyValues *msg = new KeyValues("FlashlightState");
		msg->SetFloat("time", CURTIME);
		msg->SetInt("entindex", m_nEntIndex);
		msg->SetInt("flashlightHandle", m_FlashlightHandle);
		msg->SetPtr("flashlightState", NULL);
		ToolFramework_PostToolMessage(HTOOLHANDLE_INVALID, msg);
		msg->deleteThis();
	}
#endif

	// Clear out the light
	if (m_FlashlightHandle != CLIENTSHADOW_INVALID_HANDLE)
	{
		g_pClientShadowMgr->DestroyFlashlight(m_FlashlightHandle);
		m_FlashlightHandle = CLIENTSHADOW_INVALID_HANDLE;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFlashlightEffect::LightOffOld()
{
	if (m_pPointLight && (m_pPointLight->key == m_nEntIndex))
	{
		m_pPointLight->die = CURTIME;
		m_pPointLight = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CFlashlightEffect::LightOff()
{
	LightOffOld();
	LightOffNew();
}

// Vehicle flashlights

CHeadlightEffect::CHeadlightEffect()
{

}

CHeadlightEffect::~CHeadlightEffect()
{

}
#ifdef DARKINTERVAL
void CHeadlightEffect::UpdateLight(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance, float colorR, float colorG, float colorB)
#else
void CHeadlightEffect::UpdateLight(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance)
#endif
{
	if (IsOn() == false)
		return;

	FlashlightState_t state;
	Vector basisX, basisY, basisZ;
	basisX = vecDir;
	basisY = vecRight;
	basisZ = vecUp;
	VectorNormalize(basisX);
	VectorNormalize(basisY);
	VectorNormalize(basisZ);

	BasisToQuaternion(basisX, basisY, basisZ, state.m_quatOrientation);

	state.m_vecLightOrigin = vecPos;
#ifdef DARKINTERVAL
	state.m_fHorizontalFOVDegrees = 75.0f;
	state.m_fVerticalFOVDegrees = 75.0f;
	state.m_fQuadraticAtten = 0.0;
	state.m_fLinearAtten = 100.0;
	state.m_fConstantAtten = 1.0;
	state.m_Color[0] = colorR;
	state.m_Color[1] = colorG;
	state.m_Color[2] = colorB;
	state.m_Color[3] = 0.0f;
	state.m_NearZ = 4.0f;
	state.m_FarZ = nDistance;
	state.m_bEnableShadows = false; // todo - this is done to avoid APC related flashlight bug in ch06 apcride. This needs to be revisited for Digger map
#else
	state.m_fHorizontalFOVDegrees = 45.0f;
	state.m_fVerticalFOVDegrees = 30.0f;
	state.m_fQuadraticAtten = r_flashlightquadratic.GetFloat();
	state.m_fLinearAtten = r_flashlightlinear.GetFloat();
	state.m_fConstantAtten = r_flashlightconstant.GetFloat();
	state.m_Color[ 0 ] = 1.0f;
	state.m_Color[ 1 ] = 1.0f;
	state.m_Color[ 2 ] = 1.0f;
	state.m_Color[ 3 ] = r_flashlightambient.GetFloat();
	state.m_NearZ = r_flashlightnear.GetFloat();
	state.m_FarZ = r_flashlightfar.GetFloat();
	state.m_bEnableShadows = true;
#endif
	state.m_pSpotlightTexture = m_FlashlightTexture;
	state.m_nSpotlightTextureFrame = 0;

	if (GetFlashlightHandle() == CLIENTSHADOW_INVALID_HANDLE)
	{
		SetFlashlightHandle(g_pClientShadowMgr->CreateFlashlight(state));
	}
	else
	{
		g_pClientShadowMgr->UpdateFlashlightState(GetFlashlightHandle(), state);
	}

	g_pClientShadowMgr->UpdateProjectedTexture(GetFlashlightHandle(), true);
}

// world flashlights (replacement of env_projectedtexture)
#ifdef DARKINTERVAL
CProjectedLightEffect::CProjectedLightEffect()
{

}

CProjectedLightEffect::~CProjectedLightEffect()
{

}

void CProjectedLightEffect::UpdateLight(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, float fDistance, float fFov, Vector vAttenCombo, Vector vColorCombo)
{
	if (IsOn() == false)
		return;

	FlashlightState_t state;
	Vector basisX, basisY, basisZ;
	basisX = vecDir;
	basisY = vecRight;
	basisZ = vecUp;
	VectorNormalize(basisX);
	VectorNormalize(basisY);
	VectorNormalize(basisZ);

	BasisToQuaternion(basisX, basisY, basisZ, state.m_quatOrientation);

	state.m_vecLightOrigin = vecPos;

	state.m_fHorizontalFOVDegrees = fFov;
	state.m_fVerticalFOVDegrees = fFov;
	state.m_fQuadraticAtten = vAttenCombo.x;
	state.m_fLinearAtten = vAttenCombo.y;
	state.m_fConstantAtten = vAttenCombo.z;
	state.m_Color[0] = vColorCombo.x;
	state.m_Color[1] = vColorCombo.y;
	state.m_Color[2] = vColorCombo.z;
	state.m_Color[3] = 0.0f;
	state.m_NearZ = 4.0f;
	state.m_FarZ = fDistance;
	state.m_bEnableShadows = true;
	state.m_nShadowMapResolution = 1024;
	state.m_pSpotlightTexture = m_FlashlightTexture;
	state.m_nSpotlightTextureFrame = 0;	
	state.m_flShadowAtten = 1.0f;
	state.m_flShadowSlopeScaleDepthBias = mat_slopescaledepthbias_shadowmap.GetFloat();
	state.m_flShadowDepthBias = mat_depthbias_shadowmap.GetFloat();

	if (GetFlashlightHandle() == CLIENTSHADOW_INVALID_HANDLE)
	{
		SetFlashlightHandle(g_pClientShadowMgr->CreateFlashlight(state));
	}
	else
	{
		g_pClientShadowMgr->UpdateFlashlightState(GetFlashlightHandle(), state);
	}

	g_pClientShadowMgr->UpdateProjectedTexture(GetFlashlightHandle(), true);
}
#endif // DARKINTERVAL