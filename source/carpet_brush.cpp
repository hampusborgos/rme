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

#include "main.h"

#include "carpet_brush.h"

#include "basemap.h"
#include "items.h"

//=============================================================================
// Carpet brush

uint32_t CarpetBrush::carpet_types[256];

CarpetBrush::CarpetBrush() :
	look_id(0)
{
	////
}

CarpetBrush::~CarpetBrush()
{
	////
}

bool CarpetBrush::load(pugi::xml_node node, wxArrayString& warnings)
{
	pugi::xml_attribute attribute;
	if((attribute = node.attribute("lookid"))) {
		look_id = attribute.as_ushort();
	}

	if((attribute = node.attribute("server_lookid"))) {
		look_id = g_items[attribute.as_ushort()].clientID;
	}

	for(pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
		if(as_lower_str(childNode.name()) != "carpet") {
			continue;
		}

		uint32_t alignment;
		if((attribute = childNode.attribute("align"))) {
			const std::string& alignString = attribute.as_string();
			alignment = AutoBorder::edgeNameToID(alignString);
			if(alignment == BORDER_NONE) {
				if(alignString == "center") {
					alignment = CARPET_CENTER;
				} else {
					warnings.push_back("Invalid alignment of carpet node\n");
					continue;
				}
			}
		} else {
			warnings.push_back("Could not read alignment tag of carpet node\n");
			continue;
		}

		bool use_local_id = true;
		for(pugi::xml_node subChildNode = childNode.first_child(); subChildNode; subChildNode = subChildNode.next_sibling()) {
			if(as_lower_str(subChildNode.name()) != "item") {
				continue;
			}

			use_local_id = false;
			if(!(attribute = subChildNode.attribute("id"))) {
				warnings.push_back("Could not read id tag of item node\n");
				continue;
			}

			int32_t id = attribute.as_int();
			if(!(attribute = subChildNode.attribute("chance"))) {
				warnings.push_back("Could not read chance tag of item node\n");
				continue;
			}

			int32_t chance = attribute.as_int();

			ItemType& it = g_items[id];
			if(it.id == 0) {
				warnings.push_back("There is no itemtype with id " + std::to_string(id));
				continue;
			} else if(it.brush && it.brush != this) {
				warnings.push_back("Itemtype id " + std::to_string(id) + " already has a brush");
				continue;
			}

			it.isCarpet = true;
			it.brush = this;

			auto& alignItem = carpet_items[alignment];
			alignItem.total_chance += chance;

			CarpetType t;
			t.id = id;
			t.chance = chance;

			alignItem.items.push_back(t);
		}

		if(use_local_id) {
			if(!(attribute = childNode.attribute("id"))) {
				warnings.push_back("Could not read id tag of carpet node\n");
				continue;
			}

			uint16_t id = attribute.as_ushort();

			ItemType& it = g_items[id];
			if(it.id == 0) {
				warnings.push_back("There is no itemtype with id " + std::to_string(id));
				return false;
			} else if(it.brush && it.brush != this) {
				warnings.push_back("Itemtype id " + std::to_string(id) + " already has a brush");
				return false;
			}

			it.isCarpet = true;
			it.brush = this;

			auto& alignItem = carpet_items[alignment];
			alignItem.total_chance = 1;

			CarpetType carpetType;
			carpetType.id = id;
			carpetType.chance = 1;

			alignItem.items.push_back(carpetType);
		}
	}
	return true;
}

bool CarpetBrush::canDraw(BaseMap* map, const Position& position) const
{
	return true;
}

void CarpetBrush::draw(BaseMap* map, Tile* tile, void* parameter)
{
	undraw(map, tile); // Remove old
	tile->addItem(Item::Create(getRandomCarpet(CARPET_CENTER)));
}

void CarpetBrush::undraw(BaseMap* map, Tile* tile)
{
	auto& items = tile->items;
	for(auto it = items.begin(); it != items.end(); ) {
		Item* item = *it;
		if(item->isCarpet()) {
			CarpetBrush* carpetBrush = item->getCarpetBrush();
			if(carpetBrush) {
				delete item;
				it = items.erase(it);
			} else {
				++it;
			}
		} else {
			++it;
		}
	}
}

