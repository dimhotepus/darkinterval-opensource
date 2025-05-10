//========================================================================//
// DI changes that are not ifdef'd: 
// - removed HLS stuff
//=============================================================================//

#include "cbase.h"
#include "ai_hull.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

struct ai_hull_t
{
	ai_hull_t( int bit, const char *pName, const Vector &_mins, const Vector &_maxs, const Vector &_smallMins, const Vector &_smallMaxs )
		: hullBit( bit ), mins( _mins ), maxs( _maxs ), smallMins( _smallMins ), smallMaxs( _smallMaxs ), name( pName ) {}
	int			hullBit;
	const char*	name;

	Vector	mins;
	Vector	maxs;

	Vector	smallMins;
	Vector	smallMaxs;
};

//=================================================================================
// Create the hull types here.
//=================================================================================
ai_hull_t  Human_Hull			(bits_HUMAN_HULL,			"HUMAN_HULL",			Vector(-13,-13,   0),	Vector(13, 13, 72),		Vector(-8,-8,   0),		Vector( 8,  8, 72) );
ai_hull_t  Small_Centered_Hull	(bits_SMALL_CENTERED_HULL,	"SMALL_CENTERED_HULL",	Vector(-20,-20, -20),	Vector(20, 20, 20),		Vector(-12,-12,-12),	Vector(12, 12, 12) );
ai_hull_t  Wide_Human_Hull		(bits_WIDE_HUMAN_HULL,		"WIDE_HUMAN_HULL",		Vector(-15,-15,   0),	Vector(15, 15, 72),		Vector(-10,-10, 0),		Vector(10, 10, 72) );
ai_hull_t  Tiny_Hull			(bits_TINY_HULL,			"TINY_HULL",			Vector(-12,-12,   0),	Vector(12, 12, 24),		Vector(-12,-12, 0),	    Vector(12, 12, 24) );
ai_hull_t  Wide_Short_Hull		(bits_WIDE_SHORT_HULL,		"WIDE_SHORT_HULL",		Vector(-35,-35,   0),	Vector(35, 35, 32),		Vector(-20,-20, 0),	    Vector(20, 20, 32) );
ai_hull_t  Medium_Hull			(bits_MEDIUM_HULL,			"MEDIUM_HULL",			Vector(-16,-16,   0),	Vector(16, 16, 64),		Vector(-8,-8, 0),	    Vector(8, 8, 64) );
ai_hull_t  Tiny_Centered_Hull	(bits_TINY_CENTERED_HULL,	"TINY_CENTERED_HULL",	Vector(-8,	-8,  -4),	Vector(8, 8,  4),		Vector(-8,-8, -4),		Vector( 8, 8, 4) );
ai_hull_t  Large_Hull			(bits_LARGE_HULL,			"LARGE_HULL",			Vector(-40,-40,   0),	Vector(40, 40, 100),	Vector(-40,-40, 0),		Vector(40, 40, 100) );
ai_hull_t  Large_Centered_Hull	(bits_LARGE_CENTERED_HULL,	"LARGE_CENTERED_HULL",	Vector(-38,-38, -38),	Vector(38, 38, 38),		Vector(-30,-30,-30),	Vector(30, 30, 30) );
ai_hull_t  Medium_Tall_Hull		(bits_MEDIUM_TALL_HULL,		"MEDIUM_TALL_HULL",		Vector(-18,-18,   0),	Vector(18, 18, 100),	Vector(-12,-12, 0),	    Vector(12, 12, 100) );
#ifdef DARKINTERVAL
ai_hull_t  Houndeye_Hull		(bits_HOUNDEYE_HULL,		"HOUNDEYE_HULL",		Vector(-20, -15, 0),	Vector(23, 15, 40),		Vector(-12, -10, 0),	Vector(12, 10, 30));
ai_hull_t  Crabsynth_Hull		(bits_CRABSYNTH_HULL,		"CRABSYNTH_HULL",		Vector(-48, -56, 0),	Vector(80, 56, 96),		Vector(-40, -40, 0),	Vector(40, 40, 100)); // note: "small" values for crabsynth are equal to "normal" values for HULL_LARGE (antlionguard hull).
ai_hull_t  Advisor_Hull			(bits_ADVISOR_HULL,			"ADVISOR_HULL",			Vector(-128, -96, 0),	Vector(128, 96, 272),	Vector(-112, -80, 0),	Vector(112, 80, 224));
ai_hull_t  Cremator_Hull		(bits_CREMATOR_HULL,		"CREMATOR_HULL",		Vector(-16, -16, 0),	Vector(12, 16, 92),		Vector(-12, -12, 0),	Vector(12, 12, 92));
ai_hull_t  GliderSynth_Hull		(bits_GLIDERSYNTH_HULL,		"GLIDERSYNTH_HULL",		Vector(-50, -32, 36),	Vector(38, 32, 86),		Vector(-30, -30, 40),	Vector(30, 30, 80));
ai_hull_t  WSuid_Hull			(bits_WATERSQUID_HULL,		"WATERSQUID_HULL",		Vector(-40, -16, -64),	Vector(32, 16, 8),		Vector(-32, -10, -64),	Vector(10, 10, 8));
ai_hull_t  Hopper_Hull			(bits_HOPPER_HULL,			"HOPPER_HULL",			Vector(-14, -14, 0),	Vector(14, 14, 40),		Vector(-12, -12, -12), Vector(12, 12, 12));
#endif
//
// Array of hulls. These hulls must correspond with the enumerations in AI_Hull.h!
//
ai_hull_t*	hull[NUM_HULLS] =	
{
	&Human_Hull,
	&Small_Centered_Hull,
	&Wide_Human_Hull,
	&Tiny_Hull,
	&Wide_Short_Hull,
	&Medium_Hull,
	&Tiny_Centered_Hull,
	&Large_Hull,
	&Large_Centered_Hull,
	&Medium_Tall_Hull,
#ifdef DARKINTERVAL
	&Houndeye_Hull,
	&Crabsynth_Hull,
	&Advisor_Hull,
	&Cremator_Hull,
	&GliderSynth_Hull,
	&WSuid_Hull,
	&Hopper_Hull,
#endif
};


