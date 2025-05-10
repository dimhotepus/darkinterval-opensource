#include "cbase.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
class CEnvDisplacement : public CPointEntity
{
	DECLARE_CLASS(CEnvDisplacement, CPointEntity);
public:
	DECLARE_DATADESC();
	DECLARE_SERVERCLASS();

	CEnvDisplacement();
	~CEnvDisplacement();

	void	Precache(void);
	void	Spawn(void);
	int		DrawDebugTextOverlays(void);

	CNetworkVar(int, m_targetdisp_index_int);
};

BEGIN_DATADESC(CEnvDisplacement)
DEFINE_KEYFIELD(m_targetdisp_index_int, FIELD_INTEGER, "targetdisp"),
END_DATADESC()

LINK_ENTITY_TO_CLASS(env_displacement, CEnvDisplacement);

IMPLEMENT_SERVERCLASS_ST(CEnvDisplacement, DT_EnvDisplacement)
	SendPropInt(SENDINFO(m_targetdisp_index_int), 16, SPROP_UNSIGNED),
END_SEND_TABLE()

CEnvDisplacement::CEnvDisplacement()
{
}

CEnvDisplacement::~CEnvDisplacement()
{
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvDisplacement::Precache(void)
{
	BaseClass::Precache();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CEnvDisplacement::Spawn(void)
{
	Precache();
	UTIL_SetSize(this, vec3_origin, vec3_origin);
	AddSolidFlags(FSOLID_NOT_SOLID);
	SetMoveType(MOVETYPE_NONE);
	AddEFlags(EFL_FORCE_CHECK_TRANSMIT);

}

int CEnvDisplacement::DrawDebugTextOverlays(void)
{
	int text_offset = BaseClass::DrawDebugTextOverlays();

	if (m_debugOverlays & OVERLAY_TEXT_BIT)
	{
		char tempstr[512];

		Q_snprintf(tempstr, sizeof(tempstr), "    Target displacement index: %i", m_targetdisp_index_int);
		EntityText(text_offset, tempstr, 0);
		text_offset++;
	}
	return text_offset;
}
