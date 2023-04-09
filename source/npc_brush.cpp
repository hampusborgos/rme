//////////////////////////////////////////////////////////////////////
// This file is part of Canary Map Editor
//////////////////////////////////////////////////////////////////////
// Canary Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Canary Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "npc_brush.h"
#include "settings.h"
#include "tile.h"
#include "npc.h"
#include "basemap.h"
#include "spawn_npc.h"

//=============================================================================
// Npc brush

NpcBrush::NpcBrush(NpcType* type) :
	Brush(),
	npc_type(type)
{
	ASSERT(type->brush == nullptr);
	type->brush/* = this*/;
}

NpcBrush::~NpcBrush()
{
	////
}

int NpcBrush::getLookID() const
{
	return 0;
}

std::string NpcBrush::getName() const
{
	if(npc_type)
		return npc_type->name;
	return "Npc Brush";
}

bool NpcBrush::canDraw(BaseMap* map, const Position& position) const
{
	Tile* tile = map->getTile(position);
	if(npc_type && tile && !tile->isBlocking()) {
		if (tile->getLocation()->getSpawnNpcCount() != 0 || g_settings.getInteger(Config::AUTO_CREATE_SPAWN_NPC)) {
 		   if(tile->isPZ()) {
				return true;
			} else {
				return true;
			}
		}
	}
	return false;
}

void NpcBrush::undraw(BaseMap* map, Tile* tile)
{
	delete tile->npc;
	tile->npc = nullptr;
}

void NpcBrush::draw(BaseMap* map, Tile* tile, void* parameter)
{
	ASSERT(tile);
	ASSERT(parameter);
	if(canDraw(map, tile->getPosition())) {
		undraw(map, tile);
		if(npc_type) {
			if(tile->spawnNpc == nullptr && tile->getLocation()->getSpawnNpcCount() == 0) {
				// manually place npc spawn on location
				tile->spawnNpc = newd SpawnNpc(1);
			}
			tile->npc = newd Npc(npc_type);
			tile->npc->setSpawnNpcTime(*(int*)parameter);
		}
	}
}
