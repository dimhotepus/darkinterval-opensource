#include "shaderlib/cshader.h"
class flesh_ps30_Static_Index
{
private:
	int m_nALPHABLEND;
#ifdef _DEBUG
	bool m_bALPHABLEND;
#endif
public:
	void SetALPHABLEND( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nALPHABLEND = i;
#ifdef _DEBUG
		m_bALPHABLEND = true;
#endif
	}
	void SetALPHABLEND( bool i )
	{
		m_nALPHABLEND = i ? 1 : 0;
#ifdef _DEBUG
		m_bALPHABLEND = true;
#endif
	}
private:
	int m_nTRANSMAT;
#ifdef _DEBUG
	bool m_bTRANSMAT;
#endif
public:
	void SetTRANSMAT( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nTRANSMAT = i;
#ifdef _DEBUG
		m_bTRANSMAT = true;
#endif
	}
	void SetTRANSMAT( bool i )
	{
		m_nTRANSMAT = i ? 1 : 0;
#ifdef _DEBUG
		m_bTRANSMAT = true;
#endif
	}
private:
	int m_nFRESNEL_WARP;
#ifdef _DEBUG
	bool m_bFRESNEL_WARP;
#endif
public:
	void SetFRESNEL_WARP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFRESNEL_WARP = i;
#ifdef _DEBUG
		m_bFRESNEL_WARP = true;
#endif
	}
	void SetFRESNEL_WARP( bool i )
	{
		m_nFRESNEL_WARP = i ? 1 : 0;
#ifdef _DEBUG
		m_bFRESNEL_WARP = true;
#endif
	}
private:
	int m_nEFFECTS;
#ifdef _DEBUG
	bool m_bEFFECTS;
#endif
public:
	void SetEFFECTS( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nEFFECTS = i;
#ifdef _DEBUG
		m_bEFFECTS = true;
#endif
	}
	void SetEFFECTS( bool i )
	{
		m_nEFFECTS = i ? 1 : 0;
#ifdef _DEBUG
		m_bEFFECTS = true;
#endif
	}
private:
	int m_nTINTING;
#ifdef _DEBUG
	bool m_bTINTING;
#endif
public:
	void SetTINTING( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nTINTING = i;
#ifdef _DEBUG
		m_bTINTING = true;
#endif
	}
	void SetTINTING( bool i )
	{
		m_nTINTING = i ? 1 : 0;
#ifdef _DEBUG
		m_bTINTING = true;
#endif
	}
private:
	int m_nIRIDESCENCE;
#ifdef _DEBUG
	bool m_bIRIDESCENCE;
#endif
public:
	void SetIRIDESCENCE( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nIRIDESCENCE = i;
#ifdef _DEBUG
		m_bIRIDESCENCE = true;
#endif
	}
	void SetIRIDESCENCE( bool i )
	{
		m_nIRIDESCENCE = i ? 1 : 0;
#ifdef _DEBUG
		m_bIRIDESCENCE = true;
#endif
	}
private:
	int m_nBACK_SCATTER;
#ifdef _DEBUG
	bool m_bBACK_SCATTER;
#endif
public:
	void SetBACK_SCATTER( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nBACK_SCATTER = i;
#ifdef _DEBUG
		m_bBACK_SCATTER = true;
#endif
	}
	void SetBACK_SCATTER( bool i )
	{
		m_nBACK_SCATTER = i ? 1 : 0;
#ifdef _DEBUG
		m_bBACK_SCATTER = true;
#endif
	}
private:
	int m_nFORWARD_SCATTER;
#ifdef _DEBUG
	bool m_bFORWARD_SCATTER;
#endif
public:
	void SetFORWARD_SCATTER( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nFORWARD_SCATTER = i;
#ifdef _DEBUG
		m_bFORWARD_SCATTER = true;
#endif
	}
	void SetFORWARD_SCATTER( bool i )
	{
		m_nFORWARD_SCATTER = i ? 1 : 0;
#ifdef _DEBUG
		m_bFORWARD_SCATTER = true;
#endif
	}
private:
	int m_nINTERIOR_LAYER;
#ifdef _DEBUG
	bool m_bINTERIOR_LAYER;
#endif
public:
	void SetINTERIOR_LAYER( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nINTERIOR_LAYER = i;
#ifdef _DEBUG
		m_bINTERIOR_LAYER = true;
#endif
	}
	void SetINTERIOR_LAYER( bool i )
	{
		m_nINTERIOR_LAYER = i ? 1 : 0;
#ifdef _DEBUG
		m_bINTERIOR_LAYER = true;
#endif
	}
private:
	int m_nOPACITY_TEXTURE;
#ifdef _DEBUG
	bool m_bOPACITY_TEXTURE;
#endif
public:
	void SetOPACITY_TEXTURE( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nOPACITY_TEXTURE = i;
#ifdef _DEBUG
		m_bOPACITY_TEXTURE = true;
#endif
	}
	void SetOPACITY_TEXTURE( bool i )
	{
		m_nOPACITY_TEXTURE = i ? 1 : 0;
#ifdef _DEBUG
		m_bOPACITY_TEXTURE = true;
#endif
	}
private:
	int m_nDETAIL;
#ifdef _DEBUG
	bool m_bDETAIL;
#endif
public:
	void SetDETAIL( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDETAIL = i;
#ifdef _DEBUG
		m_bDETAIL = true;
#endif
	}
	void SetDETAIL( bool i )
	{
		m_nDETAIL = i ? 1 : 0;
#ifdef _DEBUG
		m_bDETAIL = true;
#endif
	}
private:
	int m_nNORMAL2SOFT;
