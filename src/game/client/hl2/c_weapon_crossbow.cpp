//========================================================================//
//
// Purpose: 
//
//=============================================================================//

#include "cbase.h"
#include "model_types.h"
#include "clienteffectprecachesystem.h"
#ifdef DARKINTERVAL
#include "c_basehlcombatweapon.h"
#include "c_weapon__stubs.h"
#include "dlight.h"
#include "iefx.h"
#endif
#include "fx.h"
#include "c_te_effect_dispatch.h"
#include "beamdraw.h"
#ifdef DARKINTERVAL
#include "debugoverlay_shared.h"
#include "materialsystem/imaterialproxy.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "functionproxy.h"
#include <keyvalues.h>
#include "mathlib/vmatrix.h"
#include "toolframework_client.h"
#endif
CLIENTEFFECT_REGISTER_BEGIN( PrecacheEffectCrossbow )
CLIENTEFFECT_MATERIAL( "effects/muzzleflash1" )
CLIENTEFFECT_REGISTER_END()

//
// Crossbow bolt
//

class C_CrossbowBolt : public C_BaseCombatCharacter
{
	DECLARE_CLASS( C_CrossbowBolt, C_BaseCombatCharacter );
	DECLARE_CLIENTCLASS();
public:
	
	C_CrossbowBolt( void );

	virtual RenderGroup_t GetRenderGroup( void )
	{
		// We want to draw translucent bits as well as our main model
		return RENDER_GROUP_TWOPASS;
	}

	virtual void	ClientThink( void );

	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual int		DrawModel( int flags );

private:

	C_CrossbowBolt( const C_CrossbowBolt & ); // not defined, not accessible

	Vector	m_vecLastOrigin;
	bool	m_bUpdated;
};

