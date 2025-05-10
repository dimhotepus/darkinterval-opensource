#include "cbase.h"
#include "props.h"
#include "player_control.h"
#include "hl2_player.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

#define PMANHACK_OFFSET		Vector(0,0,-72)
#define PMANHACK_HULL_MINS	Vector(-16,-16,-10)
#define PMANHACK_HULL_MAXS	Vector(16,16,10)
#define PMANHACK_DECAY		3
#define PMANHACK_MAX_ALTITUDE 1024.0
#define PMANHACK_TURN_RATE	100
#define HACK_MODEL			"models/_Weapons/v_manhack.mdl"
#define SPHERE_MODEL		"models/_Weapons/v_manhack_sphere.mdl"

BEGIN_DATADESC(CPlayer_Manhack)
DEFINE_FIELD(m_bActive, FIELD_BOOLEAN),
DEFINE_FIELD(m_nSaveFOV, FIELD_INTEGER),
// I&O
DEFINE_INPUTFUNC(FIELD_VOID, "Activate", InputActivate),
DEFINE_INPUTFUNC(FIELD_VOID, "Deactivate", InputDeactivate),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetThrust", InputSetThrust),
DEFINE_INPUTFUNC(FIELD_FLOAT, "SetSideThrust", InputSetSideThrust),
DEFINE_OUTPUT(m_OnExitReached, "OnExitReached"),
DEFINE_OUTPUT(m_OnActivated, "OnActivated"),
DEFINE_OUTPUT(m_OnDeactivated, "OnDeactivated"),
// Think
DEFINE_THINKFUNC(FlyThink),
// Blade prop ehandle
DEFINE_FIELD(m_bladeModel, FIELD_EHANDLE),
END_DATADESC()

LINK_ENTITY_TO_CLASS(player_control, CPlayer_Manhack);

//ConVar player_manhack_debug("player_manhack_debug", "1");
extern float	GetFloorZ(const Vector &origin);
CPlayer_Manhack::CPlayer_Manhack()
{
	exitTarget = NULL;
}
CPlayer_Manhack::~CPlayer_Manhack()
{
	CSoundEnvelopeController &controller = CSoundEnvelopeController::GetController();
	controller.SoundDestroy(m_sndEngineSound);
}
void CPlayer_Manhack::Precache(void) {
	PrecacheModel(HACK_MODEL);
	PrecacheModel(SPHERE_MODEL);

	PrecacheScriptSound("Player_Manhack.GrindSolid");
	PrecacheScriptSound("Player_Manhack.GrindFlesh");
	PrecacheScriptSound("Player_Manhack.EngineLoop");
//	PrecacheScriptSound("Player_Manhack.ThrustLow");
//	PrecacheScriptSound("Player_Manhack.ThrustHigh");
	BaseClass::Precache();
}

void CPlayer_Manhack::Spawn(void)
{
	Precache();
	m_bActive = false;
	SetModel(SPHERE_MODEL);
	UTIL_SetSize(this, PMANHACK_HULL_MINS, PMANHACK_HULL_MAXS);
	VPhysicsInitNormal(GetSolid(), GetSolidFlags(), false);

	CapabilitiesClear();

	SetMoveType(MOVETYPE_FLY);
	AddFlag(FL_FLY);
	SetFriction(0.0f);
	SetGravity(0.0f);

	SetRenderMode(kRenderNone);
	AddEffects(EF_NOSHADOW);
}

bool CPlayer_Manhack::CreateVPhysics(void)
{
	VPhysicsInitNormal(SOLID_VPHYSICS, 0, false);
	return false;
}

void CPlayer_Manhack::OnRestore(void)
{
	if (m_bActive)
	{
		CPASAttenuationFilter filter(this);
		m_sndEngineSound = (CSoundEnvelopeController::GetController()).SoundCreate(filter, entindex(), CHAN_STATIC, "Player_Manhack.EngineLoop", ATTN_NORM);

		assert(m_sndEngineSound != NULL);
		if (m_sndEngineSound != NULL) { CSoundEnvelopeController::GetController().Play(m_sndEngineSound, 1.0f, 100); }
	}
}

void CPlayer_Manhack::InputActivate(inputdata_t &inputdata)
{
	ControlActivate();
}

void CPlayer_Manhack::InputDeactivate(inputdata_t &inputdata)
{
	ControlDeactivate();
}

