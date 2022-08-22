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
	m_actionQueue(newd ActionQueue(*this)),
	m_selection(*this),
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
	m_map.convert(version);

	static int unnamed_counter = 0;
	std::string tmp_name = "Untitled-" + i2s(++unnamed_counter);

	m_map.setWidth(2048);
	m_map.setHeight(2048);
	m_map.setName(tmp_name + ".otbm");
	m_map.setSpawnFile(tmp_name + "-spawn.xml");
	m_map.setHouseFile(tmp_name + "-house.xml");
	m_map.setDescription("No map description available.");
	m_map.setUnnamed(true);
	m_map.doChange();
}

Editor::Editor(CopyBuffer& copybuffer, const FileName& fn) :
	live_server(nullptr),
	live_client(nullptr),
	m_actionQueue(newd ActionQueue(*this)),
	m_selection(*this),
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
		success = m_map.open(nstr(fn.GetFullPath()));
		/* TODO
		if(success && ver.client == CLIENT_VERSION_854_BAD) {
			int ok = g_gui.PopupDialog("Incorrect OTB", "This map has been saved with an incorrect OTB version, do you want to convert it to the new OTB version?\n\nIf you are not sure, click Yes.", wxYES | wxNO);

			if(ok == wxID_YES){
				ver.client = CLIENT_VERSION_854;
				m_map.convert(ver);
			}
		}
		*/
	}
}

Editor::Editor(CopyBuffer& copybuffer, LiveClient* client) :
	live_server(nullptr),
	live_client(client),
	m_actionQueue(newd NetworkedActionQueue(*this)),
	m_selection(*this),
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
	m_selection.clear();
	delete m_actionQueue;
}

void Editor::addBatch(BatchAction* action, int stacking_delay)
{
	m_actionQueue->addBatch(action, stacking_delay);
	g_gui.UpdateMenus();
}

void Editor::addAction(Action* action, int stacking_delay )
{
	m_actionQueue->addAction(action, stacking_delay);
	g_gui.UpdateMenus();
}

void Editor::saveMap(FileName filename, bool showdialog)
{
	std::string savefile = filename.GetFullPath().mb_str(wxConvUTF8).data();
	bool save_as = false;
	bool save_otgz = false;

	if(savefile.empty()) {
		savefile = m_map.getFile();

		FileName c1(wxstr(savefile));
		FileName c2(wxstr(m_map.getFile()));
		save_as = c1 != c2;
	}

	// If not named yet, propagate the file name to the auxilliary files
	if(m_map.isUnnamed()) {
		FileName _name(filename);
		_name.SetExt("xml");

		_name.SetName(filename.GetName() + "-spawn");
		m_map.setSpawnFile(nstr(_name.GetFullName()));
		_name.SetName(filename.GetName() + "-house");
		m_map.setHouseFile(nstr(_name.GetFullName()));
		m_map.setUnnamed(false);
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

		converter.SetFullName(wxstr(m_map.getHouseFile()));
		if(converter.FileExists()) {
			backup_house = map_path + nstr(converter.GetName()) + ".xml~";
			std::remove(backup_house.c_str());
			std::rename((map_path + m_map.getHouseFile()).c_str(), backup_house.c_str());
		}

		converter.SetFullName(wxstr(m_map.getSpawnFile()));
		if(converter.FileExists()) {
			backup_spawn = map_path + nstr(converter.GetName()) + ".xml~";
			std::remove(backup_spawn.c_str());
			std::rename((map_path + m_map.getSpawnFile()).c_str(), backup_spawn.c_str());
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
		std::string map_file(fn.GetFullPath().mb_str(wxConvUTF8));
		std::string map_name(fn.GetFullName().mb_str(wxConvUTF8));
		m_map.setFile(map_file);
		m_map.setName(map_name);

		if(showdialog)
			g_gui.CreateLoadBar("Saving OTBM map...");

		// Perform the actual save
		IOMapOTBM mapsaver(m_map.getVersion());
		bool success = mapsaver.saveMap(m_map, fn);

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
				converter.SetFullName(wxstr(m_map.getHouseFile()));
				std::string house_filename = map_path + nstr(converter.GetName());
				std::rename(backup_house.c_str(), std::string(house_filename + ".xml").c_str());
			}

			if(!backup_spawn.empty()) {
				converter.SetFullName(wxstr(m_map.getSpawnFile()));
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
			converter.SetFullName(wxstr(m_map.getHouseFile()));
			std::string house_filename = map_path + nstr(converter.GetName());
			std::rename(backup_house.c_str(), std::string(house_filename + "." + date.str() + ".xml").c_str());
		}

		if(!backup_spawn.empty()) {
			converter.SetFullName(wxstr(m_map.getSpawnFile()));
			std::string spawn_filename = map_path + nstr(converter.GetName());
			std::rename(backup_spawn.c_str(), std::string(spawn_filename + "." + date.str() + ".xml").c_str());
		}
	} else {
		// Delete the temporary files
		std::remove(backup_otbm.c_str());
		std::remove(backup_house.c_str());
		std::remove(backup_spawn.c_str());
	}

	m_map.clearChanges();
}

