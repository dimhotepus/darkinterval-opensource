//========================================================================//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "c_basehlcombatweapon.h"
#include "c_basehlplayer.h"
#include "clienteffectprecachesystem.h"
#include "c_te_effect_dispatch.h"
#include "c_weapon__stubs.h"
#include "dlight.h"
#include "iefx.h"

#include "particle_prototype.h"
#include "particle_util.h"
#include "baseparticleentity.h"
#include "clienteffectprecachesystem.h"
#include "fx.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"
/*
class C_ImmolatorFireStream : public C_BaseParticleEntity, public IPrototypeAppEffect
{
public:
	DECLARE_CLIENTCLASS();
	DECLARE_CLASS(C_ImmolatorFireStream, C_BaseParticleEntity);

	C_ImmolatorFireStream();
	~C_ImmolatorFireStream();

	class ImmolatorFireParticle : public Particle
	{
	public:
		Vector			m_Velocity;
		float			m_flRoll;
		float			m_flRollDelta;
		float			m_Lifetime;
		float			m_DieTime;
		unsigned char	m_uchStartSize;
		unsigned char	m_uchEndSize;
	};

	int IsEmissive(void) { return false; }

	//C_BaseEntity
public:

	virtual void	OnDataChanged(DataUpdateType_t updateType);


	//IPrototypeAppEffect
public:
	virtual void		Start(CParticleMgr *pParticleMgr, IPrototypeArgAccess *pArgs);
	virtual bool		GetPropEditInfo(RecvTable **ppTable, void **ppObj);


	//IParticleEffect
public:
	virtual void	Update(float fTimeDelta);
	virtual void RenderParticles(CParticleRenderIterator *pIterator);
	virtual void SimulateParticles(CParticleSimulateIterator *pIterator);


	//Stuff from the datatable
public:

	float			m_SpreadSpeed;
	float			m_Speed;
	float			m_StartSize;
	float			m_EndSize;
	float			m_Rate;
	float			m_JetLength;	// Length of the jet. Lifetime is derived from this.

	int				m_bEmit;		// Emit particles?
//	int				m_nType;		// Type of particles to emit
//	bool			m_bFaceLeft;	// For support of legacy env_steamjet entity, which faced left instead of forward.

	int				m_spawnflags;
	float			m_flRollSpeed;
	
private:

	float			m_Lifetime;		// Calculated from m_JetLength / m_Speed;

	CParticleMgr		*m_pParticleMgr;
	PMaterialHandle	m_MaterialHandle;
	TimedEvent		m_ParticleSpawn;

private:
	C_ImmolatorFireStream(const C_ImmolatorFireStream &);
};


// ------------------------------------------------------------------------- //
// Tables.
// ------------------------------------------------------------------------- //

// Expose to the particle app.
EXPOSE_PROTOTYPE_EFFECT(ImmolatorFireStream, C_ImmolatorFireStream);


// Datatable..
IMPLEMENT_CLIENTCLASS_DT(C_ImmolatorFireStream, DT_ImmolatorFireStream, CImmolatorFireStream)
RecvPropFloat(RECVINFO(m_SpreadSpeed), 0),
RecvPropFloat(RECVINFO(m_Speed), 0),
RecvPropFloat(RECVINFO(m_StartSize), 0),
RecvPropFloat(RECVINFO(m_EndSize), 0),
RecvPropFloat(RECVINFO(m_Rate), 0),
RecvPropFloat(RECVINFO(m_JetLength), 0),
RecvPropInt(RECVINFO(m_bEmit), 0),
RecvPropInt(RECVINFO(m_spawnflags)),
RecvPropFloat(RECVINFO(m_flRollSpeed), 0),
END_RECV_TABLE()

// ------------------------------------------------------------------------- //
// C_ImmolatorFireStream implementation.
// ------------------------------------------------------------------------- //
C_ImmolatorFireStream::C_ImmolatorFireStream()
{
	m_pParticleMgr = NULL;
	m_MaterialHandle = INVALID_MATERIAL_HANDLE;

	m_SpreadSpeed = 15;
	m_Speed = 120;
	m_StartSize = 10;
	m_EndSize = 25;
	m_Rate = 26;
	m_JetLength = 80;
	m_bEmit = true;
	m_ParticleEffect.SetAlwaysSimulate(false); // Don't simulate outside the PVS or frustum.
}

C_ImmolatorFireStream::~C_ImmolatorFireStream()
{
	if (m_pParticleMgr)
		m_pParticleMgr->RemoveEffect(&m_ParticleEffect);
}

void C_ImmolatorFireStream::OnDataChanged(DataUpdateType_t updateType)
{
	C_BaseEntity::OnDataChanged(updateType);

	if (updateType == DATA_UPDATE_CREATED)
	{
		Start(ParticleMgr(), NULL);
	}

	// Recalulate lifetime in case length or speed changed.
	m_Lifetime = m_JetLength / m_Speed;
	m_ParticleEffect.SetParticleCullRadius(MAX(m_StartSize, m_EndSize));
}

void C_ImmolatorFireStream::Start(CParticleMgr *pParticleMgr, IPrototypeArgAccess *pArgs)
{
	pParticleMgr->AddEffect(&m_ParticleEffect, this);

//	switch (m_nType)
//	{
		m_MaterialHandle = g_Mat_DustPuff[0];
//		break;
//	}

	m_ParticleSpawn.Init(m_Rate);
	m_Lifetime = m_JetLength / m_Speed;
	m_pParticleMgr = pParticleMgr;
}

bool C_ImmolatorFireStream::GetPropEditInfo(RecvTable **ppTable, void **ppObj)
{
	*ppTable = &REFERENCE_RECV_TABLE(DT_ImmolatorFireStream);
	*ppObj = this;
	return true;
}

void C_ImmolatorFireStream::Update(float fTimeDelta)
{
	if (!m_pParticleMgr)
	{
		assert(false);
		return;
	}

	if (m_bEmit)
	{
		// Add new particles.
		int nToEmit = 0;
		float tempDelta = fTimeDelta;
		while (m_ParticleSpawn.NextEvent(tempDelta))
			++nToEmit;

		if (nToEmit > 0)
		{
			Vector forward, right, up;
			AngleVectors(GetAbsAngles(), &forward, &right, &up);
			
			// EVIL: Ideally, we could tell the renderer our OBB, and let it build a big box that encloses
			// the entity with its parent so it doesn't have to setup its parent's bones here.
			Vector vEndPoint = GetAbsOrigin() + forward * m_Speed;
			Vector vMin, vMax;
			VectorMin(GetAbsOrigin(), vEndPoint, vMin);
			VectorMax(GetAbsOrigin(), vEndPoint, vMax);
			m_ParticleEffect.SetBBox(vMin, vMax);

			if (m_ParticleEffect.WasDrawnPrevFrame())
			{
				while (nToEmit--)
				{
					// Make a new particle.
					if (ImmolatorFireParticle *pParticle = (ImmolatorFireParticle*)m_ParticleEffect.AddParticle(sizeof(ImmolatorFireParticle), m_MaterialHandle))
					{
						pParticle->m_Pos = GetAbsOrigin();

						pParticle->m_Velocity =
							FRand(-m_SpreadSpeed, m_SpreadSpeed) * right +
							FRand(-m_SpreadSpeed, m_SpreadSpeed) * up +
							m_Speed * forward;

						pParticle->m_Lifetime = 0;
						pParticle->m_DieTime = m_Lifetime;

						pParticle->m_uchStartSize = m_StartSize;
						pParticle->m_uchEndSize = m_EndSize;

						pParticle->m_flRoll = random->RandomFloat(0, 360);
						pParticle->m_flRollDelta = random->RandomFloat(-m_flRollSpeed, m_flRollSpeed);
					}
				}
			}
		}
	}
}

// Render a quad on the screen where you pass in color and size.
// Normal is random and "flutters"
inline void RenderParticle_ColorSizePerturbNormal(
	ParticleDraw* pDraw,
	const Vector &pos,
	const Vector &color,
	const float alpha,
	const float size
)
{
	// Don't render totally transparent particles.
	if (alpha < 0.001f)
		return;

	CMeshBuilder *pBuilder = pDraw->GetMeshBuilder();
	if (!pBuilder)
		return;

	unsigned char ubColor[4];
	ubColor[0] = (unsigned char)RoundFloatToInt(color.x * 254.9f);
	ubColor[1] = (unsigned char)RoundFloatToInt(color.y * 254.9f);
	ubColor[2] = (unsigned char)RoundFloatToInt(color.z * 254.9f);
	ubColor[3] = (unsigned char)RoundFloatToInt(alpha * 254.9f);

	Vector vNorm;

	vNorm.Random(-1.0f, 1.0f);

	// Add the 4 corner vertices.
	pBuilder->Position3f(pos.x - size, pos.y - size, pos.z);
	pBuilder->Color4ubv(ubColor);
	pBuilder->Normal3fv(vNorm.Base());
	pBuilder->TexCoord2f(0, 0, 1.0f);
	pBuilder->AdvanceVertex();

	pBuilder->Position3f(pos.x - size, pos.y + size, pos.z);
	pBuilder->Color4ubv(ubColor);
	pBuilder->Normal3fv(vNorm.Base());
	pBuilder->TexCoord2f(0, 0, 0);
	pBuilder->AdvanceVertex();

	pBuilder->Position3f(pos.x + size, pos.y + size, pos.z);
	pBuilder->Color4ubv(ubColor);
	pBuilder->Normal3fv(vNorm.Base());
	pBuilder->TexCoord2f(0, 1.0f, 0);
	pBuilder->AdvanceVertex();

	pBuilder->Position3f(pos.x + size, pos.y - size, pos.z);
	pBuilder->Color4ubv(ubColor);
	pBuilder->Normal3fv(vNorm.Base());
	pBuilder->TexCoord2f(0, 1.0f, 1.0f);
	pBuilder->AdvanceVertex();
}


void C_ImmolatorFireStream::RenderParticles(CParticleRenderIterator *pIterator)
{
	const ImmolatorFireParticle *pParticle = (const ImmolatorFireParticle*)pIterator->GetFirst();
	while (pParticle)
	{
		// Render.
		Vector tPos;
		TransformParticle(m_pParticleMgr->GetModelView(), pParticle->m_Pos, tPos);
		float sortKey = tPos.z;

		float lifetimeT = pParticle->m_Lifetime / (pParticle->m_DieTime + 0.001);
		float fRamp = lifetimeT * (5 - 1);
		int iRamp = (int)fRamp;
		float fraction = fRamp - iRamp;

	//	Vector vRampColor = m_Ramps[iRamp] + (m_Ramps[iRamp + 1] - m_Ramps[iRamp]) * fraction;

	//	vRampColor[0] = MIN(1.0f, vRampColor[0]);
	//	vRampColor[1] = MIN(1.0f, vRampColor[1]);
	//	vRampColor[2] = MIN(1.0f, vRampColor[2]);

		float sinLifetime = sin(pParticle->m_Lifetime * 3.14159f / pParticle->m_DieTime);

	
		RenderParticle_ColorSizeAngle(
			pIterator->GetParticleDraw(),
			tPos,
			Vector(1,1,1),
			sinLifetime * (m_clrRender->a / 255.0f),
			FLerp(pParticle->m_uchStartSize, pParticle->m_uchEndSize, pParticle->m_Lifetime),
			pParticle->m_flRoll);

		pParticle = (const ImmolatorFireParticle*)pIterator->GetNext(sortKey);
	}
}


void C_ImmolatorFireStream::SimulateParticles(CParticleSimulateIterator *pIterator)
{
	//Don't simulate if we're emiting particles...
	//This fixes the cases where looking away from a steam jet and then looking back would cause a break on the stream.
	if (m_ParticleEffect.WasDrawnPrevFrame() == false && m_bEmit)
		return;

	ImmolatorFireParticle *pParticle = (ImmolatorFireParticle*)pIterator->GetFirst();
	while (pParticle)
	{
		// Should this particle die?
		pParticle->m_Lifetime += pIterator->GetTimeDelta();

		if (pParticle->m_Lifetime > pParticle->m_DieTime)
		{
			pIterator->RemoveParticle(pParticle);
		}
		else
		{
			pParticle->m_flRoll += pParticle->m_flRollDelta * pIterator->GetTimeDelta();
			pParticle->m_Pos = pParticle->m_Pos + pParticle->m_Velocity * pIterator->GetTimeDelta();
		}

		pParticle = (ImmolatorFireParticle*)pIterator->GetNext();
	}
}
*/
class C_WeaponImmolator : public C_BaseHLCombatWeapon
{
	DECLARE_CLASS(C_WeaponImmolator, C_BaseHLCombatWeapon);
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();
public:
	C_WeaponImmolator();
	~C_WeaponImmolator();

