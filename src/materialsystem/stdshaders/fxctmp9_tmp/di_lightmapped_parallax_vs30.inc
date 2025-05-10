#include "shaderlib/cshader.h"
class di_lightmapped_parallax_vs30_Static_Index
{
private:
	int m_nDOTPRODUCTFIX;
#ifdef _DEBUG
	bool m_bDOTPRODUCTFIX;
#endif
public:
	void SetDOTPRODUCTFIX( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOTPRODUCTFIX = i;
#ifdef _DEBUG
		m_bDOTPRODUCTFIX = true;
#endif
	}
	void SetDOTPRODUCTFIX( bool i )
	{
		m_nDOTPRODUCTFIX = i ? 1 : 0;
#ifdef _DEBUG
		m_bDOTPRODUCTFIX = true;
#endif
	}
public:
	di_lightmapped_parallax_vs30_Static_Index( )
	{
#ifdef _DEBUG
		m_bDOTPRODUCTFIX = false;
#endif // _DEBUG
		m_nDOTPRODUCTFIX = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bDOTPRODUCTFIX;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 2 * m_nDOTPRODUCTFIX ) + 0;
	}
};
#define shaderStaticTest_di_lightmapped_parallax_vs30 vsh_forgot_to_set_static_DOTPRODUCTFIX + 0
class di_lightmapped_parallax_vs30_Dynamic_Index
{
private:
	int m_nDOWATERFOG;
#ifdef _DEBUG
	bool m_bDOWATERFOG;
#endif
public:
	void SetDOWATERFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOWATERFOG = i;
#ifdef _DEBUG
		m_bDOWATERFOG = true;
#endif
	}
	void SetDOWATERFOG( bool i )
	{
		m_nDOWATERFOG = i ? 1 : 0;
#ifdef _DEBUG
		m_bDOWATERFOG = true;
#endif
	}
public:
	di_lightmapped_parallax_vs30_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bDOWATERFOG = false;
#endif // _DEBUG
		m_nDOWATERFOG = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bDOWATERFOG;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nDOWATERFOG ) + 0;
	}
};
#define shaderDynamicTest_di_lightmapped_parallax_vs30 vsh_forgot_to_set_dynamic_DOWATERFOG + 0
