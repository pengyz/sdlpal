#pragma once
#include "common.h"
#include "palcommon.h"

namespace engine {

extern int gpHighlightWidth;
extern int gpHighlightPaletteIndex;

INT RLEBlitToSurfaceWithShadow(LPCBITMAPRLE lpBitmapRLE, SDL_Surface* lpDstSurface, PAL_POS pos, BOOL bShadow, bool clean);

}