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

#include "editor.h"
#include "items.h"
#include "monsters.h"

#include "gui.h"
#include "materials.h"
#include "brush.h"
#include "monster_brush.h"
#include "npc_brush.h"
#include "raw_brush.h"

Materials g_materials;

Materials::Materials()
{
	////
}

Materials::~Materials()
{
	clear();
}

void Materials::clear()
{
	for(TilesetContainer::iterator iter = tilesets.begin(); iter != tilesets.end(); ++iter) {
		delete iter->second;
	}

	for(MaterialsExtensionList::iterator iter = extensions.begin(); iter != extensions.end(); ++iter) {
		delete *iter;
	}

	tilesets.clear();
	extensions.clear();
}

const MaterialsExtensionList& Materials::getExtensions()
{
	return extensions;
}

MaterialsExtensionList Materials::getExtensionsByVersion(uint16_t version_id)
{
	MaterialsExtensionList ret_list;
	for(MaterialsExtensionList::iterator iter = extensions.begin(); iter != extensions.end(); ++iter) {
		if((*iter)->isForVersion(version_id)) {
			ret_list.push_back(*iter);
		}
	}
	return ret_list;
}

bool Materials::loadMaterials(const FileName& identifier, wxString& error, wxArrayString& warnings, bool loadInclude)
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(identifier.GetFullPath().mb_str());
	if(!result) {
		warnings.push_back("Could not open " + identifier.GetFullName() + " (file not found or syntax error)");
		spdlog::error("[Materials::loadMaterials] - Could not open {} (file not found or syntax error)", identifier.GetFullName().ToStdString());
		return false;
	}

	pugi::xml_node node = doc.child("materials");
	if(!node) {
		warnings.push_back(identifier.GetFullName() + ": Invalid rootheader.");
		spdlog::error("[Materials::loadMaterials] - {} : Invalid rootheader", identifier.GetFullName().ToStdString());
		return false;
	}

	if (!loadInclude) {
		unserializeMaterials(identifier, node, error, warnings);
	}
	return true;
}

bool Materials::loadExtensions(FileName directoryName, wxString& error, wxArrayString& warnings)
{
	directoryName.Mkdir(0755, wxPATH_MKDIR_FULL); // Create if it doesn't exist

	wxDir ext_dir(directoryName.GetPath());
	if(!ext_dir.IsOpened()) {
		spdlog::error("[Materials::loadExtensions] - Could not open extensions directory");
		error = "Could not open extensions directory.";
		return false;
	}

	wxString filename;
	if(!ext_dir.GetFirst(&filename)) {
		// No extensions found
		spdlog::warn("[Materials::loadExtensions] - No extensions found");
		return true;
	}

	StringVector clientVersions;
	do {
		FileName fn;
		fn.SetPath(directoryName.GetPath());
		fn.SetFullName(filename);
		if(fn.GetExt() != "xml") {
			spdlog::warn("[Materials::loadExtensions] - Invalid extension {}", fn.GetExt().ToStdString());
			continue;
		}

		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(fn.GetFullPath().mb_str());
		if(!result) {
			spdlog::error("[Materials::loadExtensions] - Could not open {} (file not found or syntax error)", filename.ToStdString());
			warnings.push_back("Could not open " + filename + " (file not found or syntax error)");
			continue;
		}

		pugi::xml_node extensionNode = doc.child("materialsextension");
		if(!extensionNode) {
			spdlog::error("[Materials::loadExtensions] - Invalid rootheader {}", filename.ToStdString());
			warnings.push_back(filename + ": Invalid rootheader.");
			continue;
		}

		pugi::xml_attribute attribute;
		if(!(attribute = extensionNode.attribute("name"))) {
			spdlog::error("[Materials::loadExtensions] - Couldn't read extension name {}", filename.ToStdString());
			warnings.push_back(filename + ": Couldn't read extension name.");
			continue;
		}

		const std::string& extensionName = attribute.as_string();
		if(!(attribute = extensionNode.attribute("author"))) {
			spdlog::error("[Materials::loadExtensions] - Couldn't read extension name {}", filename.ToStdString());
			warnings.push_back(filename + ": Couldn't read extension name.");
			continue;
		}

		const std::string& extensionAuthor = attribute.as_string();
		if(!(attribute = extensionNode.attribute("description"))) {
			spdlog::info("[Materials::loadExtensions] - Couldn't read extension name {}", filename.ToStdString());
			warnings.push_back(filename + ": Couldn't read extension name.");
			continue;
		}

		const std::string& extensionDescription = attribute.as_string();
		if(extensionName.empty() || extensionAuthor.empty() || extensionDescription.empty()) {
			spdlog::error("[Materials::loadExtensions] - Couldn't read extension attributes (name, author, description) {}", filename.ToStdString());
			warnings.push_back(filename + ": Couldn't read extension attributes (name, author, description).");
			continue;
		}

		std::string extensionUrl = extensionNode.attribute("url").as_string();
		extensionUrl.erase(std::remove(extensionUrl.begin(), extensionUrl.end(), '\''));

		std::string extensionAuthorLink = extensionNode.attribute("authorurl").as_string();
		extensionAuthorLink.erase(std::remove(extensionAuthorLink.begin(), extensionAuthorLink.end(), '\''));

		MaterialsExtension* materialExtension = newd MaterialsExtension(extensionName, extensionAuthor, extensionDescription);
		materialExtension->url = extensionUrl;
		materialExtension->author_url = extensionAuthorLink;

		extensions.push_back(materialExtension);
		if(materialExtension->isForVersion(0)) {
			unserializeMaterials(filename, extensionNode, error, warnings);
			spdlog::warn("[Materials::loadExtensions] - Extension '{}' unserialized", extensionName);
		} else {
			spdlog::warn("[Materials::loadExtensions] - Extension '{}' not loaded due to different version", extensionName);
		}
	} while(ext_dir.GetNext(&filename));

	return true;
}

