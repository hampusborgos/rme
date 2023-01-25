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
#include "materials.h"
#include "map.h"
#include "complexitem.h"
#include "settings.h"
#include "gui.h"
#include "map_display.h"
#include "brush.h"
#include "ground_brush.h"
#include "wall_brush.h"
#include "table_brush.h"
#include "carpet_brush.h"
#include "waypoint_brush.h"
#include "house_exit_brush.h"
#include "doodad_brush.h"
#include "creature_brush.h"
#include "spawn_brush.h"

#include "live_server.h"
#include "live_client.h"
#include "live_action.h"

Editor::Editor(CopyBuffer& copybuffer) :
	live_server(nullptr),
	live_client(nullptr),
	actionQueue(newd ActionQueue(*this)),
	selection(*this),
	copybuffer(copybuffer),
	replace_brush(nullptr)
{
	wxString error;
	wxArrayString warnings;
	bool ok = true;

	ClientVersionID defaultVersion = ClientVersionID(g_settings.getInteger(Config::DEFAULT_CLIENT_VERSION));
	if(defaultVersion == CLIENT_VERSION_NONE)
		defaultVersion = ClientVersion::getLatestVersion()->getID();

	if(g_gui.GetCurrentVersionID() != defaultVersion) {
		if(g_gui.CloseAllEditors()) {
			ok = g_gui.LoadVersion(defaultVersion, error, warnings);
			g_gui.PopupDialog("Error", error, wxOK);
			g_gui.ListDialog("Warnings", warnings);
		} else {
			throw std::runtime_error("All maps of different versions were not closed.");
		}
	}

	if(!ok)
		throw std::runtime_error("Couldn't load client version");

	MapVersion version;
	version.otbm = g_gui.GetCurrentVersion().getPrefferedMapVersionID();
	version.client = g_gui.GetCurrentVersionID();
	map.convert(version);

	map.height = 2048;
	map.width = 2048;

	static int unnamed_counter = 0;

	std::string sname = "Untitled-" + i2s(++unnamed_counter);
	map.name = sname + ".otbm";
	map.spawnfile = sname + "-spawn.xml";
	map.housefile = sname + "-house.xml";
	map.description = "No map description available.";
	map.unnamed = true;

	map.doChange();
}

Editor::Editor(CopyBuffer& copybuffer, const FileName& fn) :
	live_server(nullptr),
	live_client(nullptr),
	actionQueue(newd ActionQueue(*this)),
	selection(*this),
	copybuffer(copybuffer),
	replace_brush(nullptr)
{
	MapVersion ver;
	if(!IOMapOTBM::getVersionInfo(fn, ver)) {
		// g_gui.PopupDialog("Error", "Could not open file \"" + fn.GetFullPath() + "\".", wxOK);
		throw std::runtime_error("Could not open file \"" + nstr(fn.GetFullPath()) + "\".\nThis is not a valid OTBM file or it does not exist.");
	}

	/*
	if(ver < CLIENT_VERSION_760) {
		long b = g_gui.PopupDialog("Error", "Unsupported Client Version (pre 7.6), do you want to try to load the map anyways?", wxYES | wxNO);
		if(b == wxID_NO) {
			valid_state = false;
			return;
		}
	}
	*/

	bool success = true;
	if(g_gui.GetCurrentVersionID() != ver.client) {
		wxString error;
		wxArrayString warnings;
		if(g_gui.CloseAllEditors()) {
			success = g_gui.LoadVersion(ver.client, error, warnings);
			if(!success)
				g_gui.PopupDialog("Error", error, wxOK);
			else
				g_gui.ListDialog("Warnings", warnings);
		} else {
			throw std::runtime_error("All maps of different versions were not closed.");
		}
	}

	if(success) {
		ScopedLoadingBar LoadingBar("Loading OTBM map...");
		success = map.open(nstr(fn.GetFullPath()));
		/* TODO
		if(success && ver.client == CLIENT_VERSION_854_BAD) {
			int ok = g_gui.PopupDialog("Incorrect OTB", "This map has been saved with an incorrect OTB version, do you want to convert it to the new OTB version?\n\nIf you are not sure, click Yes.", wxYES | wxNO);

			if(ok == wxID_YES){
				ver.client = CLIENT_VERSION_854;
				map.convert(ver);
			}
		}
		*/
	}
}

Editor::Editor(CopyBuffer& copybuffer, LiveClient* client) :
	live_server(nullptr),
	live_client(client),
	actionQueue(newd NetworkedActionQueue(*this)),
	selection(*this),
	copybuffer(copybuffer),
	replace_brush(nullptr)
{
	;
}

Editor::~Editor() {
	if(IsLive()) {
		CloseLiveServer();
	}

	UnnamedRenderingLock();
	selection.clear();
	delete actionQueue;
}

void Editor::addBatch(BatchAction* action, int stacking_delay)
{
	actionQueue->addBatch(action, stacking_delay);
	g_gui.UpdateMenus();
}

void Editor::addAction(Action* action, int stacking_delay )
{
	actionQueue->addAction(action, stacking_delay);
	g_gui.UpdateMenus();
}

