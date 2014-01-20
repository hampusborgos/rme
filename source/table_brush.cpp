//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
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
}

TableBrush::~TableBrush() {
}

bool TableBrush::load(xmlNodePtr node, wxArrayString& warnings) {
	std::string strVal;
	int intVal;

	if(readXMLValue(node, "lookid", intVal)) {
		look_id = intVal;
	}
	if(readXMLValue(node, "server_lookid", intVal)) {
		look_id = item_db[intVal].clientID;
	}

	xmlNodePtr child = node->children;
	while(child) {
		if(xmlStrcmp(child->name,(const xmlChar*)"table") == 0) {
			uint alignment;
			if(readXMLValue(child, "align", strVal)) {
				if(strVal == "vertical") {
					alignment = TABLE_VERTICAL;
				} else if(strVal == "horizontal") {
					alignment = TABLE_HORIZONTAL;
				} else if(strVal == "south") {
					alignment = TABLE_SOUTH_END;
				} else if(strVal == "east") {
					alignment = TABLE_EAST_END;
				} else if(strVal == "north") {
					alignment = TABLE_NORTH_END;
				} else if(strVal == "west") {
					alignment = TABLE_WEST_END;
				} else if(strVal == "alone") {
					alignment = TABLE_ALONE;
				} else {
					wxString warning;
					warning = wxT("Unknown table alignment '") + wxstr(strVal) + wxT("'\n");
					warnings.push_back(warning);
					child = child->next;
					continue;
				}
			} else {
				wxString warning;
				warning = wxT("Could not read type tag of table node\n");
				warnings.push_back(warning);
				child = child->next;
				continue;
			}

			xmlNodePtr subchild = child->children;
			while(subchild) {
				do {
					if(xmlStrcmp(subchild->name,(const xmlChar*)"item") == 0) {
						int id, chance = 0;
						if(!readXMLValue(subchild, "id", id)) {
							wxString warning;
							warning += wxT("Could not read id tag of item node\n");
							warnings.push_back(warning);
							break;
						}
						readXMLValue(subchild, "chance", chance);

						ItemType& it = item_db[id];
						if(it.id == 0) {
							wxString warning;
							warning << wxT("There is no itemtype with id ") << id;
							warnings.push_back(warning);
							return false;
						}
						if(it.brush != NULL && it.brush != this) {
							wxString warning;
							warning << wxT("Itemtype id ") << id << wxT(" already has a brush");
							warnings.push_back(warning);
							return false;
						}
						it.isTable = true;
						it.brush = this;

						TableType tt;
						table_items[alignment].total_chance += chance;
						tt.chance = chance;

						tt.item_id  = uint16_t(id);
						table_items[alignment].items.push_back(tt);
					}
				} while(false);
				subchild = subchild->next;
			}
		}
		child = child->next;
	}

	return true;
}

void TableBrush::setName(std::string newname) {
	name = newname;
}

std::string TableBrush::getName() const {
	return name;
}

int TableBrush::getLookID() const {
	return look_id;
}

bool TableBrush::canDraw(BaseMap* map, Position pos) const {
	return true;
}

void TableBrush::undraw(BaseMap* map, Tile* t) {
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

void TableBrush::draw(BaseMap* map, Tile* tile, void* parameter) {
	undraw(map, tile); // Remove old

	TableNode& tn = table_items[0];
	if(tn.total_chance <= 0) {
		return;
	}
	int chance = random(1, tn.total_chance);
	uint16_t type = 0;

	for(std::vector<TableType>::const_iterator table_iter = tn.items.begin();
			table_iter != tn.items.end();
			++table_iter)
	{
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


bool hasMatchingTableBrushAtTile(BaseMap* map, TableBrush* table_brush, uint x, uint y, uint z) {
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

void TableBrush::doTables(BaseMap* map, Tile* tile) {
	ASSERT(tile);

	if(tile->hasTable() == false) {
		return;
	}

	int x = tile->getPosition().x;
	int y = tile->getPosition().y;
	int z = tile->getPosition().z;

	for(ItemVector::const_iterator item_iter = tile->items.begin();
			item_iter != tile->items.end();
			++item_iter)
	{
		Item* item = *item_iter;
		ASSERT(item);
		TableBrush* table_brush = item->getTableBrush();
		if(table_brush == NULL) {
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
		for(int i = 0; i < 8; i++) {
			if(neighbours[i]) {
				// Same table as this one, calculate what border
				tiledata |= 1 << i;
			}
		}
		::BorderType bt = BorderType(table_types[tiledata]);

		// bt is always valid.

		TableNode& tn = table_brush->table_items[int(bt)];
		if(tn.total_chance == 0) {
			return;
		}
		int chance = random(1, tn.total_chance);
		uint16_t id = 0;
		for(std::vector<TableType>::const_iterator node_iter = tn.items.begin();
				node_iter != tn.items.end();
				++node_iter)
		{
			if(chance <= node_iter->chance) {
				id = node_iter->item_id;
				break;
			}
			chance -= node_iter->chance;
		}
		if(id != 0) {
			item->setID(id);
		}
	}
}
