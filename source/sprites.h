//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/sprites.h $
// $Id: sprites.h 298 2010-02-23 17:09:13Z admin $

#ifndef RME_SPRITES_H_
#define RME_SPRITES_H_

enum {
	SPRITE_FLAG_GREEN = 2020,
	SPRITE_FLAG_GREY = 2019,
	SPRITE_FLAME_BLUE = 1397,
	SPRITE_SPAWN = 1507,
};

enum {
	EDITOR_SPRITE_SELECTION_MARKER = -1000,
	EDITOR_SPRITE_BRUSH_CD_1x1,
	EDITOR_SPRITE_BRUSH_CD_3x3,
	EDITOR_SPRITE_BRUSH_CD_5x5,
	EDITOR_SPRITE_BRUSH_CD_7x7,
	EDITOR_SPRITE_BRUSH_CD_9x9,
	EDITOR_SPRITE_BRUSH_CD_15x15,
	EDITOR_SPRITE_BRUSH_CD_19x19,
	EDITOR_SPRITE_BRUSH_SD_1x1,
	EDITOR_SPRITE_BRUSH_SD_3x3,
	EDITOR_SPRITE_BRUSH_SD_5x5,
	EDITOR_SPRITE_BRUSH_SD_7x7,
	EDITOR_SPRITE_BRUSH_SD_9x9,
	EDITOR_SPRITE_BRUSH_SD_15x15,
	EDITOR_SPRITE_BRUSH_SD_19x19,
	EDITOR_SPRITE_OPTIONAL_BORDER_TOOL,
	EDITOR_SPRITE_ERASER,
	EDITOR_SPRITE_PZ_TOOL,
	EDITOR_SPRITE_PVPZ_TOOL,
	EDITOR_SPRITE_NOLOG_TOOL,
	EDITOR_SPRITE_NOPVP_TOOL,
	EDITOR_SPRITE_DOOR_NORMAL,
	EDITOR_SPRITE_DOOR_LOCKED,
	EDITOR_SPRITE_DOOR_MAGIC,
	EDITOR_SPRITE_DOOR_QUEST,
	EDITOR_SPRITE_WINDOW_NORMAL,
	EDITOR_SPRITE_WINDOW_HATCH,
	EDITOR_SPRITE_SELECTION_GEM,
	EDITOR_SPRITE_DRAWING_GEM,
	EDITOR_SPRITE_LAST
};

static const char* selection_marker_xpm16x16[] = {
	/* columns rows colors chars-per-pixel */
	"16 16 2 1",
	"  c None",
	". c #000080",
	/* pixels */
	" . . . . . . . .",
	". . . . . . . . ",
	" . . . . . . . .",
	". . . . . . . . ",
	" . . . . . . . .",
	". . . . . . . . ",
	" . . . . . . . .",
	". . . . . . . . ",
	" . . . . . . . .",
	". . . . . . . . ",
	" . . . . . . . .",
	". . . . . . . . ",
	" . . . . . . . .",
	". . . . . . . . ",
	" . . . . . . . .",
	". . . . . . . . "
};

static const char* selection_marker_xpm32x32[] = {
	/* columns rows colors chars-per-pixel */
	"32 32 2 1",
	"  c None",
	". c #000080",
	/* pixels */
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . ",
	" . . . . . . . . . . . . . . . .",
	". . . . . . . . . . . . . . . . "
};

#endif
