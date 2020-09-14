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

#ifndef RME_HOUSE_BRUSH_H
#define RME_HOUSE_BRUSH_H

#include "brush.h"

//=============================================================================
// HouseBrush, draw house tiles
// This brush is created on demand and NOT loaded, as such, the load() method is empty
// Should be deleted by the owning palette

// Forward declaration
class HouseBrush : public Brush
{
public:
	HouseBrush();
	virtual ~HouseBrush();

	bool isHouse() const { return true; }
	HouseBrush* asHouse() { return static_cast<HouseBrush*>(this); }

	// Not used
	virtual bool load(pugi::xml_node node, wxArrayString& warnings) {return true;}

	// You can always draw house tiles!
	virtual bool canDraw(BaseMap* map, const Position& position) const {return true;}
	// Draw the shit!
	virtual void draw(BaseMap* map, Tile* tile, void* parameter);
	// Undraw the shit!
	virtual void undraw(BaseMap* map, Tile* tile);

	virtual bool canDrag() const {return true;}

	void setHouse(House* house);

	uint32_t getHouseID() const;
	virtual int getLookID() const { return 0; } // We don't have a graphic
	virtual std::string getName() const { return "House Brush"; }

protected:
	House* draw_house;
};

#endif
