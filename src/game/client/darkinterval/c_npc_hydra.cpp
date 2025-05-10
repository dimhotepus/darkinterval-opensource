#include "cbase.h"
#include "bone_setup.h"
#include "c_ai_basenpc.h"
#include "dlight.h"
#include "iefx.h"
#include "proxyentity.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "functionproxy.h"
#include "engine/ivdebugoverlay.h"
#include "tier0/vprof.h"
#include "soundinfo.h"
#include "object_motion_blur_effect.h"
#include "c_te_effect_dispatch.h"
#include "toolframework_client.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//static ConVar hydra_anim_bone_root("hydra_anim_bone_root", "1");
//static ConVar hydra_anim_bone_start("hydra_anim_bone_start", "0", 0, "If >0, will restrict hydra animation lerping to only bones starting with this number");
//static ConVar hydra_anim_bone_end("hydra_anim_bone_end", "48", 0, "If >0, will restrict hydra animation lerping to only bones up to this number");
static ConVar hydra_anim_lerp_time("hydra_anim_lerp_time", "1.0f");
static ConVar hydra_bone_chain_scale("hydra_bone_chain_scale", "1.0f", FCVAR_ARCHIVE, "");
ConVar hydra_interpolation_amount("hydra_interpolation_amount", "0.2");

class C_HydraDummy : public C_AI_BaseNPC
{
	DECLARE_CLASS(C_HydraDummy, C_AI_BaseNPC);
public:

	DECLARE_CLIENTCLASS();
	DECLARE_INTERPOLATION();

	C_HydraDummy();
	virtual			~C_HydraDummy();

	virtual void	OnDataChanged(DataUpdateType_t updateType);
	virtual void	GetRenderBounds(Vector& theMins, Vector& theMaxs);
	virtual void	ClientThink();
	void			PostDataUpdate(DataUpdateType_t updateType);

	virtual	void	OnLatchInterpolatedVariables(int flags);
	virtual bool	SetupBones(matrix3x4_t *pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime);
	virtual	void	StandardBlendingRules(CStudioHdr *hdr, Vector pos[], Quaternion q[], float currentTime, int boneMask);

	void			CalcBoneChain(Vector pos[], const Vector chain[]);
	void			CalcBoneAngles(const Vector pos[], Quaternion q[]);

	virtual bool	TestCollision(const Ray_t &ray, unsigned int fContentsMask, trace_t& tr);
	virtual bool	TestHitboxes(const Ray_t &ray, unsigned int fContentsMask, trace_t& tr);

	virtual bool	GetSoundSpatialization(SpatializationInfo_t& info);

	virtual void	ResetLatched();
	
#define	CHAIN_LINKS 32 //16

	bool			m_bNewChain;
	int				m_fLatchFlags;
	Vector			m_vecChain[CHAIN_LINKS];
	Vector			m_vecRoot;
	Vector			m_vecHeadDir;
//	Vector			m_vecHeadLight;
	float			m_lengthRelaxed;
	bool			m_suppressProcedural;
	float			m_proxy_time_to_attack_float;
	float			m_proxy_material_intensity_float;
	float			m_anim_lerp_start_time_float;
	float			m_anim_lerp_end_time_float;

	CInterpolatedVar<Vector>	m_iv_vecHeadDir;

	Vector			*m_vecPos;
	CInterpolatedVar<Vector>	*m_iv_vecPos;

	int				m_numHydraBones;
	float			*m_boneLength;

	float			m_maxPossibleLength;

private:
	C_HydraDummy(const C_HydraDummy &);	
	Vector		m_vecRenderMins;
	Vector		m_vecRenderMaxs;
};

