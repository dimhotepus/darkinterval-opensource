//-----------------------------------------------------------------------------
// Owen Odysseus O'Dell, the ship engineer. Has a flashlight built into
// his costume, and he's capable of opening scripted doors, being useful,
// guiding the player, and providing exposition. 
// Also, he's almost indestructable. 
//-----------------------------------------------------------------------------
#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_behavior_follow.h"
#include "ai_hull.h"
#include "ai_interactions.h"
#include "ai_playerally.h"
#include "npc_citizen17.h"
#include "npcevent.h"
#include "sprite.h"
#include "point_projectedlight.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

Activity ACT_CIT_BLINDED;
#define MYMODEL "models/_Characters/odell.mdl"
//-----------------------------------------------------------------------------
class CNPC_Odell : public /*CAI_PlayerAlly*/ CNPC_PlayerCompanion
{
public:
	DECLARE_CLASS(CNPC_Odell, /*CAI_PlayerAlly*/ CNPC_PlayerCompanion);
	DECLARE_DATADESC();
	DEFINE_CUSTOM_AI;

	//	virtual Disposition_t IRelationType(CBaseEntity *pTarget);
	CNPC_Odell();
	~CNPC_Odell();
	void	Precache(void);
	void	Spawn(void);
	void	Activate();
	int		OnTakeDamage_Alive(const CTakeDamageInfo &info);
	Class_T Classify(void) { return CLASS_NONE; };
	void	HandleAnimEvent(animevent_t *pEvent);
	int		GetSoundInterests(void) { return NULL; };
	bool	CreateBehaviors(void);
	int		SelectSchedule(void);
	void	NPCThink(void);
	void	InputEnableFlashlight(inputdata_t &inputdata);
	void	InputDisableFlashlight(inputdata_t &inputdata);
	//bool	ShouldPlayerAvoid(void) { return false; }

	virtual bool HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);
	COutputEvent m_OnPlayerUse;
	void	UseFunc(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);

	bool	m_bFlashlight;

private:
	CHandle<CProjectedLight> m_flashlight_handle;
	CAI_FollowBehavior	m_FollowBehavior;
	CSprite* m_flashlight_halo_sprite_handle;
	CSprite* m_flashlight_core_sprite_handle;
};

LINK_ENTITY_TO_CLASS(npc_odell, CNPC_Odell);

BEGIN_DATADESC(CNPC_Odell)
DEFINE_FIELD(m_bFlashlight, FIELD_BOOLEAN),
DEFINE_FIELD(m_flashlight_halo_sprite_handle, FIELD_CLASSPTR),
DEFINE_FIELD(m_flashlight_core_sprite_handle, FIELD_CLASSPTR),
DEFINE_FIELD(m_flashlight_handle, FIELD_EHANDLE),
DEFINE_INPUTFUNC(FIELD_VOID, "EnableFlashlight", InputEnableFlashlight),
DEFINE_INPUTFUNC(FIELD_VOID, "DisableFlashlight", InputDisableFlashlight),
DEFINE_OUTPUT(m_OnPlayerUse, "OnPlayerUse"),
DEFINE_USEFUNC(UseFunc),
END_DATADESC()

CNPC_Odell::CNPC_Odell()
{
	m_flashlight_handle = NULL;
}
CNPC_Odell::~CNPC_Odell()
{
	if (m_flashlight_handle)
	{
		UTIL_Remove(m_flashlight_handle);
		m_flashlight_handle = NULL;
	}
}

void CNPC_Odell::Precache()
{
	PrecacheModel(MYMODEL);
	PrecacheModel("sprites/light_glow01.vmt");
	PrecacheModel("sprites/light_glow03.vmt");
	BaseClass::Precache();
}

void CNPC_Odell::Spawn()
{
	Precache();
	SetModel(MYMODEL);

	BaseClass::Spawn();

//	SetHullType(HULL_HUMAN);
//	SetHullSizeNormal();

//	SetSolid(SOLID_BBOX);
//	AddSolidFlags(FSOLID_NOT_STANDABLE);
//	SetMoveType(MOVETYPE_STEP);
	SetBloodColor(BLOOD_COLOR_RED);
	m_iHealth = 30; //2599;
	m_flFieldOfView = 0.5;// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	m_NPCState = NPC_STATE_NONE;
	SetImpactEnergyScale(0.0f); // no physics damage on the gman
	CapabilitiesAdd(bits_CAP_MOVE_CLIMB);

//	m_takedamage = DAMAGE_NO;
	AddEFlags(EFL_NO_DISSOLVE /*| EFL_NO_MEGAPHYSCANNON_RAGDOLL*/);
	AddSpawnFlags(SF_NPC_NO_PLAYER_PUSHAWAY);

	m_flashlight_handle = NULL;
	m_bFlashlight = false; // start with flashlight disabled
	m_flashlight_halo_sprite_handle = 0; // start with halo disabled
	m_flashlight_core_sprite_handle = 0; // start with halo disabled

	NPCInit();

	SetUse(&CNPC_Odell::UseFunc);
}

