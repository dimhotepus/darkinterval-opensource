#include	"cbase.h"
#include	"AI_Default.h"
#include	"AI_Task.h"
#include	"AI_Schedule.h"
#include	"AI_Node.h"
#include	"AI_Hull.h"
#include	"AI_Hint.h"
#include	"AI_Route.h"
#include	"soundent.h"
#include	"game.h"
#include	"NPCEvent.h"
#include	"EntityList.h"
#include	"activitylist.h"
#include	"animation.h"
#include	"basecombatweapon.h"
#include	"ieffects.h"
#include	"vstdlib/random.h"
#include	"engine/ienginesound.h"
#include	"ammodef.h"
#include	"ai_basenpc.h"
#include	"AI_Senses.h"

// Movement constants

#define		LEECH_ACCELERATE		10
#define		LEECH_CHECK_DIST		45
#define		LEECH_SWIM_SPEED		50
#define		LEECH_SWIM_ACCEL		80
#define		LEECH_SWIM_DECEL		10
#define		LEECH_TURN_RATE			90
#define		LEECH_SIZEX				10
#define		LEECH_FRAMETIME			0.1

class CNPC_Parasite : public CAI_BaseNPC
{
	DECLARE_CLASS(CNPC_Parasite, CAI_BaseNPC);
public:

	DECLARE_SERVERCLASS();
	DECLARE_DATADESC();

	void Spawn(void);
	void Precache(void);
	
	void SwimThink(void);

	void UpdateMotion(void);

	void RecalculateWaterlevel(void);

	void SetObjectCollisionBox(void)
	{
		Vector vecMins, vecMaxs;
		vecMins = GetLocalOrigin() + Vector(-2, -2, 0);
		vecMaxs = GetLocalOrigin() + Vector(2, 2, 1);

		SetSize(vecMins, vecMaxs);
		//SetAbsMins(GetLocalOrigin() + Vector(-8, -8, 0));
		//SetAbsMaxs(GetLocalOrigin() + Vector(8, 8, 2));
	}

	Disposition_t IRelationType(CBaseEntity *pTarget);
	
	void Activate(void);

	Class_T	Classify(void) { return CLASS_NONE; };
	
private:
	// UNDONE: Remove unused boid vars, do group behavior
	float	m_flTurning;// is this boid turning?
	float	m_flAccelerate;
	float	m_top;
	float	m_bottom;
	float	m_height;
	float	m_waterTime;
	float	m_sideTime;		// Timer to randomly check clearance on sides
	float	m_stateTime;
	Vector  m_oldOrigin;
};

LINK_ENTITY_TO_CLASS(npc_parasite, CNPC_Parasite);

BEGIN_DATADESC(CNPC_Parasite)
DEFINE_FIELD(m_flTurning, FIELD_FLOAT),
DEFINE_FIELD(m_flAccelerate, FIELD_FLOAT),
DEFINE_FIELD(m_top, FIELD_FLOAT),
DEFINE_FIELD(m_bottom, FIELD_FLOAT),
DEFINE_FIELD(m_height, FIELD_FLOAT),
DEFINE_FIELD(m_waterTime, FIELD_TIME),
DEFINE_FIELD(m_sideTime, FIELD_TIME),
DEFINE_FIELD(m_stateTime, FIELD_TIME),
DEFINE_FIELD(m_oldOrigin, FIELD_VECTOR),

DEFINE_THINKFUNC(SwimThink),
END_DATADESC()

IMPLEMENT_SERVERCLASS_ST(CNPC_Parasite, DT_NPC_Parasite)
END_SEND_TABLE()

void CNPC_Parasite::Precache(void)
{
	PrecacheModel("models/_Monsters/Xen/leech.mdl");
}

void CNPC_Parasite::Spawn(void)
{
	Precache();
	SetModel("models/_Monsters/Xen/leech.mdl");

	SetHullType(HULL_TINY_CENTERED);
	SetHullSizeNormal();

	UTIL_SetSize(this, Vector(-0.5, -0.5, 0), Vector(0.5, 0.5, 0.5));
	SetModelScale(RandomFloat(0.85, 1.25), 0.0f); // we randomise the size so it's not always clones of the one model
	UpdateModelScale();
	// Don't push the minz down too much or the water check will fail because this entity is really point-sized
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_NOT_SOLID);
	SetMoveType(MOVETYPE_FLY);
	AddFlag(FL_SWIM);
	m_iHealth = 555;

	m_flFieldOfView = -0.5;	// 180 degree FOV
	SetDistLook(750);
	NPCInit();
	SetThink(&CNPC_Parasite::SwimThink);
	SetUse(NULL);
	SetTouch(NULL);
	SetViewOffset(vec3_origin);

	m_flTurning = 0;
	SetActivity(ACT_SWIM);
	SetState(NPC_STATE_IDLE);
	m_stateTime = CURTIME + random->RandomFloat(1, 5);

	SetRenderColor(33, 42, 33, 100);

	m_bloodColor = BLOOD_COLOR_RED;
	SetCollisionGroup(COLLISION_GROUP_DEBRIS);
}

void CNPC_Parasite::Activate(void)
{
	RecalculateWaterlevel();
	BaseClass::Activate();
}

Disposition_t CNPC_Parasite::IRelationType(CBaseEntity *pTarget)
{
//	if (pTarget->IsPlayer())
//		return D_HT;

	return BaseClass::IRelationType(pTarget);
}

