#ifndef RME_MAP_ALLOCATOR_H
#define RME_MAP_ALLOCATOR_H

#include "tile.h"
#include "map_region.h"

class BaseMap;

class MapAllocator
{

public:
	MapAllocator() {}
	~MapAllocator() {}

	// shorthands for tiles
	Tile* operator()(TileLocation* location) {
		return allocateTile(location);
	}
	// free
	void operator()(Tile* t) {
		freeTile(t);
	}

	//
	Tile* allocateTile(TileLocation* location) {
		return newd Tile(*location);
	}
	void freeTile(Tile* t) {
		delete t;
	}

	//
	Floor* allocateFloor(int x, int y, int z) {
		return newd Floor(x, y, z);
	}
	void freeFloor(Floor* f) {
		delete f;
	}

	//
	QTreeNode* allocateNode(BaseMap& map) {
		return newd QTreeNode(map);
	}
	void freeNode(QTreeNode* qt) {
		delete qt;
	}
};

#endif
