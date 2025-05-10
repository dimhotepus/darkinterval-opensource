#include "cbase.h"
#include "ai_basenpc.h"
#include "ai_hull.h"
#include "baseanimating.h"
#include "ndebugoverlay.h"
#include "sprite.h"

enum LightstalkFade_t
{
	FADE_NONE,
	FADE_OUT,
	FADE_IN,
};

class CSprite;

// the light fungi
class CNPC_LightFungi : public CBaseAnimating
{
public:
	DECLARE_CLASS(CNPC_LightFungi, CBaseAnimating)
	CNPC_LightFungi(void); 
	int ObjectCaps(void)
	{
		return BaseClass::ObjectCaps() | FCAP_DONT_TRANSITION_EVER;
	}
	void Spawn(void);
	void Activate(void);
	void Precache(void);
	void OnRestore();
	void Think(void);
	void InputSetLightRadius(inputdata_t &input);
	void InputSetLightColor(inputdata_t &input);
//	void SetNextThinkByDistance(void);

	color32		m_LightColor;
	CNetworkVar(bool, m_bLight); // turn on, turn off
	CNetworkArray( int, m_lightVars, 4);
	CNetworkVar(float, m_LightRadius); // radius for dyn. light
private:
//	float				m_flStartFadeTime;
//	LightstalkFade_t	m_nFadeDir;
	CSprite* m_pGlow;
public:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};

#define	FUNGI_MODEL				"models/_Monsters/Xen/Xen_Fungus.mdl"
#define FUNGI_GLOW_SPRITE		"sprites/glow03.vmt"

LINK_ENTITY_TO_CLASS(npc_lightfungi, CNPC_LightFungi);

BEGIN_DATADESC(CNPC_LightFungi)

DEFINE_FIELD(m_pGlow, FIELD_CLASSPTR),
//DEFINE_FIELD(m_flStartFadeTime, FIELD_TIME),
//DEFINE_FIELD(m_nFadeDir, FIELD_INTEGER),
DEFINE_FIELD(m_bLight, FIELD_BOOLEAN),
DEFINE_KEYFIELD(m_LightColor, FIELD_COLOR32, "lightcolor"), // this keyvalue coresponds with the fgd entry
DEFINE_KEYFIELD(m_LightRadius, FIELD_FLOAT, "radius"), // this keyvalue coresponds with the fgd entry
DEFINE_THINKFUNC( Think ),
DEFINE_INPUTFUNC( FIELD_FLOAT, "SetLightRadius", InputSetLightRadius ),
DEFINE_INPUTFUNC( FIELD_COLOR32, "SetLightColor", InputSetLightColor ),

// Function Pointers
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CNPC_LightFungi, DT_NPC_LightFungi)
SendPropBool(SENDINFO(m_bLight)), // we're sending it to the client because dyn. light is handled there
SendPropArray(

SendPropInt(SENDINFO_ARRAY(m_lightVars), 32, SPROP_NOSCALE),

m_lightVars),
SendPropFloat(SENDINFO(m_LightRadius), 0, SPROP_NOSCALE),
END_SEND_TABLE()

CNPC_LightFungi::CNPC_LightFungi(void)
{
	m_bLight = false;
}

void CNPC_LightFungi::Precache(void)
{
	PrecacheModel(FUNGI_MODEL);
	PrecacheModel(FUNGI_GLOW_SPRITE);
	BaseClass::Precache();
}

void CNPC_LightFungi::Spawn(void)
{
	Precache();
	BaseClass::Spawn();

	SetModel(FUNGI_MODEL);

	m_lightVars.Set(0, m_LightColor.r);
	m_lightVars.Set(1, m_LightColor.g);
	m_lightVars.Set(2, m_LightColor.b);
	m_lightVars.Set(3, m_LightColor.a);
	
	m_bLight = false;

	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID);

	SetModelScale(RandomFloat(0.75, 1.5), 0.0f); // we randomise the size so it's not always clones of the one model
	UpdateModelScale();

	SetSequence(SelectWeightedSequence(ACT_IDLE));
	SetCycle(random->RandomFloat(0.0f, 1.0f));
	m_flPlaybackRate = random->RandomFloat(0.5, 1.5); // we also randomise the animation rate so they don't "breathe" in sync with each other--
	SetCycle(random->RandomFloat(0.0f, 0.9f));	// more visual variety when put in clusters

	if (m_LightRadius > 0)
	{
		m_pGlow = CSprite::SpriteCreate(FUNGI_GLOW_SPRITE, GetLocalOrigin() + Vector(0, 0, (WorldAlignMins().z + WorldAlignMaxs().z)*0.5), false);
		m_pGlow->SetTransparency(kRenderGlow, m_LightColor.r, m_LightColor.g, m_LightColor.b, m_LightColor.a, kRenderFxGlowShell);
		m_pGlow->SetScale(2.0);
		m_pGlow->SetHDRColorScale(0.25); // too bright in HDR. Scale down.
		m_pGlow->SetAttachment(this, 1); // attachment defined in the qc
	}

	AddEffects(EF_NOSHADOW);
}