void CNPC_Odell::Activate()
{
	BaseClass::Activate();
	// Have to do this here because sprites do not go across level transitions
	m_flashlight_halo_sprite_handle = CSprite::SpriteCreate("sprites/light_glow03.vmt", GetLocalOrigin(), FALSE);
	m_flashlight_halo_sprite_handle->SetTransparency(kRenderWorldGlow, 255, 250, 240, 0, kRenderFxNoDissipation);
	m_flashlight_halo_sprite_handle->SetAttachment(this, LookupAttachment("flashlight_halo"));
	m_flashlight_halo_sprite_handle->SetScale(1.0);

	m_flashlight_core_sprite_handle = CSprite::SpriteCreate("sprites/light_glow01.vmt", GetLocalOrigin(), FALSE);
	m_flashlight_core_sprite_handle->SetTransparency(kRenderTransAdd, 255, 250, 240, 0, kRenderFxNoDissipation);
	m_flashlight_core_sprite_handle->SetAttachment(this, LookupAttachment("flashlight_halo"));
	m_flashlight_core_sprite_handle->SetScale(0.3);

	if (m_bFlashlight)
	{
		m_flashlight_halo_sprite_handle->SetBrightness(100);
		m_flashlight_core_sprite_handle->SetBrightness(240);
	}
	else
	{
		m_flashlight_halo_sprite_handle->SetBrightness(0);
		m_flashlight_core_sprite_handle->SetBrightness(0);
	}

}

void CNPC_Odell::NPCThink(void)
{
	BaseClass::NPCThink();
	/*
	if (GetNavType() == NAV_CLIMB)
	{
	SetCollisionGroup(COLLISION_GROUP_PLAYER);
	AddSolidFlags(FSOLID_NOT_SOLID);
	}
	else
	{
	SetCollisionGroup(COLLISION_GROUP_NPC);
	RemoveSolidFlags(FSOLID_NOT_SOLID);
	}
	*/
}

void CNPC_Odell::InputEnableFlashlight(inputdata_t &inputdata)
{
	Vector origin; QAngle angle;
	GetAttachment(LookupAttachment("flashlight"), origin, angle);

	//	inputdata_t fov;
	//	fov.value.SetFloat(50.0f);

	//	inputdata_t shadows;
	//	shadows.value.SetBool(true);

	//	inputdata_t volumetrics;
	//	volumetrics.value.SetBool(false);

	//	inputdata_t color;
	Vector vc = Vector(1.85, 1.85, 2.15);
	//	color.value.SetVector3D(vc);

	//	inputdata_t texture;
	//	texture.value.SetString(MAKE_STRING("effects/flashlight_border"));

	if (m_flashlight_handle != NULL)
	{
		UTIL_Remove(m_flashlight_handle); // Kill, then recreate
		m_flashlight_handle = NULL;
	}

	m_flashlight_handle = dynamic_cast< CProjectedLight * >(CreateEntityByName("env_flashlight"));
	m_flashlight_handle->Teleport(&origin, &angle, NULL);
	m_flashlight_handle->SetFOV(90);
	m_flashlight_handle->SetNearZ(8); // else it clips into the lantern model (placeholder)
	m_flashlight_handle->SetBrightness(1000);
	m_flashlight_handle->SetSpotlightColor(vc);
	m_flashlight_handle->SetParent(this, LookupAttachment("flashlight"));
	m_flashlight_handle->EnableShadows(); // necessary to make Eli obstruct the backward projected light, ugh
	m_flashlight_handle->SetOwnerEntity(this);
	m_flashlight_handle->TurnOn();

	if (m_flashlight_halo_sprite_handle != 0)
		m_flashlight_halo_sprite_handle->SetBrightness(100);
	if (m_flashlight_core_sprite_handle != 0)
		m_flashlight_core_sprite_handle->SetBrightness(100);

	AddEffects(EF_VERYDIMLIGHT);
	AddEffects(EF_NORECEIVESHADOW);

	AddEntityToDarknessCheck(this);
}

void CNPC_Odell::InputDisableFlashlight(inputdata_t &inputdata)
{
	if (m_flashlight_handle != NULL)
	{
		UTIL_Remove(m_flashlight_handle);
		m_flashlight_handle = NULL;
	}

	if (m_flashlight_halo_sprite_handle != 0)
		m_flashlight_halo_sprite_handle->SetBrightness(0);
	if (m_flashlight_core_sprite_handle != 0)
		m_flashlight_core_sprite_handle->SetBrightness(0);

	RemoveEffects(EF_VERYDIMLIGHT);
	RemoveEffects(EF_NORECEIVESHADOW);
	RemoveEntityFromDarknessCheck(this);
}

// Currently holds triggers for flashlight. 
void CNPC_Odell::HandleAnimEvent(animevent_t *pEvent)
{
	switch (pEvent->event)
	{
	case 1:
	default:
		BaseClass::HandleAnimEvent(pEvent);
		break;
	}
}

int CNPC_Odell::OnTakeDamage_Alive(const CTakeDamageInfo &info)
{
	CTakeDamageInfo newInfo = info;

	// Play a generic reaction to any player attacks. 
	if (newInfo.GetAttacker() && newInfo.GetAttacker()->IsPlayer())
	{
		if (m_NPCState != NPC_STATE_SCRIPT)
		{
			SetIdealActivity(GetSequenceActivity(LookupSequence("photo_react_blind")));
		}
		newInfo.ScaleDamage(0.0f);
	}

	return BaseClass::OnTakeDamage_Alive(newInfo);
}

bool CNPC_Odell::CreateBehaviors()
{
	AddBehavior(&m_FollowBehavior);

	return BaseClass::CreateBehaviors();
}

// Just blank.
int CNPC_Odell::SelectSchedule(void)
{
	if (!BehaviorSelectSchedule())
	{
	}
	return BaseClass::SelectSchedule();
}

// Also blank.
bool CNPC_Odell::HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt)
{
	return false;
}

void CNPC_Odell::UseFunc(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	m_bDontUseSemaphore = true;
	SpeakIfAllowed(TLK_USE);
	m_bDontUseSemaphore = false;

	m_OnPlayerUse.FireOutput(pActivator, pCaller);
}


AI_BEGIN_CUSTOM_NPC(npc_odell, CNPC_Odell)
//DECLARE_USES_SCHEDULE_PROVIDER(CAI_LeadBehavior)
AI_END_CUSTOM_NPC()