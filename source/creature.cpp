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

#include "creature.h"

Creature::Creature(CreatureType* ctype) : direction(NORTH), spawntime(0), saved(false), selected(false)
{
	if(ctype)
		type_name = ctype->name;
}

Creature::Creature(std::string ctype_name) : type_name(ctype_name), direction(NORTH), spawntime(0), saved(false), selected(false)
{
	////
}

Creature::~Creature()
{
	////
}

std::string Creature::DirID2Name(uint16_t id) {
	switch (id) {
	case NORTH: return "North";
	case EAST: return "East";
	case SOUTH: return "South";
	case WEST: return "West";
	default: return "Unknown";
	}
}

uint16_t Creature::DirName2ID(std::string dir)
{
	to_lower_str(dir);
	if(dir == "north") return NORTH;
	if(dir == "east") return EAST;
	if(dir == "south") return SOUTH;
	if(dir == "west") return WEST;
	return SOUTH;
}

Creature* Creature::deepCopy() const
{
	Creature* copy = newd Creature(type_name);
	copy->spawntime = spawntime;
	copy->direction = direction;
	copy->selected = selected;
	copy->saved = saved;
	return copy;
}

const Outfit& Creature::getLookType() const
{
	CreatureType* type = g_creatures[type_name];
	if(type)
		return type->outfit;
	static const Outfit otfi; // Empty outfit
	return otfi;
}
