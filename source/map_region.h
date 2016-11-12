//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/basemap.h $
// $Id: basemap.h 278 2010-02-14 22:38:30Z admin $

#ifndef RME_MAP_REGION_H
#define RME_MAP_REGION_H

#include "position.h"

class Tile;
class Floor;
class BaseMap;

class TileLocation
{
	TileLocation();
public:
	~TileLocation();

	TileLocation(const TileLocation&) = delete;
	TileLocation& operator=(const TileLocation&) = delete;

protected:
	Tile* tile;
	Position position;
	size_t spawn_count;
	size_t waypoint_count;
	HouseExitList* house_exits; // Any house exits pointing here

public:

	// Access tile
	// Can't set directly since that does not update tile count
	Tile* get() {return tile;}
	const Tile* get() const {return tile;}

	int size() const;
	bool empty() const;

	Position getPosition() const {return position;}
	int getX() const {return position.x;}
	int getY() const {return position.y;}
	int getZ() const {return position.z;}

	size_t getSpawnCount() const {return spawn_count;}
	void increaseSpawnCount() {spawn_count++;}
	void decreaseSpawnCount() {spawn_count--;}
	size_t getWaypointCount() const {return waypoint_count;}
	void increaseWaypointCount() {waypoint_count++;}
	void decreaseWaypointCount() {waypoint_count--;}
	HouseExitList* createHouseExits() {if(house_exits) return house_exits; return house_exits = newd HouseExitList;}
	HouseExitList* getHouseExits() {return house_exits;}

	friend class Floor;
	friend class QTreeNode;
};

class Floor {
public:
	Floor(int x, int y, int z);
	TileLocation locs[MAP_LAYERS];
};

// This is not a QuadTree, but a HexTree (16 child nodes to every node), so the name is abit misleading
class QTreeNode
{
public:
	QTreeNode(BaseMap& map);
	virtual ~QTreeNode();

	QTreeNode(const QTreeNode&) = delete;
	QTreeNode& operator=(const QTreeNode&) = delete;

	QTreeNode* getLeaf(int x, int y); // Might return nullptr
	QTreeNode* getLeafForce(int x, int y); // Will never return nullptr, it will create the node if it's not there

	// Coordinates are NOT relative
	TileLocation* createTile(int x, int y, int z);
	TileLocation* getTile(int x, int y, int z);
	Tile* setTile(int x, int y, int z, Tile* tile);
	void clearTile(int x, int y, int z);

	Floor* createFloor(int x, int y, int z);
	Floor* getFloor(uint32_t z) {
		ASSERT(isLeaf);
		return array[z];
	}
	Floor** getFloors() {
		return array;
	}

	void setVisible(bool overground, bool underground);
	void setVisible(uint32_t client, bool underground, bool value);
	bool isVisible(uint32_t client, bool underground);
	void clearVisible(uint32_t client);

	void setRequested(bool underground, bool r);
	bool isVisible(bool underground);
	bool isRequested(bool underground);
protected:
	BaseMap& map;
	uint32_t visible;

	bool isLeaf;
	union {
		QTreeNode* child[MAP_LAYERS];
		Floor* array[MAP_LAYERS];
#if 16 != MAP_LAYERS
#    error "You need to rewrite the QuadTree in order to handle more or less than 16 floors"
#endif
	};

	friend class BaseMap;
	friend class MapIterator;
};

#endif
