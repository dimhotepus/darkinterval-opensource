//========================================================================//
//
// Purpose: 
//
// $NoKeywords: $
//===========================================================================//

#ifndef C_FUNC_WATER_ANALOG
#define C_FUNC_WATER_ANALOG

#ifdef _WIN32
#pragma once
#endif

struct cplane_t;
class CViewSetup;

//-----------------------------------------------------------------------------
// Do we have func_water_analog with reflections in view? If so, what's the reflection plane?
//-----------------------------------------------------------------------------
bool IsWaterAnalogInView(const CViewSetup& view, cplane_t &plane);

#endif // C_FUNC_WATER_ANALOG


