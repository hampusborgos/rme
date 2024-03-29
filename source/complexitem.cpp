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

#include "complexitem.h"

#include "iomap.h"

// Container
Container::Container(const uint16_t type) : Item(type, 0)
{
	////
}

Container::~Container()
{
	for(Item* item : contents) {
		delete item;
	}
	contents.clear();
}

Item* Container::deepCopy() const
{
	Item* copy = Item::deepCopy();
	if(Container* container = copy->getContainer()) {
		for(const Item* item : contents) {
			container->contents.push_back(item->deepCopy());
		}
	}
	return copy;
}

Item* Container::getItem(size_t index) const
{
	if(index >= 0 && index < contents.size()) {
		return contents.at(index);
	}
	return nullptr;
}

// Teleport
Teleport::Teleport(const uint16_t type) : Item(type, 0),
	destination(0, 0, 0)
{
	////
}

Item* Teleport::deepCopy() const
{
	Item* copy = Item::deepCopy();
	if(Teleport* teleport = copy->getTeleport()) {
		teleport->setDestination(destination);
	}
	return copy;
}

// Door
Door::Door(const uint16_t type) : Item(type, 0),
	doorId(0)
{
	////
}

Item* Door::deepCopy() const
{
	Item* copy = Item::deepCopy();
	if(Door* door = copy->getDoor()) {
		door->doorId = doorId;
	}
	return copy;
}

// Depot
Depot::Depot(const uint16_t type) : Item(type, 0),
	depotId(0)
{
	////
}

Item* Depot::deepCopy() const
{
	Item* copy = Item::deepCopy();
	if(Depot* depot = copy->getDepot()) {
		depot->depotId = depotId;
	}
	return copy;
}
