/* -*- mode: c; tab-width: 4; c-basic-offset: 4; c-file-style: "linux" -*- */
//
// Copyright (c) 2009-2011, Wei Mingzhi <whistler_wmz@users.sf.net>.
// Copyright (c) 2011-2022, SDLPAL development team.
// All rights reserved.
//
// This file is part of SDLPAL.
//
// SDLPAL is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef PALETTE_H
#define PALETTE_H

#include "common.h"


/**
 * @brief Get the specified palette in pat.mkf file.
 * 
 * @param iPaletteNum number of the palette.
 * @param fNight whether use the night palette or not.
 * @return SDL_Color* Pointer to the palette. NULL if failed.
 */
SDL_Color* PAL_GetPalette(INT iPaletteNum, BOOL fNight);

/**
 * @brief Set the screen palette to the specified one.
 *
 * @param iPaletteNum number of the palette.
 * @param fNight whether use the night palette or not.
 * @return VOID
 */
VOID PAL_SetPalette(INT iPaletteNum, BOOL fNight);

/**
 * @brief Fadeout screen to black from the specified palette.
 *
 * @param iDelay delay time for each step.
 * @return VOID
 */
VOID PAL_FadeOut(INT iDelay);

/**
 * @brief Fade in the screen to the specified palette.
 *
 * @param iPaletteNum number of the palette.
 * @param fNight whether use the night palette or not.
 * @param iDelay delay time for each step.
 * @return VOID
 */
VOID PAL_FadeIn(INT iPaletteNum, BOOL fNight, INT iDelay);

/**
 * @brief Fade in or fade out the screen. Update the scene during the process.
 *
 * @param iPaletteNum number of the palette.
 * @param fNight whether use the night palette or not.
 * @param iStep positive to fade in, nagative to fade out.
 * @return VOID
 */
VOID PAL_SceneFade(INT iPaletteNum, BOOL fNight, INT iStep);

/**
 * @brief Fade from the current palette to the specified one.
 *
 * @param iPaletteNum number of the palette.
 * @param fNight whether use the night palette or not.
 * @param fUpdateScene TRUE if update the scene in the progress.
 * @return VOID
 */
VOID PAL_PaletteFade(INT iPaletteNum, BOOL fNight, BOOL fUpdateScene);

/**
 * @brief Fade the palette from/to the specified color.
 *
 * @param iDelay the delay time of each step.
 * @param bColor the color to fade from/to.
 * @param fFrom if TRUE then fade from bColor, else fade to bColor.
 * @return VOID
 */
VOID PAL_ColorFade(INT iDelay, BYTE bColor, BOOL fFrom);

/**
 * @brief Fade the whole screen to red color.
 *
 * @return VOID
 */
VOID PAL_FadeToRed(VOID);

#endif
