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

#include "monster_brush.h"
#include "settings.h"
#include "tile.h"
#include "monster.h"
#include "basemap.h"
#include "spawn_monster.h"

//=============================================================================
// Monster brush

MonsterBrush::MonsterBrush(MonsterType* type) :
	Brush(),
	monster_type(type)
{
	ASSERT(type->brush == nullptr);
	type->brush = this;
}

MonsterBrush::~MonsterBrush()
{
	////
}

int MonsterBrush::getLookID() const
{
	return 0;
}

std::string MonsterBrush::getName() const
{
	if(monster_type)
		return monster_type->name;
	return "Monster Brush";
}

bool MonsterBrush::canDraw(BaseMap* map, const Position& position) const
{
	Tile* tile = map->getTile(position);
	if(monster_type && tile && !tile->isBlocking()) {
		if(tile->getLocation()->getSpawnMonsterCount() != 0 || g_settings.getInteger(Config::AUTO_CREATE_SPAWN_MONSTER)) {
 			if(tile->isPZ()) {
				return false;
			} else {
				return true;
			}
		}
	}
	return false;
}

void MonsterBrush::undraw(BaseMap* map, Tile* tile)
{
	delete tile->monster;
	tile->monster = nullptr;
}

void MonsterBrush::draw(BaseMap* map, Tile* tile, void* parameter)
{
	ASSERT(tile);
	ASSERT(parameter);
	if(canDraw(map, tile->getPosition())) {
		undraw(map, tile);
		if(monster_type) {
			if(tile->spawnMonster == nullptr && tile->getLocation()->getSpawnMonsterCount() == 0) {
				// manually place spawnMonster on location
				tile->spawnMonster = newd SpawnMonster(1);
			}
			tile->monster = newd Monster(monster_type);
			tile->monster->setSpawnMonsterTime(*(int*)parameter);
		}
	}
}
