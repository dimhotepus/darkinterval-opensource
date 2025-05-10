//========================================================================//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "c_ai_basenpc.h"
// For material proxy
#include "proxyentity.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "functionproxy.h"
#include "toolframework_client.h"
#include "object_motion_blur_effect.h"

class C_NPC_CombineElite : public C_AI_BaseNPC
{
	DECLARE_CLASS(C_NPC_CombineElite, C_AI_BaseNPC);
	DECLARE_CLIENTCLASS();

public:
	C_NPC_CombineElite();

	virtual void	OnDataChanged(DataUpdateType_t updateType);

	int DrawModel(int flags);

	float m_camo_refract_value;
	float m_camo_fogginess_value;

private:
//	CMotionBlurObject m_MotionBlurObject;
	CMeshBuilder	m_Mesh; // for drawing nameplates

};

IMPLEMENT_CLIENTCLASS_DT(C_NPC_CombineElite, DT_NPC_CombineElite, CNPC_CombineElite)
RecvPropFloat(RECVINFO(m_camo_refract_value)),
RecvPropFloat(RECVINFO(m_camo_fogginess_value)),
END_RECV_TABLE()

C_NPC_CombineElite::C_NPC_CombineElite()
//	: m_MotionBlurObject(this, 1000.0f)
{

}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_NPC_CombineElite::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);
}

int C_NPC_CombineElite::DrawModel(int flags)
{
	if (BaseClass::DrawModel(flags) == false)
		return 0;
	
	return 1;
}

// forward declarations
class CCamoProxy1 : public CResultProxy
{
public:
	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	virtual void OnBind(void *pC_BaseEntity);
private:
	C_NPC_CombineElite *BindArgToEntity(void *pArg);
};

C_NPC_CombineElite *CCamoProxy1::BindArgToEntity(void *pArg)
{
	IClientRenderable *pRend = (IClientRenderable *)pArg;
	return (C_NPC_CombineElite*)pRend->GetIClientUnknown()->GetBaseEntity();
}

bool CCamoProxy1::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	return CResultProxy::Init(pMaterial, pKeyValues);
}

void CCamoProxy1::OnBind(void *pC_BaseEntity)
{
	if (!pC_BaseEntity)
		return;

	C_BaseEntity *pEntity = BindArgToEntity(pC_BaseEntity);
	C_NPC_CombineElite *pElite = BindArgToEntity(pC_BaseEntity);
	if (!pElite)
		return;

	float proxyvalue;
	proxyvalue = pElite->m_camo_refract_value;

	// This is actually a vector...
	Assert(m_pResult);
	m_pResult->SetFloatValue(proxyvalue);

	if (ToolsEnabled())
	{
		ToolFramework_RecordMaterialParams(GetMaterial());
	}
}

EXPOSE_INTERFACE(CCamoProxy1, IMaterialProxy, "Camo_Refractive" IMATERIAL_PROXY_INTERFACE_VERSION);

// forward declarations
class CCamoProxy2 : public CResultProxy
{
public:
	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	virtual void OnBind(void *pC_BaseEntity);
private:
	C_NPC_CombineElite *BindArgToEntity(void *pArg);
};

C_NPC_CombineElite *CCamoProxy2::BindArgToEntity(void *pArg)
{
	IClientRenderable *pRend = (IClientRenderable *)pArg;
	return (C_NPC_CombineElite*)pRend->GetIClientUnknown()->GetBaseEntity();
}

bool CCamoProxy2::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	return CResultProxy::Init(pMaterial, pKeyValues);
}

void CCamoProxy2::OnBind(void *pC_BaseEntity)
{
	if (!pC_BaseEntity)
		return;

	C_BaseEntity *pEntity = BindArgToEntity(pC_BaseEntity);
	C_NPC_CombineElite *pElite = BindArgToEntity(pC_BaseEntity);
	if (!pElite)
		return;

	float proxyvalue;
	proxyvalue = pElite->m_camo_fogginess_value;

	// This is actually a vector...
	Assert(m_pResult);
	m_pResult->SetFloatValue(proxyvalue);

	if (ToolsEnabled())
	{
		ToolFramework_RecordMaterialParams(GetMaterial());
	}
}

EXPOSE_INTERFACE(CCamoProxy2, IMaterialProxy, "Camo_Foggy" IMATERIAL_PROXY_INTERFACE_VERSION);