void CPlayer_Manhack::ControlActivate(void) 
{
	m_bActive = true;

	if (exitTarget == NULL)
		exitTarget = gEntList.FindEntityByName(this, "hackman_exit");
	
	CHL2_Player* player = (CHL2_Player*)(UTIL_GetLocalPlayer());

	Assert(player);

	// Save player data
	m_nSaveFOV = player->GetFOV();
	
	// set player controls
//	SetAbsAngles(QAngle(0, 0, 180));
	player->SetFOV(this, 110);
	player->AddSolidFlags(FSOLID_NOT_SOLID);
	player->SetParent(this);
	player->SetLocalOrigin(Vector(0, 0, 0));
	player->SetLocalAngles(GetAbsAngles());
	player->Teleport(&GetAbsOrigin(), &GetAbsAngles(), &GetAbsVelocity());
	player->SnapEyeAngles(QAngle(0, 0, 0));
	player->SetViewOffset(Vector(0, 0, -64));
	player->FollowEntity(this);
	player->SetControlClass(CLASS_MANHACK);

	if (player->HasWeapons()) { player->GetActiveWeapon()->Holster(); } // hide the viewmodel	
	player->m_Local.m_iHideHUD &= HIDEHUD_HEALTH;

	// Create blade prop
	m_bladeModel = (CDynamicProp*)CreateEntityByName("prop_dynamic");
	if (m_bladeModel)
	{
		m_bladeModel->SetModel(HACK_MODEL);
		m_bladeModel->SetName(AllocPooledString("hackman_blade"));
		m_bladeModel->SetParent(player);
		m_bladeModel->SetOwnerEntity(player);
		m_bladeModel->SetSolid(SOLID_NONE);
		m_bladeModel->SetLocalOrigin(Vector(0, 0, -5));
		m_bladeModel->SetLocalAngles(QAngle(0, 0, 0));
		m_bladeModel->AddSpawnFlags(SF_DYNAMICPROP_NO_VPHYSICS);
	//	m_bladeModel->AddEffects(EF_PARENT_ANIMATES);
		m_bladeModel->Spawn();

		engine->ClientCommand(player->edict(), "ent_fire hackman_blade setanimation idle");
	}

	// init engine sound
	CPASAttenuationFilter filter(this);
	m_sndEngineSound = (CSoundEnvelopeController::GetController()).SoundCreate(filter, entindex(),	CHAN_STATIC, "Player_Manhack.EngineLoop", ATTN_NORM);

	assert(m_sndEngineSound != NULL);
	if (m_sndEngineSound != NULL)	{ CSoundEnvelopeController::GetController().Play(m_sndEngineSound, 1.0f, 100); }
	
	// Think
	SetThink(&CPlayer_Manhack::FlyThink);
	SetNextThink(CURTIME);
	DispatchUpdateTransmitState();

	m_flNextAttack = CURTIME;

	m_OnActivated.FireOutput(this, this);
}

void CPlayer_Manhack::ControlDeactivate(void) 
{
	m_bActive = false;

//	SetMoveType(MOVETYPE_VPHYSICS);
//	RemoveFlag(FL_FLY);

	CHL2_Player* player = (CHL2_Player*)(UTIL_GetLocalPlayer());

	Assert(player);

	// Restore player data
	// Note: the manhack control used to store and restore
	// player's origin and angles. But angles didn't work right anyway,
	// and why bother with it when we're firing OnDeactivated output?
	// We'll just rely on map logic to teleport the player back into place. 
	player->SetFOV(this, m_nSaveFOV);
	player->RemoveSolidFlags(FSOLID_NOT_SOLID);
	player->StopFollowingEntity();
	inputdata_t inputdata;
	player->InputClearParent(inputdata);
	player->SetMoveType(MOVETYPE_WALK, MOVECOLLIDE_DEFAULT);
	player->SetControlClass(CLASS_NONE);

	if (player->HasWeapons()) { player->GetActiveWeapon()->Deploy(); } // 'restore' viewmodel
	player->m_Local.m_iHideHUD &= ~HIDEHUD_HEALTH;

	// Remove blade
	if (m_bladeModel != NULL)
	{
		UTIL_Remove(m_bladeModel);
	}

	// stop all sounds
	StopSound(player->entindex(), "Player_Manhack.GrindFlesh");
	StopSound(player->entindex(), "Player_Manhack.GrindSolid");

	if (m_sndEngineSound != NULL)
		(CSoundEnvelopeController::GetController()).SoundFadeOut(m_sndEngineSound, 0.1f);

	SetThink(NULL);
	SetTouch(NULL);
	DispatchUpdateTransmitState();

	m_OnDeactivated.FireOutput(this, this);
}

