//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "house_exit_brush.h"
#include "house.h"
#include "basemap.h"

//=============================================================================
// House Exit Brush

HouseExitBrush::HouseExitBrush() :
	Brush(),
	draw_house(0)
{
}

HouseExitBrush::~HouseExitBrush() {
}

void HouseExitBrush::setHouse(House* house) {
	draw_house = house->id;
}

uint32_t HouseExitBrush::getHouseID() const {
	return draw_house;
}


bool HouseExitBrush::canDraw(BaseMap* map, const Position& position) const
{
	Tile* tile = map->getTile(position);
	if(!tile || !tile->hasGround()) {
		return false;
	}
	if(!tile->isHouseTile() || !tile->isBlocking()) {
		return false;
	}
	return true;
}

void HouseExitBrush::undraw(BaseMap* map, Tile* tile) {
	// Never called
	ASSERT(false);
}

void HouseExitBrush::draw(BaseMap* map, Tile* tile, void* parameter) {
	// Never called
	ASSERT(false);
}