void Editor::saveMap(FileName filename, bool showdialog)
{
	std::string savefile = filename.GetFullPath().mb_str(wxConvUTF8).data();
	bool save_as = false;
	bool save_otgz = false;

	if(savefile.empty()) {
		savefile = map.filename;

		FileName c1(wxstr(savefile));
		FileName c2(wxstr(map.filename));
		save_as = c1 != c2;
	}

	// If not named yet, propagate the file name to the auxilliary files
	if(map.unnamed) {
		FileName _name(filename);
		_name.SetExt("xml");

		_name.SetName(filename.GetName() + "-spawn");
		map.spawnfile = nstr(_name.GetFullName());
		_name.SetName(filename.GetName() + "-house");
		map.housefile = nstr(_name.GetFullName());

		map.unnamed = false;
	}

	// File object to convert between local paths etc.
	FileName converter;
	converter.Assign(wxstr(savefile));
	std::string map_path = nstr(converter.GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME));

	// Make temporary backups
	//converter.Assign(wxstr(savefile));
	std::string backup_otbm, backup_house, backup_spawn;

	if(converter.GetExt() == "otgz") {
		save_otgz = true;
		if(converter.FileExists()) {
			backup_otbm = map_path + nstr(converter.GetName()) + ".otgz~";
			std::remove(backup_otbm.c_str());
			std::rename(savefile.c_str(), backup_otbm.c_str());
		}
	} else {
		if(converter.FileExists()) {
			backup_otbm = map_path + nstr(converter.GetName()) + ".otbm~";
			std::remove(backup_otbm.c_str());
			std::rename(savefile.c_str(), backup_otbm.c_str());
		}

		converter.SetFullName(wxstr(map.housefile));
		if(converter.FileExists()) {
			backup_house = map_path + nstr(converter.GetName()) + ".xml~";
			std::remove(backup_house.c_str());
			std::rename((map_path + map.housefile).c_str(), backup_house.c_str());
		}

		converter.SetFullName(wxstr(map.spawnfile));
		if(converter.FileExists()) {
			backup_spawn = map_path + nstr(converter.GetName()) + ".xml~";
			std::remove(backup_spawn.c_str());
			std::rename((map_path + map.spawnfile).c_str(), backup_spawn.c_str());
		}
	}

	// Save the map
	{
		std::string n = nstr(g_gui.GetLocalDataDirectory()) + ".saving.txt";
		std::ofstream f(n.c_str(), std::ios::trunc | std::ios::out);
		f <<
			backup_otbm << std::endl <<
			backup_house << std::endl <<
			backup_spawn << std::endl;
	}

	{

		// Set up the Map paths
		wxFileName fn = wxstr(savefile);
		map.filename = fn.GetFullPath().mb_str(wxConvUTF8);
		map.name = fn.GetFullName().mb_str(wxConvUTF8);

		if(showdialog)
			g_gui.CreateLoadBar("Saving OTBM map...");

		// Perform the actual save
		IOMapOTBM mapsaver(map.getVersion());
		bool success = mapsaver.saveMap(map, fn);

		if(showdialog)
			g_gui.DestroyLoadBar();

		// Check for errors...
		if(!success) {
			// Rename the temporary backup files back to their previous names
			if(!backup_otbm.empty()) {
				converter.SetFullName(wxstr(savefile));
				std::string otbm_filename = map_path + nstr(converter.GetName());
				std::rename(backup_otbm.c_str(), std::string(otbm_filename + (save_otgz ? ".otgz" : ".otbm")).c_str());
			}

			if(!backup_house.empty()) {
				converter.SetFullName(wxstr(map.housefile));
				std::string house_filename = map_path + nstr(converter.GetName());
				std::rename(backup_house.c_str(), std::string(house_filename + ".xml").c_str());
			}

			if(!backup_spawn.empty()) {
				converter.SetFullName(wxstr(map.spawnfile));
				std::string spawn_filename = map_path + nstr(converter.GetName());
				std::rename(backup_spawn.c_str(), std::string(spawn_filename + ".xml").c_str());
			}

			// Display the error
			g_gui.PopupDialog("Error", "Could not save, unable to open target for writing.", wxOK);
		}

		// Remove temporary save runfile
		{
			std::string n = nstr(g_gui.GetLocalDataDirectory()) + ".saving.txt";
			std::remove(n.c_str());
		}

		// If failure, don't run the rest of the function
		if(!success)
			return;
	}


	// Move to permanent backup
	if(!save_as && g_settings.getInteger(Config::ALWAYS_MAKE_BACKUP)) {
		// Move temporary backups to their proper files
		time_t t = time(nullptr);
		tm* current_time = localtime(&t);
		ASSERT(current_time);

		std::ostringstream date;
		date << (1900 + current_time->tm_year);
		if(current_time->tm_mon < 9)
			date << "-" << "0" << current_time->tm_mon+1;
		else
			date << "-" << current_time->tm_mon+1;
		date << "-" << current_time->tm_mday;
		date << "-" << current_time->tm_hour;
		date << "-" << current_time->tm_min;
		date << "-" << current_time->tm_sec;

		if(!backup_otbm.empty()) {
			converter.SetFullName(wxstr(savefile));
			std::string otbm_filename = map_path + nstr(converter.GetName());
			std::rename(backup_otbm.c_str(), std::string(otbm_filename + "." + date.str() + (save_otgz ? ".otgz" : ".otbm")).c_str());
		}

		if(!backup_house.empty()) {
			converter.SetFullName(wxstr(map.housefile));
			std::string house_filename = map_path + nstr(converter.GetName());
			std::rename(backup_house.c_str(), std::string(house_filename + "." + date.str() + ".xml").c_str());
		}

		if(!backup_spawn.empty()) {
			converter.SetFullName(wxstr(map.spawnfile));
			std::string spawn_filename = map_path + nstr(converter.GetName());
			std::rename(backup_spawn.c_str(), std::string(spawn_filename + "." + date.str() + ".xml").c_str());
		}
	} else {
		// Delete the temporary files
		std::remove(backup_otbm.c_str());
		std::remove(backup_house.c_str());
		std::remove(backup_spawn.c_str());
	}

	map.clearChanges();
}

bool Editor::importMiniMap(FileName filename, int import, int import_x_offset, int import_y_offset, int import_z_offset)
{
	return false;
}

bool Editor::exportMiniMap(FileName filename, int floor /*= GROUND_LAYER*/, bool displaydialog)
{
	return map.exportMinimap(filename, floor, displaydialog);
}

bool Editor::exportSelectionAsMiniMap(FileName directory, wxString fileName)
{
	if(!directory.Exists() || !directory.IsDirWritable())
		return false;

	int min_x = MAP_MAX_WIDTH + 1, min_y = MAP_MAX_HEIGHT + 1, min_z = MAP_MAX_LAYER + 1;
	int max_x = 0, max_y = 0, max_z = 0;

	const TileSet& tiles = selection.getTiles();
	for(Tile* tile : tiles) {
		if(tile->empty())
			continue;

		const Position& pos = tile->getPosition();

		if(pos.x < min_x)
			min_x = pos.x;
		if(pos.x > max_x)
			max_x = pos.x;

		if(pos.y < min_y)
			min_y = pos.y;
		if(pos.y > max_y)
			max_y = pos.y;

		if(pos.z < min_z)
			min_z = pos.z;
		if(pos.z > max_z)
			max_z = pos.z;
	}

	int numtiles = (max_x - min_x) * (max_y - min_y);
	int minimap_width = max_x - min_x + 1;
	int minimap_height = max_y - min_y + 1;

	if(numtiles == 0)
		return false;

	if(minimap_width > 2048 || minimap_height > 2048) {
		g_gui.PopupDialog("Error", "Minimap size greater than 2048px.", wxOK);
		return false;
	}

	int tiles_iterated = 0;

	for(int z = min_z; z <= max_z; z++) {
		uint8_t* pixels = newd uint8_t[minimap_width * minimap_height * 3]; // 3 bytes per pixel
		memset(pixels, 0, minimap_width * minimap_height * 3);

		for(Tile* tile : tiles) {
			if(tile->getZ() != z)
				continue;

			++tiles_iterated;
			if(tiles_iterated % 8192 == 0)
				g_gui.SetLoadDone(int(tiles_iterated / double(tiles.size()) * 90.0));

			if(tile->empty())
				continue;

			uint8_t color = 0;

			for(Item* item : tile->items) {
				if(item->getMiniMapColor() != 0) {
					color = item->getMiniMapColor();
					break;
				}
			}

			if(color == 0 && tile->hasGround())
				color = tile->ground->getMiniMapColor();

			uint32_t index = ((tile->getY() - min_y) * minimap_width + (tile->getX() - min_x)) * 3;

			pixels[index] = (uint8_t)(int(color / 36) % 6 * 51);     // red
			pixels[index + 1] = (uint8_t)(int(color / 6) % 6 * 51);  // green
			pixels[index + 2] = (uint8_t)(color % 6 * 51);           // blue
		}

		FileName file(fileName + "_" + i2ws(z) + ".png");
		file.Normalize(wxPATH_NORM_ALL, directory.GetFullPath());
		wxImage* image = newd wxImage(minimap_width, minimap_height, pixels, true);
		image->SaveFile(file.GetFullPath(), wxBITMAP_TYPE_PNG);
		image->Destroy();
		delete[] pixels;
	}

	return true;
}

