//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
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
}

CarpetBrush::~CarpetBrush() {
}

bool CarpetBrush::load(xmlNodePtr node, wxArrayString& warnings) {
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
		if(xmlStrcmp(child->name,(const xmlChar*)"carpet") == 0) {
			uint alignment;
			if(readXMLValue(child, "align", strVal)) {
				alignment = AutoBorder::edgeNameToID(strVal);
				if(alignment == BORDER_NONE) {
					if(strVal == "center") {
						alignment = CARPET_CENTER;
					} else {
						wxString warning;
						warning = wxT("Invalid alignment of carpet node\n");
						warnings.push_back(warning);
						child = child->next;
						continue;
					}
				}
			} else {
				wxString warning;
				warning = wxT("Could not read alignment tag of carpet node\n");
				warnings.push_back(warning);
				child = child->next;
				continue;
			}

			bool use_local_id = true;
			xmlNodePtr subchild = child->children;
			while(subchild) {
				if(xmlStrcmp(subchild->name,(const xmlChar*)"item") == 0) {
					use_local_id = false;

					int id, chance;
					if(!readXMLValue(subchild, "id", id)) {
						wxString warning;
						warning = wxT("Could not read id tag of item node\n");
						warnings.push_back(warning);
						subchild = subchild->next;
						continue;
					}

					if(!readXMLValue(subchild, "chance", chance)) {
						wxString warning;
						warning = wxT("Could not read chance tag of item node\n");
						warnings.push_back(warning);
						subchild = subchild->next;
						continue;
					}

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
					it.isCarpet = true;
					it.brush = this;

					carpet_items[alignment].total_chance += chance;
					CarpetType t;
					t.chance = chance;
					t.id = id;
					carpet_items[alignment].items.push_back(t);
				}
				subchild = subchild->next;
			}

			if(use_local_id) {
				int id;
				if(!readXMLValue(child, "id", id)) {
					wxString warning;
					warning = wxT("Could not read id tag of carpet node\n");
					warnings.push_back(warning);
					child = child->next;
					continue;
				}

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
				it.isCarpet = true;
				it.brush = this;

				carpet_items[alignment].total_chance = 1;
				CarpetType t;
				t.chance = 1;
				t.id = id;
				carpet_items[alignment].items.push_back(t);
			}
		}
		child = child->next;
	}

	return true;
}

void CarpetBrush::setName(std::string newname) {
	name = newname;
}

std::string CarpetBrush::getName() const {
	return name;
}

int CarpetBrush::getLookID() const {
	return look_id;
}

bool CarpetBrush::canDraw(BaseMap* map, Position pos) const {
	return true;
}

void CarpetBrush::undraw(BaseMap* map, Tile* t) {
	ItemVector::iterator it = t->items.begin();
	while(it != t->items.end()) {
		if((*it)->isCarpet()) {
			CarpetBrush* cb = (*it)->getCarpetBrush();
			if(cb == this) {
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

void CarpetBrush::draw(BaseMap* map, Tile* tile, void* parameter) {
	undraw(map, tile); // Remove old

	tile->addItem(Item::Create(getRandomCarpet(CARPET_CENTER)));
}


bool hasMatchingCarpetBrushAtTile(BaseMap* map, CarpetBrush* carpet_brush, uint x, uint y, uint z) {
	Tile* t = map->getTile(x, y, z);
	if(!t) return false;

	ItemVector::const_iterator it = t->items.begin();
	for(; it != t->items.end(); ++it) {
		CarpetBrush* cb = (*it)->getCarpetBrush();
		if(cb == carpet_brush) {
			return true;
		}
	}

	return false;
}

uint16_t CarpetBrush::getRandomCarpet(BorderType alignment) {
	if(carpet_items[alignment].total_chance > 0) {
		int chance = random(1, carpet_items[alignment].total_chance);
		for(std::vector<CarpetType>::const_iterator carpet_iter = carpet_items[alignment].items.begin();
				carpet_iter != carpet_items[alignment].items.end();
				++carpet_iter)
		{
			if(chance <= carpet_iter->chance) {
				return carpet_iter->id;
			}
			chance -= carpet_iter->chance;
		}
	} else {
		if(alignment != CARPET_CENTER && carpet_items[CARPET_CENTER].total_chance > 0) {
			int chance = random(1, carpet_items[CARPET_CENTER].total_chance);
			for(std::vector<CarpetType>::const_iterator carpet_iter = carpet_items[CARPET_CENTER].items.begin();
					carpet_iter != carpet_items[CARPET_CENTER].items.end();
					++carpet_iter)
			{
				if(chance <= carpet_iter->chance) {
					return carpet_iter->id;
				}
				chance -= carpet_iter->chance;
			}
		}
		// Find an item to place on the tile, first center, then the rest.
		for(int i = 0; i < 12; ++i) {
			if(carpet_items[i].total_chance > 0) {
				int chance = random(1, carpet_items[i].total_chance);
				for(std::vector<CarpetType>::const_iterator carpet_iter = carpet_items[i].items.begin();
						carpet_iter != carpet_items[i].items.end();
						++carpet_iter)
				{
					if(chance <= carpet_iter->chance) {
						return carpet_iter->id;
					}
					chance -= carpet_iter->chance;
				}
			}
		}
	}
	return 0;
}

void CarpetBrush::doCarpets(BaseMap* map, Tile* tile) {
	ASSERT(tile);


	if(tile->hasCarpet() == false) {
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
		CarpetBrush* carpet_brush = item->getCarpetBrush();
		if(carpet_brush == NULL) {
			continue;
		}

		bool neighbours[8];

		if(x == 0) {
			if(y == 0) {
				neighbours[0] = false;
				neighbours[1] = false;
				neighbours[2] = false;
				neighbours[3] = false;
				neighbours[4] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x + 1, y, z);
				neighbours[5] = false;
				neighbours[6] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x,     y + 1, z);
				neighbours[7] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x + 1, y + 1, z);
			} else {
				neighbours[0] = false;
				neighbours[1] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x,     y - 1, z);
				neighbours[2] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x + 1, y - 1, z);
				neighbours[3] = false;
				neighbours[4] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x + 1, y, z);
				neighbours[5] = false;
				neighbours[6] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x,     y + 1, z);
				neighbours[7] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x + 1, y + 1, z);
			}
		} else if(y == 0) {
			neighbours[0] = false;
			neighbours[1] = false;
			neighbours[2] = false;
			neighbours[3] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x - 1, y, z);
			neighbours[4] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x + 1, y, z);
			neighbours[5] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x - 1, y + 1, z);
			neighbours[6] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x,     y + 1, z);
			neighbours[7] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x + 1, y + 1, z);
		} else {
			neighbours[0] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x - 1, y - 1, z);
			neighbours[1] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x,     y - 1, z);
			neighbours[2] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x + 1, y - 1, z);
			neighbours[3] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x - 1, y, z);
			neighbours[4] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x + 1, y, z);
			neighbours[5] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x - 1, y + 1, z);
			neighbours[6] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x,     y + 1, z);
			neighbours[7] = hasMatchingCarpetBrushAtTile(map, carpet_brush, x + 1, y + 1, z);
		}

		uint32_t tiledata = 0;
		for(int i = 0; i < 8; i++) {
			if(neighbours[i]) {
				// Same table as this one, calculate what border
				tiledata |= 1 << i;
			}
		}
		::BorderType bt = BorderType(carpet_types[tiledata]);

		// bt is always valid.
		int id = carpet_brush->getRandomCarpet(bt);
		if(id != 0) {
			item->setID(id);
		}
	}
}

