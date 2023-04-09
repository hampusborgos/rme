//////////////////////////////////////////////////////////////////////
// This file is part of Canary Map Editor
//////////////////////////////////////////////////////////////////////
// Canary Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Canary Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_WALL_BRUSH_H
#define RME_WALL_BRUSH_H

#include "brush.h"

//=============================================================================
// Wallbrush, for drawing walls

class WallBrush : public TerrainBrush
{
public:
	static void init();

	WallBrush();
	virtual ~WallBrush();

	bool isWall() const { return true; }
	WallBrush* asWall() { return static_cast<WallBrush*>(this); }

	virtual bool load(pugi::xml_node node, wxArrayString& warnings);

	virtual bool canDraw(BaseMap* map, const Position& position) const {return true;}

	// Draw to the target tile
	// Note that this actually only puts the first WALL_NORMAL item on the tile.
	// It's up to the doWalls function to change it to the correct alignment
	virtual void draw(BaseMap* map, Tile* tile, void* parameter);
	virtual void undraw(BaseMap* map, Tile* tile);
	// Creates walls on the target tile (does not depend on brush in any way)
	static void doWalls(BaseMap* map, Tile* tile);

	// If the specified wall item is part of this wall
	bool hasWall(Item* item);
	::DoorType getDoorTypeFromID(uint16_t id);

	virtual bool canSmear() const { return false; }
	virtual bool canDrag() const { return true; }

protected:
	struct WallType {
		int chance;
		uint16_t id;
	};
	struct WallNode {
		WallNode() : total_chance(0) {}
		int total_chance;
		std::vector<WallType> items;
	};
	struct DoorType {
		::DoorType type;
		uint16_t id;
	};
	WallNode wall_items[17];
	std::vector<DoorType> door_items[17];

	WallBrush* redirect_to;

	friend class DoorBrush;

public:
	static uint32_t full_border_types[16];
	static uint32_t half_border_types[16];
};

//=============================================================================
// Wall decoration brush, for drawing decoration on walls

class WallDecorationBrush : public WallBrush
{
public:
	WallDecorationBrush();
	virtual ~WallDecorationBrush();

	bool isWallDecoration() const { return true; }
	WallDecorationBrush* asWallDecoration() { return static_cast<WallDecorationBrush*>(this); }

	// We use the exact same loading algorithm as normal walls

	virtual void draw(BaseMap* map, Tile* tile, void* parameter);
};

#endif
