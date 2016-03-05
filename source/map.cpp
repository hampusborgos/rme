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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/map.hpp $
// $Id: map.hpp 310 2010-02-26 18:03:48Z admin $

#include "main.h"

#include "gui.h" // loadbar

#include "map.h"

#include <sstream>

Map::Map() : BaseMap(),
	width(512),
	height(512),
	houses(*this),
	has_changed(false),
	unnamed(false),
	waypoints(*this)
{
	// Earliest version possible
	// Caller is responsible for converting us to proper version
	mapVersion.otbm = MAP_OTBM_1;
	mapVersion.client = CLIENT_VERSION_NONE;
}

Map::~Map()
{
	////
}

bool Map::open(const std::string file)
{
	if(file == filename)
		return true; // Do not reopen ourselves!

	tilecount = 0;

	IOMapOTBM maploader(getVersion());

	bool success = maploader.loadMap(*this, wxstr(file));
	
	mapVersion = maploader.version;

	warnings = maploader.getWarnings();
	
	if(!success) {
		error = maploader.getError();
		return false;
	}
	
	has_changed = false;

	wxFileName fn = wxstr(file);
	filename = fn.GetFullPath().mb_str(wxConvUTF8);
	name = fn.GetFullName().mb_str(wxConvUTF8);

	// convert(getReplacementMapClassic(), true);

#if 0 // This will create a replacement map out of one of SO's template files
	std::ofstream out("templateshit.cpp");
	out << "\tConversionMap replacement_map;\n";
	int count = 0;
	out << "\tstd::vector<uint16_t> veckey, vecval;\n\n";

	for(int x = 20; ; x += 2) {
		int y = 22;
		Tile* old = getTile(x, y, GROUND_LAYER);
		if(old) {
			y -= 2;
			Tile* new_ = getTile(x, y, GROUND_LAYER);
			if(new_) {
				if(old->ground || old->items.size()) {
					out << "\tvecval.clear();\n";
					if(new_->ground)
						out << "\tvecval.push_back(" << new_->ground->getID() << ");\n";
					for(ItemVector::iterator iter = new_->items.begin(); iter != new_->items.end(); ++iter)
						out << "\tvecval.push_back(" << (*iter)->getID() << ");\n";

					if(old->ground && old->items.empty()) // Single item
						out << "\treplacement_map.stm[" << old->ground->getID() << "] = vecval;\n\n";
					else if(old->ground == nullptr && old->items.size() == 1) // Single item
						out << "\treplacement_map.stm[" << old->items.front()->getID() << "] = vecval;\n\n";
					else {
						// Many items
						out << "\tveckey.clear();\n";
						if(old->ground)
							out << "\tveckey.push_back(" << old->ground->getID() << ");\n";
						for(ItemVector::iterator iter = old->items.begin(); iter != old->items.end(); ++iter)
							out << "\tveckey.push_back(" << (*iter)->getID() << ");\n";
						out << "\tstd::sort(veckey.begin(), veckey.end());\n";
						out << "\treplacement_map.mtm[veckey] = vecval;\n\n";
					}
				}
			}
		} else {
			break;
		}
	}
	out.close();
#endif
	return true;
}

bool Map::convert(MapVersion to, bool showdialog)
{
	if(mapVersion.client == to.client) {
		// Only OTBM version differs
		// No changes necessary
		mapVersion = to;
		return true;
	}

	/* TODO

	if(to.otbm == MAP_OTBM_4 && to.client < CLIENT_VERSION_850)
		return false;

	if(mapVersion.client >= CLIENT_VERSION_760 && to.client < CLIENT_VERSION_760) 
		convert(getReplacementMapFrom760To740(), showdialog);

	if(mapVersion.client < CLIENT_VERSION_810 && to.client >= CLIENT_VERSION_810) 
		convert(getReplacementMapFrom800To810(), showdialog);

	if(mapVersion.client == CLIENT_VERSION_854_BAD && to.client >= CLIENT_VERSION_854) 
		convert(getReplacementMapFrom854To854(), showdialog);
	*/
	mapVersion = to;

	return true;
}

