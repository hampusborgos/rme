//////////////////////////////////////////////////////////////////////
// This file is part of Canary Map Editor
//////////////////////////////////////////////////////////////////////
// Canary Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Canary Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "tileset.h"
#include "monsters.h"
#include "monster_brush.h"
#include "npcs.h"
#include "npc_brush.h"
#include "items.h"
#include "raw_brush.h"

Tileset::Tileset(Brushes& brushes, const std::string& name) :
	name(name),
	brushes(brushes)
{
	////
}

Tileset::~Tileset()
{
	for(TilesetCategoryArray::iterator iter = categories.begin(); iter != categories.end(); ++iter) {
		delete *iter;
	}
}

void Tileset::clear()
{
	for(TilesetCategoryArray::iterator iter = categories.begin(); iter != categories.end(); ++iter) {
		(*iter)->brushlist.clear();
	}
}

bool Tileset::containsBrush(Brush* brush) const
{
	for(TilesetCategoryArray::const_iterator iter = categories.begin(); iter != categories.end(); ++iter)
		if((*iter)->containsBrush(brush))
			return true;

	return false;
}

TilesetCategory* Tileset::getCategory(TilesetCategoryType type)
{
	ASSERT(type >= TILESET_UNKNOWN && type <= TILESET_HOUSE);
	for(TilesetCategoryArray::iterator iter = categories.begin(); iter != categories.end(); ++iter) {
		if((*iter)->getType() == type) {
			return *iter;
		}
	}
	TilesetCategory* tsc = newd TilesetCategory(*this, type);
	categories.push_back(tsc);
	return tsc;
}

bool TilesetCategory::containsBrush(Brush* brush) const
{
	for(std::vector<Brush*>::const_iterator iter = brushlist.begin(); iter != brushlist.end(); ++iter)
		if(*iter == brush)
			return true;

	return false;
}

const TilesetCategory* Tileset::getCategory(TilesetCategoryType type) const
{
	ASSERT(type >= TILESET_UNKNOWN && type <= TILESET_HOUSE);
	for(TilesetCategoryArray::const_iterator iter = categories.begin(); iter != categories.end(); ++iter) {
		if((*iter)->getType() == type) {
			return *iter;
		}
	}
	return nullptr;
}

void Tileset::loadCategory(pugi::xml_node node, wxArrayString &warnings)
{
	TilesetCategory* category = nullptr;
	TilesetCategory* subCategory = nullptr;

	const std::string& nodeName = as_lower_str(node.name());
	if(nodeName == "terrain") {
		category = getCategory(TILESET_TERRAIN);
	} else if(nodeName == "doodad") {
		category = getCategory(TILESET_DOODAD);
	} else if(nodeName == "items") {
		category = getCategory(TILESET_ITEM);
	} else if(nodeName == "raw") {
		category = getCategory(TILESET_RAW);
	} else if(nodeName == "terrain_and_raw") {
		category = getCategory(TILESET_TERRAIN);
		subCategory = getCategory(TILESET_RAW);
	} else if(nodeName == "doodad_and_raw") {
		category = getCategory(TILESET_DOODAD);
		subCategory = getCategory(TILESET_RAW);
	} else if(nodeName == "items_and_raw") {
		category = getCategory(TILESET_ITEM);
		subCategory = getCategory(TILESET_RAW);
	} else if(nodeName == "monsters") {
		category = getCategory(TILESET_MONSTER);
		for(pugi::xml_node brushNode = node.first_child(); brushNode; brushNode = brushNode.next_sibling()) {
			const std::string& brushName = as_lower_str(brushNode.name());
			if(brushName != "monster") {
				continue;
			}

			pugi::xml_attribute attribute;
			if(!(attribute = brushNode.attribute("name"))) {
				warnings.push_back("Couldn't read monster name tag of monster tileset");
				continue;
			}

			const std::string& monsterName = attribute.as_string();
			MonsterType* ctype = g_monsters[monsterName];
			if(ctype) {
				MonsterBrush* brush;
				if(ctype->brush) {
					brush = ctype->brush;
				} else {
					brush = ctype->brush = newd MonsterBrush(ctype);
					brushes.addBrush(brush);
				}
				brush->flagAsVisible();
				category->brushlist.push_back(brush);
			} else {
				warnings.push_back(wxString("Unknown monster type \"") << wxstr(monsterName) << "\"");
			}
		}
	} else if(nodeName == "npcs") {
		category = getCategory(TILESET_NPC);
		for(pugi::xml_node brushNode = node.first_child(); brushNode; brushNode = brushNode.next_sibling()) {
			const std::string& brushName = as_lower_str(brushNode.name());
			if(brushName != "npc") {
				continue;
			}

			pugi::xml_attribute attribute;
			if(!(attribute = brushNode.attribute("name"))) {
				warnings.push_back("Couldn't read npc name tag of npc tileset");
				continue;
			}

			const std::string& npcName = attribute.as_string();
			NpcType* npcType = g_npcs[npcName];
			if(npcType) {
				NpcBrush* brush;
				if(npcType->brush) {
					brush = npcType->brush;
				} else {
					brush = npcType->brush = newd NpcBrush(npcType);
					brushes.addBrush(brush);
				}
				brush->flagAsVisible();
				category->brushlist.push_back(brush);
			} else {
				warnings.push_back(wxString("Unknown npc type \"") << wxstr(npcName) << "\"");
			}
		}
	}

	if(!category) {
		return;
	}

	for(pugi::xml_node brushNode = node.first_child(); brushNode; brushNode = brushNode.next_sibling()) {
		category->loadBrush(brushNode, warnings);
		if(subCategory) {
			subCategory->loadBrush(brushNode, warnings);
		}
	}
}

