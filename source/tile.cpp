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

#include "brush.h"

#include "tile.h"
#include "creature.h"
#include "house.h"
#include "basemap.h"
#include "spawn.h"
#include "ground_brush.h"
#include "wall_brush.h"
#include "carpet_brush.h"
#include "table_brush.h"

Tile::Tile(int x, int y, int z) :
	location(nullptr),
	ground(nullptr),
	creature(nullptr),
	spawn(nullptr),
	house_id(0),
	mapflags(0),
	statflags(0),
	minimapColor(INVALID_MINIMAP_COLOR)
{
	////
}

Tile::Tile(TileLocation& loc) :
	location(&loc),
	ground(nullptr),
	creature(nullptr),
	spawn(nullptr),
	house_id(0),
	mapflags(0),
	statflags(0),
	minimapColor(INVALID_MINIMAP_COLOR)
{
	////
}

Tile::~Tile()
{
	while(!items.empty()) {
		delete items.back();
		items.pop_back();
	}
	delete creature;
	delete ground;
	delete spawn;
}

Tile* Tile::deepCopy(BaseMap& map) const
{
	Tile* copy = map.allocator.allocateTile(location);
	copy->flags = flags;
	copy->house_id = house_id;
	if(spawn) copy->spawn = spawn->deepCopy();
	if(creature) copy->creature = creature->deepCopy();
	// Spawncount & exits are not transferred on copy!
	if(ground) copy->ground = ground->deepCopy();

	for(const Item* item : items) {
		copy->items.push_back(item->deepCopy());
	}
	return copy;
}

uint32_t Tile::memsize() const
{
	uint32_t mem = sizeof(*this);
	if(ground) mem += ground->memsize();

	for(const Item* item : items) {
		mem += item->memsize();
	}

	mem += sizeof(Item*) * items.capacity();

	return mem;
}

int Tile::size() const
{
	int sz = 0;
	if(ground) ++sz;
	sz += items.size();
	if(creature) ++sz;
	if(spawn) ++sz;
	if(location) {
		if(location->getHouseExits()) ++sz;
		if(location->getSpawnCount()) ++sz;
		if(location->getWaypointCount()) ++ sz;
	}
	return sz;
}

void Tile::merge(Tile* other)
{
	if(!other) return;

	if(other->isPZ()) setPZ(true);
	if(other->house_id) {
		house_id = other->house_id;
	}

	if(other->ground) {
		delete ground;
		ground = other->ground;
		other->ground = nullptr;
	}

	if(other->creature) {
		delete creature;
		creature = other->creature;
		other->creature = nullptr;
	}

	if(other->spawn) {
		delete spawn;
		spawn = other->spawn;
		other->spawn = nullptr;
	}

	if(other->creature) {
		delete creature;
		creature = other->creature;
		other->creature = nullptr;
	}

	for(Item* item : other->items) {
		addItem(item);
	}
	other->items.clear();
}

bool Tile::hasProperty(enum ITEMPROPERTY prop) const
{
	if(prop == PROTECTIONZONE && isPZ())
		return true;

	if(ground && ground->hasProperty(prop)){
		return true;
	}

	for(const Item* item : items) {
		if(item->hasProperty(prop))
			return true;
	}

	return false;
}

int Tile::getIndexOf(Item* item) const
{
	if(!item)
		return wxNOT_FOUND;

	int index = 0;
	if(ground) {
		if(ground == item)
			return index;
		index++;
	}

	if(!items.empty()) {
		auto it = std::find(items.begin(), items.end(), item);
		if(it != items.end()) {
			index += (it - items.begin());
			return index;
		}
	}
	return wxNOT_FOUND;
}

Item* Tile::getTopItem() const
{
	if(!items.empty() && !items.back()->isMetaItem())
		return items.back();
	if(ground && !ground->isMetaItem()) {
		return ground;
	}
	return nullptr;
}

Item* Tile::getItemAt(int index) const
{
	if(index < 0)
		return nullptr;
	if(ground) {
		if(index == 0)
			return ground;
		index--;
	}
	if(!items.empty() && index >= 0 && index < items.size()) {
		return items.at(index);
	}
	return nullptr;
}

