#ifndef RME_WALL_BRUSH_H
#define RME_WALL_BRUSH_H

#include "brush.h"

//=============================================================================
// Wallbrush, for drawing walls

class WallBrush : public TerrainBrush {
public:
	static void init();

	WallBrush();
	virtual ~WallBrush();

	virtual bool load(xmlNodePtr node, wxArrayString& warnings);
	
	virtual bool canDraw(BaseMap* map, Position pos) const {return true;};

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
	
	virtual bool canSmear() const {return false;}
	virtual bool canDrag() const {return true;}
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

class WallDecorationBrush : public WallBrush {
public:
	WallDecorationBrush();
	virtual ~WallDecorationBrush();
	
	// We use the exact same loading algorithm as normal walls

	virtual void draw(BaseMap* map, Tile* tile, void* parameter);
};

#endif
