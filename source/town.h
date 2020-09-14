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

#ifndef RME_TOWN_H_
#define RME_TOWN_H_

#include "position.h"

class Town
{
public:
	Town(uint32_t _id) : id(_id), name("") {}
	Town(const Town& copy) : id(copy.id), name(copy.name), templepos(copy.templepos) {}
	~Town() {}

	const std::string& getName() const { return name; }
	void setName(const std::string& newName) { name = newName; }

	const Position& getTemplePosition() const {return templepos;}
	void setTemplePosition(const Position& _pos);

	uint32_t getID() const {return id;}
	void setID(uint32_t _id) {id = _id;}
private:
	uint32_t id;
	std::string name;
	Position templepos;
};

typedef std::map<uint32_t, Town*> TownMap;

class Towns
{
public:
	Towns();
	~Towns();

	// Clears the townmap
	void clear();

	uint32_t count() const {return towns.size();}

	bool addTown(Town* town);
	uint32_t getEmptyID();

	Town* getTown(std::string& townname);
	Town* getTown(uint32_t _townid);

	TownMap::const_iterator begin() const{return towns.begin();}
	TownMap::const_iterator end() const{return towns.end();}
	TownMap::const_iterator find(uint32_t id) const {return towns.find(id);}
	TownMap::iterator begin() {return towns.begin();}
	TownMap::iterator end() {return towns.end();}
	TownMap::iterator find(uint32_t id) {return towns.find(id);}
#ifdef __VISUALC__ // C++0x compliance to some degree :)
	TownMap::iterator erase(TownMap::iterator iter) {return towns.erase(iter);}
#else
	void erase(TownMap::iterator iter) {towns.erase(iter);}
#endif

private:
	TownMap towns;
};

#endif
