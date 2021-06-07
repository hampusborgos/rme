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

#include "table_brush.h"

#include "items.h"
#include "basemap.h"

uint32_t TableBrush::table_types[256];

//=============================================================================
// Table brush

TableBrush::TableBrush() :
	look_id(0)
{
	////
}

TableBrush::~TableBrush()
{
	////
}

bool TableBrush::load(pugi::xml_node node, wxArrayString& warnings)
{
	if(const pugi::xml_attribute attribute = node.attribute("server_lookid"))
		look_id = g_items[attribute.as_ushort()].clientID;

	if(look_id == 0) {
		look_id = node.attribute("lookid").as_ushort();
	}

	for(pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
		if(as_lower_str(childNode.name()) != "table") {
			continue;
		}

		const std::string& alignString = childNode.attribute("align").as_string();
		if(alignString.empty()) {
			warnings.push_back("Could not read type tag of table node\n");
			continue;
		}

		uint32_t alignment;
		if(alignString == "vertical") {
			alignment = TABLE_VERTICAL;
		} else if(alignString == "horizontal") {
			alignment = TABLE_HORIZONTAL;
		} else if(alignString == "south") {
			alignment = TABLE_SOUTH_END;
		} else if(alignString == "east") {
			alignment = TABLE_EAST_END;
		} else if(alignString == "north") {
			alignment = TABLE_NORTH_END;
		} else if(alignString == "west") {
			alignment = TABLE_WEST_END;
		} else if(alignString == "alone") {
			alignment = TABLE_ALONE;
		} else {
			warnings.push_back("Unknown table alignment '" + wxstr(alignString) + "'\n");
			continue;
		}

		for(pugi::xml_node subChildNode = childNode.first_child(); subChildNode; subChildNode = subChildNode.next_sibling()) {
			if(as_lower_str(subChildNode.name()) != "item") {
				continue;
			}

			uint16_t id = subChildNode.attribute("id").as_ushort();
			if(id == 0) {
				warnings.push_back("Could not read id tag of item node\n");
				break;
			}

			ItemType& it = g_items[id];
			if(it.id == 0) {
				warnings.push_back("There is no itemtype with id " + std::to_string(id));
				return false;
			} else if(it.brush && it.brush != this) {
				warnings.push_back("Itemtype id " + std::to_string(id) + " already has a brush");
				return false;
			}

			it.isTable = true;
			it.brush = this;

			TableType tt;
			tt.item_id = id;
			tt.chance = subChildNode.attribute("chance").as_int();

			table_items[alignment].total_chance += tt.chance;
			table_items[alignment].items.push_back(tt);
		}
	}
	return true;
}

bool TableBrush::canDraw(BaseMap* map, const Position& position) const
{
	return true;
}

void TableBrush::undraw(BaseMap* map, Tile* t)
{
	ItemVector::iterator it = t->items.begin();
	while(it != t->items.end()) {
		if((*it)->isTable()) {
			TableBrush* tb = (*it)->getTableBrush();
			if(tb == this) {
				delete *it;
				it = t->items.erase(it);
			} else {
				++it;
			}
		} else {
			++it;
		}
	}
}

void TableBrush::draw(BaseMap* map, Tile* tile, void* parameter)
{
	undraw(map, tile); // Remove old

	TableNode& tn = table_items[0];
	if(tn.total_chance <= 0) {
		return;
	}
	int chance = random(1, tn.total_chance);
	uint16_t type = 0;

	for(std::vector<TableType>::const_iterator table_iter = tn.items.begin(); table_iter != tn.items.end(); ++table_iter) {
		if(chance <= table_iter->chance) {
			type = table_iter->item_id;
			break;
		}
		chance -= table_iter->chance;
	}

	if(type != 0) {
		tile->addItem(Item::Create(type));
	}
}


bool hasMatchingTableBrushAtTile(BaseMap* map, TableBrush* table_brush, uint32_t x, uint32_t y, uint32_t z)
{
	Tile* t = map->getTile(x, y, z);
	if(!t) return false;

	ItemVector::const_iterator it = t->items.begin();
	for(; it != t->items.end(); ++it) {
		TableBrush* tb = (*it)->getTableBrush();
		if(tb == table_brush) {
			return true;
		}
	}

	return false;
}

void TableBrush::doTables(BaseMap* map, Tile* tile)
{
	ASSERT(tile);
	if(!tile->hasTable()) {
		return;
	}

	const Position& position = tile->getPosition();

	int32_t x = position.x;
	int32_t y = position.y;
	int32_t z = position.z;

	for(Item* item : tile->items) {
		ASSERT(item);

		TableBrush* table_brush = item->getTableBrush();
		if(!table_brush) {
			continue;
		}

		bool neighbours[8];
		if(x == 0) {
			if(y == 0) {
				neighbours[0] = false;
				neighbours[1] = false;
				neighbours[2] = false;
				neighbours[3] = false;
				neighbours[4] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y, z);
				neighbours[5] = false;
				neighbours[6] = hasMatchingTableBrushAtTile(map, table_brush, x,     y + 1, z);
				neighbours[7] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y + 1, z);
			} else {
				neighbours[0] = false;
				neighbours[1] = hasMatchingTableBrushAtTile(map, table_brush, x,     y - 1, z);
				neighbours[2] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y - 1, z);
				neighbours[3] = false;
				neighbours[4] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y, z);
				neighbours[5] = false;
				neighbours[6] = hasMatchingTableBrushAtTile(map, table_brush, x,     y + 1, z);
				neighbours[7] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y + 1, z);
			}
		} else if(y == 0) {
			neighbours[0] = false;
			neighbours[1] = false;
			neighbours[2] = false;
			neighbours[3] = hasMatchingTableBrushAtTile(map, table_brush, x - 1, y, z);
			neighbours[4] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y, z);
			neighbours[5] = hasMatchingTableBrushAtTile(map, table_brush, x - 1, y + 1, z);
			neighbours[6] = hasMatchingTableBrushAtTile(map, table_brush, x,     y + 1, z);
			neighbours[7] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y + 1, z);
		} else {
			neighbours[0] = hasMatchingTableBrushAtTile(map, table_brush, x - 1, y - 1, z);
			neighbours[1] = hasMatchingTableBrushAtTile(map, table_brush, x,     y - 1, z);
			neighbours[2] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y - 1, z);
			neighbours[3] = hasMatchingTableBrushAtTile(map, table_brush, x - 1, y, z);
			neighbours[4] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y, z);
			neighbours[5] = hasMatchingTableBrushAtTile(map, table_brush, x - 1, y + 1, z);
			neighbours[6] = hasMatchingTableBrushAtTile(map, table_brush, x,     y + 1, z);
			neighbours[7] = hasMatchingTableBrushAtTile(map, table_brush, x + 1, y + 1, z);
		}

		uint32_t tiledata = 0;
		for(int32_t i = 0; i < 8; ++i) {
			if(neighbours[i]) {
				// Same table as this one, calculate what border
				tiledata |= 1 << i;
			}
		}

		BorderType bt = static_cast<BorderType>(table_types[tiledata]);
		TableNode& tn = table_brush->table_items[static_cast<int32_t>(bt)];
		if(tn.total_chance == 0) {
			return;
		}

		int32_t chance = random(1, tn.total_chance);
		uint16_t id = 0;

		for(const TableType& tableType : tn.items) {
			if(chance <= tableType.chance) {
				id = tableType.item_id;
				break;
			}
			chance -= tableType.chance;
		}

		if(id != 0) {
			item->setID(id);
		}
	}
}
