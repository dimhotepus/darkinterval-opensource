#include "shaderlib/cshader.h"
class fluid_ps20b_Static_Index
{
private:
	int m_nCUBEMAP;
#ifdef _DEBUG
	bool m_bCUBEMAP;
#endif
public:
	void SetCUBEMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nCUBEMAP = i;
#ifdef _DEBUG
		m_bCUBEMAP = true;
#endif
	}
	void SetCUBEMAP( bool i )
	{
		m_nCUBEMAP = i ? 1 : 0;
#ifdef _DEBUG
		m_bCUBEMAP = true;
#endif
	}
private:
	int m_nNORMALMAPALPHAENVMAPMASK;
#ifdef _DEBUG
	bool m_bNORMALMAPALPHAENVMAPMASK;
#endif
public:
	void SetNORMALMAPALPHAENVMAPMASK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nNORMALMAPALPHAENVMAPMASK = i;
#ifdef _DEBUG
		m_bNORMALMAPALPHAENVMAPMASK = true;
#endif
	}
	void SetNORMALMAPALPHAENVMAPMASK( bool i )
	{
		m_nNORMALMAPALPHAENVMAPMASK = i ? 1 : 0;
#ifdef _DEBUG
		m_bNORMALMAPALPHAENVMAPMASK = true;
#endif
	}
private:
	int m_nHALFLAMBERT;
#ifdef _DEBUG
	bool m_bHALFLAMBERT;
#endif
public:
	void SetHALFLAMBERT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nHALFLAMBERT = i;
#ifdef _DEBUG
		m_bHALFLAMBERT = true;
#endif
	}
	void SetHALFLAMBERT( bool i )
	{
		m_nHALFLAMBERT = i ? 1 : 0;
#ifdef _DEBUG
		m_bHALFLAMBERT = true;
#endif
	}
private:
	int m_nDEPTHBLEND;
#ifdef _DEBUG
	bool m_bDEPTHBLEND;
#endif
public:
	void SetDEPTHBLEND( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDEPTHBLEND = i;
#ifdef _DEBUG
		m_bDEPTHBLEND = true;
#endif
	}
	void SetDEPTHBLEND( bool i )
	{
		m_nDEPTHBLEND = i ? 1 : 0;
#ifdef _DEBUG
		m_bDEPTHBLEND = true;
#endif
	}
public:
	fluid_ps20b_Static_Index( )
	{
#ifdef _DEBUG
		m_bCUBEMAP = false;
#endif // _DEBUG
		m_nCUBEMAP = 0;
#ifdef _DEBUG
		m_bNORMALMAPALPHAENVMAPMASK = false;
#endif // _DEBUG
		m_nNORMALMAPALPHAENVMAPMASK = 0;
#ifdef _DEBUG
		m_bHALFLAMBERT = false;
#endif // _DEBUG
		m_nHALFLAMBERT = 0;
#ifdef _DEBUG
		m_bDEPTHBLEND = false;
#endif // _DEBUG
		m_nDEPTHBLEND = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bCUBEMAP && m_bNORMALMAPALPHAENVMAPMASK && m_bHALFLAMBERT && m_bDEPTHBLEND;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 10 * m_nCUBEMAP ) + ( 20 * m_nNORMALMAPALPHAENVMAPMASK ) + ( 40 * m_nHALFLAMBERT ) + ( 80 * m_nDEPTHBLEND ) + 0;
	}
};
#define shaderStaticTest_fluid_ps20b psh_forgot_to_set_static_CUBEMAP + psh_forgot_to_set_static_NORMALMAPALPHAENVMAPMASK + psh_forgot_to_set_static_HALFLAMBERT + psh_forgot_to_set_static_DEPTHBLEND + 0
class fluid_ps20b_Dynamic_Index
{
private:
	int m_nNUM_LIGHTS;
#ifdef _DEBUG
	bool m_bNUM_LIGHTS;
#endif
public:
	void SetNUM_LIGHTS( int i )
	{
		Assert( i >= 0 && i <= 4 );
		m_nNUM_LIGHTS = i;
#ifdef _DEBUG
		m_bNUM_LIGHTS = true;
#endif
	}
	void SetNUM_LIGHTS( bool i )
	{
		m_nNUM_LIGHTS = i ? 1 : 0;
#ifdef _DEBUG
		m_bNUM_LIGHTS = true;
#endif
	}
private:
	int m_nAMBIENT_LIGHT;
#ifdef _DEBUG
	bool m_bAMBIENT_LIGHT;
#endif
public:
	void SetAMBIENT_LIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nAMBIENT_LIGHT = i;
#ifdef _DEBUG
		m_bAMBIENT_LIGHT = true;
#endif
	}
	void SetAMBIENT_LIGHT( bool i )
	{
		m_nAMBIENT_LIGHT = i ? 1 : 0;
#ifdef _DEBUG
		m_bAMBIENT_LIGHT = true;
#endif
	}
public:
	fluid_ps20b_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bNUM_LIGHTS = false;
#endif // _DEBUG
		m_nNUM_LIGHTS = 0;
#ifdef _DEBUG
		m_bAMBIENT_LIGHT = false;
#endif // _DEBUG
		m_nAMBIENT_LIGHT = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bNUM_LIGHTS && m_bAMBIENT_LIGHT;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nNUM_LIGHTS ) + ( 5 * m_nAMBIENT_LIGHT ) + 0;
	}
};
#define shaderDynamicTest_fluid_ps20b psh_forgot_to_set_dynamic_NUM_LIGHTS + psh_forgot_to_set_dynamic_AMBIENT_LIGHT + 0