bool Materials::unserializeMaterials(const FileName& filename, pugi::xml_node node, wxString& error, wxArrayString& warnings)
{
	wxString warning;
	pugi::xml_attribute attribute;
	for(pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
		const std::string& childName = as_lower_str(childNode.name());
		if(childName == "include") {
			if(!(attribute = childNode.attribute("file"))) {
				continue;
			}

			FileName includeName;
			includeName.SetPath(filename.GetPath());
			includeName.SetName(wxString(attribute.as_string(), wxConvUTF8));

			wxString subError;
			if(!loadMaterials(includeName, subError, warnings, true)) {
				warnings.push_back("Error while loading file \"" + includeName.GetFullName() + "\": " + subError);
				spdlog::warn("[Materials::unserializeMaterials] - Error while loading file {}", includeName.GetFullName().ToStdString());
			}
		} else if(childName == "metaitem") {
			g_items.loadMetaItem(childNode);
		} else if(childName == "border") {
			g_brushes.unserializeBorder(childNode, warnings);
			if(warning.size()) {
				warnings.push_back("materials.xml: " + warning);
			}
		} else if(childName == "brush") {
			g_brushes.unserializeBrush(childNode, warnings);
			if(warning.size()) {
				warnings.push_back("materials.xml: " + warning);
			}
		} else if(childName == "tileset") {
			unserializeTileset(childNode, warnings);
		}
	}
	return true;
}

void Materials::createOtherTileset()
{
	Tileset* others;
	if(tilesets["Others"] != nullptr) {
		others = tilesets["Others"];
		others->clear();
	} else {
		others = newd Tileset(g_brushes, "Others");
		tilesets["Others"] = others;
	}

	// There should really be an iterator to do this
	for(int32_t id = 0; id <= g_items.getMaxID(); ++id) {
		ItemType& it = g_items[id];
		if(it.id == 0) {
			continue;
		}

		if(!it.isMetaItem()) {
			Brush* brush;
			if(it.in_other_tileset) {
				others->getCategory(TILESET_RAW)->brushlist.push_back(it.raw_brush);
				continue;
			} else if(it.raw_brush == nullptr) {
				brush = it.raw_brush = newd RAWBrush(it.id);
				it.has_raw = true;
				g_brushes.addBrush(it.raw_brush);
			} else if(!it.has_raw) {
				brush = it.raw_brush;
			} else
				continue;

			brush->flagAsVisible();
			others->getCategory(TILESET_RAW)->brushlist.push_back(it.raw_brush);
			it.in_other_tileset = true;
		}
	}

	for(MonsterMap::iterator iter = g_monsters.begin(); iter != g_monsters.end(); ++iter) {
		MonsterType* type = iter->second;
		if(type->in_other_tileset) {
			others->getCategory(TILESET_MONSTER)->brushlist.push_back(type->brush);
		} else if(type->brush == nullptr) {
			type->brush = newd MonsterBrush(type);
			g_brushes.addBrush(type->brush);
			type->brush->flagAsVisible();
			type->in_other_tileset = true;

			others->getCategory(TILESET_MONSTER)->brushlist.push_back(type->brush);
		}
	}
}

void Materials::createNpcTileset()
{
	Tileset* npcTileset;
	if(tilesets["NPCs"] != nullptr) {
		npcTileset = tilesets["NPCs"];
		npcTileset->clear();
	} else {
		npcTileset = newd Tileset(g_brushes, "NPCs");
		tilesets["NPCs"] = npcTileset;
	}

	for(NpcMap::iterator iter = g_npcs.begin(); iter != g_npcs.end(); ++iter) {
		NpcType* type = iter->second;
		if(type->in_other_tileset) {
			npcTileset->getCategory(TILESET_NPC)->brushlist.push_back(type->brush);
		} else if(type->brush == nullptr) {
			type->brush = newd NpcBrush(type);
			g_brushes.addBrush(type->brush);
			type->brush->flagAsVisible();
			type->in_other_tileset = true;
			npcTileset->getCategory(TILESET_NPC)->brushlist.push_back(type->brush);
		}
	}
}

bool Materials::unserializeTileset(pugi::xml_node node, wxArrayString& warnings)
{
	pugi::xml_attribute attribute;
	if(!(attribute = node.attribute("name"))) {
		warnings.push_back("Couldn't read tileset name");
		return false;
	}

	const std::string& name = attribute.as_string();

	Tileset* tileset;
	auto it = tilesets.find(name);
	if(it != tilesets.end()) {
		tileset = it->second;
	} else {
		tileset = newd Tileset(g_brushes, name);
		tilesets.insert(std::make_pair(name, tileset));
	}

	for(pugi::xml_node childNode = node.first_child(); childNode; childNode = childNode.next_sibling()) {
		tileset->loadCategory(childNode, warnings);
	}
	return true;
}

bool Materials::isInTileset(Item* item, std::string tilesetName) const
{
	const ItemType& it = g_items[item->getID()];

	return it.id != 0 && (
		isInTileset(it.brush, tilesetName) ||
		isInTileset(it.doodad_brush, tilesetName) ||
		isInTileset(it.raw_brush, tilesetName));
}

bool Materials::isInTileset(Brush* brush, std::string tilesetName) const
{
	if(!brush)
		return false;

	TilesetContainer::const_iterator tilesetiter = tilesets.find(tilesetName);
	if(tilesetiter == tilesets.end())
		return false;
	Tileset* tileset = tilesetiter->second;

	return tileset->containsBrush(brush);
}
