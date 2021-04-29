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
#include "graphics.h"
#include "gui.h"
#include "tile.h"
#include "complexitem.h"
#include "iomap.h"
#include "item.h"

#include "ground_brush.h"
#include "carpet_brush.h"
#include "table_brush.h"
#include "wall_brush.h"
#include "pugicast.h"

Item* Item::Create(uint16_t _type, uint16_t _subtype /*= 0xFFFF*/)
{
	if(_type == 0) return nullptr;
	Item* newItem = nullptr;

	const ItemType& it = g_items[_type];

	if(it.id != 0){
		if(it.isDepot()) {
			newItem = newd Depot(_type);
		} else if(it.isContainer()) {
			newItem = newd Container(_type);
		} else if(it.isTeleport()) {
			newItem = newd Teleport(_type);
		} else if(it.isDoor()) {
			newItem = newd Door(_type);
		} else if(_subtype == 0xFFFF) {
			if(it.isFluidContainer()) {
				newItem = newd Item(_type, LIQUID_NONE);
			} else if(it.isSplash()) {
				newItem = newd Item(_type, LIQUID_WATER);
			} else if(it.charges > 0) {
				newItem = newd Item(_type, it.charges);
			} else {
				newItem = newd Item(_type, 1);
			}
		} else {
			newItem = newd Item(_type, _subtype);
		}
	} else {
		newItem = newd Item(_type, _subtype);
	}

	return newItem;
}

Item::Item(unsigned short _type, unsigned short _count) :
	id(_type),
	subtype(1),
	selected(false),
	frame(0)
{
	if(hasSubtype()) {
		subtype = _count;
	}
}

Item::~Item()
{
	////
}

Item* Item::deepCopy() const
{
	Item* copy = Create(id, subtype);
	if(copy) {
		copy->selected = selected;
		if(attributes)
			copy->attributes = newd ItemAttributeMap(*attributes);
	}
	return copy;
}

Item* transformItem(Item* old_item, uint16_t new_id, Tile* parent)
{
	if(old_item == nullptr)
		return nullptr;

	old_item->setID(new_id);
	// Through the magic of deepCopy, this will now be a pointer to an item of the correct type.
	Item* new_item = old_item->deepCopy();
	if(parent) {
		// Find the old item and remove it from the tile, insert this one instead!
		if(old_item == parent->ground) {
			delete old_item;
			parent->ground = new_item;
			return new_item;
		}

		std::queue<Container*> containers;
		for(ItemVector::iterator item_iter = parent->items.begin(); item_iter != parent->items.end(); ++item_iter) {
			if(*item_iter == old_item) {
				delete old_item;
				item_iter = parent->items.erase(item_iter);
				parent->items.insert(item_iter, new_item);
				return new_item;
			}

			Container* c = dynamic_cast<Container*>(*item_iter);
			if(c)
				containers.push(c);
		}

		while(containers.size() != 0) {
			Container* container = containers.front();
			ItemVector& v = container->getVector();
			for(ItemVector::iterator item_iter = v.begin(); item_iter != v.end(); ++item_iter) {
				Item* i = *item_iter;
				Container* c = dynamic_cast<Container*>(i);
				if(c)
					containers.push(c);

				if(i == old_item) {
					// Found it!
					item_iter = v.erase(item_iter);
					v.insert(item_iter, new_item);
					return new_item;
				}
			}
			containers.pop();
		}
	}

	delete new_item;
	return nullptr;
}

uint32_t Item::memsize() const
{
	uint32_t mem = sizeof(*this);
	return mem;
}

void Item::setID(uint16_t newid)
{
	id = newid;
}

void Item::setSubtype(uint16_t n)
{
	subtype = n;
}

bool Item::hasSubtype() const
{
	const ItemType& it = g_items[id];
	return (it.isFluidContainer() || it.isSplash() || isCharged() || it.stackable || it.charges != 0);
}

uint16_t Item::getSubtype() const
{
	if(hasSubtype()) {
		return subtype;
	}
	return 0;
}