void CPlayer_Manhack::InputSetThrust(inputdata_t &inputdata) {
	if (!m_bActive) return;
	CBasePlayer* player = UTIL_GetLocalPlayer();
	Assert(player);

	if (inputdata.value.Float() == 0) {	return;	}

	Vector vForce;
	player->EyeVectors(&vForce);
	vForce = vForce * inputdata.value.Float() * 600;

	SetLocalVelocity(vForce);
}

void CPlayer_Manhack::InputSetSideThrust(inputdata_t &inputdata) {
	if (!m_bActive) return;
	CBasePlayer*	player = UTIL_GetLocalPlayer();
	Assert(player);

	if (inputdata.value.Float() == 0) {	return; }

	Vector vForce;
	player->EyeVectors(0, &vForce, 0);
	vForce = vForce * inputdata.value.Float() * 600;

	SetLocalVelocity(vForce);
}

void CPlayer_Manhack::DrawHUD(void)
{
	CBasePlayer *player = UTIL_GetLocalPlayer();
	if (!player) return;

	float flBrightness;

	if (random->RandomInt(0, 1) == 0) { flBrightness = 255; }

	else { flBrightness = 1; }

//	color32 white = { flBrightness,flBrightness,flBrightness,20 };
//	UTIL_ScreenFade(player, white, 0.01, 0.1, FFADE_MODULATE);

	flBrightness = random->RandomInt(0, 255);
	NDebugOverlay::ScreenText(0.4, 0.15, "...M A N H A C K     A C T I V A T E D...", 255, flBrightness, flBrightness, 255, 1.0);
}

