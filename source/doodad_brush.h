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

#ifndef RME_DOODAD_BRUSH_H
#define RME_DOODAD_BRUSH_H

#include "brush.h"

//=============================================================================
// Doodadbrush, add doodads!

typedef std::vector<std::pair<Position, ItemVector> > CompositeTileList;

class DoodadBrush : public Brush
{
public:
	DoodadBrush();
	virtual ~DoodadBrush();

	bool isDoodad() const { return true; }
	DoodadBrush* asDoodad() { return static_cast<DoodadBrush*>(this); }

protected:
	struct AlternativeBlock;

public:
	bool loadAlternative(pugi::xml_node node, wxArrayString& warnings, AlternativeBlock* which = nullptr);
	virtual bool load(pugi::xml_node node, wxArrayString& warnings);

	virtual bool canDraw(BaseMap* map, const Position& position) const { return true; }
	virtual void draw(BaseMap* map, Tile* tile, void* parameter);
	const CompositeTileList& getComposite(int variation) const;
	virtual void undraw(BaseMap* map, Tile* tile);

	bool isEmpty(int variation) const;

	int getThickness() const {return thickness;}
	int getThicknessCeiling() const {return thickness_ceiling;}

	int getCompositeChance(int variation) const;
	int getSingleChance(int variation) const;
	int getTotalChance(int variation) const;

	bool hasSingleObjects(int variation) const;
	bool hasCompositeObjects(int variation) const;

	bool placeOnBlocking() const { return on_blocking; }
	bool placeOnDuplicate() const { return on_duplicate; }
	bool doNewBorders() const { return do_new_borders; }
	bool ownsItem(Item* item) const;

	virtual bool canSmear() const { return draggable; }
	virtual bool canDrag() const { return false; }
	virtual bool oneSizeFitsAll() const { return one_size; }
	virtual int getLookID() const { return look_id; }
	virtual int getMaxVariation() const { return alternatives.size(); }
	virtual std::string getName() const { return name; }
	virtual void setName(const std::string& newName) { name = newName; }

protected:
	std::string name;
	uint16_t look_id;

	int thickness;
	int thickness_ceiling;

	bool draggable;
	bool on_blocking;
	bool one_size;
	bool do_new_borders;
	bool on_duplicate;
	uint16_t clear_mapflags;
	uint16_t clear_statflags;

	struct SingleBlock {
		int chance;
		Item* item;
	};

	struct CompositeBlock {
		int chance;
		CompositeTileList items;
	};

	struct AlternativeBlock {
		AlternativeBlock();
		~AlternativeBlock();
		bool ownsItem(uint16_t id) const;
		std::vector<SingleBlock> single_items;
		std::vector<CompositeBlock> composite_items;

		int composite_chance; // Total chance of a composite
		int single_chance; // Total chance of a single object
	};

	std::vector<AlternativeBlock*> alternatives;
};

#endif
