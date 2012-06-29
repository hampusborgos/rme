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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/creatures.hpp $
// $Id: creatures.hpp 310 2010-02-26 18:03:48Z admin $

#include "main.h"

#include "gui.h"
#include "materials.h"
#include "brush.h"
#include "creatures.h"
#include "creature_brush.h"

CreatureDatabase creature_db;

CreatureType::CreatureType() : 
	isNpc(false),
	missing(false),
	in_other_tileset(false),
	standard(false),
	name(""),
	brush(NULL)
{
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
}

CreatureType* CreatureType::loadFromXML(xmlNodePtr node, wxArrayString& warnings)
{
	std::string tmp_name, tmp_type;
	if(!readXMLValue(node, "type", tmp_type)) {
		warnings.push_back(wxT("Couldn't read type tag of creature node."));
		return NULL;
	}
	if(tmp_type != "monster" && tmp_type != "npc") {
		warnings.push_back(wxT("Invalid type tag of creature node \"") + wxstr(tmp_type) + wxT("\""));
		return NULL;
	}
	if(!readXMLValue(node, "name", tmp_name)) {
		warnings.push_back(wxT("Couldn't read name tag of creature node."));
		return NULL;
	}
	CreatureType* ct = newd CreatureType();
	ct->name = tmp_name;
	ct->isNpc = (tmp_type == "npc");
	
	int intVal;

	if(readXMLValue(node, "looktype", intVal)) {
		ct->outfit.lookType = intVal;
		if(gui.gfx.getCreatureSprite(intVal) == NULL) {
			wxString war;
			war << wxT("Invalid creature \"") << wxstr(tmp_name) << wxT("\" look type #");
			war << intVal;
			warnings.push_back(war);
		}
	}
	if(readXMLValue(node, "lookitem", intVal)) {
		ct->outfit.lookItem = intVal;
	}
	if(readXMLValue(node, "lookaddon", intVal)) {
		ct->outfit.lookAddon = intVal;
	}
	if(readXMLValue(node, "lookhead", intVal)) {
		ct->outfit.lookHead = intVal;
	}
	if(readXMLValue(node, "lookbody", intVal)) {
		ct->outfit.lookBody = intVal;
	}
	if(readXMLValue(node, "looklegs", intVal)) {
		ct->outfit.lookLegs = intVal;
	}
	if(readXMLValue(node, "lookfeet", intVal)) {
		ct->outfit.lookFeet = intVal;
	}

	return ct;
}

CreatureType* CreatureType::loadFromOTXML(const FileName& filename, xmlDocPtr doc, wxArrayString& warnings)
{
	ASSERT(doc != NULL);
	xmlNodePtr root = xmlDocGetRootElement(doc);

	bool valid = false;
	bool isNpc = false;

	if(xmlStrcmp(root->name,(const xmlChar*)"monster") == 0)
		valid = true;
	
	if(xmlStrcmp(root->name,(const xmlChar*)"npc") == 0)
	{
		valid = true;
		isNpc = true;
	}

	if(valid) {
		std::string tmp_name;
		if(!readXMLValue(root, "name", tmp_name)) {
			warnings.push_back(wxT("Couldn't read name tag of creature node."));
			xmlFreeDoc(doc);
			return NULL;
		}
		CreatureType* ct = newd CreatureType();
		if(isNpc)
			ct->name = nstr(filename.GetName());
		else
			ct->name = tmp_name;
		ct->isNpc = isNpc;

		xmlNodePtr optionNode = root->children;
		while(optionNode) {
			if(xmlStrcmp(optionNode->name,(const xmlChar*)"look") == 0) {
				int intVal;

				if(readXMLValue(optionNode, "type", intVal)) {
					ct->outfit.lookType = intVal;
				}
				if(readXMLValue(optionNode, "item", intVal) || readXMLValue(optionNode, "lookex", intVal)) {
					ct->outfit.lookItem = intVal;
				}
				if(readXMLValue(optionNode, "addon", intVal)) {
					ct->outfit.lookAddon = intVal;
				}
				if(readXMLValue(optionNode, "head", intVal)) {
					ct->outfit.lookHead = intVal;
				}
				if(readXMLValue(optionNode, "body", intVal)) {
					ct->outfit.lookBody = intVal;
				}
				if(readXMLValue(optionNode, "legs", intVal)) {
					ct->outfit.lookLegs = intVal;
				}
				if(readXMLValue(optionNode, "feet", intVal)) {
					ct->outfit.lookFeet = intVal;
				}
			}
			optionNode = optionNode->next;
		}
		xmlFreeDoc(doc);
		return ct;
	} else {
		xmlFreeDoc(doc);
		warnings.push_back(wxT("This file is not a monster/npc file"));
		return NULL;
	}
}