bool Item::hasProperty(enum ITEMPROPERTY prop) const
{
	const ItemType& it = g_items[id];
	switch(prop){
		case BLOCKSOLID:
			if(it.unpassable)
				return true;
			break;

		case MOVEABLE:
			if(it.moveable && getUniqueID() == 0)
				return true;
			break;
/*
		case HASHEIGHT:
			if(it.height != 0 )
				return true;
			break;
*/
		case BLOCKPROJECTILE:
			if(it.blockMissiles)
				return true;
			break;

		case BLOCKPATHFIND:
			if(it.blockPathfinder)
				return true;
			break;

		case HOOK_SOUTH:
			if(it.hookSouth)
				return true;
			break;

		case HOOK_EAST:
			if(it.hookEast)
				return true;
			break;

		case BLOCKINGANDNOTMOVEABLE:
			if(it.unpassable && (!it.moveable || getUniqueID() != 0))
				return true;
			break;

		default:
			return false;
	}
	return false;
}

std::pair<int, int> Item::getDrawOffset() const
{
	ItemType& it = g_items[id];
	if(it.sprite != nullptr) {
		return it.sprite->getDrawOffset();
	}
	return std::make_pair(0,0);
}

double Item::getWeight() const
{
	ItemType& it = g_items[id];
	if(it.stackable) {
		return it.weight * std::max(1, (int)subtype);
	}

	return it.weight;
}

void Item::setUniqueID(unsigned short n)
{
	setAttribute("uid", n);
}

void Item::setActionID(unsigned short n)
{
	setAttribute("aid", n);
}

void Item::setText(const std::string& str)
{
	setAttribute("text", str);
}

void Item::setDescription(const std::string& str)
{
	setAttribute("desc", str);
}

double Item::getWeight()
{
	ItemType& it = g_items[id];
	if(it.isStackable()) {
		return it.weight * subtype;
	}
	return it.weight;
}

bool Item::canHoldText() const
{
	return isReadable() || canWriteText();
}

bool Item::canHoldDescription() const
{
	return g_items[id].allowDistRead;
}

uint8_t Item::getMiniMapColor() const
{
	GameSprite* spr = g_items[id].sprite;
	if(spr) {
		return spr->getMiniMapColor();
	}
	return 0;
}

GroundBrush* Item::getGroundBrush() const
{
	ItemType& item_type = g_items.getItemType(id);
	if(item_type.isGroundTile() && item_type.brush && item_type.brush->isGround()) {
		return item_type.brush->asGround();
	}
	return nullptr;
}

TableBrush* Item::getTableBrush() const
{
	ItemType& item_type = g_items.getItemType(id);
	if(item_type.isTable && item_type.brush && item_type.brush->isTable()) {
		return item_type.brush->asTable();
	}
	return nullptr;
}

CarpetBrush* Item::getCarpetBrush() const
{
	ItemType& item_type = g_items.getItemType(id);
	if(item_type.isCarpet && item_type.brush && item_type.brush->isCarpet()) {
		return item_type.brush->asCarpet();
	}
	return nullptr;
}

DoorBrush* Item::getDoorBrush() const
{
	ItemType& item_type = g_items.getItemType(id);
	if(!item_type.isWall || !item_type.isBrushDoor || !item_type.brush || !item_type.brush->isWall()) {
		return nullptr;
	}

	DoorType door_type = item_type.brush->asWall()->getDoorTypeFromID(id);
	DoorBrush* door_brush = nullptr;
	// Quite a horrible dependency on a global here, meh.
	switch(door_type) {
		case WALL_DOOR_NORMAL: {
			door_brush = g_gui.normal_door_brush;
			break;
		}
		case WALL_DOOR_LOCKED: {
			door_brush = g_gui.locked_door_brush;
			break;
		}
		case WALL_DOOR_QUEST: {
			door_brush = g_gui.quest_door_brush;
			break;
		}
		case WALL_DOOR_MAGIC: {
			door_brush = g_gui.magic_door_brush;
			break;
		}
		case WALL_WINDOW: {
			door_brush = g_gui.window_door_brush;
			break;
		}
		case WALL_HATCH_WINDOW: {
			door_brush = g_gui.hatch_door_brush;
			break;
		}
		default: {
			break;
		}
	}
	return door_brush;
}

