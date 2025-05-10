#include "cbase.h"
#include "clienteffectprecachesystem.h"
#include "screenspaceeffects.h"
#include "c_te_effect_dispatch.h"
#include "keyvalues.h"
#include "fx_quad.h"
#include "c_ai_basenpc.h"

class C_NPC_Stinger : public C_AI_BaseNPC
{
	DECLARE_CLASS(C_NPC_Stinger, C_AI_BaseNPC);
	DECLARE_CLIENTCLASS();

public:
	virtual void	OnDataChanged(DataUpdateType_t updateType);
	virtual void	ReceiveMessage(int classID, bf_read &msg);

public:
};

IMPLEMENT_CLIENTCLASS_DT(C_NPC_Stinger, DT_NPC_Stinger, CNPC_Stinger)
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_NPC_Stinger::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);
}

//-----------------------------------------------------------------------------
// Purpose: Receive messages from the server
//-----------------------------------------------------------------------------
ConVar sk_stinger_venom_pp_scale("sk_stinger_venom_pp_scale", "1.0");
ConVar sk_stinger_venom_pp_duration("sk_stinger_venom_pp_duration", "6.0");

void C_NPC_Stinger::ReceiveMessage(int classID, bf_read &msg)
{
	int messageType = msg.ReadByte();
	switch (messageType)
	{
	case 0:
	{
		//========================
		// Do visual stun/blur

		KeyValues *pKeys = new KeyValues("keys");
		if (pKeys == NULL)
			return;

		if (g_pMaterialSystemHardwareConfig->GetDXSupportLevel() < 80)
			return;
		
		// Set our keys
		float m_flDuration = sk_stinger_venom_pp_duration.GetFloat();
		pKeys->SetFloat("duration", m_flDuration);

		float m_flScale = sk_stinger_venom_pp_scale.GetFloat() / 100;
		pKeys->SetFloat("scale", m_flScale);

		pKeys->SetInt("fadeout", 0);

		g_pScreenSpaceEffects->SetScreenSpaceEffectParams("di_stinger_venom", pKeys);
		g_pScreenSpaceEffects->EnableScreenSpaceEffect("di_stinger_venom");

	//	g_pScreenSpaceEffects->SetScreenSpaceEffectParams("di_stinger_background", pKeys);
	//	g_pScreenSpaceEffects->EnableScreenSpaceEffect("di_stinger_background");

		pKeys->deleteThis();		
	}
	break;
	case 1:
	{
		KeyValues *pKeys = new KeyValues("keys");
		if (pKeys == NULL)
			return;

		if (g_pMaterialSystemHardwareConfig->GetDXSupportLevel() < 80)
			return;

		// Set our keys
		pKeys->SetFloat("duration", msg.ReadFloat());
		pKeys->SetInt("fadeout", 1);

		g_pScreenSpaceEffects->SetScreenSpaceEffectParams("di_stinger_venom", pKeys);

	//	g_pScreenSpaceEffects->SetScreenSpaceEffectParams("di_stinger_background", pKeys);

		pKeys->deleteThis();
	}
	break;
	default:
		AssertMsg1(false, "Received unknown message %d", messageType);
		break;
	}
}
