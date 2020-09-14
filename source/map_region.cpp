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

#include "main.h"

#include "map_region.h"
#include "basemap.h"
#include "position.h"
#include "tile.h"

//**************** Tile Location **********************

TileLocation::TileLocation() :
	tile(nullptr),
	position(0, 0, 0),
	spawn_count(0),
	waypoint_count(0),
	house_exits(nullptr)
{
	////
}

TileLocation::~TileLocation()
{
	delete tile;
	delete house_exits;
}

int TileLocation::size() const
{
	if(tile)
		return tile->size();
	return spawn_count + waypoint_count + (house_exits? 1 : 0);
}

bool TileLocation::empty() const
{
	return size() == 0;
}

//**************** Floor **********************

Floor::Floor(int sx, int sy, int z)
{
	sx = sx & ~3;
	sy = sy & ~3;

	for(int i = 0; i < MAP_LAYERS; ++i) {
		locs[i].position.x = sx + (i >> 2);
		locs[i].position.y = sy + (i & 3);
		locs[i].position.z = z;
	}
}

//**************** QTreeNode **********************

QTreeNode::QTreeNode(BaseMap& map) :
	map(map),
	visible(0),
	isLeaf(false)
{
	// Doesn't matter if we're leaf or node
	for(int i = 0; i < MAP_LAYERS; ++i)
		child[i] = nullptr;
}

QTreeNode::~QTreeNode()
{
	if(isLeaf) {
		for(int i = 0; i < MAP_LAYERS; ++i)
			delete array[i];
	} else {
		for(int i = 0; i < MAP_LAYERS; ++i)
			delete child[i];
	}
}

QTreeNode* QTreeNode::getLeaf(int x, int y)
{
	QTreeNode* node = this;
	uint32_t cx = x, cy = y;
	while(node) {
		if(node->isLeaf) {
			return node;
		} else {
			uint32_t index = ((cx & 0xC000) >> 14) | ((cy & 0xC000) >> 12);
			if(node->child[index]) {
				node = node->child[index];
				cx <<= 2;
				cy <<= 2;
			} else {
				return nullptr;
			}
		}
	}
	return nullptr;
}

QTreeNode* QTreeNode::getLeafForce(int x, int y)
{
	QTreeNode* node = this;
	uint32_t cx = x, cy = y;
	int level = 6;
	while(node) {
		uint32_t index = ((cx & 0xC000) >> 14) | ((cy & 0xC000) >> 12);

		QTreeNode*& qt = node->child[index];
		if(qt) {
			if(qt->isLeaf)
				return qt;

		} else {
			if(level == 0) {
				qt = newd QTreeNode(map);
				qt->isLeaf = true;
				return qt;
			} else {
				qt = newd QTreeNode(map);
			}
		}
		node = node->child[index];
		cx <<= 2;
		cy <<= 2;
		level -= 1;
	}

	return nullptr;
}


Floor* QTreeNode::createFloor(int x, int y, int z)
{
	ASSERT(isLeaf);
	if(!array[z])
		array[z] = newd Floor(x, y, z);
	return array[z];
}

bool QTreeNode::isVisible(bool underground)
{
	return testFlags(visible, underground + 1);
}

bool QTreeNode::isRequested(bool underground)
{
	if(underground) {
		return testFlags(visible, 4);
	} else {
		return testFlags(visible, 8);
	}
}

void QTreeNode::clearVisible(uint32_t u)
{
	if(isLeaf)
		visible &= u;
	else
		for(int i = 0; i < MAP_LAYERS; ++i)
			if(child[i])
				child[i]->clearVisible(u);
}

bool QTreeNode::isVisible(uint32_t client, bool underground)
{
	if(underground) {
		return testFlags(visible >> MAP_LAYERS, static_cast<uint64_t>(1) << client);
	} else {
		return testFlags(visible, static_cast<uint64_t>(1) << client);
	}
}

void QTreeNode::setVisible(bool underground, bool value)
{
	if(underground) {
		if(value)
			visible |= 2;
		else
			visible &= ~2;
	} else { // overground
		if(value)
			visible |= 1;
		else
			visible &= 1;
	}
}

void QTreeNode::setRequested(bool underground, bool r)
{
	if(r)
		visible |= (underground? 4 : 8);
	else
		visible &= ~(underground? 4 : 8);
}

void QTreeNode::setVisible(uint32_t client, bool underground, bool value)
{
	if(value)
		visible |= (1 << client << (underground ? MAP_LAYERS : 0));
	else
		visible &= ~(1 << client << (underground ? MAP_LAYERS : 0));
}

TileLocation* QTreeNode::getTile(int x, int y, int z)
{
	ASSERT(isLeaf);
	Floor* f = array[z];
	if(!f)
		return nullptr;
	return &f->locs[(x & 3) * 4 + (y & 3)];
}

TileLocation* QTreeNode::createTile(int x, int y, int z)
{
	ASSERT(isLeaf);
	Floor* f = createFloor(x, y, z);
	return &f->locs[(x & 3) * 4 + (y & 3)];
}

Tile* QTreeNode::setTile(int x, int y, int z, Tile* newtile)
{
	ASSERT(isLeaf);
	Floor* f = createFloor(x, y, z);

	int offset_x = x & 3;
	int offset_y = y & 3;

	TileLocation* tmp = &f->locs[offset_x*4+offset_y];
	Tile* oldtile = tmp->tile;
	tmp->tile = newtile;

	if(newtile && !oldtile)
		++map.tilecount;
	else if(oldtile && !newtile)
		--map.tilecount;

	return oldtile;
}

void QTreeNode::clearTile(int x, int y, int z)
{
	ASSERT(isLeaf);
	Floor* f = createFloor(x, y, z);

	int offset_x = x & 3;
	int offset_y = y & 3;

	TileLocation* tmp = &f->locs[offset_x*4+offset_y];
	delete tmp->tile;
	tmp->tile = map.allocator(tmp);
}