bool Editor::importMap(FileName filename, int import_x_offset, int import_y_offset, int import_z_offset, ImportType house_import_type, ImportType spawn_import_type)
{
	selection.clear();
	actionQueue->clear();

	Map imported_map;
	bool loaded = imported_map.open(nstr(filename.GetFullPath()));

	if(!loaded) {
		g_gui.PopupDialog("Error", "Error loading map!\n" + imported_map.getError(), wxOK | wxICON_INFORMATION);
		return false;
	}
	g_gui.ListDialog("Warning", imported_map.getWarnings());

	Position offset(import_x_offset, import_y_offset, import_z_offset);

	bool resizemap = false;
	bool resize_asked = false;
	int newsize_x = map.getWidth(), newsize_y = map.getHeight();
	int discarded_tiles = 0;

	g_gui.CreateLoadBar("Merging maps...");

	std::map<uint32_t, uint32_t> town_id_map;
	std::map<uint32_t, uint32_t> house_id_map;

	if(house_import_type != IMPORT_DONT) {
		for(TownMap::iterator tit = imported_map.towns.begin(); tit != imported_map.towns.end();) {
			Town* imported_town = tit->second;
			Town* current_town = map.towns.getTown(imported_town->getID());

			Position oldexit = imported_town->getTemplePosition();
			Position newexit = oldexit + offset;
			if(newexit.isValid()) {
				imported_town->setTemplePosition(newexit);
			}

			switch(house_import_type) {
				case IMPORT_MERGE: {
					town_id_map[imported_town->getID()] = imported_town->getID();
					if(current_town) {
						++tit;
						continue;
					}
					break;
				}
				case IMPORT_SMART_MERGE: {
					if(current_town) {
						// Compare and insert/merge depending on parameters
						if(current_town->getName() == imported_town->getName() && current_town->getID() == imported_town->getID()) {
							// Just add to map
							town_id_map[imported_town->getID()] = current_town->getID();
							++tit;
							continue;
						} else {
							// Conflict! Find a newd id and replace old
							uint32_t new_id = map.towns.getEmptyID();
							imported_town->setID(new_id);
							town_id_map[imported_town->getID()] = new_id;
						}
					} else {
						town_id_map[imported_town->getID()] = imported_town->getID();
					}
					break;
				}
				case IMPORT_INSERT: {
					// Find a newd id and replace old
					uint32_t new_id = map.towns.getEmptyID();
					imported_town->setID(new_id);
					town_id_map[imported_town->getID()] = new_id;
					break;
				}
				case IMPORT_DONT: {
					++tit;
					continue; // Should never happend..?
					break; // Continue or break ?
				}
			}

			map.towns.addTown(imported_town);

#ifdef __VISUALC__ // C++0x compliance to some degree :)
			tit = imported_map.towns.erase(tit);
#else // Bulky, slow way
			TownMap::iterator tmp_iter = tit;
			++tmp_iter;
			uint32_t next_key = 0;
			if(tmp_iter != imported_map.towns.end()) {
				next_key = tmp_iter->first;
			}
			imported_map.towns.erase(tit);
			if(next_key != 0) {
				tit = imported_map.towns.find(next_key);
			} else {
				tit = imported_map.towns.end();
			}
#endif
		}

		for(HouseMap::iterator hit = imported_map.houses.begin(); hit != imported_map.houses.end();) {
			House* imported_house = hit->second;
			House* current_house = map.houses.getHouse(imported_house->id);
			imported_house->townid = town_id_map[imported_house->townid];

			Position oldexit = imported_house->getExit();
			imported_house->setExit(nullptr, Position()); // Reset it

			switch(house_import_type) {
				case IMPORT_MERGE: {
					house_id_map[imported_house->id] = imported_house->id;
					if(current_house) {
						++hit;
						Position newexit = oldexit + offset;
						if(newexit.isValid()) current_house->setExit(&map, newexit);
						continue;
					}
					break;
				}
				case IMPORT_SMART_MERGE: {
					if(current_house) {
						// Compare and insert/merge depending on parameters
						if(current_house->name == imported_house->name && current_house->townid == imported_house->townid) {
							// Just add to map
							house_id_map[imported_house->id] = current_house->id;
							++hit;
							Position newexit = oldexit + offset;
							if(newexit.isValid()) imported_house->setExit(&map, newexit);
							continue;
						} else {
							// Conflict! Find a newd id and replace old
							uint32_t new_id = map.houses.getEmptyID();
							house_id_map[imported_house->id] = new_id;
							imported_house->id = new_id;
						}
					} else {
						house_id_map[imported_house->id] = imported_house->id;
					}
					break;
				}
				case IMPORT_INSERT: {
					// Find a newd id and replace old
					uint32_t new_id = map.houses.getEmptyID();
					house_id_map[imported_house->id] = new_id;
					imported_house->id = new_id;
					break;
				}
				case IMPORT_DONT: {
					++hit;
					Position newexit = oldexit + offset;
					if(newexit.isValid()) imported_house->setExit(&map, newexit);
						continue; // Should never happend..?
					break;
				}
			}

			Position newexit = oldexit + offset;
			if(newexit.isValid()) imported_house->setExit(&map, newexit);
			map.houses.addHouse(imported_house);

#ifdef __VISUALC__ // C++0x compliance to some degree :)
			hit = imported_map.houses.erase(hit);
#else // Bulky, slow way
			HouseMap::iterator tmp_iter = hit;
			++tmp_iter;
			uint32_t next_key = 0;
			if(tmp_iter != imported_map.houses.end()) {
				next_key = tmp_iter->first;
			}
			imported_map.houses.erase(hit);
			if(next_key != 0) {
				hit = imported_map.houses.find(next_key);
			} else {
				hit = imported_map.houses.end();
			}
#endif
		}
	}

	std::map<Position, Spawn*> spawn_map;
	if(spawn_import_type != IMPORT_DONT) {
		for(SpawnPositionList::iterator siter = imported_map.spawns.begin(); siter != imported_map.spawns.end();) {
			Position old_spawn_pos = *siter;
			Position new_spawn_pos = *siter + offset;
			switch(spawn_import_type) {
				case IMPORT_SMART_MERGE:
				case IMPORT_INSERT:
				case IMPORT_MERGE: {
					Tile* imported_tile = imported_map.getTile(old_spawn_pos);
					if(imported_tile) {
						ASSERT(imported_tile->spawn);
						spawn_map[new_spawn_pos] = imported_tile->spawn;

						SpawnPositionList::iterator next = siter;
						bool cont = true;
						Position next_spawn;

						++next;
						if(next == imported_map.spawns.end())
							cont = false;
						else
							next_spawn = *next;
						imported_map.spawns.erase(siter);
						if(cont)
							siter = imported_map.spawns.find(next_spawn);
						else
							siter = imported_map.spawns.end();
					}
					break;
				}
				case IMPORT_DONT: {
					++siter;
					break;
				}
			}
		}
	}

	// Plain merge of waypoints, very simple! :)
	for(WaypointMap::iterator iter = imported_map.waypoints.begin(); iter != imported_map.waypoints.end(); ++iter) {
		iter->second->pos += offset;
	}

	map.waypoints.waypoints.insert(imported_map.waypoints.begin(), imported_map.waypoints.end());
	imported_map.waypoints.waypoints.clear();


	uint64_t tiles_merged = 0;
	uint64_t tiles_to_import = imported_map.tilecount;
	for(MapIterator mit = imported_map.begin(); mit != imported_map.end(); ++mit) {
		if(tiles_merged % 8092 == 0) {
			g_gui.SetLoadDone(int(100.0 * tiles_merged / tiles_to_import));
		}
		++tiles_merged;

		Tile* import_tile = (*mit)->get();
		Position new_pos = import_tile->getPosition() + offset;
		if(!new_pos.isValid()) {
			++discarded_tiles;
			continue;
		}

		if(!resizemap && (new_pos.x > map.getWidth() || new_pos.y > map.getHeight())) {
			if(resize_asked) {
				++discarded_tiles;
				continue;
			} else {
				resize_asked = true;
				int ret = g_gui.PopupDialog("Collision", "The imported tiles are outside the current map scope. Do you want to resize the map? (Else additional tiles will be removed)", wxYES | wxNO);

				if(ret == wxID_YES) {
					// ...
					resizemap = true;
				} else {
					++discarded_tiles;
					continue;
				}
			}
		}

		if(new_pos.x > newsize_x) {
			newsize_x = new_pos.x;
		}
		if(new_pos.y > newsize_y) {
			newsize_y = new_pos.y;
		}

		imported_map.setTile(import_tile->getPosition(), nullptr);
		TileLocation* location = map.createTileL(new_pos);


		// Check if we should update any houses
		int new_houseid = house_id_map[import_tile->getHouseID()];
		House* house = map.houses.getHouse(new_houseid);
		if(import_tile->isHouseTile() && house_import_type != IMPORT_DONT && house) {
			// We need to notify houses of the tile moving
			house->removeTile(import_tile);
			import_tile->setLocation(location);
			house->addTile(import_tile);
		} else {
			import_tile->setLocation(location);
		}

		if(offset != Position(0,0,0)) {
			for(ItemVector::iterator iter = import_tile->items.begin(); iter != import_tile->items.end(); ++iter) {
				Item* item = *iter;
				if(Teleport* teleport = dynamic_cast<Teleport*>(item)) {
					teleport->setDestination(teleport->getDestination() + offset);
				}
			}
		}

		Tile* old_tile = map.getTile(new_pos);
		if(old_tile) {
			map.removeSpawn(old_tile);
		}
		import_tile->spawn = nullptr;

		map.setTile(new_pos, import_tile, true);
	}

	for(std::map<Position, Spawn*>::iterator spawn_iter = spawn_map.begin(); spawn_iter != spawn_map.end(); ++spawn_iter) {
		Position pos = spawn_iter->first;
		TileLocation* location = map.createTileL(pos);
		Tile* tile = location->get();
		if(!tile) {
			tile = map.allocator(location);
			map.setTile(pos, tile);
		} else if(tile->spawn) {
			map.removeSpawnInternal(tile);
			delete tile->spawn;
		}
		tile->spawn = spawn_iter->second;

		map.addSpawn(tile);
	}

	g_gui.DestroyLoadBar();

	map.setWidth(newsize_x);
	map.setHeight(newsize_y);
	g_gui.PopupDialog("Success", "Map imported successfully, " + i2ws(discarded_tiles) + " tiles were discarded as invalid.", wxOK);

	g_gui.RefreshPalettes();
	g_gui.FitViewToMap();

	return true;
}