void CPlayer_Manhack::FlyThink(void) {

	SetNextThink(CURTIME);

	// Update the player's origin
	CBasePlayer* player = UTIL_GetLocalPlayer();
	Assert(player);

	// Update blade angles
	if (m_bladeModel != NULL)
	{
		m_bladeModel->SetLocalAngles(player->LocalEyeAngles());
	}

	Vector vPlayerFacing;
	player->EyeVectors(&vPlayerFacing);

	// ----------------------------------------
	//	Trace forward to see if I hit anything
	// ----------------------------------------
	trace_t			tr;
	Vector vVelocity;
	GetVelocity(&vVelocity, NULL);
//	vVelocity.z = 0;
	Vector vCheckPos = GetAbsOrigin() /*+ (vVelocity*gpGlobals->frametime)*/;

	// Check in movement direction
	UTIL_TraceHull(GetAbsOrigin(), vCheckPos,
		Vector(-22, -22, -1), Vector(22, 22, 1),
		MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
	CheckBladeTrace(tr);

	// Check in facing direction
	UTIL_TraceHull(GetAbsOrigin(), GetAbsOrigin() + vPlayerFacing * 16,
		Vector(-22, -22, -1), Vector(22, 22, 1),
		MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
	CheckBladeTrace(tr);

	// test: continuous movement when key is pressed

	inputdata_t id;

	if (player->m_nButtons & IN_FORWARD)
	{
		id.value.SetFloat(0.6f);
		InputSetThrust(id);
	}
	else if(player->m_nButtons & IN_MOVELEFT)
	{
		id.value.SetFloat(-0.6f);
		InputSetSideThrust(id);
	}
	else if (player->m_nButtons & IN_MOVERIGHT)
	{
		id.value.SetFloat(0.6f);
		InputSetSideThrust(id);
	}
	else if (player->m_nButtons & IN_BACK)
	{
		id.value.SetFloat(-0.6f);
		InputSetThrust(id);
	}
	
	// Draw the text
	DrawHUD();

	// Add decay
	Vector vOldVelocity;

	this->GetVelocity(&vOldVelocity, NULL);

	float flDecay = -PMANHACK_DECAY*gpGlobals->frametime;
	if (flDecay < -1.0) { flDecay = -1.0; }

	ApplyAbsVelocityImpulse(flDecay*vOldVelocity);

	this->GetVelocity(&vOldVelocity, NULL);
	if (vOldVelocity.Length() > PMANHACK_MAX_SPEED) {
		Vector vNewVelocity = vOldVelocity;
		VectorNormalize(vNewVelocity);
		vNewVelocity = vNewVelocity *PMANHACK_MAX_SPEED;
		Vector vSubtract = vNewVelocity - vOldVelocity;
		this->VelocityPunch(vSubtract);
	}

	// Hover noise
	float	flNoiseSine = sin(CURTIME * 2.0f);// 10 * sin(4 * CURTIME);
	float	flNoiseAmp = 120;
	Vector  vNoise = Vector(0, 0, (flNoiseSine * flNoiseAmp )*gpGlobals->frametime);
	ApplyAbsVelocityImpulse(vNoise);

	// Check if we're close to designated map exit, and if so, fire the output.

	if (exitTarget)
	{
		Vector vExitPos;
		vExitPos = GetAbsOrigin() - exitTarget->GetAbsOrigin();

		if (vExitPos.Length() < 128.0f)
			m_OnExitReached.FireOutput(this, this);
	}

	CheckDamageSphere(32.0f, vPlayerFacing);
}

void CPlayer_Manhack::CheckDamageSphere(float fRadius, Vector vFacing)
{
	Vector vO = GetAbsOrigin();

	CBaseEntity *victim = NULL;

	CBroadcastRecipientFilter filter;

	while ((victim = gEntList.FindEntityInSphere(victim, vO, fRadius)) != NULL)
	{
		if ( m_flNextAttack < CURTIME)
		{
			if (victim->ClassMatches("npc_citizen"))
			{
				CTakeDamageInfo info(this, this, 20, DMG_SLASH);
				CalculateMeleeDamageForce(&info, vFacing, vO);
				victim->TakeDamage(info);

				CBaseCombatCharacter* pBCC = ToBaseCombatCharacter(victim);

				if (victim) // still alive? 
				{
					Vector vBloodPos = vO + vFacing * 16;
					SpawnBlood( vBloodPos, vFacing, pBCC->BloodColor(), 1.0 );
					EmitSound("Player_Manhack.GrindFlesh");
					m_flNextAttack = CURTIME + 0.1f;
				}
			}
			else if (victim->ClassMatches("func_breakable"))
			{
				CTakeDamageInfo info(this, this, 10, DMG_SLASH);
				CalculateMeleeDamageForce(&info, vFacing, vO);
				victim->TakeDamage(info);

				Vector vSparkPos = vO + vFacing * 32;
				g_pEffects->Sparks(vSparkPos, 2.5, 1.5);
				te->DynamicLight(filter, 0.0, &GetAbsOrigin(), 255, 180, 100, 0.1, 256, 0.1, 0);
				EmitSound("Player_Manhack.GrindSolid");
				m_flNextAttack = CURTIME + 0.1f;
			}
			else
				continue;
		}
	}
}

void CPlayer_Manhack::CheckBladeTrace(trace_t &tr)
{
	CBasePlayer*	pPlayer = UTIL_PlayerByIndex(1);
	Assert(pPlayer);

	if (tr.fraction != 1.0 || tr.startsolid)
	{
		CPASAttenuationFilter filter(pPlayer, "Player_Manhack.GrindSolid");
		EmitSound(filter, pPlayer->entindex(), "Player_Manhack.GrindSolid");

		// For decals and sparks we must trace a line in the direction of the surface norm
		// that we hit.
		Vector vCheck = GetAbsOrigin() - (tr.plane.normal * 24);

		UTIL_TraceLine(GetAbsOrigin(), vCheck, MASK_SOLID_BRUSHONLY, this, COLLISION_GROUP_NONE, &tr);
		if (tr.fraction != 1.0)
		{
			g_pEffects->Sparks(tr.endpos);

			CBroadcastRecipientFilter filter;
			te->DynamicLight(filter, 0.0,
				&tr.endpos, 255, 180, 100, 0, 10, 0.1, 0);

			Vector vecDir = Vector(0, 0, 1);//tr.endpos - GetLocalOrigin();
		//	vecDir.z = 0.0; // planar
			VectorNormalize(vecDir);

		
			vecDir *= 50.0f;

		//	ApplyAbsVelocityImpulse(vecDir);

		//	SetLocalVelocity(vecDir);
			
			// Leave decal only if colliding horizontally
		//	if ((DotProduct(Vector(0, 0, 1), tr.plane.normal) < 0.5) && (DotProduct(Vector(0, 0, -1), tr.plane.normal) < 0.5))
		//	{
		//		UTIL_DecalTrace(&tr, "ManhackCut");
		//	}
		}
	}
}

int CPlayer_Manhack::UpdateTransmitState()
{
	if (m_bActive)
	{
		return SetTransmitState(FL_EDICT_ALWAYS);
	}
	else
	{
		return BaseClass::UpdateTransmitState();
	}
}