xmlNodePtr CreatureType::saveToXML()
{
	xmlNodePtr node =  xmlNewNode(NULL,(const xmlChar*)"creature");
	xmlSetProp(node, (const xmlChar*)"name", (const xmlChar*)name.c_str());
	xmlSetProp(node, (const xmlChar*)"type", (const xmlChar*)(isNpc? "npc" : "monster"));
	if(outfit.lookType != 0) xmlSetProp(node, (const xmlChar*)"looktype", (const xmlChar*)i2s(outfit.lookType).c_str());
	if(outfit.lookItem != 0) xmlSetProp(node, (const xmlChar*)"lookitem", (const xmlChar*)i2s(outfit.lookItem).c_str());
	if(outfit.lookAddon!= 0) xmlSetProp(node, (const xmlChar*)"lookaddon",(const xmlChar*)i2s(outfit.lookAddon).c_str());
	if(outfit.lookHead != 0) xmlSetProp(node, (const xmlChar*)"lookhead", (const xmlChar*)i2s(outfit.lookHead).c_str());
	if(outfit.lookBody != 0) xmlSetProp(node, (const xmlChar*)"lookbody", (const xmlChar*)i2s(outfit.lookBody).c_str());
	if(outfit.lookLegs != 0) xmlSetProp(node, (const xmlChar*)"looklegs", (const xmlChar*)i2s(outfit.lookLegs).c_str());
	if(outfit.lookFeet != 0) xmlSetProp(node, (const xmlChar*)"lookfeet", (const xmlChar*)i2s(outfit.lookFeet).c_str());
	return node;
}

CreatureDatabase::CreatureDatabase()
{
}

CreatureDatabase::~CreatureDatabase()
{
	clear();
}

void CreatureDatabase::clear()
{
	for(CreatureMap::iterator iter = creature_map.begin();
			iter != creature_map.end();
			++iter)
	{
		delete iter->second;
	}
	creature_map.clear();
}

CreatureType* CreatureDatabase::operator[](std::string name)
{
	CreatureMap::iterator iter = creature_map.find(as_lower_str(name));
	if(iter != creature_map.end()) {
		return iter->second;
	}
	return NULL;
}

CreatureType* CreatureDatabase::addMissingCreatureType(std::string name, bool isNpc)
{
	assert((*this)[name] == NULL);
	
	CreatureType* ct = newd CreatureType();
	ct->name = name;
	ct->isNpc = isNpc;
	ct->missing = true;
	ct->outfit.lookType = 130;

	creature_map.insert(std::make_pair(as_lower_str(name), ct));
	return ct;
}

bool CreatureDatabase::hasMissing() const
{
	for(CreatureMap::const_iterator iter = creature_map.begin();
			iter != creature_map.end();
			++iter)
	{
		if(iter->second->missing)
		{
			return true;
		}
	}
	return false;
}

bool CreatureDatabase::loadFromXML(const FileName& filename, bool standard, wxString& error, wxArrayString& warnings)
{
	xmlDocPtr doc = xmlParseFile(filename.GetFullPath().mb_str());
	if(doc)
	{
		xmlNodePtr root = xmlDocGetRootElement(doc);

		if(xmlStrcmp(root->name,(const xmlChar*)"creatures") != 0) {
			error = wxT("Invalid file signature, this file is not a valid creatures file.");
			xmlFreeDoc(doc);
			return false;
		}
		xmlNodePtr creatureNode = root->children;
		while(creatureNode)
		{
			if(xmlStrcmp(creatureNode->name, (const xmlChar*)"creature") == 0)
			{
				CreatureType* ct = CreatureType::loadFromXML(creatureNode, warnings);
				if(ct)
				{
					ct->standard = standard;
					if((*this)[ct->name])
					{
						warnings.push_back(wxT("Duplicate creature name \"") + wxstr(ct->name) + wxT("\"! Discarding..."));
						delete ct;
					}
					else
					{
						creature_map[as_lower_str(ct->name)] = ct;
					}
				}
			}
			creatureNode = creatureNode->next;
		}
		xmlFreeDoc(doc);
		return true;
	}
	error = wxT("Couldn't open file \"") + filename.GetFullName() + wxT("\", invalid format?");
	return false;
}

