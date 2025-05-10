//========================================================================//
//
// Purpose: Eli. Plain, simple "Eli". The Junkman.
//=============================================================================//


//-----------------------------------------------------------------------------
// Generic NPC - purely for scripted sequence work.
//-----------------------------------------------------------------------------
#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_hull.h"
#include "ai_baseactor.h"
#ifdef DARKINTERVAL
#include "ai_behavior_actbusy.h"
#include "ai_behavior_lead.h"
#include "ai_behavior_follow.h"
#include "npc_playercompanion.h"
#endif
#include "npcevent.h"
#ifdef DARKINTERVAL // for the flashlight
#include "darkinterval/point_projectedlight.h"
#include "spotlightend.h"
#include "sprite.h"
#endif
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// NPC's Anim Events Go Here
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
#ifdef DARKINTERVAL
class CNPC_Eli : public CNPC_PlayerCompanion
#else
class CNPC_Eli : public CAI_BaseActor
#endif
{
public:
#ifdef DARKINTERVAL
	DECLARE_CLASS( CNPC_Eli, CNPC_PlayerCompanion);
#else
	DECLARE_CLASS(CNPC_Eli, CAI_BaseActor);
#endif
	DECLARE_DATADESC();

	CNPC_Eli();
	~CNPC_Eli();

	void	Precache(void);
	void	Spawn( void );
#ifdef DARKINTERVAL
	void	Activate();
#endif
	Class_T Classify ( void );
	void	HandleAnimEvent( animevent_t *pEvent );
	int		GetSoundInterests( void );
	void	SetupWithoutParent( void );
	void	PrescheduleThink( void );
#ifdef DARKINTERVAL
	bool	CreateBehaviors(void);

	void	UseFunc(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value);
	bool	ShouldLookForBetterWeapon() { return false; }
	bool	ShouldRegenerateHealth(void) { return false; }

	Activity NPC_TranslateActivity(Activity activity);

	void	InputStartCarryLantern(inputdata_t &input);
	void	InputStopCarryLantern(inputdata_t &input);
	void	InputStartCarryGG(inputdata_t &input);
	void	InputStopCarryGG(inputdata_t &input);
	void	InputEnableFlashlight(inputdata_t &input);
	void	InputDisableFlashlight(inputdata_t &input);

	COutputEvent m_OnPlayerUse;
private:
	int		m_carrying_mode_int; // if 1, use lantern carry walk/idle anims, if 2, use gravgun carry walk/idle anims
	CHandle<CProjectedLight> m_flashlight_handle;
	CSprite* m_flashlight_halo_sprite_handle;
	CSprite* m_flashlight_core_sprite_handle;
#endif // DARKINTERVAL
};

LINK_ENTITY_TO_CLASS( npc_eli, CNPC_Eli );
#ifdef DARKINTERVAL
LINK_ENTITY_TO_CLASS( npc_junkman, CNPC_Eli); // Eli spawns HL2's Eli. Junkman spawn's DI's Eli.
#endif
#ifdef DARKINTERVAL
// Constructor, destructor
CNPC_Eli::CNPC_Eli()
{
	m_flashlight_handle = NULL;
}
CNPC_Eli::~CNPC_Eli()
{
	if (m_flashlight_handle)
	{
		UTIL_Remove(m_flashlight_handle);
		m_flashlight_handle = NULL;
	}
}
#endif
//-----------------------------------------------------------------------------
// Classify - indicates this NPC's place in the 
// relationship table.
//-----------------------------------------------------------------------------
Class_T	CNPC_Eli::Classify ( void )
{
	return	CLASS_PLAYER_ALLY_VITAL;
}

//-----------------------------------------------------------------------------
// HandleAnimEvent - catches the NPC-specific messages
// that occur when tagged animation frames are played.
//-----------------------------------------------------------------------------
void CNPC_Eli::HandleAnimEvent( animevent_t *pEvent )
{
	switch( pEvent->event )
	{
	case 1:
	default:
		BaseClass::HandleAnimEvent( pEvent );
		break;
	}
}

//-----------------------------------------------------------------------------
// GetSoundInterests - generic NPC can't hear.
//-----------------------------------------------------------------------------
int CNPC_Eli::GetSoundInterests ( void )
{
	return	NULL;
}

