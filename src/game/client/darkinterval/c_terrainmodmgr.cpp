//========= Copyright © 1996-2001, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================
#include "cbase.h"
#include "terrainmodmgr_shared.h"
#include "terrainmod.h"
#include "bitbuf.h"
//#include "parsemsg.h"
#include "hud_macros.h"
#include "iefx.h"

void __MsgFunc_TerrainMod( /*const char *pszName, int iSize,*/ bf_read &msg  /*void *pbuf*/ )
{
	ITerrainMod *mod;
	CTerrainModParams params;

//	msg.Read
//	BEGIN_READ( pbuf, iSize );

	unsigned char type = msg.ReadByte();

	params.m_vCenter.x = msg.ReadFloat(); //READ_FLOAT();
	params.m_vCenter.y = msg.ReadFloat(); //READ_FLOAT();
	params.m_vCenter.z = msg.ReadFloat(); //READ_FLOAT();

	params.m_flRadius   = RemapVal( msg.ReadShort(), 0, 65535, MIN_TMOD_RADIUS, MAX_TMOD_RADIUS );

	params.m_vecMin.x = msg.ReadFloat(); //READ_FLOAT();
	params.m_vecMin.y = msg.ReadFloat(); //READ_FLOAT();
	params.m_vecMin.z = msg.ReadFloat(); //READ_FLOAT();

	params.m_vecMax.x = msg.ReadFloat(); //READ_FLOAT();
	params.m_vecMax.y = msg.ReadFloat(); //READ_FLOAT();
	params.m_vecMax.z = msg.ReadFloat(); //READ_FLOAT();

	params.m_flStrength = msg.ReadFloat(); //READ_FLOAT();

	params.m_Flags = msg.ReadByte(); //READ_BYTE();

	if( type == TMod_Suck && (params.m_Flags & CTerrainModParams::TMOD_SUCKTONORMAL) )
	{
		params.m_vNormal.x = msg.ReadFloat(); //READ_FLOAT();
		params.m_vNormal.y = msg.ReadFloat(); //READ_FLOAT();
		params.m_vNormal.z = msg.ReadFloat(); //READ_FLOAT();
	}	

	// Apply the decal first because the place where we're applying the decal 
	// may not be there if we blow it out first!
	Vector vPosition(0,0,0);
	QAngle vAngles(0,0,0);
	int iModel = 0;
	C_BaseEntity *ent = cl_entitylist->GetEnt( iModel );
	if( ent && type == TMod_Sphere )
	{
		effects->DecalShoot( 
			effects->Draw_DecalIndexFromName( "decals/tscorch" ), 
			iModel, 
			ent->GetModel(), 
			vPosition, 
			vAngles, 
			params.m_vCenter, 
			NULL, 
			0 );
	}
	
	mod->ApplyMod(params.m_vCenter + Vector(0, 0, params.m_flRadius * 2), params.m_vCenter);
//	engine->ApplyTerrainMod( (TerrainModType)type, params );
}

void TerrainMod_Init()
{
	usermessages->Register( "TerrainMod", -1 );
}

void C_TerrainMod_Init()
{
	HOOK_MESSAGE( TerrainMod );
}




