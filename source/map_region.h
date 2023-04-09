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
	size_t spawn_monster_count;
	size_t spawn_npc_count;
	size_t waypoint_count;
	HouseExitList* house_exits; // Any house exits pointing here

public:

	// Access tile
	// Can't set directly since that does not update tile count
	Tile* get() {return tile;}
	const Tile* get() const {return tile;}

	int size() const;
	bool empty() const;

	Position getPosition() const { return position; }

	int getX() const {return position.x;}
	int getY() const {return position.y;}
	int getZ() const {return position.z;}

	size_t getSpawnMonsterCount() const {return spawn_monster_count;}
	void increaseSpawnCount() {spawn_monster_count++;}
	void decreaseSpawnMonsterCount() {spawn_monster_count--;}

	size_t getSpawnNpcCount() const {return spawn_npc_count;}
	void increaseSpawnNpcCount() {spawn_npc_count++;}
	void decreaseSpawnNpcCount() {spawn_npc_count--;}

	size_t getWaypointCount() const {return waypoint_count;}
	void increaseWaypointCount() {waypoint_count++;}
	void decreaseWaypointCount() {waypoint_count--;}
	HouseExitList* createHouseExits() {if(house_exits) return house_exits; return house_exits = newd HouseExitList;}
	HouseExitList* getHouseExits() {return house_exits;}

	friend class Floor;
	friend class QTreeNode;
	friend class Waypoints;
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
