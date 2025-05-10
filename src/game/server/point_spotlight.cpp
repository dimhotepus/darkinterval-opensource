//========================================================================//
//
// Purpose: 
//
//===========================================================================//

#include "cbase.h"
#include "point_spotlight.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// Spawnflags
#define SF_SPOTLIGHT_START_LIGHT_ON			0x1
#define SF_SPOTLIGHT_NO_DYNAMIC_LIGHT		0x2
#ifdef DARKINTERVAL
#define SF_SPOTLIGHT_NO_CORE_LIGHT			1<<11 // disables the core glow, saves up 1 edict, and often looks good enough
//#define SF_SPOTLIGHT_GODRAYS				1<<11 // unused/cut feature that allowed trimming them dynamically against spinning fans, etc; didn't work right
#endif
BEGIN_DATADESC( CPointSpotlight )
	DEFINE_FIELD( m_flSpotlightCurLength, FIELD_FLOAT ),

	DEFINE_FIELD( m_bSpotlightOn,			FIELD_BOOLEAN ),
	DEFINE_FIELD( m_bEfficientSpotlight,	FIELD_BOOLEAN ),
	DEFINE_FIELD( m_vSpotlightTargetPos,	FIELD_POSITION_VECTOR ),
	DEFINE_FIELD( m_vSpotlightCurrentPos,	FIELD_POSITION_VECTOR ),
#ifdef DARKINTERVAL
//	DEFINE_FIELD( m_bgodrayfade, FIELD_BOOLEAN ),
#endif
	// Robin: Don't Save, recreated after restore/transition
	//DEFINE_FIELD( m_hSpotlight,			FIELD_EHANDLE ),
	//DEFINE_FIELD( m_hSpotlightTarget,		FIELD_EHANDLE ),

	DEFINE_FIELD( m_vSpotlightDir,			FIELD_VECTOR ),
	DEFINE_FIELD( m_nHaloSprite,			FIELD_INTEGER ),
#ifdef DARKINTERVAL
	DEFINE_KEYFIELD(m_bIgnoreSolid, FIELD_BOOLEAN, "IgnoreSolid"),
#endif
	DEFINE_KEYFIELD( m_flSpotlightMaxLength,FIELD_FLOAT, "SpotlightLength"),
	DEFINE_KEYFIELD( m_flSpotlightGoalWidth,FIELD_FLOAT, "SpotlightWidth"),
	DEFINE_KEYFIELD( m_flHDRColorScale, FIELD_FLOAT, "HDRColorScale" ),
	DEFINE_KEYFIELD( m_nMinDXLevel, FIELD_INTEGER, "mindxlevel" ),
#ifdef DARKINTERVAL // to allow custom textures for the beam
	DEFINE_AUTO_ARRAY_KEYFIELD( szTextureName, FIELD_CHARACTER, "spotlighttexture"),
#endif
	// Inputs
	DEFINE_INPUTFUNC( FIELD_VOID,		"LightOn",		InputLightOn ),
	DEFINE_INPUTFUNC( FIELD_VOID,		"LightOff",		InputLightOff ),
#ifdef DARKINTERVAL
	DEFINE_INPUTFUNC( FIELD_VOID,		"SetBeamWidth", InputSetBeamWidth),
#endif
	DEFINE_OUTPUT( m_OnOn, "OnLightOn" ),
	DEFINE_OUTPUT( m_OnOff, "OnLightOff" ),

	DEFINE_THINKFUNC( SpotlightThink ),

END_DATADESC()

