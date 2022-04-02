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

#ifndef ENDGAME_H
#define ENDGAME_H

#include "common.h"

/**
 * @brief Set the effect sprite of the ending.
 *
 * @param wSpriteNum the number of the sprite.
 * @return VOID
 */
VOID PAL_EndingSetEffectSprite(WORD wSpriteNum);

/**
 * @brief Draw an FBP picture to the screen.
 *
 * @param wChunkNum number of chunk in fbp.mkf file.
 * @param wFade fading speed of showing the picture.
 * @return VOID
 */
VOID PAL_ShowFBP(WORD wChunkNum, WORD wFade);

/**
 * @brief Scroll up an FBP picture to the screen.
 *
 * @param wChunkNum number of chunk in fbp.mkf file.
 * @param wScrollSpeed scrolling speed of showing the picture.
 * @param fScrollDown TRUE if scroll down, FALSE if scroll up.
 * @return VOID
 */
VOID PAL_ScrollFBP(WORD wChunkNum, WORD wScrollSpeed, BOOL fScrollDown);

/**
 * @brief Show the ending animation.
 *
 * @return VOID
 */
VOID PAL_EndingAnimation(VOID);

/**
 * @brief Show the ending screen for Win95 version.
 *
 * @return VOID
 */
VOID PAL_EndingScreen(VOID);

#endif
