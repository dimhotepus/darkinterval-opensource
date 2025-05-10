//========================================================================//
//
// Purpose:
//
//=============================================================================

#ifndef VIEWPOSTPROCESS_H
#define VIEWPOSTPROCESS_H

#if defined( _WIN32 )
#pragma once
#endif
#ifdef DARKINTERVAL // depth of field
enum DOFType {
	DOF_DEFAULT,
	DOF_PAUSED_SCREEN,
	DOF_UNDERWATER,
	DOF_EXTRA
};
#endif
void DoEnginePostProcessing( int x, int y, int w, int h, bool bFlashlightIsOn, bool bPostVGui = false );
void DoImageSpaceMotionBlur(const CViewSetup &view, int x, int y, int w, int h);
void DumpTGAofRenderTarget( const int width, const int height, const char *pFilename );
#ifdef DARKINTERVAL // depth of field and numerous other screenspace post process effects
bool IsDepthOfFieldEnabled();
bool DOFType();
void DoDepthOfField(const CViewSetup &view, int getDOFType);
bool IsChromAbEnabled();
void DoChromaticAberration(const CViewSetup &view);
bool IsCubicDistEnabled();
void DoCubicDistortion(const CViewSetup &view);
bool IsInfluenceEnabled();
void DoInfluence(const CViewSetup &view);
bool IsVignetteEnabled();
void DoVignette(const CViewSetup &view);
bool IsScreenFilterEnabled();
void DoScreenFilter(const CViewSetup &view);
bool IsObjectMotionBlurEnabled();
void DoObjectMotionBlur(const CViewSetup &view); // object motion blur ported from 2015
void DoDepthOfField(const CViewSetup &view, int getDOFType);
#endif

#endif // VIEWPOSTPROCESS_H
