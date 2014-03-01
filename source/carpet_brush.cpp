//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "carpet_brush.h"

#include "basemap.h"
#include "items.h"
#include "pugicast.h"

//=============================================================================
// Carpet brush

uint32_t CarpetBrush::carpet_types[256];

CarpetBrush::CarpetBrush() :
	look_id(0)
{
}

CarpetBrush::~CarpetBrush() {
}

bool CarpetBrush::load(pugi::xml_node node, wxArrayString& warnings)
{
	pugi::xml_attribute attribute;
	if ((attribute = node.attribute("lookid"))) {
		look_id = pugi::cast<uint16_t>(attribute.value());
	}

	if ((attribute = node.attribute("server_lookid"))) {
		look_id = item_db[pugi::cast<uint16_t>(attribute.value())].clientID;
	}

	for (pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
		if (as_lower_str(childNode.name()) != "carpet") {
			continue;
		}

		uint32_t alignment;
		if ((attribute = childNode.attribute("align"))) {
			const std::string& alignString = attribute.as_string();
			alignment = AutoBorder::edgeNameToID(alignString);
			if (alignment == BORDER_NONE) {
				if (alignString == "center") {
					alignment = CARPET_CENTER;
				} else {
					warnings.push_back(wxT("Invalid alignment of carpet node\n"));
					continue;
				}
			}
		} else {
			warnings.push_back(wxT("Could not read alignment tag of carpet node\n"));
			continue;
		}

		bool use_local_id = true;
		for (pugi::xml_node subChildNode = childNode.first_child(); subChildNode; subChildNode = subChildNode.next_sibling()) {
			if (as_lower_str(subChildNode.name()) != "item") {
				continue;
			}

			use_local_id = false;
			if (!(attribute = subChildNode.attribute("id"))) {
				warnings.push_back(wxT("Could not read id tag of item node\n"));
				continue;
			}

			int32_t id = pugi::cast<int32_t>(attribute.value());
			if (!(attribute = subChildNode.attribute("chance"))) {
				warnings.push_back(wxT("Could not read chance tag of item node\n"));
				continue;
			}

			int32_t chance = pugi::cast<int32_t>(attribute.value());

			ItemType& it = item_db[id];
			if (it.id == 0) {
				warnings.push_back(wxT("There is no itemtype with id ") + std::to_string(id));
				continue;
			} else if (it.brush && it.brush != this) {
				warnings.push_back(wxT("Itemtype id ") + std::to_string(id) + wxT(" already has a brush"));
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

		if (use_local_id) {
			if (!(attribute = childNode.attribute("id"))) {
				warnings.push_back(wxT("Could not read id tag of carpet node\n"));
				continue;
			}

			int32_t id = pugi::cast<int32_t>(attribute.value());

			ItemType& it = item_db[id];
			if (it.id == 0) {
				warnings.push_back(wxT("There is no itemtype with id ") + std::to_string(id));
				return false;
			} else if (it.brush && it.brush != this) {
				warnings.push_back(wxT("Itemtype id ") + std::to_string(id) + wxT(" already has a brush"));
				return false;
			}

			it.isCarpet = true;
			it.brush = this;

			auto& alignItem = carpet_items[alignment];
			alignItem.total_chance = 1;

			CarpetType t;
			t.id = id;
			t.chance = 1;

			alignItem.items.push_back(t);
		}
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

bool CarpetBrush::canDraw(BaseMap* map, const Position& position) const {
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


bool hasMatchingCarpetBrushAtTile(BaseMap* map, CarpetBrush* carpet_brush, uint32_t x, uint32_t y, uint32_t z) {
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
		if(carpet_brush == nullptr) {
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