void Tile::addItem(Item* item)
{
	if(!item) return;

	if(item->isGroundTile()) {
		delete ground;
		ground = item;
		return;
	}

	ItemVector::iterator it;

	uint16_t gid = item->getGroundEquivalent();
	if(gid != 0) {
		delete ground;
		ground = Item::Create(gid);
		// At the very bottom!
		it = items.begin();
	} else {
		if(item->isAlwaysOnBottom()) {
			it = items.begin();
			while(true) {
				if(it == items.end()) {
					break;
				} else if((*it)->isAlwaysOnBottom()) {
					if(item->getTopOrder() < (*it)->getTopOrder()) {
						break;
					}
				} else { // Always on top
					break;
				}
				++it;
			}
		} else {
			it = items.end();
		}
	}

	items.insert(it, item);

	if(item->isSelected()) {
		statflags |= TILESTATE_SELECTED;
	}
}

void Tile::select()
{
	if(size() == 0) return;
	if(ground) ground->select();
	if(spawn) spawn->select();
	if(creature) creature->select();

	for(Item* item : items) {
		item->select();
	}

	statflags |= TILESTATE_SELECTED;
}

void Tile::deselect()
{
	if(ground) ground->deselect();
	if(spawn) spawn->deselect();
	if(creature) creature->deselect();

	for(Item* item : items) {
		item->deselect();
	}

	statflags &= ~TILESTATE_SELECTED;
}

Item* Tile::getTopSelectedItem()
{
	for(auto it = items.rbegin(); it != items.rend(); ++it) {
		if((*it)->isSelected() && !(*it)->isMetaItem()) {
			return *it;
		}
	}
	if(ground && ground->isSelected() && !ground->isMetaItem()) {
		return ground;
	}
	return nullptr;
}

ItemVector Tile::popSelectedItems(bool ignoreTileSelected)
{
	ItemVector pop_items;

	if(!ignoreTileSelected && !isSelected()) return pop_items;

	if(ground && ground->isSelected()) {
		pop_items.push_back(ground);
		ground = nullptr;
	}

	for(auto it = items.begin(); it != items.end();) {
		Item* item = (*it);
		if(item->isSelected()) {
			pop_items.push_back(item);
			it = items.erase(it);
		}
		else ++it;
	}

	statflags &= ~TILESTATE_SELECTED;
	return pop_items;
}

ItemVector Tile::getSelectedItems()
{
	ItemVector selected_items;

	if(!isSelected()) return selected_items;

	if(ground && ground->isSelected()) {
		selected_items.push_back(ground);
	}

	for(Item* item : items) {
		if(item->isSelected()) {
			selected_items.push_back(item);
		}
	}

	return selected_items;
}

uint8_t Tile::getMiniMapColor() const
{
	if(minimapColor != INVALID_MINIMAP_COLOR)
		return minimapColor;

	for(auto it = items.rbegin(); it != items.rend(); ++it) {
		uint8_t color = (*it)->getMiniMapColor();
		if(color != 0) {
			return color;
		}
	}

	// check ground too
	if(hasGround()) {
		return ground->getMiniMapColor();
	}

	return 0;
}

void Tile::update()
{
	statflags &= TILESTATE_MODIFIED;

	if(spawn && spawn->isSelected()) {
		statflags |= TILESTATE_SELECTED;
	}
	if(creature && creature->isSelected()) {
		statflags |= TILESTATE_SELECTED;
	}

	if(ground) {
		if(ground->isSelected()) {
			statflags |= TILESTATE_SELECTED;
		}
		if(ground->isBlocking()) {
			statflags |= TILESTATE_BLOCKING;
		}
		if(ground->getUniqueID() != 0) {
			statflags |= TILESTATE_UNIQUE;
		}
		if(ground->getMiniMapColor() != 0) {
			minimapColor = ground->getMiniMapColor();
		}
	}

	for(const Item* item : items) {
		if(item->isSelected()) {
			statflags |= TILESTATE_SELECTED;
		}
		if(item->getUniqueID() != 0) {
			statflags |= TILESTATE_UNIQUE;
		}
		if(item->getMiniMapColor() != 0) {
			minimapColor = item->getMiniMapColor();
		}

		const ItemType& type = g_items.getItemType(item->getID());
		if(type.unpassable) {
			statflags |= TILESTATE_BLOCKING;
		}
		if(type.isOptionalBorder) {
			statflags |= TILESTATE_OP_BORDER;
		}
		if(type.isTable) {
			statflags |= TILESTATE_HAS_TABLE;
		}
		if(type.isCarpet) {
			statflags |= TILESTATE_HAS_CARPET;
		}
	}

	if((statflags & TILESTATE_BLOCKING) == 0) {
		if(!ground && items.empty()) {
			statflags |= TILESTATE_BLOCKING;
		}
	}
}

