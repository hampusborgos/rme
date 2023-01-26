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

#include "tile.h"
#include "basemap.h"

BaseMap::BaseMap() :
	allocator(),
	tilecount(0),
	root(*this)
{
	////
}

BaseMap::~BaseMap()
{
	////
}

void BaseMap::clear(bool del)
{
	PositionVector pos_vec;
	for(MapIterator map_iter = begin(); map_iter != end(); ++map_iter) {
		Tile* t = (*map_iter)->get();
		pos_vec.push_back(t->getPosition());
	}
	for(PositionVector::iterator pos_iter = pos_vec.begin(); pos_iter != pos_vec.end(); ++pos_iter) {
		setTile(*pos_iter, nullptr, del);
	}
}

void BaseMap::clearVisible(uint32_t mask)
{
	root.clearVisible(mask);
}

Tile* BaseMap::createTile(int x, int y, int z)
{
	ASSERT(z < MAP_LAYERS);
	QTreeNode* leaf = root.getLeafForce(x, y);
	TileLocation* loc = leaf->createTile(x, y, z);
	if(loc->get())
		return loc->get();
	Tile* t = allocator(loc);
	leaf->setTile(x, y, z, t);
	return t;
}

TileLocation* BaseMap::getTileL(int x, int y, int z)
{
	ASSERT(z < MAP_LAYERS);
	QTreeNode* leaf = root.getLeaf(x, y);
	if(leaf) {
		Floor* floor = leaf->getFloor(z);
		if(floor)
			return &floor->locs[(x & 3)*4 + (y & 3)];
	}
	return nullptr;
}

const TileLocation* BaseMap::getTileL(int x, int y, int z) const
{
	// Don't create static const maps!
	BaseMap* self = const_cast<BaseMap*>(this);
	return self->getTileL(x, y, z);
}

TileLocation* BaseMap::getTileL(const Position& pos)
{
	return getTileL(pos.x, pos.y, pos.z);
}

const TileLocation* BaseMap::getTileL(const Position& pos) const
{
	return getTileL(pos.x, pos.y, pos.z);
}

TileLocation* BaseMap::createTileL(int x, int y, int z)
{
	ASSERT(z < MAP_LAYERS);

	QTreeNode* leaf = root.getLeafForce(x, y);
	Floor* floor = leaf->createFloor(x, y, z);
	uint32_t offsetX = x & 3;
	uint32_t offsetY = y & 3;

	return &floor->locs[offsetX*4+offsetY];
}

TileLocation* BaseMap::createTileL(const Position& pos)
{
	return createTileL(pos.x, pos.y, pos.z);
}

void BaseMap::setTile(int x, int y, int z, Tile* new_tile, bool remove)
{
	ASSERT(!new_tile || new_tile->getX() == x);
	ASSERT(!new_tile || new_tile->getY() == y);
	ASSERT(!new_tile || new_tile->getZ() == z);

	QTreeNode* leaf = root.getLeafForce(x, y);
	Tile* old_tile = leaf->setTile(x, y, z, new_tile);

	if ((remove && old_tile) || new_tile)
		updateUniqueIds(remove ? old_tile : nullptr, new_tile);

	if (remove) {
		delete old_tile;
	}
}

void BaseMap::setTile(const Position& position, Tile* new_tile, bool remove)
{
	setTile(position.x, position.y, position.z, new_tile, remove);
}

void BaseMap::setTile(Tile* new_tile, bool remove)
{
	ASSERT(new_tile);

	const Position& position = new_tile->getPosition();
	setTile(position.x, position.y, position.z, new_tile, remove);
}

Tile* BaseMap::swapTile(int x, int y, int z, Tile* new_tile)
{
	ASSERT(z < MAP_LAYERS);
	ASSERT(!new_tile || new_tile->getX() == x);
	ASSERT(!new_tile || new_tile->getY() == y);
	ASSERT(!new_tile || new_tile->getZ() == z);

	QTreeNode* leaf = root.getLeafForce(x, y);
	Tile* old_tile = leaf->setTile(x, y, z, new_tile);

	if (old_tile || new_tile)
		updateUniqueIds(old_tile, new_tile);

	return old_tile;
}

Tile* BaseMap::swapTile(const Position& position, Tile* new_tile)
{
	return swapTile(position.x, position.y, position.z, new_tile);
}

// Iterators

MapIterator::MapIterator(BaseMap* _map) :
	local_i(0),
	local_z(0),
	current_tile(nullptr),
	map(_map)
{
	////
}

MapIterator::~MapIterator()
{
	////
}

