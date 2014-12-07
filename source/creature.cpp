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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/creature.hpp $
// $Id: creature.hpp 310 2010-02-26 18:03:48Z admin $

#include "main.h"

#include "creature.h"

Creature::Creature(CreatureType* ctype) : direction(SOUTH), spawntime(0), saved(false), selected(false)
{
	if(ctype)
		type_name = ctype->name;
}

Creature::Creature(std::string ctype_name) : type_name(ctype_name), direction(SOUTH), spawntime(0), saved(false), selected(false)
{
}

Creature::~Creature()
{
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
	CreatureType* type = creature_db[type_name];
	if(type)
		return type->outfit;
	static const Outfit otfi; // Empty outfit
	return otfi;
}