WallBrush* Item::getWallBrush() const
{
	ItemType& item_type = g_items.getItemType(id);
	if(item_type.isWall && item_type.brush && item_type.brush->isWall())
		return item_type.brush->asWall();
	return nullptr;
}

BorderType Item::getWallAlignment() const
{
	ItemType& it = g_items[id];
	if(!it.isWall) {
		return BORDER_NONE;
	}
	return it.border_alignment;
}

BorderType Item::getBorderAlignment() const
{
	ItemType& it = g_items[id];
	return it.border_alignment;
}

void Item::animate()
{
	ItemType& type = g_items[id];
	GameSprite* sprite = type.sprite;
	if(!sprite || !sprite->animator)
		return;

	frame = sprite->animator->getFrame();
}

// ============================================================================
// Static conversions

std::string Item::LiquidID2Name(uint16_t id)
{
	switch(id) {
		case LIQUID_NONE: return "None";
		case LIQUID_WATER: return "Water";
		case LIQUID_BLOOD: return "Blood";
		case LIQUID_BEER: return "Beer";
		case LIQUID_SLIME: return "Slime";
		case LIQUID_LEMONADE: return "Lemonade";
		case LIQUID_MILK: return "Milk";
		case LIQUID_MANAFLUID: return "Manafluid";
		case LIQUID_WATER2: return "Water";
		case LIQUID_LIFEFLUID: return "Lifefluid";
		case LIQUID_OIL: return "Oil";
		case LIQUID_SLIME2: return "Slime";
		case LIQUID_URINE: return "Urine";
		case LIQUID_COCONUT_MILK: return "Coconut Milk";
		case LIQUID_WINE: return "Wine";
		case LIQUID_MUD: return "Mud";
		case LIQUID_FRUIT_JUICE: return "Fruit Juice";
		case LIQUID_LAVA: return "Lava";
		case LIQUID_RUM: return "Rum";
		case LIQUID_SWAMP: return "Swamp";
		case LIQUID_INK: return "Ink";
		case LIQUID_TEA: return "Tea";
		case LIQUID_MEAD: return "Mead";
		default: return "Unknown";
	}
}

uint16_t Item::LiquidName2ID(std::string liquid)
{
	to_lower_str(liquid);
	if(liquid == "none") return LIQUID_NONE;
	if(liquid == "water") return LIQUID_WATER;
	if(liquid == "blood") return LIQUID_BLOOD;
	if(liquid == "beer") return LIQUID_BEER;
	if(liquid == "slime") return LIQUID_SLIME;
	if(liquid == "lemonade") return LIQUID_LEMONADE;
	if(liquid == "milk") return LIQUID_MILK;
	if(liquid == "manafluid") return LIQUID_MANAFLUID;
	if(liquid == "lifefluid") return LIQUID_LIFEFLUID;
	if(liquid == "oil") return LIQUID_OIL;
	if(liquid == "urine") return LIQUID_URINE;
	if(liquid == "coconut milk") return LIQUID_COCONUT_MILK;
	if(liquid == "wine") return LIQUID_WINE;
	if(liquid == "mud") return LIQUID_MUD;
	if(liquid == "fruit juice") return LIQUID_FRUIT_JUICE;
	if(liquid == "lava") return LIQUID_LAVA;
	if(liquid == "rum") return LIQUID_RUM;
	if(liquid == "swamp") return LIQUID_SWAMP;
	if(liquid == "ink") return LIQUID_INK;
	if(liquid == "tea") return LIQUID_TEA;
	if(liquid == "mead") return LIQUID_MEAD;
	return LIQUID_NONE;
}

// ============================================================================
// XML Saving & loading

Item* Item::Create(pugi::xml_node xml)
{
	pugi::xml_attribute attribute;

	int32_t id = 0;
	if((attribute = xml.attribute("id"))) {
		id = pugi::cast<int32_t>(attribute.value());
	}

	int32_t count = 1;
	if((attribute = xml.attribute("count")) || (attribute = xml.attribute("subtype"))) {
		count = pugi::cast<int32_t>(attribute.value());
	}

	return Create(id, count);
}