bool Map::convert(const ConversionMap& rm, bool showdialog)
{
	if(showdialog)
		gui.CreateLoadBar(wxT("Converting map ..."));

	uint64_t tiles_done = 0;
	std::vector<uint16_t> id_list;

	//std::ofstream conversions("converted_items.txt");

	for(MapIterator miter = begin(); miter != end(); ++miter) {
		Tile* tile = (*miter)->get();
		ASSERT(tile);

		if(tile->size() == 0)
			continue;
		
		// id_list try MTM conversion
		id_list.clear();

		if(tile->ground)
			id_list.push_back(tile->ground->getID());
		for(ItemVector::const_iterator item_iter = tile->items.begin(); item_iter != tile->items.end(); ++item_iter)
			if((*item_iter)->isBorder())
				id_list.push_back((*item_iter)->getID());

		std::sort(id_list.begin(), id_list.end());
		
		ConversionMap::MTM::const_iterator cfmtm = rm.mtm.end();

		while(id_list.size()) {
			cfmtm = rm.mtm.find(id_list);
			if(cfmtm != rm.mtm.end())
				break;
			id_list.pop_back();
		}

		// Keep track of how many items have been inserted at the bottom
		size_t inserted_items = 0;

		if(cfmtm != rm.mtm.end()) {
			const std::vector<uint16_t>& v = cfmtm->first;

			if(tile->ground && std::find(v.begin(), v.end(), tile->ground->getID()) != v.end()) {
				delete tile->ground;
				tile->ground = nullptr;
			}

			for(ItemVector::iterator item_iter = tile->items.begin(); item_iter != tile->items.end(); ) {
				if(std::find(v.begin(), v.end(), (*item_iter)->getID()) != v.end()) {
					delete *item_iter;
					item_iter = tile->items.erase(item_iter);
				}
				else
					++item_iter;
			}
		
			const std::vector<uint16_t>& new_items = cfmtm->second;
			for(std::vector<uint16_t>::const_iterator iit = new_items.begin(); iit != new_items.end(); ++iit) {
				Item* item = Item::Create(*iit);
				if(item->isGroundTile())
					tile->ground = item;
				else {
					tile->items.insert(tile->items.begin(), item);
					++inserted_items;
				}
			}
		}

		if(tile->ground) {
			ConversionMap::STM::const_iterator cfstm = rm.stm.find(tile->ground->getID());
			if(cfstm != rm.stm.end()) {
				uint16_t aid = tile->ground->getActionID();
				uint16_t uid = tile->ground->getUniqueID();
				delete tile->ground;
				tile->ground = nullptr;

				const std::vector<uint16_t>& v = cfstm->second;
				//conversions << "Converted " << tile->getX() << ":" << tile->getY() << ":" << tile->getZ() << " " << id << " -> ";
				for(std::vector<uint16_t>::const_iterator iit = v.begin(); iit != v.end(); ++iit) {
					Item* item = Item::Create(*iit);
					//conversions << *iit << " ";
					if(item->isGroundTile()) {
						item->setActionID(aid);
						item->setUniqueID(uid);
						tile->addItem(item);
					} else {
						tile->items.insert(tile->items.begin(), item);
						++inserted_items;
					}
				}
				//conversions << std::endl;
			}
		}

		for(ItemVector::iterator replace_item_iter = tile->items.begin() + inserted_items; replace_item_iter != tile->items.end(); ) {
			uint16_t id = (*replace_item_iter)->getID();
			ConversionMap::STM::const_iterator cf = rm.stm.find(id);
			if(cf != rm.stm.end()) {
				//uint16_t aid = (*replace_item_iter)->getActionID();
				//uint16_t uid = (*replace_item_iter)->getUniqueID();
				delete *replace_item_iter;

				replace_item_iter = tile->items.erase(replace_item_iter);
				const std::vector<uint16_t>& v = cf->second;
				for(std::vector<uint16_t>::const_iterator iit = v.begin(); iit != v.end(); ++iit) {
					replace_item_iter = tile->items.insert(replace_item_iter, Item::Create(*iit));
					//conversions << "Converted " << tile->getX() << ":" << tile->getY() << ":" << tile->getZ() << " " << id << " -> " << *iit << std::endl;
					++replace_item_iter;
				}
			}
			else
				++replace_item_iter;
		}

		++tiles_done;
		if(showdialog && tiles_done % 0x10000 == 0) {
			gui.SetLoadDone(int(tiles_done / double(getTileCount()) * 100.0));
		}
	}

	if(showdialog)
		gui.DestroyLoadBar();

	return true;
}

