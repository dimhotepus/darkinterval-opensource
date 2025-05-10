//========================================================================//
//
// Purpose: Precaches and defs for entities and other data that must always be available.
// DI changes that are not ifdef'd: 
// - removed all CS/TF/MP stuff
// $NoKeywords: $
//===========================================================================//

#include "cbase.h"
#include "activitylist.h"
#include "ai_network.h"
#include "ai_networkmanager.h"
#include "ai_schedule.h"
#include "ai_utils.h"
#include "basetempentity.h"
#include "client.h"
#include "decals.h"
#include "engine/ienginesound.h"
#include "engine/IStaticPropMgr.h"
#include "env_message.h"
#include "eventlist.h"
#include "eventqueue.h"
#include "gamerules.h"
#include "globals.h"
#include "globalstate.h"
#include "igamesystem.h"
#include "isaverestore.h"
#include "mempool.h"
#include "particle_parse.h"
#include "physics.h"
#include "player.h"
#include "soundent.h"
#ifndef DARKINTERVAL // DI is singleplayer, so it ditched teams
#include "teamplay_gamerules.h"
#endif
#include "world.h"
#ifdef DARKINTERVAL
#include "darkinterval\npc_parse.h"
//#include "filesystem.h"
//#include "steam\steam_api.h"
#endif
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern CBaseEntity				*g_pLastSpawn;
void InitBodyQue(void);
extern void W_Precache(void);
extern void ActivityList_Free( void );
extern CUtlMemoryPool g_EntityListPool;

#define SF_DECAL_NOTINDEATHMATCH		2048

class CDecal : public CPointEntity
{
public:
	DECLARE_CLASS( CDecal, CPointEntity );

	void	Spawn( void );
	bool	KeyValue( const char *szKeyName, const char *szValue );

	// Need to apply static decals here to get them into the signon buffer for the server appropriately
	virtual void Activate();

	void	TriggerDecal( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	// Input handlers.
	void	InputActivate( inputdata_t &inputdata );

	DECLARE_DATADESC();

public:
	int		m_nTexture;
	bool	m_bLowPriority;

private:

	void	StaticDecal( void );
};

BEGIN_DATADESC( CDecal )

	DEFINE_FIELD( m_nTexture, FIELD_INTEGER ),
	DEFINE_KEYFIELD( m_bLowPriority, FIELD_BOOLEAN, "LowPriority" ), // Don't mark as FDECAL_PERMANENT so not save/restored and will be reused on the client preferentially