void CNPC_LightFungi::Activate(void)
{
	BaseClass::Activate();

	// Idly think
	SetThink(&CNPC_LightFungi::Think);
	SetNextThink(CURTIME + RandomFloat(0.05f, 0.15f));
}

void CNPC_LightFungi::OnRestore()
{
	SetThink(&CNPC_LightFungi::Think);
	SetNextThink(CURTIME + RandomFloat(0.05f, 0.15f));
}

void CNPC_LightFungi::Think(void)
{
//	StudioFrameAdvance();

	if (m_LightRadius == 0.0f)
	{
		SetNextThink(CURTIME + 0.1f);
		return;
	}

	m_bLight = m_LightRadius > 1.0f;

	/*
	bool InPVS = ((UTIL_FindClientInPVS(edict()) != NULL)
		|| (UTIL_ClientPVSIsExpanded()
			&& UTIL_FindClientInVisibilityPVS(edict())));

	// See how close the player is
	CBasePlayer *pPlayerEnt = AI_GetSinglePlayer();
	float flDistToPlayerSqr = (GetAbsOrigin() - pPlayerEnt->GetAbsOrigin()).LengthSqr();

	// If they're far enough away, just wait to think again
	if (flDistToPlayerSqr > Square((m_LightRadius * 2) * 8) || !InPVS)
	{
		m_bLight = false;
	}
	if (m_nFadeDir && m_pGlow)
	{
		float flFadePercent = (CURTIME - m_flStartFadeTime) / 2;

		if (flFadePercent > 1)
		{
			m_nFadeDir = FADE_NONE;
		}
		else
		{
			if (m_nFadeDir == FADE_IN)
			{
				m_pGlow->SetBrightness(120 * flFadePercent);
			}
			else
			{
				m_pGlow->SetBrightness(0);
			}
		}
	}
*/
	if (m_bLight)
	{
		m_lightVars.Set(0, m_LightColor.r);
		m_lightVars.Set(1, m_LightColor.g);
		m_lightVars.Set(2, m_LightColor.b);
		m_lightVars.Set(3, m_LightColor.a);
	}
	
	SetNextThink(CURTIME + 0.1f);
	
	/*
	m_bLight = m_LightRadius > 0;

	if (m_nFadeDir && m_pGlow)
	{
		float flFadePercent = (CURTIME - m_flStartFadeTime) / 2;

		if (flFadePercent > 1)
		{
			m_nFadeDir = FADE_NONE;
		}
		else
		{
			if (m_nFadeDir == FADE_IN)
			{
				m_pGlow->SetBrightness(120 * flFadePercent);
			}
			else
			{
				m_pGlow->SetBrightness(0);
			}
		}
	}

	StudioFrameAdvance();
	SetNextThink(CURTIME + 0.15f);
*/
}

void CNPC_LightFungi::InputSetLightRadius(inputdata_t &input)
{
	m_LightRadius = input.value.Float();
}
void CNPC_LightFungi::InputSetLightColor(inputdata_t &input)
{
	m_LightColor = input.value.Color32();
}

// the lightstalk
class CNPC_LightStalk : public CAI_BaseNPC
{
public:
	DECLARE_CLASS( CNPC_LightStalk, CAI_BaseNPC);

	CNPC_LightStalk( void );

	int ObjectCaps(void)
	{
		return BaseClass::ObjectCaps() | FCAP_DONT_TRANSITION_EVER;
	}
	void		Spawn( void );
	void		Precache( void );
	void		Touch( CBaseEntity *pOther );
	void		OnSave();
	void		OnRestore();
	void		Think( void );
	void		LightRise( void );
	void		LightLower( void );

	COutputEvent m_OnRise;
	COutputEvent m_OnLower;

	color32		m_LightColor;
	
	CNetworkVar( bool, m_bLight ); // again, we network these into the client
	CNetworkVar( int, m_lightr );
	CNetworkVar( int, m_lightg );
	CNetworkVar( int, m_lightb );
	CNetworkVar(float, m_LightRadius); // radius for dyn. light

