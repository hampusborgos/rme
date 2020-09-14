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

#ifndef RME_GROUND_BRUSH_H
#define RME_GROUND_BRUSH_H

#include "brush.h"

//=============================================================================

class GroundBrush : public TerrainBrush
{
protected:
	struct BorderBlock;

public:
	static void init();

	GroundBrush();
	virtual ~GroundBrush();

	bool isGround() const { return true; }
	GroundBrush* asGround() { return static_cast<GroundBrush*>(this); }

	virtual bool load(pugi::xml_node node, wxArrayString& warnings);

	virtual void draw(BaseMap* map, Tile* tile, void* parameter);
	virtual void undraw(BaseMap* map, Tile* tile);
	static void doBorders(BaseMap* map, Tile* tile);
	static const BorderBlock* getBrushTo(GroundBrush* first, GroundBrush* second);

	virtual int32_t getZ() const { return z_order; }
	bool useSoloOptionalBorder() const { return use_only_optional; }
	bool isReRandomizable() const { return randomize; }

	bool hasOuterZilchBorder() const { return has_zilch_outer_border || optional_border; }
	bool hasInnerZilchBorder() const { return has_zilch_inner_border; }
	bool hasOuterBorder() const { return has_outer_border || optional_border; }
	bool hasInnerBorder() const { return has_inner_border; }
	bool hasOptionalBorder() const { return optional_border != nullptr; }

protected: // Members
	int32_t z_order;
	bool has_zilch_outer_border;
	bool has_zilch_inner_border;
	bool has_outer_border;
	bool has_inner_border;
	AutoBorder* optional_border;
	bool use_only_optional; // If this is true, there will be no normal border under the gravel
	bool randomize;

	struct SpecificCaseBlock {
		SpecificCaseBlock() : match_group(0), group_match_alignment(BORDER_NONE), to_replace_id(0), with_id(0), delete_all(false) {}
		std::vector<uint16_t> items_to_match;
		uint32_t match_group;
		BorderType group_match_alignment;
		uint16_t to_replace_id;
		uint16_t with_id;
		bool delete_all;
	};

	struct BorderBlock {
		bool outer;
		bool super;
		uint32_t to;

		AutoBorder* autoborder;
		std::vector<SpecificCaseBlock*> specific_cases;
	};

	struct ItemChanceBlock {
		int chance;
		uint16_t id;
	};

	struct BorderCluster {
		uint32_t alignment;
		int32_t z;
		const AutoBorder* border;

		bool operator<(const BorderCluster& other) const {
			return other.z > z;
		}
	};

	std::vector<BorderBlock*> borders;
	std::vector<ItemChanceBlock> border_items;
	int total_chance;

public: // Static global members
	static uint32_t border_types[256];
};

#endif
