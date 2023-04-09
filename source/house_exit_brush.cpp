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

#include "house_exit_brush.h"
#include "house.h"
#include "basemap.h"

//=============================================================================
// House Exit Brush

HouseExitBrush::HouseExitBrush() :
	Brush(),
	draw_house(0)
{
	////
}

HouseExitBrush::~HouseExitBrush()
{
	////
}

void HouseExitBrush::setHouse(House* house)
{
	draw_house = house->id;
}

uint32_t HouseExitBrush::getHouseID() const
{
	return draw_house;
}


bool HouseExitBrush::canDraw(BaseMap* map, const Position& position) const
{
	Tile* tile = map->getTile(position);
	if(!tile || !tile->hasGround()) {
		return false;
	}
	if(tile->isHouseTile() || tile->isBlocking()) {
		return false;
	}
	return true;
}

void HouseExitBrush::undraw(BaseMap* map, Tile* tile)
{
	// Never called
	ASSERT(false);
}

void HouseExitBrush::draw(BaseMap* map, Tile* tile, void* parameter)
{
	// Never called
	ASSERT(false);
}
