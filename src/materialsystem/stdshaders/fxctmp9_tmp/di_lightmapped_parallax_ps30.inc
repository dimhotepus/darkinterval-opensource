#include "shaderlib/cshader.h"
class di_lightmapped_parallax_ps30_Static_Index
{
private:
	int m_nBASEMAPALPHAPARALLAXMAP;
#ifdef _DEBUG
	bool m_bBASEMAPALPHAPARALLAXMAP;
#endif
public:
	void SetBASEMAPALPHAPARALLAXMAP( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nBASEMAPALPHAPARALLAXMAP = i;
#ifdef _DEBUG
		m_bBASEMAPALPHAPARALLAXMAP = true;
#endif
	}
	void SetBASEMAPALPHAPARALLAXMAP( bool i )
	{
		m_nBASEMAPALPHAPARALLAXMAP = i ? 1 : 0;
#ifdef _DEBUG
		m_bBASEMAPALPHAPARALLAXMAP = true;
#endif
	}
private:
	int m_nDETAILTEXTURE;
#ifdef _DEBUG
	bool m_bDETAILTEXTURE;
#endif
public:
	void SetDETAILTEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDETAILTEXTURE = i;
#ifdef _DEBUG
		m_bDETAILTEXTURE = true;
#endif
	}
	void SetDETAILTEXTURE( bool i )
	{
		m_nDETAILTEXTURE = i ? 1 : 0;
#ifdef _DEBUG
		m_bDETAILTEXTURE = true;
#endif
	}
private:
	int m_nSELFILLUM;
#ifdef _DEBUG
	bool m_bSELFILLUM;
#endif
public:
	void SetSELFILLUM( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nSELFILLUM = i;
#ifdef _DEBUG
		m_bSELFILLUM = true;
#endif
	}
	void SetSELFILLUM( bool i )
	{
		m_nSELFILLUM = i ? 1 : 0;
#ifdef _DEBUG
		m_bSELFILLUM = true;
#endif
	}
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
	int m_nBUMPMAP;
#ifdef _DEBUG
	bool m_bBUMPMAP;
#endif
public:
	void SetBUMPMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBUMPMAP = i;
#ifdef _DEBUG
		m_bBUMPMAP = true;
#endif
	}
	void SetBUMPMAP( bool i )
	{
		m_nBUMPMAP = i ? 1 : 0;
#ifdef _DEBUG
		m_bBUMPMAP = true;
#endif
	}
private:
	int m_nFLASHLIGHT;
#ifdef _DEBUG
	bool m_bFLASHLIGHT;
#endif
public:
	void SetFLASHLIGHT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFLASHLIGHT = i;
#ifdef _DEBUG
		m_bFLASHLIGHT = true;
#endif
	}
	void SetFLASHLIGHT( bool i )
	{
		m_nFLASHLIGHT = i ? 1 : 0;
#ifdef _DEBUG
		m_bFLASHLIGHT = true;
#endif
	}
private:
	int m_nUNLIT;
#ifdef _DEBUG
	bool m_bUNLIT;
#endif
public:
	void SetUNLIT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nUNLIT = i;
#ifdef _DEBUG
		m_bUNLIT = true;
#endif
	}
	void SetUNLIT( bool i )
	{
		m_nUNLIT = i ? 1 : 0;
#ifdef _DEBUG
		m_bUNLIT = true;
#endif
	}
public:
	di_lightmapped_parallax_ps30_Static_Index( )
	{
#ifdef _DEBUG
		m_bBASEMAPALPHAPARALLAXMAP = false;
#endif // _DEBUG
		m_nBASEMAPALPHAPARALLAXMAP = 0;
#ifdef _DEBUG
		m_bDETAILTEXTURE = false;
#endif // _DEBUG
		m_nDETAILTEXTURE = 0;
#ifdef _DEBUG
		m_bSELFILLUM = false;
#endif // _DEBUG
		m_nSELFILLUM = 0;
#ifdef _DEBUG
		m_bCUBEMAP = false;
#endif // _DEBUG
		m_nCUBEMAP = 0;
#ifdef _DEBUG
		m_bBUMPMAP = false;
#endif // _DEBUG
		m_nBUMPMAP = 0;
#ifdef _DEBUG
		m_bFLASHLIGHT = false;
#endif // _DEBUG
		m_nFLASHLIGHT = 0;
#ifdef _DEBUG
		m_bUNLIT = false;
#endif // _DEBUG
		m_nUNLIT = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bBASEMAPALPHAPARALLAXMAP && m_bDETAILTEXTURE && m_bSELFILLUM && m_bCUBEMAP && m_bBUMPMAP && m_bFLASHLIGHT && m_bUNLIT;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 8 * m_nBASEMAPALPHAPARALLAXMAP ) + ( 8 * m_nDETAILTEXTURE ) + ( 16 * m_nSELFILLUM ) + ( 32 * m_nCUBEMAP ) + ( 64 * m_nBUMPMAP ) + ( 128 * m_nFLASHLIGHT ) + ( 256 * m_nUNLIT ) + 0;
	}
};
#define shaderStaticTest_di_lightmapped_parallax_ps30 psh_forgot_to_set_static_BASEMAPALPHAPARALLAXMAP + psh_forgot_to_set_static_DETAILTEXTURE + psh_forgot_to_set_static_SELFILLUM + psh_forgot_to_set_static_CUBEMAP + psh_forgot_to_set_static_BUMPMAP + psh_forgot_to_set_static_FLASHLIGHT + psh_forgot_to_set_static_UNLIT + 0
class di_lightmapped_parallax_ps30_Dynamic_Index
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
	int m_nPARALLAXDEBUG;
#ifdef _DEBUG
	bool m_bPARALLAXDEBUG;
#endif
public:
	void SetPARALLAXDEBUG( int i )
	{
		Assert( i >= 0 && i <= 3 );
		m_nPARALLAXDEBUG = i;
#ifdef _DEBUG
		m_bPARALLAXDEBUG = true;
#endif
	}
	void SetPARALLAXDEBUG( bool i )
	{
		m_nPARALLAXDEBUG = i ? 1 : 0;
#ifdef _DEBUG
		m_bPARALLAXDEBUG = true;
#endif
	}
public:
	di_lightmapped_parallax_ps30_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bPIXELFOGTYPE = false;
#endif // _DEBUG
		m_nPIXELFOGTYPE = 0;
#ifdef _DEBUG
		m_bPARALLAXDEBUG = false;
#endif // _DEBUG
		m_nPARALLAXDEBUG = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bPIXELFOGTYPE && m_bPARALLAXDEBUG;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nPIXELFOGTYPE ) + ( 2 * m_nPARALLAXDEBUG ) + 0;
	}
};
#define shaderDynamicTest_di_lightmapped_parallax_ps30 psh_forgot_to_set_dynamic_PIXELFOGTYPE + psh_forgot_to_set_dynamic_PARALLAXDEBUG + 0