	// Function pointers
	DEFINE_FUNCTION( StaticDecal ),
	DEFINE_FUNCTION( TriggerDecal ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Activate", InputActivate ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( infodecal, CDecal );

void CDecal::Spawn( void )
{
}

void CDecal::Activate()
{
	BaseClass::Activate();

	if ( !GetEntityName() )
	{
		StaticDecal();
	}
	else
	{
		// if there IS a targetname, the decal sprays itself on when it is triggered.
		SetThink ( &CDecal::SUB_DoNothing );
		SetUse(&CDecal::TriggerDecal);
	}
}

void CDecal::TriggerDecal ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	// this is set up as a USE function for info_decals that have targetnames, so that the
	// decal doesn't get applied until it is fired. (usually by a scripted sequence)
	trace_t		trace;
	int			entityIndex;

	UTIL_TraceLine( GetAbsOrigin() - Vector(5,5,5), GetAbsOrigin() + Vector(5,5,5), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trace );

	entityIndex = trace.m_pEnt ? trace.m_pEnt->entindex() : 0;

	CBroadcastRecipientFilter filter;

	te->BSPDecal( filter, 0.0, 
		&GetAbsOrigin(), entityIndex, m_nTexture );

	SetThink( &CDecal::SUB_Remove );
	SetNextThink( CURTIME + 0.1f );
}


void CDecal::InputActivate( inputdata_t &inputdata )
{
	TriggerDecal( inputdata.pActivator, inputdata.pCaller, USE_ON, 0 );
}


void CDecal::StaticDecal( void )
{
	class CTraceFilterValidForDecal : public CTraceFilterSimple
	{
	public:
		CTraceFilterValidForDecal(const IHandleEntity *passentity, int collisionGroup )
		 :	CTraceFilterSimple( passentity, collisionGroup )
		{
		}

		virtual bool ShouldHitEntity( IHandleEntity *pServerEntity, int contentsMask )
		{
			static const char *ppszIgnoredClasses[] = 
			{
				"weapon_*",
				"item_*",
				"prop_ragdoll",
#ifndef DARKINTERVAL
				"prop_dynamic",
				"prop_static",
#endif
				"prop_physics",
				"npc_bullseye",  // Tracker 15335
			};

			CBaseEntity *pEntity = EntityFromEntityHandle( pServerEntity );

			// Tracker 15335:  Never impact decals against entities which are not rendering, either.
			if ( pEntity->IsEffectActive( EF_NODRAW ) )
				return false;

			for ( int i = 0; i < ARRAYSIZE(ppszIgnoredClasses); i++ )
			{
				if ( pEntity->ClassMatches( ppszIgnoredClasses[i] ) )
					return false;
			}


			return CTraceFilterSimple::ShouldHitEntity( pServerEntity, contentsMask );
		}
	};

	trace_t trace;
	CTraceFilterValidForDecal traceFilter( this, COLLISION_GROUP_NONE );
	int entityIndex, modelIndex = 0;

	Vector position = GetAbsOrigin();
	UTIL_TraceLine( position - Vector(5,5,5), position + Vector(5,5,5),  MASK_SOLID, &traceFilter, &trace );

	bool canDraw = true;

	entityIndex = trace.m_pEnt ? (short)trace.m_pEnt->entindex() : 0;
	if ( entityIndex )
	{
		CBaseEntity *ent = trace.m_pEnt;
		if ( ent )
		{
			modelIndex = ent->GetModelIndex();
			VectorITransform( GetAbsOrigin(), ent->EntityToWorldTransform(), position );

			canDraw = ( modelIndex != 0 );
			if ( !canDraw )
			{
				Warning( "Suppressed StaticDecal which would have hit entity %i (class:%s, name:%s) with modelindex = 0\n",
					ent->entindex(),
					ent->GetClassname(),
					STRING( ent->GetEntityName() ) );
			}
		}
	}

	if ( canDraw )
	{
		engine->StaticDecal( position, m_nTexture, entityIndex, modelIndex, m_bLowPriority );
	}

	SUB_Remove();
}


bool CDecal::KeyValue( const char *szKeyName, const char *szValue )
{
	if (FStrEq(szKeyName, "texture"))
	{
		// FIXME:  should decals all be preloaded?
		m_nTexture = UTIL_PrecacheDecal( szValue, true );
		
		// Found
		if (m_nTexture >= 0 )
			return true;
		Warning( "Can't find decal %s\n", szValue );
	}
	else
	{
		return BaseClass::KeyValue( szKeyName, szValue );
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Projects a decal against a prop
//-----------------------------------------------------------------------------
class CProjectedDecal : public CPointEntity
{
public:
	DECLARE_CLASS( CProjectedDecal, CPointEntity );

	void	Spawn( void );
	bool	KeyValue( const char *szKeyName, const char *szValue );

	// Need to apply static decals here to get them into the signon buffer for the server appropriately
	virtual void Activate();

	void	TriggerDecal( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value );

	// Input handlers.
	void	InputActivate( inputdata_t &inputdata );

	DECLARE_DATADESC();

public:
	int		m_nTexture;
	float	m_flDistance;

private:
	void	ProjectDecal( CRecipientFilter& filter );

	void	StaticDecal( void );
};

BEGIN_DATADESC( CProjectedDecal )

	DEFINE_FIELD( m_nTexture, FIELD_INTEGER ),

	DEFINE_KEYFIELD( m_flDistance, FIELD_FLOAT, "Distance" ),

	// Function pointers
	DEFINE_FUNCTION( StaticDecal ),
	DEFINE_FUNCTION( TriggerDecal ),

	DEFINE_INPUTFUNC( FIELD_VOID, "Activate", InputActivate ),

END_DATADESC()

LINK_ENTITY_TO_CLASS( info_projecteddecal, CProjectedDecal );

// UNDONE:  These won't get sent to joining players in multi-player
void CProjectedDecal::Spawn( void )
{
}

void CProjectedDecal::Activate()
{
	BaseClass::Activate();

	if ( !GetEntityName() )
	{
		StaticDecal();
	}
	else
	{
		// if there IS a targetname, the decal sprays itself on when it is triggered.
		SetThink ( &CProjectedDecal::SUB_DoNothing );
		SetUse(&CProjectedDecal::TriggerDecal);
	}
}

void CProjectedDecal::InputActivate( inputdata_t &inputdata )
{
	TriggerDecal( inputdata.pActivator, inputdata.pCaller, USE_ON, 0 );
}

void CProjectedDecal::ProjectDecal( CRecipientFilter& filter )
{
	te->ProjectDecal( filter, 0.0, 
		&GetAbsOrigin(), &GetAbsAngles(), m_flDistance, m_nTexture );
}

void CProjectedDecal::TriggerDecal ( CBaseEntity *pActivator, CBaseEntity *pCaller, USE_TYPE useType, float value )
{
	CBroadcastRecipientFilter filter;

	ProjectDecal( filter );

	SetThink( &CProjectedDecal::SUB_Remove );
	SetNextThink( CURTIME + 0.1f );
}

void CProjectedDecal::StaticDecal( void )
{
	CBroadcastRecipientFilter initFilter;
	initFilter.MakeInitMessage();

	ProjectDecal( initFilter );

	SUB_Remove();
}


bool CProjectedDecal::KeyValue( const char *szKeyName, const char *szValue )
{
	if (FStrEq(szKeyName, "texture"))
	{
		// FIXME:  should decals all be preloaded?
		m_nTexture = UTIL_PrecacheDecal( szValue, true );
		
		// Found
		if (m_nTexture >= 0 )
			return true;
		Warning( "Can't find decal %s\n", szValue );
	}
	else
	{
		return BaseClass::KeyValue( szKeyName, szValue );
	}

	return true;
}

//=======================
// CWorld
//
// This spawns first when each level begins.
//=======================
LINK_ENTITY_TO_CLASS( worldspawn, CWorld );

BEGIN_DATADESC( CWorld )

	DEFINE_FIELD( m_flWaveHeight, FIELD_FLOAT ),

	// keyvalues are parsed from map, but not saved/loaded
	DEFINE_KEYFIELD( m_bStartDark,		FIELD_BOOLEAN, "startdark" ),
	DEFINE_KEYFIELD( m_bDisplayTitle,	FIELD_BOOLEAN, "gametitle" ),
	DEFINE_FIELD( m_WorldMins, FIELD_VECTOR ),
	DEFINE_FIELD( m_WorldMaxs, FIELD_VECTOR ),
	DEFINE_KEYFIELD( m_flMaxOccludeeArea, FIELD_FLOAT, "maxoccludeearea" ),
	DEFINE_KEYFIELD( m_flMinOccluderArea, FIELD_FLOAT, "minoccluderarea" ),
	DEFINE_KEYFIELD( m_flMaxPropScreenSpaceWidth, FIELD_FLOAT, "maxpropscreenwidth" ),
	DEFINE_KEYFIELD( m_flMinPropScreenSpaceWidth, FIELD_FLOAT, "minpropscreenwidth" ),
	DEFINE_KEYFIELD( m_iszDetailSpriteMaterial, FIELD_STRING, "detailmaterial" ),
	DEFINE_KEYFIELD( m_bColdWorld,		FIELD_BOOLEAN, "coldworld" ),
#ifdef DARKINTERVAL
	DEFINE_KEYFIELD(m_iszChapterTitle, FIELD_STRING, "chaptertitle"),
	DEFINE_KEYFIELD(cStartDarkColor, FIELD_VECTOR, "startdarkcolor"),
	DEFINE_KEYFIELD(m_flStartDarkTime, FIELD_FLOAT, "startdarktime"),
	DEFINE_KEYFIELD(m_flDetailDist, FIELD_FLOAT, "detaildist"),
	DEFINE_KEYFIELD( m_bRainExpensive,	FIELD_BOOLEAN, "rainexpensive" ),
	DEFINE_KEYFIELD( m_bForceCheapWater, FIELD_BOOLEAN, "forcecheapwater"),
	DEFINE_KEYFIELD( m_bForceCheapShadows, FIELD_BOOLEAN, "forcecheapshadows"),
	DEFINE_KEYFIELD(m_SnowDot, FIELD_FLOAT, "snowdot"), // control dynamic snow from world settings // todo - move to some env_weather_controller entity
	DEFINE_KEYFIELD(m_SnowDir, FIELD_VECTOR, "snowdir"),
#endif
END_DATADESC()

// SendTable stuff.
IMPLEMENT_SERVERCLASS_ST(CWorld, DT_WORLD)
	SendPropFloat	(SENDINFO(m_flWaveHeight), 8, SPROP_ROUNDUP,	0.0f,	8.0f),
	SendPropVector	(SENDINFO(m_WorldMins),	-1,	SPROP_COORD),
	SendPropVector	(SENDINFO(m_WorldMaxs),	-1,	SPROP_COORD),
	SendPropInt		(SENDINFO(m_bStartDark), 1, SPROP_UNSIGNED ),
	SendPropFloat	(SENDINFO(m_flMaxOccludeeArea), 0, SPROP_NOSCALE ),
	SendPropFloat	(SENDINFO(m_flMinOccluderArea), 0, SPROP_NOSCALE ),
	SendPropFloat	(SENDINFO(m_flMaxPropScreenSpaceWidth), 0, SPROP_NOSCALE ),
	SendPropFloat	(SENDINFO(m_flMinPropScreenSpaceWidth), 0, SPROP_NOSCALE ),
	SendPropStringT (SENDINFO(m_iszDetailSpriteMaterial) ),
	SendPropFloat	(SENDINFO(m_flDetailDist), 0, SPROP_NOSCALE),
	SendPropInt		(SENDINFO(m_bColdWorld), 1, SPROP_UNSIGNED ),
#ifdef DARKINTERVAL
	SendPropFloat	(SENDINFO(m_flStartDarkTime), 0, SPROP_NOSCALE),
	SendPropVector	(SENDINFO(cStartDarkColor), -1, SPROP_COORD),
	SendPropInt		(SENDINFO(m_bRainExpensive), 1, SPROP_UNSIGNED),
	SendPropInt		(SENDINFO(m_bForceCheapShadows), 1, SPROP_UNSIGNED),
	SendPropInt		(SENDINFO(m_bForceCheapWater), 1, SPROP_UNSIGNED),
	SendPropVector(SENDINFO(m_SnowDir), -1, SPROP_COORD),
	SendPropFloat(SENDINFO(m_SnowDot), 0, SPROP_NOSCALE),
#endif
END_SEND_TABLE()

//
// Just to ignore the "wad" field.
//
bool CWorld::KeyValue( const char *szKeyName, const char *szValue )
{
	if ( FStrEq(szKeyName, "skyname") )
	{
		// Sent over net now.
		ConVarRef skyname( "sv_skyname" );
		skyname.SetValue( szValue );
	}
	else if ( FStrEq(szKeyName, "newunit") )
	{
		// Single player only.  Clear save directory if set
		if ( atoi(szValue) )
		{
			extern void Game_SetOneWayTransition();
			Game_SetOneWayTransition();
		}
	}
	else if ( FStrEq(szKeyName, "world_mins") )
	{
		Vector vec;
		sscanf(	szValue, "%f %f %f", &vec.x, &vec.y, &vec.z );
		m_WorldMins = vec;
	}
	else if ( FStrEq(szKeyName, "world_maxs") )
	{
		Vector vec;
		sscanf(	szValue, "%f %f %f", &vec.x, &vec.y, &vec.z ); 
		m_WorldMaxs = vec;
	}
#ifdef DARKINTERVAL
	else if (FStrEq(szKeyName, "snowdir"))
	{
		Vector vec;
		sscanf(szValue, "%f %f %f", &vec.x, &vec.y, &vec.z);
		m_SnowDir = vec;
	}
	else if (FStrEq(szKeyName, "snowdot"))
	{
		if (atof(szValue))
		{
			m_SnowDot = atof(szValue);
		}
	}
#endif
	else
		return BaseClass::KeyValue( szKeyName, szValue );

	return true;
}


extern bool		g_fGameOver;
static CWorld *g_WorldEntity = NULL;

CWorld* GetWorldEntity()
{
	return g_WorldEntity;
}

CWorld::CWorld( )
{
	AddEFlags( EFL_NO_AUTO_EDICT_ATTACH | EFL_KEEP_ON_RECREATE_ENTITIES );
	NetworkProp()->AttachEdict( INDEXENT(RequiredEdictIndex()) );
	ActivityList_Init();
	EventList_Init();
	
	SetSolid( SOLID_BSP );
	SetMoveType( MOVETYPE_NONE );

	m_bColdWorld = false;
#ifdef DARKINTERVAL
	m_bRainExpensive = true;
	m_bForceCheapWater = false;

	m_SnowDir = Vector(0, 0, 1);
	m_SnowDot = 0.5f;
#endif
}

CWorld::~CWorld( )
{
	EventList_Free();
	ActivityList_Free();
	if ( g_pGameRules )
	{
		g_pGameRules->LevelShutdown();
		delete g_pGameRules;
		g_pGameRules = NULL;
	}
	g_WorldEntity = NULL;
}


//------------------------------------------------------------------------------
// Purpose : Add a decal to the world
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CWorld::DecalTrace( trace_t *pTrace, char const *decalName)
{
	int index = decalsystem->GetDecalIndexForName( decalName );
	if ( index < 0 )
		return;

	CBroadcastRecipientFilter filter;
	if ( pTrace->hitbox != 0 )
	{
		te->Decal( filter, 0.0f, &pTrace->endpos, &pTrace->startpos, 0, pTrace->hitbox, index );
	}
	else
	{
		te->WorldDecal( filter, 0.0, &pTrace->endpos, index );
	}
}

void CWorld::RegisterSharedActivities( void )
{
	ActivityList_RegisterSharedActivities();
}

void CWorld::RegisterSharedEvents( void )
{
	EventList_RegisterSharedEvents();
}


void CWorld::Spawn( void )
{
	SetLocalOrigin( vec3_origin );
	SetLocalAngles( vec3_angle );
	// NOTE:  SHOULD NEVER BE ANYTHING OTHER THAN 1!!!
	SetModelIndex( 1 );
	// world model
	SetModelName( AllocPooledString( modelinfo->GetModelName( GetModel() ) ) );
	AddFlag( FL_WORLDBRUSH );

	g_EventQueue.Init();
	Precache( );
#ifndef DARKINTERVAL	
	GlobalEntity_Add( "is_console", STRING(gpGlobals->mapname), ( IsConsole() ) ? GLOBAL_ON : GLOBAL_OFF );
	GlobalEntity_Add("is_pc", STRING(gpGlobals->mapname), ( !IsConsole() ) ? GLOBAL_ON : GLOBAL_OFF);
#else
	ConVarRef viewrange("r_farz"); // set maximum draw range to twice that of original
	if (viewrange.IsValid())		viewrange.SetValue(60000);

	ConVarRef loadingimage("cl_showloadingimage");
	if( loadingimage.IsValid() )	loadingimage.SetValue(1);

	ConVarRef eyeglint("r_eyeglintlodpixels"); // mysteriously cannot be found in game code?! 
	if (eyeglint.IsValid())			eyeglint.SetValue(10.0f);

	ConVarRef eyeglint2("r_glint_procedural"); // mysteriously cannot be found in game code?! 
	if (eyeglint.IsValid())			eyeglint2.SetValue(1);

	ConVarRef eyeglint3("r_glint_alwaysdraw"); // mysteriously cannot be found in game code?! 
	if (eyeglint.IsValid())			eyeglint3.SetValue(1);
#endif
}

static const char *g_DefaultLightstyles[] =
{
	// 0 normal
	"m",
	// 1 FLICKER (first variety)
	"mmnmmommommnonmmonqnmmo",
	// 2 SLOW STRONG PULSE
	"abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba",
	// 3 CANDLE (first variety)
	"mmmmmaaaaammmmmaaaaaabcdefgabcdefg",
	// 4 FAST STROBE
	"mamamamamama",
	// 5 GENTLE PULSE 1
	"jklmnopqrstuvwxyzyxwvutsrqponmlkj",
	// 6 FLICKER (second variety)
	"nmonqnmomnmomomno",
	// 7 CANDLE (second variety)
	"mmmaaaabcdefgmmmmaaaammmaamm",
	// 8 CANDLE (third variety)
	"mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa",
	// 9 SLOW STROBE (fourth variety)
	"aaaaaaaazzzzzzzz",
	// 10 FLUORESCENT FLICKER
	"mmamammmmammamamaaamammma",
	// 11 SLOW PULSE NOT FADE TO BLACK
	"abcdefghijklmnopqrrqponmlkjihgfedcba",
	// 12 UNDERWATER LIGHT MUTATION
	// this light only distorts the lightmap - no contribution
	// is made to the brightness of affected surfaces
	"mmnnmmnnnmmnn",
};


const char *GetDefaultLightstyleString( int styleIndex )
{
	if ( styleIndex < ARRAYSIZE(g_DefaultLightstyles) )
	{
		return g_DefaultLightstyles[styleIndex];
	}
	return "m";
}
#ifdef DARKINTERVAL
//-----------------------------------------------------------------------------
// Purpose: this has to mirror same list in props.cpp, used for random loot in simple crates
//-----------------------------------------------------------------------------
const char *precacheCrateLootItems[10] = {
	"models/props_junk/cardboard_box004a.mdl",
	"models/props_junk/cinderblock01a.mdl",
	"models/props_junk/garbage_metalcan001a.mdl",
	"models/props_junk/garbage_milkcarton002a.mdl",
	"models/props_junk/glassbottle01a.mdl",
	"models/props_junk/popcan01a.mdl",
	"models/props_junk/flare.mdl",
	"models/props_junk/shoe001a.mdl",
	"models/weapons/w_stunbaton.mdl", // for special occurances
	"models/props_junk/sawblade001a.mdl", // for even more special occurances

};

void CWorld::PrecacheCommonModels(void)
{
}
#endif
void CWorld::Precache( void )
{
#ifdef DARKINTERVAL
	PrecacheCommonModels();

//	ConVarRef dockswaterfix("r_waterforcereflectentities");
//	if( m_bForceCheapWater )
//		dockswaterfix.SetValue(0);
//	else
//		dockswaterfix.SetValue(1);

	ConVarRef fullbrightfix("mat_fullbright");
	fullbrightfix.SetValue(0);  // Try and disable fullbright. Done to fix transitions from fullbright maps
								// when normal maps after them keep the fullbright turned on. 
								// Doing this in Precache() because we want to preceed dynamic models spawn
								// otherwise there's this glitch when they retain fullbright on them. 

	ConVarRef detail_fixup("cl_detail_use_legacy_fixup");

	ConVarRef enableskybox("r_drawskybox");
	enableskybox.SetValue(1);

	ConVarRef forcedynamic("mat_forcedynamic");
	forcedynamic.SetValue(0); // reset dynamic buffer to 0. For maps that explicitly want it, they can specify it in the map cfg.
	
	char szMapName[256];
	Q_strncpy(szMapName, STRING(gpGlobals->mapname), sizeof(szMapName));
	Q_strlower(szMapName);

	if (!Q_strnicmp(szMapName, "d1_", 3) || !Q_strnicmp(szMapName, "d2_", 3) || !Q_strnicmp(szMapName, "d3_", 3) || !Q_strnicmp(szMapName, "ep1", 3))
	{
		detail_fixup.SetValue(1);
	}
	else
	{
		detail_fixup.SetValue(0);
	}

	if (gpGlobals->eLoadType != MapLoad_NewGame && gpGlobals->eLoadType != MapLoad_Transition)
		SetStartDark(false);
#endif // DARKINTERVAL
	g_WorldEntity = this;
	g_fGameOver = false;
	g_pLastSpawn = NULL;

	ConVarRef stepsize( "sv_stepsize" );
	stepsize.SetValue( 18 );

	ConVarRef roomtype( "room_type" );
	roomtype.SetValue( 0 );

	// Set up game rules
	Assert( !g_pGameRules );
	if (g_pGameRules)
	{
		delete g_pGameRules;
	}

	InstallGameRules();
	Assert( g_pGameRules );
	g_pGameRules->Init();

	CSoundEnt::InitSoundEnt();

	// Only allow precaching between LevelInitPreEntity and PostEntity
	CBaseEntity::SetAllowPrecache( true );
	IGameSystem::LevelInitPreEntityAllSystems( STRING( GetModelName() ) );

	// Create the player resource
	g_pGameRules->CreateStandardEntities();

	// UNDONE: Make most of these things server systems or precache_registers
	// =================================================
	//	Activities
	// =================================================
	ActivityList_Free();
	RegisterSharedActivities();

	EventList_Free();
	RegisterSharedEvents();

	InitBodyQue();
// init sentence group playback stuff from sentences.txt.
// ok to call this multiple times, calls after first are ignored.

	SENTENCEG_Init();

	// Precache standard particle systems
	PrecacheStandardParticleSystems( );

// the area based ambient sounds MUST be the first precache_sounds

// player precaches     
	W_Precache ();									// get weapon precaches
	ClientPrecache();
	g_pGameRules->Precache();
	// precache all temp ent stuff
	CBaseTempEntity::PrecacheTempEnts();
#ifndef DARKINTERVAL
	g_Language.SetValue( LANGUAGE_ENGLISH );	// TODO use VGUI to get current language

	if ( g_Language.GetInt() == LANGUAGE_GERMAN )
	{
		PrecacheModel("models/germangibs.mdl");
	} 
	else
#endif
	PrecacheModel( "models/gibs/hgibs.mdl" );
#ifdef DARKINTERVAL
	for (int i = 0; i < 10; ++i)
	{
		PrecacheModel(precacheCrateLootItems[i]);
	}

	PrecacheFileNPCInfoDatabase(filesystem, g_pGameRules->GetEncryptionKey());
#endif
	PrecacheScriptSound( "BaseEntity.EnterWater" );
	PrecacheScriptSound( "BaseEntity.ExitWater" );

//
// Setup light animation tables. 'a' is total darkness, 'z' is maxbright.
//
	for ( int i = 0; i < ARRAYSIZE(g_DefaultLightstyles); i++ )
	{
		engine->LightStyle( i, GetDefaultLightstyleString(i) );
	}

	// styles 32-62 are assigned by the light program for switchable lights

	// 63 testing
	engine->LightStyle(63, "a");

	// =================================================
	//	Load and Init AI Networks
	// =================================================
	CAI_NetworkManager::InitializeAINetworks();
	// =================================================
	//	Load and Init AI Schedules
	// =================================================
	g_AI_SchedulesManager.LoadAllSchedules();
	// =================================================
	//	Initialize NPC Relationships
	// =================================================
	g_pGameRules->InitDefaultAIRelationships();
	CBaseCombatCharacter::InitInteractionSystem();

	// Call all registered precachers.
	CPrecacheRegister::Precache();	

#ifndef DARKINTERVAL
	if ( m_iszChapterTitle != NULL_STRING )
	{
		DevMsg( 2, "Chapter title: %s\n", STRING(m_iszChapterTitle) );
		CMessage *pMessage = (CMessage *)CBaseEntity::Create( "env_message", vec3_origin, vec3_angle, NULL );
		if ( pMessage )
		{
			pMessage->SetMessage( m_iszChapterTitle );
			m_iszChapterTitle = NULL_STRING;

			// send the message entity a play message command, delayed by 1 second
			pMessage->AddSpawnFlags( SF_MESSAGE_ONCE );
			pMessage->SetThink( &CMessage::SUB_CallUseToggle );
			pMessage->SetNextThink( CURTIME + 1.0f );
		}
	}
#endif

	g_iszFuncBrushClassname = AllocPooledString("func_brush");

#ifdef DARKINTERVAL	 // execute map-specific config; this happens in Precache because some things
	CUtlString sCmd; // shoudl be decided before spawning. todo - consider having post-spawn config too
	sCmd.Format("exec %s.cfg\n", gpGlobals->mapname);
	engine->ServerCommand(sCmd.Get());
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float GetRealTime()
{
	return engine->Time();
}

bool CWorld::GetDisplayTitle() const
{
	return m_bDisplayTitle;
}
#ifdef DARKINTERVAL
const char *CWorld::GetMapTitle()
{
	static char outstr[256];
	outstr[0] = 0;
	bool gotname = false;

	const char *maptitle = m_iszChapterTitle.ToCStr();
	if (maptitle && maptitle[0])
	{
		Q_strncpy(outstr, maptitle, sizeof(outstr));
		gotname = true;
	}

	if (!gotname)
	{
		Q_strncpy(outstr, "", sizeof(outstr));
	}

	return outstr;
}
#endif
bool CWorld::GetStartDark() const
{
	return m_bStartDark;
}

void CWorld::SetDisplayTitle( bool display )
{
	m_bDisplayTitle = display;
}

void CWorld::SetStartDark( bool startdark )
{
	m_bStartDark = startdark;
}

bool CWorld::IsColdWorld( void )
{
	return m_bColdWorld;
}
#ifdef DARKINTERVAL
bool CWorld::IsRainExpensive(void)
{
	return m_bRainExpensive;
}

bool CWorld::ForceCheapShadows(void)
{
	return m_bForceCheapShadows;
}
#endif