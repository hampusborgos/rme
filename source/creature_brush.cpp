//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "creature_brush.h"
#include "settings.h"
#include "tile.h"
#include "creature.h"
#include "basemap.h"
#include "spawn.h"

//=============================================================================
// Creature brush

CreatureBrush::CreatureBrush(CreatureType* type) :
	Brush(),
	creature_type(type)
{
	ASSERT(type->brush == nullptr);
	type->brush = this;
}

CreatureBrush::~CreatureBrush() {

}

int CreatureBrush::getLookID() const {
	return 0;
}

std::string CreatureBrush::getName() const {
	if(creature_type)
		return creature_type->name;
	return "Creature Brush";
}

bool CreatureBrush::canDraw(BaseMap* map, const Position& position) const
{
	Tile* tile = map->getTile(position);
	if (creature_type && tile && !tile->isBlocking()) {
        if (tile->getLocation()->getSpawnCount() != 0 || settings.getInteger(Config::AUTO_CREATE_SPAWN)) {
            if (tile->isPZ()) {
                if (creature_type->isNpc) {
                    return true;
                }
            } else {
                return true;
            }
        }
	}
	return false;
}

void CreatureBrush::undraw(BaseMap* map, Tile* tile) {
	delete tile->creature;
	tile->creature = nullptr;
}

void CreatureBrush::draw(BaseMap* map, Tile* tile, void* parameter) {
	ASSERT(tile);
	ASSERT(parameter);
	if(canDraw(map, tile->getPosition())) {
		undraw(map, tile);
		if(creature_type) {
            if (tile->spawn == nullptr && tile->getLocation()->getSpawnCount() == 0) {
                // manually place spawn on location
                tile->spawn = newd Spawn(1);
            }
			tile->creature = newd Creature(creature_type);
			tile->creature->setSpawnTime(*(int*)parameter);
		}
	}
}
