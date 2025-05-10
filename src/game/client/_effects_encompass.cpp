// Memo:
//=============================================================================//
//
// Use this macro to register a client effect callback. 
// If you do DECLARE_CLIENT_EFFECT( "MyEffectName", MyCallback ), then MyCallback will be 
// called when the server does DispatchEffect( "MyEffect", data )
//
// #define DECLARE_CLIENT_EFFECT( effectName, callbackFunction ) \
// 	static CClientEffectRegistration ClientEffectReg_##callbackFunction( effectName, callbackFunction );

// void DispatchEffectToCallback(const char *pEffectName, const CEffectData &m_EffectData);
// void DispatchEffect(const char *pName, const CEffectData &data);
//=============================================================================//
#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "decals.h"
#include "dlight.h"
#include "fx.h"
#include "fx_impact.h"
#include "fx_line.h"
#include "fx_quad.h"
#include "fx_sparks.h"
#include "iefx.h"
#include "particles_localspace.h"
#include "vcollide_parse.h" // for gib hit/touch functions
#include "view.h"
#include "debugoverlay_shared.h"

#include "tier0/vprof.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern Vector GetTracerOrigin(const CEffectData &data);
extern void FX_TracerSound(const Vector &start, const Vector &end, int iTracerType);

// General precache of effects materials
CLIENTEFFECT_REGISTER_BEGIN(PrecacheTracers)
CLIENTEFFECT_MATERIAL("effects/gunshiptracer")
CLIENTEFFECT_MATERIAL("effects/huntertracer")
CLIENTEFFECT_MATERIAL("effects/blueblackflash")
CLIENTEFFECT_MATERIAL("effects/bluespark")
CLIENTEFFECT_MATERIAL("effects/muzzleflash1")
CLIENTEFFECT_MATERIAL("effects/muzzleflash2")
CLIENTEFFECT_MATERIAL("effects/muzzleflash3")
CLIENTEFFECT_MATERIAL("effects/muzzleflash4")
CLIENTEFFECT_MATERIAL("effects/combinemuzzle1")
CLIENTEFFECT_MATERIAL("effects/combinemuzzle2_nocull")
CLIENTEFFECT_MATERIAL("effects/tracer_middle")
CLIENTEFFECT_MATERIAL("effects/tracer_middle2")
CLIENTEFFECT_MATERIAL("effects/turret_tracer")
CLIENTEFFECT_MATERIAL("effects/strider_tracer")
CLIENTEFFECT_MATERIAL("effects/spark")
CLIENTEFFECT_MATERIAL("effects/spark_noz")
CLIENTEFFECT_MATERIAL("effects/fleck_cement1")
CLIENTEFFECT_MATERIAL("effects/fleck_cement2")
CLIENTEFFECT_MATERIAL("effects/tracer_base")
CLIENTEFFECT_REGISTER_END()
//

//-----------------------------------------------------------------------------
// General muzzleflash function
//-----------------------------------------------------------------------------
void CreateMuzzleflashELight(const Vector &origin, int exponent, int nMinRadius, int nMaxRadius, ClientEntityHandle_t hEntity)
{
//	if (muzzleflash_light.GetInt())
	{
		int entityIndex = ClientEntityList().HandleToEntIndex(hEntity);
		if (entityIndex >= 0)
		{
			dlight_t *el = effects->CL_AllocElight(LIGHT_INDEX_MUZZLEFLASH + entityIndex);

			el->origin = origin;

			el->color.r = 255;
			el->color.g = 192;
			el->color.b = 64;
			el->color.exponent = exponent;

			el->radius = random->RandomInt(nMinRadius, nMaxRadius);
			el->decay = el->radius / 0.05f;
			el->die = CURTIME + 0.1f;
		}
	}
}

//-----------------------------------------------------------------------------
// Specialised bullet impact FX callbacks
//-----------------------------------------------------------------------------
void ImpactJeepCallback(const CEffectData &data)
{
	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;
	C_BaseEntity *pEntity = ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);

	if (!pEntity)
	{
		// This happens for impacts that occur on an object that's then destroyed.
		// Clear out the fraction so it uses the server's data
		tr.fraction = 1.0;
		PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
		return;
	}

	// If we hit, perform our custom effects and play the sound
	if (Impact(vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr))
	{
		// Check for custom effects based on the Decal index
		PerformCustomEffects(vecOrigin, tr, vecShotDir, iMaterial, 2);
	}

	PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
}

DECLARE_CLIENT_EFFECT("ImpactJeep", ImpactJeepCallback);
//-----------------------------------------------------------------------------
void ImpactGaussCallback(const CEffectData &data)
{
	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;
	C_BaseEntity *pEntity = ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);

	if (!pEntity)
	{
		// This happens for impacts that occur on an object that's then destroyed.
		// Clear out the fraction so it uses the server's data
		tr.fraction = 1.0;
		PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
		return;
	}

	// If we hit, perform our custom effects and play the sound
	if (Impact(vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr))
	{
		// Check for custom effects based on the Decal index
		PerformCustomEffects(vecOrigin, tr, vecShotDir, iMaterial, 2);
	}

	PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
}

DECLARE_CLIENT_EFFECT("ImpactGauss", ImpactGaussCallback);
//-----------------------------------------------------------------------------
void ImpactCallback(const CEffectData &data)
{
	VPROF_BUDGET("ImpactCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;
	C_BaseEntity *pEntity = ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);

	if (!pEntity)
	{
		// This happens for impacts that occur on an object that's then destroyed.
		// Clear out the fraction so it uses the server's data
		tr.fraction = 1.0;
		PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
		return;
	}

	// If we hit, perform our custom effects and play the sound
	if (Impact(vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr))
	{
		// Check for custom effects based on the Decal index
		PerformCustomEffects(vecOrigin, tr, vecShotDir, iMaterial, 1.0);
	}

	PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
}

