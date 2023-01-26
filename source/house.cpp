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

#include "complexitem.h" // Door
#include "house.h"
#include "tile.h"
#include "map.h"

Houses::Houses(Map& map) :
	map(map),
	max_house_id(0)
{
	////
}

Houses::~Houses()
{
	for(auto it = houses.begin(); it != houses.end(); ++it)
		delete it->second;
	houses.clear();
}

uint32_t Houses::getEmptyID()
{
	return ++max_house_id;
}

void Houses::addHouse(House* new_house)
{
	ASSERT(new_house);
	HouseMap::iterator it = houses.find(new_house->id);
	ASSERT(it == houses.end());
	new_house->map = &map;
	if(new_house->id > max_house_id)
		max_house_id = new_house->id;
	houses[new_house->id] = new_house;
}

void Houses::removeHouse(House* house_to_remove)
{
	HouseMap::iterator it = houses.find(house_to_remove->id);
	if(it != houses.end())
		houses.erase(it);

	house_to_remove->clean();
	delete house_to_remove;
}

House* Houses::getHouse(uint32_t houseid)
{
	HouseMap::iterator it = houses.find(houseid);
	if(it != houses.end()) {
		return it->second;
	}
	return nullptr;
}

const House* Houses::getHouse(uint32_t houseid) const
{
	HouseMap::const_iterator it = houses.find(houseid);
	if(it != houses.end())
		return it->second;

	return nullptr;
}

House::House(Map& map) :
	id(0),
	rent(0),
	townid(0),
	guildhall(false),
	map(&map),
	exit(0,0,0)
{
	////
}

void House::clean()
{
	for(PositionList::const_iterator pos_iter = tiles.begin(); pos_iter != tiles.end(); ++pos_iter) {
		Tile* tile = map->getTile(*pos_iter);
		if(tile)
			tile->setHouse(nullptr);
	}

	Tile* tile = map->getTile(exit);
	if(tile)
		tile->removeHouseExit(this);
}

size_t House::size() const
{
	size_t count = 0;
	for(PositionList::const_iterator pos_iter = tiles.begin(); pos_iter != tiles.end(); ++pos_iter) {
		Tile* tile = map->getTile(*pos_iter);
		if(tile && !tile->isBlocking())
			++count;
	}
	return count;
}

void House::addTile(Tile* tile)
{
	ASSERT(tile);
	tile->setHouse(this);
	tiles.push_back(tile->getPosition());
}

void House::removeTile(Tile* tile)
{
	ASSERT(tile);
	for(PositionList::iterator tile_iter = tiles.begin(); tile_iter != tiles.end(); ++tile_iter) {
		if(*tile_iter == tile->getPosition()) {
			tiles.erase(tile_iter);
			tile->setHouse(nullptr);
			return;
		}
	}
}

uint8_t House::getEmptyDoorID() const
{
	std::set<uint8_t> taken;
	for(PositionList::const_iterator tile_iter = tiles.begin(); tile_iter != tiles.end(); ++tile_iter) {
		if(const Tile* tile = map->getTile(*tile_iter)) {
			for(ItemVector::const_iterator item_iter = tile->items.begin(); item_iter != tile->items.end(); ++item_iter) {
				if(Door* door = dynamic_cast<Door*>(*item_iter))
					taken.insert(door->getDoorID());
			}
		}
	}

	for(int i = 1; i < 256; ++i) {
		std::set<uint8_t>::iterator it = taken.find(uint8_t(i));
		if(it == taken.end()) {
			// Free ID!
			return i;
		}
	}
	return 255;
}

Position House::getDoorPositionByID(uint8_t id) const
{
	for(PositionList::const_iterator tile_iter = tiles.begin(); tile_iter != tiles.end(); ++tile_iter) {
		if(const Tile* tile = map->getTile(*tile_iter)) {
			for(ItemVector::const_iterator item_iter = tile->items.begin(); item_iter != tile->items.end(); ++item_iter) {
				if(Door* door = dynamic_cast<Door*>(*item_iter)) {
					if(door->getDoorID() == id) {
						return *tile_iter;
					}
				}
			}
		}
	}
	return Position();
}

std::string House::getDescription()
{
	std::ostringstream os;
	os << name;
	os << " (ID:" << id << "; Rent: " << rent << ")";
	return os.str();
}

void House::setExit(Map* targetmap, const Position& pos)
{
	// This might fail when the user decides to put an exit at 0,0,0, let's just hope noone does (Noone SHOULD, so there is no problem? Hm?)
	if(pos == exit || !pos.isValid())
		return;

	if(exit.isValid()) {
		Tile* oldexit = targetmap->getTile(exit);
		if(oldexit)
			oldexit->removeHouseExit(this);
	}

	Tile* newexit = targetmap->getTile(pos);
	if(!newexit) {
		newexit = targetmap->allocator(targetmap->createTileL(pos));
		targetmap->setTile(pos, newexit);
	}

	newexit->addHouseExit(this);
	exit = pos;
}

void House::setExit(const Position& pos)
{
	setExit(map, pos);
}