IMPLEMENT_CLIENTCLASS_DT(C_HydraDummy, DT_hydraDummy, hydraDummy)
RecvPropArray(RecvPropVector(RECVINFO(m_vecChain[0])), m_vecChain),
RecvPropVector(RECVINFO(m_vecRoot)),
RecvPropVector(RECVINFO(m_vecHeadDir)),
RecvPropFloat(RECVINFO(m_lengthRelaxed)),
RecvPropFloat(RECVINFO(m_anim_lerp_start_time_float)),
RecvPropFloat(RECVINFO(m_anim_lerp_end_time_float)),
RecvPropBool(RECVINFO(m_suppressProcedural)), // tell the model to animate traditionally instead of calculating bone setups
RecvPropFloat(RECVINFO(m_proxy_time_to_attack_float)),
RecvPropFloat(RECVINFO(m_proxy_material_intensity_float)),
END_RECV_TABLE()

C_HydraDummy::C_HydraDummy() : m_iv_vecHeadDir("C_HydraDummy::m_iv_vecHeadDir")
{
	AddVar(&m_vecHeadDir, &m_iv_vecHeadDir, LATCH_SIMULATION_VAR);

	m_numHydraBones = 0;
	m_boneLength = NULL;
	m_maxPossibleLength = 1;
	m_vecPos = NULL;
	m_iv_vecPos = NULL;
	m_anim_lerp_start_time_float = 0;
	m_anim_lerp_end_time_float = 0;
}

C_HydraDummy::~C_HydraDummy()
{
	delete m_boneLength;
	delete m_vecPos;
	delete[] m_iv_vecPos;
	m_iv_vecPos = NULL;
}

void C_HydraDummy::OnDataChanged(DataUpdateType_t updateType)
{
	if (updateType == DATA_UPDATE_CREATED )
	{
		AddSolidFlags(FSOLID_CUSTOMBOXTEST | FSOLID_CUSTOMRAYTEST);
		// We need to have our render bounds defined or shadow creation won't work correctly
		ClientThink();
		ClientThinkList()->SetNextClientThink(GetClientHandle(), CLIENT_THINK_ALWAYS);
	}
	
	BaseClass::OnDataChanged(updateType);
}

void C_HydraDummy::ClientThink()
{
#if 0
	for (int i = 0; i < m_numHydraBones; i++)
	{
		CEffectData data;
		data.m_nMaterial = reinterpret_cast< int >(this);
		data.m_nHitBox = (i << 8);
		data.m_flScale = 0.5;
		data.m_vOrigin = m_vecPos[i];
		DispatchEffect("FX_HydraCordLight", data);
	}
#endif
	if (m_suppressProcedural)
	{
		BaseClass::ClientThink();
		return;
	}

	ComputeEntitySpaceHitboxSurroundingBox(&m_vecRenderMins, &m_vecRenderMaxs);
	
	// True argument because the origin may have stayed the same, but the size is expected to always change
	g_pClientShadowMgr->AddToDirtyShadowList(this, true);
	
	InvalidateBoneCache();

//	GetBaseAnimating()->DrawClientHitboxes(0.1f);
}

void C_HydraDummy::PostDataUpdate(DataUpdateType_t updateType)
{
	BaseClass::PostDataUpdate(updateType);
	
	CalcBoneChain(m_vecPos, m_vecChain);
}

void C_HydraDummy::GetRenderBounds(Vector& theMins, Vector& theMaxs)
{
	theMins = m_vecRenderMins;
	theMaxs = m_vecRenderMaxs;
}

void C_HydraDummy::OnLatchInterpolatedVariables(int flags)
{
	m_bNewChain = true;
	m_fLatchFlags = flags;

	BaseClass::OnLatchInterpolatedVariables(flags);
}

void C_HydraDummy::ResetLatched()
{
	for (int i = 0; i < m_numHydraBones; i++)
	{
		m_iv_vecPos[i].Reset();
	}

	BaseClass::ResetLatched();
}

bool C_HydraDummy::SetupBones(matrix3x4_t *pBoneToWorldOut, int nMaxBones, int boneMask, float currentTime)
{
	return BaseClass::SetupBones(pBoneToWorldOut, nMaxBones, boneMask, currentTime);
}