void CarpetBrush::doCarpets(BaseMap* map, Tile* tile)
{
	static const auto hasMatchingCarpetBrushAtTile = [](BaseMap* map, CarpetBrush* carpetBrush, uint32_t x, uint32_t y, uint32_t z) -> bool {
		Tile* tile = map->getTile(x, y, z);
		if(!tile) {
			return false;
		}

		for(Item* item : tile->items) {
			if(item->getCarpetBrush() == carpetBrush) {
				return true;
			}
		}
		return false;
	};

	ASSERT(tile);
	if(!tile->hasCarpet()) {
		return;
	}

	const Position& position = tile->getPosition();
	uint32_t x = position.x;
	uint32_t y = position.y;
	uint32_t z = position.z;
	/*
	static const std::pair<int32_t, int32_t> positionOffset[8] = {
		{-1, -1}, {0, -1}, {1, -1}, {-1, 0}, {1, 0}, {-1, 1}, {0, 1}, {1, 1}
	};

	const auto& offset = positionOffset[i];
	if(neighbours[i] && hasMatchingCarpetBrushAtTile(map, carpetBrush, x + offset.first, y + offset.second, z)) {
		//
	}
	*/
	for(Item* item : tile->items) {
		ASSERT(item);

		CarpetBrush* carpetBrush = item->getCarpetBrush();
		if(!carpetBrush) {
			continue;
		}

		bool neighbours[8] = { false };
		if(x == 0) {
			if(y == 0) {
				neighbours[0] = false;
				neighbours[1] = false;
				neighbours[2] = false;
				neighbours[3] = false;
				neighbours[4] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y,     z);
				neighbours[5] = false;
				neighbours[6] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x,     y + 1, z);
				neighbours[7] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y + 1, z);
			} else {
				neighbours[0] = false;
				neighbours[1] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x,     y - 1, z);
				neighbours[2] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y - 1, z);
				neighbours[3] = false;
				neighbours[4] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y,     z);
				neighbours[5] = false;
				neighbours[6] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x,     y + 1, z);
				neighbours[7] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y + 1, z);
			}
		} else if(y == 0) {
			neighbours[0] = false;
			neighbours[1] = false;
			neighbours[2] = false;
			neighbours[3] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x - 1, y,     z);
			neighbours[4] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y,     z);
			neighbours[5] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x - 1, y + 1, z);
			neighbours[6] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x,     y + 1, z);
			neighbours[7] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y + 1, z);
		} else {
			neighbours[0] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x - 1, y - 1, z);
			neighbours[1] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x,     y - 1, z);
			neighbours[2] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y - 1, z);
			neighbours[3] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x - 1, y,     z);
			neighbours[4] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y,     z);
			neighbours[5] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x - 1, y + 1, z);
			neighbours[6] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x,     y + 1, z);
			neighbours[7] = hasMatchingCarpetBrushAtTile(map, carpetBrush, x + 1, y + 1, z);
		}

		uint32_t tileData = 0;
		for(uint32_t i = 0; i < 8; ++i) {
			if(neighbours[i]) {
				// Same table as this one, calculate what border
				tileData |= static_cast<uint32_t>(1) << i;
			}
		}

		// border type is always valid.
		uint16_t id = carpetBrush->getRandomCarpet(static_cast<BorderType>(carpet_types[tileData]));
		if(id != 0) {
			item->setID(id);
		}
	}
}

uint16_t CarpetBrush::getRandomCarpet(BorderType alignment)
{
	static const auto findRandomCarpet = [](const CarpetNode& node) -> uint16_t {
		int32_t chance = random(1, node.total_chance);
		for(const CarpetType& carpetType : node.items) {
			if(chance <= carpetType.chance) {
				return carpetType.id;
			}
			chance -= carpetType.chance;
		}
		return 0;
	};

	CarpetNode node = carpet_items[alignment];
	if(node.total_chance > 0) {
		return findRandomCarpet(node);
	}

	node = carpet_items[CARPET_CENTER];
	if(alignment != CARPET_CENTER && node.total_chance > 0) {
		uint16_t id = findRandomCarpet(node);
		if(id != 0) {
			return id;
		}
	}

	// Find an item to place on the tile, first center, then the rest.
	for(int32_t i = 0; i < 12; ++i) {
		node = carpet_items[i];
		if(node.total_chance > 0) {
			uint16_t id = findRandomCarpet(node);
			if(id != 0) {
				return id;
			}
		}
	}
	return 0;
}