void Editor::borderizeSelection()
{
	if(selection.size() == 0) {
		g_gui.SetStatusText("No items selected. Can't borderize.");
	}

	Action* action = actionQueue->createAction(ACTION_BORDERIZE);
	for(Tile* tile : selection) {
		Tile* newTile = tile->deepCopy(map);
		newTile->borderize(&map);
		newTile->select();
		action->addChange(newd Change(newTile));
	}
	addAction(action);
}

void Editor::borderizeMap(bool showdialog)
{
	if(showdialog) {
		g_gui.CreateLoadBar("Borderizing map...");
	}

	uint64_t tiles_done = 0;
	for(TileLocation* tileLocation : map) {
		if(showdialog && tiles_done % 4096 == 0) {
			g_gui.SetLoadDone(static_cast<int32_t>(tiles_done / double(map.tilecount) * 100.0));
		}

		Tile* tile = tileLocation->get();
		ASSERT(tile);

		tile->borderize(&map);
		++tiles_done;
	}

	if(showdialog) {
		g_gui.DestroyLoadBar();
	}
}

void Editor::randomizeSelection()
{
	if(selection.size() == 0) {
		g_gui.SetStatusText("No items selected. Can't randomize.");
	}

	Action* action = actionQueue->createAction(ACTION_RANDOMIZE);
	for(Tile* tile : selection) {
		Tile* newTile = tile->deepCopy(map);
		GroundBrush* groundBrush = newTile->getGroundBrush();
		if(groundBrush && groundBrush->isReRandomizable()) {
			groundBrush->draw(&map, newTile, nullptr);

			Item* oldGround = tile->ground;
			Item* newGround = newTile->ground;
			if(oldGround && newGround) {
				newGround->setActionID(oldGround->getActionID());
				newGround->setUniqueID(oldGround->getUniqueID());
			}

			newTile->select();
			action->addChange(newd Change(newTile));
		}
	}
	addAction(action);
}

void Editor::randomizeMap(bool showdialog)
{
	if(showdialog) {
		g_gui.CreateLoadBar("Randomizing map...");
	}

	uint64_t tiles_done = 0;
	for(TileLocation* tileLocation : map) {
		if(showdialog && tiles_done % 4096 == 0) {
			g_gui.SetLoadDone(static_cast<int32_t>(tiles_done / double(map.tilecount) * 100.0));
		}

		Tile* tile = tileLocation->get();
		ASSERT(tile);

		GroundBrush* groundBrush = tile->getGroundBrush();
		if(groundBrush) {
			Item* oldGround = tile->ground;

			uint16_t actionId, uniqueId;
			if(oldGround) {
				actionId = oldGround->getActionID();
				uniqueId = oldGround->getUniqueID();
			} else {
				actionId = 0;
				uniqueId = 0;
			}
			groundBrush->draw(&map, tile, nullptr);

			Item* newGround = tile->ground;
			if(newGround) {
				newGround->setActionID(actionId);
				newGround->setUniqueID(uniqueId);
			}
			tile->update();
		}
		++tiles_done;
	}

	if(showdialog) {
		g_gui.DestroyLoadBar();
	}
}

void Editor::clearInvalidHouseTiles(bool showdialog)
{
	if(showdialog) {
		g_gui.CreateLoadBar("Clearing invalid house tiles...");
	}

	Houses& houses = map.houses;

	HouseMap::iterator iter = houses.begin();
	while(iter != houses.end()) {
		House* h = iter->second;
		if(map.towns.getTown(h->townid) == nullptr) {
#ifdef __VISUALC__ // C++0x compliance to some degree :)
			iter = houses.erase(iter);
#else // Bulky, slow way
			HouseMap::iterator tmp_iter = iter;
			++tmp_iter;
			uint32_t next_key = 0;
			if(tmp_iter != houses.end()) {
				next_key = tmp_iter->first;
			}
			houses.erase(iter);
			if(next_key != 0) {
				iter = houses.find(next_key);
			} else {
				iter = houses.end();
			}
#endif
		} else {
			++iter;
		}
	}

	uint64_t tiles_done = 0;
	for(MapIterator map_iter = map.begin(); map_iter != map.end(); ++map_iter) {
		if(showdialog && tiles_done % 4096 == 0) {
			g_gui.SetLoadDone(int(tiles_done / double(map.tilecount) * 100.0));
		}

		Tile* tile = (*map_iter)->get();
		ASSERT(tile);
		if(tile->isHouseTile()) {
			if(houses.getHouse(tile->getHouseID()) == nullptr) {
				tile->setHouse(nullptr);
			}
		}
		++tiles_done;
	}

	if(showdialog) {
		g_gui.DestroyLoadBar();
	}
}