#define HYDRA_ANIM_LERP_TIME hydra_anim_lerp_time.GetFloat()
void C_HydraDummy::StandardBlendingRules(CStudioHdr *hdr, Vector pos[], Quaternion q[], float currentTime, int boneMask)
{
	BaseClass::StandardBlendingRules(hdr, pos, q, currentTime, boneMask);
	
	if (m_suppressProcedural) return;

	VPROF("C_HydraDummy::StandardBlendingRules");

	if (!hdr)
		return;

	int i;

	bool bNewlyInited = false;

	if (m_numHydraBones != hdr->numbones())
	{
		m_numHydraBones = hdr->numbones();

		float poseparam[MAXSTUDIOPOSEPARAM];
		for (i = 0; i < hdr->GetNumPoseParameters(); i++)
		{
			poseparam[i] = 0;
		}

		//CalcPose(hdr, NULL, pos, q, 0.0f, 0.0f, poseparam, BONE_USED_BY_ANYTHING);

		if (m_boneLength)
		{
			delete[] m_boneLength;
		}

		m_boneLength = new float[m_numHydraBones];

		if (m_vecPos)
		{
			delete[] m_vecPos;
		}

		m_vecPos = new Vector[m_numHydraBones];

		if (m_iv_vecPos)
		{
			delete m_iv_vecPos;
		}

		m_iv_vecPos = new CInterpolatedVar<Vector>[m_numHydraBones];

		for (i = 0; i < m_numHydraBones; i++)
		{
			m_iv_vecPos[i].Setup(&m_vecPos[i], LATCH_SIMULATION_VAR /*| INTERPOLATE_LINEAR_ONLY*/ | EXCLUDE_AUTO_LATCH | EXCLUDE_AUTO_INTERPOLATE);
		}

		m_maxPossibleLength = 0;

		for (i = 0; i < m_numHydraBones - 1; i++)
		{
			m_boneLength[i] = (pos[i + 1] - pos[i]).Length();
			m_maxPossibleLength += m_boneLength[i];
		}

		m_boneLength[i] = 0.0f;

		bNewlyInited = true;

	}

	if (m_bNewChain)
	{
		CalcBoneChain(m_vecPos, m_vecChain);
		for (i = 0; i < m_numHydraBones; i++)
		{
			m_vecPos[i] = m_vecPos[i] - GetAbsOrigin();

			if (m_fLatchFlags & LATCH_SIMULATION_VAR)
			{
				m_iv_vecPos[i].NoteChanged(currentTime, true);
			}
		}

		m_bNewChain = false;
	}

	if (bNewlyInited)
	{

		for (i = 0; i < m_numHydraBones; i++)
		{
			m_iv_vecPos[i].Reset();
		}
	}

	for (i = 0; i < m_numHydraBones; i++)
	{
		if (i == 0)
		{
			pos[i] = vec3_origin; // we want the root to always stay attached
			continue;
		}

		m_iv_vecPos[i].Interpolate(currentTime, hydra_interpolation_amount.GetFloat());

		if ((m_anim_lerp_start_time_float + HYDRA_ANIM_LERP_TIME > CURTIME/* && i <= hydra_anim_bone_end.GetInt()*/))
		{
		//	if (i > hydra_anim_bone_start.GetInt() || i < hydra_anim_bone_end.GetInt())
		//	{
		//		pos[i] = m_vecPos[i];
		//	}
		//	else
			{
				pos[i] = VectorLerp(pos[i], m_vecPos[i], (m_anim_lerp_start_time_float + HYDRA_ANIM_LERP_TIME) - CURTIME);
			}
		}
		else
		{
			if (CURTIME >= m_anim_lerp_end_time_float /*|| i > hydra_anim_bone_end.GetInt()*/)
			{
				pos[i] = m_vecPos[i];
			}
			else
			{
				if (m_anim_lerp_end_time_float - CURTIME < HYDRA_ANIM_LERP_TIME)
				{
				//	if (i > hydra_anim_bone_start.GetInt() || i < hydra_anim_bone_end.GetInt())
				//	{
				//		pos[i] = m_vecPos[i];
				//	}
				//	else
					{
						pos[i] = VectorLerp(m_vecPos[i], pos[i], m_anim_lerp_end_time_float - CURTIME);
					}
				}
			}
		}
	}

	CalcBoneAngles(pos, q);

	Quaternion qTmp;
	AngleQuaternion(QAngle(0, -90, 0), qTmp);
	QuaternionMult(q[m_numHydraBones - 1], qTmp, q[m_numHydraBones - 1]);
}