//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
const Vector &NAI_Hull::Mins(int id)			
{ 
	return hull[id]->mins;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
const Vector &NAI_Hull::Maxs(int id)			
{ 
	return hull[id]->maxs;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
const Vector &NAI_Hull::SmallMins(int id)			
{ 
	return hull[id]->smallMins;
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
const Vector &NAI_Hull::SmallMaxs(int id)			
{ 
	return hull[id]->smallMaxs;
}

//-----------------------------------------------------------------------------
// Purpose:

// Input  :
// Output :
//-----------------------------------------------------------------------------
float NAI_Hull::Length(int id)
{
	return (hull[id]->maxs.x - hull[id]->mins.x); 
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
float NAI_Hull::Width(int id)
{
	return (hull[id]->maxs.y - hull[id]->mins.y);
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
float NAI_Hull::Height(int id)
{
	return (hull[id]->maxs.z - hull[id]->mins.z); 
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
int NAI_Hull::Bits(int id)			
{ 
	return hull[id]->hullBit;		
}

//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
const char *NAI_Hull::Name(int id)	
{ 
	return hull[id]->name;	
}


//-----------------------------------------------------------------------------
// Purpose:
// Input  :
// Output :
//-----------------------------------------------------------------------------
Hull_t NAI_Hull::LookupId(const char *szName)	
{ 
	int i;
	if (!szName)
	{
		return HULL_HUMAN;
	}
	for (i = 0; i < NUM_HULLS; i++)
	{
		if (stricmp( szName, NAI_Hull::Name( i )) == 0)
		{
			return (Hull_t)i;
		}
	}
	return HULL_HUMAN;
}