void Editor::clearModifiedTileState(bool showdialog)
{
	if(showdialog) {
		g_gui.CreateLoadBar("Clearing modified state from all tiles...");
	}

	uint64_t tiles_done = 0;
	for(MapIterator map_iter = map.begin(); map_iter != map.end(); ++map_iter) {
		if(showdialog && tiles_done % 4096 == 0) {
			g_gui.SetLoadDone(int(tiles_done / double(map.tilecount) * 100.0));
		}

		Tile* tile = (*map_iter)->get();
		ASSERT(tile);
		tile->unmodify();
		++tiles_done;
	}

	if(showdialog) {
		g_gui.DestroyLoadBar();
	}
}

void Editor::moveSelection(Position offset)
{
	BatchAction* batchAction = actionQueue->createBatch(ACTION_MOVE); // Our saved action batch, for undo!
	Action* action;

	// Remove tiles from the map
	action = actionQueue->createAction(batchAction); // Our action!
	bool doborders = false;
	TileSet tmp_storage;

	// Update the tiles with the newd positions
	for(TileSet::iterator it = selection.begin(); it != selection.end(); ++it) {
		// First we get the old tile and it's position
		Tile* tile = (*it);
		//const Position pos = tile->getPosition();

		// Create the duplicate source tile, which will replace the old one later
		Tile* old_src_tile = tile;
		Tile* new_src_tile;

		new_src_tile = old_src_tile->deepCopy(map);

		Tile* tmp_storage_tile = map.allocator(tile->getLocation());

		// Get all the selected items from the NEW source tile and iterate through them
		// This transfers ownership to the temporary tile
		ItemVector tile_selection = new_src_tile->popSelectedItems();
		for(ItemVector::iterator iit = tile_selection.begin(); iit != tile_selection.end(); iit++) {
			// Add the copied item to the newd destination tile,
			Item* item = (*iit);
			tmp_storage_tile->addItem(item);
		}
		// Move spawns
		if(new_src_tile->spawn && new_src_tile->spawn->isSelected()) {
			tmp_storage_tile->spawn = new_src_tile->spawn;
			new_src_tile->spawn = nullptr;
		}
		// Move creatures
		if(new_src_tile->creature && new_src_tile->creature->isSelected()) {
			tmp_storage_tile->creature = new_src_tile->creature;
			new_src_tile->creature = nullptr;
		}

		// Move house data & tile status if ground is transferred
		if(tmp_storage_tile->ground) {
			tmp_storage_tile->house_id = new_src_tile->house_id;
			new_src_tile->house_id = 0;
			tmp_storage_tile->setMapFlags(new_src_tile->getMapFlags());
			new_src_tile->setMapFlags(TILESTATE_NONE);
			doborders = true;
		}

		tmp_storage.insert(tmp_storage_tile);
		// Add the tile copy to the action
		action->addChange(newd Change(new_src_tile));
	}
	// Commit changes to map
	batchAction->addAndCommitAction(action);

	// Remove old borders (and create some newd?)
	if(g_settings.getInteger(Config::USE_AUTOMAGIC) &&
			g_settings.getInteger(Config::BORDERIZE_DRAG) &&
			selection.size() < size_t(g_settings.getInteger(Config::BORDERIZE_DRAG_THRESHOLD))) {
		action = actionQueue->createAction(batchAction);
		TileList borderize_tiles;
		// Go through all modified (selected) tiles (might be slow)
		for(TileSet::iterator it = tmp_storage.begin(); it != tmp_storage.end(); ++it) {
			const Position& pos = (*it)->getPosition();
			// Go through all neighbours
			Tile* t;
			t = map.getTile(pos.x  , pos.y  , pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); }
			t = map.getTile(pos.x-1, pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); }
			t = map.getTile(pos.x  , pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); }
			t = map.getTile(pos.x+1, pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); }
			t = map.getTile(pos.x-1, pos.y  , pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); }
			t = map.getTile(pos.x+1, pos.y  , pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); }
			t = map.getTile(pos.x-1, pos.y+1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); }
			t = map.getTile(pos.x  , pos.y+1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); }
			t = map.getTile(pos.x+1, pos.y+1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); }
		}
		// Remove duplicates
		borderize_tiles.sort();
		borderize_tiles.unique();
		// Do le borders!
		for(TileList::iterator it = borderize_tiles.begin(); it != borderize_tiles.end(); ++it) {
			Tile* tile = *it;
			Tile* new_tile = (*it)->deepCopy(map);
			if(doborders) new_tile->borderize(&map);
			new_tile->wallize(&map);
			new_tile->tableize(&map);
			new_tile->carpetize(&map);
			if(tile->ground && tile->ground->isSelected()) new_tile->selectGround();
			action->addChange(newd Change(new_tile));
		}
		// Commit changes to map
		batchAction->addAndCommitAction(action);
	}

	// New action for adding the destination tiles
	action = actionQueue->createAction(batchAction);
	for(TileSet::iterator it = tmp_storage.begin(); it != tmp_storage.end(); ++it) {
		Tile* tile = (*it);
		const Position old_pos = tile->getPosition();
		Position new_pos;

		new_pos = old_pos - offset;

		if(new_pos.z < 0 && new_pos.z > MAP_MAX_LAYER) {
			delete tile;
			continue;
		}
		// Create the duplicate dest tile, which will replace the old one later
		TileLocation* location = map.createTileL(new_pos);
		Tile* old_dest_tile = location->get();
		Tile* new_dest_tile = nullptr;

		if(g_settings.getInteger(Config::MERGE_MOVE) || !tile->ground) {
			// Move items
			if(old_dest_tile) {
				new_dest_tile = old_dest_tile->deepCopy(map);
			} else {
				new_dest_tile = map.allocator(location);
			}
			new_dest_tile->merge(tile);
			delete tile;
		} else {
			// Replace tile instead of just merge
			tile->setLocation(location);
			new_dest_tile = tile;
		}

		action->addChange(newd Change(new_dest_tile));
	}

	// Commit changes to the map
	batchAction->addAndCommitAction(action);

	// Create borders
	if(g_settings.getInteger(Config::USE_AUTOMAGIC) &&
			g_settings.getInteger(Config::BORDERIZE_DRAG) &&
			selection.size() < size_t(g_settings.getInteger(Config::BORDERIZE_DRAG_THRESHOLD))) {
		action = actionQueue->createAction(batchAction);
		TileList borderize_tiles;
		// Go through all modified (selected) tiles (might be slow)
		for(TileSet::iterator it = selection.begin(); it != selection.end(); it++) {
			bool add_me = false; // If this tile is touched
			const Position& pos = (*it)->getPosition();
			// Go through all neighbours
			Tile* t;
			t = map.getTile(pos.x-1, pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x-1, pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x  , pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x+1, pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x-1, pos.y  , pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x+1, pos.y  , pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x-1, pos.y+1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x  , pos.y+1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true; }
			t = map.getTile(pos.x+1, pos.y+1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true; }
			if(add_me) borderize_tiles.push_back(*it);
		}
		// Remove duplicates
		borderize_tiles.sort();
		borderize_tiles.unique();
		// Do le borders!
		for(TileList::iterator it = borderize_tiles.begin(); it != borderize_tiles.end(); it++) {
			Tile* tile = *it;
			if(tile->ground) {
				if(tile->ground->getGroundBrush()) {
					Tile* new_tile = tile->deepCopy(map);

					if(doborders)
						new_tile->borderize(&map);

					new_tile->wallize(&map);
					new_tile->tableize(&map);
					new_tile->carpetize(&map);
					if(tile->ground->isSelected())
						new_tile->selectGround();

					action->addChange(newd Change(new_tile));
				}
			}
		}
		// Commit changes to map
		batchAction->addAndCommitAction(action);
	}

	// Store the action for undo
	addBatch(batchAction);
	selection.updateSelectionCount();
}