	int			m_lighta;

private:
	bool				m_bDead;
	int					m_iHide;
	float				m_flFadePercent;
	float				m_flHideEndTime;
	float				m_flStartFadeTime;
	LightstalkFade_t	m_nFadeDir;
	CSprite*			m_pGlow;

public:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();
};

#define	LIGHTSTALK_MODEL			"models/_Monsters/Xen/xen_light.mdl"
#define LIGHTSTALK_GLOW_SPRITE		"sprites/glow03.vmt"
#define LIGHTSTALK_HIDE_TIME		5
#define	LIGHTSTALK_FADE_TIME		2
#define LIGHTSTALK_MAX_BRIGHT		120

LINK_ENTITY_TO_CLASS(npc_lightstalk, CNPC_LightStalk);

BEGIN_DATADESC( CNPC_LightStalk )

	DEFINE_FIELD( m_iHide,				FIELD_INTEGER ),
	DEFINE_FIELD( m_pGlow,				FIELD_CLASSPTR ),
	DEFINE_FIELD( m_flStartFadeTime,	FIELD_TIME ),
	DEFINE_FIELD( m_nFadeDir,			FIELD_INTEGER ),
	DEFINE_FIELD( m_flHideEndTime,		FIELD_TIME ),
	DEFINE_FIELD( m_bLight,				FIELD_BOOLEAN ),
	DEFINE_FIELD( m_flFadePercent,		FIELD_FLOAT ),
	DEFINE_KEYFIELD(m_bDead,			FIELD_BOOLEAN, "dead"),
	DEFINE_KEYFIELD( m_LightColor,		FIELD_COLOR32, "lightcolor" ),
	DEFINE_KEYFIELD(m_LightRadius,		FIELD_FLOAT, "radius"), // this keyvalue corresponds with the fgd entry
	DEFINE_THINKFUNC(Think),
	DEFINE_ENTITYFUNC( Touch ),

	// outputs
	DEFINE_OUTPUT( m_OnRise,  "OnRise" ),
	DEFINE_OUTPUT( m_OnLower, "OnLower" ),

	// Function Pointers
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST( CNPC_LightStalk, DT_NPC_LightStalk )
	SendPropBool( SENDINFO( m_bLight )), // these go to the client to be handled there
	SendPropInt( SENDINFO( m_lightr ), 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_lightg ), 1, SPROP_UNSIGNED ),
	SendPropInt( SENDINFO( m_lightb ), 1, SPROP_UNSIGNED ),
	SendPropFloat(SENDINFO(m_LightRadius), 0, SPROP_NOSCALE),
END_SEND_TABLE()

