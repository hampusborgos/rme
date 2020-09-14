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

#ifndef RME_RAW_BRUSH_H
#define RME_RAW_BRUSH_H

#include "brush.h"

//=============================================================================
// RAWBrush, draw items like SimOne's editor

class RAWBrush : public Brush
{
public:
	RAWBrush(uint16_t itemid); // Create a RAWBrush of the specified type
	virtual ~RAWBrush();

	bool isRaw() const { return true; }
	RAWBrush* asRaw() { return static_cast<RAWBrush*>(this); }

	virtual bool canDraw(BaseMap* map, const Position& position) const { return true; }
	virtual void draw(BaseMap* map, Tile* tile, void* parameter);
	virtual void undraw(BaseMap* map, Tile* tile);

	virtual bool canDrag() const { return true; }
	virtual int getLookID() const;
	virtual std::string getName() const;
	ItemType* getItemType() const { return itemtype; }
	uint16_t getItemID() const;

protected:
	ItemType* itemtype;
};

#endif
