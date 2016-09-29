//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/basemap.h $
// $Id: basemap.h 278 2010-02-14 22:38:30Z admin $

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

	for(int i = 0; i < 16; ++i) {
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
	for(int i = 0; i < 16; ++i)
		child[i] = nullptr;
}

QTreeNode::~QTreeNode()
{
	if(isLeaf) {
		delete array[0];
		delete array[1];
		delete array[2];
		delete array[3];
		delete array[4];
		delete array[5];
		delete array[6];
		delete array[7];
		delete array[8];
		delete array[9];
		delete array[10];
		delete array[11];
		delete array[12];
		delete array[13];
		delete array[14];
		delete array[15];
	} else {
		delete child[0];
		delete child[1];
		delete child[2];
		delete child[3];
		delete child[4];
		delete child[5];
		delete child[6];
		delete child[7];
		delete child[8];
		delete child[9];
		delete child[10];
		delete child[11];
		delete child[12];
		delete child[13];
		delete child[14];
		delete child[15];
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
		for(int i = 0; i < 16; ++i)
			if(child[i])
				child[i]->clearVisible(u);
}

bool QTreeNode::isVisible(uint32_t client, bool underground)
{
	if(underground) {
		return testFlags(visible >> 16, 1 << client);
	} else {
		return testFlags(visible, 1 << client);
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
		visible |= (1 << client << (underground? 16 : 0));
	else
		visible &= ~(1 << client << (underground? 16 : 0));
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


