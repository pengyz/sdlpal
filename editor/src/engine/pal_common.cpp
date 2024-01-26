#include "pal_common.h"

namespace engine {

int gpHighlightWidth = 0;
int gpHighlightPaletteIndex = -1;

inline BYTE calcShadowColor(BYTE bSourceColor)
{
    return ((bSourceColor & 0xF0) | ((bSourceColor & 0x0F) >> 1));
}

INT RLEBlitToSurfaceWithShadow(LPCBITMAPRLE lpBitmapRLE, SDL_Surface* lpDstSurface, PAL_POS pos, BOOL bShadow, bool clean)
{
    UINT i, j, k, sx;
    INT x, y;
    UINT uiLen = 0;
    UINT uiWidth = 0;
    UINT uiHeight = 0;
    UINT uiSrcX = 0;
    BYTE T;
    INT dx = PAL_X(pos);
    INT dy = PAL_Y(pos);
    LPBYTE p;

    //
    // Check for NULL pointer.
    //
    if (lpBitmapRLE == NULL || lpDstSurface == NULL) {
        return -1;
    }

    //
    // Skip the 0x00000002 in the file header.
    //
    if (lpBitmapRLE[0] == 0x02 && lpBitmapRLE[1] == 0x00 && lpBitmapRLE[2] == 0x00 && lpBitmapRLE[3] == 0x00) {
        lpBitmapRLE += 4;
    }

    //
    // Get the width and height of the bitmap.
    //
    uiWidth = lpBitmapRLE[0] | (lpBitmapRLE[1] << 8);
    uiHeight = lpBitmapRLE[2] | (lpBitmapRLE[3] << 8);

    //
    // Check whether bitmap intersects the surface.
    //
    if (uiWidth + dx <= 0 || dx >= lpDstSurface->w || uiHeight + dy <= 0 || dy >= lpDstSurface->h) {
        goto end;
    }

    //
    // Calculate the total length of the bitmap.
    // The bitmap is 8-bpp, each pixel will use 1 byte.
    //
    uiLen = uiWidth * uiHeight;

    //
    // Start decoding and blitting the bitmap.
    //
    lpBitmapRLE += 4;
    for (i = 0; i < uiLen;) {
        T = *lpBitmapRLE++;
        if ((T & 0x80) && T <= 0x80 + uiWidth) {
            i += T - 0x80;
            uiSrcX += T - 0x80;
            if (uiSrcX >= uiWidth) {
                uiSrcX -= uiWidth;
                dy++;
            }
        } else {
            //
            // Prepare coordinates.
            //
            j = 0;
            sx = uiSrcX;
            x = dx + uiSrcX;
            y = dy;

            //
            // Skip the points which are out of the surface.
            //
            if (y < 0) {
                j += -y * uiWidth;
                y = 0;
            } else if (y >= lpDstSurface->h) {
                goto end; // No more pixels needed, break out
            }

            while (j < T) {
                //
                // Skip the points which are out of the surface.
                //
                if (x < 0) {
                    j += -x;
                    if (j >= T)
                        break;
                    sx += -x;
                    x = 0;
                } else if (x >= lpDstSurface->w) {
                    j += uiWidth - sx;
                    x -= sx;
                    sx = 0;
                    y++;
                    if (y >= lpDstSurface->h) {
                        goto end; // No more pixels needed, break out
                    }
                    continue;
                }

                //
                // Put the pixels in row onto the surface
                //
                k = T - j;
                if (lpDstSurface->w - x < k)
                    k = lpDstSurface->w - x;
                if (uiWidth - sx < k)
                    k = uiWidth - sx;
                sx += k;
                p = ((LPBYTE)lpDstSurface->pixels) + y * lpDstSurface->pitch;
                if (bShadow) {
                    j += k;
                    for (; k != 0; k--) {
                        p[x] = clean ? 0 : calcShadowColor(p[x]);
                        x++;
                    }
                } else {
                    int k_start = k;
                    for (; k != 0; k--) {
                        BYTE b = lpBitmapRLE[j];
                        if (clean) {
                            b = 0;
                        } else {
                            if (gpHighlightWidth && gpHighlightPaletteIndex >= 0 && gpHighlightPaletteIndex < 256) {
                                if (((k - k_start) < gpHighlightWidth) || (k <= gpHighlightWidth)) {
                                    b = gpHighlightPaletteIndex;
                                } else {
                                    b = lpBitmapRLE[j];
                                }
                            }
                        }
                        p[x] = b;
                        j++;
                        x++;
                    }
                }

                if (sx >= uiWidth) {
                    sx -= uiWidth;
                    x -= uiWidth;
                    y++;
                    if (y >= lpDstSurface->h) {
                        goto end; // No more pixels needed, break out
                    }
                }
            }
            lpBitmapRLE += T;
            i += T;
            uiSrcX += T;
            while (uiSrcX >= uiWidth) {
                uiSrcX -= uiWidth;
                dy++;
            }
        }
    }

end:
    //
    // Success
    //
    return 0;
}
}