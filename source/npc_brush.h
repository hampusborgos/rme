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

#ifndef RME_NPC_BRUSH_H
#define RME_NPC_BRUSH_H

#include "brush.h"

//=============================================================================
// NpcBrush, place npcs

class NpcBrush : public Brush
{
public:
	NpcBrush(NpcType* type); // Create a RAWBrush of the specified type
	virtual ~NpcBrush();

	bool isNpc() const { return true; }
	NpcBrush* asNpc() { return static_cast<NpcBrush*>(this); }

	virtual bool canDraw(BaseMap* map, const Position& position) const;
	virtual void draw(BaseMap* map, Tile* tile, void* parameter);
	virtual void undraw(BaseMap* map, Tile* tile);

	NpcType* getType() const {return npc_type;}

	virtual int getLookID() const; // We don't have a look type, this will always return 0
	virtual std::string getName() const;
	virtual bool canDrag() const { return false; }
	virtual bool canSmear() const { return true; }
	virtual bool oneSizeFitsAll() const { return true; }

protected:
	NpcType* npc_type;
};

#endif
