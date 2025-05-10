#include "cbase.h"
#include "debugoverlay_shared.h"
#include "c_env_fog_controller.h"
#include "ragdoll_serverside.h"
#include "c_vehicle_apc.h"
#include "c_physicsprop.h"
#include "materialsystem/imaterialproxy.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "functionproxy.h"
#include <keyvalues.h>
#include "mathlib/vmatrix.h"
#include "toolframework_client.h"

// forward declarations
class CCrossbowProxy : public CResultProxy
{
public:
	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	virtual void OnBind(void *pC_BaseEntity);
private:
	float	m_Scale;
};

bool CCrossbowProxy::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	if (!CResultProxy::Init(pMaterial, pKeyValues))
		return false;

	m_Scale = pKeyValues->GetFloat("scale", 0.01);
	return true;
}

void CCrossbowProxy::OnBind(void *pC_BaseEntity)
{
	if (!pC_BaseEntity)
		return;

	C_BaseEntity *pEntity = BindArgToEntity(pC_BaseEntity);
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	float proxyvalue;
	proxyvalue = pPlayer->WeaponProxyValueSlot01();

	// This is actually a vector...
	Assert(m_pResult);
	m_pResult->SetFloatValue(proxyvalue);

	if (ToolsEnabled())
	{
		ToolFramework_RecordMaterialParams(GetMaterial());
	}
}

EXPOSE_INTERFACE(CCrossbowProxy, IMaterialProxy, "CrossbowHeating" IMATERIAL_PROXY_INTERFACE_VERSION);

#if 0
class CSkyboxTinting : public CResultProxy
{
public:
	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	virtual void OnBind(void *pC_BaseEntity);
private:
	float	m_tintrate;
	float	m_tintcolor_r;
	float	m_tintcolor_g;
	float	m_tintcolor_b;
};

bool CSkyboxTinting::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	if (!CResultProxy::Init(pMaterial, pKeyValues))
		return false;

	m_tintrate = pKeyValues->GetFloat("tintRate", 1.0);
	m_tintcolor_r = pKeyValues->GetFloat("tintRed", 1.0);
	m_tintcolor_g = pKeyValues->GetFloat("tintGreen", 1.0);
	m_tintcolor_b = pKeyValues->GetFloat("tintBlue", 1.0);
	return true;
}

void CSkyboxTinting::OnBind(void *pC_BaseEntity)
{
	if (!pC_BaseEntity)
		return;

	C_BaseEntity *pEntity = BindArgToEntity(pC_BaseEntity);

	if (!pPlayer)
		return;

	float proxyvalue = pPlayer->WeaponProxyValueSlot01();

	// This is actually a vector...
	Assert(m_pResult);
	m_pResult->SetFloatValue(proxyvalue);

	if (ToolsEnabled())
	{
		ToolFramework_RecordMaterialParams(GetMaterial());
	}
}

EXPOSE_INTERFACE(CSkyboxTinting, IMaterialProxy, "SkyboxTinting" IMATERIAL_PROXY_INTERFACE_VERSION);
#endif
//========================================================================================
// This is the new proxy that lets us create lasting visual effects on corpses, such
// as fading glows and others.
//========================================================================================
// forward declarations
//void ToolFramework_RecordMaterialParams(IMaterial *pMaterial);

class CRagdollFXProxy : public IMaterialProxy
{
public:
	CRagdollFXProxy();
	virtual ~CRagdollFXProxy();

	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	virtual void OnBind(void *pC_BaseEntity);
	virtual void Release(void) { delete this; }
	virtual IMaterial *GetMaterial();

private:
	C_ServerRagdoll *BindArgToEntity(void *pArg);

	IMaterialVar *m_pControlledVariable;
};

CRagdollFXProxy::CRagdollFXProxy()
{
	m_pControlledVariable = NULL;
}

CRagdollFXProxy::~CRagdollFXProxy()
{
}


bool CRagdollFXProxy::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	char const* pControllerVariableName = pKeyValues->GetString("VariableToChange");
	if (!pControllerVariableName)
		return false;

	bool foundVar;
	m_pControlledVariable = pMaterial->FindVar(pControllerVariableName, &foundVar, false);
	if (!foundVar)
		return false;

	return true;
}