#ifdef _DEBUG
	bool m_bNORMAL2SOFT;
#endif
public:
	void SetNORMAL2SOFT( int i )
	{
		Assert( i >= 0 && i <= 0 );
		m_nNORMAL2SOFT = i;
#ifdef _DEBUG
		m_bNORMAL2SOFT = true;
#endif
	}
	void SetNORMAL2SOFT( bool i )
	{
		m_nNORMAL2SOFT = i ? 1 : 0;
#ifdef _DEBUG
		m_bNORMAL2SOFT = true;
#endif
	}
public:
	flesh_ps30_Static_Index( )
	{
#ifdef _DEBUG
		m_bALPHABLEND = false;
#endif // _DEBUG
		m_nALPHABLEND = 0;
#ifdef _DEBUG
		m_bTRANSMAT = false;
#endif // _DEBUG
		m_nTRANSMAT = 0;
#ifdef _DEBUG
		m_bFRESNEL_WARP = false;
#endif // _DEBUG
		m_nFRESNEL_WARP = 0;
#ifdef _DEBUG
		m_bEFFECTS = false;
#endif // _DEBUG
		m_nEFFECTS = 0;
#ifdef _DEBUG
		m_bTINTING = false;
#endif // _DEBUG
		m_nTINTING = 0;
#ifdef _DEBUG
		m_bIRIDESCENCE = false;
#endif // _DEBUG
		m_nIRIDESCENCE = 0;
#ifdef _DEBUG
		m_bBACK_SCATTER = false;
#endif // _DEBUG
		m_nBACK_SCATTER = 0;
#ifdef _DEBUG
		m_bFORWARD_SCATTER = false;
#endif // _DEBUG
		m_nFORWARD_SCATTER = 0;
#ifdef _DEBUG
		m_bINTERIOR_LAYER = false;
#endif // _DEBUG
		m_nINTERIOR_LAYER = 0;
#ifdef _DEBUG
		m_bOPACITY_TEXTURE = false;
#endif // _DEBUG
		m_nOPACITY_TEXTURE = 0;
#ifdef _DEBUG
		m_bDETAIL = false;
#endif // _DEBUG
		m_nDETAIL = 0;
#ifdef _DEBUG
		m_bNORMAL2SOFT = false;
#endif // _DEBUG
		m_nNORMAL2SOFT = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllStaticVarsDefined = m_bALPHABLEND && m_bTRANSMAT && m_bFRESNEL_WARP && m_bEFFECTS && m_bTINTING && m_bIRIDESCENCE && m_bBACK_SCATTER && m_bFORWARD_SCATTER && m_bINTERIOR_LAYER && m_bOPACITY_TEXTURE && m_bDETAIL && m_bNORMAL2SOFT;
		Assert( bAllStaticVarsDefined );
#endif // _DEBUG
		return ( 20 * m_nALPHABLEND ) + ( 40 * m_nTRANSMAT ) + ( 80 * m_nFRESNEL_WARP ) + ( 160 * m_nEFFECTS ) + ( 320 * m_nTINTING ) + ( 640 * m_nIRIDESCENCE ) + ( 640 * m_nBACK_SCATTER ) + ( 1280 * m_nFORWARD_SCATTER ) + ( 2560 * m_nINTERIOR_LAYER ) + ( 2560 * m_nOPACITY_TEXTURE ) + ( 5120 * m_nDETAIL ) + ( 10240 * m_nNORMAL2SOFT ) + 0;
	}
};
#define shaderStaticTest_flesh_ps30 psh_forgot_to_set_static_ALPHABLEND + psh_forgot_to_set_static_TRANSMAT + psh_forgot_to_set_static_FRESNEL_WARP + psh_forgot_to_set_static_EFFECTS + psh_forgot_to_set_static_TINTING + psh_forgot_to_set_static_IRIDESCENCE + psh_forgot_to_set_static_BACK_SCATTER + psh_forgot_to_set_static_FORWARD_SCATTER + psh_forgot_to_set_static_INTERIOR_LAYER + psh_forgot_to_set_static_OPACITY_TEXTURE + psh_forgot_to_set_static_DETAIL + psh_forgot_to_set_static_NORMAL2SOFT + 0
class flesh_ps30_Dynamic_Index
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
public:
	flesh_ps30_Dynamic_Index()
	{
#ifdef _DEBUG
		m_bPIXELFOGTYPE = false;
#endif // _DEBUG
		m_nPIXELFOGTYPE = 0;
#ifdef _DEBUG
		m_bNUM_LIGHTS = false;
#endif // _DEBUG
		m_nNUM_LIGHTS = 0;
#ifdef _DEBUG
		m_bFLASHLIGHT = false;
#endif // _DEBUG
		m_nFLASHLIGHT = 0;
	}
	int GetIndex()
	{
		// Asserts to make sure that we aren't using any skipped combinations.
		// Asserts to make sure that we are setting all of the combination vars.
#ifdef _DEBUG
		bool bAllDynamicVarsDefined = m_bPIXELFOGTYPE && m_bNUM_LIGHTS && m_bFLASHLIGHT;
		Assert( bAllDynamicVarsDefined );
#endif // _DEBUG
		return ( 1 * m_nPIXELFOGTYPE ) + ( 2 * m_nNUM_LIGHTS ) + ( 10 * m_nFLASHLIGHT ) + 0;
	}
};
#define shaderDynamicTest_flesh_ps30 psh_forgot_to_set_dynamic_PIXELFOGTYPE + psh_forgot_to_set_dynamic_NUM_LIGHTS + psh_forgot_to_set_dynamic_FLASHLIGHT + 0