void Map::cleanInvalidTiles(bool showdialog)
{
	if(showdialog)
		gui.CreateLoadBar(wxT("Removing invalid tiles..."));

	uint64_t tiles_done = 0;

	for(MapIterator miter = begin(); miter != end(); ++miter) {
		Tile* tile = (*miter)->get();
		ASSERT(tile);

		if(tile->size() == 0)
			continue;

		for(ItemVector::iterator item_iter = tile->items.begin(); item_iter != tile->items.end();) {
			if(item_db.typeExists((*item_iter)->getID()))
				++item_iter;
			else {
				delete *item_iter;
				item_iter = tile->items.erase(item_iter);
			}
		}

		++tiles_done;
		if(showdialog && tiles_done % 0x10000 == 0) {
			gui.SetLoadDone(int(tiles_done / double(getTileCount()) * 100.0));
		}
	}

	if(showdialog)
		gui.DestroyLoadBar();
}

MapVersion Map::getVersion() const
{
	return mapVersion;
}

bool Map::hasChanged() const
{
	return has_changed;
}

bool Map::doChange()
{
	bool doupdate = !has_changed;
	has_changed = true;
	return doupdate;
}

bool Map::clearChanges()
{
	bool doupdate = has_changed;
	has_changed = false;
	return doupdate;
}

bool Map::hasFile() const
{
	return filename != "";
}

void Map::setWidth(int new_width)
{
	if(new_width > 65000)
		width = 65000;
	else if(new_width < 64)
		width = 64;
	else
		width = new_width;
}

void Map::setHeight(int new_height)
{
	if(new_height > 65000)
		height = 65000;
	else if(new_height < 64)
		height = 64;
	else
		height = new_height;
}
void Map::setMapDescription(const std::string& new_description)
{
	description = new_description;
}

void Map::setHouseFilename(const std::string&  new_housefile)
{
	housefile = new_housefile;
	unnamed = false;
}

void Map::setSpawnFilename(const std::string&  new_spawnfile)
{
	spawnfile = new_spawnfile;
	unnamed = false;
}

bool Map::addSpawn(Tile* tile)
{
	Spawn* spawn = tile->spawn;
	if(spawn) {
		int z = tile->getZ();
		int start_x = tile->getX() - spawn->getSize();
		int start_y = tile->getY() - spawn->getSize();
		int end_x = tile->getX() + spawn->getSize();
		int end_y = tile->getY() + spawn->getSize();

		for(int y = start_y; y <= end_y; ++y) {
			for(int x = start_x; x <= end_x; ++x) {
				TileLocation* ctile_loc = createTileL(x, y, z);
				ctile_loc->increaseSpawnCount();
			}
		}
		spawns.addSpawn(tile);
		return true;
	}
	return false;
}

void Map::removeSpawnInternal(Tile* tile)
{
	Spawn* spawn = tile->spawn;
	ASSERT(spawn);

	int z = tile->getZ();
	int start_x = tile->getX() - spawn->getSize();
	int start_y = tile->getY() - spawn->getSize();
	int end_x = tile->getX() + spawn->getSize();
	int end_y = tile->getY() + spawn->getSize();

	for(int y = start_y; y <= end_y; ++y) {
		for(int x = start_x; x <= end_x; ++x) {
			TileLocation* ctile_loc = getTileL(x, y, z);
			if(ctile_loc != nullptr && ctile_loc->getSpawnCount() > 0)
				ctile_loc->decreaseSpawnCount();
		}
	}
}

void Map::removeSpawn(Tile* tile)
{
	if(tile->spawn) {
		removeSpawnInternal(tile);
		spawns.removeSpawn(tile);
	}
}