	virtual void			ClientThink(void);
	virtual void			ReceiveMessage(int classID, bf_read &msg);
	virtual void			OnDataChanged(DataUpdateType_t updateType);
	virtual int				DrawModel(int flags) {	return BaseClass::DrawModel(flags);	}

protected:
	CNetworkVar(bool, m_bLight);
	CNewParticleEffect *m_hEffect;
private:
	C_WeaponImmolator(const C_WeaponImmolator &);
};

STUB_WEAPON_CLASS_IMPLEMENT(weapon_immolator, C_WeaponImmolator);

IMPLEMENT_CLIENTCLASS_DT(C_WeaponImmolator, DT_WeaponImmolator, CWeaponImmolator)
RecvPropInt(RECVINFO(m_bLight), 0),
END_RECV_TABLE()

C_WeaponImmolator::C_WeaponImmolator()
{
	m_bLight = false;
}
C_WeaponImmolator::~C_WeaponImmolator()
{
	if (m_hEffect)
	{
		m_hEffect->StopEmission(false, false, true);
		m_hEffect = NULL;
	}
}

void C_WeaponImmolator::ClientThink(void)
{
	if (m_bLight)
	{
		dlight_t *dl;
		dlight_t *el;

		int a = LookupAttachment("light");
		Vector vAttachment;
		QAngle dummyAngles;
		GetAttachment(a, vAttachment, dummyAngles);

		dl = effects->CL_AllocDlight(index);
		dl->origin = vAttachment;
		dl->color.r = 50;
		dl->color.g = 250;
		dl->color.b = 140;
		dl->color.exponent = 5;
		dl->radius = random->RandomFloat(128,160);
		dl->die = CURTIME + 0.001;

		el = effects->CL_AllocElight(index);
		dl->origin = vAttachment;
		el->color.r = 50;
		el->color.g = 250;
		el->color.b = 70;
		el->color.exponent = 2.5;
		el->radius = random->RandomFloat(128,160);
		el->die = CURTIME + 0.001;

		return;
	}

	BaseClass::ClientThink();
}

void C_WeaponImmolator::ReceiveMessage(int classID, bf_read &msg)
{
	int messageType = msg.ReadByte();
	switch (messageType)
	{
	case 0:
	{
	}
	break;
	case 1:
	{
	}
	break;
	default:
		AssertMsg1(false, "Received unknown message %d", messageType);
	}
}


//-----------------------------------------------------------------------------
// Purpose: Starts the client-side version thinking
//-----------------------------------------------------------------------------
void C_WeaponImmolator::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);
	if (updateType == DATA_UPDATE_CREATED)
	{
		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}
}
