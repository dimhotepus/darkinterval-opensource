#include "shaderlib/cshader.h"
class post_ambientocclusion_ps30_Static_Index
{
public:
	post_ambientocclusion_ps30_Static_Index( )
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
#define shaderStaticTest_post_ambientocclusion_ps30 0
class post_ambientocclusion_ps30_Dynamic_Index
{
private:
	int m_nAO_MODE;
#ifdef _DEBUG
	bool m_bAO_MODE;
#endif
public:
	void SetAO_MODE( int i )
	{
		Assert( i >= 0 && i <= 2 );
		m_nAO_MODE = i;
#ifdef _DEBUG
		m_bAO_MODE = true;
#endif
	}
	void SetAO_MODE( bool i )
	{
		m_nAO_MODE = i ? 1 : 0;
#ifdef _DEBUG
		m_bAO_MODE = true;
#endif
	}
public:
	post_ambientocclusion_ps30_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bAO_MODE = false;
#endif // _DEBUG
		m_nAO_MODE = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bAO_MODE;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nAO_MODE ) + 0;
	}
};
#define shaderDynamicTest_post_ambientocclusion_ps30 psh_forgot_to_set_dynamic_AO_MODE + 0
