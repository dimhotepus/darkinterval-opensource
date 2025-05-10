//========================================================================//
//
// Purpose:		Crowbar - an old favorite
//
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#ifdef DARKINTERVAL
#include "basehlcombatweapon.h"
#include "hl2_player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "soundent.h"
#include "animation.h"
#include "ai_condition.h"
#include "weapon_crowbar.h"
#include "ndebugoverlay.h"
#include "te_effect_dispatch.h"
#include "rumble_shared.h"
#include "gamestats.h"
#include "npcevent.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar    sk_plr_dmg_crowbar("sk_plr_dmg_crowbar", "0");
ConVar    sk_npc_dmg_crowbar("sk_npc_dmg_crowbar", "0");
ConVar	  sk_crowbar_lead_time("sk_crowbar_lead_time", "0.9");

IMPLEMENT_SERVERCLASS_ST(CPlayerCrowbar, DT_PlayerCrowbar)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_crowbar, CPlayerCrowbar);
PRECACHE_WEAPON_REGISTER(weapon_crowbar);

acttable_t CPlayerCrowbar::m_acttable[] =
{
	{ ACT_MELEE_ATTACK1,	ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },
};

IMPLEMENT_ACTTABLE(CPlayerCrowbar);

#define BLUDGEON_HULL_DIM		8

static const Vector g_bludgeonMins(-BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM, -BLUDGEON_HULL_DIM);
static const Vector g_bludgeonMaxs(BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM, BLUDGEON_HULL_DIM);

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CPlayerCrowbar::CPlayerCrowbar()
{
	m_can_fire_underwater_boolean = true;
}

