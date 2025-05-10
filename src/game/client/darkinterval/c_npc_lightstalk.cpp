//========= Copyright © 2017, Dark Interval.==================================//
//
// Purpose: 
//
//============================================================================//

#include "cbase.h"
#include "c_ai_basenpc.h"
#include "dlight.h"
#include "iefx.h"

#if 1
class C_NPC_LightFungi : public C_AI_BaseNPC
{
	DECLARE_CLASS(C_NPC_LightFungi, C_AI_BaseNPC)
public:
	DECLARE_CLIENTCLASS();

	C_NPC_LightFungi(void);
public:
	bool	m_bLight;
	int		m_lightVars[4];
	float	m_LightRadius;
public:
	void OnSave()
	{
		BaseClass::OnSave();
	}

	void OnDataChanged(DataUpdateType_t updateType)
	{
		BaseClass::OnDataChanged(updateType);
		if (updateType == DATA_UPDATE_CREATED) CreateDLight();
	}

	void OnRestore()
	{
		CreateDLight();
		BaseClass::OnRestore();
	}

	void	CreateDLight(void);

private:
	C_NPC_LightFungi(const C_NPC_LightFungi &);

	int		m_iAttachment;
};

IMPLEMENT_CLIENTCLASS_DT(C_NPC_LightFungi, DT_NPC_LightFungi, CNPC_LightFungi)
RecvPropBool(RECVINFO(m_bLight)),
//RecvPropInt(RECVINFO(m_lightr)),
//RecvPropInt(RECVINFO(m_lightg)),
//RecvPropInt(RECVINFO(m_lightb)),
//RecvPropInt(RECVINFO(m_lighta)),
RecvPropArray(RecvPropInt(RECVINFO(m_lightVars[0])), m_lightVars),
RecvPropFloat(RECVINFO(m_LightRadius)),
END_RECV_TABLE()

// Constructor

C_NPC_LightFungi::C_NPC_LightFungi()
{
	m_iAttachment = 1;
}

void C_NPC_LightFungi::CreateDLight(void)
{
	if (m_bLight && m_LightRadius > 1)
	{
		m_iAttachment = LookupAttachment("0");

		Vector effect_origin;
		QAngle effect_angles;

		GetAttachment(m_iAttachment, effect_origin, effect_angles);

		dlight_t *dl = effects->CL_AllocDlight(index);

		dl->origin = effect_origin + Vector(0, 0, 8);

		dl->color.r = m_lightVars[0];//m_lightr;
		dl->color.g = m_lightVars[1];//m_lightg;
		dl->color.b = m_lightVars[2];//m_lightb;
		dl->color.exponent = m_lightVars[3] / 100; //m_lighta / 100;

		dl->radius = m_LightRadius;
		dl->die = CURTIME + FLT_MAX;
/*
		dlight_t *el = effects->CL_AllocElight(index);

		el->origin = effect_origin + Vector(0, 0, 2);

		el->color.r = m_lightr;
		el->color.g = m_lightg;
		el->color.b = m_lightb;
		el->color.exponent = 1.5;

		el->radius = m_LightRadius; //RandomFloat(200.0f, 256.0f);
		el->die = CURTIME + 0.15f;
*/
	//	m_bLight = true;
	}
}
#endif


class C_NPC_LightStalk : public C_AI_BaseNPC
{
	DECLARE_CLASS( C_NPC_LightStalk, C_AI_BaseNPC )
public:
	DECLARE_CLIENTCLASS();

	C_NPC_LightStalk( void );
	
	bool	m_bLight;
	int		m_lightr;
	int		m_lightg;
	int		m_lightb;
	float 	m_LightRadius;

	void OnDataChanged( DataUpdateType_t updateType )
	{
		BaseClass::OnDataChanged( updateType );

		if( updateType == DATA_UPDATE_DATATABLE_CHANGED) Think();
	}

	void OnRestore()
	{
		Think();
		BaseClass::OnRestore();
	}

	void	Think( void );

private:	
	C_NPC_LightStalk( const C_NPC_LightStalk & );

	int		m_iAttachment;
};

IMPLEMENT_CLIENTCLASS_DT( C_NPC_LightStalk, DT_NPC_LightStalk, CNPC_LightStalk)
	RecvPropBool( RECVINFO( m_bLight ) ),
	RecvPropInt( RECVINFO( m_lightr ) ),
	RecvPropInt( RECVINFO( m_lightg ) ),
	RecvPropInt( RECVINFO( m_lightb ) ),
	RecvPropFloat(RECVINFO(m_LightRadius)),
END_RECV_TABLE()

// Constructor

C_NPC_LightStalk::C_NPC_LightStalk()
{
//	m_bLight = true;
	m_iAttachment = 1;
}

void C_NPC_LightStalk::Think( void )
{
	if ( !m_bLight && m_LightRadius > 1)
	{
		m_iAttachment = LookupAttachment( "0" );
		
		Vector effect_origin;
		QAngle effect_angles;
				
		GetAttachment( m_iAttachment, effect_origin, effect_angles );

		dlight_t *dl= effects->CL_AllocDlight( index );	

		dl->origin	= effect_origin + Vector(0,0,2);

		dl->color.r = m_lightr;
		dl->color.g = m_lightg;
		dl->color.b = m_lightb;
		dl->color.exponent = 1.75;

		dl->radius = m_LightRadius;//RandomFloat( 200.0f, 256.0f );
		dl->die		= CURTIME + 0.2f;
/*		
		dlight_t *el= effects->CL_AllocElight( index );

		el->origin	= effect_origin + Vector(0,0,2);

		el->color.r = m_lightr;
		el->color.g = m_lightg;
		el->color.b = m_lightb;
		el->color.exponent = 1.5;

		el->radius	= m_LightRadius;//RandomFloat( 200.0f, 256.0f );
		el->die		= CURTIME + 0.2f;
*/
	//	m_bLight = true;
	}
}

