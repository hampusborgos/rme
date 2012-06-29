//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/complexitem.hpp $
// $Id: complexitem.hpp 264 2009-10-05 06:36:21Z remere $

#include "main.h"

#include "complexitem.h"

#include "iomap.h"

// ============================================================================
// Container

Container::Container(const unsigned short _type) : Item(_type, 0) {
	// ...
}

Container::~Container() {
	for(ItemVector::iterator it = contents.begin(); it != contents.end(); ++it) {
		delete *it;
	}
}

Item* Container::deepCopy() const {
	Container* copy = static_cast<Container*>(Item::deepCopy());
	for(ItemVector::const_iterator it = contents.begin(); it != contents.end(); ++it) {
		copy->contents.push_back((*it)->deepCopy());
	}
	return copy;
}

ItemVector& Container::getVector() {
	return contents;
}

Item* Container::getItem(uint index) {
	if(index >= contents.size()) {
		return NULL;
	}
	return contents.at(index);
}

double Container::getWeight() {
	return item_db[id].weight;
}

// ============================================================================
// Teleport

Teleport::Teleport(const unsigned short _type) :
	Item(_type, 0),
	destination(0,0,0)
{
	// ...
}

Teleport::~Teleport() {
	// ...
}

Item* Teleport::deepCopy() const {
	Teleport* copy = static_cast<Teleport*>(Item::deepCopy());
	copy->destination = destination;
	return copy;
}

// ============================================================================
// Door

Door::Door(const unsigned short _type) :
	Item(_type, 0),
	doorid(0)
{
	// ...
}

Door::~Door() {
	// ...
}

Item* Door::deepCopy() const {
	Door* copy = static_cast<Door*>(Item::deepCopy());
	copy->doorid = doorid;
	return copy;
}

// ============================================================================
// Depot

Depot::Depot(const unsigned short _type) :
	Item(_type, 0),
	depotid(0)
{
	// ...
}

Depot::~Depot() {
	// ...
}

Item* Depot::deepCopy() const {
	Depot* copy = static_cast<Depot*>(Item::deepCopy());
	copy->depotid = depotid;
	return copy;
}
