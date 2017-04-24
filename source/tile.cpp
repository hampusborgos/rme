//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/tile.hpp $
// $Id: tile.hpp 298 2010-02-23 17:09:13Z admin $

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
	statflags(0)
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
	statflags(0)
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
	//printf("%d,%d,%d,%p\n", tilePos.x, tilePos.y, tilePos.z, ground);
	delete ground;
	delete spawn;
}

Tile* Tile::deepCopy(BaseMap& map)
{
	Tile* copy = map.allocator.allocateTile(location);
	copy->flags = flags;
	copy->house_id = house_id;
	if(spawn) copy->spawn = spawn->deepCopy();
	if(creature) copy->creature = creature->deepCopy();
	// Spawncount & exits are not transferred on copy!
	if(ground) copy->ground = ground->deepCopy();

	ItemVector::iterator it;

	it = items.begin();
	while(it != items.end()) {
		copy->items.push_back((*it)->deepCopy());
		++it;
	}

	return copy;
}

uint32_t Tile::memsize() const
{
	uint32_t mem = sizeof(*this);
	if(ground) mem += ground->memsize();

	ItemVector::const_iterator it;

	it = items.begin();
	while(it != items.end()) {
		mem += (*it)->memsize();
		++it;
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

void Tile::merge(Tile* other) {
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

	ItemVector::iterator it;

	it = other->items.begin();
	while(it != other->items.end()) {
		addItem(*it);
		++it;
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

	ItemVector::const_iterator iit;
	for(iit = items.begin(); iit != items.end(); ++iit){
		if((*iit)->hasProperty(prop))
			return true;
	}

	return false;
}

Item* Tile::getTopItem()
{
	if(items.size() > 0 && !items.back()->isMetaItem())
		return items.back();
	if(ground && !ground->isMetaItem()) {
		return ground;
	}
	return nullptr;
}

void Tile::addItem(Item* item)
{
	if(!item) return;
	if(item->isGroundTile()) {
		//printf("ADDING GROUND\n");
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

	ItemVector::iterator it;

	it = items.begin();
	while(it != items.end()) {
		(*it)->select();
		++it;
	}

	statflags |= TILESTATE_SELECTED;
}


void Tile::deselect()
{
	if(ground) ground->deselect();
	if(spawn) spawn->deselect();
	if(creature) creature->deselect();

	ItemVector::iterator it;

	it = items.begin();
	while(it != items.end()) {
		(*it)->deselect();
		++it;
	}

	statflags &= ~TILESTATE_SELECTED;
}

Item* Tile::getTopSelectedItem()
{
	for(ItemVector::reverse_iterator iter = items.rbegin(); iter != items.rend(); ++iter) {
		if((*iter)->isSelected() && !(*iter)->isMetaItem()) {
			return *iter;
		}
	}
	if(ground && ground->isSelected() && !ground->isMetaItem()) {
		return ground;
	}
	return nullptr;
}

ItemVector Tile::popSelectedItems()
{
	ItemVector pop_items;

	if(!isSelected()) return pop_items;

	if(ground && ground->isSelected()) {
		pop_items.push_back(ground);
		ground = nullptr;
	}

	ItemVector::iterator it;

	it = items.begin();
	while(it != items.end()) {
		if((*it)->isSelected()) {
			pop_items.push_back(*it);
			it = items.erase(it);
		} else {
			++it;
		}
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

	ItemVector::iterator it;

	it = items.begin();
	while(it != items.end()) {
		if((*it)->isSelected()) {
			selected_items.push_back(*it);
		} it++;
	}

	return selected_items;
}

uint8_t Tile::getMiniMapColor() const
{
	for(ItemVector::const_reverse_iterator item_iter = items.rbegin(); item_iter != items.rend(); ++item_iter) {
		if((*item_iter)->getMiniMapColor()) {
			return (*item_iter)->getMiniMapColor();
			break;
		}
	}
	// check ground too
	if(hasGround()) {
		return ground->getMiniMapColor();
	}
	return 0;
}

bool tilePositionLessThan(const Tile* a, const Tile* b)
{
	return a->getPosition() < b->getPosition();
}

bool tilePositionVisualLessThan(const Tile* a, const Tile* b)
{
	Position pa = a->getPosition();
	Position pb = b->getPosition();

	if(pa.z > pb.z)
		return true;
	if(pa.z < pb.z)
		return false;

	if(pa.y < pb.y)
		return true;
	if(pa.y > pb.y)
		return false;

	if(pa.x < pb.x)
		return true;

	return false;
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
	}

	ItemVector::const_iterator iter = items.begin();
	while(iter != items.end()) {
		Item* i = *iter;
		if(i->isSelected()) {
			statflags |= TILESTATE_SELECTED;
		}
		if(i->getUniqueID() != 0) {
			statflags |= TILESTATE_UNIQUE;
		}

		ItemType& it = g_items[i->getID()];
		if(it.blockSolid) {
			statflags |= TILESTATE_BLOCKING;
		}
		if(it.isOptionalBorder) {
			statflags |= TILESTATE_OP_BORDER;
		}
		if(it.isTable) {
			statflags |= TILESTATE_HAS_TABLE;
		}
		if(it.isCarpet) {
			statflags |= TILESTATE_HAS_CARPET;
		}
		++iter;
	}

	if((statflags & TILESTATE_BLOCKING) == 0) {
		if(ground == nullptr && items.size() == 0) {
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
	if(ground) {
		if(ground->getGroundBrush()) {
			return ground->getGroundBrush();
		}
	}
	return nullptr;
}

void Tile::cleanBorders()
{
	ItemVector::iterator it;

	it = items.begin();
	while(it != items.end()) {
		if((*it)->isBorder()) {
			delete *it;
			it = items.erase(it);
		} else {
			// Borders should only be on the bottom, we can ignore the rest of the items
			return;
		}
	}
}

void Tile::wallize(BaseMap* parent)
{
	WallBrush::doWalls(parent, this);
}

Item* Tile::getWall() const
{
	ItemVector::const_iterator it;

	it = items.begin();
	while(it != items.end()) {
		if((*it)->isWall()) {
			return *it;
		} ++it;
	}
	return nullptr;
}

Item* Tile::getCarpet() const
{
	ItemVector::const_iterator it;

	it = items.begin();
	while(it != items.end()) {
		if((*it)->isCarpet()) {
			return *it;
		} ++it;
	}
	return nullptr;
}

Item* Tile::getTable() const
{
	ItemVector::const_iterator it;

	it = items.begin();
	while(it != items.end()) {
		if((*it)->isTable()) {
			return *it;
		} ++it;
	}
	return nullptr;
}

void Tile::addWallItem(Item* item)
{
	if(!item) {
		return;
	}
	ASSERT(item->isWall());

	addItem(item);
}

void Tile::cleanWalls(bool dontdelete)
{
	ItemVector::iterator it;

	it = items.begin();
	while(it != items.end()) {
		if((*it)->isWall()) {
			if(!dontdelete) {
				delete *it;
			}
			it = items.erase(it);
		} else ++it;
	}
}

void Tile::cleanWalls(WallBrush* wb)
{
	ItemVector::iterator it;

	it = items.begin();
	while(it != items.end()) {
		if((*it)->isWall() && wb->hasWall(*it)) {
			delete *it;
			it = items.erase(it);
		} else ++it;
	}
}

void Tile::cleanTables(bool dontdelete)
{
	ItemVector::iterator it;

	it = items.begin();
	while(it != items.end()) {
		if((*it)->isTable()) {
			if(!dontdelete) {
				delete *it;
			}
			it = items.erase(it);
		} else ++it;
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
	bool selected_ = false;
	if(ground) {
		ground->select();
		selected_ = true;
	}
	ItemVector::iterator it;

	it = items.begin();
	while(it != items.end()) {
		if((*it)->isBorder()) {
			(*it)->select();
			selected_ = true;
		} else {
			break;
		}
		++it;
	}
	if(selected_) statflags |= TILESTATE_SELECTED;
}


void Tile::deselectGround()
{
	if(ground) {
		ground->deselect();
	}
	ItemVector::iterator it = items.begin();
	while(it != items.end()) {
		if((*it)->isBorder()) {
			(*it)->deselect();
		} else {
			break;
		}
		++it;
	}
}

void Tile::setHouse(House* _house)
{
	house_id = (_house? _house->id : 0);
}

void Tile::addHouseExit(House* h)
{
	if(!h)
		return;
	HouseExitList* house_exits = location->createHouseExits();
	house_exits->push_back(h->id);
}

void Tile::removeHouseExit(House* h)
{
	if(!h)
		return;

	HouseExitList* house_exits = location->getHouseExits();
	if(!house_exits)
		return;

	for(std::vector<uint32_t>::iterator it = house_exits->begin(); it != house_exits->end(); ++it) {
		if(*it == h->id) {
			house_exits->erase(it);
			return;
		}
	}
}