CNPC_LightStalk::CNPC_LightStalk( void )
{
	m_bLight = false; // init with light disabled, then create/enable
	m_flFadePercent = 0;
	m_iHide = 0;
	m_bDead = 0;
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_LightStalk::Precache( void )
{
	PrecacheModel( LIGHTSTALK_MODEL );
	if( !m_bDead)
		PrecacheModel( LIGHTSTALK_GLOW_SPRITE );

	BaseClass::Precache();
}
//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_LightStalk::Spawn( void )
{
	Precache();
	SetModel( LIGHTSTALK_MODEL );

	m_lightr = m_LightColor.r;
	m_lightg = m_LightColor.g;
	m_lightb = m_LightColor.b;
	m_lighta = m_LightColor.a;

	m_bDead = 0;
	m_bLight = false; // init with light disabled, then create/enable
	
	SetSolid( SOLID_BBOX );
	AddSolidFlags( FSOLID_TRIGGER ); // this NPC serves as a trigger - makes reactions possible
	AddSolidFlags( FSOLID_NOT_SOLID );

	if (!m_bDead)
	{
		// the size of the trigger. When the player touches this virtual box, stuff happens.
		UTIL_SetSize( this, Vector(-80,-80,-80), Vector(80,80,80));
		SetActivity(ACT_IDLE);
		SetCycle(random->RandomFloat(0.0f, 0.9f));
		SetThink(&CNPC_LightStalk::Think);
		SetNextThink(CURTIME + 0.1f);
		SetTouch(&CNPC_LightStalk::Touch);
	}
	else
	{
		SetActivity(ACT_CROUCHIDLE);
		SetThink(NULL);
		SetTouch(NULL);
		m_nSkin = 2;
	}

	// randomise for more random idle animation playback
	SetCycle(random->RandomFloat(0.0f, 0.9f));

	if (m_LightRadius > 0 && !m_bDead)
	{
		m_pGlow = CSprite::SpriteCreate(LIGHTSTALK_GLOW_SPRITE, GetLocalOrigin() + Vector(0, 0, (WorldAlignMins().z + WorldAlignMaxs().z)*0.5), false);
		m_pGlow->SetTransparency(kRenderGlow, m_lightr, m_lightg, m_lightb, m_lighta, kRenderFxGlowShell);
		m_pGlow->SetScale(0.25);
		m_pGlow->SetHDRColorScale(0.25); // to bright in HDR. Scale down.
		m_pGlow->SetAttachment(this, 1);
		LightRise();

		AddEffects(EF_NOSHADOW);
	}

	BaseClass::Spawn();
}
//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_LightStalk::OnRestore()
{
	m_lightr = m_LightColor.r;
	m_lightg = m_LightColor.g;
	m_lightb = m_LightColor.b;
	m_lighta = m_LightColor.a;

	BaseClass::OnRestore();
}
//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_LightStalk::Think( void )
{
	bool InPVS = ((UTIL_FindClientInPVS(edict()) != NULL)
		|| (UTIL_ClientPVSIsExpanded()
			&& UTIL_FindClientInVisibilityPVS(edict())));

	// See how close the player is
	CBasePlayer *pPlayerEnt = AI_GetSinglePlayer();
	float flDistToPlayerSqr = (GetAbsOrigin() - pPlayerEnt->GetAbsOrigin()).LengthSqr();

	float fadeDist = m_fadeMaxDist;
	if (fadeDist <= 0) fadeDist = 2500; // override in case it was the default fade - otherwise this works incorrectly

	// If they're far enough away, just wait to think again
	if (flDistToPlayerSqr > (fadeDist * fadeDist) || !InPVS)
	{
		m_bLight = false;
		SetNextThink(CURTIME + 0.3f);
		SetActivity(ACT_IDLE);
		m_iHide = 1;
		return;
	}

	StudioFrameAdvance();
	SetNextThink(CURTIME + 0.1f);
	
	switch( m_iHide )
	{
	case 0: // standing up - ACT_STAND
		if (IsActivityFinished())
		{
			SetActivity(ACT_IDLE);
			m_iHide = 1;
		}
		break;
	case 1: // standing full up - ACT_IDLE
		break;
	case 2: // going down - ACT_CROUCH
		if (IsActivityFinished())
		{
			SetActivity(ACT_CROUCHIDLE);
			m_iHide = 3;
		}
		break;
	case 3: // hiding - ACT_CROUCHIDLE
		if (CURTIME > m_flHideEndTime)
		{
			m_OnRise.FireOutput(this, this);
			SetActivity(ACT_STAND);
			m_iHide = 0;
			m_nSkin = 0;
			m_nFadeDir = FADE_IN;
			m_flStartFadeTime = CURTIME;
		}
		break;
	default:
		break;
	}

	if (m_nFadeDir && m_pGlow)
	{
		m_flFadePercent = (CURTIME - m_flStartFadeTime)/LIGHTSTALK_FADE_TIME;

		if (m_flFadePercent > 1)
		{
			m_nFadeDir = FADE_NONE;
		}
		else
		{
			if  (m_nFadeDir == FADE_IN)
			{
				m_pGlow->SetBrightness(LIGHTSTALK_MAX_BRIGHT*m_flFadePercent);
				m_bLight = false;
			}
			else
			{
				m_pGlow->SetBrightness(LIGHTSTALK_MAX_BRIGHT*(1-m_flFadePercent));
				// Fade out immedately
				m_pGlow->SetBrightness(0);
				m_bLight = true;
			}
		}
	}
}
//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_LightStalk::Touch( CBaseEntity *pOther )
{
	if ( pOther->IsPlayer() ) // only react to the player
	{
		m_flHideEndTime = CURTIME + LIGHTSTALK_HIDE_TIME;
		if (m_iHide < 2 ) // either standing or trying to rise
		{
			m_OnLower.FireOutput(this, this);
			SetActivity(ACT_CROUCH);
			m_iHide = 2;
			m_nSkin = 1;
			m_nFadeDir = FADE_OUT;
			m_flStartFadeTime = CURTIME;
		}
	}
}
//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_LightStalk::LightRise( void )
{
	m_OnRise.FireOutput( this, this );
	SetActivity(ACT_STAND);
	m_iHide				= 0;
	m_nSkin				= 0;
	m_nFadeDir			= FADE_IN;
	m_flStartFadeTime	= CURTIME;
}
//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CNPC_LightStalk::LightLower( void )
{
	m_OnLower.FireOutput( this, this );
	SetActivity(ACT_CROUCH);
	m_iHide				= 2;
	m_nSkin				= 1;
	m_nFadeDir			= FADE_OUT;
	m_flStartFadeTime	= CURTIME;
}