void Editor::destroySelection()
{
	if(selection.size() == 0) {
		g_gui.SetStatusText("No selected items to delete.");
	} else {
		int tile_count = 0;
		int item_count = 0;
		PositionList tilestoborder;

		BatchAction* batch = actionQueue->createBatch(ACTION_DELETE_TILES);
		Action* action = actionQueue->createAction(batch);

		for(TileSet::iterator it = selection.begin(); it != selection.end(); ++it) {
			tile_count++;

			Tile* tile = *it;
			Tile* newtile = tile->deepCopy(map);

			ItemVector tile_selection = newtile->popSelectedItems();
			for(ItemVector::iterator iit = tile_selection.begin(); iit != tile_selection.end(); ++iit) {
				++item_count;
				// Delete the items from the tile
				delete *iit;
			}

			if(newtile->creature && newtile->creature->isSelected()) {
				delete newtile->creature;
				newtile->creature = nullptr;
			}

			if(newtile->spawn && newtile->spawn->isSelected()) {
				delete newtile->spawn;
				newtile->spawn = nullptr;
			}

			if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
				for(int y = -1; y <= 1; y++) {
					for(int x = -1; x <= 1; x++) {
						const Position& position = tile->getPosition();
						tilestoborder.push_back(Position(position.x + x, position.y + y, position.z));
					}
				}
			}
			action->addChange(newd Change(newtile));
		}

		batch->addAndCommitAction(action);

		if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
			// Remove duplicates
			tilestoborder.sort();
			tilestoborder.unique();

			action = actionQueue->createAction(batch);
			for(PositionList::iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
				TileLocation* location = map.createTileL(*it);
				Tile* tile = location->get();

				if(tile) {
					Tile* new_tile = tile->deepCopy(map);
					new_tile->borderize(&map);
					new_tile->wallize(&map);
					new_tile->tableize(&map);
					new_tile->carpetize(&map);
					action->addChange(newd Change(new_tile));
				} else {
					Tile* new_tile = map.allocator(location);
					new_tile->borderize(&map);
					if(new_tile->size()) {
						action->addChange(newd Change(new_tile));
					} else {
						delete new_tile;
					}
				}
			}

			batch->addAndCommitAction(action);
		}

		addBatch(batch);
		wxString ss;
		ss << "Deleted " << tile_count << " tile" << (tile_count > 1 ? "s" : "") <<  " (" << item_count << " item" << (item_count > 1? "s" : "") << ")";
		g_gui.SetStatusText(ss);
	}
}

	// Macro to avoid useless code repetition
void doSurroundingBorders(DoodadBrush* doodad_brush, PositionList& tilestoborder, Tile* buffer_tile, Tile* new_tile)
{
	if(doodad_brush->doNewBorders() && g_settings.getInteger(Config::USE_AUTOMAGIC)) {
		const Position& position = new_tile->getPosition();
		tilestoborder.push_back(Position(position));
		if(buffer_tile->hasGround()) {
			for(int y = -1; y <= 1; y++) {
				for(int x = -1; x <= 1; x++) {
					tilestoborder.push_back(Position(position.x + x, position.y + y, position.z));
				}
			}
		} else if(buffer_tile->hasWall()) {
			tilestoborder.push_back(Position(position.x, position.y-1, position.z));
			tilestoborder.push_back(Position(position.x-1, position.y, position.z));
			tilestoborder.push_back(Position(position.x+1, position.y, position.z));
			tilestoborder.push_back(Position(position.x, position.y+1, position.z));
		}
	}
}

void removeDuplicateWalls(Tile* buffer, Tile* tile)
{
	for(ItemVector::const_iterator iter = buffer->items.begin(); iter != buffer->items.end(); ++iter) {
		if((*iter)->getWallBrush()) {
			tile->cleanWalls((*iter)->getWallBrush());
		}
	}
}

void Editor::drawInternal(Position offset, bool alt, bool dodraw)
{
	Brush* brush = g_gui.GetCurrentBrush();
	if(!brush)
		return;

	if(brush->isDoodad()) {
		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);
		BaseMap* buffer_map = g_gui.doodad_buffer_map;

		Position delta_pos = offset - Position(0x8000, 0x8000, 0x8);
		PositionList tilestoborder;

		for(MapIterator it = buffer_map->begin(); it != buffer_map->end(); ++it) {
			Tile* buffer_tile = (*it)->get();
			Position pos = buffer_tile->getPosition() + delta_pos;
			if(!pos.isValid()) {
				continue;
			}

			TileLocation* location = map.createTileL(pos);
			Tile* tile = location->get();
			DoodadBrush* doodad_brush = brush->asDoodad();

			if(doodad_brush->placeOnBlocking() || alt) {
				if(tile) {
					bool place = true;
					if(!doodad_brush->placeOnDuplicate() && !alt) {
						for(ItemVector::const_iterator iter = tile->items.begin(); iter != tile->items.end(); ++iter) {
							if(doodad_brush->ownsItem(*iter)) {
								place = false;
								break;
							}
						}
					}
					if(place) {
						Tile* new_tile = tile->deepCopy(map);
						removeDuplicateWalls(buffer_tile, new_tile);
						doSurroundingBorders(doodad_brush, tilestoborder, buffer_tile, new_tile);
						new_tile->merge(buffer_tile);
						action->addChange(newd Change(new_tile));
					}
				} else {
					Tile* new_tile = map.allocator(location);
					removeDuplicateWalls(buffer_tile, new_tile);
					doSurroundingBorders(doodad_brush, tilestoborder, buffer_tile, new_tile);
					new_tile->merge(buffer_tile);
					action->addChange(newd Change(new_tile));
				}
			} else {
				if(tile && !tile->isBlocking()) {
					bool place = true;
					if(!doodad_brush->placeOnDuplicate() && !alt) {
						for(ItemVector::const_iterator iter = tile->items.begin(); iter != tile->items.end(); ++iter) {
							if(doodad_brush->ownsItem(*iter)) {
								place = false;
								break;
							}
						}
					}
					if(place) {
						Tile* new_tile = tile->deepCopy(map);
						removeDuplicateWalls(buffer_tile, new_tile);
						doSurroundingBorders(doodad_brush, tilestoborder, buffer_tile, new_tile);
						new_tile->merge(buffer_tile);
						action->addChange(newd Change(new_tile));
					}
				}
			}
		}
		batch->addAndCommitAction(action);

		if(tilestoborder.size() > 0) {
			Action* action = actionQueue->createAction(batch);

			// Remove duplicates
			tilestoborder.sort();
			tilestoborder.unique();

			for(PositionList::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
				Tile* tile = map.getTile(*it);
				if(tile) {
					Tile* new_tile = tile->deepCopy(map);
					new_tile->borderize(&map);
					new_tile->wallize(&map);
					action->addChange(newd Change(new_tile));
				}
			}
			batch->addAndCommitAction(action);
		}
		addBatch(batch, 2);
	} else if(brush->isHouseExit()) {
		HouseExitBrush* house_exit_brush = brush->asHouseExit();
		if(!house_exit_brush->canDraw(&map, offset))
			return;

		House* house = map.houses.getHouse(house_exit_brush->getHouseID());
		if(!house)
			return;

		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);
		action->addChange(Change::Create(house, offset));
		batch->addAndCommitAction(action);
		addBatch(batch, 2);
	} else if(brush->isWaypoint()) {
		WaypointBrush* waypoint_brush = brush->asWaypoint();
		if(!waypoint_brush->canDraw(&map, offset))
			return;

		Waypoint* waypoint = map.waypoints.getWaypoint(waypoint_brush->getWaypoint());
		if(!waypoint || waypoint->pos == offset)
			return;

		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);
		action->addChange(Change::Create(waypoint, offset));
		batch->addAndCommitAction(action);
		addBatch(batch, 2);
	} else if(brush->isWall()) {
		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);
		// This will only occur with a size 0, when clicking on a tile (not drawing)
		Tile* tile = map.getTile(offset);
		Tile* new_tile = nullptr;
		if(tile) {
			new_tile = tile->deepCopy(map);
		} else {
			new_tile = map.allocator(map.createTileL(offset));
		}

		if(dodraw) {
			bool b = true;
			brush->asWall()->draw(&map, new_tile, &b);
		} else {
			brush->asWall()->undraw(&map, new_tile);
		}
		action->addChange(newd Change(new_tile));
		batch->addAndCommitAction(action);
		addBatch(batch, 2);
	} else if(brush->isSpawn() || brush->isCreature()) {
		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);

		Tile* tile = map.getTile(offset);
		Tile* new_tile = nullptr;
		if(tile) {
			new_tile = tile->deepCopy(map);
		} else {
			new_tile = map.allocator(map.createTileL(offset));
		}
		int param;
		if(!brush->isCreature()) {
			param = g_gui.GetBrushSize();
		}
		if(dodraw) {
			brush->draw(&map, new_tile, &param);
		} else {
			brush->undraw(&map, new_tile);
		}
		action->addChange(newd Change(new_tile));
		batch->addAndCommitAction(action);
		addBatch(batch, 2);
	}
}

