//========================================================================//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//
#include "cbase.h"
#include "view_shared.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

class C_FuncWaterAnalog : public C_BaseEntity
{
public:
	DECLARE_CLASS(C_FuncWaterAnalog, C_BaseEntity);
	DECLARE_CLIENTCLASS();

	// C_BaseEntity.
public:
	C_FuncWaterAnalog();
	virtual ~C_FuncWaterAnalog();

	virtual bool	ShouldDraw();

	C_FuncWaterAnalog	*m_pNext;
public:
	bool	m_bReflectEntities;
	bool	m_bDisableDraw;
};

IMPLEMENT_CLIENTCLASS_DT(C_FuncWaterAnalog, DT_FuncWaterAnalog, CFuncMoveLinear)
RecvPropBool(RECVINFO(m_bReflectEntities)),
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Globals
//-----------------------------------------------------------------------------
C_EntityClassList<C_FuncWaterAnalog> g_WaterAnalogList;
template<> C_FuncWaterAnalog *C_EntityClassList<C_FuncWaterAnalog>::m_pClassList = NULL;

C_FuncWaterAnalog* GetWaterAnalogList()
{
	return g_WaterAnalogList.m_pClassList;
}


//-----------------------------------------------------------------------------
// Constructor, destructor
//-----------------------------------------------------------------------------
C_FuncWaterAnalog::C_FuncWaterAnalog()
{
	g_WaterAnalogList.Insert(this);
}

C_FuncWaterAnalog::~C_FuncWaterAnalog()
{
	g_WaterAnalogList.Remove(this);
}


bool C_FuncWaterAnalog::ShouldDraw()
{
	return true;
}


//-----------------------------------------------------------------------------
// Do we have water analog with reflections in view?
//-----------------------------------------------------------------------------
bool IsWaterAnalogInView(const CViewSetup& view, cplane_t &plane)
{
	// Early out if no cameras
	C_FuncWaterAnalog *pWaterAnalog = GetWaterAnalogList();
	if (!pWaterAnalog)
		return false;

	Frustum_t frustum;
	GeneratePerspectiveFrustum(view.origin, view.angles, view.zNear, view.zFar, view.fov, view.m_flAspectRatio, frustum);

	cplane_t localPlane;
	Vector vecOrigin, vecWorld, vecDelta, vecForward;
	AngleVectors(view.angles, &vecForward, NULL, NULL);

	for (; pWaterAnalog != NULL; pWaterAnalog = pWaterAnalog->m_pNext)
	{
		if (pWaterAnalog->IsDormant())
			continue;

		Vector vecMins, vecMaxs;
		//pWaterAnalog->GetRenderBoundsWorldspace(vecMins, vecMaxs);
		//if (R_CullBox(vecMins, vecMaxs, frustum))
		//	continue;

		const model_t *pModel = pWaterAnalog->GetModel();
		const matrix3x4_t& mat = pWaterAnalog->EntityToWorldTransform();

		int nCount = modelinfo->GetBrushModelPlaneCount(pModel);
		for (int i = 0; i < nCount; ++i)
		{
			modelinfo->GetBrushModelPlane(pModel, i, localPlane, &vecOrigin);

			MatrixTransformPlane(mat, localPlane, plane);			// Transform to world space
			VectorTransform(vecOrigin, mat, vecWorld);

			if (view.origin.Dot(plane.normal) <= plane.dist)	// Check for view behind plane
				continue;

			VectorSubtract(vecWorld, view.origin, vecDelta);		// Backface cull
			if (vecDelta.Dot(plane.normal) >= 0)
				continue;

			return true;
		}
	}

	return false;
}



