#ifndef C_SHAREDENTS_ENCOMPASS_H
#define C_SHAREDENTS_ENCOMPASS_H
#ifdef _WIN32
#pragma once
#endif

#include "mathlib/mathlib.h"
#include "mathlib/Vector.h"
#include "UtlVector.h"
#include "SheetSimulator.h"

//-----------------------------------------------------------------------------
// Purpose: Energy Wave Effect
//-----------------------------------------------------------------------------
class C_TestSheetEffect : public CSheetSimulator
{
public:
	C_TestSheetEffect(TraceLineFunc_t traceline, TraceHullFunc_t traceHull);

	void	Precache();
	void	Spawn();

	// Computes the opacity....
	float	ComputeOpacity(const Vector& pt, const Vector& center) const;

//	void	DetectCollisions();

private:
	C_TestSheetEffect(const C_TestSheetEffect &); // not defined, not accessible
												  // Simulation set up
	void ComputeRestPositions();
	void MakeSpring(int p1, int p2);
	void ComputeSprings();

	float m_RestDistance;
	float m_SheetTheta;
	float m_SheetPhi;
};

#if 1
class C_TestSheetEntity : public C_BaseAnimating
{
	DECLARE_CLASS(C_TestSheetEntity, C_BaseAnimating);
	DECLARE_CLIENTCLASS();
public:
	C_TestSheetEntity();
	~C_TestSheetEntity();

	virtual RenderGroup_t GetRenderGroup(void)
	{
		// We want to draw translucent bits as well as our main model
		return RENDER_GROUP_TRANSLUCENT_ENTITY;
	}

	virtual void	OnDataChanged(DataUpdateType_t updateType);
	virtual int		DrawModel(int flags);
	void PostDataUpdate(DataUpdateType_t updateType);
	void ComputePoint(float s, float t, Vector& pt, Vector& normal, float& opacity);
	void DrawWireframeModel();

	C_TestSheetEffect m_SheetEffect;

protected:
	virtual bool	InitMaterials(void);
	IMaterial* m_pWireframe;
	IMaterial* m_pSheetMat;

private:
	C_TestSheetEntity(const C_TestSheetEntity &); // not defined, not accessible

	void ComputeTestSheetPoints(Vector* pt, Vector* normal, float* opacity);
	void DrawTestSheetPoints(Vector* pt, Vector* normal, float* opacity);

};
#endif
#endif // C_SHAREDENTS_ENCOMPASS_H
