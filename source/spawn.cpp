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

#include "spawn.h"

#include "main.h"
#include "tile.h"

Spawn::Spawn(int size) : size(0), selected(false) { setSize(size); }

Spawn* Spawn::deepCopy() const
{
	Spawn* copy = new Spawn(size);
	copy->selected = selected;
	return copy;
}

void Spawns::addSpawn(Tile* tile)
{
	ASSERT(tile->spawn);

	auto it = spawns.insert(tile->getPosition());
	ASSERT(it.second);
}

void Spawns::removeSpawn(Tile* tile)
{
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

std::ostream& operator<<(std::ostream& os, const Spawn& spawn)
{
	os << &spawn << ":: -> " << spawn.getSize() << std::endl;
	return os;
}
