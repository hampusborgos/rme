//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/materials.hpp $
// $Id: materials.hpp 310 2010-02-26 18:03:48Z admin $

#include "main.h"

#include <wx/dir.h>

#include "editor.h"
#include "items.h"
#include "creatures.h"

#include "gui.h"
#include "materials.h"
#include "brush.h"
#include "creature_brush.h"
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

bool Materials::loadMaterials(const FileName& identifier, wxString& error, wxArrayString& warnings)
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(identifier.GetFullPath().mb_str());
	if(!result) {
		warnings.push_back("Could not open " + identifier.GetFullName() + " (file not found or syntax error)");
		return false;
	}

	pugi::xml_node node = doc.child("materials");
	if(!node) {
		warnings.push_back(identifier.GetFullName() + ": Invalid rootheader.");
		return false;
	}

	unserializeMaterials(identifier, node, error, warnings);
	return true;
}

bool Materials::loadExtensions(FileName directoryName, wxString& error, wxArrayString& warnings)
{
	directoryName.Mkdir(0755, wxPATH_MKDIR_FULL); // Create if it doesn't exist

	wxDir ext_dir(directoryName.GetPath());
	if(!ext_dir.IsOpened()) {
		error = "Could not open extensions directory.";
		return false;
	}

	wxString filename;
	if(!ext_dir.GetFirst(&filename)) {
		// No extensions found
		return true;
	}

	StringVector clientVersions;
	do {
		FileName fn;
		fn.SetPath(directoryName.GetPath());
		fn.SetFullName(filename);
		if(fn.GetExt() != "xml") {
			continue;
		}

		pugi::xml_document doc;
		pugi::xml_parse_result result = doc.load_file(fn.GetFullPath().mb_str());
		if(!result) {
			warnings.push_back("Could not open " + filename + " (file not found or syntax error)");
			continue;
		}

		pugi::xml_node extensionNode = doc.child("materialsextension");
		if(!extensionNode) {
			warnings.push_back(filename + ": Invalid rootheader.");
			continue;
		}

		pugi::xml_attribute attribute;
		if(!(attribute = extensionNode.attribute("name"))) {
			warnings.push_back(filename + ": Couldn't read extension name.");
			continue;
		}

		const std::string& extensionName = attribute.as_string();
		if(!(attribute = extensionNode.attribute("author"))) {
			warnings.push_back(filename + ": Couldn't read extension name.");
			continue;
		}

		const std::string& extensionAuthor = attribute.as_string();
		if(!(attribute = extensionNode.attribute("description"))) {
			warnings.push_back(filename + ": Couldn't read extension name.");
			continue;
		}

		const std::string& extensionDescription = attribute.as_string();
		if(extensionName.empty() || extensionAuthor.empty() || extensionDescription.empty()) {
			warnings.push_back(filename + ": Couldn't read extension attributes (name, author, description).");
			continue;
		}

		std::string extensionUrl = extensionNode.attribute("url").as_string();
		extensionUrl.erase(std::remove(extensionUrl.begin(), extensionUrl.end(), '\''), extensionUrl.end());

		std::string extensionAuthorLink = extensionNode.attribute("authorurl").as_string();
		extensionAuthorLink.erase(std::remove(extensionAuthorLink.begin(), extensionAuthorLink.end(), '\''), extensionAuthorLink.end());

		MaterialsExtension* materialExtension = newd MaterialsExtension(extensionName, extensionAuthor, extensionDescription);
		materialExtension->url = extensionUrl;
		materialExtension->author_url = extensionAuthorLink;

		if((attribute = extensionNode.attribute("client"))) {
			clientVersions.clear();
			const std::string& extensionClientString = attribute.as_string();

			size_t lastPosition = 0;
			size_t position = extensionClientString.find(';');
			while(position != std::string::npos) {
				clientVersions.push_back(extensionClientString.substr(lastPosition, position - lastPosition));
				lastPosition = position + 1;
				position = extensionClientString.find(';', lastPosition);
			}

			clientVersions.push_back(extensionClientString.substr(lastPosition));
			for(const std::string& version : clientVersions) {
				materialExtension->addVersion(version);
			}

			std::sort(materialExtension->version_list.begin(), materialExtension->version_list.end(), VersionComparisonPredicate);

			auto duplicate = std::unique(materialExtension->version_list.begin(), materialExtension->version_list.end());
			while(duplicate != materialExtension->version_list.end()) {
				materialExtension->version_list.erase(duplicate);
				duplicate = std::unique(materialExtension->version_list.begin(), materialExtension->version_list.end());
			}
		} else {
			warnings.push_back(filename + ": Extension is not available for any version.");
		}

		extensions.push_back(materialExtension);
		if(materialExtension->isForVersion(g_gui.GetCurrentVersionID())) {
			unserializeMaterials(filename, extensionNode, error, warnings);
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
			includeName.SetFullName(wxString(attribute.as_string(), wxConvUTF8));

			wxString subError;
			if(!loadMaterials(includeName, subError, warnings)) {
				warnings.push_back("Error while loading file \"" + includeName.GetFullName() + "\": " + subError);
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
	Tileset* npc_tileset;

	if(tilesets["Others"] != nullptr) {
		others = tilesets["Others"];
		others->clear();
	} else {
		others = newd Tileset(g_brushes, "Others");
		tilesets["Others"] = others;
	}

	if(tilesets["NPCs"] != nullptr) {
		npc_tileset = tilesets["NPCs"];
		npc_tileset->clear();
	} else {
		npc_tileset = newd Tileset(g_brushes, "NPCs");
		tilesets["NPCs"] = npc_tileset;
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

	for(CreatureMap::iterator iter = g_creatures.begin(); iter != g_creatures.end(); ++iter) {
		CreatureType* type = iter->second;
		if(type->in_other_tileset) {
			if(type->isNpc) {
				npc_tileset->getCategory(TILESET_CREATURE)->brushlist.push_back(type->brush);
			} else {
				others->getCategory(TILESET_CREATURE)->brushlist.push_back(type->brush);
			}
		} else if(type->brush == nullptr) {
			type->brush = newd CreatureBrush(type);
			g_brushes.addBrush(type->brush);
			type->brush->flagAsVisible();
			type->in_other_tileset = true;
			if(type->isNpc) {
				npc_tileset->getCategory(TILESET_CREATURE)->brushlist.push_back(type->brush);
			} else {
				others->getCategory(TILESET_CREATURE)->brushlist.push_back(type->brush);
			}
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
