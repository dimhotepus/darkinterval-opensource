#include "shaderlib/cshader.h"
class di_depth_of_field_ps20b_Static_Index
{
public:
	di_depth_of_field_ps20b_Static_Index( )
	{
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
#endif // _DEBUG
		return 0;
	}
};
#define shaderStaticTest_di_depth_of_field_ps20b 0
class di_depth_of_field_ps20b_Dynamic_Index
{
private:
	int m_nPIXELFOGTYPE;
#ifdef _DEBUG
	bool m_bPIXELFOGTYPE;
#endif
public:
	void SetPIXELFOGTYPE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nPIXELFOGTYPE = i;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = true;
#endif
	}
	void SetPIXELFOGTYPE( bool i )
	{
		m_nPIXELFOGTYPE = i ? 1 : 0;
#ifdef _DEBUG
		m_bPIXELFOGTYPE = true;
#endif
	}
private:
	int m_nVISUALIZE_DOF;
#ifdef _DEBUG
	bool m_bVISUALIZE_DOF;
#endif
public:
	void SetVISUALIZE_DOF( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nVISUALIZE_DOF = i;
#ifdef _DEBUG
		m_bVISUALIZE_DOF = true;
#endif
	}
	void SetVISUALIZE_DOF( bool i )
	{
		m_nVISUALIZE_DOF = i ? 1 : 0;
#ifdef _DEBUG
		m_bVISUALIZE_DOF = true;
#endif
	}
public:
	di_depth_of_field_ps20b_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bPIXELFOGTYPE = false;
#endif // _DEBUG
		m_nPIXELFOGTYPE = 0;
#ifdef _DEBUG
		m_bVISUALIZE_DOF = false;
#endif // _DEBUG
		m_nVISUALIZE_DOF = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bPIXELFOGTYPE && m_bVISUALIZE_DOF;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nPIXELFOGTYPE ) + ( 2 * m_nVISUALIZE_DOF ) + 0;
	}
};
#define shaderDynamicTest_di_depth_of_field_ps20b psh_forgot_to_set_dynamic_PIXELFOGTYPE + psh_forgot_to_set_dynamic_VISUALIZE_DOF + 0