LINK_ENTITY_TO_CLASS(point_spotlight, CPointSpotlight);
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CPointSpotlight::CPointSpotlight()
{
#ifdef _DEBUG
	m_vSpotlightTargetPos.Init();
	m_vSpotlightCurrentPos.Init();
	m_vSpotlightDir.Init();
#endif
	m_flHDRColorScale = 1.0f;
	m_nMinDXLevel = 0;
#ifdef DARKINTERVAL
	m_bIgnoreSolid = false;

	Q_strcpy(szTextureName.GetForModify(), "sprites/glow_test02.vmt");
#endif
}
#ifdef DARKINTERVAL // to allow custom textures for the beam
bool CPointSpotlight::KeyValue(const char *szKeyValueName, const char* szKeyValue)
{
	if (FStrEq(szKeyValueName, "spotlighttexture"))
	{
		Q_strcpy(szTextureName.GetForModify(), szKeyValue);
	}
	else
	{
		return BaseClass::KeyValue(szKeyValueName, szKeyValue);
	}

	return true;
}
#endif
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPointSpotlight::Precache(void)
{
	BaseClass::Precache();
	// Sprites.	
	m_nHaloSprite = PrecacheModel("sprites/light_glow03.vmt");
#ifdef DARKINTERVAL
	PrecacheModel(szTextureName);
#else
	PrecacheModel("sprites/glow_test02.vmt");
#endif
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPointSpotlight::Spawn(void)
{
	Precache();
	UTIL_SetSize( this,vec3_origin,vec3_origin );
	AddSolidFlags( FSOLID_NOT_SOLID );
	SetMoveType( MOVETYPE_NONE );
	m_bEfficientSpotlight = true;
	// Check for user error
	if (m_flSpotlightMaxLength <= 0)
	{
		DevMsg("%s (%s) has an invalid spotlight length <= 0, setting to 500\n", GetClassname(), GetDebugName() );
		m_flSpotlightMaxLength = 500;
	}
	if (m_flSpotlightGoalWidth <= 0)
	{
		DevMsg("%s (%s) has an invalid spotlight width <= 0, setting to 10\n", GetClassname(), GetDebugName() );
		m_flSpotlightGoalWidth = 10;
	}
	
	if (m_flSpotlightGoalWidth > MAX_BEAM_WIDTH )
	{
		DevMsg("%s (%s) has an invalid spotlight width %.1f (max %.1f).\n", GetClassname(), GetDebugName(), m_flSpotlightGoalWidth, MAX_BEAM_WIDTH );
		m_flSpotlightGoalWidth = MAX_BEAM_WIDTH; 
	}

	// ------------------------------------
	//	Init all class vars 
	// ------------------------------------
	m_vSpotlightTargetPos	= vec3_origin;
	m_vSpotlightCurrentPos	= vec3_origin;
	m_hSpotlight			= NULL;
	m_hSpotlightTarget		= NULL;
	m_vSpotlightDir			= vec3_origin;
	m_flSpotlightCurLength	= m_flSpotlightMaxLength;

	m_bSpotlightOn = HasSpawnFlags( SF_SPOTLIGHT_START_LIGHT_ON );

	SetThink( &CPointSpotlight::SpotlightThink );
	SetNextThink( CURTIME + 0.1f );
}

//-----------------------------------------------------------------------------
// Creates the efficient spotlight 
//-----------------------------------------------------------------------------
void CPointSpotlight::CreateEfficientSpotlight()
{
	if ( m_hSpotlightTarget.Get() != NULL )
		return;
#ifdef DARKINTERVAL
	SpotlightCreate( szTextureName );
#else
	SpotlightCreate();
#endif
	m_vSpotlightCurrentPos = SpotlightCurrentPos();
	m_hSpotlightTarget->SetAbsOrigin( m_vSpotlightCurrentPos );
	m_hSpotlightTarget->m_vSpotlightOrg = GetAbsOrigin();
	VectorSubtract( m_hSpotlightTarget->GetAbsOrigin(), m_hSpotlightTarget->m_vSpotlightOrg, m_hSpotlightTarget->m_vSpotlightDir );
	m_flSpotlightCurLength = VectorNormalize( m_hSpotlightTarget->m_vSpotlightDir );
	m_hSpotlightTarget->SetMoveType( MOVETYPE_NONE );
	ComputeRenderInfo();

	m_OnOn.FireOutput( this, this );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPointSpotlight::Activate(void)
{
	BaseClass::Activate();

	if ( GetMoveParent() )
	{
		m_bEfficientSpotlight = false;
	}

	if ( m_bEfficientSpotlight )
	{
		if ( m_bSpotlightOn )
		{
			CreateEfficientSpotlight();
		}

		// Don't think
		SetThink( NULL );
	}

}

//-------------------------------------------------------------------------------------
// Optimization to deal with spotlights
//-------------------------------------------------------------------------------------
void CPointSpotlight::OnEntityEvent( EntityEvent_t event, void *pEventData )
{
	if ( event == ENTITY_EVENT_PARENT_CHANGED )
	{
		if ( GetMoveParent() )
		{
			m_bEfficientSpotlight = false;
			if ( m_hSpotlightTarget )
			{
				m_hSpotlightTarget->SetMoveType( MOVETYPE_FLY );
			}
			SetThink( &CPointSpotlight::SpotlightThink );
			SetNextThink( CURTIME + 0.1f );
		}
	}

	BaseClass::OnEntityEvent( event, pEventData );
}
	
//-------------------------------------------------------------------------------------
// Purpose : Send even though we don't have a model so spotlight gets proper position
// Input   :
// Output  :
//-------------------------------------------------------------------------------------
int CPointSpotlight::UpdateTransmitState()
{
	if ( m_bEfficientSpotlight )
		return SetTransmitState( FL_EDICT_DONTSEND );

	return SetTransmitState( FL_EDICT_PVSCHECK );
}

//-----------------------------------------------------------------------------
// Purpose: Plays the engine sound.
//-----------------------------------------------------------------------------
void CPointSpotlight::SpotlightThink( void )
{
	if ( GetMoveParent() )
	{
		SetNextThink( CURTIME + TICK_INTERVAL );
	}
	else
	{
		SetNextThink( CURTIME + 0.1f );
	}

	SpotlightUpdate();
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
#ifdef DARKINTERVAL
void CPointSpotlight::SpotlightCreate(const char *pSpotlightTextureName)
#else
void CPointSpotlight::SpotlightCreate(void)
#endif
{
	if ( m_hSpotlightTarget.Get() != NULL )
		return;

	AngleVectors( GetAbsAngles(), &m_vSpotlightDir );
#ifdef DARKINTERVAL
	Vector vTargetPos;
	if (m_bIgnoreSolid)
	{
		vTargetPos = GetAbsOrigin() + m_vSpotlightDir * m_flSpotlightMaxLength;
	}
	else
	{
		trace_t tr;
		UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + m_vSpotlightDir * m_flSpotlightMaxLength, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
		vTargetPos = tr.endpos;
	}
#else
	trace_t tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + m_vSpotlightDir * m_flSpotlightMaxLength, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
#endif
	m_hSpotlightTarget = (CSpotlightEnd*)CreateEntityByName( "spotlight_end" );
	m_hSpotlightTarget->Spawn();
#ifdef DARKINTERVAL
	m_hSpotlightTarget->SetAbsOrigin(vTargetPos);
#else
	m_hSpotlightTarget->SetAbsOrigin(tr.endpos);
#endif
	m_hSpotlightTarget->SetOwnerEntity( this );
	m_hSpotlightTarget->m_clrRender = m_clrRender;
	m_hSpotlightTarget->m_Radius = m_flSpotlightMaxLength;

	if ( FBitSet (m_spawnflags, SF_SPOTLIGHT_NO_DYNAMIC_LIGHT) )
	{
		m_hSpotlightTarget->m_flLightScale = 0.0;
	}
#ifdef DARKINTERVAL
	m_hSpotlight = CBeam::BeamCreate( pSpotlightTextureName, m_flSpotlightGoalWidth);
#else
	m_hSpotlight = CBeam::BeamCreate("sprites/glow_test02.vmt", m_flSpotlightGoalWidth);
#endif
	// Set the temporary spawnflag on the beam so it doesn't save (we'll recreate it on restore)
	m_hSpotlight->SetHDRColorScale( m_flHDRColorScale );
	m_hSpotlight->AddSpawnFlags( SF_BEAM_TEMPORARY );
	m_hSpotlight->SetColor( m_clrRender->r, m_clrRender->g, m_clrRender->b ); 
	m_hSpotlight->SetHaloTexture(m_nHaloSprite);
#ifdef DARKINTERVAL
	m_hSpotlight->SetHaloScale(HasSpawnFlags(SF_SPOTLIGHT_NO_CORE_LIGHT) ? 0 : 60);
#else
	m_hSpotlight->SetHaloScale(60);
#endif
	m_hSpotlight->SetEndWidth(m_flSpotlightGoalWidth);
	m_hSpotlight->SetBeamFlags( (FBEAM_SHADEOUT|FBEAM_NOTILE) );
	m_hSpotlight->SetBrightness( 64 );
	m_hSpotlight->SetNoise( 0 );
	m_hSpotlight->SetMinDXLevel( m_nMinDXLevel );

	if ( m_bEfficientSpotlight )
	{
		m_hSpotlight->PointsInit( GetAbsOrigin(), m_hSpotlightTarget->GetAbsOrigin() );
	}
	else
	{
		m_hSpotlight->EntsInit( this, m_hSpotlightTarget );
	}
#ifdef DARKINTERVAL
	m_hSpotlight->PointsInit(GetAbsOrigin(), m_hSpotlightTarget->GetAbsOrigin());
#endif
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
Vector CPointSpotlight::SpotlightCurrentPos(void)
{
	AngleVectors( GetAbsAngles(), &m_vSpotlightDir );

	//	Get beam end point.  Only collide with solid objects, not npcs
#ifdef DARKINTERVAL
	Vector vEndPos = GetAbsOrigin() + (m_vSpotlightDir * 2 * m_flSpotlightMaxLength);
	if (m_bIgnoreSolid)
	{
		return vEndPos;
	}
	else
	{
		trace_t tr;
		UTIL_TraceLine(GetAbsOrigin(), vEndPos, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
		return tr.endpos;
	}
#else
	trace_t tr;
	UTIL_TraceLine(GetAbsOrigin(), GetAbsOrigin() + ( m_vSpotlightDir * 2 * m_flSpotlightMaxLength ), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
	return tr.endpos;
#endif
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CPointSpotlight::SpotlightDestroy(void)
{
	if ( m_hSpotlight )
	{
		m_OnOff.FireOutput( this, this );

		UTIL_Remove(m_hSpotlight);
		UTIL_Remove(m_hSpotlightTarget);
	}
}

//------------------------------------------------------------------------------
// Purpose : Update the direction and position of my spotlight
// Input   :
// Output  :
//------------------------------------------------------------------------------
void CPointSpotlight::SpotlightUpdate(void)
{
	// ---------------------------------------------------
	//  If I don't have a spotlight attempt to create one
	// ---------------------------------------------------
	if (!m_hSpotlight)
	{
		if (m_bSpotlightOn)
		{
			// Make the spotlight
#ifdef DARKINTERVAL
			SpotlightCreate(szTextureName);
#else
			SpotlightCreate();
#endif
		}
		else
		{
			return;
		}
	}
	else if (!m_bSpotlightOn)
	{
		SpotlightDestroy();
		return;
	}

	if (!m_hSpotlightTarget)
	{
		DevWarning("**Attempting to update point_spotlight but target ent is NULL\n");
		SpotlightDestroy();
#ifdef DARKINTERVAL
		SpotlightCreate(szTextureName);
#else
		SpotlightCreate();
#endif
		if (!m_hSpotlightTarget)
			return;
	}

	m_vSpotlightCurrentPos = SpotlightCurrentPos();

	//  Update spotlight target velocity
	Vector vTargetDir;
	VectorSubtract(m_vSpotlightCurrentPos, m_hSpotlightTarget->GetAbsOrigin(), vTargetDir);
	float vTargetDist = vTargetDir.Length();

	// If we haven't moved at all, don't recompute
	if (vTargetDist < 1)
	{
		m_hSpotlightTarget->SetAbsVelocity(vec3_origin);
		return;
	}

	Vector vecNewVelocity = vTargetDir;
	VectorNormalize(vecNewVelocity);
	vecNewVelocity *= (10 * vTargetDist);

	// If a large move is requested, just jump to final spot as we probably hit a discontinuity
#ifdef DARKINTERVAL
	if (vecNewVelocity.Length() > 1000)
#else
	if ( vecNewVelocity.Length() > 200 )
#endif
	{
		VectorNormalize(vecNewVelocity);
		vecNewVelocity *= 200;
		VectorNormalize(vTargetDir);
		m_hSpotlightTarget->SetAbsOrigin(m_vSpotlightCurrentPos);
	}
	m_hSpotlightTarget->SetAbsVelocity( vecNewVelocity );
	m_hSpotlightTarget->m_vSpotlightOrg = GetAbsOrigin();

	// Avoid sudden change in where beam fades out when cross disconinuities
	VectorSubtract( m_hSpotlightTarget->GetAbsOrigin(), m_hSpotlightTarget->m_vSpotlightOrg, m_hSpotlightTarget->m_vSpotlightDir );
	float flBeamLength	= VectorNormalize( m_hSpotlightTarget->m_vSpotlightDir );
	m_flSpotlightCurLength = (0.60*m_flSpotlightCurLength) + (0.4*flBeamLength);

	ComputeRenderInfo();

	//NDebugOverlay::Cross3D(GetAbsOrigin(),Vector(-5,-5,-5),Vector(5,5,5),0,255,0,true,0.1);
	//NDebugOverlay::Cross3D(m_vSpotlightCurrentPos,Vector(-5,-5,-5),Vector(5,5,5),0,255,0,true,0.1);
	//NDebugOverlay::Cross3D(m_vSpotlightTargetPos,Vector(-5,-5,-5),Vector(5,5,5),255,0,0,true,0.1);
}

//-----------------------------------------------------------------------------
// Computes render info for a spotlight
//-----------------------------------------------------------------------------
void CPointSpotlight::ComputeRenderInfo()
{
	// Fade out spotlight end if past max length. 
	if (m_flSpotlightCurLength > 2 * m_flSpotlightMaxLength)
	{
		m_hSpotlightTarget->SetRenderColorA(0);
		m_hSpotlight->SetFadeLength(m_flSpotlightMaxLength);
	}
	else if (m_flSpotlightCurLength > m_flSpotlightMaxLength)
	{
		m_hSpotlightTarget->SetRenderColorA((1 - ((m_flSpotlightCurLength - m_flSpotlightMaxLength) / m_flSpotlightMaxLength)));
		m_hSpotlight->SetFadeLength(m_flSpotlightMaxLength);
	}
	else
	{
		m_hSpotlightTarget->SetRenderColorA(1.0);
		m_hSpotlight->SetFadeLength(m_flSpotlightCurLength);
	}

	// Adjust end width to keep beam width constant
	float flNewWidth = m_flSpotlightGoalWidth * (m_flSpotlightCurLength / m_flSpotlightMaxLength);
	flNewWidth = clamp(flNewWidth, 0.f, MAX_BEAM_WIDTH);
	m_hSpotlight->SetEndWidth(flNewWidth);

	// Adjust width of light on the end.  
	if (FBitSet(m_spawnflags, SF_SPOTLIGHT_NO_DYNAMIC_LIGHT))
	{
		m_hSpotlightTarget->m_flLightScale = 0.0;
	}
	else
	{
		// <<TODO>> - magic number 1.8 depends on sprite size
		m_hSpotlightTarget->m_flLightScale = 1.8*flNewWidth;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPointSpotlight::InputLightOn( inputdata_t &inputdata )
{
	if ( !m_bSpotlightOn )
	{
		m_bSpotlightOn = true;
		if ( m_bEfficientSpotlight )
		{
			CreateEfficientSpotlight();
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPointSpotlight::InputLightOff( inputdata_t &inputdata )
{
	if ( m_bSpotlightOn )
	{
		m_bSpotlightOn = false;
		if ( m_bEfficientSpotlight )
		{
			SpotlightDestroy();
		}
	}
}
#ifdef DARKINTERVAL
void CPointSpotlight::InputSetBeamWidth(inputdata_t &inputdata)
{
	m_flSpotlightGoalWidth = inputdata.value.Float();
	SpotlightUpdate();
}
#endif