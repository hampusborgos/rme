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

#include "gui.h"
#include "materials.h"
#include "brush.h"
#include "creatures.h"
#include "creature_brush.h"

CreatureDatabase g_creatures;

CreatureType::CreatureType() :
	isNpc(false),
	missing(false),
	in_other_tileset(false),
	standard(false),
	name(""),
	brush(nullptr)
{
	////
}

CreatureType::CreatureType(const CreatureType& ct) :
	isNpc(ct.isNpc),
	missing(ct.missing),
	in_other_tileset(ct.in_other_tileset),
	standard(ct.standard),
	name(ct.name),
	outfit(ct.outfit),
	brush(ct.brush)
{
	////
}

CreatureType& CreatureType::operator=(const CreatureType& ct)
{
	isNpc = ct.isNpc;
	missing = ct.missing;
	in_other_tileset = ct.in_other_tileset;
	standard = ct.standard;
	name = ct.name;
	outfit = ct.outfit;
	brush = ct.brush;
	return *this;
}

CreatureType::~CreatureType()
{
	////
}

CreatureType* CreatureType::loadFromXML(pugi::xml_node node, wxArrayString& warnings)
{
	pugi::xml_attribute attribute;
	if (!(attribute = node.attribute("type"))) {
		warnings.push_back("Couldn't read type tag of creature node.");
		return nullptr;
	}

	const std::string& tmpType = attribute.as_string();
	if (tmpType != "monster" && tmpType != "npc") {
		warnings.push_back("Invalid type tag of creature node \"" + wxstr(tmpType) + "\"");
		return nullptr;
	}

	if (!(attribute = node.attribute("name"))) {
		warnings.push_back("Couldn't read name tag of creature node.");
		return nullptr;
	}

	CreatureType* ct = newd CreatureType();
	ct->name = attribute.as_string();
	ct->isNpc = tmpType == "npc";

	if ((attribute = node.attribute("looktype"))) {
		ct->outfit.lookType = attribute.as_int();
		if(g_gui.gfx.getCreatureSprite(ct->outfit.lookType) == nullptr) {
			warnings.push_back("Invalid creature \"" + wxstr(ct->name) + "\" look type #" + std::to_string(ct->outfit.lookType));
		}
	}

	if ((attribute = node.attribute("lookitem"))) {
		ct->outfit.lookItem = attribute.as_int();
	}

	if ((attribute = node.attribute("lookmount"))) {
		ct->outfit.lookMount = attribute.as_int();
	}

	if ((attribute = node.attribute("lookaddon"))) {
		ct->outfit.lookAddon = attribute.as_int();
	}

	if ((attribute = node.attribute("lookhead"))) {
		ct->outfit.lookHead = attribute.as_int();
	}

	if ((attribute = node.attribute("lookbody"))) {
		ct->outfit.lookBody = attribute.as_int();
	}

	if ((attribute = node.attribute("looklegs"))) {
		ct->outfit.lookLegs = attribute.as_int();
	}

	if ((attribute = node.attribute("lookfeet"))) {
		ct->outfit.lookFeet = attribute.as_int();
	}

	if ((attribute = node.attribute("lookmounthead"))) {
		ct->outfit.lookMountHead = attribute.as_int();
	}

	if ((attribute = node.attribute("lookmountbody"))) {
		ct->outfit.lookMountBody = attribute.as_int();
	}

	if ((attribute = node.attribute("lookmountlegs"))) {
		ct->outfit.lookMountLegs = attribute.as_int();
	}

	if ((attribute = node.attribute("lookmountfeet"))) {
		ct->outfit.lookMountFeet = attribute.as_int();
	}

	return ct;
}

