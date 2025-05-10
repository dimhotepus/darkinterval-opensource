//========================================================================//
//
// Purpose: Flame entity to be attached to target entity. Serves two purposes:
//
//			1) An entity that can be placed by a level designer and triggered
//			   to ignite a target entity.
//
//			2) An entity that can be created at runtime to ignite a target entity.
//
//=============================================================================//

#include "cbase.h"
#include "entityflame.h"
#include "ai_basenpc.h"
#include "fire.h"
#include "shareddefs.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

BEGIN_DATADESC( CEntityFlame )

	DEFINE_KEYFIELD( m_flLifetime, FIELD_FLOAT, "lifetime" ),

	DEFINE_FIELD( m_flSize, FIELD_FLOAT ),
	DEFINE_FIELD( m_hEntAttached, FIELD_EHANDLE ),
#ifdef DARKINTERVAL
	DEFINE_FIELD( m_iDangerSound, FIELD_INTEGER),
#endif
	DEFINE_FIELD( m_bUseHitboxes, FIELD_BOOLEAN ),
#if !defined CLASSIC_FIRE
	DEFINE_FIELD( m_iNumHitboxFires, FIELD_INTEGER ),
	DEFINE_FIELD( m_flHitboxFireScale, FIELD_FLOAT ),
#endif
//	DEFINE_FIELD( m_bPlayingSound, FIELD_BOOLEAN ),
	
	DEFINE_FUNCTION( FlameThink ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Ignite", InputIgnite ),

END_DATADESC()


IMPLEMENT_SERVERCLASS_ST( CEntityFlame, DT_EntityFlame )
#if defined(CLASSIC_FIRE)
	SendPropFloat(SENDINFO(m_flSize), 16, SPROP_NOSCALE),
#endif
	SendPropEHandle(SENDINFO(m_hEntAttached)),
#if defined(CLASSIC_FIRE)
	SendPropInt(SENDINFO(m_bUseHitboxes), 1, SPROP_UNSIGNED),
	SendPropTime(SENDINFO(m_flLifetime))
#endif
END_SEND_TABLE()

LINK_ENTITY_TO_CLASS( entityflame, CEntityFlame );
LINK_ENTITY_TO_CLASS( env_entity_igniter, CEntityFlame );
PRECACHE_REGISTER(entityflame);

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CEntityFlame::CEntityFlame( void )
{
	m_flSize			= 0.0f;
#if !defined (CLASSIC_FIRE)
	m_iNumHitboxFires	= 10;
	m_flHitboxFireScale	= 1.0f;
#endif
	m_flLifetime		= 0.0f;
	m_bPlayingSound		= false;
#ifdef DARKINTERVAL
	m_iDangerSound = SOUNDLIST_EMPTY;
#endif
}

void CEntityFlame::UpdateOnRemove()
{
	// Sometimes the entity I'm burning gets destroyed by other means,
	// which kills me. Make sure to stop the burning sound.
	if ( m_bPlayingSound )
	{
		EmitSound( "General.StopBurning" );
		m_bPlayingSound = false;
	}

	BaseClass::UpdateOnRemove();
}

void CEntityFlame::Precache()
{
	BaseClass::Precache();

	PrecacheScriptSound( "General.StopBurning" );
	PrecacheScriptSound( "General.BurningFlesh" );
	PrecacheScriptSound( "General.BurningObject" );
}
#ifdef DARKINTERVAL
void CEntityFlame::Spawn()
{
	BaseClass::Spawn();
	m_flLifetime = CURTIME;

	SetThink(&CEntityFlame::FlameThink);
	SetNextThink(CURTIME + 0.1f);
	//Send to the client even though we don't have a model
	AddEFlags(EFL_FORCE_CHECK_TRANSMIT);

#if 1 //HL2_EP3
	CSoundEnt::InsertSound(SOUND_DANGER | SOUND_CONTEXT_FROM_FIRE | SOUND_CONTEXT_FOLLOW_OWNER,
		GetAbsOrigin(), m_flSize * 2.0f, FLT_MAX, this);
#endif
}

//-----------------------------------------------------------------------------
// Since we don't save/load danger links, we need to reacquire them here
//-----------------------------------------------------------------------------
void CEntityFlame::Activate()
{
	BaseClass::Activate();

#ifdef HL2_EP3 // todo - make it work
#if 0
	// Mark nearby links as dangerous
	const Vector &vecOrigin = GetAbsOrigin();
	float flMaxDistSqr = m_flSize * m_flSize;
	m_hObstacle = CAI_LocalNavigator::AddGlobalObstacle(vecOrigin, m_flSize, AIMST_AVOID_DANGER);
	for (int i = 0; i < g_pBigAINet->NumNodes(); i++)
	{
		CAI_Node *pSrcNode = g_pBigAINet->GetNode(i);
		int nSrcNodeId = pSrcNode->GetId();
		for (int j = 0; j < pSrcNode->NumLinks(); j++)
		{
			CAI_Link *pLink = pSrcNode->GetLinkByIndex(j);
			int nDstNodeId = pLink->DestNodeID(nSrcNodeId);

			// Eliminates double-checking of links
			if (nDstNodeId < nSrcNodeId)
				continue;

			CAI_Node *pDstNode = g_pBigAINet->GetNode(nDstNodeId);
			float flDistSqr = CalcDistanceSqrToLineSegment(vecOrigin, pSrcNode->GetOrigin(), pDstNode->GetOrigin());
			if (flDistSqr > flMaxDistSqr)
				continue;

			++pLink->m_nDangerCount;
			m_DangerLinks.AddToTail(pLink);
		}
	}
#endif
#endif // HL2_EP3
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : inputdata - 
//-----------------------------------------------------------------------------
void CEntityFlame::InputIgnite( inputdata_t &inputdata )
{
#ifndef DARKINTERVAL
	if (m_target != NULL_STRING)
	{
		CBaseEntity *pTarget = NULL;
		while ((pTarget = gEntList.FindEntityGeneric(pTarget, STRING(m_target), this, inputdata.pActivator)) != NULL)
		{
			// Combat characters know how to catch themselves on fire.
			CBaseCombatCharacter *pBCC = pTarget->MyCombatCharacterPointer();
			if (pBCC)
			{
				// DVS TODO: consider promoting Ignite to CBaseEntity and doing everything here
				pBCC->Ignite(m_flLifetime);
			}
			// Everything else, we handle here.
			else
			{
				CEntityFlame *pFlame = CEntityFlame::Create(pTarget);
				if (pFlame)
				{
					pFlame->SetLifetime(m_flLifetime);
				}
			}
		}
	}
#endif
}


//-----------------------------------------------------------------------------
// Purpose: Creates a flame and attaches it to a target entity.
// Input  : pTarget - 
//-----------------------------------------------------------------------------
CEntityFlame *CEntityFlame::Create( CBaseEntity *pTarget, bool useHitboxes )
{
	CEntityFlame *pFlame = (CEntityFlame *) CreateEntityByName( "entityflame" );

	if ( pFlame == NULL )
		return NULL;

	float xSize = pTarget->CollisionProp()->OBBMaxs().x - pTarget->CollisionProp()->OBBMins().x;
	float ySize = pTarget->CollisionProp()->OBBMaxs().y - pTarget->CollisionProp()->OBBMins().y;

	float size = ( xSize + ySize ) * 0.5f;
	
	if ( size < 16.0f )
	{
		size = 16.0f;
	}

	UTIL_SetOrigin( pFlame, pTarget->GetAbsOrigin() );

	pFlame->m_flSize = size;
	pFlame->SetThink( &CEntityFlame::FlameThink );
	pFlame->SetNextThink( CURTIME + 0.1f );

	pFlame->AttachToEntity( pTarget );
	pFlame->SetLifetime( 2.0f );

	//Send to the client even though we don't have a model
	pFlame->AddEFlags( EFL_FORCE_CHECK_TRANSMIT );

	pFlame->SetUseHitboxes( useHitboxes );

	return pFlame;
}


//-----------------------------------------------------------------------------
// Purpose: Attaches the flame to an entity and moves with it
// Input  : pTarget - target entity to attach to
//-----------------------------------------------------------------------------
void CEntityFlame::AttachToEntity( CBaseEntity *pTarget )
{
	// For networking to the client.
	m_hEntAttached = pTarget;

	if( pTarget->IsNPC() )
	{
		EmitSound( "General.BurningFlesh" );
	}
	else
	{
		EmitSound( "General.BurningObject" );
	}

	m_bPlayingSound = true;

	// So our heat emitter follows the entity around on the server.
	SetParent( pTarget );
}


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : lifetime - 
//-----------------------------------------------------------------------------
void CEntityFlame::SetLifetime( float lifetime )
{
	m_flLifetime = CURTIME + lifetime;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : use - 
//-----------------------------------------------------------------------------
void CEntityFlame::SetUseHitboxes( bool use )
{
	m_bUseHitboxes = use;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : iNumHitBoxFires - 
//-----------------------------------------------------------------------------
#if !defined(CLASSIC_FIRE)
void CEntityFlame::SetNumHitboxFires( int iNumHitboxFires )
{
	m_iNumHitboxFires = iNumHitboxFires;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flHitboxFireScale - 
//-----------------------------------------------------------------------------
void CEntityFlame::SetHitboxFireScale( float flHitboxFireScale )
{
	m_flHitboxFireScale = flHitboxFireScale;
}

float CEntityFlame::GetRemainingLife( void )
{
	return m_flLifetime - CURTIME;
}

int CEntityFlame::GetNumHitboxFires( void )
{
	return m_iNumHitboxFires;
}

float CEntityFlame::GetHitboxFireScale( void )
{
	return m_flHitboxFireScale;
}
#endif // CLASSIC_FIRE
//-----------------------------------------------------------------------------
// Purpose: Burn targets around us
//-----------------------------------------------------------------------------
void CEntityFlame::FlameThink( void )
{
	// Assure that this function will be ticked again even if we early-out in the if below.
	SetNextThink( CURTIME + FLAME_DAMAGE_INTERVAL );

	if ( m_hEntAttached )
	{
		if ( m_hEntAttached->GetFlags() & FL_TRANSRAGDOLL )
		{
			SetRenderColorA( 0 );
			return;
		}
	
		CAI_BaseNPC *pNPC = m_hEntAttached->MyNPCPointer();
		if ( pNPC && !pNPC->IsAlive() )
		{
			UTIL_Remove( this );
			// Notify the NPC that it's no longer burning!
			pNPC->Extinguish();
			return;
		}

		if( m_hEntAttached->GetWaterLevel() > 0 )
		{
			Vector mins, maxs;

			mins = m_hEntAttached->WorldSpaceCenter();
			maxs = mins;

			maxs.z = m_hEntAttached->WorldSpaceCenter().z;
			maxs.x += 32;
			maxs.y += 32;
			
			mins.z -= 32;
			mins.x -= 32;
			mins.y -= 32;

			UTIL_Bubbles( mins, maxs, 12 );
		}
	}
	else
	{
		UTIL_Remove( this );
		return;
	}

	// See if we're done burning, or our attached ent has vanished
	if ( m_flLifetime < CURTIME || m_hEntAttached == NULL )
	{
		EmitSound( "General.StopBurning" );
		m_bPlayingSound = false;
		SetThink( &CEntityFlame::SUB_Remove );
		SetNextThink( CURTIME + 0.5f );

		// Notify anything we're attached to
		if ( m_hEntAttached )
		{
			CBaseCombatCharacter *pAttachedCC = m_hEntAttached->MyCombatCharacterPointer();

			if( pAttachedCC )
			{
				// Notify the NPC that it's no longer burning!
				pAttachedCC->Extinguish();
			}
		}

		return;
	}

	if ( m_hEntAttached )
	{
#ifdef DARKINTERVAL
		// Prevent the flames from growing too big
		float size = m_flSize;
		if (size >= 16) m_flSize = 16;
#endif
		// Do radius damage ignoring the entity I'm attached to. This will harm things around me.
#ifdef DARKINTERVAL
		RadiusDamage( CTakeDamageInfo( this, this, 4.0f, DMG_BURN ), GetAbsOrigin(), m_flSize, CLASS_NONE, m_hEntAttached );
#else
		RadiusDamage(CTakeDamageInfo(this, this, 4.0f, DMG_BURN), GetAbsOrigin(), m_flSize/2, CLASS_NONE, m_hEntAttached);
#endif
		// Directly harm the entity I'm attached to. This is so we can precisely control how much damage the entity
		// that is on fire takes without worrying about the flame's position relative to the bodytarget (which is the
		// distance that the radius damage code uses to determine how much damage to inflict)
		m_hEntAttached->TakeDamage( CTakeDamageInfo( this, this, FLAME_DIRECT_DAMAGE, DMG_BURN | DMG_DIRECT ) );

		if( !m_hEntAttached->IsNPC() )
		{
			const float ENTITYFLAME_MOVE_AWAY_DIST = 24.0f;
			// Make a sound near my origin, and up a little higher (in case I'm on the ground, so NPC's still hear it)
			CSoundEnt::InsertSound( SOUND_MOVE_AWAY, GetAbsOrigin(), ENTITYFLAME_MOVE_AWAY_DIST, 0.1f, this, SOUNDENT_CHANNEL_REPEATED_DANGER );
			CSoundEnt::InsertSound( SOUND_MOVE_AWAY, GetAbsOrigin() + Vector( 0, 0, 48.0f ), ENTITYFLAME_MOVE_AWAY_DIST, 0.1f, this, SOUNDENT_CHANNEL_REPEATING );
		}
	}
	else
	{
		RadiusDamage( CTakeDamageInfo( this, this, FLAME_RADIUS_DAMAGE, DMG_BURN ), GetAbsOrigin(), m_flSize/2, CLASS_NONE, NULL );
	}

	FireSystem_AddHeatInRadius( GetAbsOrigin(), m_flSize/2, 0.2f );

}  


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : pEnt -	
//-----------------------------------------------------------------------------
void CreateEntityFlame(CBaseEntity *pEnt)
{
	CEntityFlame::Create( pEnt );
}
