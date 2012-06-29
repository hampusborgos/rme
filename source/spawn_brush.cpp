//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
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
}

SpawnBrush::~SpawnBrush() {

}

int SpawnBrush::getLookID() const {
	return 0;
}

std::string SpawnBrush::getName() const {
	return "Spawn Brush";
}

bool SpawnBrush::canDraw(BaseMap* map, Position pos) const {
	Tile* tile = map->getTile(pos);
	if(tile) {
		if(tile->spawn) {
			return false;
		}
	}
	return true;
}

void SpawnBrush::undraw(BaseMap* map, Tile* tile) {
	delete tile->spawn;
	tile->spawn = NULL;
}

void SpawnBrush::draw(BaseMap* map, Tile* tile, void* parameter) {
	ASSERT(tile);
	ASSERT(parameter); // Should contain an int which is the size of the newd spawn
	if(tile->spawn == NULL) {
		tile->spawn = newd Spawn(max(1, *(int*)parameter));
	}
}
