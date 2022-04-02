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

#ifndef SCRIPT_H
#define SCRIPT_H

#include "common.h"

#define PAL_ITEM_DESC_BOTTOM (1 << 15)

/**
 * @brief Runs a trigger script.
 * 
 * @param wScriptEntry The script entry to execute.
 * @param wEventObjectID The event object ID which invoked the script.
 * @return WORD The entry point of the script.
 */
WORD PAL_RunTriggerScript(WORD wScriptEntry, WORD wEventObjectID);

/**
 * @brief Runs the autoscript of the specified event object.
 * 
 * @param wScriptEntry The script entry to execute.
 * @param wEventObjectID The event object ID which invoked the script.
 * @return WORD The address of the next script instruction to execute.
 */
WORD PAL_RunAutoScript(WORD wScriptEntry, WORD wEventObjectID);

extern BOOL g_fScriptSuccess;

#endif