DECLARE_CLIENT_EFFECT("Impact", ImpactCallback);
//-----------------------------------------------------------------------------
void FX_AirboatGunImpact(const Vector &origin, const Vector &normal, float scale)
{
#ifdef _XBOX

	Vector offset = origin + (normal * 1.0f);

	CSmartPtr<CTrailParticles> sparkEmitter = CTrailParticles::Create("FX_MetalSpark 1");

	if (sparkEmitter == NULL)
		return;

	//Setup our information
	sparkEmitter->SetSortOrigin(offset);
	sparkEmitter->SetFlag(bitsPARTICLE_TRAIL_VELOCITY_DAMPEN);
	sparkEmitter->SetVelocityDampen(8.0f);
	sparkEmitter->SetGravity(800.0f);
	sparkEmitter->SetCollisionDamped(0.25f);
	sparkEmitter->GetBinding().SetBBox(offset - Vector(32, 32, 32), offset + Vector(32, 32, 32));

	int	numSparks = random->RandomInt(4, 8);

	TrailParticle	*pParticle;
	PMaterialHandle	hMaterial = sparkEmitter->GetPMaterial("effects/spark");
	Vector			dir;

	float	length = 0.1f;

	//Dump out sparks
	for (int i = 0; i < numSparks; i++)
	{
		pParticle = (TrailParticle *)sparkEmitter->AddParticle(sizeof(TrailParticle), hMaterial, offset);

		if (pParticle == NULL)
			return;

		pParticle->m_flLifetime = 0.0f;
		pParticle->m_flDieTime = random->RandomFloat(0.05f, 0.1f);

		float	spreadOfs = random->RandomFloat(0.0f, 2.0f);

		dir[0] = normal[0] + random->RandomFloat(-(0.5f*spreadOfs), (0.5f*spreadOfs));
		dir[1] = normal[1] + random->RandomFloat(-(0.5f*spreadOfs), (0.5f*spreadOfs));
		dir[2] = normal[2] + random->RandomFloat(-(0.5f*spreadOfs), (0.5f*spreadOfs));

		VectorNormalize(dir);

		pParticle->m_flWidth = random->RandomFloat(1.0f, 4.0f);
		pParticle->m_flLength = random->RandomFloat(length*0.25f, length);

		pParticle->m_vecVelocity = dir * random->RandomFloat((128.0f*(2.0f - spreadOfs)), (512.0f*(2.0f - spreadOfs)));

		Color32Init(pParticle->m_color, 255, 255, 255, 255);
	}

#else

	// Normal metal spark
	FX_MetalSpark(origin, normal, normal, (int)scale);

#endif // _XBOX

	// Add a quad to highlite the hit point
	FX_AddQuad(origin,
		normal,
		random->RandomFloat(16, 32),
		random->RandomFloat(32, 48),
		0.75f,
		1.0f,
		0.0f,
		0.4f,
		random->RandomInt(0, 360),
		0,
		Vector(1.0f, 1.0f, 1.0f),
		0.05f,
		"effects/combinemuzzle2_nocull",
		(FXQUAD_BIAS_SCALE | FXQUAD_BIAS_ALPHA));
}
//-----------------------------------------------------------------------------
void ImpactAirboatGunCallback(const CEffectData &data)
{
	VPROF_BUDGET("ImpactAirboatGunCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;
	C_BaseEntity *pEntity = ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);

	if (!pEntity)
	{
		// This happens for impacts that occur on an object that's then destroyed.
		// Clear out the fraction so it uses the server's data
		tr.fraction = 1.0;
		PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
		return;
	}

#if !defined( _XBOX )
	// If we hit, perform our custom effects and play the sound. Don't create decals
	if (Impact(vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr, IMPACT_NODECAL | IMPACT_REPORT_RAGDOLL_IMPACTS))
	{
		FX_AirboatGunImpact(vecOrigin, tr.plane.normal, 2);
	}
#else
	FX_AirboatGunImpact(vecOrigin, tr.plane.normal, 1);
#endif
}