SpawnList Map::getSpawnList(Tile* where)
{
	SpawnList list;
	TileLocation* tile_loc = where->getLocation();
	if(tile_loc) {
		if(tile_loc->getSpawnCount() > 0) {
			uint32_t found = 0;
			if(where->spawn) {
				++found;
				list.push_back(where->spawn);
			}

			// Scans the border tiles in an expanding square around the original spawn
			int z = where->getZ();
			int start_x = where->getX() - 1, end_x = where->getX() + 1;
			int start_y = where->getY() - 1, end_y = where->getY() + 1;
			while(found != tile_loc->getSpawnCount()) {
				for(int x = start_x; x <= end_x; ++x) {
					Tile* tile = getTile(x, start_y, z);
					if(tile && tile->spawn) {
						list.push_back(tile->spawn);
						++found;
					}
					tile = getTile(x, end_y, z);
					if(tile && tile->spawn) {
						list.push_back(tile->spawn);
						++found;
					}
				}

				for(int y = start_y + 1; y < end_y; ++y) {
					Tile* tile = getTile(start_x, y, z);
					if(tile && tile->spawn) {
						list.push_back(tile->spawn);
						++found;
					}
					tile = getTile(end_x, y, z);
					if(tile && tile->spawn) {
						list.push_back(tile->spawn);
						++found;
					}
				}
				--start_x, --start_y;
				++end_x, ++end_y;
			}
		}
	}
	return list;
}

bool Map::exportMinimap(FileName filename, wxBitmapType type, int floor /*= GROUND_LAYER*/, bool displaydialog)
{
	// TODO Divide huge maps into pieces.

	if(size() == 0)
		return true;

	int min_x = MAP_MAX_WIDTH + 1;
	int min_y = MAP_MAX_HEIGHT + 1;
	int max_x = 0;
	int max_y = 0;

	for(MapIterator mit = begin(); mit != end(); ++mit) {
		if((*mit)->get() == nullptr || (*mit)->empty())
			continue;

		Position pos = (*mit)->getPosition();

		if(pos.x < min_x)
			min_x = pos.x;

		if(pos.y < min_y)
			min_y = pos.y;

		if(pos.x > max_x)
			max_x = pos.x;

		if(pos.y > max_y)
			max_y = pos.y;
	}

	int minimap_width = max_x - min_x + 1;
	int minimap_height = max_y - min_y + 1;
	uint8_t* pixels = newd uint8_t[minimap_width * minimap_height * 3]; // 3 bytes per pixel
	memset(pixels, 0, minimap_width * minimap_height * 3);

	int tiles_iterated = 0;
	for(MapIterator mit = begin(); mit != end(); ++mit) {
		Tile* tile = (*mit)->get();
		++tiles_iterated;
		if(tiles_iterated % 8192 == 0 && displaydialog)
			gui.SetLoadDone(int(tiles_iterated / double(tilecount) * 90.0));

		if(tile == nullptr || tile->empty() || tile->getZ() != floor)
			continue;

		uint8_t color = 0;

		for(ItemVector::const_reverse_iterator item_iter = tile->items.rbegin(); item_iter != tile->items.rend(); ++item_iter) {
			if((*item_iter)->getMiniMapColor()) {
				color = (*item_iter)->getMiniMapColor();
				break;
			}
		}

		if(color == 0) {
			if(tile->hasGround()) {
				color = tile->ground->getMiniMapColor();
			}
		}

		uint32_t index = ((tile->getY() - min_y) * minimap_width + (tile->getX() - min_x)) * 3;

		pixels[index    ] = (uint8_t)(int(color / 36) % 6 * 51); // red
		pixels[index + 1] = (uint8_t)(int(color / 6) % 6 * 51);  // green
		pixels[index + 2] = (uint8_t)(color % 6 * 51);           // blue
	}

	wxImage* image = newd wxImage(minimap_width, minimap_height, pixels, true);
	image->SaveFile(filename.GetFullPath(), type);
	image->Destroy();
	delete[] pixels;
	return true;
}
