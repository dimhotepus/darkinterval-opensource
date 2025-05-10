//========================================================================//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "c_ai_basenpc.h"
#include "model_types.h"
#include "clienteffectprecachesystem.h"
#define RIDEABLE_NPCS
#ifdef RIDEABLE_NPCS
#include "iclientvehicle.h"
#endif

CLIENTEFFECT_REGISTER_BEGIN(C_NPCCrabSynth)
CLIENTEFFECT_MATERIAL("custom_uni/shield_overlay")
CLIENTEFFECT_REGISTER_END()

class C_NPC_CrabSynth : public C_AI_BaseNPC
#ifdef RIDEABLE_NPCS
	, public IClientVehicle
#endif
{
	DECLARE_CLASS(C_NPC_CrabSynth, C_AI_BaseNPC);
	DECLARE_CLIENTCLASS();

public:
	virtual void OnDataChanged(DataUpdateType_t updateType);
	virtual int DrawModel(int flags);

	bool m_crabsynth_shield_enabled;

#ifdef RIDEABLE_NPCS
	// IClientVehicle overrides.
public:
	// IClientVehicle overrides.
	virtual void GetVehicleFOV(float &flFOV) { flFOV = 0.0f; }
	virtual int GetPrimaryAmmoType() const { return -1; }
	virtual int GetPrimaryAmmoCount() const { return -1; }
	virtual int GetPrimaryAmmoClip() const { return -1; }
	virtual bool PrimaryAmmoUsesClips() const { return false; }
	virtual int GetJoystickResponseCurve() const { return 0; }

	virtual void OnEnteredVehicle(C_BasePlayer *pPlayer) {}
	virtual void GetVehicleViewPosition(int nRole, Vector *pOrigin, QAngle *pAngles, float *pFOV = NULL);
	virtual void DrawHudElements() {}
	virtual bool IsPassengerUsingStandardWeapons(int nRole = VEHICLE_ROLE_DRIVER) { return true; }

	virtual void UpdateViewAngles(C_BasePlayer *pLocalPlayer, CUserCmd *pCmd)
	{
		int eyeAttachmentIndex = LookupAttachment("vehicle_driver_eyes");
		Vector origin;
		QAngle angles;
		GetAttachmentLocal(eyeAttachmentIndex, origin, angles);

		// Limit the view yaw to the relative range specified
		float flAngleDiff = AngleDiff(pCmd->viewangles.y, angles.y);
		flAngleDiff = clamp(flAngleDiff, -90, 90);
		pCmd->viewangles.y = angles.y + flAngleDiff;
	}
	virtual C_BasePlayer* GetPassenger(int nRole) { return m_hPlayer; }
	virtual int	GetPassengerRole(C_BaseCombatCharacter *pEnt) { return VEHICLE_ROLE_DRIVER; }
	virtual void GetVehicleClipPlanes(float &flZNear, float &flZFar) const {}
	virtual C_BaseEntity *GetVehicleEnt() { return this; }

	virtual IClientVehicle *GetClientVehicle() { return this; }
	virtual void SetupMove(C_BasePlayer *player, CUserCmd *ucmd, IMoveHelper *pHelper, CMoveData *move) {}
	virtual void ProcessMovement(C_BasePlayer *pPlayer, CMoveData *pMoveData) {}
	virtual void FinishMove(C_BasePlayer *player, CUserCmd *ucmd, CMoveData *move) {}
	virtual bool IsPredicted() const { return false; }
	virtual void ItemPostFrame(C_BasePlayer *pPlayer) {}
private:
	CHandle<C_BasePlayer>	m_hPlayer;
#endif
};

IMPLEMENT_CLIENTCLASS_DT(C_NPC_CrabSynth, DT_NPC_CrabSynth, CNPC_CrabSynth)
RecvPropBool(RECVINFO(m_crabsynth_shield_enabled)),
#ifdef RIDEABLE_NPCS
RecvPropEHandle(RECVINFO(m_hPlayer)),
#endif
END_RECV_TABLE()

void C_NPC_CrabSynth::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);
}

int C_NPC_CrabSynth::DrawModel(int flags)
{
	int retVal = BaseClass::DrawModel(flags);

	if (m_crabsynth_shield_enabled)
	{
		IMaterial *pMatGlowColor = NULL;
		pMatGlowColor = materials->FindMaterial("custom_uni/shield_overlay", TEXTURE_GROUP_OTHER, true);
		modelrender->ForcedMaterialOverride(pMatGlowColor);
		BaseClass::DrawModel(STUDIO_RENDER | STUDIO_TRANSPARENCY);
		modelrender->ForcedMaterialOverride(0);
	}

	return retVal;
}

#ifdef RIDEABLE_NPCS
void C_NPC_CrabSynth::GetVehicleViewPosition(int nRole, Vector *pAbsOrigin, QAngle *pAbsAngles, float *pFOV)
{
	Assert(nRole == VEHICLE_ROLE_DRIVER);
	CBasePlayer *pPlayer = GetPassenger(nRole);
	Assert(pPlayer);
	*pAbsAngles = pPlayer->EyeAngles();
	int eyeAttachmentIndex = LookupAttachment("vehicle_driver_eyes");
	QAngle tmp;
	GetAttachment(eyeAttachmentIndex, *pAbsOrigin, tmp);
}
#endif