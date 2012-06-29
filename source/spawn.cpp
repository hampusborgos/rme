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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/spawn.hpp $
// $Id: spawn.hpp 264 2009-10-05 06:36:21Z remere $

#include "main.h"

#include "tile.h"
#include "spawn.h"

Spawns::Spawns() {
	;
}

Spawns::~Spawns() {
	;
}

void Spawns::addSpawn(Tile* tile) {
	ASSERT(tile->spawn);
	std::pair<SpawnPositionList::iterator, bool> s = spawns.insert(tile->getPosition());
	ASSERT(s.second);
}

void Spawns::removeSpawn(Tile* tile) {
	ASSERT(tile->spawn);
	spawns.erase(tile->getPosition());
#if 0
	SpawnPositionList::iterator iter = begin();
	while(iter != end()) {
		if(*iter == tile->getPosition()) {
			spawns.erase(iter);
			return;
		}
		++iter;
	}
	ASSERT(false);
#endif
}

std::ostream& operator<<(std::ostream& os, const Spawn& spawn) {
	os << &spawn << ":: -> " << spawn.getSize() << std::endl;
	return os;
}
