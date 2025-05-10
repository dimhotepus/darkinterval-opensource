#include "cbase.h"
#include "materialsystem/imesh.h"
#include "clienteffectprecachesystem.h"
#include "disp_common.h"
#include "disp_powerinfo.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_EnvDisplacement : public C_BaseEntity
{
public:
	DECLARE_CLASS(C_EnvDisplacement, C_BaseEntity);
	DECLARE_CLIENTCLASS();

	C_EnvDisplacement();

public:
	void	OnDataChanged(DataUpdateType_t updateType);
	bool	ShouldDraw();
	void	ClientThink(void);
	virtual int DrawModel(int flags);

	int		m_targetdisp_index_int;
protected:
	IMaterial* m_pSheetMat;
}; 

CLIENTEFFECT_REGISTER_BEGIN(PrecacheEnvDisp)
CLIENTEFFECT_MATERIAL("brick/brickfloor001a")
CLIENTEFFECT_REGISTER_END()

C_EnvDisplacement::C_EnvDisplacement(void)
{
}

void C_EnvDisplacement::OnDataChanged(DataUpdateType_t updateType)
{
	if (updateType == DATA_UPDATE_CREATED)
	{
		if (m_pSheetMat == NULL)
		{
			m_pSheetMat = materials->FindMaterial("brick/brickfloor001a", 0);

			if (m_pSheetMat == NULL)
			{
				Warning("Could not find material for env_displacement!\n");
				return;
			}
		}
		SetNextClientThink(CLIENT_THINK_ALWAYS);
	}
}

bool C_EnvDisplacement::ShouldDraw()
{
	return true;
}

void C_EnvDisplacement::ClientThink(void)
{
	// If no target disp, stop
	if (m_targetdisp_index_int <= 0)
		return;

	/*
	CMatRenderContextPtr pRenderContext(materials);
	CMeshBuilder meshBuilder;
	IMesh *pMesh = pRenderContext->GetDynamicMesh(true, NULL, NULL, materials->FindMaterial("shadertest/wireframevertexcolor", NULL));
	meshBuilder.Begin(pMesh, MATERIAL_QUADS, 1);
	meshBuilder.DrawQuad(pMesh, mins.Base(), maxs.Base(), (mins + maxs).Base(), (maxs - mins).Base(), color, true);
	meshBuilder.End();
	*/
	SetNextClientThink(CLIENT_THINK_ALWAYS);
}

int C_EnvDisplacement::DrawModel(int flags)
{
	Vector mins, maxs;
	mins = -Vector(128, 128, 128);
	maxs = Vector(128, 128, 128);
	mins.z += 32;
	unsigned char color[] = { 100, 255, 100 };

	CMatRenderContextPtr pRenderContext(materials);
	IMesh* pMesh = pRenderContext->GetDynamicMesh(false, NULL, NULL, m_pSheetMat);

	CMeshBuilder meshBuilder;
	meshBuilder.Begin(pMesh, MATERIAL_TRIANGLES, 2);

	meshBuilder.Position3f(-128, -128, 32);
	meshBuilder.TexCoord2f(0, 0, 0);
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f(-128, 128, 32);
	meshBuilder.TexCoord2f(0, 1, 0);
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f(128, 128, 32);
	meshBuilder.TexCoord2f(0, 1, 1);
	meshBuilder.AdvanceVertex();

	meshBuilder.Position3f(128, -128, 32);
	meshBuilder.TexCoord2f(0, 0, 1);
	meshBuilder.AdvanceVertex();

	meshBuilder.End();
	pMesh->Draw();
	
	return BaseClass::DrawModel(flags);
}

IMPLEMENT_CLIENTCLASS_DT(C_EnvDisplacement, DT_EnvDisplacement, CEnvDisplacement)
	RecvPropInt(RECVINFO(m_targetdisp_index_int)),
END_RECV_TABLE()
