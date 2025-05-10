#ifndef RAGDOLL_SERVERSIDE_H
#define RAGDOLL_SERVERSIDE_H

#ifdef _WIN32
#pragma once
#endif
#include "c_baseanimating.h"
#include "ragdoll_shared.h"
#include "mathlib/vmatrix.h"
#include "bone_setup.h"
#include "materialsystem/imesh.h"
#include "engine/ivmodelinfo.h"
#include "iviewrender.h"
#include "tier0/vprof.h"
#include "view.h"
#include "physics_saverestore.h"
#include "vphysics/constraints.h"
//-----------------------------------------------------------------------------
// Purpose: Server Ragdoll class
//-----------------------------------------------------------------------------
class C_ServerRagdoll : public C_BaseAnimating
{
public:
	DECLARE_CLASS(C_ServerRagdoll, C_BaseAnimating);
	DECLARE_CLIENTCLASS();
	DECLARE_INTERPOLATION();

	C_ServerRagdoll(void);

	virtual void PostDataUpdate(DataUpdateType_t updateType);

	virtual int InternalDrawModel(int flags);
	virtual CStudioHdr *OnNewModel(void);
	virtual unsigned char GetClientSideFade();
	virtual void	SetupWeights(const matrix3x4_t *pBoneToWorld, int nFlexWeightCount, float *pFlexWeights, float *pFlexDelayedWeights);

	void GetRenderBounds(Vector& theMins, Vector& theMaxs);
	virtual void AddEntity(void);
	virtual void AccumulateLayers(IBoneSetup &boneSetup, Vector pos[], Quaternion q[], float currentTime);
	virtual void BuildTransformations(CStudioHdr *pStudioHdr, Vector *pos, Quaternion q[], const matrix3x4_t &cameraTransform, int boneMask, CBoneBitList &boneComputed);
	IPhysicsObject *GetElement(int elementNum);
	virtual void UpdateOnRemove();
	virtual float LastBoneChangedTime();
#ifdef DARKINTERVAL // for the ragdoll postmortem proxy
	float GetGoalValueForControlledVariable() { return m_hGoalValueForControlledVariable; }
#endif
	// Incoming from network
	Vector		m_ragPos[RAGDOLL_MAX_ELEMENTS];
	QAngle		m_ragAngles[RAGDOLL_MAX_ELEMENTS];

	CInterpolatedVarArray< Vector, RAGDOLL_MAX_ELEMENTS >	m_iv_ragPos;
	CInterpolatedVarArray< QAngle, RAGDOLL_MAX_ELEMENTS >	m_iv_ragAngles;

	int			m_elementCount;
	int			m_boneIndex[RAGDOLL_MAX_ELEMENTS];

private:
	C_ServerRagdoll(const C_ServerRagdoll &src);

	typedef CHandle<C_BaseAnimating> CBaseAnimatingHandle;
	CNetworkVar(CBaseAnimatingHandle, m_hUnragdoll);
	CNetworkVar(float, m_flBlendWeight);
	float m_flBlendWeightCurrent;
	CNetworkVar(int, m_nOverlaySequence);
	float m_flLastBoneChangeTime;
	float  m_hGoalValueForControlledVariable;
};

#endif