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

#ifndef RME_HOUSE_H_
#define RME_HOUSE_H_

#include "position.h"

class Map;
class Tile;
class Door;

class Houses;

class House {
public:
	House(Map& map);
	~House();

	void clean();
	void addTile(Tile* tile);
	void removeTile(Tile* tile);
	size_t size() const;
	std::string getDescription();

	uint32_t id;
	int rent;
	//HouseDoorList doorList;
	std::string name;
	uint32_t townid;
	bool guildhall;

	void setExit(const Position& pos);
	void setExit(Map* map, const Position& pos);
	Position getExit() const {return exit;}
	uint8_t getEmptyDoorID() const;
	Position getDoorPositionByID(uint8_t id) const;
protected:
	Map* map;
	PositionList tiles;
	Position exit;

	friend class Houses;
};

typedef std::map<uint32_t, House*> HouseMap;

class Houses {
public:
	Houses(Map& map);
	~Houses();

	uint32_t count() const {return houses.size();}

	HouseMap::iterator begin() {return houses.begin();}
	HouseMap::iterator end() {return houses.end();}
	HouseMap::const_iterator begin() const {return houses.begin();}
	HouseMap::const_iterator end() const {return houses.end();}
	HouseMap::iterator erase(HouseMap::iterator iter) {return houses.erase(iter);}
	HouseMap::iterator find(uint32_t val) {return houses.find(val);}

	void removeHouse(House* house_to_remove);
	void addHouse(House* new_house);
	House* getHouse(uint32_t houseid);
	const House* getHouse(uint32_t houseid) const;
	uint32_t getEmptyID();
protected:
	Map& map;
	uint32_t max_house_id;
	HouseMap houses;
};

#endif