//-----------------------------------------------------------------------------
// Precache - precaches all resources this NPC needs
//-----------------------------------------------------------------------------
void CNPC_Eli::Precache()
{
	PrecacheModel(STRING(GetModelName()));
#ifdef DARKINTERVAL
	PrecacheModel("sprites/light_glow01.vmt");
	PrecacheModel("sprites/light_glow03.vmt");
#endif
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Spawn
//-----------------------------------------------------------------------------
void CNPC_Eli::Spawn()
{
	// Eli is allowed to use multiple models, because he appears in the pod.
	// He defaults to his normal model.
	char *szModel = (char *)STRING( GetModelName() );
	if (!szModel || !*szModel)
	{
#ifdef DARKINTERVAL
		if( ClassMatches("npc_eli"))
			szModel = "models/eli.mdl";
		else // (ClassMatches("npc_junkman"))
			szModel = "models/_Characters/eli.mdl";
#else		
		szModel = "models/eli.mdl";
#endif
		SetModelName( AllocPooledString(szModel) );
	}

	Precache();
	SetModel( szModel );

	BaseClass::Spawn();

	SetHullType(HULL_HUMAN);
	SetHullSizeNormal();

	// If Eli has a parent, he's currently inside a pod. Prevent him from moving.
	if ( GetMoveParent() )
	{
		SetSolid( SOLID_BBOX );
		AddSolidFlags( FSOLID_NOT_STANDABLE );
		SetMoveType( MOVETYPE_NONE );

		CapabilitiesAdd( bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD );
		CapabilitiesAdd( bits_CAP_FRIENDLY_DMG_IMMUNE );
	}
	else
	{
		SetupWithoutParent();
	}
#ifndef DARKINTERVAL
	AddEFlags( EFL_NO_DISSOLVE | EFL_NO_MEGAPHYSCANNON_RAGDOLL | EFL_NO_PHYSCANNON_INTERACTION );
#endif
	SetBloodColor( BLOOD_COLOR_RED );
#ifdef DARKINTERVAL
	m_iHealth			= 30;
#else
	m_iHealth			= 8;
#endif
	m_flFieldOfView		= 0.5;// indicates the width of this NPC's forward view cone ( as a dotproduct result )
	m_NPCState			= NPC_STATE_NONE;

	NPCInit();
#ifdef DARKINTERVAL
	m_flashlight_handle = NULL; // start with flashlight disabled
	m_flashlight_halo_sprite_handle = 0; // start with halo disabled
	m_flashlight_core_sprite_handle = 0; // start with halo disabled

	SetUse(&CNPC_Eli::UseFunc);
#endif
}
#ifdef DARKINTERVAL
void CNPC_Eli::Activate()
{
	BaseClass::Activate();
	// Have to do this here because sprites do not go across level transitions
	/* // disabled now - Eli only uses the flashlight in one level
	m_flashlight_halo_sprite_handle = CSprite::SpriteCreate("sprites/light_glow03.vmt", GetLocalOrigin(), FALSE);
	m_flashlight_halo_sprite_handle->SetTransparency(kRenderWorldGlow, 255, 250, 240, 0, kRenderFxNoDissipation);
	m_flashlight_halo_sprite_handle->SetAttachment(this, LookupAttachment("flashlight_halo"));
	m_flashlight_halo_sprite_handle->SetScale(1.0);

	m_flashlight_core_sprite_handle = CSprite::SpriteCreate("sprites/light_glow01.vmt", GetLocalOrigin(), FALSE);
	m_flashlight_core_sprite_handle->SetTransparency(kRenderTransAdd, 255, 250, 240, 0, kRenderFxNoDissipation);
	m_flashlight_core_sprite_handle->SetAttachment(this, LookupAttachment("flashlight_halo"));
	m_flashlight_core_sprite_handle->SetScale(0.3);

	m_flashlight_halo_sprite_handle->SetBrightness(0);
	m_flashlight_core_sprite_handle->SetBrightness(0);
	*/
}
#endif
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Eli::SetupWithoutParent( void )
{
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_NOT_STANDABLE );
	SetMoveType( MOVETYPE_STEP );

	CapabilitiesAdd( bits_CAP_MOVE_GROUND | bits_CAP_OPEN_DOORS | bits_CAP_ANIMATEDFACE | bits_CAP_TURN_HEAD );
#ifndef DARKINTERVAL
	CapabilitiesAdd( bits_CAP_FRIENDLY_DMG_IMMUNE );
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CNPC_Eli::PrescheduleThink( void )
{
	BaseClass::PrescheduleThink();

	// Figure out if Eli has just been removed from his parent
	if ( GetMoveType() == MOVETYPE_NONE && !GetMoveParent() )
	{
		SetupWithoutParent();
		SetupVPhysicsHull();
	}
}
#ifdef DARKINTERVAL
// hacky solution to have special carry anims for lantern and later, gravity gun
Activity CNPC_Eli::NPC_TranslateActivity(Activity activity)
{	
	if (m_carrying_mode_int == 1)
	{
		if (activity == ACT_WALK || activity == ACT_WALK_AIM || activity == ACT_WALK_AGITATED)
		{
			return ACT_WALK_CARRY;
		}
		if (activity == ACT_IDLE || activity == ACT_IDLE_AGITATED)
		{
			return ACT_IDLE_CARRY;
		}
	}
	else if (m_carrying_mode_int == 2)
	{
		if (activity == ACT_WALK || activity == ACT_WALK_AIM || activity == ACT_WALK_AGITATED)
		{
			return ACT_WALK_CARRY2;
		}
		if (activity == ACT_IDLE || activity == ACT_IDLE_AGITATED)
		{
			return ACT_IDLE_CARRY2;
		}
	}
	return BaseClass::NPC_TranslateActivity(activity);
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void CNPC_Eli::UseFunc(CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value)
{
	m_bDontUseSemaphore = true;
	SpeakIfAllowed(TLK_USE);
	m_bDontUseSemaphore = false;

	m_OnPlayerUse.FireOutput(pActivator, pCaller);
}

bool CNPC_Eli::CreateBehaviors(void)
{
	BaseClass::CreateBehaviors();
	AddBehavior(&m_ActBusyBehavior);
	AddBehavior(&m_LeadBehavior);
	AddBehavior(&m_FollowBehavior);
	return true;
}

void CNPC_Eli::InputStartCarryLantern(inputdata_t &input)
{
	m_carrying_mode_int = 1;
	ResetIdealActivity(ACT_IDLE_CARRY);
}

void CNPC_Eli::InputStopCarryLantern(inputdata_t &input)
{
	m_carrying_mode_int = 0;
}

void CNPC_Eli::InputStartCarryGG(inputdata_t &input)
{
	m_carrying_mode_int = 2;
	ResetIdealActivity(ACT_IDLE_CARRY2);
}

void CNPC_Eli::InputStopCarryGG(inputdata_t &input)
{
	m_carrying_mode_int = 0;
}

void CNPC_Eli::InputEnableFlashlight(inputdata_t &inputdata)
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

//	AddEffects(EF_VERYDIMLIGHT);
	AddEffects(EF_NORECEIVESHADOW);

	AddEntityToDarknessCheck(this);
}

void CNPC_Eli::InputDisableFlashlight(inputdata_t &inputdata)
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

//	RemoveEffects(EF_VERYDIMLIGHT);
	RemoveEffects(EF_NORECEIVESHADOW);
	RemoveEntityFromDarknessCheck(this);
}
#endif // DARKINTERVAL