void  C_HydraDummy::CalcBoneChain(Vector pos[], const Vector chain[])
{
	int i, j;

	i = CHAIN_LINKS - 1;
	while (i > 0)
	{
		if ((chain[i] - chain[i - 1]).LengthSqr() > 0.0)
		{
			break;
		}

		i--;
	}

	j = m_numHydraBones - 1;

	float totalLength = 0;

	for (int k = i; k > 0; k--)
	{
		totalLength += (chain[k] - chain[k - 1]).Length();
	}

	totalLength = clamp(totalLength, 1.0, m_maxPossibleLength);

	float scale = m_lengthRelaxed / totalLength * hydra_bone_chain_scale.GetFloat();

	float dist = 0;

	while (j >= 0 && i > 0)
	{
		float dt = (chain[i] - chain[i - 1]).Length() * scale;
		float dx = dt;
		while (j >= 0 && dist + dt >= m_boneLength[j])
		{
			float s = (dx - (dt - (m_boneLength[j] - dist))) / dx;

			if (s < 0 || s > 1.) s = 0;
									
			Catmull_Rom_Spline(chain[(i < CHAIN_LINKS - 1) ? i + 1 : CHAIN_LINKS - 1], chain[i], chain[(i > 0) ? i - 1 : 0], chain[(i > 1) ? i - 2 : 0], s, pos[j]);

			dt = dt - (m_boneLength[j] - dist);
			j--;
			dist = 0;
		}
		dist += dt;
		i--;
	}

	while (j >= 0)
	{
		pos[j] = chain[0];
		j--;
	}
}

void C_HydraDummy::CalcBoneAngles(const Vector pos[], Quaternion q[])
{
	int i;
	matrix3x4_t bonematrix;

	for (i = m_numHydraBones - 1; i >= 0; i--)
	{
		Vector forward;
		Vector left2;

		if (i != m_numHydraBones - 1)
		{
			QuaternionMatrix(q[i + 1], bonematrix);
			MatrixGetColumn(bonematrix, 1, left2);

			forward = (pos[i + 1] - pos[i]);
			float length = VectorNormalize(forward);
			if (length == 0.0)
			{
				q[i] = q[i + 1];
				continue;
			}
		}
		else
		{
			forward = m_vecHeadDir;
			VectorNormalize(forward);

			VectorMatrix(forward, bonematrix);
			MatrixGetColumn(bonematrix, 1, left2);
		}

		Vector up = CrossProduct(forward, left2);
		VectorNormalize(up);

		Vector left = CrossProduct(up, forward);

		MatrixSetColumn(forward, 0, bonematrix);
		MatrixSetColumn(left, 1, bonematrix);
		MatrixSetColumn(up, 2, bonematrix);

		QAngle angles;
		MatrixAngles(bonematrix, angles);
		AngleQuaternion(angles, q[i]);
	}
}

bool C_HydraDummy::TestCollision(const Ray_t &ray, unsigned int fContentsMask, trace_t& tr)
{
	if (ray.m_IsRay && IsSolidFlagSet(FSOLID_CUSTOMRAYTEST))
	{
		if (!TestHitboxes(ray, fContentsMask, tr))
			return true;

		return tr.DidHit();
	}

	if (!ray.m_IsRay && IsSolidFlagSet(FSOLID_CUSTOMBOXTEST))
	{
		if (!TestHitboxes(ray, fContentsMask, tr))
			return true;

		return true;
	}

	// We shouldn't get here.
	Assert(0);
	return false;
}

