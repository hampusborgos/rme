//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_BRUSH_ENUMS_H_
#define RME_BRUSH_ENUMS_H_

enum BorderType
{
	// Border types (also used for carpets)
	BORDER_NONE = 0,
	NORTH_HORIZONTAL = 1,
	EAST_HORIZONTAL = 2,
	SOUTH_HORIZONTAL = 3,
	WEST_HORIZONTAL = 4,
	NORTHWEST_CORNER = 5,
	NORTHEAST_CORNER = 6,
	SOUTHWEST_CORNER = 7,
	SOUTHEAST_CORNER = 8,
	NORTHWEST_DIAGONAL = 9,
	NORTHEAST_DIAGONAL = 10,
	SOUTHEAST_DIAGONAL = 11,
	SOUTHWEST_DIAGONAL = 12,
	// Additional carpet type
	CARPET_CENTER = 13,
	// Wall types
	WALL_POLE = 0,
	WALL_SOUTH_END = 1,
	WALL_EAST_END = 2,
	WALL_NORTHWEST_DIAGONAL = 3,
	WALL_WEST_END = 4,
	WALL_NORTHEAST_DIAGONAL = 5,
	WALL_HORIZONTAL = 6,
	WALL_SOUTH_T = 7,
	WALL_NORTH_END = 8,
	WALL_VERTICAL = 9,
	WALL_SOUTHWEST_DIAGONAL = 10,
	WALL_EAST_T = 11,
	WALL_SOUTHEAST_DIAGONAL = 12,
	WALL_WEST_T = 13,
	WALL_NORTH_T = 14,
	WALL_INTERSECTION = 15,
	WALL_UNTOUCHABLE = 16,
	// Table types
	TABLE_NORTH_END = 0,
	TABLE_SOUTH_END = 1,
	TABLE_EAST_END = 2,
	TABLE_WEST_END = 3,
	TABLE_HORIZONTAL = 4,
	TABLE_VERTICAL = 5,
	TABLE_ALONE = 6,
};

enum TileAlignement
{
	TILE_NORTHWEST = 1,
	TILE_NORTH = 2,
	TILE_NORTHEAST = 4,
	TILE_WEST = 8,
	TILE_EAST = 16,
	TILE_SOUTHWEST = 32,
	TILE_SOUTH = 64,
	TILE_SOUTHEAST = 128,
	WALLTILE_NORTH = 1,
	WALLTILE_WEST = 2,
	WALLTILE_EAST = 4,
	WALLTILE_SOUTH = 8,
};

enum DoorType
{
	WALL_UNDEFINED,
	WALL_ARCHWAY,
	WALL_DOOR_NORMAL,
	WALL_DOOR_LOCKED,
	WALL_DOOR_QUEST,
	WALL_DOOR_MAGIC,
	WALL_WINDOW,
	WALL_HATCH_WINDOW,
};

#endif
