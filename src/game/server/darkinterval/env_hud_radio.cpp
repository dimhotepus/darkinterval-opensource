#include "cbase.h"
#include "engine/ienginesound.h"
#include "baseentity.h"
#include "entityoutput.h"
#include "recipientfilter.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class EnvHudRadio : public CPointEntity
{
public:
	DECLARE_CLASS(EnvHudRadio, CPointEntity);

	void	Spawn(void);
	void	Precache(void);

private:
	inline	bool	AllPlayers(void) { return true; }

	void InputShowHudRadio(inputdata_t &inputdata);
	void InputHideHudRadio(inputdata_t &inputdata);
	string_t m_iszMessage;
	float m_hud_lifetime_float;
	DECLARE_DATADESC();
};

LINK_ENTITY_TO_CLASS(env_hud_radio, EnvHudRadio);

BEGIN_DATADESC(EnvHudRadio)

DEFINE_KEYFIELD(m_iszMessage, FIELD_STRING, "message"),
DEFINE_KEYFIELD(m_hud_lifetime_float, FIELD_FLOAT, "hud_lifetime"),
DEFINE_INPUTFUNC(FIELD_FLOAT, "ShowHudRadio", InputShowHudRadio),
DEFINE_INPUTFUNC(FIELD_VOID, "HideHudRadio", InputHideHudRadio),

END_DATADESC()



//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void EnvHudRadio::Spawn(void)
{
	Precache();

	SetSolid(SOLID_NONE);
	SetMoveType(MOVETYPE_NONE);
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void EnvHudRadio::Precache(void)
{
}

//-----------------------------------------------------------------------------
// Purpose: Input handler for showing the message and/or playing the sound.
//-----------------------------------------------------------------------------
void EnvHudRadio::InputShowHudRadio(inputdata_t &inputdata)
{
	if (AllPlayers())
	{
		CReliableBroadcastRecipientFilter user;
		UserMessageBegin(user, "SuitRadio");
		WRITE_BYTE(1);	// one message
		WRITE_FLOAT(inputdata.value.Float() > 0 ? inputdata.value.Float() : m_hud_lifetime_float); // length of icon's lifetime
		WRITE_STRING(STRING(m_iszMessage));
		MessageEnd();
	}
	else
	{
		CBaseEntity *pPlayer = NULL;
		if (inputdata.pActivator && inputdata.pActivator->IsPlayer())
		{
			pPlayer = inputdata.pActivator;
		}
		else
		{
			pPlayer = UTIL_GetLocalPlayer();
		}

		if (!pPlayer || !pPlayer->IsNetClient())
			return;

		CSingleUserRecipientFilter user((CBasePlayer *)pPlayer);
		user.MakeReliable();
		UserMessageBegin(user, "SuitRadio");
		WRITE_BYTE(1);	// one message
		WRITE_FLOAT(inputdata.value.Float() > 0 ? inputdata.value.Float() : m_hud_lifetime_float); // length of icon's lifetime
		WRITE_STRING(STRING(m_iszMessage));
		MessageEnd();
	}
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void EnvHudRadio::InputHideHudRadio(inputdata_t &inputdata)
{
	if (AllPlayers())
	{
		CReliableBroadcastRecipientFilter user;
		UserMessageBegin(user, "SuitRadio");
		WRITE_BYTE(1);	// one message
		WRITE_FLOAT(0); // length of icon's lifetime
		WRITE_STRING(STRING(NULL_STRING));
		MessageEnd();
	}
	else
	{
		CBaseEntity *pPlayer = NULL;

		if (inputdata.pActivator && inputdata.pActivator->IsPlayer())
		{
			pPlayer = inputdata.pActivator;
		}
		else
		{
			pPlayer = UTIL_GetLocalPlayer();
		}

		if (!pPlayer || !pPlayer->IsNetClient())
			return;

		CSingleUserRecipientFilter user((CBasePlayer *)pPlayer);
		user.MakeReliable();
		UserMessageBegin(user, "SuitRadio");
		WRITE_BYTE(1);	// one message
		WRITE_FLOAT(0); // length of icon's lifetime
		WRITE_STRING(STRING(NULL_STRING));
		MessageEnd();
	}
}
