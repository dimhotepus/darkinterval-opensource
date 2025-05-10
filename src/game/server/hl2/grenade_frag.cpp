//========================================================================//
//
// Purpose: 
// DI changes that are not ifdef'd: 
// - removed all CS/TF/MP stuff
// $NoKeywords: $
//=============================================================================//

#include "cbase.h"
#include "basegrenade_shared.h"
#include "grenade_frag.h"
#include "sprite.h"
#include "env_spritetrail.h"
#include "soundent.h"
#ifdef DARKINTERVAL
#include "props.h" // to spawn shrapnel, which is physics props
#include "te_effect_dispatch.h" // for the smoke grenade
#include "world.h" // to set damage source as "world" for shrapnel
#endif
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define FRAG_GRENADE_BLIP_FREQUENCY			1.0f
#define FRAG_GRENADE_BLIP_FAST_FREQUENCY	0.3f

#define FRAG_GRENADE_GRACE_TIME_AFTER_PICKUP 1.5f
#define FRAG_GRENADE_WARN_TIME 1.5f

const float GRENADE_COEFFICIENT_OF_RESTITUTION = 0.2f;

ConVar sk_plr_dmg_fraggrenade	( "sk_plr_dmg_fraggrenade","0");
ConVar sk_npc_dmg_fraggrenade	( "sk_npc_dmg_fraggrenade","0");
ConVar sk_fraggrenade_radius	( "sk_fraggrenade_radius", "0");

#define GRENADE_MODEL "models/Weapons/w_grenade.mdl"

class CGrenadeFrag : public CBaseGrenade
{
	DECLARE_CLASS( CGrenadeFrag, CBaseGrenade );

#if !defined( CLIENT_DLL )
	DECLARE_DATADESC();
#endif
					
	~CGrenadeFrag( void );

public:
	void	Spawn( void );
	void	OnRestore( void );
	void	Precache( void );
	bool	CreateVPhysics( void );
	void	CreateEffects( void );
	void	SetTimer( float detonateDelay, float warnDelay );
	void	SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity );
	int		OnTakeDamage( const CTakeDamageInfo &inputInfo );
	void	BlipSound() { EmitSound( "Grenade.Blip" ); }
	void	DelayThink();
#ifdef DARKINTERVAL
	void	Detonate(void);
	void    CreateSmokeEffect(void);
#endif
	void	VPhysicsUpdate( IPhysicsObject *pPhysics );
	void	OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason );
	void	SetCombineSpawned( bool combineSpawned ) { m_combineSpawned = combineSpawned; }
	bool	IsCombineSpawned( void ) const { return m_combineSpawned; }
#ifdef DARKINTERVAL
	void	SetGrenadeType(int grenadeType) { m_grenadeType = grenadeType; }
	void	SetNextBlipTime(float nextBlipTime) { m_flNextBlipTime = nextBlipTime; } // for cooking
#endif
	void	SetPunted( bool punt ) { m_punted = punt; }
	bool	WasPunted( void ) const { return m_punted; }

	// this function only used in episodic.
#if defined(HL2_EPISODIC) && 0 // FIXME: HandleInteraction() is no longer called now that base grenade derives from CBaseAnimating
	bool	HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt);
#endif 

	// Inputs
	void	InputSetTimer( inputdata_t &inputdata );
#ifdef DARKINTERVAL
	// Outputs
	COutputEvent m_OnDetonate;
#endif

protected:
	CHandle<CSprite>		m_pMainGlow;
	CHandle<CSpriteTrail>	m_pGlowTrail;

	float	m_flNextBlipTime;
	bool	m_inSolid;
	bool	m_combineSpawned;
#ifdef DARKINTERVAL
	int		m_grenadeType;
#endif
	bool	m_punted;
};

LINK_ENTITY_TO_CLASS( npc_grenade_frag, CGrenadeFrag );


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CGrenadeFrag::~CGrenadeFrag( void )
{
}

void CGrenadeFrag::Precache(void)
{
	PrecacheModel(GRENADE_MODEL);
	PrecacheScriptSound("Grenade.Blip");

	PrecacheModel("sprites/redglow1.vmt");
	PrecacheModel("sprites/bluelaser1.vmt");
#ifdef DARKINTERVAL
	PrecacheModel("models/gibs/grenade_shrapnel.mdl");
	PrecacheScriptSound("NPC_Metropolice.GrenadeDetonate");
#endif
	BaseClass::Precache();
}

