//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "brush.h"
#include "sprites.h"
#include "basemap.h"
#include "settings.h"

//=============================================================================
// Eraser brush

EraserBrush::EraserBrush() {
}

EraserBrush::~EraserBrush() {
}

std::string EraserBrush::getName() const {
	return "Eraser";
}

int EraserBrush::getLookID() const {
	return EDITOR_SPRITE_ERASER;
}

bool EraserBrush::canDraw(BaseMap* map, const Position& position) const {
	return true;
}

void EraserBrush::undraw(BaseMap* map, Tile* tile) {
	for(ItemVector::iterator item_iter = tile->items.begin();
			item_iter != tile->items.end();)
	{
		Item* item = *item_iter;
		if(item->isComplex() && settings.getInteger(Config::ERASER_LEAVE_UNIQUE)) {
			++item_iter;
		} else {
			delete item;
			item_iter = tile->items.erase(item_iter);
		}
	}
	if(tile->ground) {
		if(settings.getInteger(Config::ERASER_LEAVE_UNIQUE)) {
			if(tile->ground->isComplex() == false) {
				delete tile->ground;
				tile->ground = nullptr;
			}
		} else {
			delete tile->ground;
			tile->ground = nullptr;
		}
	}
}

void EraserBrush::draw(BaseMap* map, Tile* tile, void* parameter) {
	// Draw is undraw, undraw is super-undraw!
	for(ItemVector::iterator item_iter = tile->items.begin();
			item_iter != tile->items.end();)
	{
		Item* item = *item_iter;
		if((item->isComplex() || item->isBorder()) && settings.getInteger(Config::ERASER_LEAVE_UNIQUE)) {
			++item_iter;
		//} else if(item->getDoodadBrush()) {
			//++item_iter;
		} else {
			delete item;
			item_iter = tile->items.erase(item_iter);
		}
	}
}