void Editor::drawInternal(const PositionVector& tilestodraw, bool alt, bool dodraw)
{
	Brush* brush = g_gui.GetCurrentBrush();
	if(!brush)
		return;

#ifdef __DEBUG__
	if(brush->isGround() || brush->isWall()) {
		// Wrong function, end call
		return;
	}
#endif

	Action* action = actionQueue->createAction(ACTION_DRAW);

	if(brush->isOptionalBorder()) {
		// We actually need to do borders, but on the same tiles we draw to
		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = map.createTileL(*it);
			Tile* tile = location->get();
			if(tile) {
				if(dodraw) {
					Tile* new_tile = tile->deepCopy(map);
					brush->draw(&map, new_tile);
					new_tile->borderize(&map);
					action->addChange(newd Change(new_tile));
				} else if(!dodraw && tile->hasOptionalBorder()) {
					Tile* new_tile = tile->deepCopy(map);
					brush->undraw(&map, new_tile);
					new_tile->borderize(&map);
					action->addChange(newd Change(new_tile));
				}
			} else if(dodraw) {
				Tile* new_tile = map.allocator(location);
				brush->draw(&map, new_tile);
				new_tile->borderize(&map);
				if(new_tile->size() == 0) {
					delete new_tile;
					continue;
				}
				action->addChange(newd Change(new_tile));
			}
		}
	} else {

		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = map.createTileL(*it);
			Tile* tile = location->get();
			if(tile) {
				Tile* new_tile = tile->deepCopy(map);
				if(dodraw) {
					brush->draw(&map, new_tile, &alt);
				} else {
					brush->undraw(&map, new_tile);
				}
				action->addChange(newd Change(new_tile));
			} else if(dodraw) {
				Tile* new_tile = map.allocator(location);
				brush->draw(&map, new_tile, &alt);
				action->addChange(newd Change(new_tile));
			}
		}
	}
	addAction(action, 2);
}