bool C_HydraDummy::TestHitboxes(const Ray_t &ray, unsigned int fContentsMask, trace_t& tr)
{
	VPROF("C_HydraDummy::TestHitboxes");

	MDLCACHE_CRITICAL_SECTION();

	CStudioHdr *pStudioHdr = GetModelPtr();
	if (!pStudioHdr)
		return false;

	mstudiohitboxset_t *set = pStudioHdr->pHitboxSet(m_nHitboxSet);
	if (!set || !set->numhitboxes)
		return false;

	// Use vcollide for box traces.
	if (!ray.m_IsRay)
		return false;

	// This *has* to be true for the existing code to function correctly.
	Assert(ray.m_StartOffset == vec3_origin);

	CBoneCache *pCache = GetBoneCache(pStudioHdr);
	matrix3x4_t *hitboxbones[MAXSTUDIOBONES];
	pCache->ReadCachedBonePointers(hitboxbones, pStudioHdr->numbones());

	if (TraceToStudio(physprops, ray, pStudioHdr, set, hitboxbones, fContentsMask, GetRenderOrigin(), GetModelScale(), tr))
	{
		mstudiobbox_t *pbox = set->pHitbox(tr.hitbox);
		mstudiobone_t *pBone = pStudioHdr->pBone(pbox->bone);
		tr.surface.name = "**studio**";
		tr.surface.flags = SURF_HITBOX;
		tr.surface.surfaceProps = physprops->GetSurfaceIndex(pBone->pszSurfaceProp());
		if (IsRagdoll())
		{
			IPhysicsObject *pReplace = m_pRagdoll->GetElement(tr.physicsbone);
			if (pReplace)
			{
				VPhysicsSetObject(NULL);
				VPhysicsSetObject(pReplace);
			}
		}
	}

	return true;
}

bool C_HydraDummy::GetSoundSpatialization(SpatializationInfo_t& info)
{
	bool bret = BaseClass::GetSoundSpatialization(info);
	if (bret)
	{
	}

	return bret;
}

// forward declarations
class CHydraProxy1 : public CResultProxy
{
public:
	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	virtual void OnBind(void *pC_BaseEntity);
private:
	C_HydraDummy *BindArgToEntity(void *pArg);
};

C_HydraDummy *CHydraProxy1::BindArgToEntity(void *pArg)
{
	IClientRenderable *pRend = (IClientRenderable *)pArg;
	return (C_HydraDummy*)pRend->GetIClientUnknown()->GetBaseEntity();
}

bool CHydraProxy1::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	return CResultProxy::Init(pMaterial, pKeyValues);
}

void CHydraProxy1::OnBind(void *pC_BaseEntity)
{
	if (!pC_BaseEntity)
		return;

	C_BaseEntity *pEntity = BindArgToEntity(pC_BaseEntity);
	C_HydraDummy *pHydra = BindArgToEntity(pC_BaseEntity);
	if (!pHydra)
		return;

	float proxyvalue;
	if (pHydra)
		proxyvalue = pHydra->m_proxy_time_to_attack_float;
	else
		proxyvalue = 0;

	// This is actually a vector...
	Assert(m_pResult);
	m_pResult->SetFloatValue(proxyvalue);

	if (ToolsEnabled())
	{
		ToolFramework_RecordMaterialParams(GetMaterial());
	}
}

EXPOSE_INTERFACE(CHydraProxy1, IMaterialProxy, "Hydra_Pulsation" IMATERIAL_PROXY_INTERFACE_VERSION);

// forward declarations
class CHydraProxy2 : public CResultProxy
{
public:
	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	virtual void OnBind(void *pC_BaseEntity);
private:
	C_HydraDummy *BindArgToEntity(void *pArg);
};

C_HydraDummy *CHydraProxy2::BindArgToEntity(void *pArg)
{
	IClientRenderable *pRend = (IClientRenderable *)pArg;
	return (C_HydraDummy*)pRend->GetIClientUnknown()->GetBaseEntity();
}

bool CHydraProxy2::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	return CResultProxy::Init(pMaterial, pKeyValues);
}