void CNPC_Parasite::RecalculateWaterlevel(void)
{
	// Calculate boundaries
	Vector vecTest = GetLocalOrigin() - Vector(0, 0, 400);

	trace_t tr;

	UTIL_TraceLine(GetLocalOrigin(), vecTest, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);

	if (tr.fraction != 1.0)
		m_bottom = tr.endpos.z + 1;
	else
		m_bottom = vecTest.z;

	m_top = UTIL_WaterLevel(GetLocalOrigin(), GetLocalOrigin().z, GetLocalOrigin().z + 400) - 1;

	// Chop off 20% of the outside range
	float newBottom = m_bottom * 0.8 + m_top * 0.2;
	m_top = m_bottom * 0.2 + m_top * 0.8;
	m_bottom = newBottom;
	m_height = random->RandomFloat(m_bottom, m_top);
	m_waterTime = CURTIME + random->RandomFloat(5, 7);
}

void CNPC_Parasite::SwimThink(void)
{
	trace_t			tr;
	float			flLeftSide;
	float			flRightSide;
	float			targetSpeed;
	float			targetYaw = 0;

	SetNextThink(CURTIME + 0.1);

	targetSpeed = LEECH_SWIM_SPEED;

	if (m_waterTime < CURTIME)
		RecalculateWaterlevel();

	if (random->RandomInt(0, 100) < 10)
		targetYaw = random->RandomInt(-30, 30);

	// oldorigin test
	if ((GetLocalOrigin() - m_oldOrigin).Length() < 1)
	{
		// If leech didn't move, there must be something blocking it, so try to turn
		m_sideTime = 0;
	}

	Vector vForward, vRight;

	AngleVectors(GetAbsAngles(), &vForward, &vRight, NULL);

	if (m_flTurning == 0)// something in the way and leech is not already turning to avoid
	{
		Vector vecTest;
		// measure clearance on left and right to pick the best dir to turn
		vecTest = GetLocalOrigin() + (vRight * LEECH_SIZEX) + (vForward * LEECH_CHECK_DIST);
		UTIL_TraceLine(GetLocalOrigin(), vecTest, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);
		flRightSide = tr.fraction;

		vecTest = GetLocalOrigin() + (vRight * -LEECH_SIZEX) + (vForward * LEECH_CHECK_DIST);
		UTIL_TraceLine(GetLocalOrigin(), vecTest, MASK_SOLID, this, COLLISION_GROUP_NONE, &tr);

		flLeftSide = tr.fraction;

		// turn left, right or random depending on clearance ratio
		float delta = (flRightSide - flLeftSide);
		if (delta > 0.1 || (delta > -0.1 && random->RandomInt(0, 100) < 50))
			m_flTurning = -LEECH_TURN_RATE;
		else
			m_flTurning = LEECH_TURN_RATE;
	}

	m_flSpeed = UTIL_Approach(-(LEECH_SWIM_SPEED*0.5), m_flSpeed, LEECH_SWIM_DECEL * LEECH_FRAMETIME);
	SetAbsVelocity(vForward * m_flSpeed);

	GetMotor()->SetIdealYaw(m_flTurning);
	UpdateMotion();
}

void CNPC_Parasite::UpdateMotion(void)
{
	float flapspeed = (m_flSpeed - m_flAccelerate) / LEECH_ACCELERATE;
	m_flAccelerate = m_flAccelerate * 0.8 + m_flSpeed * 0.2;

	if (flapspeed < 0)
		flapspeed = -flapspeed;
	flapspeed += 1.0;
	if (flapspeed < 0.5)
		flapspeed = 0.5;
	if (flapspeed > 1.9)
		flapspeed = 1.9;

	m_flPlaybackRate = flapspeed;

	QAngle vAngularVelocity = GetLocalAngularVelocity();
	QAngle vAngles = GetLocalAngles();

	vAngularVelocity.y = GetMotor()->GetIdealYaw();

	if (vAngularVelocity.y > 150)
		SetIdealActivity(ACT_TURN_LEFT);
	else if (vAngularVelocity.y < -150)
		SetIdealActivity(ACT_TURN_RIGHT);
	else
		SetIdealActivity(ACT_SWIM);

	// lean
	float targetPitch, delta;
	delta = m_height - GetLocalOrigin().z;

	/*	if ( delta < -10 )
	targetPitch = -30;
	else if ( delta > 10 )
	targetPitch = 30;
	else*/
	targetPitch = 0;

	vAngles.x = UTIL_Approach(targetPitch, vAngles.x, 60 * LEECH_FRAMETIME);

	// bank
	vAngularVelocity.z = -(vAngles.z + (vAngularVelocity.y * 0.25));

	if (m_NPCState == NPC_STATE_COMBAT && HasCondition(COND_CAN_MELEE_ATTACK1))
		SetIdealActivity(ACT_MELEE_ATTACK1);

	// Out of water check
	if (!GetWaterLevel())
	{
		SetMoveType(MOVETYPE_FLYGRAVITY);
		SetIdealActivity(ACT_HOP);
		SetAbsVelocity(vec3_origin);

		// Animation will intersect the floor if either of these is non-zero
		vAngles.z = 0;
		vAngles.x = 0;

		if (m_flPlaybackRate < 1.0)
			m_flPlaybackRate = 1.0;
	}
	else if (GetMoveType() == MOVETYPE_FLYGRAVITY)
	{
		SetMoveType(MOVETYPE_FLY);
		RemoveFlag(FL_ONGROUND);

		//  TODO
		RecalculateWaterlevel();
		m_waterTime = CURTIME + 2;	// Recalc again soon, water may be rising
	}

	if (GetActivity() != GetIdealActivity())
	{
		SetActivity(GetIdealActivity());
	}
	StudioFrameAdvance();

	DispatchAnimEvents(this);

	SetLocalAngles(vAngles);
	SetLocalAngularVelocity(vAngularVelocity);

	Vector vForward, vRight;

	AngleVectors(vAngles, &vForward, &vRight, NULL);
}