void Editor::drawInternal(const PositionVector& tilestodraw, PositionVector& tilestoborder, bool alt, bool dodraw)
{
	Brush* brush = g_gui.GetCurrentBrush();
	if(!brush)
		return;

	if(brush->isGround() || brush->isEraser()) {
		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);

		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = map.createTileL(*it);
			Tile* tile = location->get();
			if(tile) {
				Tile* new_tile = tile->deepCopy(map);
				if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
					new_tile->cleanBorders();
				}
				if(dodraw)
					if(brush->isGround() && alt) {
						std::pair<bool, GroundBrush*> param;
						if(replace_brush) {
							param.first = false;
							param.second = replace_brush;
						} else {
							param.first = true;
							param.second = nullptr;
						}
						g_gui.GetCurrentBrush()->draw(&map, new_tile, &param);
					} else {
						g_gui.GetCurrentBrush()->draw(&map, new_tile, nullptr);
					}
				else {
					g_gui.GetCurrentBrush()->undraw(&map, new_tile);
					tilestoborder.push_back(*it);
				}
				action->addChange(newd Change(new_tile));
			} else if(dodraw) {
				Tile* new_tile = map.allocator(location);
				if(brush->isGround() && alt) {
					std::pair<bool, GroundBrush*> param;
					if(replace_brush) {
						param.first = false;
						param.second = replace_brush;
					} else {
						param.first = true;
						param.second = nullptr;
					}
					g_gui.GetCurrentBrush()->draw(&map, new_tile, &param);
				} else {
					g_gui.GetCurrentBrush()->draw(&map, new_tile, nullptr);
				}
				action->addChange(newd Change(new_tile));
			}
		}

		// Commit changes to map
		batch->addAndCommitAction(action);

		if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
			// Do borders!
			action = actionQueue->createAction(batch);
			for(PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
				TileLocation* location = map.createTileL(*it);
				Tile* tile = location->get();
				if(tile) {
					Tile* new_tile = tile->deepCopy(map);
					if(brush->isEraser()) {
						new_tile->wallize(&map);
						new_tile->tableize(&map);
						new_tile->carpetize(&map);
					}
					new_tile->borderize(&map);
					action->addChange(newd Change(new_tile));
				} else {
					Tile* new_tile = map.allocator(location);
					if(brush->isEraser()) {
						// There are no carpets/tables/walls on empty tiles...
						//new_tile->wallize(map);
						//new_tile->tableize(map);
						//new_tile->carpetize(map);
					}
					new_tile->borderize(&map);
					if(new_tile->size() > 0) {
						action->addChange(newd Change(new_tile));
					} else {
						delete new_tile;
					}
				}
			}
			batch->addAndCommitAction(action);
		}

		addBatch(batch, 2);
	} else if(brush->isTable() || brush->isCarpet()) {
		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);

		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = map.createTileL(*it);
			Tile* tile = location->get();
			if(tile) {
				Tile* new_tile = tile->deepCopy(map);
				if(dodraw)
					g_gui.GetCurrentBrush()->draw(&map, new_tile, nullptr);
				else
					g_gui.GetCurrentBrush()->undraw(&map, new_tile);
				action->addChange(newd Change(new_tile));
			} else if(dodraw) {
				Tile* new_tile = map.allocator(location);
				g_gui.GetCurrentBrush()->draw(&map, new_tile, nullptr);
				action->addChange(newd Change(new_tile));
			}
		}

		// Commit changes to map
		batch->addAndCommitAction(action);

		// Do borders!
		action = actionQueue->createAction(batch);
		for(PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
			Tile* tile = map.getTile(*it);
			if(brush->isTable()) {
				if(tile && tile->hasTable()) {
					Tile* new_tile = tile->deepCopy(map);
					new_tile->tableize(&map);
					action->addChange(newd Change(new_tile));
				}
			} else if(brush->isCarpet()) {
				if(tile && tile->hasCarpet()) {
					Tile* new_tile = tile->deepCopy(map);
					new_tile->carpetize(&map);
					action->addChange(newd Change(new_tile));
				}
			}
		}
		batch->addAndCommitAction(action);

		addBatch(batch, 2);
	} else if(brush->isWall()) {
		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);

		if(alt && dodraw) {
			// This is exempt from USE_AUTOMAGIC
			g_gui.doodad_buffer_map->clear();
			BaseMap* draw_map = g_gui.doodad_buffer_map;

			for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
				TileLocation* location = map.createTileL(*it);
				Tile* tile = location->get();
				if(tile) {
					Tile* new_tile = tile->deepCopy(map);
					new_tile->cleanWalls(brush->isWall());
					g_gui.GetCurrentBrush()->draw(draw_map, new_tile);
					draw_map->setTile(*it, new_tile, true);
				} else if(dodraw) {
					Tile* new_tile = map.allocator(location);
					g_gui.GetCurrentBrush()->draw(draw_map, new_tile);
					draw_map->setTile(*it, new_tile, true);
				}
			}
			for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
				// Get the correct tiles from the draw map instead of the editor map
				Tile* tile = draw_map->getTile(*it);
				if(tile) {
					tile->wallize(draw_map);
					action->addChange(newd Change(tile));
				}
			}
			draw_map->clear(false);
			// Commit
			batch->addAndCommitAction(action);
		} else {
			for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
				TileLocation* location = map.createTileL(*it);
				Tile* tile = location->get();
				if(tile) {
					Tile* new_tile = tile->deepCopy(map);
					// Wall cleaning is exempt from automagic
					new_tile->cleanWalls(brush->isWall());
					if(dodraw)
						g_gui.GetCurrentBrush()->draw(&map, new_tile);
					else
						g_gui.GetCurrentBrush()->undraw(&map, new_tile);
					action->addChange(newd Change(new_tile));
				} else if(dodraw) {
					Tile* new_tile = map.allocator(location);
					g_gui.GetCurrentBrush()->draw(&map, new_tile);
					action->addChange(newd Change(new_tile));
				}
			}

			// Commit changes to map
			batch->addAndCommitAction(action);

			if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
				// Do borders!
				action = actionQueue->createAction(batch);
				for(PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
					Tile* tile = map.getTile(*it);
					if(tile) {
						Tile* new_tile = tile->deepCopy(map);
						new_tile->wallize(&map);
						//if(*tile == *new_tile) delete new_tile;
						action->addChange(newd Change(new_tile));
					}
				}
				batch->addAndCommitAction(action);
			}
		}

		actionQueue->addBatch(batch, 2);
	} else if(brush->isDoor()) {
		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);
		DoorBrush* door_brush = brush->asDoor();

		// Loop is kind of redundant since there will only ever be one index.
		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = map.createTileL(*it);
			Tile* tile = location->get();
			if(tile) {
				Tile* new_tile = tile->deepCopy(map);
				// Wall cleaning is exempt from automagic
				if(brush->isWall())
					new_tile->cleanWalls(brush->asWall());
				if(dodraw)
					door_brush->draw(&map, new_tile, &alt);
				else
					door_brush->undraw(&map, new_tile);
				action->addChange(newd Change(new_tile));
			} else if(dodraw) {
				Tile* new_tile = map.allocator(location);
				door_brush->draw(&map, new_tile, &alt);
				action->addChange(newd Change(new_tile));
			}
		}

		// Commit changes to map
		batch->addAndCommitAction(action);

		if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
			// Do borders!
			action = actionQueue->createAction(batch);
			for(PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
				Tile* tile = map.getTile(*it);
				if(tile) {
					Tile* new_tile = tile->deepCopy(map);
					new_tile->wallize(&map);
					//if(*tile == *new_tile) delete new_tile;
					action->addChange(newd Change(new_tile));
				}
			}
			batch->addAndCommitAction(action);
		}

		addBatch(batch, 2);
	} else {
		Action* action = actionQueue->createAction(ACTION_DRAW);
		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = map.createTileL(*it);
			Tile* tile = location->get();
			if(tile) {
				Tile* new_tile = tile->deepCopy(map);
				if(dodraw)
					g_gui.GetCurrentBrush()->draw(&map, new_tile);
				else
					g_gui.GetCurrentBrush()->undraw(&map, new_tile);
				action->addChange(newd Change(new_tile));
			} else if(dodraw) {
				Tile* new_tile = map.allocator(location);
				g_gui.GetCurrentBrush()->draw(&map, new_tile);
				action->addChange(newd Change(new_tile));
			}
		}
		addAction(action, 2);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Live!

bool Editor::IsLiveClient() const
{
	return live_client != nullptr;
}

bool Editor::IsLiveServer() const
{
	return live_server != nullptr;
}

bool Editor::IsLive() const
{
	return IsLiveClient() || IsLiveServer();
}

bool Editor::IsLocal() const
{
	return !IsLive();
}

LiveClient* Editor::GetLiveClient() const
{
	return live_client;
}

LiveServer* Editor::GetLiveServer() const
{
	return live_server;
}

LiveSocket& Editor::GetLive() const
{
	if(live_server)
		return *live_server;
	return *live_client;
}

LiveServer* Editor::StartLiveServer()
{
	ASSERT(IsLocal());
	live_server = newd LiveServer(*this);

	delete actionQueue;
	actionQueue = newd NetworkedActionQueue(*this);

	return live_server;
}

void Editor::BroadcastNodes(DirtyList& dirtyList)
{
	if(IsLiveClient()) {
		live_client->sendChanges(dirtyList);
	} else {
		live_server->broadcastNodes(dirtyList);
	}
}

void Editor::CloseLiveServer()
{
	ASSERT(IsLive());
	if(live_client) {
		live_client->close();

		delete live_client;
		live_client = nullptr;
	}

	if(live_server) {
		live_server->close();

		delete live_server;
		live_server = nullptr;

		delete actionQueue;
		actionQueue = newd ActionQueue(*this);
	}

	NetworkConnection& connection = NetworkConnection::getInstance();
	connection.stop();
}

void Editor::QueryNode(int ndx, int ndy, bool underground)
{
	ASSERT(live_client);
	live_client->queryNode(ndx, ndy, underground);
}

void Editor::SendNodeRequests()
{
	if(live_client) {
		live_client->sendNodeRequests();
	}
}