CreatureType* CreatureType::loadFromOTXML(const FileName& filename, pugi::xml_document& doc, wxArrayString& warnings)
{
	ASSERT(doc != nullptr);

	bool isNpc;
	pugi::xml_node node;
	if((node = doc.child("monster"))) {
		isNpc = false;
	} else if((node = doc.child("npc"))) {
		isNpc = true;
	} else {
		warnings.push_back("This file is not a monster/npc file");
		return nullptr;
	}

	pugi::xml_attribute attribute;
	if(!(attribute = node.attribute("name"))) {
		warnings.push_back("Couldn't read name tag of creature node.");
		return nullptr;
	}

	CreatureType* ct = newd CreatureType();
	if(isNpc) {
		ct->name = nstr(filename.GetName());
	} else {
		ct->name = attribute.as_string();
	}
	ct->isNpc = isNpc;

	for(pugi::xml_node optionNode = node.first_child(); optionNode; optionNode = optionNode.next_sibling()) {
		if(as_lower_str(optionNode.name()) != "look") {
			continue;
		}

		if((attribute = optionNode.attribute("type"))) {
			ct->outfit.lookType = attribute.as_int();
		}

		if((attribute = optionNode.attribute("item")) || (attribute = optionNode.attribute("lookex")) || (attribute = optionNode.attribute("typeex"))) {
			ct->outfit.lookItem = attribute.as_int();
		}

		if ((attribute = optionNode.attribute("mount"))) {
			ct->outfit.lookMount = attribute.as_int();
		}

		if((attribute = optionNode.attribute("addon"))) {
			ct->outfit.lookAddon = attribute.as_int();
		}

		if((attribute = optionNode.attribute("head"))) {
			ct->outfit.lookHead = attribute.as_int();
		}

		if((attribute = optionNode.attribute("body"))) {
			ct->outfit.lookBody = attribute.as_int();
		}

		if((attribute = optionNode.attribute("legs"))) {
			ct->outfit.lookLegs = attribute.as_int();
		}

		if((attribute = optionNode.attribute("feet"))) {
			ct->outfit.lookFeet = attribute.as_int();
		}

		if ((attribute = optionNode.attribute("mounthead"))) {
			ct->outfit.lookMountHead = attribute.as_int();
		}

		if ((attribute = optionNode.attribute("mountbody"))) {
			ct->outfit.lookMountBody = attribute.as_int();
		}

		if ((attribute = optionNode.attribute("mountlegs"))) {
			ct->outfit.lookMountLegs = attribute.as_int();
		}

		if ((attribute = optionNode.attribute("mountfeet"))) {
			ct->outfit.lookMountFeet = attribute.as_int();
		}
	}
	return ct;
}

CreatureDatabase::CreatureDatabase()
{
	////
}

CreatureDatabase::~CreatureDatabase()
{
	clear();
}

void CreatureDatabase::clear()
{
	for(CreatureMap::iterator iter = creature_map.begin(); iter != creature_map.end(); ++iter) {
		delete iter->second;
	}
	creature_map.clear();
}

CreatureType* CreatureDatabase::operator[](const std::string& name)
{
	CreatureMap::iterator iter = creature_map.find(as_lower_str(name));
	if(iter != creature_map.end()) {
		return iter->second;
	}
	return nullptr;
}

CreatureType* CreatureDatabase::addMissingCreatureType(const std::string& name, bool isNpc)
{
	assert((*this)[name] == nullptr);

	CreatureType* ct = newd CreatureType();
	ct->name = name;
	ct->isNpc = isNpc;
	ct->missing = true;
	ct->outfit.lookType = 130;

	creature_map.insert(std::make_pair(as_lower_str(name), ct));
	return ct;
}

CreatureType* CreatureDatabase::addCreatureType(const std::string& name, bool isNpc, const Outfit& outfit)
{
	assert((*this)[name] == nullptr);

	CreatureType* ct = newd CreatureType();
	ct->name = name;
	ct->isNpc = isNpc;
	ct->missing = false;
	ct->outfit = outfit;

	creature_map.insert(std::make_pair(as_lower_str(name), ct));
	return ct;
}

bool CreatureDatabase::hasMissing() const
{
	for(CreatureMap::const_iterator iter = creature_map.begin(); iter != creature_map.end(); ++iter) {
		if(iter->second->missing) {
			return true;
		}
	}
	return false;
}

bool CreatureDatabase::loadFromXML(const FileName& filename, bool standard, wxString& error, wxArrayString& warnings)
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filename.GetFullPath().mb_str());
	if(!result) {
		error = "Couldn't open file \"" + filename.GetFullName() + "\", invalid format?";
		return false;
	}

	pugi::xml_node node = doc.child("creatures");
	if(!node) {
		error = "Invalid file signature, this file is not a valid creatures file.";
		return false;
	}

	for(pugi::xml_node creatureNode = node.first_child(); creatureNode; creatureNode = creatureNode.next_sibling()) {
		if(as_lower_str(creatureNode.name()) != "creature") {
			continue;
		}

		CreatureType* creatureType = CreatureType::loadFromXML(creatureNode, warnings);
		if(creatureType) {
			creatureType->standard = standard;
			if((*this)[creatureType->name]) {
				warnings.push_back("Duplicate creature type name \"" + wxstr(creatureType->name) + "\"! Discarding...");
				delete creatureType;
			} else {
				creature_map[as_lower_str(creatureType->name)] = creatureType;
			}
		}
	}
	return true;
}