void CGrenadeFrag::Spawn( void )
{
	Precache( );

	SetModel( GRENADE_MODEL );

	if( GetOwnerEntity() && GetOwnerEntity()->IsPlayer() )
	{
		m_flDamage		= sk_plr_dmg_fraggrenade.GetFloat();
		m_DmgRadius		= sk_fraggrenade_radius.GetFloat();
	}
	else
	{
		m_flDamage		= sk_npc_dmg_fraggrenade.GetFloat();
		m_DmgRadius		= sk_fraggrenade_radius.GetFloat();
	}

	m_takedamage	= DAMAGE_YES;
	m_iHealth		= 1;

	SetSize( -Vector(4,4,4), Vector(4,4,4) );
	SetCollisionGroup( COLLISION_GROUP_WEAPON );
	CreateVPhysics();
#ifdef DARKINTERVAL // for cooking
	if (m_flNextBlipTime <= CURTIME)
	{
		BlipSound();
		if (CURTIME >= m_flDetonateTime - 1.5f)
		{
			m_flNextBlipTime = CURTIME + FRAG_GRENADE_BLIP_FAST_FREQUENCY;
		}
		else
		{
			m_flNextBlipTime = CURTIME + FRAG_GRENADE_BLIP_FREQUENCY;
		}
	}
#else
	BlipSound();
	m_flNextBlipTime = CURTIME + FRAG_GRENADE_BLIP_FREQUENCY;
#endif
	AddSolidFlags( FSOLID_NOT_STANDABLE );

	m_combineSpawned	= false;
	m_punted			= false;
#ifdef DARKINTERVAL
	m_grenadeType =		FRAGGRENADE_TYPE_DEFAULT;
#endif
	BaseClass::Spawn();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeFrag::OnRestore( void )
{
	// If we were primed and ready to detonate, put FX on us.
	if (m_flDetonateTime > 0)
		CreateEffects();

	BaseClass::OnRestore();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CGrenadeFrag::CreateEffects( void )
{
	// Start up the eye glow
	m_pMainGlow = CSprite::SpriteCreate( "sprites/redglow1.vmt", GetLocalOrigin(), false );

	int	nAttachment = LookupAttachment( "fuse" );

	if ( m_pMainGlow != NULL )
	{
		m_pMainGlow->FollowEntity( this );
		m_pMainGlow->SetAttachment( this, nAttachment );
		m_pMainGlow->SetTransparency( kRenderGlow, 255, 255, 255, 200, kRenderFxNoDissipation );
		m_pMainGlow->SetScale( 0.2f );
		m_pMainGlow->SetGlowProxySize( 4.0f );
	}

	// Start up the eye trail
	m_pGlowTrail	= CSpriteTrail::SpriteTrailCreate( "sprites/bluelaser1.vmt", GetLocalOrigin(), false );

	if ( m_pGlowTrail != NULL )
	{
		m_pGlowTrail->FollowEntity( this );
		m_pGlowTrail->SetAttachment( this, nAttachment );
		m_pGlowTrail->SetTransparency( kRenderTransAdd, 255, 0, 0, 255, kRenderFxNone );
		m_pGlowTrail->SetStartWidth( 8.0f );
		m_pGlowTrail->SetEndWidth( 1.0f );
		m_pGlowTrail->SetLifeTime( 0.5f );
	}
}

bool CGrenadeFrag::CreateVPhysics()
{
	// Create the object in the physics system
	VPhysicsInitNormal( SOLID_BBOX, 0, false );
	return true;
}

// this will hit only things that are in newCollisionGroup, but NOT in collisionGroupAlreadyChecked
class CTraceFilterCollisionGroupDelta : public CTraceFilterEntitiesOnly
{
public:
	// It does have a base, but we'll never network anything below here..
	DECLARE_CLASS_NOBASE( CTraceFilterCollisionGroupDelta );
	
	CTraceFilterCollisionGroupDelta( const IHandleEntity *passentity, int collisionGroupAlreadyChecked, int newCollisionGroup )
		: m_pPassEnt(passentity), m_collisionGroupAlreadyChecked( collisionGroupAlreadyChecked ), m_newCollisionGroup( newCollisionGroup )
	{
	}
	
	virtual bool ShouldHitEntity( IHandleEntity *pHandleEntity, int contentsMask )
	{
		if ( !PassServerEntityFilter( pHandleEntity, m_pPassEnt ) )
			return false;
		CBaseEntity *pEntity = EntityFromEntityHandle( pHandleEntity );

		if ( pEntity )
		{
			if ( g_pGameRules->ShouldCollide( m_collisionGroupAlreadyChecked, pEntity->GetCollisionGroup() ) )
				return false;
			if ( g_pGameRules->ShouldCollide( m_newCollisionGroup, pEntity->GetCollisionGroup() ) )
				return true;
		}

		return false;
	}

protected:
	const IHandleEntity *m_pPassEnt;
	int		m_collisionGroupAlreadyChecked;
	int		m_newCollisionGroup;
};

void CGrenadeFrag::VPhysicsUpdate( IPhysicsObject *pPhysics )
{
	BaseClass::VPhysicsUpdate( pPhysics );
	Vector vel;
	AngularImpulse angVel;
	pPhysics->GetVelocity( &vel, &angVel );
	
	Vector start = GetAbsOrigin();
	// find all entities that my collision group wouldn't hit, but COLLISION_GROUP_NONE would and bounce off of them as a ray cast
	CTraceFilterCollisionGroupDelta filter( this, GetCollisionGroup(), COLLISION_GROUP_NONE );
	trace_t tr;

	// UNDONE: Hull won't work with hitboxes - hits outer hull.  But the whole point of this test is to hit hitboxes.
#if 0
	UTIL_TraceHull( start, start + vel * gpGlobals->frametime, CollisionProp()->OBBMins(), CollisionProp()->OBBMaxs(), CONTENTS_HITBOX|CONTENTS_MONSTER|CONTENTS_SOLID, &filter, &tr );
#else
	UTIL_TraceLine( start, start + vel * gpGlobals->frametime, CONTENTS_HITBOX|CONTENTS_MONSTER|CONTENTS_SOLID, &filter, &tr );
#endif
	if ( tr.startsolid )
	{
		if ( !m_inSolid )
		{
			// UNDONE: Do a better contact solution that uses relative velocity?
			vel *= -GRENADE_COEFFICIENT_OF_RESTITUTION; // bounce backwards
			pPhysics->SetVelocity( &vel, NULL );
		}
		m_inSolid = true;
		return;
	}
	m_inSolid = false;
	if ( tr.DidHit() )
	{
		Vector dir = vel;
		VectorNormalize(dir);
		// send a tiny amount of damage so the character will react to getting bonked
		CTakeDamageInfo info( this, GetThrower(), pPhysics->GetMass() * vel, GetAbsOrigin(), 0.1f, DMG_CRUSH );
		tr.m_pEnt->TakeDamage( info );

		// reflect velocity around normal
		vel = -2.0f * tr.plane.normal * DotProduct(vel,tr.plane.normal) + vel;
		
		// absorb 80% in impact
		vel *= GRENADE_COEFFICIENT_OF_RESTITUTION;
		angVel *= -0.5f;
		pPhysics->SetVelocity( &vel, &angVel );
	}
}

void CGrenadeFrag::SetTimer( float detonateDelay, float warnDelay )
{
	m_flDetonateTime = CURTIME + detonateDelay;
#ifdef DARKINTERVAL
	m_flWarnAITime = CURTIME + (detonateDelay / 6);
#else
	m_flWarnAITime = CURTIME + warnDelay;
#endif
	SetThink( &CGrenadeFrag::DelayThink );
	SetNextThink( CURTIME );

	CreateEffects();
}

void CGrenadeFrag::OnPhysGunPickup( CBasePlayer *pPhysGunUser, PhysGunPickup_t reason )
{
	SetThrower( pPhysGunUser );	
	SetPunted( true );

	BaseClass::OnPhysGunPickup( pPhysGunUser, reason );
}

void CGrenadeFrag::DelayThink() 
{
	if( CURTIME > m_flDetonateTime )
	{
		Detonate();
		return;
	}
#ifdef DARKINTERVAL
	if (m_grenadeType == FRAGGRENADE_TYPE_DEFAULT)
#endif
	{
		if (!m_bHasWarnedAI && CURTIME >= m_flWarnAITime)
		{
#if !defined( CLIENT_DLL )
			CSoundEnt::InsertSound(SOUND_DANGER, GetAbsOrigin(), 400, 2.5, this);
#endif
			m_bHasWarnedAI = true;
		}
	}
	
	if( CURTIME > m_flNextBlipTime )
	{
		BlipSound();
#ifdef DARKINTERVAL
		if( CURTIME >= m_flDetonateTime - 1.5f) // for cooking
#else
		if ( m_bHasWarnedAI )
#endif
		{
			m_flNextBlipTime = CURTIME + FRAG_GRENADE_BLIP_FAST_FREQUENCY;
		}
		else
		{
			m_flNextBlipTime = CURTIME + FRAG_GRENADE_BLIP_FREQUENCY;
		}
	}

	SetNextThink( CURTIME + 0.1 );
}

void CGrenadeFrag::SetVelocity( const Vector &velocity, const AngularImpulse &angVelocity )
{
	IPhysicsObject *pPhysicsObject = VPhysicsGetObject();
	if ( pPhysicsObject )
	{
		pPhysicsObject->AddVelocity( &velocity, &angVelocity );
	}
}

int CGrenadeFrag::OnTakeDamage( const CTakeDamageInfo &inputInfo )
{
	// Manually apply vphysics because BaseCombatCharacter takedamage doesn't call back to CBaseEntity OnTakeDamage
	VPhysicsTakeDamage( inputInfo );

	// Grenades only suffer blast damage and burn damage.
	if( !(inputInfo.GetDamageType() & (DMG_BLAST|DMG_BURN) ) )
		return 0;

	return BaseClass::OnTakeDamage( inputInfo );
}

#if defined(HL2_EPISODIC) && 0 // FIXME: HandleInteraction() is no longer called now that base grenade derives from CBaseAnimating
extern int	g_interactionBarnacleVictimGrab; ///< usually declared in ai_interactions.h but no reason to haul all of that in here.
extern int g_interactionBarnacleVictimBite;
extern int g_interactionBarnacleVictimReleased;
bool CGrenadeFrag::HandleInteraction(int interactionType, void *data, CBaseCombatCharacter* sourceEnt)
{
	// allow fragnades to be grabbed by barnacles. 
	if ( interactionType == g_interactionBarnacleVictimGrab )
	{
		// give the grenade another five seconds seconds so the player can have the satisfaction of blowing up the barnacle with it
		float timer = m_flDetonateTime - CURTIME + 5.0f;
		SetTimer( timer, timer - FRAG_GRENADE_WARN_TIME );

		return true;
	}
	else if ( interactionType == g_interactionBarnacleVictimBite )
	{
		// detonate the grenade immediately 
		SetTimer( 0, 0 );
		return true;
	}
	else if ( interactionType == g_interactionBarnacleVictimReleased )
	{
		// take the five seconds back off the timer.
		float timer = MAX(m_flDetonateTime - CURTIME - 5.0f,0.0f);
		SetTimer( timer, timer - FRAG_GRENADE_WARN_TIME );
		return true;
	}
	else
	{
		return BaseClass::HandleInteraction( interactionType, data, sourceEnt );
	}
}
#endif

void CGrenadeFrag::InputSetTimer( inputdata_t &inputdata )
{
	SetTimer( inputdata.value.Float(), inputdata.value.Float() - FRAG_GRENADE_WARN_TIME );
}
#ifdef DARKINTERVAL
CBaseGrenade *Fraggrenade_Create( const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, bool combineSpawned, int grenadeType, float nextBlipTime )
#else
CBaseGrenade *Fraggrenade_Create(const Vector &position, const QAngle &angles, const Vector &velocity, const AngularImpulse &angVelocity, CBaseEntity *pOwner, float timer, bool combineSpawned)
#endif
{
	// Don't set the owner here, or the player can't interact with grenades he's thrown
	CGrenadeFrag *pGrenade = (CGrenadeFrag *)CBaseEntity::Create( "npc_grenade_frag", position, angles, pOwner );
	
	pGrenade->SetTimer( timer, timer - FRAG_GRENADE_WARN_TIME );
	pGrenade->SetVelocity( velocity, angVelocity );
	pGrenade->SetThrower( ToBaseCombatCharacter( pOwner ) );
	pGrenade->m_takedamage = DAMAGE_EVENTS_ONLY;
	pGrenade->SetCombineSpawned( combineSpawned );
#ifdef DARKINTERVAL
	pGrenade->SetGrenadeType( grenadeType );
	pGrenade->SetNextBlipTime(nextBlipTime); // for cooking
#endif
	return pGrenade;
}
#ifdef DARKINTERVAL
ConVar sk_fraggrenade_shrapnel_amount("sk_fraggrenade_shrapnel_amount", "8");
ConVar sk_fraggrenade_shrapnel_speed("sk_fraggrenade_shrapnel_speed", "3000");
ConVar sk_fraggrenade_debug("sk_fraggrenade_debug", "0");
#define BREATHING_ROOM 64
void CGrenadeFrag::Detonate(void)
{
	m_OnDetonate.FireOutput(this, this);
	switch (m_grenadeType)
	{
	case FRAGGRENADE_TYPE_DEFAULT:
	{
		BaseClass::Detonate(); // this should also push out the shrapnel bits

		// ensure that we won't crash the game with this
		if (sk_fraggrenade_shrapnel_amount.GetInt() <= 0
			|| (engine->GetEntityCount() + sk_fraggrenade_shrapnel_amount.GetInt() + BREATHING_ROOM < MAX_EDICTS))
		{
			//	int ammoType = GetAmmoDef()->Index("Pistol");
			Vector shrapnelDir;
			Vector2D shrapnel2DDir;
			Vector shrapnelSrc = GetAbsOrigin() + Vector(0, 0, 2);
			for (int g = 0; g < sk_fraggrenade_shrapnel_amount.GetInt(); g++)
			{
				RandomVectorInUnitCircle(&shrapnel2DDir);
				shrapnelDir.x = shrapnel2DDir.x;
				shrapnelDir.y = shrapnel2DDir.y;
				shrapnelDir.z = random->RandomFloat(-0.15, 0.15);

				// Spawn actual physical shrapnel
				CPhysicsProp *shrapnel = assert_cast<CPhysicsProp*>(CreateEntityByName("prop_physics"));
				shrapnel->SetModel("models/gibs/grenade_shrapnel.mdl");
				shrapnel->SetBodygroup(0, RandomInt(0, 2));
				shrapnel->SetAbsOrigin(GetAbsOrigin());
				shrapnel->SetAbsAngles(RandomAngle(0, 360));
				shrapnel->SetMoveType(MOVETYPE_VPHYSICS);
				shrapnel->SetCollisionGroup(COLLISION_GROUP_PROJECTILE);
				shrapnel->AddSpawnFlags(SF_PHYSPROP_PREVENT_PICKUP);
				shrapnel->AddEffects(EF_NOSHADOW);
				shrapnel->Spawn();
				shrapnel->ApplyAbsVelocityImpulse(shrapnelDir * sk_fraggrenade_shrapnel_speed.GetFloat());
				shrapnel->ThinkSet(&CBaseEntity::SUB_Remove);
				shrapnel->SetNextThink(CURTIME + RandomFloat(12.5, 13.5));
#if 0
				UTIL_TraceLine(shrapnelSrc, shrapnelSrc + shrapnelDir * 64, MASK_SOLID_BRUSHONLY, NULL, COLLISION_GROUP_NONE, &tr);

				if (tr.fraction == 1.0f)
				{
					if (sk_fraggrenade_debug.GetBool())
						NDebugOverlay::Line(shrapnelSrc, shrapnelSrc + shrapnelDir * 64, 25, 255, 150, true, 2.5f);

					UTIL_TraceHull(shrapnelSrc, shrapnelSrc + shrapnelDir * sk_fraggrenade_shrapnel_dist.GetFloat(), Vector(-2, -2, -2), Vector(2, 2, 2), MASK_SOLID, NULL, COLLISION_GROUP_NONE, &tr);

					if (tr.fraction < 1.0f)
					{
						// Make sure we don't have a dangling damage target from a recursive call
						if (g_MultiDamage.GetTarget() != NULL)
						{
							ApplyMultiDamage();
						}
						ClearMultiDamage();
						g_MultiDamage.SetDamageType(DMG_SLASH | DMG_NEVERGIB);
						// Damage specified by function parameter
						CTakeDamageInfo info(GetWorldEntity(), GetWorldEntity(), shrapnelDir * 100, shrapnelSrc, 25, DMG_SLASH);
						tr.m_pEnt->TakeDamage(info);

						if (sk_fraggrenade_debug.GetBool())
						{
							NDebugOverlay::SweptBox(shrapnelSrc, shrapnelSrc + shrapnelDir * sk_fraggrenade_shrapnel_dist.GetFloat() * tr.fraction, Vector(-2, -2, -2), Vector(2, 2, 2), vec3_angle, 255, 50, 0, 25, 2.5f);
							NDebugOverlay::Cross3D(tr.endpos, 4, 255, 0, 0, true, 2.5f);
						}
					}
					/*
					FireBulletsInfo_t info;
					info.m_vecSrc = shrapnelSrc;
					info.m_vecDirShooting = shrapnelDir;
					info.m_iTracerFreq = 1;
					info.m_iShots = sk_fraggrenade_shrapnel_amount.GetInt();
					info.m_pAttacker = this;
					info.m_vecSpread = Vector(0.707, 0.707, 0.707);
					info.m_flDistance = sk_fraggrenade_shrapnel_dist.GetFloat();
					info.m_iAmmoType = ammoType;

					FireBullets(info);
					*/
				}
				else
				{
					if (sk_fraggrenade_debug.GetBool())
						NDebugOverlay::Line(shrapnelSrc, shrapnelSrc + shrapnelDir * 64 * tr.fraction, 255, 50, 0, true, 2.5f);
			}
#endif
		}
	}
	}
	break;
	case FRAGGRENADE_TYPE_SMOKE:
	{
		CreateSmokeEffect();
		SetThink(&CGrenadeFrag::SUB_Remove);
		SetNextThink(CURTIME + 0.25f);
		SetTouch(NULL);
		SetSolid(SOLID_NONE);
		AddEffects(EF_NODRAW);
		SetAbsVelocity(vec3_origin);
		UTIL_Remove(m_pMainGlow);
		UTIL_Remove(m_pGlowTrail);
	}
	break;
	default:
	{
		BaseClass::Detonate();
	}
	break;
	}
}

void CGrenadeFrag::CreateSmokeEffect(void)
{
//	for (int i = 0; i < 2; i++)
	{
		Vector smoke_origin = GetAbsOrigin();
		smoke_origin.z += 8.0f;

		DispatchParticleEffect("smokegrenade_smoke", smoke_origin, vec3_angle);
		
		CSoundEnt::InsertSound(SOUND_HIDE_POSITION, GetAbsOrigin(), 1024, 15, this);

		/*
		ParticleSmokeGrenade *pGren = (ParticleSmokeGrenade*)CBaseEntity::Create(PARTICLESMOKEGRENADE_ENTITYNAME, GetAbsOrigin(), QAngle(0, 0, 0), NULL);
		if (pGren)
		{
			pGren->SetFadeTime(17, 22);
			pGren->SetAbsOrigin(smoke_origin);
			pGren->SetVolumeColour(Vector(1.5, 1.3, 0.7));
			pGren->SetVolumeSize(Vector(60, 60, 80));
			pGren->SetExpandTime(1.0f);
		//	pGren->FillVolume();
			//		pGren->SetVolumeEmissionRate(0.3);
					//pGren->SetVolumeColour(Vector(1.5, 1.3, 0.7));
			//		pGren->SetVolumeSize(Vector(100, 80, 80));
			//		pGren->SetExpandTime(0.5f);
		}
		*/
	}
	
	EmitSound("NPC_Metropolice.GrenadeDetonate");
}
#endif
bool Fraggrenade_WasPunted( const CBaseEntity *pEntity )
{
	const CGrenadeFrag *pFrag = dynamic_cast<const CGrenadeFrag *>( pEntity );
	if ( pFrag )
	{
		return pFrag->WasPunted();
	}

	return false;
}

bool Fraggrenade_WasCreatedByCombine( const CBaseEntity *pEntity )
{
	const CGrenadeFrag *pFrag = dynamic_cast<const CGrenadeFrag *>( pEntity );
	if ( pFrag )
	{
		return pFrag->IsCombineSpawned();
	}

	return false;
}

BEGIN_DATADESC(CGrenadeFrag)

// Fields
DEFINE_FIELD(m_pMainGlow, FIELD_EHANDLE),
DEFINE_FIELD(m_pGlowTrail, FIELD_EHANDLE),
DEFINE_FIELD(m_flNextBlipTime, FIELD_TIME),
DEFINE_FIELD(m_inSolid, FIELD_BOOLEAN),
DEFINE_FIELD(m_combineSpawned, FIELD_BOOLEAN),
#ifdef DARKINTERVAL
DEFINE_FIELD(m_grenadeType, FIELD_INTEGER),
#endif
DEFINE_FIELD(m_punted, FIELD_BOOLEAN),

// Function Pointers
DEFINE_THINKFUNC(DelayThink),

// Inputs
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetTimer", InputSetTimer),
#ifdef DARKINTERVAL
// Outputs
DEFINE_OUTPUT(m_OnDetonate, "OnDetonate"),
#endif
END_DATADESC()