bool Editor::importMiniMap(FileName filename, int import, int import_x_offset, int import_y_offset, int import_z_offset)
{
	return false;
}

bool Editor::exportMiniMap(FileName filename, int floor /*= GROUND_LAYER*/, bool displaydialog)
{
	return m_map.exportMinimap(filename, floor, displaydialog);
}

bool Editor::exportSelectionAsMiniMap(FileName directory, wxString fileName)
{
	if(!directory.Exists() || !directory.IsDirWritable())
		return false;

	int min_x = MAP_MAX_WIDTH + 1, min_y = MAP_MAX_HEIGHT + 1, min_z = MAP_MAX_LAYER + 1;
	int max_x = 0, max_y = 0, max_z = 0;

	const TileSet& tiles = m_selection.getTiles();
	for(Tile* tile : tiles) {
		if(tile->empty())
			continue;

		Position pos = tile->getPosition();

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
	m_selection.clear();
	m_actionQueue->clear();

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
	int newsize_x = m_map.getWidth(), newsize_y = m_map.getHeight();
	int discarded_tiles = 0;

	g_gui.CreateLoadBar("Merging maps...");

	std::map<uint32_t, uint32_t> town_id_map;
	std::map<uint32_t, uint32_t> house_id_map;

	if(house_import_type != IMPORT_DONT) {
		for(TownMap::iterator tit = imported_map.getTowns().begin(); tit != imported_map.getTowns().end();) {
			Town* imported_town = tit->second;
			Town* current_town = m_map.getTowns().getTown(imported_town->getID());

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
							uint32_t new_id = m_map.getTowns().getEmptyID();
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
					uint32_t new_id = m_map.getTowns().getEmptyID();
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

			m_map.getTowns().addTown(imported_town);

#ifdef __VISUALC__ // C++0x compliance to some degree :)
			tit = imported_map.getTowns().erase(tit);
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

		for(HouseMap::iterator hit = imported_map.getHouses().begin(); hit != imported_map.getHouses().end();) {
			House* imported_house = hit->second;
			House* current_house = m_map.getHouses().getHouse(imported_house->id);
			imported_house->townid = town_id_map[imported_house->townid];

			Position oldexit = imported_house->getExit();
			imported_house->setExit(nullptr, Position()); // Reset it

			switch(house_import_type) {
				case IMPORT_MERGE: {
					house_id_map[imported_house->id] = imported_house->id;
					if(current_house) {
						++hit;
						Position newexit = oldexit + offset;
						if(newexit.isValid()) current_house->setExit(&m_map, newexit);
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
							if(newexit.isValid()) imported_house->setExit(&m_map, newexit);
							continue;
						} else {
							// Conflict! Find a newd id and replace old
							uint32_t new_id = m_map.getHouses().getEmptyID();
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
					uint32_t new_id = m_map.getHouses().getEmptyID();
					house_id_map[imported_house->id] = new_id;
					imported_house->id = new_id;
					break;
				}
				case IMPORT_DONT: {
					++hit;
					Position newexit = oldexit + offset;
					if(newexit.isValid()) imported_house->setExit(&m_map, newexit);
						continue; // Should never happend..?
					break;
				}
			}

			Position newexit = oldexit + offset;
			if(newexit.isValid()) imported_house->setExit(&m_map, newexit);
			m_map.getHouses().addHouse(imported_house);

#ifdef __VISUALC__ // C++0x compliance to some degree :)
			hit = imported_map.getHouses().erase(hit);
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
		for(SpawnPositionList::iterator siter = imported_map.getSpawns().begin(); siter != imported_map.getSpawns().end();) {
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
						if(next == imported_map.getSpawns().end())
							cont = false;
						else
							next_spawn = *next;
						imported_map.getSpawns().erase(siter);
						if(cont)
							siter = imported_map.getSpawns().find(next_spawn);
						else
							siter = imported_map.getSpawns().end();
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
	for(WaypointMap::iterator iter = imported_map.getWaypoints().begin(); iter != imported_map.getWaypoints().end(); ++iter) {
		iter->second->pos += offset;
	}

	m_map.getWaypoints().waypoints.insert(imported_map.getWaypoints().begin(), imported_map.getWaypoints().end());
	imported_map.getWaypoints().waypoints.clear();


	uint64_t tiles_merged = 0;
	uint64_t tiles_to_import = imported_map.m_tileCount;
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

		if(!resizemap && (new_pos.x > m_map.getWidth() || new_pos.y > m_map.getHeight())) {
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
		TileLocation* location = m_map.createTileL(new_pos);


		// Check if we should update any houses
		int new_houseid = house_id_map[import_tile->getHouseID()];
		House* house = m_map.getHouses().getHouse(new_houseid);
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

		Tile* old_tile = m_map.getTile(new_pos);
		if(old_tile) {
			m_map.removeSpawn(old_tile);
		}
		import_tile->spawn = nullptr;

		m_map.setTile(new_pos, import_tile, true);
	}

	for(std::map<Position, Spawn*>::iterator spawn_iter = spawn_map.begin(); spawn_iter != spawn_map.end(); ++spawn_iter) {
		Position pos = spawn_iter->first;
		TileLocation* location = m_map.createTileL(pos);
		Tile* tile = location->get();
		if(!tile) {
			tile = m_map.allocator(location);
			m_map.setTile(pos, tile);
		} else if(tile->spawn) {
			m_map.removeSpawnInternal(tile);
			delete tile->spawn;
		}
		tile->spawn = spawn_iter->second;

		m_map.addSpawn(tile);
	}

	g_gui.DestroyLoadBar();

	m_map.setWidth(newsize_x);
	m_map.setHeight(newsize_y);
	g_gui.PopupDialog("Success", "Map imported successfully, " + i2ws(discarded_tiles) + " tiles were discarded as invalid.", wxOK);

	g_gui.RefreshPalettes();
	g_gui.FitViewToMap();

	return true;
}

void Editor::borderizeSelection()
{
	if(m_selection.size() == 0) {
		g_gui.SetStatusText("No items selected. Can't borderize.");
	}

	Action* action = m_actionQueue->createAction(ACTION_BORDERIZE);
	for(Tile* tile : m_selection) {
		Tile* newTile = tile->deepCopy(m_map);
		newTile->borderize(&m_map);
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
	for(TileLocation* tileLocation : m_map) {
		if(showdialog && tiles_done % 4096 == 0) {
			g_gui.SetLoadDone(static_cast<int32_t>(tiles_done / double(m_map.m_tileCount) * 100.0));
		}

		Tile* tile = tileLocation->get();
		ASSERT(tile);

		tile->borderize(&m_map);
		++tiles_done;
	}

	if(showdialog) {
		g_gui.DestroyLoadBar();
	}
}

void Editor::randomizeSelection()
{
	if(m_selection.size() == 0) {
		g_gui.SetStatusText("No items selected. Can't randomize.");
	}

	Action* action = m_actionQueue->createAction(ACTION_RANDOMIZE);
	for(Tile* tile : m_selection) {
		Tile* newTile = tile->deepCopy(m_map);
		GroundBrush* groundBrush = newTile->getGroundBrush();
		if(groundBrush && groundBrush->isReRandomizable()) {
			groundBrush->draw(&m_map, newTile, nullptr);

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
	for(TileLocation* tileLocation : m_map) {
		if(showdialog && tiles_done % 4096 == 0) {
			g_gui.SetLoadDone(static_cast<int32_t>(tiles_done / double(m_map.m_tileCount) * 100.0));
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
			groundBrush->draw(&m_map, tile, nullptr);

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

	Houses& houses = m_map.getHouses();

	HouseMap::iterator iter = houses.begin();
	while(iter != houses.end()) {
		House* h = iter->second;
		if(m_map.getTowns().getTown(h->townid) == nullptr) {
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
	for(MapIterator map_iter = m_map.begin(); map_iter != m_map.end(); ++map_iter) {
		if(showdialog && tiles_done % 4096 == 0) {
			g_gui.SetLoadDone(int(tiles_done / double(m_map.m_tileCount) * 100.0));
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
	for(MapIterator map_iter = m_map.begin(); map_iter != m_map.end(); ++map_iter) {
		if(showdialog && tiles_done % 4096 == 0) {
			g_gui.SetLoadDone(int(tiles_done / double(m_map.m_tileCount) * 100.0));
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
	BatchAction* batchAction = m_actionQueue->createBatch(ACTION_MOVE); // Our saved action batch, for undo!
	Action* action;

	// Remove tiles from the map
	action = m_actionQueue->createAction(batchAction); // Our action!
	bool doborders = false;
	TileSet tmp_storage;

	// Update the tiles with the newd positions
	for(TileSet::iterator it = m_selection.begin(); it != m_selection.end(); ++it) {
		// First we get the old tile and it's position
		Tile* tile = (*it);
		//const Position pos = tile->getPosition();

		// Create the duplicate source tile, which will replace the old one later
		Tile* old_src_tile = tile;
		Tile* new_src_tile;

		new_src_tile = old_src_tile->deepCopy(m_map);

		Tile* tmp_storage_tile = m_map.allocator(tile->getLocation());

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
		m_selection.size() < size_t(g_settings.getInteger(Config::BORDERIZE_DRAG_THRESHOLD))) {
		action = m_actionQueue->createAction(batchAction);
		TileList borderize_tiles;
		// Go through all modified (selected) tiles (might be slow)
		for(TileSet::iterator it = tmp_storage.begin(); it != tmp_storage.end(); ++it) {
			Position pos = (*it)->getPosition();
			// Go through all neighbours
			Tile* t;
			t = m_map.getTile(pos.x  , pos.y  , pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t);}
			t = m_map.getTile(pos.x-1, pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t);}
			t = m_map.getTile(pos.x  , pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t);}
			t = m_map.getTile(pos.x+1, pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t);}
			t = m_map.getTile(pos.x-1, pos.y  , pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t);}
			t = m_map.getTile(pos.x+1, pos.y  , pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t);}
			t = m_map.getTile(pos.x-1, pos.y+1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t);}
			t = m_map.getTile(pos.x  , pos.y+1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t);}
			t = m_map.getTile(pos.x+1, pos.y+1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t);}
		}
		// Remove duplicates
		borderize_tiles.sort();
		borderize_tiles.unique();
		// Do le borders!
		for(TileList::iterator it = borderize_tiles.begin(); it != borderize_tiles.end(); ++it) {
			Tile* tile = *it;
			Tile* new_tile = (*it)->deepCopy(m_map);
			if(doborders) new_tile->borderize(&m_map);
			new_tile->wallize(&m_map);
			new_tile->tableize(&m_map);
			new_tile->carpetize(&m_map);
			if(tile->ground && tile->ground->isSelected()) new_tile->selectGround();
			action->addChange(newd Change(new_tile));
		}
		// Commit changes to map
		batchAction->addAndCommitAction(action);
	}

	// New action for adding the destination tiles
	action = m_actionQueue->createAction(batchAction);
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
		TileLocation* location = m_map.createTileL(new_pos);
		Tile* old_dest_tile = location->get();
		Tile* new_dest_tile = nullptr;

		if(g_settings.getInteger(Config::MERGE_MOVE) || !tile->ground) {
			// Move items
			if(old_dest_tile) {
				new_dest_tile = old_dest_tile->deepCopy(m_map);
			} else {
				new_dest_tile = m_map.allocator(location);
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
		m_selection.size() < size_t(g_settings.getInteger(Config::BORDERIZE_DRAG_THRESHOLD))) {
		action = m_actionQueue->createAction(batchAction);
		TileList borderize_tiles;
		// Go through all modified (selected) tiles (might be slow)
		for(TileSet::iterator it = m_selection.begin(); it != m_selection.end(); it++) {
			bool add_me = false; // If this tile is touched
			Position pos = (*it)->getPosition();
			// Go through all neighbours
			Tile* t;
			t = m_map.getTile(pos.x-1, pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true;}
			t = m_map.getTile(pos.x-1, pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true;}
			t = m_map.getTile(pos.x  , pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true;}
			t = m_map.getTile(pos.x+1, pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true;}
			t = m_map.getTile(pos.x-1, pos.y  , pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true;}
			t = m_map.getTile(pos.x+1, pos.y  , pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true;}
			t = m_map.getTile(pos.x-1, pos.y+1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true;}
			t = m_map.getTile(pos.x  , pos.y+1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true;}
			t = m_map.getTile(pos.x+1, pos.y+1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true;}
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
					Tile* new_tile = tile->deepCopy(m_map);

					if(doborders)
						new_tile->borderize(&m_map);

					new_tile->wallize(&m_map);
					new_tile->tableize(&m_map);
					new_tile->carpetize(&m_map);
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
	m_selection.updateSelectionCount();
}

void Editor::destroySelection()
{
	if(m_selection.size() == 0) {
		g_gui.SetStatusText("No selected items to delete.");
	} else {
		int tile_count = 0;
		int item_count = 0;
		PositionList tilestoborder;

		BatchAction* batch = m_actionQueue->createBatch(ACTION_DELETE_TILES);
		Action* action = m_actionQueue->createAction(batch);

		for(TileSet::iterator it = m_selection.begin(); it != m_selection.end(); ++it) {
			tile_count++;

			Tile* tile = *it;
			Tile* newtile = tile->deepCopy(m_map);

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
						tilestoborder.push_back(
							Position(tile->getPosition().x + x,
							tile->getPosition().y + y,
							tile->getPosition().z));
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

			action = m_actionQueue->createAction(batch);
			for(PositionList::iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
				TileLocation* location = m_map.createTileL(*it);
				Tile* tile = location->get();

				if(tile) {
					Tile* new_tile = tile->deepCopy(m_map);
					new_tile->borderize(&m_map);
					new_tile->wallize(&m_map);
					new_tile->tableize(&m_map);
					new_tile->carpetize(&m_map);
					action->addChange(newd Change(new_tile));
				} else {
					Tile* new_tile = m_map.allocator(location);
					new_tile->borderize(&m_map);
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
		tilestoborder.push_back(Position(new_tile->getPosition().x, new_tile->getPosition().y, new_tile->getPosition().z));
		if(buffer_tile->hasGround()) {
			for(int y = -1; y <= 1; y++) {
				for(int x = -1; x <= 1; x++) {
					tilestoborder.push_back(Position(new_tile->getPosition().x + x, new_tile->getPosition().y + y, new_tile->getPosition().z));
				}
			}
		} else if(buffer_tile->hasWall()) {
			tilestoborder.push_back(Position(new_tile->getPosition().x, new_tile->getPosition().y-1, new_tile->getPosition().z));
			tilestoborder.push_back(Position(new_tile->getPosition().x-1, new_tile->getPosition().y, new_tile->getPosition().z));
			tilestoborder.push_back(Position(new_tile->getPosition().x+1, new_tile->getPosition().y, new_tile->getPosition().z));
			tilestoborder.push_back(Position(new_tile->getPosition().x, new_tile->getPosition().y+1, new_tile->getPosition().z));
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
		BatchAction* batch = m_actionQueue->createBatch(ACTION_DRAW);
		Action* action = m_actionQueue->createAction(batch);
		BaseMap* buffer_map = g_gui.doodad_buffer_map;

		Position delta_pos = offset - Position(0x8000, 0x8000, 0x8);
		PositionList tilestoborder;

		for(MapIterator it = buffer_map->begin(); it != buffer_map->end(); ++it) {
			Tile* buffer_tile = (*it)->get();
			Position pos = buffer_tile->getPosition() + delta_pos;
			if(!pos.isValid()) {
				continue;
			}

			TileLocation* location = m_map.createTileL(pos);
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
						Tile* new_tile = tile->deepCopy(m_map);
						removeDuplicateWalls(buffer_tile, new_tile);
						doSurroundingBorders(doodad_brush, tilestoborder, buffer_tile, new_tile);
						new_tile->merge(buffer_tile);
						action->addChange(newd Change(new_tile));
					}
				} else {
					Tile* new_tile = m_map.allocator(location);
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
						Tile* new_tile = tile->deepCopy(m_map);
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
			Action* action = m_actionQueue->createAction(batch);

			// Remove duplicates
			tilestoborder.sort();
			tilestoborder.unique();

			for(PositionList::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
				Tile* tile = m_map.getTile(*it);
				if(tile) {
					Tile* new_tile = tile->deepCopy(m_map);
					new_tile->borderize(&m_map);
					new_tile->wallize(&m_map);
					action->addChange(newd Change(new_tile));
				}
			}
			batch->addAndCommitAction(action);
		}
		addBatch(batch, 2);
	} else if(brush->isHouseExit()) {
		HouseExitBrush* house_exit_brush = brush->asHouseExit();
		if(!house_exit_brush->canDraw(&m_map, offset))
			return;

		House* house = m_map.getHouses().getHouse(house_exit_brush->getHouseID());
		if(!house)
			return;

		BatchAction* batch = m_actionQueue->createBatch(ACTION_DRAW);
		Action* action = m_actionQueue->createAction(batch);
		action->addChange(Change::Create(house, offset));
		batch->addAndCommitAction(action);
		addBatch(batch, 2);
	} else if(brush->isWaypoint()) {
		WaypointBrush* waypoint_brush = brush->asWaypoint();
		if(!waypoint_brush->canDraw(&m_map, offset))
			return;

		Waypoint* waypoint = m_map.getWaypoints().getWaypoint(waypoint_brush->getWaypoint());
		if(!waypoint || waypoint->pos == offset)
			return;

		BatchAction* batch = m_actionQueue->createBatch(ACTION_DRAW);
		Action* action = m_actionQueue->createAction(batch);
		action->addChange(Change::Create(waypoint, offset));
		batch->addAndCommitAction(action);
		addBatch(batch, 2);
	} else if(brush->isWall()) {
		BatchAction* batch = m_actionQueue->createBatch(ACTION_DRAW);
		Action* action = m_actionQueue->createAction(batch);
		// This will only occur with a size 0, when clicking on a tile (not drawing)
		Tile* tile = m_map.getTile(offset);
		Tile* new_tile = nullptr;
		if(tile) {
			new_tile = tile->deepCopy(m_map);
		} else {
			new_tile = m_map.allocator(m_map.createTileL(offset));
		}

		if(dodraw) {
			bool b = true;
			brush->asWall()->draw(&m_map, new_tile, &b);
		} else {
			brush->asWall()->undraw(&m_map, new_tile);
		}
		action->addChange(newd Change(new_tile));
		batch->addAndCommitAction(action);
		addBatch(batch, 2);
	} else if(brush->isSpawn() || brush->isCreature()) {
		BatchAction* batch = m_actionQueue->createBatch(ACTION_DRAW);
		Action* action = m_actionQueue->createAction(batch);

		Tile* tile = m_map.getTile(offset);
		Tile* new_tile = nullptr;
		if(tile) {
			new_tile = tile->deepCopy(m_map);
		} else {
			new_tile = m_map.allocator(m_map.createTileL(offset));
		}
		int param;
		if(!brush->isCreature()) {
			param = g_gui.GetBrushSize();
		}
		if(dodraw) {
			brush->draw(&m_map, new_tile, &param);
		} else {
			brush->undraw(&m_map, new_tile);
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

	Action* action = m_actionQueue->createAction(ACTION_DRAW);

	if(brush->isOptionalBorder()) {
		// We actually need to do borders, but on the same tiles we draw to
		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = m_map.createTileL(*it);
			Tile* tile = location->get();
			if(tile) {
				if(dodraw) {
					Tile* new_tile = tile->deepCopy(m_map);
					brush->draw(&m_map, new_tile);
					new_tile->borderize(&m_map);
					action->addChange(newd Change(new_tile));
				} else if(!dodraw && tile->hasOptionalBorder()) {
					Tile* new_tile = tile->deepCopy(m_map);
					brush->undraw(&m_map, new_tile);
					new_tile->borderize(&m_map);
					action->addChange(newd Change(new_tile));
				}
			} else if(dodraw) {
				Tile* new_tile = m_map.allocator(location);
				brush->draw(&m_map, new_tile);
				new_tile->borderize(&m_map);
				if(new_tile->size() == 0) {
					delete new_tile;
					continue;
				}
				action->addChange(newd Change(new_tile));
			}
		}
	} else {

		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = m_map.createTileL(*it);
			Tile* tile = location->get();
			if(tile) {
				Tile* new_tile = tile->deepCopy(m_map);
				if(dodraw) {
					brush->draw(&m_map, new_tile, &alt);
				} else {
					brush->undraw(&m_map, new_tile);
				}
				action->addChange(newd Change(new_tile));
			} else if(dodraw) {
				Tile* new_tile = m_map.allocator(location);
				brush->draw(&m_map, new_tile, &alt);
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
		BatchAction* batch = m_actionQueue->createBatch(ACTION_DRAW);
		Action* action = m_actionQueue->createAction(batch);

		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = m_map.createTileL(*it);
			Tile* tile = location->get();
			if(tile) {
				Tile* new_tile = tile->deepCopy(m_map);
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
						g_gui.GetCurrentBrush()->draw(&m_map, new_tile, &param);
					} else {
						g_gui.GetCurrentBrush()->draw(&m_map, new_tile, nullptr);
					}
				else {
					g_gui.GetCurrentBrush()->undraw(&m_map, new_tile);
					tilestoborder.push_back(*it);
				}
				action->addChange(newd Change(new_tile));
			} else if(dodraw) {
				Tile* new_tile = m_map.allocator(location);
				if(brush->isGround() && alt) {
					std::pair<bool, GroundBrush*> param;
					if(replace_brush) {
						param.first = false;
						param.second = replace_brush;
					} else {
						param.first = true;
						param.second = nullptr;
					}
					g_gui.GetCurrentBrush()->draw(&m_map, new_tile, &param);
				} else {
					g_gui.GetCurrentBrush()->draw(&m_map, new_tile, nullptr);
				}
				action->addChange(newd Change(new_tile));
			}
		}

		// Commit changes to map
		batch->addAndCommitAction(action);

		if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
			// Do borders!
			action = m_actionQueue->createAction(batch);
			for(PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
				TileLocation* location = m_map.createTileL(*it);
				Tile* tile = location->get();
				if(tile) {
					Tile* new_tile = tile->deepCopy(m_map);
					if(brush->isEraser()) {
						new_tile->wallize(&m_map);
						new_tile->tableize(&m_map);
						new_tile->carpetize(&m_map);
					}
					new_tile->borderize(&m_map);
					action->addChange(newd Change(new_tile));
				} else {
					Tile* new_tile = m_map.allocator(location);
					if(brush->isEraser()) {
						// There are no carpets/tables/walls on empty tiles...
						//new_tile->wallize(map);
						//new_tile->tableize(map);
						//new_tile->carpetize(map);
					}
					new_tile->borderize(&m_map);
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
		BatchAction* batch = m_actionQueue->createBatch(ACTION_DRAW);
		Action* action = m_actionQueue->createAction(batch);

		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = m_map.createTileL(*it);
			Tile* tile = location->get();
			if(tile) {
				Tile* new_tile = tile->deepCopy(m_map);
				if(dodraw)
					g_gui.GetCurrentBrush()->draw(&m_map, new_tile, nullptr);
				else
					g_gui.GetCurrentBrush()->undraw(&m_map, new_tile);
				action->addChange(newd Change(new_tile));
			} else if(dodraw) {
				Tile* new_tile = m_map.allocator(location);
				g_gui.GetCurrentBrush()->draw(&m_map, new_tile, nullptr);
				action->addChange(newd Change(new_tile));
			}
		}

		// Commit changes to map
		batch->addAndCommitAction(action);

		// Do borders!
		action = m_actionQueue->createAction(batch);
		for(PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
			Tile* tile = m_map.getTile(*it);
			if(brush->isTable()) {
				if(tile && tile->hasTable()) {
					Tile* new_tile = tile->deepCopy(m_map);
					new_tile->tableize(&m_map);
					action->addChange(newd Change(new_tile));
				}
			} else if(brush->isCarpet()) {
				if(tile && tile->hasCarpet()) {
					Tile* new_tile = tile->deepCopy(m_map);
					new_tile->carpetize(&m_map);
					action->addChange(newd Change(new_tile));
				}
			}
		}
		batch->addAndCommitAction(action);

		addBatch(batch, 2);
	} else if(brush->isWall()) {
		BatchAction* batch = m_actionQueue->createBatch(ACTION_DRAW);
		Action* action = m_actionQueue->createAction(batch);

		if(alt && dodraw) {
			// This is exempt from USE_AUTOMAGIC
			g_gui.doodad_buffer_map->clear();
			BaseMap* draw_map = g_gui.doodad_buffer_map;

			for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
				TileLocation* location = m_map.createTileL(*it);
				Tile* tile = location->get();
				if(tile) {
					Tile* new_tile = tile->deepCopy(m_map);
					new_tile->cleanWalls(brush->isWall());
					g_gui.GetCurrentBrush()->draw(draw_map, new_tile);
					draw_map->setTile(*it, new_tile, true);
				} else if(dodraw) {
					Tile* new_tile = m_map.allocator(location);
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
				TileLocation* location = m_map.createTileL(*it);
				Tile* tile = location->get();
				if(tile) {
					Tile* new_tile = tile->deepCopy(m_map);
					// Wall cleaning is exempt from automagic
					new_tile->cleanWalls(brush->isWall());
					if(dodraw)
						g_gui.GetCurrentBrush()->draw(&m_map, new_tile);
					else
						g_gui.GetCurrentBrush()->undraw(&m_map, new_tile);
					action->addChange(newd Change(new_tile));
				} else if(dodraw) {
					Tile* new_tile = m_map.allocator(location);
					g_gui.GetCurrentBrush()->draw(&m_map, new_tile);
					action->addChange(newd Change(new_tile));
				}
			}

			// Commit changes to map
			batch->addAndCommitAction(action);

			if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
				// Do borders!
				action = m_actionQueue->createAction(batch);
				for(PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
					Tile* tile = m_map.getTile(*it);
					if(tile) {
						Tile* new_tile = tile->deepCopy(m_map);
						new_tile->wallize(&m_map);
						//if(*tile == *new_tile) delete new_tile;
						action->addChange(newd Change(new_tile));
					}
				}
				batch->addAndCommitAction(action);
			}
		}

		m_actionQueue->addBatch(batch, 2);
	} else if(brush->isDoor()) {
		BatchAction* batch = m_actionQueue->createBatch(ACTION_DRAW);
		Action* action = m_actionQueue->createAction(batch);
		DoorBrush* door_brush = brush->asDoor();

		// Loop is kind of redundant since there will only ever be one index.
		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = m_map.createTileL(*it);
			Tile* tile = location->get();
			if(tile) {
				Tile* new_tile = tile->deepCopy(m_map);
				// Wall cleaning is exempt from automagic
				if(brush->isWall())
					new_tile->cleanWalls(brush->asWall());
				if(dodraw)
					door_brush->draw(&m_map, new_tile, &alt);
				else
					door_brush->undraw(&m_map, new_tile);
				action->addChange(newd Change(new_tile));
			} else if(dodraw) {
				Tile* new_tile = m_map.allocator(location);
				door_brush->draw(&m_map, new_tile, &alt);
				action->addChange(newd Change(new_tile));
			}
		}

		// Commit changes to map
		batch->addAndCommitAction(action);

		if(g_settings.getInteger(Config::USE_AUTOMAGIC)) {
			// Do borders!
			action = m_actionQueue->createAction(batch);
			for(PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
				Tile* tile = m_map.getTile(*it);
				if(tile) {
					Tile* new_tile = tile->deepCopy(m_map);
					new_tile->wallize(&m_map);
					//if(*tile == *new_tile) delete new_tile;
					action->addChange(newd Change(new_tile));
				}
			}
			batch->addAndCommitAction(action);
		}

		addBatch(batch, 2);
	} else {
		Action* action = m_actionQueue->createAction(ACTION_DRAW);
		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			TileLocation* location = m_map.createTileL(*it);
			Tile* tile = location->get();
			if(tile) {
				Tile* new_tile = tile->deepCopy(m_map);
				if(dodraw)
					g_gui.GetCurrentBrush()->draw(&m_map, new_tile);
				else
					g_gui.GetCurrentBrush()->undraw(&m_map, new_tile);
				action->addChange(newd Change(new_tile));
			} else if(dodraw) {
				Tile* new_tile = m_map.allocator(location);
				g_gui.GetCurrentBrush()->draw(&m_map, new_tile);
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

	delete m_actionQueue;
	m_actionQueue = newd NetworkedActionQueue(*this);

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

		delete m_actionQueue;
		m_actionQueue = newd ActionQueue(*this);
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

