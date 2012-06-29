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

Materials materials;

Materials::Materials()
{
	// ...
}

Materials::~Materials()
{
	clear();
}

void Materials::clear()
{
	for(TilesetContainer::iterator iter = tilesets.begin();
			iter != tilesets.end();
			++iter)
	{
		delete iter->second;
	}

	for(MaterialsExtensionList::iterator iter = extensions.begin();
			iter != extensions.end();
			++iter)
	{
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
	for(MaterialsExtensionList::iterator iter = extensions.begin();
			iter != extensions.end();
			++iter)
	{
		if((*iter)->isForVersion(version_id))
		{
			ret_list.push_back(*iter);
		}
	}
	return ret_list;
}

bool Materials::loadMaterials(const FileName& identifier, wxString& error, wxArrayString& warnings)
{
	xmlDocPtr doc = xmlParseFile(identifier.GetFullPath().mb_str());
	std::string strValue;
	std::string warning;

	if(doc)
	{
		xmlNodePtr root = xmlDocGetRootElement(doc);
		
		if(xmlStrcmp(root->name,(const xmlChar*)"materials") != 0)
		{
			xmlFreeDoc(doc);
			error = wxT("materials.xml: Invalid root header");
			return false;
		}

		unserializeMaterials(identifier, root, error, warnings);
	}
	else
	{
		error = wxT("Couldn't open materials.xml (file not found or syntax error)");
		return false;
	}
	return true;
}

bool Materials::loadExtensions(FileName directoryName, wxString& error, wxArrayString& warnings)
{
	directoryName.Mkdir(0755, wxPATH_MKDIR_FULL); // Create if it doesn't exist

	wxDir ext_dir(directoryName.GetPath());
	if(ext_dir.IsOpened() == false)
	{
		error = wxT("Could not open extensions directory.");
		return false;
	}

	wxString filename;
	if(!ext_dir.GetFirst(&filename))
	{
		// No extensions found
		return true;
	}

	do
	{
		FileName fn;
		fn.SetPath(directoryName.GetPath());
		fn.SetFullName(filename);
		if(fn.GetExt() != wxT("xml"))
			continue;

		xmlDocPtr doc = xmlParseFile(fn.GetFullPath().mb_str());

		if(doc)
		{
			std::string strVal;

			xmlNodePtr root = xmlDocGetRootElement(doc);
			
			if(xmlStrcmp(root->name,(const xmlChar*)"materialsextension") != 0){
				xmlFreeDoc(doc);
				warnings.push_back(filename + wxT(": Invalid rootheader."));
				continue;
			}
			std::string ext_name, ext_author, ext_desc, ext_client_str;
			StringVector clientVersions;
			if(
				!readXMLValue(root, "name", ext_name) ||
				!readXMLValue(root, "author", ext_author) ||
				!readXMLValue(root, "description", ext_desc))
			{
				warnings.push_back(filename + wxT(": Couldn't read extension attributes (name, author, description)."));
				continue;
			}

			MaterialsExtension* me = newd MaterialsExtension(ext_name, ext_author, ext_desc);
			if(readXMLValue(root, "client", ext_client_str))
			{
				size_t last_pos = std::numeric_limits<size_t>::max();
				size_t pos;
				do
				{
					size_t to_pos = (last_pos == std::numeric_limits<size_t>::max()? 0 : last_pos+1);
					pos = ext_client_str.find(';', to_pos);
					if(size_t(pos) != std::string::npos)
					{
						clientVersions.push_back(ext_client_str.substr(to_pos, pos-(to_pos)));
						last_pos = pos;
					}
					else
					{
						clientVersions.push_back(ext_client_str.substr(to_pos));
						break;
					}
				} while(true);

				for(StringVector::iterator iter = clientVersions.begin();
						iter != clientVersions.end();
						++iter)
				{
					me->addVersion(*iter);
				}
			}
			else
			{
				warnings.push_back(filename + wxT(": Extension is not available for any version."));
			}
			extensions.push_back(me);
			
			if(me->isForVersion(gui.GetCurrentVersionID()))
			{
				unserializeMaterials(filename, root, error, warnings);
			}
		}
		else
		{
			warnings.push_back(wxT("Could not open ") + filename + wxT(" (file not found or syntax error)"));
			continue;
		}
	} while(ext_dir.GetNext(&filename));

	return true;
}

bool Materials::unserializeMaterials(const FileName& filename, xmlNodePtr root, wxString& error, wxArrayString& warnings)
{
	xmlNodePtr materialNode = root->children;
	wxString warning;
	std::string strValue;

	while(materialNode)
	{
		warning = wxT("");

		if(xmlStrcmp(materialNode->name,(const xmlChar*)"include") == 0)
		{
			std::string include_file;
			if(readXMLValue(materialNode, "file", include_file))
			{
				FileName include_name;
				include_name.SetPath(filename.GetPath());
				include_name.SetFullName(wxstr(include_file));
				wxString suberror;
				bool success = loadMaterials(include_name, suberror, warnings);
				if(!success)
					warnings.push_back(wxT("Error while loading file \"") + wxstr(include_file) + wxT("\": ") + suberror);
			}
		}
		else if(xmlStrcmp(materialNode->name,(const xmlChar*)"metaitem") == 0)
		{
			item_db.loadMetaItem(materialNode);
		}
		else if(xmlStrcmp(materialNode->name,(const xmlChar*)"border") == 0)
		{
			brushes.unserializeBorder(materialNode, warnings);
			if(warning.size()) warnings.push_back(wxT("materials.xml: ") + warning);
		}
		else if(xmlStrcmp(materialNode->name,(const xmlChar*)"brush") == 0)
		{
			brushes.unserializeBrush(materialNode, warnings);
			if(warning.size()) warnings.push_back(wxT("materials.xml: ") + warning);
		}
		else if(xmlStrcmp(materialNode->name,(const xmlChar*)"tileset") == 0)
		{
			unserializeTileset(materialNode, warnings);
		}
		materialNode = materialNode->next;
	}
	return true;
}

void Materials::createOtherTileset()
{
	Tileset* others;
	Tileset* npc_tileset;

	if(tilesets["Others"] != NULL)
	{
		others = tilesets["Others"];
		others->clear();
	}
	else
	{
		others = newd Tileset(brushes, "Others");
		tilesets["Others"] = others;
	}

	if(tilesets["NPCs"] != NULL)
	{
		npc_tileset = tilesets["NPCs"];
		npc_tileset->clear();
	}
	else
	{
		npc_tileset = newd Tileset(brushes, "NPCs");
		tilesets["NPCs"] = npc_tileset;
	}

	// There should really be an iterator to do this
	for(int id = 0; id < item_db.getMaxID(); ++id)
	{
		ItemType& it = item_db[id];
		if(it.id == 0) {
			continue;
		}
		if(it.isMetaItem() == false)
		{
			Brush* brush;
			if(it.in_other_tileset)
			{
				others->getCategory(TILESET_RAW)->brushlist.push_back(it.raw_brush);
				continue;
			}
			else if(it.raw_brush == NULL)
			{
				brush = it.raw_brush = newd RAWBrush(it.id);
				it.has_raw = true;
				brushes.addBrush(it.raw_brush);
			}
			else if(it.has_raw == false)
			{
				brush = it.raw_brush;
			}
			else
				continue;

			brush->flagAsVisible();
			others->getCategory(TILESET_RAW)->brushlist.push_back(it.raw_brush);
			it.in_other_tileset = true;
		}
	}
	for(CreatureMap::iterator iter = creature_db.begin();
			iter != creature_db.end();
			++iter)
	{
		CreatureType* type = iter->second;
		if(type->in_other_tileset)
		{
			if(type->isNpc)
				npc_tileset->getCategory(TILESET_CREATURE)->brushlist.push_back(type->brush);
			else
				others->getCategory(TILESET_CREATURE)->brushlist.push_back(type->brush);

		}
		else if(type->brush == NULL)
		{
			type->brush = newd CreatureBrush(type);
			brushes.addBrush(type->brush);
			type->brush->flagAsVisible();
			type->in_other_tileset = true;
			if(type->isNpc)
				npc_tileset->getCategory(TILESET_CREATURE)->brushlist.push_back(type->brush);
			else
				others->getCategory(TILESET_CREATURE)->brushlist.push_back(type->brush);

		}
	}
}

bool Materials::unserializeTileset(xmlNodePtr node, wxArrayString& warnings)
{
	std::string strVal;

	if(readXMLString(node, "name", strVal))
	{
		Tileset* ts;
		TilesetContainer::iterator iter = tilesets.find(strVal);
		if(iter != tilesets.end())
		{
			ts = iter->second;
		}
		else
		{
			ts = newd Tileset(brushes, strVal);
			tilesets.insert(make_pair(strVal, ts));
		}

		xmlNodePtr child = node->children;
		while(child)
		{
			ts->loadCategory(child, warnings);
			child = child->next;
		}
	}
	else
	{
		warnings.push_back(wxT("Couldn't read tileset name"));
		return false;
	}
	return true;
}

bool Materials::isInTileset(Item* item, std::string tilesetName) const
{
	const ItemType& it = item_db[item->getID()];

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

MaterialsExtension::MaterialsExtension(std::string name, std::string author, std::string description) :
	name(name),
	author(author),
	description(description),
	for_all_versions(false)
{
	// ...
}

MaterialsExtension::~MaterialsExtension()
{
	// ...
}

void MaterialsExtension::addVersion(std::string verStr)
{
	// We should go through the "real" version list and look through the names there instead
	// But as that list doesn't exist yet, this'll do for the moment
	if(verStr == "all")
		versionList.push_back(0);
	else if(verStr == "7.60" || verStr == "7.6")
		versionList.push_back(CLIENT_VERSION_760);
	else if(verStr == "8.00" || verStr == "8.0")
		versionList.push_back(CLIENT_VERSION_800);
	else if(verStr == "8.10" || verStr == "8.1" || verStr == "8.11")
		versionList.push_back(CLIENT_VERSION_810);
	else if(verStr == "8.20" || verStr == "8.21" || verStr == "8.22" || verStr == "8.30" || verStr == "8.31")
	{
		versionList.push_back(CLIENT_VERSION_820);
		versionList.push_back(CLIENT_VERSION_840);
		versionList.push_back(CLIENT_VERSION_850);
		versionList.push_back(CLIENT_VERSION_854);
		versionList.push_back(CLIENT_VERSION_860);
		versionList.push_back(CLIENT_VERSION_870);
		versionList.push_back(CLIENT_VERSION_910);
		versionList.push_back(CLIENT_VERSION_920);
	}
	else if(verStr == "8.40")
	{
		versionList.push_back(CLIENT_VERSION_840);
		versionList.push_back(CLIENT_VERSION_850);
		versionList.push_back(CLIENT_VERSION_854);
		versionList.push_back(CLIENT_VERSION_860);
		versionList.push_back(CLIENT_VERSION_870);
		versionList.push_back(CLIENT_VERSION_910);
		versionList.push_back(CLIENT_VERSION_920);
	}
	else if(verStr == "8.50")
	{
		versionList.push_back(CLIENT_VERSION_850);
		versionList.push_back(CLIENT_VERSION_854);
		versionList.push_back(CLIENT_VERSION_860);
		versionList.push_back(CLIENT_VERSION_870);
		versionList.push_back(CLIENT_VERSION_910);
		versionList.push_back(CLIENT_VERSION_920);
	}
	else if(verStr == "8.54")
	{
		versionList.push_back(CLIENT_VERSION_854);
		versionList.push_back(CLIENT_VERSION_860);
		versionList.push_back(CLIENT_VERSION_870);
		versionList.push_back(CLIENT_VERSION_910);
		versionList.push_back(CLIENT_VERSION_920);
	}
	else if(verStr == "8.60")
	{
		versionList.push_back(CLIENT_VERSION_860);
		versionList.push_back(CLIENT_VERSION_870);
		versionList.push_back(CLIENT_VERSION_910);
		versionList.push_back(CLIENT_VERSION_920);
	}
	else if(verStr == "8.70")
	{
		versionList.push_back(CLIENT_VERSION_870);
		versionList.push_back(CLIENT_VERSION_910);
		versionList.push_back(CLIENT_VERSION_920);
	}
	else if(verStr == "9.10")
	{
		versionList.push_back(CLIENT_VERSION_910);
		versionList.push_back(CLIENT_VERSION_920);
	}
	else if(verStr == "9.20")
	{
		versionList.push_back(CLIENT_VERSION_920);
	}
}

bool MaterialsExtension::isForVersion(uint16_t ver_id)
{
	for(std::vector<uint16_t>::iterator iter = versionList.begin();
			iter != versionList.end();
			++iter)
	{
		if(*iter == 0)
		{
			return true;
		}
		else if(*iter == ver_id)
		{
			return true;
		}
	}
	return false;
}

std::string MaterialsExtension::getVersionString()
{
	std::string versions;
	std::string last;
	for(std::vector<uint16_t>::iterator iter = versionList.begin();
			iter != versionList.end();
			++iter)
	{
		if(last.size())
		{
			if(versions.size())
				versions += ", " + last;
			else
				versions = last;
		}
		if(*iter == 0)
			return "All";
		else if(*iter == CLIENT_VERSION_740)
			last = "7.40";
		else if(*iter == CLIENT_VERSION_760)
			last = "7.60";
		else if(*iter == CLIENT_VERSION_800)
			last = "8.00";
		else if(*iter == CLIENT_VERSION_810)
			last = "8.10";
		else if(*iter == CLIENT_VERSION_820)
			last = "8.20 & 8.30";
		else if(*iter == CLIENT_VERSION_840)
			last = "8.40";
		else if(*iter == CLIENT_VERSION_850)
			last = "8.50";
		else if(*iter == CLIENT_VERSION_854)
			last = "8.54";
		else if(*iter == CLIENT_VERSION_860)
			last = "8.60";
	}
	if(last.size())
	{
		if(versions.size())
			versions += " and " + last;
		else
			versions = last;
	}
	else
		return "None";

	return versions;
}