DECLARE_CLIENT_EFFECT("AirboatGunImpact", ImpactAirboatGunCallback);
//-----------------------------------------------------------------------------
void ImpactHelicopterCallback(const CEffectData &data)
{
	VPROF_BUDGET("ImpactHelicopterCallback", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	trace_t tr;
	Vector vecOrigin, vecStart, vecShotDir;
	int iMaterial, iDamageType, iHitbox;
	short nSurfaceProp;
	C_BaseEntity *pEntity = ParseImpactData(data, &vecOrigin, &vecStart, &vecShotDir, nSurfaceProp, iMaterial, iDamageType, iHitbox);

	if (!pEntity)
	{
		// This happens for impacts that occur on an object that's then destroyed.
		// Clear out the fraction so it uses the server's data
		tr.fraction = 1.0;
		PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
		return;
	}

	// If we hit, perform our custom effects and play the sound. Don't create decals
	if (Impact(vecOrigin, vecStart, iMaterial, iDamageType, iHitbox, pEntity, tr, IMPACT_NODECAL | IMPACT_REPORT_RAGDOLL_IMPACTS))
	{
		FX_AirboatGunImpact(vecOrigin, tr.plane.normal, IsXbox() ? 1 : 2);

		// Only do metal + computer custom effects
		if ((iMaterial == CHAR_TEX_METAL) || (iMaterial == CHAR_TEX_COMPUTER))
		{
			PerformCustomEffects(vecOrigin, tr, vecShotDir, iMaterial, 1.0, FLAGS_CUSTIOM_EFFECTS_NOFLECKS);
		}
	}

	PlayImpactSound(pEntity, tr, vecOrigin, nSurfaceProp);
}

DECLARE_CLIENT_EFFECT("HelicopterImpact", ImpactHelicopterCallback);

//-----------------------------------------------------------------------------
// Specialised tracer FX callbacks
//-----------------------------------------------------------------------------
void GenericBulletTracerCallback(const CEffectData &data)
{
	float flVelocity = 4000.0f;
	bool bWhiz = (data.m_fFlags & TRACER_FLAG_WHIZ);
	FX_GenericBulletTracer((Vector&)data.m_vStart, (Vector&)data.m_vOrigin, flVelocity, bWhiz);
}

DECLARE_CLIENT_EFFECT("GenericBulletTracer", GenericBulletTracerCallback);
//-----------------------------------------------------------------------------
void TurretTracerCallback(const CEffectData &data)
{
	float flVelocity = data.m_flScale;
	bool bWhiz = (data.m_fFlags & TRACER_FLAG_WHIZ);
	FX_TurretTracer((Vector&)data.m_vStart, (Vector&)data.m_vOrigin, flVelocity, bWhiz);
}

DECLARE_CLIENT_EFFECT("TurretTracer", TurretTracerCallback);
//-----------------------------------------------------------------------------
void GunshipTracerCallback(const CEffectData &data)
{
	float flVelocity = data.m_flScale;
	bool bWhiz = (data.m_fFlags & TRACER_FLAG_WHIZ);
	FX_GunshipTracer((Vector&)data.m_vStart, (Vector&)data.m_vOrigin, flVelocity, bWhiz);
}

DECLARE_CLIENT_EFFECT("GunshipTracer", GunshipTracerCallback);
//-----------------------------------------------------------------------------
void StriderTracerCallback(const CEffectData &data)
{
	float flVelocity = data.m_flScale;
	bool bWhiz = (data.m_fFlags & TRACER_FLAG_WHIZ);
	FX_StriderTracer((Vector&)data.m_vStart, (Vector&)data.m_vOrigin, flVelocity, bWhiz);
}

DECLARE_CLIENT_EFFECT("StriderTracer", StriderTracerCallback);
//-----------------------------------------------------------------------------
void HunterTracerCallback(const CEffectData &data)
{
	float flVelocity = data.m_flScale;
	bool bWhiz = (data.m_fFlags & TRACER_FLAG_WHIZ);
	FX_HunterTracer((Vector&)data.m_vStart, (Vector&)data.m_vOrigin, flVelocity, bWhiz);
}

DECLARE_CLIENT_EFFECT("HunterTracer", HunterTracerCallback);
//-----------------------------------------------------------------------------
void GaussTracerCallback(const CEffectData &data)
{
	float flVelocity = data.m_flScale;
	bool bWhiz = (data.m_fFlags & TRACER_FLAG_WHIZ);
	FX_GaussTracer((Vector&)data.m_vStart, (Vector&)data.m_vOrigin, flVelocity, bWhiz);
}

DECLARE_CLIENT_EFFECT("GaussTracer", GaussTracerCallback);
//-----------------------------------------------------------------------------
void AirboatGunHeavyTracerCallback(const CEffectData &data)
{
	// Grab the data
	Vector vecStart = GetTracerOrigin(data);
	float flVelocity = data.m_flScale;

	// Use default velocity if none specified
	if (!flVelocity)
	{
		flVelocity = 8000;
	}

	//Get out shot direction and length
	Vector vecShotDir;
	VectorSubtract(data.m_vOrigin, vecStart, vecShotDir);
	float flTotalDist = VectorNormalize(vecShotDir);

	// Don't make small tracers
	if (flTotalDist <= 64)
		return;

	float flLength = random->RandomFloat(300.0f, 400.0f);
	float flLife = (flTotalDist + flLength) / flVelocity;	//NOTENOTE: We want the tail to finish its run as well

	// Add it
	FX_AddDiscreetLine(vecStart, vecShotDir, flVelocity, flLength, flTotalDist, 5.0f, flLife, "effects/gunshiptracer");
}

DECLARE_CLIENT_EFFECT("AirboatGunHeavyTracer", AirboatGunHeavyTracerCallback);
//-----------------------------------------------------------------------------
void AirboatGunTracerCallback(const CEffectData &data)
{
	// Grab the data
	Vector vecStart = GetTracerOrigin(data);
	float flVelocity = data.m_flScale;

	// Use default velocity if none specified
	if (!flVelocity)
	{
		flVelocity = 10000;
	}

	//Get out shot direction and length
	Vector vecShotDir;
	VectorSubtract(data.m_vOrigin, vecStart, vecShotDir);
	float flTotalDist = VectorNormalize(vecShotDir);

	// Don't make small tracers
	if (flTotalDist <= 64)
		return;

	float flLength = random->RandomFloat(256.0f, 384.0f);
	float flLife = (flTotalDist + flLength) / flVelocity;	//NOTENOTE: We want the tail to finish its run as well

	// Add it
	FX_AddDiscreetLine(vecStart, vecShotDir, flVelocity, flLength, flTotalDist, 2.0f, flLife, "effects/gunshiptracer");
}

DECLARE_CLIENT_EFFECT("AirboatGunTracer", AirboatGunTracerCallback);
//-----------------------------------------------------------------------------
void HelicopterTracerCallback(const CEffectData &data)
{
	// Grab the data
	Vector vecStart = GetTracerOrigin(data);
	float flVelocity = data.m_flScale;

	// Use default velocity if none specified
	if (!flVelocity)
	{
		flVelocity = 8000;
	}

	//Get out shot direction and length
	Vector vecShotDir;
	VectorSubtract(data.m_vOrigin, vecStart, vecShotDir);
	float flTotalDist = VectorNormalize(vecShotDir);

	// Don't make small tracers
	if (flTotalDist <= 256)
		return;

	float flLength = random->RandomFloat(256.0f, 384.0f);
	float flLife = (flTotalDist + flLength) / flVelocity;	//NOTENOTE: We want the tail to finish its run as well

	// Add it
	FX_AddDiscreetLine(vecStart, vecShotDir, flVelocity, flLength, flTotalDist, 5.0f, flLife, "effects/gunshiptracer");

	if (data.m_fFlags & TRACER_FLAG_WHIZ)
	{
		FX_TracerSound(vecStart, data.m_vOrigin, TRACER_TYPE_GUNSHIP);
	}
}

DECLARE_CLIENT_EFFECT("HelicopterTracer", HelicopterTracerCallback);
//-----------------------------------------------------------------------------
void FX_PlayerAR2Tracer(const Vector &start, const Vector &end)
{
	VPROF_BUDGET("FX_PlayerAR2Tracer", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	Vector	shotDir, dStart, dEnd;
	float	length;

	//Find the direction of the tracer
	VectorSubtract(end, start, shotDir);
	length = VectorNormalize(shotDir);

	//We don't want to draw them if they're too close to us
	if (length < 128)
		return;

	//Randomly place the tracer along this line, with a random length
	VectorMA(start, random->RandomFloat(0.0f, 8.0f), shotDir, dStart);
	VectorMA(dStart, MIN(length, random->RandomFloat(256.0f, 1024.0f)), shotDir, dEnd);

	//Create the line
	CFXStaticLine *tracerLine = new CFXStaticLine("Tracer", dStart, dEnd,
		random->RandomFloat(6.0f, 12.0f), 0.01f, "effects/gunshiptracer", 0);
	assert(tracerLine);

	//Throw it into the list
	clienteffects->AddEffect(tracerLine);
}
//-----------------------------------------------------------------------------
void FX_AR2Tracer(Vector& start, Vector& end, int velocity, bool makeWhiz)
{
	VPROF_BUDGET("FX_AR2Tracer", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	//Don't make small tracers
	float dist;
	Vector dir;

	VectorSubtract(end, start, dir);
	dist = VectorNormalize(dir);

	// Don't make short tracers.
	if (dist < 128)
		return;

	float length = random->RandomFloat(128.0f, 256.0f);
	float life = (dist + length) / velocity;	//NOTENOTE: We want the tail to finish its run as well

												//Add it
	FX_AddDiscreetLine(start, dir, velocity, length, dist,
		random->RandomFloat(0.5f, 1.5f), life, "effects/gunshiptracer");

	if (makeWhiz)
	{
		FX_TracerSound(start, end, TRACER_TYPE_GUNSHIP);
	}
}
//-----------------------------------------------------------------------------
void AR2TracerCallback(const CEffectData &data)
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();

	if (player == NULL)
		return;

	// Grab the data
	Vector vecStart = GetTracerOrigin(data);
	float flVelocity = data.m_flScale;
	bool bWhiz = (data.m_fFlags & TRACER_FLAG_WHIZ);
	int iEntIndex = data.entindex();

	if (iEntIndex && iEntIndex == player->index)
	{
		Vector	foo = data.m_vStart;
		QAngle	vangles;
		Vector	vforward, vright, vup;

		engine->GetViewAngles(vangles);
		AngleVectors(vangles, &vforward, &vright, &vup);

		VectorMA(data.m_vStart, 4, vright, foo);
		foo[2] -= 0.5f;

		FX_PlayerAR2Tracer(foo, (Vector&)data.m_vOrigin);
		return;
	}

	// Use default velocity if none specified
	if (!flVelocity)
	{
		flVelocity = 8000;
	}

	// Do tracer effect
	FX_AR2Tracer((Vector&)vecStart, (Vector&)data.m_vOrigin, flVelocity, bWhiz);
}

DECLARE_CLIENT_EFFECT("AR2Tracer", AR2TracerCallback);
//-----------------------------------------------------------------------------
void AR2ExplosionCallback(const CEffectData &data)
{
	float lifetime = random->RandomFloat(0.4f, 0.75f);

	// Ground splash
	FX_AddQuad(data.m_vOrigin,
		data.m_vNormal,
		data.m_flRadius,
		data.m_flRadius * 4.0f,
		0.85f,
		1.0f,
		0.0f,
		0.25f,
		random->RandomInt(0, 360),
		random->RandomFloat(-4, 4),
		Vector(1.0f, 1.0f, 1.0f),
		lifetime,
		"effects/combinemuzzle1",
		(FXQUAD_BIAS_SCALE | FXQUAD_BIAS_ALPHA));

	Vector	vRight, vUp;
	VectorVectors(data.m_vNormal, vRight, vUp);

	Vector	start, end;

	float radius = data.m_flRadius * 0.15f;

	// Base vertical shaft
	FXLineData_t lineData;

	start = data.m_vOrigin;
	end = start + (data.m_vNormal * random->RandomFloat(radius*2.0f, radius*4.0f));

	lineData.m_flDieTime = lifetime;

	lineData.m_flStartAlpha = 1.0f;
	lineData.m_flEndAlpha = 0.0f;

	lineData.m_flStartScale = radius * 4;
	lineData.m_flEndScale = radius * 5;

	lineData.m_pMaterial = materials->FindMaterial("effects/ar2ground2", 0, 0);

	lineData.m_vecStart = start;
	lineData.m_vecStartVelocity = vec3_origin;

	lineData.m_vecEnd = end;
	lineData.m_vecEndVelocity = data.m_vNormal * random->RandomFloat(200, 350);

	FX_AddLine(lineData);

	// Inner filler shaft
	start = data.m_vOrigin;
	end = start + (data.m_vNormal * random->RandomFloat(16, radius*0.25f));

	lineData.m_flDieTime = lifetime - 0.1f;

	lineData.m_flStartAlpha = 1.0f;
	lineData.m_flEndAlpha = 0.0f;

	lineData.m_flStartScale = radius * 2;
	lineData.m_flEndScale = radius * 4;

	lineData.m_pMaterial = materials->FindMaterial("effects/ar2ground2", 0, 0);

	lineData.m_vecStart = start;
	lineData.m_vecStartVelocity = vec3_origin;

	lineData.m_vecEnd = end;
	lineData.m_vecEndVelocity = data.m_vNormal * random->RandomFloat(64, 128);

	FX_AddLine(lineData);
}

DECLARE_CLIENT_EFFECT("AR2Explosion", AR2ExplosionCallback);
//-----------------------------------------------------------------------------
void AR2ImpactCallback(const CEffectData &data)
{
	FX_AddQuad(data.m_vOrigin,
		data.m_vNormal,
		random->RandomFloat(24, 32),
		0,
		0.75f,
		1.0f,
		0.0f,
		0.4f,
		random->RandomInt(0, 360),
		0,
		Vector(1.0f, 1.0f, 1.0f),
		0.25f,
		"effects/combinemuzzle2_nocull",
		(FXQUAD_BIAS_SCALE | FXQUAD_BIAS_ALPHA));
}

DECLARE_CLIENT_EFFECT("AR2Impact", AR2ImpactCallback);

//-----------------------------------------------------------------------------
// Non-weapon driven muzzleflashes (i. e. not from player-held or NPC-held)
//-----------------------------------------------------------------------------
void MuzzleFlash_Airboat(ClientEntityHandle_t hEntity, int attachmentIndex)
{
	VPROF_BUDGET("MuzzleFlash_Airboat", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	CSmartPtr<CLocalSpaceEmitter> pSimple = CLocalSpaceEmitter::Create("MuzzleFlash", hEntity, attachmentIndex);

	SimpleParticle *pParticle;
	Vector			forward(1, 0, 0), offset; //NOTENOTE: All coords are in local space

	float flScale = random->RandomFloat(0.75f, 2.5f);

	PMaterialHandle pMuzzle[2];
	pMuzzle[0] = pSimple->GetPMaterial("effects/combinemuzzle1");
	pMuzzle[1] = pSimple->GetPMaterial("effects/combinemuzzle2");

	// Flash
	for (int i = 1; i < 7; i++)
	{
		offset = (forward * (i*6.0f*flScale));

		pParticle = (SimpleParticle *)pSimple->AddParticle(sizeof(SimpleParticle), pMuzzle[random->RandomInt(0, 1)], offset);

		if (pParticle == NULL)
			return;

		pParticle->m_flLifetime = 0.0f;
		pParticle->m_flDieTime = 0.01f;

		pParticle->m_vecVelocity.Init();

		pParticle->m_uchColor[0] = 255;
		pParticle->m_uchColor[1] = 255;
		pParticle->m_uchColor[2] = 255;

		pParticle->m_uchStartAlpha = 255;
		pParticle->m_uchEndAlpha = 128;

		pParticle->m_uchStartSize = ((random->RandomFloat(6.0f, 8.0f) * (9 - (i)) / 7) * flScale);
		pParticle->m_uchEndSize = pParticle->m_uchStartSize;
		pParticle->m_flRoll = random->RandomInt(0, 360);
		pParticle->m_flRollDelta = 0.0f;
	}
/*
	// Tack on the smoke
	pParticle = (SimpleParticle *)pSimple->AddParticle(sizeof(SimpleParticle), pSimple->GetPMaterial("sprites/ar2_muzzle1"), vec3_origin);

	if (pParticle == NULL)
		return;

	pParticle->m_flLifetime = 0.0f;
	pParticle->m_flDieTime = 0.05f;

	pParticle->m_vecVelocity.Init();

	pParticle->m_uchColor[0] = 255;
	pParticle->m_uchColor[1] = 255;
	pParticle->m_uchColor[2] = 255;

	pParticle->m_uchStartAlpha = 255;
	pParticle->m_uchEndAlpha = 128;

	pParticle->m_uchStartSize = random->RandomFloat(16.0f, 24.0f);
	pParticle->m_uchEndSize = pParticle->m_uchStartSize;

	float spokePos = random->RandomInt(0, 5);

	pParticle->m_flRoll = (360.0 / 6.0f)*spokePos;
	pParticle->m_flRollDelta = 0.0f;

	// Grab the origin out of the transform for the attachment
//	if (muzzleflash_light.GetInt())
	{
		// If the client hasn't seen this entity yet, bail.
		matrix3x4_t	matAttachment;
		if (FX_GetAttachmentTransform(hEntity, attachmentIndex, matAttachment))
		{
			Vector		origin;
			MatrixGetColumn(matAttachment, 3, &origin);
			CreateMuzzleflashELight(origin, 5, 64, 128, hEntity);
		}
	}
*/
}
//-----------------------------------------------------------------------------
void AirboatMuzzleFlashCallback(const CEffectData &data)
{
	MuzzleFlash_Airboat(data.m_hEntity, data.m_nAttachmentIndex);
}

DECLARE_CLIENT_EFFECT("AirboatMuzzleFlash", AirboatMuzzleFlashCallback);
//-----------------------------------------------------------------------------
void MuzzleFlash_Chopper(ClientEntityHandle_t hEntity, int attachmentIndex)
{
	VPROF_BUDGET("MuzzleFlash_Chopper", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	matrix3x4_t	matAttachment;
	// If the client hasn't seen this entity yet, bail.
	if (!FX_GetAttachmentTransform(hEntity, attachmentIndex, matAttachment))
		return;

	CSmartPtr<CLocalSpaceEmitter> pSimple = CLocalSpaceEmitter::Create("MuzzleFlash", hEntity, attachmentIndex);

	SimpleParticle *pParticle;
	Vector			forward(1, 0, 0), offset; //NOTENOTE: All coords are in local space

	float flScale = random->RandomFloat(2.5f, 4.5f);

	// Flash
	for (int i = 1; i < 7; i++)
	{
		offset = (forward * (i*2.0f*flScale));

		pParticle = (SimpleParticle *)pSimple->AddParticle(sizeof(SimpleParticle), pSimple->GetPMaterial(VarArgs("effects/combinemuzzle%d", random->RandomInt(1, 2))), offset);

		if (pParticle == NULL)
			return;

		pParticle->m_flLifetime = 0.0f;
		pParticle->m_flDieTime = random->RandomFloat(0.05f, 0.1f);

		pParticle->m_vecVelocity.Init();

		pParticle->m_uchColor[0] = 255;
		pParticle->m_uchColor[1] = 255;
		pParticle->m_uchColor[2] = 255;

		pParticle->m_uchStartAlpha = 255;
		pParticle->m_uchEndAlpha = 128;

		pParticle->m_uchStartSize = ((random->RandomFloat(6.0f, 8.0f) * (10 - (i)) / 7) * flScale);
		pParticle->m_uchEndSize = pParticle->m_uchStartSize;
		pParticle->m_flRoll = random->RandomInt(0, 360);
		pParticle->m_flRollDelta = 0.0f;
	}

	// Grab the origin out of the transform for the attachment
	Vector		origin;
	MatrixGetColumn(matAttachment, 3, &origin);
	CreateMuzzleflashELight(origin, 6, 128, 256, hEntity);
}
//-----------------------------------------------------------------------------
void ChopperMuzzleFlashCallback(const CEffectData &data)
{
	MuzzleFlash_Chopper(data.m_hEntity, data.m_nAttachmentIndex);
}

DECLARE_CLIENT_EFFECT("ChopperMuzzleFlash", ChopperMuzzleFlashCallback);
//-----------------------------------------------------------------------------
void MuzzleFlash_Gunship(ClientEntityHandle_t hEntity, int attachmentIndex)
{
	VPROF_BUDGET("MuzzleFlash_Gunship", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	// If the client hasn't seen this entity yet, bail.
	matrix3x4_t	matAttachment;
	if (!FX_GetAttachmentTransform(hEntity, attachmentIndex, matAttachment))
		return;

	CSmartPtr<CLocalSpaceEmitter> pSimple = CLocalSpaceEmitter::Create("MuzzleFlash", hEntity, attachmentIndex);

	SimpleParticle *pParticle;
	Vector			forward(1, 0, 0), offset; //NOTENOTE: All coords are in local space

	float flScale = random->RandomFloat(2.5f, 4.5f);

	// Flash
	offset = (forward * (2.0f*flScale));

	pParticle = (SimpleParticle *)pSimple->AddParticle(sizeof(SimpleParticle), pSimple->GetPMaterial("effects/blueblackflash"), offset);
	if (pParticle == NULL)
		return;

	pParticle->m_flLifetime = 0.0f;
	pParticle->m_flDieTime = random->RandomFloat(0.05f, 0.1f);

	pParticle->m_vecVelocity.Init();

	pParticle->m_uchColor[0] = 255;
	pParticle->m_uchColor[1] = 255;
	pParticle->m_uchColor[2] = 255;

	pParticle->m_uchStartAlpha = 128;
	pParticle->m_uchEndAlpha = 32;

	pParticle->m_uchStartSize = ((random->RandomFloat(6.0f, 8.0f) * 10.0 / 7.0) * flScale);
	pParticle->m_uchEndSize = pParticle->m_uchStartSize;
	pParticle->m_flRoll = random->RandomInt(0, 360);
	pParticle->m_flRollDelta = 0.0f;

	// Grab the origin out of the transform for the attachment
	Vector		origin;
	MatrixGetColumn(matAttachment, 3, &origin);
	CreateMuzzleflashELight(origin, 6, 256, 512, hEntity);
}
//-----------------------------------------------------------------------------
void GunshipMuzzleFlashCallback(const CEffectData &data)
{
	MuzzleFlash_Gunship(data.m_hEntity, data.m_nAttachmentIndex);
}

DECLARE_CLIENT_EFFECT("GunshipMuzzleFlash", GunshipMuzzleFlashCallback);
//-----------------------------------------------------------------------------
void MuzzleFlash_Hunter(ClientEntityHandle_t hEntity, int attachmentIndex)
{
	VPROF_BUDGET("MuzzleFlash_Hunter", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	// If the client hasn't seen this entity yet, bail.
	matrix3x4_t	matAttachment;
	if (!FX_GetAttachmentTransform(hEntity, attachmentIndex, matAttachment))
		return;

	// Grab the origin out of the transform for the attachment
	Vector		origin;
	MatrixGetColumn(matAttachment, 3, &origin);

	dlight_t *el = effects->CL_AllocElight(LIGHT_INDEX_MUZZLEFLASH);
	el->origin = origin;// + Vector( 12.0f, 0, 0 );

	el->color.r = 50;
	el->color.g = 222;
	el->color.b = 213;
	el->color.exponent = 5;

	el->radius = random->RandomInt(120, 200);
	el->decay = el->radius / 0.05f;
	el->die = CURTIME + 0.05f;
}
//-----------------------------------------------------------------------------
void HunterMuzzleFlashCallback(const CEffectData &data)
{
	MuzzleFlash_Hunter(data.m_hEntity, data.m_nAttachmentIndex);
}

DECLARE_CLIENT_EFFECT("HunterMuzzleFlash", HunterMuzzleFlashCallback);
//-----------------------------------------------------------------------------
void FX_PlayerBulletTracer(const Vector &start, const Vector &end)
{
	VPROF_BUDGET("FX_PlayerBulletTracer", VPROF_BUDGETGROUP_PARTICLE_RENDERING);
	Vector	shotDir, dStart, dEnd;
	float	length;

	//Find the direction of the tracer
	VectorSubtract(end, start, shotDir);
	length = VectorNormalize(shotDir);

	//We don't want to draw them if they're too close to us
	if (length < 128)
		return;

	//Randomly place the tracer along this line, with a random length
	VectorMA(start, random->RandomFloat(0.0f, 8.0f), shotDir, dStart);
	VectorMA(dStart, MIN(length, random->RandomFloat(256.0f, 1024.0f)), shotDir, dEnd);

	FX_AddDiscreetLine(start, shotDir, 8000, length, 128,
	random->RandomFloat(1.5f, 2.0f), 0.1f, "effects/spark");

	//Create the line
	CFXStaticLine *tracerLine = new CFXStaticLine("Tracer", dStart, dEnd,
		random->RandomFloat(6.0f, 12.0f), 0.01f, "effects/spark", 0);
	assert(tracerLine);

	//Throw it into the list
	clienteffects->AddEffect(tracerLine);
}

void FX_BulletTracer(Vector& start, Vector& end, int velocity, bool makeWhiz)
{
	VPROF_BUDGET("FX_BulletTracer", VPROF_BUDGETGROUP_PARTICLE_RENDERING);

	//Don't make small tracers
	float dist;
	Vector dir;

	VectorSubtract(end, start, dir);
	dist = VectorNormalize(dir);

	// Don't make short tracers.
	if (dist < 128)
	return;

	float length = 128.0f;
	float life = (dist + length) / velocity;	//NOTENOTE: We want the tail to finish its run as well

	//Add it
	FX_AddDiscreetLine(start, dir, velocity, length, dist,
	random->RandomFloat(1.5f, 2.0f), life, "effects/spark");

	if (makeWhiz)
	{
	FX_TracerSound(start, end, TRACER_TYPE_DEFAULT);
	}
}
//-----------------------------------------------------------------------------
void BulletTracerCallback(const CEffectData &data)
{
	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();

	if (player == NULL)
		return;

	// Grab the data
	Vector vecStart = GetTracerOrigin(data);
	float flVelocity = data.m_flScale;
	bool bWhiz = (data.m_fFlags & TRACER_FLAG_WHIZ);
	int iEntIndex = data.entindex();

	if (iEntIndex && iEntIndex == player->index)
	{
		Vector	foo = data.m_vStart;
		QAngle	vangles;
		Vector	vforward, vright, vup;

		engine->GetViewAngles(vangles);
		AngleVectors(vangles, &vforward, &vright, &vup);

		VectorMA(data.m_vStart, 4, vright, foo);
		foo[2] -= 0.5f;

		FX_PlayerBulletTracer(foo, (Vector&)data.m_vOrigin);
		return;
	}

	// Use default velocity if none specified
	if (!flVelocity)
	{
		flVelocity = 8000;
	}

	// Do tracer effect
	FX_BulletTracer((Vector&)vecStart, (Vector&)data.m_vOrigin, flVelocity, bWhiz);
}

DECLARE_CLIENT_EFFECT("BulletTracer", BulletTracerCallback);
//-----------------------------------------------------------------------------
// Specialised gibbing FX, base & child version (antlion gibs)
//-----------------------------------------------------------------------------

ConVar g_antlion_maxgibs("g_antlion_maxgibs", "16", FCVAR_ARCHIVE);
ConVar g_antlion_gibdecals("g_antlion_gibdecals", "1", FCVAR_ARCHIVE);

class CAntlionGibManager : public CAutoGameSystemPerFrame
{
public:
	CAntlionGibManager(char const *name) : CAutoGameSystemPerFrame(name)
	{
	}

	// Methods of IGameSystem
	virtual void Update(float frametime);
	virtual void LevelInitPreEntity(void);

	void	AddGib(C_BaseEntity *pEntity);
	void	RemoveGib(C_BaseEntity *pEntity);

private:
	typedef CHandle<C_BaseEntity> CGibHandle;
	CUtlLinkedList< CGibHandle > m_LRU;

};

void CAntlionGibManager::LevelInitPreEntity(void)
{
	m_LRU.Purge();
}

CAntlionGibManager s_AntlionGibManager("CAntlionGibManager");

void CAntlionGibManager::AddGib(C_BaseEntity *pEntity)
{
	m_LRU.AddToTail(pEntity);
}

void CAntlionGibManager::RemoveGib(C_BaseEntity *pEntity)
{
	m_LRU.FindAndRemove(pEntity);
}

//-----------------------------------------------------------------------------
// Methods of IGameSystem
//-----------------------------------------------------------------------------
void CAntlionGibManager::Update(float frametime)
{
	if (m_LRU.Count() < g_antlion_maxgibs.GetInt())
		return;

	int i = 0;
	i = m_LRU.Head();

	if (m_LRU[i].Get())
	{
		m_LRU[i].Get()->SetNextClientThink(CURTIME);
	}

	m_LRU.Remove(i);
}

class C_Gib : public C_BaseAnimating
{
	typedef C_BaseAnimating BaseClass;
public:

	~C_Gib(void){ VPhysicsDestroyObject(); }

	static C_Gib *CreateClientsideGib(const char *pszModelName, 
		Vector vecOrigin, Vector vecForceDir, 
		AngularImpulse vecAngularImp, 
		float flLifetime = 4.0f) 
	{
		C_Gib *pGib = new C_Gib;

		if (pGib == NULL)
			return NULL;

		if (pGib->InitializeGib(pszModelName, vecOrigin, vecForceDir, vecAngularImp, flLifetime) == false)
			return NULL;

		return pGib;
	}

	bool	InitializeGib(const char *pszModelName, 
		Vector vecOrigin, 
		Vector vecForceDir, 
		AngularImpulse vecAngularImp, 
		float flLifetime = 4.0f)
	{
		if (InitializeAsClientEntity(pszModelName, RENDER_GROUP_OPAQUE_ENTITY) == false)
		{
			Release();
			return false;
		}

		SetAbsOrigin(vecOrigin);
		SetCollisionGroup(COLLISION_GROUP_DEBRIS);

		solid_t tmpSolid;
		PhysModelParseSolid(tmpSolid, this, GetModelIndex());

		m_pPhysicsObject = VPhysicsInitNormal(SOLID_VPHYSICS, 0, false, &tmpSolid);

		if (m_pPhysicsObject)
		{
			float flForce = m_pPhysicsObject->GetMass();
			vecForceDir *= flForce;

			m_pPhysicsObject->ApplyForceOffset(vecForceDir, GetAbsOrigin());
			m_pPhysicsObject->SetCallbackFlags(m_pPhysicsObject->GetCallbackFlags() | CALLBACK_GLOBAL_TOUCH | CALLBACK_GLOBAL_TOUCH_STATIC);
		}
		else
		{
			// failed to create a physics object
			Release();
			return false;
		}

		SetNextClientThink(CURTIME + flLifetime);

		return true;
	}

	void	ClientThink(void)
	{
		SetRenderMode(kRenderTransAlpha);
		m_nRenderFX = kRenderFxFadeFast;

		if (m_clrRender->a == 0)
		{
			s_AntlionGibManager.RemoveGib(this);
			Release();
			return;
		}

		SetNextClientThink(CURTIME + 1.0f);
	}

	void	StartTouch(C_BaseEntity *pOther)
	{
		// Limit the amount of times we can bounce
		if (m_flTouchDelta < CURTIME)
		{
			HitSurface(pOther);
			m_flTouchDelta = CURTIME + 0.1f;
		}

		BaseClass::StartTouch(pOther);
	}

	virtual	void HitSurface(C_BaseEntity *pOther)
	{
		//TODO: Implement splatter or effects in child versions
	}

protected:
	float	m_flTouchDelta;		// Amount of time that must pass before another touch function can be called
};

PMaterialHandle	g_Material_Blood[2] = { NULL, NULL };

// Use all the gibs
#define	NUM_ANTLION_GIBS_UNIQUE	3
const char *pszAntlionGibs_Unique[NUM_ANTLION_GIBS_UNIQUE] = {
	"models/gibs/antlion_gib_large_1.mdl",
	"models/gibs/antlion_gib_large_2.mdl",
	"models/gibs/antlion_gib_large_3.mdl"
};

#define	NUM_ANTLION_GIBS_MEDIUM	3
const char *pszAntlionGibs_Medium[NUM_ANTLION_GIBS_MEDIUM] = {
	"models/gibs/antlion_gib_medium_1.mdl",
	"models/gibs/antlion_gib_medium_2.mdl",
	"models/gibs/antlion_gib_medium_3.mdl"
};

// XBox doesn't use the smaller gibs, so don't cache them
#define	NUM_ANTLION_GIBS_SMALL	3
const char *pszAntlionGibs_Small[NUM_ANTLION_GIBS_SMALL] = {
	"models/gibs/antlion_gib_small_1.mdl",
	"models/gibs/antlion_gib_small_2.mdl",
	"models/gibs/antlion_gib_small_3.mdl"
};

// Antlion gib - marks surfaces when it bounces

class C_AntlionGib : public C_Gib
{
	typedef C_Gib BaseClass;
public:

	static C_AntlionGib *CreateClientsideGib(const char *pszModelName, 
		Vector vecOrigin, Vector vecForceDir, 
		AngularImpulse vecAngularImp, 
		float m_flLifetime = 4.0f)
	{
		C_AntlionGib *pGib = new C_AntlionGib;

		if (pGib == NULL)
			return NULL;

		if (pGib->InitializeGib(pszModelName, vecOrigin, vecForceDir, vecAngularImp, m_flLifetime) == false)
			return NULL;

		s_AntlionGibManager.AddGib(pGib);

		return pGib;
	}

	// Decal the surface
	virtual	void HitSurface(C_BaseEntity *pOther)
	{
		if (g_antlion_gibdecals.GetBool())
		{
			//JDW: Removed for the time being
			int index = decalsystem->GetDecalIndexForName("YellowBlood");

			if (index >= 0)
			{
				effects->DecalShoot(index, pOther->entindex(), pOther->GetModel(), pOther->GetAbsOrigin(), pOther->GetAbsAngles(), GetAbsOrigin(), 0, 0);
			}
		}
	}
};
//-----------------------------------------------------------------------------
// Specialised effect events, usually called by NPCs.
//-----------------------------------------------------------------------------
void FX_AntlionGib(const Vector &origin, const Vector &direction, float scale)
{
	Vector	offset;
	int numGibs = random->RandomInt(1, NUM_ANTLION_GIBS_UNIQUE);

	// Spawn all the unique gibs
	for (int i = 0; i < numGibs; i++)
	{
		offset = RandomVector(-16, 16) + origin;

		C_AntlionGib::CreateClientsideGib(pszAntlionGibs_Unique[i], offset, (direction + RandomVector(-0.8f, 0.8f)) * (150 * scale), RandomAngularImpulse(-32, 32), 2.0f);
	}

	numGibs = random->RandomInt(1, NUM_ANTLION_GIBS_MEDIUM);

	// Spawn all the medium gibs
	for (int i = 0; i < numGibs; i++)
	{
		offset = RandomVector(-16, 16) + origin;

		C_AntlionGib::CreateClientsideGib(pszAntlionGibs_Medium[i], offset, (direction + RandomVector(-0.8f, 0.8f)) * (250 * scale), RandomAngularImpulse(-200, 200), 1.0f);
	}

	numGibs = random->RandomInt(1, NUM_ANTLION_GIBS_SMALL);

	// Spawn all the small gibs
	for (int i = 0; i < NUM_ANTLION_GIBS_SMALL; i++)
	{
		offset = RandomVector(-16, 16) + origin;

		C_AntlionGib::CreateClientsideGib(pszAntlionGibs_Small[i], offset, (direction + RandomVector(-0.8f, 0.8f)) * (400 * scale), RandomAngularImpulse(-300, 300), 0.5f);
	}
	
	// Spawn blood
	CSmartPtr<CSimpleEmitter> pSimple = CSimpleEmitter::Create("FX_AntlionGib");
	pSimple->SetSortOrigin(origin);

	Vector	vDir;

	vDir.Random(-1.0f, 1.0f);

	for (int i = 0; i < 4; i++)
	{
		SimpleParticle *sParticle = (SimpleParticle *)pSimple->AddParticle(sizeof(SimpleParticle), g_Mat_BloodPuff[0], origin);

		if (sParticle == NULL)
			return;

		sParticle->m_flLifetime = 0.0f;
		sParticle->m_flDieTime = random->RandomFloat(0.5f, 0.75f);

		float	speed = random->RandomFloat(16.0f, 64.0f);

		sParticle->m_vecVelocity = vDir * -speed;
		sParticle->m_vecVelocity[2] += 16.0f;

		sParticle->m_uchColor[0] = 185;
		sParticle->m_uchColor[1] = 147;
		sParticle->m_uchColor[2] = 142;
		sParticle->m_uchStartAlpha = 255;
		sParticle->m_uchEndAlpha = 0;
		sParticle->m_uchStartSize = /*scale **/ random->RandomInt(16, 32);
		sParticle->m_uchEndSize = /*scale **/ sParticle->m_uchStartSize * 2;
		sParticle->m_flRoll = random->RandomInt(0, 360);
		sParticle->m_flRollDelta = random->RandomFloat(-1.0f, 1.0f);
	}

	for (int i = 0; i < 4; i++)
	{
		SimpleParticle *sParticle = (SimpleParticle *)pSimple->AddParticle(sizeof(SimpleParticle), g_Mat_BloodPuff[1], origin);

		if (sParticle == NULL)
		{
			return;
		}

		sParticle->m_flLifetime = 0.0f;
		sParticle->m_flDieTime = random->RandomFloat(0.5f, 0.75f);

		float	speed = random->RandomFloat(16.0f, 64.0f);

		sParticle->m_vecVelocity = vDir * -speed;
		sParticle->m_vecVelocity[2] += 16.0f;

		sParticle->m_uchColor[0] = 185;
		sParticle->m_uchColor[1] = 147;
		sParticle->m_uchColor[2] = 142;
		sParticle->m_uchStartAlpha = random->RandomInt(64, 128);
		sParticle->m_uchEndAlpha = 0;
		sParticle->m_uchStartSize = random->RandomInt(16, 32);
		sParticle->m_uchEndSize = /*scale **/ sParticle->m_uchStartSize * 2;
		sParticle->m_flRoll = random->RandomInt(0, 360);
		sParticle->m_flRollDelta = random->RandomFloat(-1.0f, 1.0f);
	}
}
//-----------------------------------------------------------------------------
void AntlionGibCallback(const CEffectData &data)
{
	FX_AntlionGib(data.m_vOrigin, data.m_vNormal, data.m_flScale);
}

DECLARE_CLIENT_EFFECT("AntlionGib", AntlionGibCallback);
//-----------------------------------------------------------------------------

void FX_HeadcrabDust(const Vector &origin, const Vector &direction, float scale)
{
	// Spawn particles
	CSmartPtr<CSimpleEmitter> pSimple = CSimpleEmitter::Create("FX_HeadcrabDust");
	pSimple->SetSortOrigin(origin);

	Vector	vDir;

	vDir.Random(-1.0f, 1.0f);

	for (int i = 0; i < 4; i++)
	{
		SimpleParticle *sParticle = (SimpleParticle *)pSimple->AddParticle(sizeof(SimpleParticle), g_Mat_DustPuff[0], origin);

		if (sParticle == NULL)
			return;

		sParticle->m_flLifetime = 0.0f;
		sParticle->m_flDieTime = random->RandomFloat(0.5f, 0.75f);

		float	speed = random->RandomFloat(16.0f, 64.0f);

		sParticle->m_vecVelocity = vDir * -speed;
		sParticle->m_vecVelocity[2] += 16.0f;

		sParticle->m_uchColor[0] = 55;
		sParticle->m_uchColor[1] = 62;
		sParticle->m_uchColor[2] = 65;
		sParticle->m_uchStartAlpha = 255;
		sParticle->m_uchEndAlpha = 0;
		sParticle->m_uchStartSize = scale;
		sParticle->m_uchEndSize = scale;
		sParticle->m_flRoll = random->RandomInt(0, 360);
		sParticle->m_flRollDelta = random->RandomFloat(-1.0f, 1.0f);
	}

	for (int i = 0; i < 4; i++)
	{
		SimpleParticle *sParticle = (SimpleParticle *)pSimple->AddParticle(sizeof(SimpleParticle), g_Mat_DustPuff[random->RandomInt(0,1)], origin);

		if (sParticle == NULL)
		{
			return;
		}

		sParticle->m_flLifetime = 0.0f;
		sParticle->m_flDieTime = random->RandomFloat(0.5f, 0.75f);

		float	speed = random->RandomFloat(16.0f, 64.0f);

		sParticle->m_vecVelocity = vDir * -speed;
		sParticle->m_vecVelocity[2] += 16.0f;

		sParticle->m_uchColor[0] = 185;
		sParticle->m_uchColor[1] = 147;
		sParticle->m_uchColor[2] = 142;
		sParticle->m_uchStartAlpha = random->RandomInt(64, 128);
		sParticle->m_uchEndAlpha = 0;
		sParticle->m_uchStartSize = random->RandomInt(16, 32);
		sParticle->m_uchEndSize = scale;
		sParticle->m_flRoll = random->RandomInt(0, 360);
		sParticle->m_flRollDelta = random->RandomFloat(-1.0f, 1.0f);
	}
}
//-----------------------------------------------------------------------------
void HeadcrabDustCallback(const CEffectData &data)
{
	FX_HeadcrabDust(data.m_vOrigin, data.m_vNormal, data.m_flScale);
}

DECLARE_CLIENT_EFFECT("HeadcrabDust", HeadcrabDustCallback);

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void StalkerDecalCallback(const CEffectData &data)
{
	/*
	// Quick flash
	FX_AddQuad(data.m_vOrigin,
		data.m_vNormal,
		data.m_flRadius * 10.0f,
		0,
		0.75f,
		1.0f,
		0.0f,
		0.4f,
		random->RandomInt(0, 360),
		0,
		Vector(1.0f, 1.0f, 1.0f),
		0.25f,
		"decals/redglowfade",
		(FXQUAD_BIAS_SCALE | FXQUAD_BIAS_ALPHA));
	*/
	// Lingering burn
	FX_AddQuad(data.m_vOrigin,
		data.m_vNormal,
		data.m_flRadius/* * 2.0f*/,
		data.m_flRadius/* * 4.0f*/,
		1.00f,
		1.0f,
		0.0f,
		0.4f,
		random->RandomInt(0, 360),
		0,
		Vector(1.0f, 1.0f, 1.0f),
		4.0f,
		"decals/redglowfade",
		(FXQUAD_BIAS_SCALE | FXQUAD_BIAS_ALPHA));

	// Throw sparks
//	FX_ElectricSpark(data.m_vOrigin, 2, 1, &data.m_vNormal);
}

DECLARE_CLIENT_EFFECT("redglowfade", StalkerDecalCallback);

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
ConVar di_footsteps_lifetime("di_footsteps_lifetime", "10");
void FootprintRightCallback(const CEffectData &data)
{
	FX_AddQuad(data.m_vOrigin,
		data.m_vNormal,
		data.m_flRadius/* * 2.0f*/,
		data.m_flRadius/* * 4.0f*/,
		1.00f,
		1.0f,
		0.0f,
		0.4f,
		data.m_vAngles[1],
		0,
		Vector(1.0f, 1.0f, 1.0f),
		di_footsteps_lifetime.GetFloat(),
		"decals/footprint_wet",
		(FXQUAD_BIAS_SCALE | FXQUAD_BIAS_ALPHA));
}

DECLARE_CLIENT_EFFECT("footprint_right", FootprintRightCallback);

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void FootprintLeftCallback(const CEffectData &data)
{
	FX_AddQuad(data.m_vOrigin,
		data.m_vNormal,
		data.m_flRadius/* * 2.0f*/,
		data.m_flRadius/* * 4.0f*/,
		1.00f,
		1.0f,
		0.0f,
		0.4f,
		data.m_vAngles[1],
		0,
		Vector(1.0f, 1.0f, 1.0f),
		di_footsteps_lifetime.GetFloat(),
		"decals/footprint_wet_left",
		(FXQUAD_BIAS_SCALE | FXQUAD_BIAS_ALPHA));
}

DECLARE_CLIENT_EFFECT("footprint_left", FootprintLeftCallback);