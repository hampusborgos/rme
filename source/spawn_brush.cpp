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

#include "spawn_brush.h"
#include "basemap.h"
#include "spawn.h"

//=============================================================================
// Spawn brush

SpawnBrush::SpawnBrush() :
	Brush()
{
	////
}

SpawnBrush::~SpawnBrush()
{
	////
}

int SpawnBrush::getLookID() const
{
	return 0;
}

std::string SpawnBrush::getName() const
{
	return "Spawn Brush";
}

bool SpawnBrush::canDraw(BaseMap* map, const Position& position) const
{
	Tile* tile = map->getTile(position);
	if(tile) {
		if(tile->spawn) {
			return false;
		}
	}
	return true;
}

void SpawnBrush::undraw(BaseMap* map, Tile* tile)
{
	delete tile->spawn;
	tile->spawn = nullptr;
}

void SpawnBrush::draw(BaseMap* map, Tile* tile, void* parameter)
{
	ASSERT(tile);
	ASSERT(parameter); // Should contain an int which is the size of the newd spawn
	if(tile->spawn == nullptr) {
		tile->spawn = newd Spawn(max(1, *(int*)parameter));
	}
}
