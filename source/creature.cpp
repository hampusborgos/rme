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

#include "creature.h"

#include "main.h"

Creature::Creature(CreatureType* type) : direction(NORTH), spawntime(0), saved(false), selected(false)
{
	if (type) type_name = type->name;
}

Creature::Creature(const std::string& type_name) :
    type_name(type_name), direction(NORTH), spawntime(0), saved(false), selected(false)
{
	////
}

Creature* Creature::deepCopy() const
{
	Creature* copy = new Creature(type_name);
	copy->spawntime = spawntime;
	copy->direction = direction;
	copy->selected = selected;
	copy->saved = saved;
	return copy;
}

const Outfit& Creature::getLookType() const
{
	const CreatureType* type = g_creatures[type_name];
	if (type) return type->outfit;
	static const Outfit otfi; // Empty outfit
	return otfi;
}

bool Creature::isNpc() const
{
	const CreatureType* type = g_creatures[type_name];
	if (type) {
		return type->isNpc;
	}
	return false;
}

std::string Creature::getName() const
{
	const CreatureType* type = g_creatures[type_name];
	if (type) {
		return type->name;
	}
	return "";
}

CreatureBrush* Creature::getBrush() const
{
	const CreatureType* type = g_creatures[type_name];
	if (type) {
		return type->brush;
	}
	return nullptr;
}

std::string Creature::DirID2Name(uint16_t id)
{
	switch (id) {
		case NORTH:
			return "North";
		case EAST:
			return "East";
		case SOUTH:
			return "South";
		case WEST:
			return "West";
		default:
			return "Unknown";
	}
}

uint16_t Creature::DirName2ID(std::string dir)
{
	to_lower_str(dir);
	if (dir == "north") return NORTH;
	if (dir == "east") return EAST;
	if (dir == "south") return SOUTH;
	if (dir == "west") return WEST;
	return SOUTH;
}