C_ServerRagdoll *CRagdollFXProxy::BindArgToEntity(void *pArg)
{
	IClientRenderable *pRend = (IClientRenderable *)pArg;
	if (pRend && pRend->GetIClientUnknown()->GetBaseEntity())
	{
		C_ServerRagdoll *pRagdoll = dynamic_cast<C_ServerRagdoll*>(pRend->GetIClientUnknown()->GetBaseEntity());
		if (pRagdoll)
		{
			return pRagdoll;
		}
	}
	return NULL;
}

void CRagdollFXProxy::OnBind(void *pC_BaseEntity)
{
	if (!pC_BaseEntity)
		return;

	//	C_BaseEntity *pEntity = BindArgToEntity(pC_BaseEntity);

	//	if (!pEntity)
	//		return;

	C_ServerRagdoll *pH = BindArgToEntity(pC_BaseEntity); //dynamic_cast<C_ServerRagdoll*>(pEntity);

	if (!pH)
		return;

	if (!m_pControlledVariable)
	{
		return;
	}

	float flH = pH->GetGoalValueForControlledVariable();
	//	float flRate = 0.5;

	m_pControlledVariable->SetFloatValue(flH);

	if (ToolsEnabled())
	{
		ToolFramework_RecordMaterialParams(GetMaterial());
	}
}

IMaterial *CRagdollFXProxy::GetMaterial()
{
	return m_pControlledVariable ? m_pControlledVariable->GetOwningMaterial() : NULL;
}

EXPOSE_INTERFACE(CRagdollFXProxy, IMaterialProxy, "RagdollPostmortem" IMATERIAL_PROXY_INTERFACE_VERSION);

//========================================================================================
// This new proxy lets us count the times a physical props got stained with dirt
// (usually by OnStartTouch trigger placed inside either mud or water, where
// touching clean water will decrease the counter and mud will increase it)
//========================================================================================
#if 0
class CPropStainedCount : public CResultProxy
{
public:
	CPropStainedCount();
	virtual ~CPropStainedCount();
	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	virtual void OnBind(void *pC_BaseEntity);
	virtual IMaterial *GetMaterial();

private:
	C_PhysicsProp *BindArgToEntity(void *pArg);
	IMaterialVar *m_pControlledVariable;
};

CPropStainedCount::CPropStainedCount()
{
	m_pControlledVariable = NULL;
}

CPropStainedCount::~CPropStainedCount()
{
}

C_PhysicsProp *CPropStainedCount::BindArgToEntity(void *pArg)
{
	IClientRenderable *pRend = (IClientRenderable *)pArg;
	return (C_PhysicsProp*)pRend->GetIClientUnknown()->GetBaseEntity();
}

bool CPropStainedCount::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	char const* pControllerVariableName = pKeyValues->GetString("VariableToChange");
	if (!pControllerVariableName)
		return false;

	bool foundVar;
	m_pControlledVariable = pMaterial->FindVar(pControllerVariableName, &foundVar, false);
	if (!foundVar)
		return false;

	return true;
}

void CPropStainedCount::OnBind(void *pC_BaseEntity)
{
	if (!pC_BaseEntity)
		return;

	C_PhysicsProp *pH = BindArgToEntity(pC_BaseEntity);

	if (!pH)
		return;

	if (!m_pControlledVariable)
	{
		return;
	}

	int stainCount;
	stainCount = pH->GetStainCount();
	
	m_pControlledVariable->SetIntValue(stainCount);

	if (ToolsEnabled())
	{
		ToolFramework_RecordMaterialParams(GetMaterial());
	}
}

IMaterial *CPropStainedCount::GetMaterial()
{
	return m_pControlledVariable ? m_pControlledVariable->GetOwningMaterial() : NULL;
}

EXPOSE_INTERFACE(CPropStainedCount, IMaterialProxy, "AccumulationPercentage" IMATERIAL_PROXY_INTERFACE_VERSION);
#endif