MapIterator::MapIterator(const MapIterator& other)
{
	for(std::vector<MapIterator::NodeIndex>::const_iterator it = other.nodestack.begin(); it != other.nodestack.end(); it++) {
		nodestack.push_back(MapIterator::NodeIndex(*it));
	}
	local_i = other.local_i;
	local_z = other.local_z;
	map = other.map;
	current_tile = other.current_tile;
}

MapIterator BaseMap::begin()
{
	MapIterator it(this);
	it.nodestack.push_back(MapIterator::NodeIndex(&root));

	while(true) {
		MapIterator::NodeIndex& current = it.nodestack.back();
		QTreeNode* node = current.node;
		int& index = current.index;
		//printf("Contemplating %p of %p (stack size %d)\n", node, this, it.nodestack.size());

		bool unwind = false;
		for(; index < 16; ++index) {
			//printf("\tChecking index %d of %p\n", index, node);
			if(QTreeNode* child = node->child[index]) {
				if(child->isLeaf) {
					QTreeNode* leaf = child;
					//printf("\t%p is leaf\n", child);
					for(it.local_z = 0; it.local_z < MAP_LAYERS; ++it.local_z) {
						if(Floor* floor = leaf->array[it.local_z]) {
							for(it.local_i = 0; it.local_i < 16; ++it.local_i) {
								//printf("\tit(%d;%d;%d)\n", it.local_x, it.local_y, it.local_z);
								TileLocation& t = floor->locs[it.local_i];
								if(t.get()) {
									//printf("return it\n");
									it.current_tile = &t;
									return it;
								}
							}
						}
					}
				} else {
					//printf("\tAdding %p\n", child);
					++index;
					it.nodestack.push_back(MapIterator::NodeIndex(child));
					unwind = true;
					break;
				}
			}
		}
		if(unwind)
			continue;

		//printf("Discarding dead node %p\n", node);
		it.nodestack.pop_back();
		if(it.nodestack.empty())
			break;
	}
	return end();
}

MapIterator BaseMap::end()
{
	MapIterator it(this);
	it.local_i = -1;
	it.local_z = -1;
	return it;
}

TileLocation* MapIterator::operator*()
{
	return current_tile;
}

TileLocation* MapIterator::operator->()
{
	return current_tile;
}

MapIterator& MapIterator::operator++()
{
	//printf("MapIterator::operator++");
	bool increased = false;
	bool first = true;
	while(true) {
		MapIterator::NodeIndex& current = nodestack.back();
		QTreeNode* node = current.node;
		int& index = current.index;
		//printf("Contemplating %p (stack size %d)\n", node, nodestack.size());

		bool unwind = false;
		for(; index < MAP_LAYERS; ++index) {
			//printf("\tChecking index %d of %p\n", index, node);
			if(QTreeNode* child = node->child[index]) {
				if(child->isLeaf) {
					QTreeNode* leaf = child;
					//printf("\t%p is leaf\n", child);
					for(; local_z < MAP_LAYERS; ++local_z) {
						//printf("\t\tIterating over Z:%d of %p", local_z, child);
						if(Floor* floor = leaf->array[local_z]) {
							//printf("\n");
							for(; local_i < MAP_LAYERS; ++local_i) {
								//printf("\t\tIterating over Y:%d of %p\n", local_y, child);
								TileLocation& t = floor->locs[local_i];
								if(t.get()) {
									if(increased) {
										//printf("Modified %p to %p\n", current_tile, t);
										current_tile = &t;
										return *this;
									} else {
										increased = true;
									}
								} else if(first) {
									increased = true;
									first = false;
								}
							}

							if(local_i > MAP_MAX_LAYER) {
								//printf("\t\tReset local_x\n");
								local_i = 0;
							}
						} else {
							//printf(":dead floor\n");
						}
					} if(local_z == MAP_LAYERS) {
							//printf("\t\tReset local_z\n");
							local_z = 0;
					}
				} else {
					//printf("\tAdding %p\n", child);
					++index;
					nodestack.push_back(MapIterator::NodeIndex(child));
					unwind = true;
					break;
				}
			}
		}
		if(unwind)
			continue;

		//printf("Discarding dead node %p\n", node);
		nodestack.pop_back();
		if(nodestack.size() == 0) {
			// Set all values to "end"
			//printf("END\n");
			local_z = -1;
			local_i = -1;
			return *this;
		}
	}
	return *this;
}

MapIterator MapIterator::operator++(int)
{
	MapIterator i(*this);
	++*this;
	return i;
}
