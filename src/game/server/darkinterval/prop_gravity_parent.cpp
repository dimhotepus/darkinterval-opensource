// This is an entity that you can attach prop_physcs
// (or func_physbox, anything physically simulated like that)
// and have it be affected by trigger_gravity (look in triggers.cpp
// for a fix that lets it affect entities other than the player.)
//
// Because physics props are actually a crappily coded gimmick,
// they NEVER get SetGravity() to work on them.
// They are NEVER affected by trigger_gravity and such.
//
// But trigger gravity can affect BaseCombatCharacters, things like
// flaregun's flares, crossbow bolts and the like.
// So, we are going to create an invisible point entity 
// that moves under gravity, and let it be a parent
// of our physiscs prop. So, basically a helper
// (helping against once again radished Source engine).
// 
// Also, this entity is not going to use EF_NODRAW, 
// because entities that aren't drawn don't exist and 
// can't be targeted. Source = radishedness. It's actually true -
// if an entity is nodraw, it stops being synched. 
// So the remedy against that is render mode 'none'. 
//
// Of note, normally, one drawback of this being a CBaseCombatCharacter
// is it can't pass MASK_NPCSOLID, like npc clips and such.
// This forever has been an issue in HL2 - 
// grenades, crossbow bolts, flares can't pass clips.
// Moronic, like the whole rest of the engine. Because it'scatnip.
// Anyway, this is addressed by including that 
// PhysicsSolidMaskForEntity() bit. 

#include "cbase.h"
#include "basecombatcharacter.h"

class CGravityParent : public CBaseCombatCharacter
{
public:
	DECLARE_CLASS(CGravityParent, CBaseCombatCharacter);

	CGravityParent();
	~CGravityParent();
	
	void	Spawn(void);
	void	Precache(void);
	int		Restore(IRestore &restore);
	void	Activate(void);
	void	WakeUp(inputdata_t &inputdata);

	virtual unsigned int PhysicsSolidMaskForEntity(void) const;
	
	Class_T Classify(void);
	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(prop_gravity_parent, CGravityParent);

BEGIN_DATADESC(CGravityParent)
DEFINE_INPUTFUNC(FIELD_VOID, "WakeUp", WakeUp),
END_DATADESC()


Class_T CGravityParent::Classify(void)
{
	return CLASS_NONE;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
CGravityParent::CGravityParent(void)
{
}

CGravityParent::~CGravityParent()
{
}

void CGravityParent::Precache(void)
{
	PrecacheModel("models/weapons/flare.mdl"); // it's gonna use this model, but be invisible.
}

int CGravityParent::Restore(IRestore &restore)
{
	int result = BaseClass::Restore(restore);
	return result;
}

void CGravityParent::Spawn(void)
{
	Precache();
	SetModel("models/props_junk/flare.mdl");
	UTIL_SetSize(this, Vector(-4, -4, -4), Vector(4, 4, 4));

	SetSolid(SOLID_BBOX);
	SetMoveType(MOVETYPE_FLYGRAVITY);
	SetFriction(0.5f);
	SetGravity(UTIL_ScaleForGravity(600));

	AddEffects(EF_NOSHADOW | EF_NORECEIVESHADOW);

	m_nRenderMode = kRenderNone;
	
	AddFlag(FL_VISIBLE_TO_NPCS);

	AddSolidFlags(FSOLID_NOT_STANDABLE);
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
	SetGravity(1.0f);
}

void CGravityParent::Activate(void)
{
	BaseClass::Activate();
}

unsigned int CGravityParent::PhysicsSolidMaskForEntity(void) const
{
	return MASK_SOLID;
}

void CGravityParent::WakeUp(inputdata_t &inputdata)
{
	SetMoveType(MOVETYPE_FLYGRAVITY, MOVECOLLIDE_FLY_BOUNCE);
	SetGravity(1.0f);
	SetAbsVelocity(Vector(0,0,8));
}