bool CreatureDatabase::importXMLFromOT(const FileName& filename, wxString& error, wxArrayString& warnings)
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(filename.GetFullPath().mb_str());
	if(!result) {
		error = "Couldn't open file \"" + filename.GetFullName() + "\", invalid format?";
		return false;
	}

	pugi::xml_node node;
	if((node = doc.child("monsters"))) {
		for(pugi::xml_node monsterNode = node.first_child(); monsterNode; monsterNode = monsterNode.next_sibling()) {
			if(as_lower_str(monsterNode.name()) != "monster") {
				continue;
			}

			pugi::xml_attribute attribute;
			if(!(attribute = monsterNode.attribute("file"))) {
				continue;
			}

			FileName monsterFile(filename);
			monsterFile.SetFullName(wxString(attribute.as_string(), wxConvUTF8));

			pugi::xml_document monsterDoc;
			pugi::xml_parse_result monsterResult = monsterDoc.load_file(monsterFile.GetFullPath().mb_str());
			if(!monsterResult) {
				continue;
			}

			CreatureType* creatureType = CreatureType::loadFromOTXML(monsterFile, monsterDoc, warnings);
			if(creatureType) {
				CreatureType* current = (*this)[creatureType->name];
				if(current) {
					*current = *creatureType;
					delete creatureType;
				} else {
					creature_map[as_lower_str(creatureType->name)] = creatureType;

					Tileset* tileSet = nullptr;
					if(creatureType->isNpc) {
						tileSet = g_materials.tilesets["NPCs"];
					} else {
						tileSet = g_materials.tilesets["Others"];
					}
					ASSERT(tileSet != nullptr);

					Brush* brush = newd CreatureBrush(creatureType);
					g_brushes.addBrush(brush);

					TilesetCategory* tileSetCategory = tileSet->getCategory(TILESET_CREATURE);
					tileSetCategory->brushlist.push_back(brush);
				}
			}
		}
	} else if((node = doc.child("monster")) || (node = doc.child("npc"))) {
		CreatureType* creatureType = CreatureType::loadFromOTXML(filename, doc, warnings);
		if(creatureType) {
			CreatureType* current = (*this)[creatureType->name];

			if(current) {
				*current = *creatureType;
				delete creatureType;
			} else {
				creature_map[as_lower_str(creatureType->name)] = creatureType;

				Tileset* tileSet = nullptr;
				if(creatureType->isNpc) {
					tileSet = g_materials.tilesets["NPCs"];
				} else {
					tileSet = g_materials.tilesets["Others"];
				}
				ASSERT(tileSet != nullptr);

				Brush* brush = newd CreatureBrush(creatureType);
				g_brushes.addBrush(brush);

				TilesetCategory* tileSetCategory = tileSet->getCategory(TILESET_CREATURE);
				tileSetCategory->brushlist.push_back(brush);
			}
		}
	} else {
		error = "This is not valid OT npc/monster data file.";
		return false;
	}
	return true;
}

bool CreatureDatabase::saveToXML(const FileName& filename)
{
	pugi::xml_document doc;

	pugi::xml_node decl = doc.prepend_child(pugi::node_declaration);
	decl.append_attribute("version") = "1.0";

	pugi::xml_node creatureNodes = doc.append_child("creatures");
	for(const auto& creatureEntry : creature_map) {
		CreatureType* creatureType = creatureEntry.second;
		if(!creatureType->standard) {
			pugi::xml_node creatureNode = creatureNodes.append_child("creature");

			creatureNode.append_attribute("name") = creatureType->name.c_str();
			creatureNode.append_attribute("type") = creatureType->isNpc ? "npc" : "monster";

			const Outfit& outfit = creatureType->outfit;
			creatureNode.append_attribute("looktype") = outfit.lookType;
			creatureNode.append_attribute("lookitem") = outfit.lookItem;
			creatureNode.append_attribute("lookmount") = outfit.lookMount;
			creatureNode.append_attribute("lookaddon") = outfit.lookAddon;
			creatureNode.append_attribute("lookhead") = outfit.lookHead;
			creatureNode.append_attribute("lookbody") = outfit.lookBody;
			creatureNode.append_attribute("looklegs") = outfit.lookLegs;
			creatureNode.append_attribute("lookfeet") = outfit.lookFeet;
			creatureNode.append_attribute("lookmounthead") = outfit.lookMountHead;
			creatureNode.append_attribute("lookmountbody") = outfit.lookMountBody;
			creatureNode.append_attribute("lookmountlegs") = outfit.lookMountLegs;
			creatureNode.append_attribute("lookmountfeet") = outfit.lookMountFeet;
		}
	}
	return doc.save_file(filename.GetFullPath().mb_str(), "\t", pugi::format_default, pugi::encoding_utf8);
}
