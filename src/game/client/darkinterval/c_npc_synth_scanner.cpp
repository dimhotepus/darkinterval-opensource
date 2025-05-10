//========================================================================//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "particles_simple.h"
#include "citadel_effects_shared.h"
#include "particles_attractor.h"
#include "iefx.h"
#include "dlight.h"
#include "clienteffectprecachesystem.h"
#include "screenspaceeffects.h"
#include "c_te_effect_dispatch.h"
#include "keyvalues.h"
#include "fx_quad.h"

#include "c_ai_basenpc.h"

// For material proxy
#include "proxyentity.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"

#define NUM_INTERIOR_PARTICLES	8

#define DLIGHT_RADIUS (150.0f)
#define DLIGHT_MINLIGHT (40.0f/255.0f)

class C_NPC_SynthScanner : public C_AI_BaseNPC
{
	DECLARE_CLASS(C_NPC_SynthScanner, C_AI_BaseNPC);
	DECLARE_CLIENTCLASS();

public:
	virtual void	OnDataChanged(DataUpdateType_t updateType);
	virtual void	ReceiveMessage(int classID, bf_read &msg);

public:
};

IMPLEMENT_CLIENTCLASS_DT(C_NPC_SynthScanner, DT_NPC_SynthScanner, CNPC_SynthScanner)
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_NPC_SynthScanner::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);
}

// FIXME: Move to shared code!
#define VORTFX_ZAPBEAM	0
#define VORTFX_ARMBEAM	1

//-----------------------------------------------------------------------------
// Purpose: Receive messages from the server
//-----------------------------------------------------------------------------
void C_NPC_SynthScanner::ReceiveMessage(int classID, bf_read &msg)
{
	int messageType = msg.ReadByte();
	switch (messageType)
	{
	case 0:
	{
	//========================
		// Create zappy fx
		unsigned char nAttachment = msg.ReadByte();
		Vector vecStart;
		QAngle vecAngles;
		GetAttachment(nAttachment, vecStart, vecAngles);

		Vector vecEndPos;
		msg.ReadBitVec3Coord(vecEndPos);

		CNewParticleEffect *pEffect = ParticleProp()->Create("vortigaunt_beam", PATTACH_POINT_FOLLOW, nAttachment);
		if (pEffect)
		{
			pEffect->SetControlPoint(0, vecStart);
			pEffect->SetControlPoint(1, vecEndPos);
		}
	}
	break;
	case 1:
	{
	//========================
		// Do visual stun/blur
		float m_flDuration = msg.ReadByte();

		KeyValues *pKeys = new KeyValues("keys");
		if (pKeys == NULL)
			return;
		pKeys->SetFloat("duration", m_flDuration);

		g_pScreenSpaceEffects->SetScreenSpaceEffectParams("episodic_stun", pKeys);
		g_pScreenSpaceEffects->EnableScreenSpaceEffect("episodic_stun");

		pKeys->deleteThis();
	}
	break;
	case 2:
	{
		g_pScreenSpaceEffects->DisableScreenSpaceEffect("episodic_stun");
	}
	break;
	case 3:
	{
	//========================
		// Create poison cloud and move it towards the target (set endpoint from target's origin)
		unsigned char nAttachment = msg.ReadByte();
		Vector vecStart;
		QAngle vecAngles;
		GetAttachment(nAttachment, vecStart, vecAngles);

		Vector vecEndPos;
		msg.ReadBitVec3Coord(vecEndPos);

		CNewParticleEffect *pEffect = ParticleProp()->Create("synth_scanner_gas", PATTACH_POINT_FOLLOW, nAttachment);
		if (pEffect)
		{
			pEffect->SetControlPoint(0, vecEndPos);
			pEffect->SetControlPoint(1, vecEndPos);
		}
	}
	break;
	default:
		AssertMsg1(false, "Received unknown message %d", messageType);
		break;
	}
}