//

TilesetCategory::TilesetCategory(Tileset& parent, TilesetCategoryType type) : type(type), tileset(parent)
{
	ASSERT(type >= TILESET_UNKNOWN && type <= TILESET_HOUSE);
}

TilesetCategory::~TilesetCategory()
{
	ASSERT(type >= TILESET_UNKNOWN && type <= TILESET_HOUSE);
}

bool TilesetCategory::isTrivial() const
{
	return (type == TILESET_ITEM) || (type == TILESET_RAW);
}

void TilesetCategory::loadBrush(pugi::xml_node node, wxArrayString& warnings)
{
	pugi::xml_attribute attribute;

	std::string brushName = node.attribute("after").as_string();
	if((attribute = node.attribute("afteritem"))) {
		ItemType& it = g_items[static_cast<uint16_t>(attribute.as_uint())];
		if(it.id != 0) {
			brushName = it.raw_brush ? it.raw_brush->getName() : std::string();
		}
	}

	const std::string& nodeName = as_lower_str(node.name());
	if(nodeName == "brush") {
		if(!(attribute = node.attribute("name"))) {
			return;
		}

		Brush* brush = tileset.brushes.getBrush(attribute.as_string());
		if(brush) {
			auto insertPosition = brushlist.end();
			if(!brushName.empty()) {
				for(auto itt = brushlist.begin(); itt != brushlist.end(); ++itt) {
					if((*itt)->getName() == brushName) {
						insertPosition = ++itt;
						break;
					}
				}
			}
			brush->flagAsVisible();
			brushlist.insert(insertPosition, brush);
		} else {
			warnings.push_back("Brush \"" + wxString(attribute.as_string(), wxConvUTF8) + "\" doesn't exist.");
		}
	} else if(nodeName == "item") {
		uint16_t fromId = 0, toId = 0;
		if(!(attribute = node.attribute("id"))) {
			if(!(attribute = node.attribute("fromid"))) {
				warnings.push_back("Couldn't read raw ids.");
			}
			toId = node.attribute("toid").as_uint();
		}

		fromId = static_cast<uint16_t>(attribute.as_uint());
		toId = std::max<uint16_t>(fromId, toId);

		std::vector<Brush*> tempBrushVector;
		for(uint16_t id = fromId; id <= toId; ++id) {
			ItemType& it = g_items[id];
			// Ignore item if not exist
			if(it.id == 0) {
				continue;
			}

			RAWBrush* brush;
			if(it.raw_brush) {
				brush = it.raw_brush;
			} else {
				brush = it.raw_brush = newd RAWBrush(it.id);
				it.has_raw = true;
				tileset.brushes.addBrush(brush); // This will take care of cleaning up afterwards
			}

			if(it.doodad_brush == nullptr && !isTrivial()) {
				it.doodad_brush = brush;
			}

			brush->flagAsVisible();
			tempBrushVector.push_back(brush);
		}

		auto insertPosition = brushlist.end();
		if(!brushName.empty()) {
			for(auto itt = brushlist.begin(); itt != brushlist.end(); ++itt) {
				if((*itt)->getName() == brushName) {
					insertPosition = ++itt;
					break;
				}
			}
		}
		brushlist.insert(insertPosition, tempBrushVector.begin(), tempBrushVector.end());
	}
}