void Tile::borderize(BaseMap* parent)
{
	GroundBrush::doBorders(parent, this);
}

void Tile::addBorderItem(Item* item)
{
	if(!item) return;
	ASSERT(item->isBorder());
	items.insert(items.begin(), item);
}

GroundBrush* Tile::getGroundBrush() const
{
	if(ground && ground->getGroundBrush()) {
		return ground->getGroundBrush();
	}
	return nullptr;
}

void Tile::cleanBorders()
{
	if(items.empty()) return;

	for(auto it = items.begin(); it != items.end();) {
		Item* item = (*it);
		// Borders should only be on the bottom, we can ignore the rest of the items
		if(!item->isBorder()) break; 

		delete item;
		it = items.erase(it);
	}
}

void Tile::wallize(BaseMap* parent)
{
	WallBrush::doWalls(parent, this);
}

Item* Tile::getWall() const
{
	for(Item* item : items) {
		if(item->isWall()) {
			return item;
		}
	}
	return nullptr;
}

Item* Tile::getCarpet() const
{
	for(Item* item : items) {
		if(item->isCarpet()) {
			return item;
		}
	}
	return nullptr;
}

Item* Tile::getTable() const
{
	for(Item* item : items) {
		if(item->isTable()) {
			return item;
		}
	}
	return nullptr;
}

void Tile::addWallItem(Item* item)
{
	if(!item) return;
	ASSERT(item->isWall());
	addItem(item);
}

void Tile::cleanWalls(bool dontdelete)
{
	if(items.empty()) return;

	for(auto it = items.begin(); it != items.end();) {
		Item* item = (*it);
		if(item && item->isWall()) {
			if(!dontdelete) {
				delete item;
			}
			it = items.erase(it);
		}
		else ++it;
	}
}

void Tile::cleanWalls(WallBrush* brush)
{
	if(!brush || items.empty())
		return;

	for(auto it = items.begin(); it != items.end();) {
		Item* item = (*it);
		if(item && item->isWall() && brush->hasWall(item)) {
			delete item;
			it = items.erase(it);
		}
		else ++it;
	}
}

void Tile::cleanTables(bool dontdelete)
{
	if(items.empty()) return;

	for(auto it = items.begin(); it != items.end();) {
		Item* item = (*it);
		if(item && item->isTable()) {
			if(!dontdelete) {
				delete item;
			}
			it = items.erase(it);
		}
		else ++it;
	}
}

void Tile::tableize(BaseMap* parent)
{
	TableBrush::doTables(parent, this);
}

void Tile::carpetize(BaseMap* parent)
{
	CarpetBrush::doCarpets(parent, this);
}

void Tile::selectGround()
{
	bool selected = false;
	if(ground) {
		ground->select();
		selected = true;
	}

	for(Item* item : items) {
		if(!item->isBorder())
			break;
		item->select();
		selected = true;
	}

	if(selected) statflags |= TILESTATE_SELECTED;
}


void Tile::deselectGround()
{
	if(ground) {
		ground->deselect();
	}
	for(Item* item : items) {
		if(!item->isBorder())
			break;
		item->deselect();
	}
}

void Tile::setHouse(House* house)
{
	house_id = (house ? house->id : 0);
}

void Tile::addHouseExit(House* house)
{
	if(!house) return;

	HouseExitList* exits = location->createHouseExits();
	exits->push_back(house->id);
}

void Tile::removeHouseExit(House* house)
{
	if(!house) return;

	HouseExitList* exits = location->getHouseExits();
	if(!exits || exits->empty()) return;

	auto it = std::find(exits->begin(), exits->end(), house->id);
	if(it != exits->end())
		exits->erase(it);
}

bool Tile::hasHouseExit(uint32_t houseId) const
{
	const HouseExitList* exits = getHouseExits();
	if(!exits || exits->empty())
		return false;

	auto it = std::find(exits->begin(), exits->end(), houseId);
	return it != exits->end();
}
