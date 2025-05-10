//========================================================================//
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_basehlplayer.h"
#include "playerandobjectenumerator.h"
#include "engine/ivdebugoverlay.h"
#include "c_ai_basenpc.h"
#include "in_buttons.h"
#include "collisionutils.h"
#ifdef PORTAL_COMPILE
#include "vcollide_parse.h"
#include "view.h"
#include "c_basetempentity.h"
#include "takedamageinfo.h"
#include "iviewrender_beams.h"
#include "r_efx.h"
#include "dlight.h"
#include "portalrender.h"
#include "toolframework/itoolframework.h"
#include "toolframework_client.h"
#include "tier1/keyvalues.h"
#include "screenspaceeffects.h"
#include "portal_shareddefs.h"
#include "ivieweffects.h"		// for screenshake
#include "prop_portal_shared.h"
#endif // PORTAL_COMPILE
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

// How fast to avoid collisions with center of other object, in units per second
#define AVOID_SPEED 2000.0f
extern ConVar cl_forwardspeed;
extern ConVar cl_backspeed;
extern ConVar cl_sidespeed;

extern ConVar zoom_sensitivity_ratio;
extern ConVar default_fov;
extern ConVar sensitivity;

ConVar cl_npc_speedmod_intime( "cl_npc_speedmod_intime", "0.25", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
ConVar cl_npc_speedmod_outtime( "cl_npc_speedmod_outtime", "1.5", FCVAR_CLIENTDLL | FCVAR_ARCHIVE );
#ifdef DARKINTERVAL
ConVar di_new_underwater_fx("di_new_underwater_fx", "1", 0, "If 1 or 2, apply underwater colour correction and change FOV slightly. If 2, only apply CC and do not change FOV underwater." );
#endif
#ifdef PORTAL_COMPILE
LINK_ENTITY_TO_CLASS(player, C_BaseHLPlayer);
#endif

IMPLEMENT_CLIENTCLASS_DT(C_BaseHLPlayer, DT_HL2_Player, CHL2_Player)
	RecvPropDataTable( RECVINFO_DT(m_HL2Local),0, &REFERENCE_RECV_TABLE(DT_HL2Local) ),
	RecvPropBool( RECVINFO( m_fIsSprinting ) ),
#ifdef PORTAL_COMPILE
	RecvPropFloat(RECVINFO(m_angEyeAngles[0])),
	RecvPropFloat(RECVINFO(m_angEyeAngles[1])),
	RecvPropBool(RECVINFO(m_bHeldObjectOnOppositeSideOfPortal)),
	RecvPropEHandle(RECVINFO(m_pHeldObjectPortal)),
	RecvPropBool(RECVINFO(m_bPitchReorientation)),
	RecvPropEHandle(RECVINFO(m_hPortalEnvironment)),
	RecvPropEHandle(RECVINFO(m_hSurroundingLiquidPortal)),
#endif
END_RECV_TABLE()

BEGIN_PREDICTION_DATA( C_BaseHLPlayer )
	DEFINE_PRED_TYPEDESCRIPTION( m_HL2Local, C_HL2PlayerLocalData ),
	DEFINE_PRED_FIELD( m_fIsSprinting, FIELD_BOOLEAN, FTYPEDESC_INSENDTABLE ),
END_PREDICTION_DATA()
#ifdef DARKINTERVAL
#define	_WALK_SPEED 150
#define	_NORM_SPEED 190
#define	_SPRINT_SPEED 320

#define UNDERWATER_CC_LOOKUP_FILENAME "materials/correction/cc_underwater.raw"
#endif

#ifdef PORTAL_COMPILE
extern bool g_bUpsideDown;
#define REORIENTATION_RATE 120.0f
#define REORIENTATION_ACCELERATION_RATE 400.0f
ConVar cl_reorient_in_air("cl_reorient_in_air", "1", FCVAR_ARCHIVE, "Allows the player to only reorient from being upside down while in the air.");
#endif
//-----------------------------------------------------------------------------
// Purpose: Drops player's primary weapon
//-----------------------------------------------------------------------------
void CC_DropPrimary( void )
{
	C_BasePlayer *pPlayer = (C_BasePlayer *) C_BasePlayer::GetLocalPlayer();
	
	if ( pPlayer == NULL )
		return;

	pPlayer->Weapon_DropPrimary();
}

static ConCommand dropprimary("dropprimary", CC_DropPrimary, "dropprimary: Drops the primary weapon of the player.");

//-----------------------------------------------------------------------------
// Constructor
//-----------------------------------------------------------------------------
//#ifdef PORTAL_COMPILE()
//C_BaseHLPlayer::C_BaseHLPlayer() : m_iv_angEyeAngles("C_BaseHLPlayer::m_iv_angEyeAngles")
//{
//#else 
C_BaseHLPlayer::C_BaseHLPlayer()
{
//#endif
	AddVar( &m_Local.m_vecPunchAngle, &m_Local.m_iv_vecPunchAngle, LATCH_SIMULATION_VAR );
	AddVar( &m_Local.m_vecPunchAngleVel, &m_Local.m_iv_vecPunchAngleVel, LATCH_SIMULATION_VAR );

	m_flZoomStart		= 0.0f;
	m_flZoomEnd			= 0.0f;
	m_flZoomRate		= 0.0f;
	m_flZoomStartTime	= 0.0f;
	m_flSpeedMod		= cl_forwardspeed.GetFloat();
#ifdef DARKINTERVAL
	m_CCUnderWaterHandle = INVALID_CLIENT_CCHANDLE;
#endif
#ifdef PORTAL_COMPILE
	m_bHeldObjectOnOppositeSideOfPortal = false;
	m_pHeldObjectPortal = 0;

	m_bPitchReorientation = false;
	m_fReorientationRate = 0.0f;

	m_angEyeAngles.Init();

	AddVar(&m_angEyeAngles, &m_iv_angEyeAngles, LATCH_SIMULATION_VAR);
#endif
}
#ifdef DARKINTERVAL
C_BaseHLPlayer::~C_BaseHLPlayer()
{
	g_pColorCorrectionMgr->RemoveColorCorrection(m_CCUnderWaterHandle);
}
#endif
#ifdef PORTAL_COMPILE
void C_BaseHLPlayer::FixTeleportationRoll(void)
{
	if (IsInAVehicle()) //HL2 compatibility fix. do absolutely nothing to the view in vehicles
		return;

	if (!IsLocalPlayer())
		return;
	
	// Normalize roll from odd portal transitions
	QAngle vAbsAngles = EyeAngles();


	Vector vCurrentForward, vCurrentRight, vCurrentUp;
	AngleVectors(vAbsAngles, &vCurrentForward, &vCurrentRight, &vCurrentUp);

	if ( vAbsAngles[ ROLL ] == 0.0f )
	{
		m_fReorientationRate = 0.0f;
		g_bUpsideDown = ( vCurrentUp.z < 0.0f );
		return;
	}

	bool bForcePitchReorient = ( vAbsAngles[ ROLL ] > 175.0f && vCurrentForward.z > 0.99f );
	bool bOnGround = ( GetGroundEntity() != NULL );

	if ( bForcePitchReorient )
	{
		m_fReorientationRate = REORIENTATION_RATE * ( ( bOnGround ) ? ( 2.0f ) : ( 1.0f ) );
	} else
	{
		// Don't reorient in air if they don't want to
		if ( !cl_reorient_in_air.GetBool() && !bOnGround )
		{
			g_bUpsideDown = ( vCurrentUp.z < 0.0f );
			return;
		}
	}

	if ( vCurrentUp.z < 0.75f )
	{
		m_fReorientationRate += gpGlobals->frametime * REORIENTATION_ACCELERATION_RATE;

		// Upright faster if on the ground
		float fMaxReorientationRate = REORIENTATION_RATE * ( ( bOnGround ) ? ( 2.0f ) : ( 1.0f ) );
		if ( m_fReorientationRate > fMaxReorientationRate )
			m_fReorientationRate = fMaxReorientationRate;
	} else
	{
		if ( m_fReorientationRate > REORIENTATION_RATE * 0.5f )
		{
			m_fReorientationRate -= gpGlobals->frametime * REORIENTATION_ACCELERATION_RATE;
			if ( m_fReorientationRate < REORIENTATION_RATE * 0.5f )
				m_fReorientationRate = REORIENTATION_RATE * 0.5f;
		} else if ( m_fReorientationRate < REORIENTATION_RATE * 0.5f )
		{
			m_fReorientationRate += gpGlobals->frametime * REORIENTATION_ACCELERATION_RATE;
			if ( m_fReorientationRate > REORIENTATION_RATE * 0.5f )
				m_fReorientationRate = REORIENTATION_RATE * 0.5f;
		}
	}

	if ( !m_bPitchReorientation && !bForcePitchReorient )
	{
		// Randomize which way we roll if we're completely upside down
		if ( vAbsAngles[ ROLL ] == 180.0f && RandomInt(0, 1) == 1 )
		{
			vAbsAngles[ ROLL ] = -180.0f;
		}

		if ( vAbsAngles[ ROLL ] < 0.0f )
		{
			vAbsAngles[ ROLL ] += gpGlobals->frametime * m_fReorientationRate;
			if ( vAbsAngles[ ROLL ] > 0.0f )
				vAbsAngles[ ROLL ] = 0.0f;
			engine->SetViewAngles(vAbsAngles);
		} else if ( vAbsAngles[ ROLL ] > 0.0f )
		{
			vAbsAngles[ ROLL ] -= gpGlobals->frametime * m_fReorientationRate;
			if ( vAbsAngles[ ROLL ] < 0.0f )
				vAbsAngles[ ROLL ] = 0.0f;
			engine->SetViewAngles(vAbsAngles);
			m_angEyeAngles = vAbsAngles;
			m_iv_angEyeAngles.Reset();
		}
	} else
	{
		if ( vAbsAngles[ ROLL ] != 0.0f )
		{
			if ( vCurrentUp.z < 0.2f )
			{
				float fDegrees = gpGlobals->frametime * m_fReorientationRate;
				if ( vCurrentForward.z > 0.0f )
				{
					fDegrees = -fDegrees;
				}

				// Rotate around the right axis
				VMatrix mAxisAngleRot = SetupMatrixAxisRot(vCurrentRight, fDegrees);

				vCurrentUp = mAxisAngleRot.VMul3x3(vCurrentUp);
				vCurrentForward = mAxisAngleRot.VMul3x3(vCurrentForward);

				VectorAngles(vCurrentForward, vCurrentUp, vAbsAngles);

				engine->SetViewAngles(vAbsAngles);
				m_angEyeAngles = vAbsAngles;
				m_iv_angEyeAngles.Reset();
			} else
			{
				if ( vAbsAngles[ ROLL ] < 0.0f )
				{
					vAbsAngles[ ROLL ] += gpGlobals->frametime * m_fReorientationRate;
					if ( vAbsAngles[ ROLL ] > 0.0f )
						vAbsAngles[ ROLL ] = 0.0f;
					engine->SetViewAngles(vAbsAngles);
					m_angEyeAngles = vAbsAngles;
					m_iv_angEyeAngles.Reset();
				} else if ( vAbsAngles[ ROLL ] > 0.0f )
				{
					vAbsAngles[ ROLL ] -= gpGlobals->frametime * m_fReorientationRate;
					if ( vAbsAngles[ ROLL ] < 0.0f )
						vAbsAngles[ ROLL ] = 0.0f;
					engine->SetViewAngles(vAbsAngles);
					m_angEyeAngles = vAbsAngles;
					m_iv_angEyeAngles.Reset();
				}
			}
		}
	}

	// Keep track of if we're upside down for look control
	vAbsAngles = EyeAngles();
	AngleVectors(vAbsAngles, NULL, NULL, &vCurrentUp);

	if ( bForcePitchReorient )
		g_bUpsideDown = ( vCurrentUp.z < 0.0f );
	else
		g_bUpsideDown = false;
}

void C_BaseHLPlayer::PlayerPortalled(C_Prop_Portal *pEnteredPortal)
{
	if (pEnteredPortal)
	{
		m_bPortalledMessagePending = true;
		m_PendingPortalMatrix = pEnteredPortal->MatrixThisToLinked();

		if (IsLocalPlayer())
			g_pPortalRender->EnteredPortal(pEnteredPortal);
	}
}

void C_BaseHLPlayer::OnPreDataChanged(DataUpdateType_t type)
{
	Assert(m_pPortalEnvironment_LastCalcView == m_hPortalEnvironment.Get());
	PreDataChanged_Backup.m_hPortalEnvironment = m_hPortalEnvironment;
	PreDataChanged_Backup.m_hSurroundingLiquidPortal = m_hSurroundingLiquidPortal;
	PreDataChanged_Backup.m_qEyeAngles = m_iv_angEyeAngles.GetCurrent();

	BaseClass::OnPreDataChanged(type);
}
//CalcView() gets called between OnPreDataChanged() and OnDataChanged(), and these changes need to be known about in both before CalcView() gets called, and if CalcView() doesn't get called
bool C_BaseHLPlayer::DetectAndHandlePortalTeleportation(void)
{
	if (m_bPortalledMessagePending)
	{
		m_bPortalledMessagePending = false;

		//C_Prop_Portal *pOldPortal = PreDataChanged_Backup.m_hPortalEnvironment.Get();
		//Assert( pOldPortal );
		//if( pOldPortal )
		{
			Vector ptNewPosition = GetNetworkOrigin();

			UTIL_Portal_PointTransform(m_PendingPortalMatrix, PortalEyeInterpolation.m_vEyePosition_Interpolated, PortalEyeInterpolation.m_vEyePosition_Interpolated);
			UTIL_Portal_PointTransform(m_PendingPortalMatrix, PortalEyeInterpolation.m_vEyePosition_Uninterpolated, PortalEyeInterpolation.m_vEyePosition_Uninterpolated);

			PortalEyeInterpolation.m_bEyePositionIsInterpolating = true;

			UTIL_Portal_AngleTransform(m_PendingPortalMatrix, m_qEyeAngles_LastCalcView, m_angEyeAngles);
			m_angEyeAngles.x = AngleNormalize(m_angEyeAngles.x);
			m_angEyeAngles.y = AngleNormalize(m_angEyeAngles.y);
			m_angEyeAngles.z = AngleNormalize(m_angEyeAngles.z);
			m_iv_angEyeAngles.Reset(); //copies from m_angEyeAngles

			if (engine->IsPlayingDemo())
			{
				pl.v_angle = m_angEyeAngles;
				engine->SetViewAngles(pl.v_angle);
			}

			engine->ResetDemoInterpolation();
			if (IsLocalPlayer())
			{
				//DevMsg( "FPT: %.2f %.2f %.2f\n", m_angEyeAngles.x, m_angEyeAngles.y, m_angEyeAngles.z );
				SetLocalAngles(m_angEyeAngles);
			}
			// Reorient last facing direction to fix pops in view model lag
			for (int i = 0; i < MAX_VIEWMODELS; i++)
			{
				CBaseViewModel *vm = GetViewModel(i);
				if (!vm)
					continue;

				UTIL_Portal_VectorTransform(m_PendingPortalMatrix, vm->m_vecLastFacing, vm->m_vecLastFacing);
			}
		}
		m_bPortalledMessagePending = false;
	}

	return false;
}
#endif // PORTAL_COMPILE
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::OnDataChanged( DataUpdateType_t updateType )
{
	// Make sure we're thinking
	if ( updateType == DATA_UPDATE_CREATED )
	{
#ifdef DARKINTERVAL
		// Load color correction lookup for the underwater effect
		m_CCUnderWaterHandle = g_pColorCorrectionMgr->AddColorCorrection(UNDERWATER_CC_LOOKUP_FILENAME);
		if (m_CCUnderWaterHandle != INVALID_CLIENT_CCHANDLE)
		{
			g_pColorCorrectionMgr->SetColorCorrectionWeight(m_CCUnderWaterHandle, 0.0f);
		}
#endif
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}

	BaseClass::OnDataChanged( updateType );
#ifdef PORTAL_COMPILE
	if (m_hSurroundingLiquidPortal != PreDataChanged_Backup.m_hSurroundingLiquidPortal)
	{
		CLiquidPortal_InnerLiquidEffect *pLiquidEffect = (CLiquidPortal_InnerLiquidEffect *)g_pScreenSpaceEffects->GetScreenSpaceEffect("LiquidPortal_InnerLiquid");
		if (pLiquidEffect)
		{
			C_Func_LiquidPortal *pSurroundingPortal = m_hSurroundingLiquidPortal.Get();
			if (pSurroundingPortal != NULL)
			{
				C_Func_LiquidPortal *pOldSurroundingPortal = PreDataChanged_Backup.m_hSurroundingLiquidPortal.Get();
				if (pOldSurroundingPortal != pSurroundingPortal->m_hLinkedPortal.Get())
				{
					pLiquidEffect->m_pImmersionPortal = pSurroundingPortal;
					pLiquidEffect->m_bFadeBackToReality = false;
				}
				else
				{
					pLiquidEffect->m_bFadeBackToReality = true;
					pLiquidEffect->m_fFadeBackTimeLeft = pLiquidEffect->s_fFadeBackEffectTime;
				}
			}
			else
			{
				pLiquidEffect->m_pImmersionPortal = NULL;
				pLiquidEffect->m_bFadeBackToReality = false;
			}
		}
	}

	DetectAndHandlePortalTeleportation();

	if (updateType == DATA_UPDATE_CREATED)
	{

		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}
	UpdateVisibility();
#endif // PORTAL_COMPILE
}
#ifdef DARKINTERVAL
void CC_ToggleUnderWaterFX(void)
{
	ConVarRef r("di_new_underwater_fx");
	bool a = r.GetBool();
	r.SetValue(!a);
/*
	if( r.GetBool() )
		NDebugOverlay::ScreenText(0.05f, 0.25f, "UNDERWATER FX: ON", 255, 255, 255, 225, 2.0f);
	else
		NDebugOverlay::ScreenText(0.05f, 0.25f, "UNDERWATER FX: OFF", 255, 255, 255, 225, 2.0f);
*/
}

static ConCommand di_toggle_underwater_fx("di_toggle_underwater_fx", CC_ToggleUnderWaterFX, "Toggles new underwater fx");

void C_BaseHLPlayer::ClientThink(void)
{
#ifdef PORTAL_COMPILE
	FixTeleportationRoll();
#endif

	ConVarRef fovval("di_new_underwater_fx");

	if (fovval.GetInt() > 0)
	{
		if (m_CCUnderWaterHandle != INVALID_CLIENT_CCHANDLE)
		{
			if (GetWaterLevel() > 2)
			{
				g_pColorCorrectionMgr->SetColorCorrectionWeight(m_CCUnderWaterHandle, 0.5f);
				if (di_new_underwater_fx.GetInt() < 2 )
				{
					SetFOV(this, (GetFOV() - 5), 0.0f);
					fovval.SetValue(2);
				}
			}
			else
			{
				g_pColorCorrectionMgr->SetColorCorrectionWeight(m_CCUnderWaterHandle, 0.0f);
				fovval.SetValue(1);
				if (GetWaterLevel() == 2 && !IsZoomed()) // use exactly 2, because:
					// there's never a case when the player was underwater (level 3) and 
					// came out immediately out of any water or even to knee-deep.
					// and if you skip this check, this just always pops FOV back to 0. 
					// it also messes with let-go fov-go-back on suit zoom.
					// so this is a compromise: pop fov back to 0 when we are in deep water,
					// but otherwise don't do it. 
				{
					SetFOV(this, 0, 0.0f);
				}
			}
		}
	}
	else
	{
		if (m_CCUnderWaterHandle != INVALID_CLIENT_CCHANDLE)
			g_pColorCorrectionMgr->SetColorCorrectionWeight(m_CCUnderWaterHandle, 0.0f);
		fovval.SetValue(1);
		SetFOV(this, 0, 0.0f);
	}
}
#endif
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::Weapon_DropPrimary( void )
{
	engine->ServerCmd( "DropPrimary" );
}

float C_BaseHLPlayer::GetFOV()
{
	//Find our FOV with offset zoom value
	float flFOVOffset = BaseClass::GetFOV() + GetZoom();

	// Clamp FOV in MP
	int min_fov = ( gpGlobals->maxClients == 1 ) ? 5 : default_fov.GetInt();
	
	// Don't let it go too low
	flFOVOffset = MAX( min_fov, flFOVOffset );

	return flFOVOffset;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Output : float
//-----------------------------------------------------------------------------
float C_BaseHLPlayer::GetZoom( void )
{
	float fFOV = m_flZoomEnd;

	//See if we need to lerp the values
	if ( ( m_flZoomStart != m_flZoomEnd ) && ( m_flZoomRate > 0.0f ) )
	{
		float deltaTime = (float)( CURTIME - m_flZoomStartTime ) / m_flZoomRate;

		if ( deltaTime >= 1.0f )
		{
			//If we're past the zoom time, just take the new value and stop lerping
			fFOV = m_flZoomStart = m_flZoomEnd;
		}
		else
		{
			fFOV = SimpleSplineRemapVal( deltaTime, 0.0f, 1.0f, m_flZoomStart, m_flZoomEnd );
		}
	}

	return fFOV;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : FOVOffset - 
//			time - 
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::Zoom( float FOVOffset, float time )
{
	m_flZoomStart		= GetZoom();
	m_flZoomEnd			= FOVOffset;
	m_flZoomRate		= time;
	m_flZoomStartTime	= CURTIME;
}


//-----------------------------------------------------------------------------
// Purpose: Hack to zero out player's pitch, use value from poseparameter instead
// Input  : flags - 
// Output : int
//-----------------------------------------------------------------------------
int C_BaseHLPlayer::DrawModel( int flags )
{
	// Not pitch for player
	QAngle saveAngles = GetLocalAngles();

	QAngle useAngles = saveAngles;
	useAngles[ PITCH ] = 0.0f;

	SetLocalAngles( useAngles );

	int iret = BaseClass::DrawModel( flags );

	SetLocalAngles( saveAngles );

	return iret;
}

//-----------------------------------------------------------------------------
// Purpose: Helper to remove from ladder
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::ExitLadder()
{
	if ( MOVETYPE_LADDER != GetMoveType() )
		return;
	
	SetMoveType( MOVETYPE_WALK );
	SetMoveCollide( MOVECOLLIDE_DEFAULT );
	// Remove from ladder
	m_HL2Local.m_hLadder = NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Determines if a player can be safely moved towards a point
// Input:   pos - position to test move to, fVertDist - how far to trace downwards to see if the player would fall,
//			radius - how close the player can be to the object, objPos - position of the object to avoid,
//			objDir - direction the object is travelling
//-----------------------------------------------------------------------------
bool C_BaseHLPlayer::TestMove( const Vector &pos, float fVertDist, float radius, const Vector &objPos, const Vector &objDir )
{
	trace_t trUp;
	trace_t trOver;
	trace_t trDown;
	float flHit1, flHit2;
	
	UTIL_TraceHull( GetAbsOrigin(), pos, GetPlayerMins(), GetPlayerMaxs(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trOver );
	if ( trOver.fraction < 1.0f )
	{
		// check if the endpos intersects with the direction the object is travelling.  if it doesn't, this is a good direction to move.
		if ( objDir.IsZero() ||
			( IntersectInfiniteRayWithSphere( objPos, objDir, trOver.endpos, radius, &flHit1, &flHit2 ) && 
			( ( flHit1 >= 0.0f ) || ( flHit2 >= 0.0f ) ) )
			)
		{
			// our first trace failed, so see if we can go farther if we step up.

			// trace up to see if we have enough room.
			UTIL_TraceHull( GetAbsOrigin(), GetAbsOrigin() + Vector( 0, 0, m_Local.m_flStepSize ), 
				GetPlayerMins(), GetPlayerMaxs(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trUp );

			// do a trace from the stepped up height
			UTIL_TraceHull( trUp.endpos, pos + Vector( 0, 0, trUp.endpos.z - trUp.startpos.z ), 
				GetPlayerMins(), GetPlayerMaxs(), MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trOver );

			if ( trOver.fraction < 1.0f )
			{
				// check if the endpos intersects with the direction the object is travelling.  if it doesn't, this is a good direction to move.
				if ( objDir.IsZero() ||
					( IntersectInfiniteRayWithSphere( objPos, objDir, trOver.endpos, radius, &flHit1, &flHit2 ) && ( ( flHit1 >= 0.0f ) || ( flHit2 >= 0.0f ) ) ) )
				{
					return false;
				}
			}
		}
	}

	// trace down to see if this position is on the ground
	UTIL_TraceLine( trOver.endpos, trOver.endpos - Vector( 0, 0, fVertDist ), 
		MASK_SOLID_BRUSHONLY, NULL, COLLISION_GROUP_NONE, &trDown );

	if ( trDown.fraction == 1.0f ) 
		return false;

	return true;
}

//-----------------------------------------------------------------------------
// Client-side obstacle avoidance
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::PerformClientSideObstacleAvoidance( float flFrameTime, CUserCmd *pCmd )
{
	// Don't avoid if noclipping or in movetype none
	switch ( GetMoveType() )
	{
	case MOVETYPE_NOCLIP:
	case MOVETYPE_NONE:
	case MOVETYPE_OBSERVER:
		return;
	default:
		break;
	}

	// Try to steer away from any objects/players we might interpenetrate
	Vector size = WorldAlignSize();

	float radius = 0.7f * sqrt( size.x * size.x + size.y * size.y );
	float curspeed = GetLocalVelocity().Length2D();

	//int slot = 1;
	//engine->Con_NPrintf( slot++, "speed %f\n", curspeed );
	//engine->Con_NPrintf( slot++, "radius %f\n", radius );

	// If running, use a larger radius
	float factor = 1.0f;

	if ( curspeed > 150.0f )
	{
		curspeed = MIN( 2048.0f, curspeed );
		factor = ( 1.0f + ( curspeed - 150.0f ) / 150.0f );

		//engine->Con_NPrintf( slot++, "scaleup (%f) to radius %f\n", factor, radius * factor );

		radius = radius * factor;
	}

	Vector currentdir;
	Vector rightdir;

	QAngle vAngles = pCmd->viewangles;
	vAngles.x = 0;

	AngleVectors( vAngles, &currentdir, &rightdir, NULL );
		
	bool istryingtomove = false;
	bool ismovingforward = false;
	if ( fabs( pCmd->forwardmove ) > 0.0f || 
		fabs( pCmd->sidemove ) > 0.0f )
	{
		istryingtomove = true;
		if ( pCmd->forwardmove > 1.0f )
		{
			ismovingforward = true;
		}
	}

	if ( istryingtomove == true )
		 radius *= 1.3f;

	CPlayerAndObjectEnumerator avoid( radius );
	partition->EnumerateElementsInSphere( PARTITION_CLIENT_SOLID_EDICTS, GetAbsOrigin(), radius, false, &avoid );

	// Okay, decide how to avoid if there's anything close by
	int c = avoid.GetObjectCount();
	if ( c <= 0 )
		return;

	//engine->Con_NPrintf( slot++, "moving %s forward %s\n", istryingtomove ? "true" : "false", ismovingforward ? "true" : "false"  );

	float adjustforwardmove = 0.0f;
	float adjustsidemove	= 0.0f;

	for ( int i = 0; i < c; i++ )
	{
		C_AI_BaseNPC *obj = dynamic_cast< C_AI_BaseNPC *>(avoid.GetObject( i ));

		if( !obj )
			continue;

		Vector vecToObject = obj->GetAbsOrigin() - GetAbsOrigin();

		float flDist = vecToObject.Length2D();
		
		// Figure out a 2D radius for the object
		Vector vecWorldMins, vecWorldMaxs;
		obj->CollisionProp()->WorldSpaceAABB( &vecWorldMins, &vecWorldMaxs );
		Vector objSize = vecWorldMaxs - vecWorldMins;

		float objectradius = 0.5f * sqrt( objSize.x * objSize.x + objSize.y * objSize.y );

		//Don't run this code if the NPC is not moving UNLESS we are in stuck inside of them.
		if ( !obj->IsMoving() && flDist > objectradius )
			  continue;

		if ( flDist > objectradius && obj->IsEffectActive( EF_NODRAW ) )
		{
			obj->RemoveEffects( EF_NODRAW );
		}

		Vector vecNPCVelocity;
		obj->EstimateAbsVelocity( vecNPCVelocity );
		float flNPCSpeed = VectorNormalize( vecNPCVelocity );

		Vector vPlayerVel = GetAbsVelocity();
		VectorNormalize( vPlayerVel );

		float flHit1, flHit2;
		Vector vRayDir = vecToObject;
		VectorNormalize( vRayDir );

		float flVelProduct = DotProduct( vecNPCVelocity, vPlayerVel );
		float flDirProduct = DotProduct( vRayDir, vPlayerVel );

		if ( !IntersectInfiniteRayWithSphere(
				GetAbsOrigin(),
				vRayDir,
				obj->GetAbsOrigin(),
				radius,
				&flHit1,
				&flHit2 ) )
			continue;

        Vector dirToObject = -vecToObject;
		VectorNormalize( dirToObject );

		float fwd = 0;
		float rt = 0;

		float sidescale = 2.0f;
		float forwardscale = 1.0f;
		bool foundResult = false;

		Vector vMoveDir = vecNPCVelocity;
		if ( flNPCSpeed > 0.001f )
		{
			// This NPC is moving. First try deflecting the player left or right relative to the NPC's velocity.
			// Start with whatever side they're on relative to the NPC's velocity.
			Vector vecNPCTrajectoryRight = CrossProduct( vecNPCVelocity, Vector( 0, 0, 1) );
			int iDirection = ( vecNPCTrajectoryRight.Dot( dirToObject ) > 0 ) ? 1 : -1;
			for ( int nTries = 0; nTries < 2; nTries++ )
			{
				Vector vecTryMove = vecNPCTrajectoryRight * iDirection;
				VectorNormalize( vecTryMove );
				
				Vector vTestPosition = GetAbsOrigin() + vecTryMove * radius * 2;

				if ( TestMove( vTestPosition, size.z * 2, radius * 2, obj->GetAbsOrigin(), vMoveDir ) )
				{
					fwd = currentdir.Dot( vecTryMove );
					rt = rightdir.Dot( vecTryMove );
					
					//Msg( "PUSH DEFLECT fwd=%f, rt=%f\n", fwd, rt );
					
					foundResult = true;
					break;
				}
				else
				{
					// Try the other direction.
					iDirection *= -1;
				}
			}
		}
		else
		{
			// the object isn't moving, so try moving opposite the way it's facing
			Vector vecNPCForward;
			obj->GetVectors( &vecNPCForward, NULL, NULL );
			
			Vector vTestPosition = GetAbsOrigin() - vecNPCForward * radius * 2;
			if ( TestMove( vTestPosition, size.z * 2, radius * 2, obj->GetAbsOrigin(), vMoveDir ) )
			{
				fwd = currentdir.Dot( -vecNPCForward );
				rt = rightdir.Dot( -vecNPCForward );

				if ( flDist < objectradius )
				{
#ifndef DARKINTERVAL
				//	Msg("nodraw not\n");
					obj->AddEffects( EF_NODRAW );
#endif
				}

				//Msg( "PUSH AWAY FACE fwd=%f, rt=%f\n", fwd, rt );

				foundResult = true;
			}
		}

		if ( !foundResult )
		{
			// test if we can move in the direction the object is moving
			Vector vTestPosition = GetAbsOrigin() + vMoveDir * radius * 2;
			if ( TestMove( vTestPosition, size.z * 2, radius * 2, obj->GetAbsOrigin(), vMoveDir ) )
			{
				fwd = currentdir.Dot( vMoveDir );
				rt = rightdir.Dot( vMoveDir );

				if ( flDist < objectradius )
				{
#ifndef DARKINTERVAL
				//	Msg("nodraw not\n");
					obj->AddEffects( EF_NODRAW );
#endif
				}

				//Msg( "PUSH ALONG fwd=%f, rt=%f\n", fwd, rt );

				foundResult = true;
			}
			else
			{
				// try moving directly away from the object
				Vector vTestPosition = GetAbsOrigin() - dirToObject * radius * 2;
				if ( TestMove( vTestPosition, size.z * 2, radius * 2, obj->GetAbsOrigin(), vMoveDir ) )
				{
					fwd = currentdir.Dot( -dirToObject );
					rt = rightdir.Dot( -dirToObject );
					foundResult = true;

					//Msg( "PUSH AWAY fwd=%f, rt=%f\n", fwd, rt );
				}
			}
		}

		if ( !foundResult )
		{
			// test if we can move through the object
			Vector vTestPosition = GetAbsOrigin() - vMoveDir * radius * 2;
			fwd = currentdir.Dot( -vMoveDir );
			rt = rightdir.Dot( -vMoveDir );

			if ( flDist < objectradius )
			{
#ifndef DARKINTERVAL
			//	Msg("nodraw not\n");
				obj->AddEffects( EF_NODRAW );
#endif
			}

			//Msg( "PUSH THROUGH fwd=%f, rt=%f\n", fwd, rt );

			foundResult = true;
		}

		// If running, then do a lot more sideways veer since we're not going to do anything to
		//  forward velocity
		if ( istryingtomove )
		{
			sidescale = 6.0f;
		}

		if ( flVelProduct > 0.0f && flDirProduct > 0.0f )
		{
			sidescale = 0.1f;
		}

		float force = 1.0f;
		float forward = forwardscale * fwd * force * AVOID_SPEED;
		float side = sidescale * rt * force * AVOID_SPEED;

		adjustforwardmove	+= forward;
		adjustsidemove		+= side;
	}

	pCmd->forwardmove	+= adjustforwardmove;
	pCmd->sidemove		+= adjustsidemove;
	
	// Clamp the move to within legal limits, preserving direction. This is a little
	// complicated because we have different limits for forward, back, and side

	//Msg( "PRECLAMP: forwardmove=%f, sidemove=%f\n", pCmd->forwardmove, pCmd->sidemove );

	float flForwardScale = 1.0f;
	if ( pCmd->forwardmove > fabs( cl_forwardspeed.GetFloat() ) )
	{
		flForwardScale = fabs( cl_forwardspeed.GetFloat() ) / pCmd->forwardmove;
	}
	else if ( pCmd->forwardmove < -fabs( cl_backspeed.GetFloat() ) )
	{
		flForwardScale = fabs( cl_backspeed.GetFloat() ) / fabs( pCmd->forwardmove );
	}
	
	float flSideScale = 1.0f;
	if ( fabs( pCmd->sidemove ) > fabs( cl_sidespeed.GetFloat() ) )
	{
		flSideScale = fabs( cl_sidespeed.GetFloat() ) / fabs( pCmd->sidemove );
	}
	
	float flScale = MIN( flForwardScale, flSideScale );
	pCmd->forwardmove *= flScale;
	pCmd->sidemove *= flScale;

	//Msg( "POSTCLAMP: forwardmove=%f, sidemove=%f\n", pCmd->forwardmove, pCmd->sidemove );
}


void C_BaseHLPlayer::PerformClientSideNPCSpeedModifiers( float flFrameTime, CUserCmd *pCmd )
{
	if ( m_hClosestNPC == NULL )
	{
		if ( m_flSpeedMod != cl_forwardspeed.GetFloat() )
		{
			float flDeltaTime = (m_flSpeedModTime - CURTIME);
			m_flSpeedMod = RemapValClamped( flDeltaTime, cl_npc_speedmod_outtime.GetFloat(), 0, m_flExitSpeedMod, cl_forwardspeed.GetFloat() );
		}
	}
	else
	{
		C_AI_BaseNPC *pNPC = dynamic_cast< C_AI_BaseNPC *>( m_hClosestNPC.Get() );

		if ( pNPC )
		{
			float flDist = (GetAbsOrigin() - pNPC->GetAbsOrigin()).LengthSqr();
			bool bShouldModSpeed = false; 

			// Within range?
			if ( flDist < pNPC->GetSpeedModifyRadius() )
			{
				// Now, only slowdown if we're facing & running parallel to the target's movement
				// Facing check first (in 2D)
				Vector vecTargetOrigin = pNPC->GetAbsOrigin();
				Vector los = ( vecTargetOrigin - EyePosition() );
				los.z = 0;
				VectorNormalize( los );
				Vector facingDir;
				AngleVectors( GetAbsAngles(), &facingDir );
				float flDot = DotProduct( los, facingDir );
				if ( flDot > 0.8 )
				{
					/*
					// Velocity check (abort if the target isn't moving)
					Vector vecTargetVelocity;
					pNPC->EstimateAbsVelocity( vecTargetVelocity );
					float flSpeed = VectorNormalize(vecTargetVelocity);
					Vector vecMyVelocity = GetAbsVelocity();
					VectorNormalize(vecMyVelocity);
					if ( flSpeed > 1.0 )
					{
						// Velocity roughly parallel?
						if ( DotProduct(vecTargetVelocity,vecMyVelocity) > 0.4  )
						{
							bShouldModSpeed = true;
						}
					} 
					else
					{
						// NPC's not moving, slow down if we're moving at him
						//Msg("Dot: %.2f\n", DotProduct( los, vecMyVelocity ) );
						if ( DotProduct( los, vecMyVelocity ) > 0.8 )
						{
							bShouldModSpeed = true;
						} 
					}
					*/

					bShouldModSpeed = true;
				}
			}

			if ( !bShouldModSpeed )
			{
				m_hClosestNPC = NULL;
				m_flSpeedModTime = CURTIME + cl_npc_speedmod_outtime.GetFloat();
				m_flExitSpeedMod = m_flSpeedMod;
				return;
			}
			else 
			{
				if ( m_flSpeedMod != pNPC->GetSpeedModifySpeed() )
				{
					float flDeltaTime = (m_flSpeedModTime - CURTIME);
					m_flSpeedMod = RemapValClamped( flDeltaTime, cl_npc_speedmod_intime.GetFloat(), 0, cl_forwardspeed.GetFloat(), pNPC->GetSpeedModifySpeed() );
				}
			}
		}
	}

	if ( pCmd->forwardmove > 0.0f )
	{
		pCmd->forwardmove = clamp( pCmd->forwardmove, -m_flSpeedMod, m_flSpeedMod );
	}
	else
	{
		pCmd->forwardmove = clamp( pCmd->forwardmove, -m_flSpeedMod, m_flSpeedMod );
	}
	pCmd->sidemove = clamp( pCmd->sidemove, -m_flSpeedMod, m_flSpeedMod );
   
	//Msg( "fwd %f right %f\n", pCmd->forwardmove, pCmd->sidemove );
}

//-----------------------------------------------------------------------------
// Purpose: Input handling
//-----------------------------------------------------------------------------
bool C_BaseHLPlayer::CreateMove( float flInputSampleTime, CUserCmd *pCmd )
{
	bool bResult = BaseClass::CreateMove( flInputSampleTime, pCmd );

	if ( !IsInAVehicle() )
	{
		PerformClientSideObstacleAvoidance( TICK_INTERVAL, pCmd );
		PerformClientSideNPCSpeedModifiers( TICK_INTERVAL, pCmd );
	}

	return bResult;
}


//-----------------------------------------------------------------------------
// Purpose: Input handling
//-----------------------------------------------------------------------------
void C_BaseHLPlayer::BuildTransformations( CStudioHdr *hdr, Vector *pos, Quaternion q[], const matrix3x4_t& cameraTransform, int boneMask, CBoneBitList &boneComputed )
{
	BaseClass::BuildTransformations( hdr, pos, q, cameraTransform, boneMask, boneComputed );
}

#ifdef PORTAL_COMPILE

void C_BaseHLPlayer::UpdatePortalEyeInterpolation(void)
{
	//PortalEyeInterpolation.m_bEyePositionIsInterpolating = false;
	if (PortalEyeInterpolation.m_bUpdatePosition_FreeMove)
	{
		PortalEyeInterpolation.m_bUpdatePosition_FreeMove = false;

		C_Prop_Portal *pOldPortal = PreDataChanged_Backup.m_hPortalEnvironment.Get();
		if (pOldPortal)
		{
			UTIL_Portal_PointTransform(pOldPortal->MatrixThisToLinked(), PortalEyeInterpolation.m_vEyePosition_Interpolated, PortalEyeInterpolation.m_vEyePosition_Interpolated);
			//PortalEyeInterpolation.m_vEyePosition_Interpolated = pOldPortal->m_matrixThisToLinked * PortalEyeInterpolation.m_vEyePosition_Interpolated;

			//Vector vForward;
			//m_hPortalEnvironment.Get()->GetVectors( &vForward, NULL, NULL );

			PortalEyeInterpolation.m_vEyePosition_Interpolated = EyeFootPosition();

			PortalEyeInterpolation.m_bEyePositionIsInterpolating = true;
		}
	}

	if (IsInAVehicle())
		PortalEyeInterpolation.m_bEyePositionIsInterpolating = false;

	if (!PortalEyeInterpolation.m_bEyePositionIsInterpolating)
	{
		PortalEyeInterpolation.m_vEyePosition_Uninterpolated = EyeFootPosition();
		PortalEyeInterpolation.m_vEyePosition_Interpolated = PortalEyeInterpolation.m_vEyePosition_Uninterpolated;
		return;
	}

	Vector vThisFrameUninterpolatedPosition = EyeFootPosition();

	//find offset between this and last frame's uninterpolated movement, and apply this as freebie movement to the interpolated position
	PortalEyeInterpolation.m_vEyePosition_Interpolated += (vThisFrameUninterpolatedPosition - PortalEyeInterpolation.m_vEyePosition_Uninterpolated);
	PortalEyeInterpolation.m_vEyePosition_Uninterpolated = vThisFrameUninterpolatedPosition;

	Vector vDiff = vThisFrameUninterpolatedPosition - PortalEyeInterpolation.m_vEyePosition_Interpolated;
	float fLength = vDiff.Length();
	float fFollowSpeed = gpGlobals->frametime * 100.0f;
	const float fMaxDiff = 150.0f;
	if (fLength > fMaxDiff)
	{
		//camera lagging too far behind, give it a speed boost to bring it within maximum range
		fFollowSpeed = fLength - fMaxDiff;
	}
	else if (fLength < fFollowSpeed)
	{
		//final move
		PortalEyeInterpolation.m_bEyePositionIsInterpolating = false;
		PortalEyeInterpolation.m_vEyePosition_Interpolated = vThisFrameUninterpolatedPosition;
		return;
	}

	if (fLength > 0.001f)
	{
		vDiff *= (fFollowSpeed / fLength);
		PortalEyeInterpolation.m_vEyePosition_Interpolated += vDiff;
	}
	else
	{
		PortalEyeInterpolation.m_vEyePosition_Interpolated = vThisFrameUninterpolatedPosition;
	}
}

Vector C_BaseHLPlayer::EyePosition()
{
	return PortalEyeInterpolation.m_vEyePosition_Interpolated;
}

Vector C_BaseHLPlayer::EyeFootPosition(const QAngle &qEyeAngles)
{
#if 0
	static int iPrintCounter = 0;
	++iPrintCounter;
	if (iPrintCounter == 50)
	{
		QAngle vAbsAngles = qEyeAngles;
		DevMsg("Eye Angles: %f %f %f\n", vAbsAngles.x, vAbsAngles.y, vAbsAngles.z);
		iPrintCounter = 0;
	}
#endif

	//interpolate between feet and normal eye position based on view roll (gets us wall/ceiling & ceiling/ceiling teleportations without an eye position pop)
	float fFootInterp = fabs(qEyeAngles[ROLL]) * ((1.0f / 180.0f) * 0.75f); //0 when facing straight up, 0.75 when facing straight down
	return (BaseClass::EyePosition() - (fFootInterp * m_vecViewOffset)); //TODO: Find a good Up vector for this rolled player and interpolate along actual eye/foot axis
}

void C_BaseHLPlayer::CalcView(Vector &eyeOrigin, QAngle &eyeAngles, float &zNear, float &zFar, float &fov)
{
	DetectAndHandlePortalTeleportation();
	//if( DetectAndHandlePortalTeleportation() )
	//	DevMsg( "Teleported within OnDataChanged\n" );

	m_iForceNoDrawInPortalSurface = -1;
	bool bEyeTransform_Backup = m_bEyePositionIsTransformedByPortal;
	m_bEyePositionIsTransformedByPortal = false; //assume it's not transformed until it provably is
	UpdatePortalEyeInterpolation();

	QAngle qEyeAngleBackup = EyeAngles();
	Vector ptEyePositionBackup = EyePosition();
	C_Prop_Portal *pPortalBackup = m_hPortalEnvironment.Get();

	if (m_lifeState != LIFE_ALIVE)
	{
		if (g_nKillCamMode != 0)
		{
			return;
		}

		Vector origin = EyePosition();

		BaseClass::CalcView(eyeOrigin, eyeAngles, zNear, zFar, fov);

		eyeOrigin = origin;

		Vector vForward;
		AngleVectors(eyeAngles, &vForward);

		VectorNormalize(vForward);
#if !PORTAL_HIDE_PLAYER_RAGDOLL
		VectorMA(origin, -CHASE_CAM_DISTANCE, vForward, eyeOrigin);
#endif //PORTAL_HIDE_PLAYER_RAGDOLL

		Vector WALL_MIN(-WALL_OFFSET, -WALL_OFFSET, -WALL_OFFSET);
		Vector WALL_MAX(WALL_OFFSET, WALL_OFFSET, WALL_OFFSET);

		trace_t trace; // clip against world
		C_BaseEntity::PushEnableAbsRecomputations(false); // HACK don't recompute positions while doing RayTrace
		UTIL_TraceHull(origin, eyeOrigin, WALL_MIN, WALL_MAX, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &trace);
		C_BaseEntity::PopEnableAbsRecomputations();

		if (trace.fraction < 1.0)
		{
			eyeOrigin = trace.endpos;
		}
	}
	else
	{
		IClientVehicle *pVehicle;
		pVehicle = GetVehicle();

		if (!pVehicle)
		{
			if (IsObserver())
			{
				CalcObserverView(eyeOrigin, eyeAngles, fov);
			}
			else
			{
				CalcPlayerView(eyeOrigin, eyeAngles, fov);
				if (m_hPortalEnvironment.Get() != NULL)
				{
					//time for hax
					m_bEyePositionIsTransformedByPortal = bEyeTransform_Backup;
					CalcPortalView(eyeOrigin, eyeAngles);
				}
			}
		}
		else
		{
			CalcVehicleView(pVehicle, eyeOrigin, eyeAngles, zNear, zFar, fov);
		}
	}

	m_qEyeAngles_LastCalcView = qEyeAngleBackup;
	m_ptEyePosition_LastCalcView = ptEyePositionBackup;
	m_pPortalEnvironment_LastCalcView = pPortalBackup;
}

void C_BaseHLPlayer::SetLocalViewAngles(const QAngle &viewAngles)
{
	// Nothing
	if (engine->IsPlayingDemo())
		return;
	BaseClass::SetLocalViewAngles(viewAngles);
}

void C_BaseHLPlayer::SetViewAngles(const QAngle& ang)
{
	BaseClass::SetViewAngles(ang);

	if (engine->IsPlayingDemo())
	{
		pl.v_angle = ang;
	}
}

void C_BaseHLPlayer::CalcPortalView(Vector &eyeOrigin, QAngle &eyeAngles)
{
	//although we already ran CalcPlayerView which already did these copies, they also fudge these numbers in ways we don't like, so recopy
	VectorCopy(EyePosition(), eyeOrigin);
	VectorCopy(EyeAngles(), eyeAngles);

	//Re-apply the screenshake (we just stomped it)
	vieweffects->ApplyShake(eyeOrigin, eyeAngles, 1.0);

	C_Prop_Portal *pPortal = m_hPortalEnvironment.Get();
	assert(pPortal);

	C_Prop_Portal *pRemotePortal = pPortal->m_hLinkedPortal;
	if (!pRemotePortal)
	{
		return; //no hacks possible/necessary
	}

	Vector ptPortalCenter;
	Vector vPortalForward;

	ptPortalCenter = pPortal->GetNetworkOrigin();
	pPortal->GetVectors(&vPortalForward, NULL, NULL);
	float fPortalPlaneDist = vPortalForward.Dot(ptPortalCenter);

	bool bOverrideSpecialEffects = false; //sometimes to get the best effect we need to kill other effects that are simply for cleanliness

	float fEyeDist = vPortalForward.Dot(eyeOrigin) - fPortalPlaneDist;
	bool bTransformEye = false;
	if (fEyeDist < 0.0f) //eye behind portal
	{
		if (pPortal->m_PortalSimulator.EntityIsInPortalHole(this)) //player standing in portal
		{
			bTransformEye = true;
		}
		else if (vPortalForward.z < -0.01f) //there's a weird case where the player is ducking below a ceiling portal. As they unduck their eye moves beyond the portal before the code detects that they're in the portal hole.
		{
			Vector ptPlayerOrigin = GetAbsOrigin();
			float fOriginDist = vPortalForward.Dot(ptPlayerOrigin) - fPortalPlaneDist;

			if (fOriginDist > 0.0f)
			{
				float fInvTotalDist = 1.0f / (fOriginDist - fEyeDist); //fEyeDist is negative
				Vector ptPlaneIntersection = (eyeOrigin * fOriginDist * fInvTotalDist) - (ptPlayerOrigin * fEyeDist * fInvTotalDist);
				Assert(fabs(vPortalForward.Dot(ptPlaneIntersection) - fPortalPlaneDist) < 0.01f);

				Vector vIntersectionTest = ptPlaneIntersection - ptPortalCenter;

				Vector vPortalRight, vPortalUp;
				pPortal->GetVectors(NULL, &vPortalRight, &vPortalUp);

				if ((vIntersectionTest.Dot(vPortalRight) <= PORTAL_HALF_WIDTH) &&
					(vIntersectionTest.Dot(vPortalUp) <= PORTAL_HALF_HEIGHT))
				{
					bTransformEye = true;
				}
			}
		}
	}

	if (bTransformEye)
	{
		m_bEyePositionIsTransformedByPortal = true;

		//DevMsg( 2, "transforming portal view from <%f %f %f> <%f %f %f>\n", eyeOrigin.x, eyeOrigin.y, eyeOrigin.z, eyeAngles.x, eyeAngles.y, eyeAngles.z );

		VMatrix matThisToLinked = pPortal->MatrixThisToLinked();
		UTIL_Portal_PointTransform(matThisToLinked, eyeOrigin, eyeOrigin);
		UTIL_Portal_AngleTransform(matThisToLinked, eyeAngles, eyeAngles);

		//DevMsg( 2, "transforming portal view to   <%f %f %f> <%f %f %f>\n", eyeOrigin.x, eyeOrigin.y, eyeOrigin.z, eyeAngles.x, eyeAngles.y, eyeAngles.z );

		if (IsToolRecording())
		{
			static EntityTeleportedRecordingState_t state;

			KeyValues *msg = new KeyValues("entity_teleported");
			msg->SetPtr("state", &state);
			state.m_bTeleported = false;
			state.m_bViewOverride = true;
			state.m_vecTo = eyeOrigin;
			state.m_qaTo = eyeAngles;
			MatrixInvert(matThisToLinked.As3x4(), state.m_teleportMatrix);

			// Post a message back to all IToolSystems
			Assert((int)GetToolHandle() != 0);
			ToolFramework_PostToolMessage(GetToolHandle(), msg);

			msg->deleteThis();
		}

		bOverrideSpecialEffects = true;
	}
	else
	{
		m_bEyePositionIsTransformedByPortal = false;
	}

	if (bOverrideSpecialEffects)
	{
		m_iForceNoDrawInPortalSurface = ((pRemotePortal->m_bIsPortal2) ? (2) : (1));
		pRemotePortal->m_fStaticAmount = 0.0f;
	}
}
/*
extern float g_fMaxViewModelLag;
void C_BaseHLPlayer::CalcViewModelView(const Vector& eyeOrigin, const QAngle& eyeAngles)
{
	// HACK: Manually adjusting the eye position that view model looking up and down are similar
	// (solves view model "pop" on floor to floor transitions)
	Vector vInterpEyeOrigin = eyeOrigin;

	Vector vForward;
	Vector vRight;
	Vector vUp;
	AngleVectors(eyeAngles, &vForward, &vRight, &vUp);

	if (vForward.z < 0.0f)
	{
		float fT = vForward.z * vForward.z;
		vInterpEyeOrigin += vRight * (fT * 4.7f) + vForward * (fT * 5.0f) + vUp * (fT * 4.0f);
	}

	if (UTIL_IntersectEntityExtentsWithPortal(this))
		g_fMaxViewModelLag = 0.0f;
	else
		g_fMaxViewModelLag = 1.5f;

	for (int i = 0; i < MAX_VIEWMODELS; i++)
	{
		CBaseViewModel *vm = GetViewModel(i);
		if (!vm)
			continue;

		vm->CalcViewModelView(this, vInterpEyeOrigin, eyeAngles);
	}
}
*/
bool LocalPlayerIsCloseToPortal(void)
{
	C_BaseHLPlayer *player = C_BaseHLPlayer::GetLocalPlayer();
	if (player == NULL) return false;
	return C_BaseHLPlayer::GetLocalPlayer()->IsCloseToPortal();
}
#endif // PORTAL_COMPILE