void CHydraProxy2::OnBind(void *pC_BaseEntity)
{
	if (!pC_BaseEntity)
		return;

	C_BaseEntity *pEntity = BindArgToEntity(pC_BaseEntity);
	C_HydraDummy *pHydra = BindArgToEntity(pC_BaseEntity);
	if (!pHydra)
		return;
	
	float proxyvalue;
	if (pHydra)
		proxyvalue = (pHydra && pHydra->m_proxy_material_intensity_float);
	else
		proxyvalue = 0;

	// This is actually a vector...
	Assert(m_pResult);
	m_pResult->SetFloatValue(proxyvalue);

	if (ToolsEnabled())
	{
		ToolFramework_RecordMaterialParams(GetMaterial());
	}
}

EXPOSE_INTERFACE(CHydraProxy2, IMaterialProxy, "Hydra_Intensity" IMATERIAL_PROXY_INTERFACE_VERSION);

//-----------------------------------------------------------------------------
// Purpose: Small Hydra client side implementation for computing collision bounds
//-----------------------------------------------------------------------------
class C_SmallHydra : public C_AI_BaseNPC
{
public:

	DECLARE_CLASS(C_SmallHydra, C_AI_BaseNPC);
	DECLARE_CLIENTCLASS();

	C_SmallHydra(void);

	virtual void GetRenderBounds(Vector &theMins, Vector &theMaxs)
	{
		BaseClass::GetRenderBounds(theMins, theMaxs);
	}

#if 0
	// Purpose: Initialize absmin & absmax to the appropriate box
	virtual void ComputeWorldSpaceSurroundingBox(Vector *pVecWorldMins, Vector *pVecWorldMaxs)
	{
		CollisionProp()->WorldSpaceAABB(pVecWorldMins, pVecWorldMaxs);
		VectorMin(*pVecWorldMins, *pVecWorldMins, *pVecWorldMins);
		VectorMax(*pVecWorldMaxs, *pVecWorldMaxs, *pVecWorldMaxs);
	}
#endif

//	void	OnDataChanged(DataUpdateType_t updateType);
//	void	ClientThink(void);
//	void	StandardBlendingRules(CStudioHdr *pStudioHdr, Vector pos[], Quaternion q[], float currentTime, int boneMask);

	float	m_proxy_blood_visual_float;

private:
	CMotionBlurObject m_MotionBlurObject;
};

C_SmallHydra::C_SmallHydra() :
	m_MotionBlurObject(this, 10.0f)
{

}

IMPLEMENT_CLIENTCLASS_DT(C_SmallHydra, DT_SmallHydra, CSmallHydra)
RecvPropFloat(RECVINFO(m_proxy_blood_visual_float)),
END_RECV_TABLE()


// forward declarations
class CHydraProxy3 : public CResultProxy
{
public:
	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	virtual void OnBind(void *pC_BaseEntity);
private:
	C_SmallHydra *BindArgToEntity(void *pArg);
};

C_SmallHydra *CHydraProxy3::BindArgToEntity(void *pArg)
{
	IClientRenderable *pRend = (IClientRenderable *)pArg;
	return (C_SmallHydra*)pRend->GetIClientUnknown()->GetBaseEntity();
}

bool CHydraProxy3::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	return CResultProxy::Init(pMaterial, pKeyValues);
}

void CHydraProxy3::OnBind(void *pC_BaseEntity)
{
	if (!pC_BaseEntity)
		return;

	C_BaseEntity *pEntity = BindArgToEntity(pC_BaseEntity);
	C_SmallHydra *pHydra = BindArgToEntity(pC_BaseEntity);
	if (!pHydra)
		return;

	float proxyvalue;
	if (pHydra)
		proxyvalue = pHydra->m_proxy_blood_visual_float;
	else
		proxyvalue = 0;

	// This is actually a vector...
	Assert(m_pResult);
	m_pResult->SetFloatValue(proxyvalue);

	if (ToolsEnabled())
	{
		ToolFramework_RecordMaterialParams(GetMaterial());
	}
}

EXPOSE_INTERFACE(CHydraProxy3, IMaterialProxy, "Hydra_BloodVisual" IMATERIAL_PROXY_INTERFACE_VERSION);