IMPLEMENT_CLIENTCLASS_DT( C_CrossbowBolt, DT_CrossbowBolt, CCrossbowBolt )
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_CrossbowBolt::C_CrossbowBolt( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_CrossbowBolt::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		m_bUpdated = false;
		m_vecLastOrigin = GetAbsOrigin();
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flags - 
// Output : int
//-----------------------------------------------------------------------------
int C_CrossbowBolt::DrawModel( int flags )
{
#ifndef DARKINTERVAL // I think this was taken out because it was deemed wholly unnecessary...
	// See if we're drawing the motion blur
	if ( flags & STUDIO_TRANSPARENCY )
	{
		float		color[3];
		IMaterial	*pBlurMaterial = materials->FindMaterial( "effects/muzzleflash1", NULL, false );

		Vector	vecDir = GetAbsOrigin() - m_vecLastOrigin;
		float	speed = VectorNormalize( vecDir );
		
		speed = clamp( speed, 0, 32 );
		
		if ( speed > 0 )
		{
			float	stepSize = MIN( ( speed * 0.5f ), 4.0f );

			Vector	spawnPos = GetAbsOrigin() + ( vecDir * 24.0f );
			Vector	spawnStep = -vecDir * stepSize;

			CMatRenderContextPtr pRenderContext( materials );
			pRenderContext->Bind( pBlurMaterial );

			float	alpha;

			// Draw the motion blurred trail
			for ( int i = 0; i < 20; i++ )
			{
				spawnPos += spawnStep;

				alpha = RemapValClamped( i, 5, 11, 0.25f, 0.05f );

				color[0] = color[1] = color[2] = alpha;

				DrawHalo( pBlurMaterial, spawnPos, 3.0f, color );
			}
		}

		if ( gpGlobals->frametime > 0.0f && !m_bUpdated)
		{
			m_bUpdated = true;
			m_vecLastOrigin = GetAbsOrigin();
		}

		return 1;
	}
#endif
	// Draw the normal portion
	return BaseClass::DrawModel( flags );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_CrossbowBolt::ClientThink( void )
{
	m_bUpdated = false;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : &data - 
//-----------------------------------------------------------------------------
void CrosshairLoadCallback( const CEffectData &data )
{
	IClientRenderable *pRenderable = data.GetRenderable( );
	if ( !pRenderable )
		return;
	
	Vector	position;
	QAngle	angles;

	// If we found the attachment, emit sparks there
	if ( pRenderable->GetAttachment( data.m_nAttachmentIndex, position, angles ) )
	{
		FX_ElectricSpark( position, 1.0f, 1.0f, NULL );
	}
}

DECLARE_CLIENT_EFFECT( "CrossbowLoad", CrosshairLoadCallback );

#ifdef DARKINTERVAL // the crossbow itself utilises client code for dynamic lighting and viewmodel proxy
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
//ConVar crossbow_offset_front("crossbow_offset_front", 0);
//ConVar crossbow_offset_right("crossbow_offset_right", 0);
//ConVar crossbow_offset_up("crossbow_offset_up", 0);
class C_WeaponCrossbow : public C_BaseHLCombatWeapon
{
	DECLARE_CLASS(C_WeaponCrossbow, C_BaseHLCombatWeapon);
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
public:
	C_WeaponCrossbow(void);

	virtual void			ClientThink(void);
	virtual void			OnDataChanged(DataUpdateType_t updateType);
	virtual int				DrawModel(int flags) { return BaseClass::DrawModel(flags); }

	float					m_heating_amount_float;

	float GetGoalValueForControlledVariable() { return m_heating_amount_float; }
private:
	C_WeaponCrossbow(const C_WeaponCrossbow &);
};

STUB_WEAPON_CLASS_IMPLEMENT(weapon_crossbow, C_WeaponCrossbow);

IMPLEMENT_CLIENTCLASS_DT(C_WeaponCrossbow, DT_WeaponCrossbow, CWeaponCrossbow)
RecvPropFloat(RECVINFO(m_heating_amount_float)),
END_RECV_TABLE()

C_WeaponCrossbow::C_WeaponCrossbow(void)
{
	m_heating_amount_float = 0.0f;
}

void C_WeaponCrossbow::OnDataChanged(DataUpdateType_t updateType)
{
	if (m_heating_amount_float > 0.00f)
	{
		dlight_t *dl;

		C_BasePlayer *poop = ToBasePlayer(GetOwner());

		if (poop)
		{
			C_BaseViewModel *crap = dynamic_cast<CBaseViewModel *>(poop->GetRenderedWeaponModel());

			if (crap)
			{
				//		int a = crap->LookupBone("Crossbow_model.bolt"); // LookupAttachment("bolt_light");
				//		Vector vAttachment;
				//		QAngle dummyAngles;
				//		GetAttachment(a, vAttachment, dummyAngles);

				Vector	vecEye = poop->EyePosition();
				Vector	vForward, vRight, vUp;

				poop->EyeVectors(&vForward, &vRight, &vUp);
				Vector vecSrc = vecEye
					+ vForward * 24 /*crossbow_offset_front.GetInt()*/
					+ vRight * 3 /*crossbow_offset_right.GetInt()*/
					+ vUp * -6 /*crossbow_offset_up.GetInt()*/;

				//	NDebugOverlay::Cross3D(vecSrc, 4, 25, 255, 25, true, 0.025f);

				dl = effects->CL_AllocDlight(crap->GetModelIndex());
				dl->origin = vecSrc;
				dl->color.r = 120;
				dl->color.g = 20;
				dl->color.b = 10;
				dl->color.exponent = 5 * m_heating_amount_float;
				dl->radius = random->RandomFloat(100, 120);
				dl->die = CURTIME + 0.01;

				//		el = effects->CL_AllocElight(crap->GetModelIndex());
				//		el->origin = vecSrc;
				//		el->color.r = 120;
				//		el->color.g = 20;
				//		el->color.b = 10;
				//		dl->color.exponent = 2.5 * m_heating_amount_float;
				//		el->radius = random->RandomFloat(100, 120);
				//		el->die = CURTIME + 0.001;
				return;
			}
		}
	}
	BaseClass::OnDataChanged(updateType);
	SetNextClientThink(CLIENT_THINK_ALWAYS);
}

void C_WeaponCrossbow::ClientThink(void)
{
	BaseClass::ClientThink();
}
#endif