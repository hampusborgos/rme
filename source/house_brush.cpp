//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "house_brush.h"
#include "house.h"
#include "map.h"
#include "complexitem.h"

//=============================================================================
// House Brush

HouseBrush::HouseBrush() :
	Brush(),
	draw_house(nullptr)
{
	// ...
}

HouseBrush::~HouseBrush() {
	// ...
}

void HouseBrush::setHouse(House* house) {
	draw_house = house;
}

uint32_t HouseBrush::getHouseID() const {
	if(draw_house)
		return draw_house->id;
	return 0;
}

void HouseBrush::undraw(BaseMap* map, Tile* tile) {
	if(tile->isHouseTile()) {
		tile->setPZ(false);
	}
	tile->setHouse(nullptr);
	if(settings.getInteger(Config::AUTO_ASSIGN_DOORID)) {
		// Is there a door? If so, remove any door id it has
		for(ItemVector::iterator it = tile->items.begin();
			it != tile->items.end();
			++it)
		{
			if(Door* door = dynamic_cast<Door*>(*it)) {
				door->setDoorID(0);
			}
		}
	}
}

void HouseBrush::draw(BaseMap* map, Tile* tile, void* parameter) {
	ASSERT(draw_house);
	uint32_t old_house_id = tile->getHouseID();
	tile->setHouse(draw_house);
	tile->setPZ(true);
	if(settings.getInteger(Config::HOUSE_BRUSH_REMOVE_ITEMS)) {
		// Remove loose items
		for(ItemVector::iterator it = tile->items.begin();
			it != tile->items.end();
			/*..*/)
		{
			Item* item = *it;
			if(item->isNotMoveable() == 0) {
				delete item;
				it = tile->items.erase(it);
			} else {
				++it;
			}
		}
	}
	if(settings.getInteger(Config::AUTO_ASSIGN_DOORID)) {
		// Is there a door? If so, find an empty ID and assign it (if the door doesn't already have an id.
		for(ItemVector::iterator it = tile->items.begin();
			it != tile->items.end();
			++it)
		{
			if(Door* door = dynamic_cast<Door*>(*it)) {
				if(door->getDoorID() == 0 || old_house_id != 0) {
					Map* real_map = dynamic_cast<Map*>(map);
					if(real_map) {
						door->setDoorID(draw_house->getEmptyDoorID());
					}
				}
			}
		}
	}
	// The tile will automagically be added to the house via the Action functions
	//draw_house->addTile(tile);
}