bool CreatureDatabase::importXMLFromOT(const FileName& filename, wxString& error, wxArrayString& warnings)
{
	xmlDocPtr doc = xmlParseFile(filename.GetFullPath().mb_str());
	if(doc)
	{
		xmlNodePtr root = xmlDocGetRootElement(doc);

		if(xmlStrcmp(root->name,(const xmlChar*)"monsters") == 0)
		{
			xmlNodePtr monsterNode = root->children;
			while(monsterNode)
			{
				if(xmlStrcmp(monsterNode->name, (const xmlChar*)"monster") == 0)
				{
					std::string fn;
					if(readXMLValue(monsterNode, "file", fn))
					{
						FileName monster_fn(filename);
						monster_fn.SetFullName(wxstr(fn));

						xmlDocPtr monster_doc = xmlParseFile(monster_fn.GetFullPath().mb_str());
						if(monster_doc)
						{
							CreatureType* ct = CreatureType::loadFromOTXML(monster_fn, monster_doc, warnings);
							if(ct)
							{
								CreatureType* current = (*this)[ct->name];

								if(current)
								{
									*current = *ct;
									delete ct;
								}
								else
								{
									creature_map[as_lower_str(ct->name)] = ct;
									Tileset* ts = NULL;
									if(ct->isNpc)
										ts = materials.tilesets["NPCs"];
									else
										ts = materials.tilesets["Others"];
									
									ASSERT(ts);
									Brush* brush = newd CreatureBrush(ct);
									brushes.addBrush(brush);
									TilesetCategory* tsc = ts->getCategory(TILESET_CREATURE);
									tsc->brushlist.push_back(brush);
								}
							}
						}
					}
				}
				monsterNode = monsterNode->next;
			}
			xmlFreeDoc(doc);
		}
		else if(xmlStrcmp(root->name,(const xmlChar*)"monster") == 0 || xmlStrcmp(root->name,(const xmlChar*)"npc") == 0)
		{
			CreatureType* ct = CreatureType::loadFromOTXML(filename, doc, warnings);
			if(ct)
			{
				CreatureType* current = (*this)[ct->name];

				if(current)
				{
					*current = *ct;
					delete ct;
				}
				else
				{
					creature_map[as_lower_str(ct->name)] = ct;
					Tileset* ts = NULL;
					if(ct->isNpc)
						ts = materials.tilesets["NPCs"];
					 else
						ts = materials.tilesets["Others"];

					ASSERT(ts);
					Brush* brush = newd CreatureBrush(ct);
					brushes.addBrush(brush);
					TilesetCategory* tsc = ts->getCategory(TILESET_CREATURE);
					tsc->brushlist.push_back(brush);
				}
			}
		}
		else
		{
			xmlFreeDoc(doc);
			error = wxT("This is not valid OT npc/monster data file.");
			return false;
		}
		return true;
	}
	error = wxT("Couldn't open file \"") + filename.GetFullName() + wxT("\", invalid format?");
	return false;
}

bool CreatureDatabase::saveToXML(const FileName& filename)
{
	xmlDocPtr doc = xmlNewDoc((const xmlChar*)"1.0");
	doc->children = xmlNewDocNode(doc, NULL, (const xmlChar*)"creatures", NULL);

	for(CreatureMap::iterator iter = creature_map.begin();
			iter != creature_map.end();
			++iter)
	{
		if(!iter->second->standard)
		{
			xmlAddChild(doc->children, (iter->second)->saveToXML());
		}
	}
	bool result = xmlSaveFormatFileEnc(filename.GetFullPath().mb_str(), doc, "UTF-8", 1);
	xmlFreeDoc(doc);
	return result;
}