//-----------------------------------------------------------------------------
// Purpose: Spawn the weapon
//-----------------------------------------------------------------------------
void CPlayerCrowbar::Spawn(void)
{
	m_fMinRange1 = 0;
	m_fMinRange2 = 0;
	m_fMaxRange1 = 64;
	m_fMaxRange2 = 64;
	//Call base class first
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: Precache the weapon
//-----------------------------------------------------------------------------
void CPlayerCrowbar::Precache(void)
{
	//Call base class first
	BaseClass::Precache();
	PrecacheScriptSound("DI_Crowbar.Kill");
}

int CPlayerCrowbar::CapabilitiesGet()
{
	return bits_CAP_WEAPON_MELEE_ATTACK1;
}

void CPlayerCrowbar::UpdateViewModel(void)
{
	if (GetOwner() && GetOwner()->IsPlayer())
	{
		CBasePlayer *owner = ToBasePlayer(GetOwner());
		if (GetOwner() != NULL && GetOwner()->IsPlayer())
		{
			CBaseViewModel *viewmodel = owner->GetViewModel(0);
			if (viewmodel != NULL)
			{
				if (!owner->IsSuitEquipped())
				{
					viewmodel->SetBodygroup(1, 1);
					viewmodel->m_nSkin = 1;
				}
				else
				{
					viewmodel->SetBodygroup(1, 0);
					viewmodel->m_nSkin = 0;
				}
			}
		}
	}
}

int CPlayerCrowbar::WeaponMeleeAttack1Condition(float flDot, float flDist)
{
	if (flDist > 64)
	{
		return COND_TOO_FAR_TO_ATTACK;
	}
	else if (flDot < 0.7)
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}

//------------------------------------------------------------------------------
// Purpose : Update weapon
//------------------------------------------------------------------------------
void CPlayerCrowbar::ItemPostFrame(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner == NULL)
		return;

	UpdateViewModel();

	if ((pOwner->m_nButtons & IN_ATTACK) && (m_flNextPrimaryAttack <= CURTIME))
	{
		PrimaryAttack();
	}
	else if ((pOwner->m_nButtons & IN_ATTACK2) && (m_flNextSecondaryAttack <= CURTIME))
	{
		SecondaryAttack();
	}
	else
	{
		WeaponIdle();
		return;
	}
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CPlayerCrowbar::PrimaryAttack()
{
	CBasePlayer *player = ToBasePlayer(GetOwner());
	if (player && !player->IsSuitEquipped()) Swing(true); // without HEV, behave like in HL2
	else
		Swing(false); // otherwise, have powerful primary and regular secondary
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CPlayerCrowbar::SecondaryAttack()
{
	CBasePlayer *player = ToBasePlayer(GetOwner());
	if (player && !player->IsSuitEquipped()) return; // without HEV, behave like in HL2
	Swing(true);
}


//------------------------------------------------------------------------------
// Purpose: Implement impact function
//------------------------------------------------------------------------------
void CPlayerCrowbar::Hit(trace_t &traceHit, Activity nHitActivity, bool bIsSecondary)
{	
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	//Do view kick
	AddViewKick();

	//Make sound for the AI
	CSoundEnt::InsertSound(SOUND_BULLET_IMPACT, traceHit.endpos, 400, 0.2f, pPlayer);

	// This isn't great, but it's something for when the crowbar hits.
	pPlayer->RumbleEffect(RUMBLE_AR2, 0, RUMBLE_FLAG_RESTART);

	trace_t impactHit;

	UTIL_TraceLine(traceHit.startpos, traceHit.startpos + GetOwner()->EyeDirection3D() * GetRange(), MASK_SHOT_HULL, GetOwner(), COLLISION_GROUP_NONE, &impactHit);

	if (impactHit.fraction != 1.0f)
	{
		CBaseEntity	*pHitEntity = impactHit.m_pEnt;

		//Apply damage to a hit target
		if (pHitEntity != NULL)
		{
			Vector hitDirection;
			pPlayer->EyeVectors(&hitDirection, NULL, NULL);
			VectorNormalize(hitDirection);

			CTakeDamageInfo info(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_CLUB);

			if (pPlayer && pHitEntity->IsNPC())
			{
				// If bonking an NPC, adjust damage.
				info.AdjustPlayerDamageInflictedForSkillLevel();

				// play meaty impact sound
				EmitSound("DI_Crowbar.Kill");
			}

			CalculateMeleeDamageForce(&info, hitDirection, impactHit.endpos);

			pHitEntity->DispatchTraceAttack(info, hitDirection, &impactHit);

			ApplyMultiDamage();
			
			// Now hit all triggers along the ray that... 
			TraceAttackToTriggers(info, impactHit.startpos, impactHit.endpos, hitDirection);

			if (ToBaseCombatCharacter(pHitEntity))
			{
				gamestats->Event_WeaponHit(pPlayer, !bIsSecondary, GetClassname(), info);
			}
		}
	}

	// Apply an impact effect
	ImpactEffect(traceHit);
}

Activity CPlayerCrowbar::ChooseIntersectionPointAndActivity(trace_t &hitTrace, const Vector &mins, const Vector &maxs, CBasePlayer *pOwner)
{
	int			i, j, k;
	float		distance;
	const float	*minmaxs[2] = { mins.Base(), maxs.Base() };
	trace_t		tmpTrace;
	Vector		vecHullEnd = hitTrace.endpos;
	Vector		vecEnd;

	distance = 1e6f;
	Vector vecSrc = hitTrace.startpos;

	vecHullEnd = vecSrc + ((vecHullEnd - vecSrc) * 2);
	UTIL_TraceLine(vecSrc, vecHullEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace);
	if (tmpTrace.fraction == 1.0)
	{
		for (i = 0; i < 2; i++)
		{
			for (j = 0; j < 2; j++)
			{
				for (k = 0; k < 2; k++)
				{
					vecEnd.x = vecHullEnd.x + minmaxs[i][0];
					vecEnd.y = vecHullEnd.y + minmaxs[j][1];
					vecEnd.z = vecHullEnd.z + minmaxs[k][2];

					UTIL_TraceLine(vecSrc, vecEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &tmpTrace);
					if (tmpTrace.fraction < 1.0)
					{
						float thisDistance = (tmpTrace.endpos - vecSrc).Length();
						if (thisDistance < distance)
						{
							hitTrace = tmpTrace;
							distance = thisDistance;
						}
					}
				}
			}
		}
	}
	else
	{
		hitTrace = tmpTrace;
	}
	
	return ACT_VM_HITCENTER2;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &traceHit - 
//-----------------------------------------------------------------------------
bool CPlayerCrowbar::ImpactWater(const Vector &start, const Vector &end)
{
	//FIXME: This doesn't handle the case of trying to splash while being underwater, but that's not going to look good
	//		 right now anyway...

	// We must start outside the water
	if (UTIL_PointContents(start) & (CONTENTS_WATER | CONTENTS_SLIME))
		return false;

	// We must end inside of water
	if (!(UTIL_PointContents(end) & (CONTENTS_WATER | CONTENTS_SLIME)))
		return false;

	trace_t	waterTrace;

	UTIL_TraceLine(start, end, (CONTENTS_WATER | CONTENTS_SLIME), GetOwner(), COLLISION_GROUP_NONE, &waterTrace);

	if (waterTrace.fraction < 1.0f)
	{
		CEffectData	data;

		data.m_fFlags = 0;
		data.m_vOrigin = waterTrace.endpos;
		data.m_vNormal = waterTrace.plane.normal;
		data.m_flScale = 8.0f;

		// See if we hit slime
		if (waterTrace.contents & CONTENTS_SLIME)
		{
			data.m_fFlags |= FX_WATER_IN_SLIME;
		}

		DispatchEffect("watersplash", data);
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlayerCrowbar::ImpactEffect(trace_t &traceHit)
{
	// See if we hit water (we don't do the other impact effects in this case)
	if (ImpactWater(traceHit.startpos, traceHit.endpos))
		return;

	if (!GetOwner()) return;
	
	trace_t impactHit;

	UTIL_TraceLine(traceHit.startpos, traceHit.startpos + GetOwner()->EyeDirection3D() * (GetRange() + 8.0f), MASK_PLAYERSOLID, GetOwner(), COLLISION_GROUP_NONE, &impactHit);

	if (impactHit.fraction != 1.0f)
	{
	//	CEffectData	data;
	//	data.m_vOrigin = impactHit.endpos;
	//	data.m_vNormal = impactHit.plane.normal;
	//	data.m_nEntIndex = impactHit.fraction != 1.0f;
	//	DispatchEffect("BoltImpact", data);
		UTIL_ImpactTrace(&impactHit, DMG_BULLET);
	}
}


//------------------------------------------------------------------------------
// Purpose : Starts the swing of the weapon and determines the animation
// Input   : bIsSecondary - is this a secondary attack?
//------------------------------------------------------------------------------
void CPlayerCrowbar::Swing(int bIsSecondary)
{
	trace_t traceHit;

	// Try a ray
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	pOwner->RumbleEffect(RUMBLE_CROWBAR_SWING, 0, RUMBLE_FLAG_RESTART);

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());

	Vector swingEnd = swingStart + forward * GetRange();
	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
	Activity nHitActivity = ACT_VM_HITCENTER;

	CBaseEntity *ent = traceHit.m_pEnt;
	if (traceHit.DidHit())
	{
		nHitActivity = ACT_VM_HITCENTER;
		if (ent && ent->IsNPC())
		{
			if (ent->ClassMatches("npc_headcra*")
				|| ent->ClassMatches("npc_hopper")
				|| ent->ClassMatches("npc_cscanner")
				|| ent->ClassMatches("npc_sscanner")
				|| ent->ClassMatches("npc_manhack")) // these small enemies are too fast for the slow strong swing
				nHitActivity = ACT_VM_HITCENTER;
			else
				nHitActivity = ACT_VM_HITCENTER2;
		}

		if (bIsSecondary) nHitActivity = ACT_VM_HITCENTER;
	}

	// Like bullets, bludgeon traces have to trace against triggers.
	CTakeDamageInfo triggerInfo(GetOwner(), GetOwner(), GetDamageForActivity(nHitActivity), DMG_CLUB);
	triggerInfo.SetDamagePosition(traceHit.startpos);
	triggerInfo.SetDamageForce(forward);
	TraceAttackToTriggers(triggerInfo, traceHit.startpos, traceHit.endpos, forward);

	if (traceHit.fraction == 1.0)
	{
		float bludgeonHullRadius = 1.4142f * BLUDGEON_HULL_DIM;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point

																// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull(swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
		if (traceHit.fraction < 1.0 && traceHit.m_pEnt)
		{
			Vector vecToTarget = traceHit.m_pEnt->GetAbsOrigin() - swingStart;
			VectorNormalize(vecToTarget);

			float dot = vecToTarget.Dot(forward);

			// YWB:  Make sure they are sort of facing the guy at least...
			if (dot < 0.70721f)
			{
				// Force amiss
				traceHit.fraction = 1.0f;
			}
			else
			{
				nHitActivity = ChooseIntersectionPointAndActivity(traceHit, g_bludgeonMins, g_bludgeonMaxs, pOwner);
			}
		}
	}

	// -------------------------
	//	Miss
	// -------------------------
	if (traceHit.fraction == 1.0f)
	{
		nHitActivity = bIsSecondary ? ACT_VM_MISSCENTER2 : ACT_VM_MISSCENTER;

		// We want to test the first swing again
		Vector testEnd = swingStart + forward * GetRange();

		// See if we happened to hit water
		ImpactWater(swingStart, testEnd);
	}
	else
	{
		if (!bIsSecondary)
		{
			m_iPrimaryAttacks++;
		}
		else
		{
			m_iSecondaryAttacks++;
		}

		gamestats->Event_WeaponFired(pOwner, !bIsSecondary, GetClassname());
	//	Hit(traceHit, nHitActivity, bIsSecondary ? true : false);
	}

	// Send the anim
	SendWeaponAnim(nHitActivity);

	//Setup our next attack times
	m_flNextPrimaryAttack = CURTIME + GetFireRate();
	m_flNextSecondaryAttack = CURTIME + SequenceDuration();

	//Play swing sound
	WeaponSound(SINGLE);
}

void CPlayerCrowbar::Operator_HandleAnimEvent(animevent_t *pEvent, CBaseCombatCharacter *pOperator)
{
	switch (pEvent->event)
	{
	case EVENT_WEAPON_MELEE_HIT:
	{
		CHL2_Player *pOwner = ToHL2Player(GetOwner());
		if (!pOwner)
			return;

		// Try a ray
		trace_t traceHit;

		pOwner->RumbleEffect(RUMBLE_CROWBAR_SWING, 0, RUMBLE_FLAG_RESTART);

		Vector swingStart = pOwner->Weapon_ShootPosition();
		
		Vector forward;

		forward = pOwner->EyeDirection3D(); //GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());

		Vector swingEnd = swingStart + forward * GetRange();
	//	UTIL_TraceLine(swingStart, swingEnd, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);

		int hull_dim = BLUDGEON_HULL_DIM;
		
		float bludgeonHullRadius = 1.4142f /*1.732f*/ * hull_dim;  // hull is +/- 16, so use cuberoot of 2 to determine how big the hull is from center to the corner point
																		// Back off by hull "radius"
		swingEnd -= forward * bludgeonHullRadius;

		UTIL_TraceHull(swingStart, swingEnd, g_bludgeonMins, g_bludgeonMaxs, MASK_SHOT_HULL, pOwner, COLLISION_GROUP_NONE, &traceHit);
	//	NDebugOverlay::Cross3D(swingStart, 6, 255, 0, 0, true, 5.0f);
	//	NDebugOverlay::Cross3D(swingEnd, 6, 0, 255, 0, true, 5.0f);
	//	NDebugOverlay::BoxDirection(swingStart, g_bludgeonMins, g_bludgeonMaxs, forward, 0, 0, 255, 50, 5.0f);
		if (traceHit.fraction < 1.0 /*&& traceHit.m_pEnt*/)
		{
	//		NDebugOverlay::Cross3D(traceHit.endpos, 6, 255, 255, 0, true, 5.0f);
			Hit( traceHit, GetIdealActivity(), (GetIdealActivity() == ACT_VM_HITCENTER ));
		}
	}
		break;

	default:
		BaseClass::Operator_HandleAnimEvent(pEvent, pOperator);
		break;
	}
}

float CPlayerCrowbar::GetDamageForActivity(Activity hitActivity)
{
	if ((GetOwner() != NULL) && (GetOwner()->IsPlayer()))
	{
		if (hitActivity == ACT_VM_HITCENTER2)
			return sk_plr_dmg_crowbar.GetFloat() * 3;
		else if (hitActivity == ACT_VM_HITCENTER)
			return sk_plr_dmg_crowbar.GetFloat() * 1;
		else
			return 0.0f;
	}

	return sk_npc_dmg_crowbar.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: Add in a view kick for this weapon
//-----------------------------------------------------------------------------
void CPlayerCrowbar::AddViewKick(void)
{
	CBasePlayer *pPlayer = ToBasePlayer(GetOwner());

	if (pPlayer == NULL)
		return;

	QAngle punchAng;

	punchAng.x = random->RandomFloat(1.0f, 2.0f);
	punchAng.y = random->RandomFloat(-2.0f, -1.0f);
	punchAng.z = 0.0f;

	pPlayer->ViewPunch(punchAng);
}
#else
#include "basehlcombatweapon.h"
#include "player.h"
#include "gamerules.h"
#include "ammodef.h"
#include "mathlib/mathlib.h"
#include "in_buttons.h"
#include "soundent.h"
#include "basebludgeonweapon.h"
#include "vstdlib/random.h"
#include "npcevent.h"
#include "ai_basenpc.h"
#include "weapon_crowbar.h"
#include "rumble_shared.h"
#include "gamestats.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar    sk_plr_dmg_crowbar		( "sk_plr_dmg_crowbar","0");
ConVar    sk_npc_dmg_crowbar		( "sk_npc_dmg_crowbar","0");

// Crowbar with worker hands skin

class CWeaponCrowbarWorker : public CWeaponCrowbar
{
	DECLARE_DATADESC();
public:
	DECLARE_CLASS(CWeaponCrowbarWorker, CWeaponCrowbar);

	CWeaponCrowbarWorker() {};

	DECLARE_SERVERCLASS();

	void	Precache(void)
	{
		BaseClass::Precache();
	}

	void PrimaryAttack(void)
	{
		BaseClass::SecondaryAttack();
	}

	void SecondaryAttack(void)
	{
		BaseClass::SecondaryAttack();
	}
};

IMPLEMENT_SERVERCLASS_ST(CWeaponCrowbarWorker, DT_WeaponCrowbarWorker)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS(weapon_crowbar_worker, CWeaponCrowbarWorker);
PRECACHE_WEAPON_REGISTER(weapon_crowbar_worker);

BEGIN_DATADESC(CWeaponCrowbarWorker)
END_DATADESC()

//-----------------------------------------------------------------------------
// CWeaponCrowbar
//-----------------------------------------------------------------------------

IMPLEMENT_SERVERCLASS_ST(CWeaponCrowbar, DT_WeaponCrowbar)
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( weapon_crowbar, CWeaponCrowbar );
PRECACHE_WEAPON_REGISTER( weapon_crowbar );

acttable_t CWeaponCrowbar::m_acttable[] = 
{
	{ ACT_MELEE_ATTACK1,	ACT_MELEE_ATTACK_SWING, true },
	{ ACT_IDLE,				ACT_IDLE_ANGRY_MELEE,	false },
	{ ACT_IDLE_ANGRY,		ACT_IDLE_ANGRY_MELEE,	false },
};

IMPLEMENT_ACTTABLE(CWeaponCrowbar);

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
CWeaponCrowbar::CWeaponCrowbar( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: Get the damage amount for the animation we're doing
// Input  : hitActivity - currently played activity
// Output : Damage amount
//-----------------------------------------------------------------------------
float CWeaponCrowbar::GetDamageForActivity( Activity hitActivity )
{
	if ((GetOwner() != NULL) && (GetOwner()->IsPlayer()))
	{
		if (hitActivity == ACT_VM_HITCENTER2)
			return sk_plr_dmg_crowbar.GetFloat() * 3;
		else if (hitActivity == ACT_VM_HITCENTER)
			return sk_plr_dmg_crowbar.GetFloat() * 1;
		else
			return 0.0f;
	}

	return sk_npc_dmg_crowbar.GetFloat();
}

//-----------------------------------------------------------------------------
// Purpose: Add in a view kick for this weapon
//-----------------------------------------------------------------------------
void CWeaponCrowbar::AddViewKick( void )
{
	CBasePlayer *pPlayer  = ToBasePlayer( GetOwner() );
	
	if ( pPlayer == NULL )
		return;

	QAngle punchAng;

	punchAng.x = random->RandomFloat( 1.0f, 2.0f );
	punchAng.y = random->RandomFloat( -2.0f, -1.0f );
	punchAng.z = 0.0f;
	
	pPlayer->ViewPunch( punchAng ); 
}


//-----------------------------------------------------------------------------
// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
//-----------------------------------------------------------------------------
ConVar sk_crowbar_lead_time( "sk_crowbar_lead_time", "0.9" );

int CWeaponCrowbar::WeaponMeleeAttack1Condition( float flDot, float flDist )
{
	// Attempt to lead the target (needed because citizens can't hit manhacks with the crowbar!)
	CAI_BaseNPC *pNPC	= GetOwner()->MyNPCPointer();
	CBaseEntity *pEnemy = pNPC->GetEnemy();
	if (!pEnemy)
		return COND_NONE;

	Vector vecVelocity;
	vecVelocity = pEnemy->GetSmoothedVelocity( );

	// Project where the enemy will be in a little while
	float dt = sk_crowbar_lead_time.GetFloat();
	dt += random->RandomFloat( -0.3f, 0.2f );
	if ( dt < 0.0f )
		dt = 0.0f;

	Vector vecExtrapolatedPos;
	VectorMA( pEnemy->WorldSpaceCenter(), dt, vecVelocity, vecExtrapolatedPos );

	Vector vecDelta;
	VectorSubtract( vecExtrapolatedPos, pNPC->WorldSpaceCenter(), vecDelta );

	if ( fabs( vecDelta.z ) > 70 )
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	Vector vecForward = pNPC->BodyDirection2D( );
	vecDelta.z = 0.0f;
	float flExtrapolatedDist = Vector2DNormalize( vecDelta.AsVector2D() );
	if ((flDist > 64) && (flExtrapolatedDist > 64))
	{
		return COND_TOO_FAR_TO_ATTACK;
	}

	float flExtrapolatedDot = DotProduct2D( vecDelta.AsVector2D(), vecForward.AsVector2D() );
	if ((flDot < 0.7) && (flExtrapolatedDot < 0.7))
	{
		return COND_NOT_FACING_ATTACK;
	}

	return COND_CAN_MELEE_ATTACK1;
}



void CWeaponCrowbar::MeleeAttack(bool strong)
{
	trace_t traceHit;

	// Try a ray
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (!pOwner)
		return;

	pOwner->RumbleEffect(RUMBLE_CROWBAR_SWING, 0, RUMBLE_FLAG_RESTART);

	Vector swingStart = pOwner->Weapon_ShootPosition();
	Vector forward;

	forward = pOwner->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());

	Vector swingEnd = swingStart + forward * GetRange();	
	UTIL_TraceHull(swingStart, swingEnd, Vector(-8,-8,-8), Vector(8,8,8), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &traceHit);

	Activity nHitActivity = ACT_VM_MISSCENTER;

	m_iPrimaryAttacks++;
	gamestats->Event_WeaponFired(pOwner, true, GetClassname());

	CBaseEntity *ent = traceHit.m_pEnt;

	// -------------------------
	//	Miss
	// -------------------------
	if (traceHit.DidHit())
	{
		nHitActivity = ACT_VM_HITCENTER;
		if (ent && ent->IsNPC())
		{
			if (ent->ClassMatches("npc_headcra*")
				|| ent->ClassMatches("npc_hopper")
				|| ent->ClassMatches("npc_cscanner")
				|| ent->ClassMatches("npc_sscanner") 
				|| ent->ClassMatches("npc_manhack")) // these small enemies are too fast for the slow strong swing
				nHitActivity = ACT_VM_HITCENTER;
			else
				nHitActivity = ACT_VM_HITCENTER2;
		}

		if( !strong ) nHitActivity = ACT_VM_HITCENTER;
	}

	// Send the anim
	SendWeaponAnim(nHitActivity);

	//Setup our next attack times
	m_flNextPrimaryAttack = CURTIME + SequenceDuration(); 
	m_flNextSecondaryAttack = CURTIME + GetFireRate();

	//Play swing sound
	WeaponSound(SINGLE);
}

//-----------------------------------------------------------------------------
// Animation event handlers
//-----------------------------------------------------------------------------
void CWeaponCrowbar::HandleAnimEventMeleeHit( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	// Trace up or down based on where the enemy is...
	// But only if we're basically facing that direction
	Vector vecDirection;
	AngleVectors( GetAbsAngles(), &vecDirection );

	CBasePlayer *pPlayer = ToBasePlayer(pOperator);

	CBaseEntity *pEnemy = pPlayer->MyNPCPointer() ? pPlayer->MyNPCPointer()->GetEnemy() : NULL;
	if ( pEnemy )
	{
		Vector vecDelta;
		VectorSubtract( pEnemy->WorldSpaceCenter(), pPlayer->Weapon_ShootPosition(), vecDelta );
		VectorNormalize( vecDelta );
		
		Vector2D vecDelta2D = vecDelta.AsVector2D();
		Vector2DNormalize( vecDelta2D );
		if ( DotProduct2D( vecDelta2D, vecDirection.AsVector2D() ) > 0.8f )
		{
			vecDirection = vecDelta;
		}
	}

	trace_t traceHit;
	Ray_t traceRay;
		
	Vector swingStart = pPlayer->Weapon_ShootPosition();
	Vector forward;

	forward = pPlayer->GetAutoaimVector(AUTOAIM_SCALE_DEFAULT, GetRange());

	Vector swingEnd = swingStart + forward * GetRange();
	traceRay.Init(swingStart, swingEnd, Vector (-128,-128,-128), Vector(128,128,128));
	UTIL_TraceHull(swingStart, swingEnd, Vector(-8, -8, -8), Vector(8, 8, 8), MASK_SHOT_HULL, pPlayer, COLLISION_GROUP_NONE, &traceHit);

	if (traceHit.DidHitWorld())
	{
		AddViewKick();
		CSoundEnt::InsertSound(SOUND_BULLET_IMPACT, traceHit.endpos, 400, 0.2f, pPlayer);
		ImpactEffect(traceHit);
	}
	else if (traceHit.DidHitNonWorldEntity())
	{
		AddViewKick();
		if ( traceHit.m_pEnt)
		{
			ClearMultiDamage();

			FireBulletsInfo_t dmgInfo;
			dmgInfo.m_iShots = 1;
			dmgInfo.m_vecSrc = swingStart;
			dmgInfo.m_vecDirShooting = swingEnd - swingStart;
			dmgInfo.m_vecSpread = vec3_origin;
			dmgInfo.m_flDistance = 128.0f;
			dmgInfo.m_iAmmoType = GetAmmoDef()->Index("MeleeTrace_Club");
			dmgInfo.m_iTracerFreq = 1;
			dmgInfo.m_flDamage = GetDamageForActivity(GetIdealActivity());
			dmgInfo.m_pAttacker = pPlayer;
			dmgInfo.m_flDamageForceScale = 1.0f;
			dmgInfo.m_pAdditionalIgnoreEnt = pPlayer;
			dmgInfo.m_nFlags = FIRE_BULLETS_FIRST_SHOT_ACCURATE;
			
		//	CTakeDamageInfo dmgInfo;
		//	dmgInfo.SetInflictor(pPlayer);
		//	dmgInfo.SetAttacker(pPlayer);
		//	dmgInfo.SetDamage(GetDamageForActivity(GetIdealActivity()));
		//	dmgInfo.SetDamageType(DMG_CLUB);
		//	dmgInfo.ScaleDamageForce(0.25f);
		//	dmgInfo.SetDamagePosition(traceHit.endpos);
		//	CalculateMeleeDamageForce(&dmgInfo, forward, traceHit.endpos);
		//	traceHit.m_pEnt->DispatchTraceAttack(dmgInfo, forward, &traceHit);

			FireBullets(dmgInfo);

			ApplyMultiDamage();

			ImpactEffect(traceHit);

			if (traceHit.m_pEnt->IsNPC())
			{
				EmitSound("DI_Crowbar.Kill");
			}
		}
	}
	else
	{
		WeaponSound( MELEE_MISS );
	}
}

//-----------------------------------------------------------------------------
// Animation event
//-----------------------------------------------------------------------------
void CWeaponCrowbar::Operator_HandleAnimEvent( animevent_t *pEvent, CBaseCombatCharacter *pOperator )
{
	switch( pEvent->event )
	{
	case EVENT_WEAPON_MELEE_HIT:
		HandleAnimEventMeleeHit( pEvent, pOperator );
		break;

	default:
		BaseClass::Operator_HandleAnimEvent( pEvent, pOperator );
		break;
	}
}
#endif