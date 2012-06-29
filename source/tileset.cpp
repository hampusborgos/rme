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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/tileset.hpp $
// $Id: tileset.hpp 310 2010-02-26 18:03:48Z admin $

#include "main.h"

#include "tileset.h"
#include "creatures.h"
#include "creature_brush.h"
#include "items.h"
#include "raw_brush.h"

Tileset::Tileset(Brushes& brushes, const std::string& name) :
	brushes(brushes),
		name(name)
{
}

Tileset::~Tileset()
{
	for(TilesetCategoryArray::iterator iter = categories.begin(); iter != categories.end(); ++iter)
	{
		delete *iter;
	}
}

void Tileset::clear() {
	for(TilesetCategoryArray::iterator iter = categories.begin(); iter != categories.end(); ++iter)
	{
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

TilesetCategory* Tileset::getCategory(TilesetCategoryType type) {
	ASSERT(type >= TILESET_UNKNOWN && type <= TILESET_HOUSE);
	for(TilesetCategoryArray::iterator iter = categories.begin(); iter != categories.end(); ++iter)
	{
		if((*iter)->getType() == type)
		{
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
	for(TilesetCategoryArray::const_iterator iter = categories.begin(); iter != categories.end(); ++iter)
	{
		if((*iter)->getType() == type)
		{
			return *iter;
		}
	}
	return NULL;
}

void Tileset::loadCategory(xmlNodePtr node, wxArrayString &warnings)
{
	TilesetCategory* category = NULL;
	TilesetCategory* category2 = NULL;

	if(xmlStrcmp(node->name,(const xmlChar*)"terrain") == 0)
	{
		category = getCategory(TILESET_TERRAIN);
	}
	else if(xmlStrcmp(node->name,(const xmlChar*)"doodad") == 0)
	{
		category = getCategory(TILESET_DOODAD);
	}
	else if(xmlStrcmp(node->name,(const xmlChar*)"items") == 0)
	{
		category = getCategory(TILESET_ITEM);
	}
	else if(xmlStrcmp(node->name,(const xmlChar*)"raw") == 0)
	{
		category = getCategory(TILESET_RAW);
	} else if(xmlStrcmp(node->name,(const xmlChar*)"terrain_and_raw") == 0)
	{
		category = getCategory(TILESET_TERRAIN);
		category2 = getCategory(TILESET_RAW);
	}
	else if(xmlStrcmp(node->name,(const xmlChar*)"doodad_and_raw") == 0)
	{
		category = getCategory(TILESET_DOODAD);
		category2 = getCategory(TILESET_RAW);
	}
	else if(xmlStrcmp(node->name,(const xmlChar*)"items_and_raw") == 0)
	{
		category = getCategory(TILESET_ITEM);
		category2 = getCategory(TILESET_RAW);
	}
	else if(xmlStrcmp(node->name,(const xmlChar*)"creatures") == 0)
	{
		category = getCategory(TILESET_CREATURE);
		for(xmlNodePtr brushNode = node->children; brushNode != NULL; brushNode = brushNode->next)
		{
			if(xmlStrcmp(brushNode->name,(const xmlChar*)"creature") == 0)
			{
				std::string creature_name;
				if(!readXMLValue(brushNode, "name", creature_name))
				{
					warnings.push_back(wxT("Couldn't read creature name tag of creature tileset"));
					continue;
				}

				CreatureType* ctype = creature_db[creature_name];
				if(ctype)
				{
					CreatureBrush* brush;
					if(ctype->brush)
					{
						brush = ctype->brush;
					}
					else
					{
						brush = ctype->brush = newd CreatureBrush(ctype);
						brushes.addBrush(brush);
					}
					brush->flagAsVisible();
					category->brushlist.push_back(brush);
				}
				else
				{
					warnings.push_back(wxString(wxT("Unknown creature type \"")) << wxstr(creature_name) << wxT("\""));
				}
			}
		}
	}
	
	if(!category)
		return;

	for(xmlNodePtr brushNode = node->children; brushNode != NULL; brushNode = brushNode->next)
	{
		category->loadBrush(brushNode, warnings);
		if(category2)
			category2->loadBrush(brushNode, warnings);
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
	if(type == TILESET_ITEM)
		return true;
	if(type == TILESET_RAW)
		return true;

	return false;
}

void TilesetCategory::loadBrush(xmlNodePtr node, wxArrayString& warnings)
{
	std::string strVal;
	std::string brush_name;
	int intVal = 0;
	
	readXMLValue(node, "after", brush_name);
	if(readXMLValue(node, "afteritem", intVal))
	{
		ItemType& it = item_db[intVal];
		if(it.id != 0)// Verify that we have a valid item
		{
			brush_name = (it.raw_brush? it.raw_brush->getName() : "");
		}
	}

	if(xmlStrcmp(node->name,(const xmlChar*)"brush") == 0)
	{
		if(readXMLString(node, "name", strVal))
		{
			Brush* brush = tileset.brushes.getBrush(strVal);
			if(brush)
			{
				std::vector<Brush*>::iterator insert_here = brushlist.end();
				if(brush_name.size())
				{
					for(std::vector<Brush*>::iterator iter = brushlist.begin(); iter != brushlist.end(); ++iter)
					{
						if((*iter)->getName() == brush_name)
						{
							insert_here = ++iter;
							break;
						}
					}
				}
				brush->flagAsVisible();
				brushlist.insert(insert_here, brush);
			}
			else
			{
				warnings.push_back(wxT("Brush \"") + wxstr(strVal) + wxT("\" doesn't exist."));
			}
		}
	}
	else if(xmlStrcmp(node->name,(const xmlChar*)"item") == 0)
	{
		int fromid = 0, toid = 0;
		if(!readXMLInteger(node, "id", fromid))
		{
			if(!readXMLInteger(node, "fromid", fromid))
			{
				warnings.push_back(wxT("Couldn't read raw ids."));
			}
			readXMLInteger(node, "toid", toid);
		}
		toid = std::max(toid, fromid);
		
		std::vector<Brush*>::iterator insert_here = brushlist.end();
		if(brush_name.size())
		{
			for(std::vector<Brush*>::iterator iter = brushlist.begin(); iter != brushlist.end(); ++iter)
			{
				if((*iter)->getName() == brush_name) 
				{
					insert_here = ++iter;
					break;
				}
			}
		}
		
		std::vector<Brush*> temp_vec;
		for(int id = fromid; id <= toid; ++id)
		{
			ItemType& it = item_db[id];
			if(it.id == 0) // Verify that we have a valid item
			{
				warnings.push_back(wxT("Unknown item id #") + i2ws(id) + wxT("."));
			}
			else
			{
				RAWBrush* brush;
				if(it.raw_brush)
				{
					brush = it.raw_brush;
				}
				else
				{
					brush = it.raw_brush = newd RAWBrush(it.id);
					it.has_raw = true;
					tileset.brushes.addBrush(brush); // This will take care of cleaning up afterwards
				}

				if(it.doodad_brush == NULL && !isTrivial())
				{
					it.doodad_brush = brush;
				}

				brush->flagAsVisible();
				temp_vec.push_back(brush);
			}
		}
		brushlist.insert(insert_here, temp_vec.begin(), temp_vec.end());
	}
}
