#include "cbase.h"
#include "proxyentity.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Called on an NPC entity when it dies (health <= 0)
//-----------------------------------------------------------------------------
class CNPCDyingFadeProxy : public CEntityMaterialProxy//CResultProxy
{
public:
	virtual bool Init(IMaterial *pMaterial, KeyValues *pKeyValues);
	virtual void OnBind(C_BaseEntity *pEntity);
	virtual IMaterial *GetMaterial();
private:
	IMaterialVar *m_rateVar;
	IMaterialVar *m_emissiveVar;
};

bool CNPCDyingFadeProxy::Init(IMaterial *pMaterial, KeyValues *pKeyValues)
{
	bool foundVar;
	m_rateVar = pMaterial->FindVar("$dyingfaderate", &foundVar, false);
	m_emissiveVar = pMaterial->FindVar("$emissiveBlendStrength", &foundVar, false);
	return foundVar;
}

void CNPCDyingFadeProxy::OnBind(C_BaseEntity *pEntity)
{
	if (!pEntity)
		return;

	if (m_rateVar)
	{
		float flValue;

		if (pEntity->GetHealth() <= 0)
		{
			m_emissiveVar->SetFloatValue(( 1.0 / gpGlobals->curtime) / m_rateVar->GetFloatValue());
			Msg("NPC health below 0, resultvar = %f\n", flValue);
		}
	}
}

IMaterial *CNPCDyingFadeProxy::GetMaterial()
{
	if (!m_rateVar || !m_emissiveVar)
		return NULL;

	return m_rateVar->GetOwningMaterial();
}

EXPOSE_INTERFACE(CNPCDyingFadeProxy, IMaterialProxy, "NPCDyingFade" IMATERIAL_PROXY_INTERFACE_VERSION);