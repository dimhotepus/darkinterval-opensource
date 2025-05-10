#include "cbase.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// CEnvSkytexture
//-----------------------------------------------------------------------------
class CEnvSkytexture : public CServerOnlyPointEntity
{
	DECLARE_CLASS(CEnvSkytexture, CServerOnlyPointEntity);
public:
	DECLARE_DATADESC();

	virtual void Spawn(void);
	virtual void Precache(void);

	void InputTrigger(inputdata_t &inputdata);

protected:
	string_t m_iszSkyboxName;
};

LINK_ENTITY_TO_CLASS(env_skytexture, CEnvSkytexture);

BEGIN_DATADESC(CEnvSkytexture)
DEFINE_KEYFIELD(m_iszSkyboxName, FIELD_STRING, "SkyboxName"),
// Inputs
DEFINE_INPUTFUNC(FIELD_VOID, "Trigger", InputTrigger),
END_DATADESC()


//-----------------------------------------------------------------------------
// Purpose: not much
//-----------------------------------------------------------------------------
void CEnvSkytexture::Spawn(void)
{
	Precache();
}

//-----------------------------------------------------------------------------
// Purpose: Precache the skybox materials.
//-----------------------------------------------------------------------------
void CEnvSkytexture::Precache(void)
{
	if (Q_strlen(m_iszSkyboxName.ToCStr()) == 0)
	{
		Warning("Env_skytexture (%s) has no skybox specified!\n", GetEntityName());
		return;
	}

	char  name[MAX_PATH];
	char *skyboxsuffix[6] = { "rt", "bk", "lf", "ft", "up", "dn" };
	for (int i = 0; i < 6; i++)
	{
		Q_snprintf(name, sizeof(name), "skybox/%s%s", m_iszSkyboxName.ToCStr(), skyboxsuffix[i]);
		PrecacheMaterial(name);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Input handler that triggers the skybox swap.
//-----------------------------------------------------------------------------
void CEnvSkytexture::InputTrigger(inputdata_t &inputdata)
{
	static ConVarRef skyname("sv_skyname", false);
	if (!skyname.IsValid())
	{
		Warning("Env_skytexture (%s) trigger input failed - cannot find 'sv_skyname' convar!\n", GetEntityName());
		return;
	}
	skyname.SetValue(m_iszSkyboxName.ToCStr());
}