#ifdef DARKINTERVAL
//---------------------------------------------------------
// Save/Restore
//---------------------------------------------------------
BEGIN_DATADESC(CNPC_Eli)
	DEFINE_FIELD(m_flashlight_halo_sprite_handle, FIELD_CLASSPTR),
	DEFINE_FIELD(m_flashlight_core_sprite_handle, FIELD_CLASSPTR),
	DEFINE_FIELD(m_flashlight_handle, FIELD_EHANDLE),
	DEFINE_OUTPUT(m_OnPlayerUse, "OnPlayerUse"),
	DEFINE_USEFUNC(UseFunc),
	DEFINE_INPUTFUNC(FIELD_VOID, "StartCarryLantern", InputStartCarryLantern),
	DEFINE_INPUTFUNC(FIELD_VOID, "StopCarryLantern", InputStopCarryLantern),
	DEFINE_INPUTFUNC(FIELD_VOID, "StartCarryGG", InputStartCarryGG),
	DEFINE_INPUTFUNC(FIELD_VOID, "StopCarryGG", InputStopCarryGG),
	DEFINE_INPUTFUNC(FIELD_VOID, "EnableFlashlight", InputEnableFlashlight),
	DEFINE_INPUTFUNC(FIELD_VOID, "DisableFlashlight", InputDisableFlashlight),
	DEFINE_FIELD(m_carrying_mode_int, FIELD_INTEGER),
END_DATADESC()
#endif
//-----------------------------------------------------------------------------
// AI Schedules Specific to this NPC
//-----------------------------------------------------------------------------
