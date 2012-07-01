//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/editor.hpp $
// $Id: editor.hpp 317 2010-03-01 01:54:30Z admin $

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
	valid_state(true),
	live_client(NULL),
	live_server(NULL),
	actionQueue(newd ActionQueue(*this)),
	selection(*this),
	copybuffer(copybuffer),
	replace_brush(NULL)
{
	wxString error;
	wxArrayString warnings;
	bool ok = true;
	ClientVersionID defaultVersion = ClientVersionID(settings.getInteger(Config::DEFAULT_CLIENT_VERSION));
	if (defaultVersion == CLIENT_VERSION_NONE)
		defaultVersion = ClientVersion::getLatestVersion()->getID();
	if(gui.GetCurrentVersionID() != defaultVersion) {
		if(gui.CloseAllEditors()) {
			ok = gui.LoadVersion(defaultVersion, error, warnings);
			gui.PopupDialog(wxT("Error"), error, wxOK);
			gui.ListDialog(wxT("Warnings"), warnings);
		} else {
			throw std::runtime_error("All maps of different versions were not closed.");
		}
	}

	if(ok == false)
		throw std::runtime_error("Couldn't load client version");

	MapVersion version;
	version.otbm = gui.GetCurrentVersion().getPrefferedMapVersionID();
	version.client = gui.GetCurrentVersionID();
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
	valid_state(true),
	live_client(NULL),
	live_server(NULL),
	actionQueue(newd ActionQueue(*this)),
	selection(*this),
	copybuffer(copybuffer),
	replace_brush(NULL)
{
	MapVersion ver = IOMapOTBM::getVersionInfo(fn);
	if(ver.client == CLIENT_VERSION_NONE) {
		gui.PopupDialog(wxT("Error"), wxT("Could not open file \"") + fn.GetFullPath() + wxT("\"."), wxOK);
		valid_state = false;
		return;
	}

	/*
	if(ver < CLIENT_VERSION_760) {
		long b = gui.PopupDialog(wxT("Error"), wxT("Unsupported Client Version (pre 7.6), do you want to try to load the map anyways?"), wxYES | wxNO);
		if(b == wxID_NO) {
			valid_state = false;
			return;
		}
	}
	*/

	bool success = true;
	if(gui.GetCurrentVersionID() != ver.client) {
		wxString error;
		wxArrayString warnings;
		if(gui.CloseAllEditors()) {
			success = gui.LoadVersion(ver.client, error, warnings);
			gui.PopupDialog(wxT("Error"), error, wxOK);
			gui.ListDialog(wxT("Warnings"), warnings);
		} else {
			throw std::runtime_error("All maps of different versions were not closed.");
		}
	}

	if(success)
	{
		success = map.open(nstr(fn.GetFullPath()), true);
		/* TODO
		if(success && ver.client == CLIENT_VERSION_854_BAD)
		{
			int ok = gui.PopupDialog(wxT("Incorrect OTB"), wxT("This map has been saved with an incorrect OTB version, do you want to convert it to the new OTB version?\n\nIf you are not sure, click Yes."), wxYES | wxNO);
			
			if(ok == wxID_YES){
				ver.client = CLIENT_VERSION_854;
				map.convert(ver);
			}
		}
		*/
	}

	valid_state = success;
}

Editor::Editor(CopyBuffer& copybuffer, LiveClient* client) :
	valid_state(true),
	live_server(NULL),
	live_client(client),
	actionQueue(newd NetworkedActionQueue(*this)),
	selection(*this),
	copybuffer(copybuffer),
	replace_brush(NULL)
{
	;
}

Editor::~Editor() {
	if(IsLive())
		CloseLiveServer();

	UnnamedRenderingLock();
	selection.clear();
	delete actionQueue;
}

void Editor::addBatch(BatchAction* action, int stacking_delay) {
	actionQueue->addBatch(action, stacking_delay);
	gui.UpdateMenus();
}

void Editor::addAction(Action* action, int stacking_delay ) {
	actionQueue->addAction(action, stacking_delay);
	gui.UpdateMenus();
}

void Editor::saveMap(FileName filename, bool showdialog) {
	wxString w = filename.GetFullPath();
	std::string savefile = (const char*)w.mb_str(wxConvUTF8);
	if(savefile == "") {
		savefile = map.filename;
	}

	FileName c1(wxstr(savefile));
	FileName c2(wxstr(map.filename));
	bool save_as = c1 != c2;


	if(map.unnamed) {
		FileName _name(filename);
		_name.SetExt(wxT("xml"));

		_name.SetName(filename.GetName() + wxT("-spawn"));
		map.spawnfile = nstr(_name.GetFullName());
		_name.SetName(filename.GetName() + wxT("-house"));
		map.housefile = nstr(_name.GetFullName());
	}
	
	// File object to convert between local paths etc.
	FileName converter;
	converter.Assign(wxstr(savefile));
	std::string map_path = nstr(converter.GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME));

	// Make temporary backups
	//converter.Assign(wxstr(savefile));
	std::string backup_otbm, backup_house, backup_spawn;

	if(converter.FileExists())
	{
		backup_otbm = map_path + nstr(converter.GetName()) + ".otbm~";
		std::remove(backup_otbm.c_str());
		std::rename(savefile.c_str(), backup_otbm.c_str());
	}

	converter.SetFullName(wxstr(map.housefile));
	if(converter.FileExists())
	{
		backup_house = map_path + nstr(converter.GetName()) + ".xml~";
		std::remove(backup_house.c_str());
		std::rename((map_path + map.housefile).c_str(), backup_house.c_str());
	}

	converter.SetFullName(wxstr(map.spawnfile));
	if(converter.FileExists())
	{
		backup_spawn = map_path + nstr(converter.GetName()) + ".xml~";
		std::remove(backup_spawn.c_str());
		std::rename((map_path + map.spawnfile).c_str(), backup_spawn.c_str());
	}

	// Save the map
	{
		std::string n = nstr(gui.GetLocalDataDirectory()) + "saving";
		std::ofstream f(n.c_str(), std::ios::trunc | std::ios::out);
		f << 
			backup_otbm << std::endl << 
			backup_house << std::endl << 
			backup_spawn << std::endl;
	}

	{
		IOMapOTBM mapsaver(map.getVersion());

		wxFileName fn = wxstr(savefile);
		map.filename = fn.GetFullPath().mb_str(wxConvUTF8);
		map.name = fn.GetFullName().mb_str(wxConvUTF8);

		bool success = mapsaver.saveMap(map, fn, showdialog);
		if(!success) {
			gui.PopupDialog(wxT("Error"), wxT("Could not save, unable to open target for writing."), wxOK);
			
			// Must rename temporary backup files
			if(!backup_otbm.empty())
			{
				converter.SetFullName(wxstr(savefile));
				std::string otbm_filename = map_path + nstr(converter.GetName());
				std::rename(backup_otbm.c_str(), std::string(otbm_filename + ".otbm").c_str());
			}

			if(!backup_house.empty())
			{
				converter.SetFullName(wxstr(map.housefile));
				std::string house_filename = map_path + nstr(converter.GetName());
				std::rename(backup_house.c_str(), std::string(house_filename + ".xml").c_str());
			}

			if(!backup_spawn.empty())
			{
				converter.SetFullName(wxstr(map.spawnfile));
				std::string spawn_filename = map_path + nstr(converter.GetName());
				std::rename(backup_spawn.c_str(), std::string(spawn_filename + ".xml").c_str());
			}
		}
		
		// Remove temporary save runfile
		{
			std::string n = nstr(gui.GetLocalDataDirectory()) + "saving";
			std::remove(n.c_str());
		}

		// If failure, don't run the rest of the function
		if(!success)
			return;
	}
	

	// Move to permanent backup
	if(save_as == false && settings.getInteger(Config::ALWAYS_MAKE_BACKUP))
	{
		// Move temporary backups to their proper files
		time_t t = time(NULL);
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

		if(!backup_otbm.empty())
		{
			converter.SetFullName(wxstr(savefile));
			std::string otbm_filename = map_path + nstr(converter.GetName());
			std::rename(backup_otbm.c_str(), std::string(otbm_filename + "." + date.str() + ".otbm").c_str());
		}

		if(!backup_house.empty())
		{
			converter.SetFullName(wxstr(map.housefile));
			std::string house_filename = map_path + nstr(converter.GetName());
			std::rename(backup_house.c_str(), std::string(house_filename + "." + date.str() + ".xml").c_str());
		}

		if(!backup_spawn.empty())
		{
			converter.SetFullName(wxstr(map.spawnfile));
			std::string spawn_filename = map_path + nstr(converter.GetName());
			std::rename(backup_spawn.c_str(), std::string(spawn_filename + "." + date.str() + ".xml").c_str());
		}
	}
	else
	{
		// Delete the temporary files
		std::remove(backup_otbm.c_str());
		std::remove(backup_house.c_str());
		std::remove(backup_spawn.c_str());
	}

	map.clearChanges();
}

bool Editor::importMiniMap(FileName filename, int import, int import_x_offset, int import_y_offset, int import_z_offset) {
	return false;
}

bool Editor::exportMiniMap(FileName filename, int floor /*= 7*/, bool displaydialog) {
	return map.exportMinimap(filename, floor, displaydialog);
	return false;
}


bool Editor::importMap(FileName filename, int import_x_offset, int import_y_offset, ImportType house_import_type, ImportType spawn_import_type) {selection.clear();
	actionQueue->clear();

	Map imported_map;
	bool loaded = imported_map.open(nstr(filename.GetFullPath()), true);

	if(!loaded) {
		gui.PopupDialog(wxT("Error"), wxT("Error loading map!\n") + imported_map.getError(), wxOK | wxICON_INFORMATION);
		return false;
	}
	gui.ListDialog(wxT("Warning"), imported_map.getWarnings());

	Position offset(import_x_offset, import_y_offset, 0);

	bool resizemap = false;
	bool resize_asked = false;
	int newsize_x = map.getWidth(), newsize_y = map.getHeight();
	int discarded_tiles = 0;

	gui.CreateLoadBar(wxT("Merging maps..."));

	std::map<uint32_t, uint32_t> town_id_map;
	std::map<uint32_t, uint32_t> house_id_map;

	if(house_import_type != IMPORT_DONT) {
		for(TownMap::iterator tit = imported_map.towns.begin();
				tit != imported_map.towns.end();)
		{
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
				} break;
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
				} break;
				case IMPORT_INSERT: {
					// Find a newd id and replace old
					uint32_t new_id = map.towns.getEmptyID();
					imported_town->setID(new_id);
					town_id_map[imported_town->getID()] = new_id;
				} break;
				case IMPORT_DONT: {
					++tit;
					continue; // Should never happend..?
				} break;
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
		for(HouseMap::iterator hit = imported_map.houses.begin();
				hit != imported_map.houses.end();)
		{
			House* imported_house = hit->second;
			House* current_house = map.houses.getHouse(imported_house->id);
			imported_house->townid = town_id_map[imported_house->townid];

			Position oldexit = imported_house->getExit();
			imported_house->setExit(NULL, Position()); // Reset it

			switch(house_import_type) {
				case IMPORT_MERGE: {
					house_id_map[imported_house->id] = imported_house->id;
					if(current_house) {
						++hit;
						Position newexit = oldexit + offset;
						if(newexit.isValid()) current_house->setExit(&map, newexit);
						continue;
					}
				} break;
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
				} break;
				case IMPORT_INSERT: {
					// Find a newd id and replace old
					uint32_t new_id = map.houses.getEmptyID();
					house_id_map[imported_house->id] = new_id;
					imported_house->id = new_id;
				} break;
				case IMPORT_DONT: {
					++hit;
					Position newexit = oldexit + offset;
					if(newexit.isValid()) imported_house->setExit(&map, newexit);
					continue; // Should never happend..?
				} break;
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
	if(spawn_import_type != IMPORT_DONT)
	{
		for(SpawnPositionList::iterator siter = imported_map.spawns.begin();
				siter != imported_map.spawns.end();)
		{
			Position old_spawn_pos = *siter;
			Position new_spawn_pos = *siter + offset;
			switch(spawn_import_type)
			{
				case IMPORT_SMART_MERGE:
				case IMPORT_INSERT:
				case IMPORT_MERGE: 
				{
					Tile* imported_tile = imported_map.getTile(old_spawn_pos);
					if(imported_tile)
					{
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
				} break;
				case IMPORT_DONT:
				{
					++siter;
				} break;
			}
		}
	}

	// Plain merge of waypoints, very simple! :)
	for(WaypointMap::iterator iter = imported_map.waypoints.begin();
		iter != imported_map.waypoints.end();
		++iter)
	{
		iter->second->pos += offset;
	}

	map.waypoints.waypoints.insert(imported_map.waypoints.begin(), imported_map.waypoints.end());
	imported_map.waypoints.waypoints.clear();


	uint64_t tiles_merged = 0;
	uint64_t tiles_to_import = imported_map.tilecount;
	for(MapIterator mit = imported_map.begin();
			mit != imported_map.end();
			++mit)
	{
		if(tiles_merged % 8092 == 0) {
			gui.SetLoadDone(int(100.0 * tiles_merged / tiles_to_import));
		}
		++tiles_merged;

		Tile* import_tile = (*mit)->get();
		Position new_pos = import_tile->getPosition() + offset;
		if(!new_pos.isValid()) {
			++discarded_tiles;
			continue;
		}

		if(resizemap == false && (new_pos.x > map.getWidth() || new_pos.y > map.getHeight())) {
			if(resize_asked) {
				++discarded_tiles;
				continue;
			} else {
				resize_asked = true;
				int ret = gui.PopupDialog(wxT("Collision"), wxT("The imported tiles are outside the current map scope. Do you want to resize the map? (Else additional tiles will be removed)"), wxYES | wxNO);

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

		imported_map.setTile(import_tile->getPosition(), NULL);
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
			for(ItemVector::iterator iter = import_tile->items.begin();
					iter != import_tile->items.end();
					++iter)
			{
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
		import_tile->spawn = NULL;

		map.setTile(new_pos, import_tile, true);
	}

	for(std::map<Position, Spawn*>::iterator spawn_iter = spawn_map.begin();
			spawn_iter != spawn_map.end();
			++spawn_iter)
	{
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

	gui.DestroyLoadBar();

	map.setWidth(newsize_x);
	map.setHeight(newsize_y);
	gui.PopupDialog(wxT("Success"), wxT("Map imported successfully, ") + i2ws(discarded_tiles) + wxT(" tiles were discarded as invalid."), wxOK);
 
	gui.RefreshPalettes();
	gui.FitViewToMap();

	return true;
}

void Editor::borderizeSelection() {
	if(selection.size() == 0) {
		gui.SetStatusText(wxT("No items selected. Can't borderize."));
	}

	Action* action = actionQueue->createAction(ACTION_BORDERIZE);
	for(TileVector::iterator it = selection.begin(); it != selection.end(); it++) {
		Tile* tile = *it;
		Tile* new_tile = tile->deepCopy(map);
		new_tile->borderize(&map);
		new_tile->select();
		action->addChange(newd Change(new_tile));
	}
	addAction(action);
}

void Editor::borderizeMap(bool showdialog) {
	if(showdialog) {
		gui.CreateLoadBar(wxT("Borderizing map..."));
	}

	uint64_t tiles_done = 0;
	for(MapIterator map_iter = map.begin();
			map_iter != map.end();
			++map_iter)
	{
		if(showdialog && tiles_done % 4096 == 0) {
			gui.SetLoadDone(int(tiles_done / double(map.tilecount) * 100.0));
		}

		Tile* tile = (*map_iter)->get();
		ASSERT(tile);
		tile->borderize(&map);
		tiles_done += 1;
	}

	if(showdialog) {
		gui.DestroyLoadBar();
	}
}

void Editor::randomizeSelection() {
	if(selection.size() == 0) {
		gui.SetStatusText(wxT("No items selected. Can't randomize."));
	}

	Action* action = actionQueue->createAction(ACTION_RANDOMIZE);
	for(TileVector::iterator it = selection.begin(); it != selection.end(); it++) {
		Tile* tile = *it;
		Tile* new_tile = tile->deepCopy(map);
		GroundBrush* gb = new_tile->getGroundBrush();
		Item* old_ground = tile->ground;
		if(gb && gb->isReRandomizable()) {
			gb->draw(&map, new_tile, NULL);
			Item* new_ground = new_tile->ground;
			if(old_ground && new_ground) {
				new_ground->setActionID(old_ground? old_ground->getActionID() : 0);
				new_ground->setUniqueID(old_ground? old_ground->getUniqueID() : 0);
			}
			new_tile->select();
			action->addChange(newd Change(new_tile));
		}
	}
	addAction(action);
}

void Editor::randomizeMap(bool showdialog) {
	if(showdialog) {
		gui.CreateLoadBar(wxT("Randomizing map..."));
	}

	uint64_t tiles_done = 0;
	for(MapIterator map_iter = map.begin();
			map_iter != map.end();
			++map_iter)
	{
		if(showdialog && tiles_done % 4096 == 0) {
			gui.SetLoadDone(int(tiles_done / double(map.tilecount) * 100.0));
		}

		Tile* tile = (*map_iter)->get();
		ASSERT(tile);
		GroundBrush* gb = tile->getGroundBrush();
		Item* old_ground = tile->ground;
		uint16_t old_aid = old_ground? old_ground->getActionID() : 0;
		uint16_t old_uid = old_ground? old_ground->getUniqueID() : 0;
		if(gb) {
			gb->draw(&map, tile, NULL);
			Item* new_ground = tile->ground;
			if(new_ground) {
				new_ground->setActionID(old_aid);
				new_ground->setUniqueID(old_uid);
			}
			tile->update();
		}
		tiles_done += 1;
	}

	if(showdialog) {
		gui.DestroyLoadBar();
	}
}

void Editor::clearInvalidHouseTiles(bool showdialog) {
	if(showdialog) {
		gui.CreateLoadBar(wxT("Clearing invalid house tiles..."));
	}

	Houses& houses = map.houses;

	HouseMap::iterator iter = houses.begin();
	while(iter != houses.end()) {
		House* h = iter->second;
		if(map.towns.getTown(h->townid) == NULL) {
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
	for(MapIterator map_iter = map.begin();
			map_iter != map.end();
			++map_iter)
	{
		if(showdialog && tiles_done % 4096 == 0) {
			gui.SetLoadDone(int(tiles_done / double(map.tilecount) * 100.0));
		}

		Tile* tile = (*map_iter)->get();
		ASSERT(tile);
		if(tile->isHouseTile()) {
			if(houses.getHouse(tile->getHouseID()) == NULL) {
				tile->setHouse(NULL);
			}
		}
	}

	if(showdialog) {
		gui.DestroyLoadBar();
	}
}

void Editor::clearModifiedTileState(bool showdialog) {
	if(showdialog) {
		gui.CreateLoadBar(wxT("Clearing modified state from all tiles..."));
	}

	
	uint64_t tiles_done = 0;
	for(MapIterator map_iter = map.begin();
			map_iter != map.end();
			++map_iter)
	{
		if(showdialog && tiles_done % 4096 == 0) {
			gui.SetLoadDone(int(tiles_done / double(map.tilecount) * 100.0));
		}

		Tile* tile = (*map_iter)->get();
		ASSERT(tile);
		tile->unmodify();
	}

	if(showdialog) {
		gui.DestroyLoadBar();
	}
}

void Editor::moveSelection(Position offset)
{
	BatchAction* batchAction = actionQueue->createBatch(ACTION_MOVE); // Our saved action batch, for undo!
	Action* action;

	// Remove tiles from the map
	action = actionQueue->createAction(batchAction); // Our action!
	bool doborders = false;
	TileVector tmp_storage;

	// Update the tiles with the newd positions
	for(TileVector::iterator it = selection.begin();
		it != selection.end(); ++it)
	{
		// First we get the old tile and it's position
		Tile* tile = (*it);
		const Position pos = tile->getPosition();

		// Create the duplicate source tile, which will replace the old one later
		Tile* old_src_tile = tile;
		Tile* new_src_tile;

		new_src_tile = old_src_tile->deepCopy(map);

		Tile* tmp_storage_tile = map.allocator(tile->getLocation());

		// Get all the selected items from the NEW source tile and iterate through them
		// This transfers ownership to the temporary tile
		ItemVector tile_selection = new_src_tile->popSelectedItems();
		for(ItemVector::iterator iit = tile_selection.begin();
			iit != tile_selection.end();
			iit++)
		{
			// Add the copied item to the newd destination tile,
			Item* item = (*iit);
			tmp_storage_tile->addItem(item);
		}
		// Move spawns
		if(new_src_tile->spawn && new_src_tile->spawn->isSelected()) {
			tmp_storage_tile->spawn = new_src_tile->spawn;
			new_src_tile->spawn = NULL;
		}
		// Move creatures
		if(new_src_tile->creature && new_src_tile->creature->isSelected()) {
			tmp_storage_tile->creature = new_src_tile->creature;
			new_src_tile->creature = NULL;
		}

		// Move house data & tile status if ground is transferred
		if(tmp_storage_tile->ground) {
			tmp_storage_tile->house_id = new_src_tile->house_id;
			new_src_tile->house_id = 0;
			tmp_storage_tile->setMapFlags(new_src_tile->getMapFlags());
			new_src_tile->setMapFlags(TILESTATE_NONE);
			doborders = true;
		}

		tmp_storage.push_back(tmp_storage_tile);
		// Add the tile copy to the action
		action->addChange(newd Change(new_src_tile));
	}
	// Commit changes to map
	batchAction->addAndCommitAction(action);

	// Remove old borders (and create some newd?)
	if(settings.getInteger(Config::USE_AUTOMAGIC) &&
			settings.getInteger(Config::BORDERIZE_DRAG) &&
			selection.size() < size_t(settings.getInteger(Config::BORDERIZE_DRAG_THRESHOLD)))
	{
		action = actionQueue->createAction(batchAction);
		TileList borderize_tiles;
		// Go through all modified (selected) tiles (might be slow)
		for(TileVector::iterator it = tmp_storage.begin();
			it != tmp_storage.end(); ++it)
		{
			Position pos = (*it)->getPosition();
			// Go through all neighbours
			Tile* t;
			t = map.getTile(pos.x  , pos.y  , pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t);}
			t = map.getTile(pos.x-1, pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t);}
			t = map.getTile(pos.x  , pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t);}
			t = map.getTile(pos.x+1, pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t);}
			t = map.getTile(pos.x-1, pos.y  , pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t);}
			t = map.getTile(pos.x+1, pos.y  , pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t);}
			t = map.getTile(pos.x-1, pos.y+1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t);}
			t = map.getTile(pos.x  , pos.y+1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t);}
			t = map.getTile(pos.x+1, pos.y+1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t);}
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
	for(TileVector::iterator it = tmp_storage.begin();
		it != tmp_storage.end(); ++it)
	{
		Tile* tile = (*it);
		const Position old_pos = tile->getPosition();
		Position new_pos;

		new_pos = old_pos - offset;

		if(new_pos.z < 0 && new_pos.z > 15) {
			delete tile;
			continue;
		}
		// Create the duplicate dest tile, which will replace the old one later
		TileLocation* location = map.createTileL(new_pos);
		Tile* old_dest_tile = location->get();
		Tile* new_dest_tile = NULL;

		if(settings.getInteger(Config::MERGE_MOVE) || !tile->ground) {
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
	if(settings.getInteger(Config::USE_AUTOMAGIC) &&
			settings.getInteger(Config::BORDERIZE_DRAG) &&
			selection.size() < size_t(settings.getInteger(Config::BORDERIZE_DRAG_THRESHOLD)))
	{
		action = actionQueue->createAction(batchAction);
		TileList borderize_tiles;
		// Go through all modified (selected) tiles (might be slow)
		for(TileVector::iterator it = selection.begin(); it != selection.end(); it++) {
			bool add_me = false; // If this tile is touched
			Position pos = (*it)->getPosition();
			// Go through all neighbours
			Tile* t;
			t = map.getTile(pos.x-1, pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true;}
			t = map.getTile(pos.x-1, pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true;}
			t = map.getTile(pos.x  , pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true;}
			t = map.getTile(pos.x+1, pos.y-1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true;}
			t = map.getTile(pos.x-1, pos.y  , pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true;}
			t = map.getTile(pos.x+1, pos.y  , pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true;}
			t = map.getTile(pos.x-1, pos.y+1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true;}
			t = map.getTile(pos.x  , pos.y+1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true;}
			t = map.getTile(pos.x+1, pos.y+1, pos.z); if(t && !t->isSelected()) {borderize_tiles.push_back(t); add_me = true;}
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

void Editor::destroySelection() {
	if(selection.size() == 0) {
		gui.SetStatusText(wxT("No selected items to delete."));
	} else {
		int tile_count = 0;
		int item_count = 0;
		PositionList tilestoborder;

		BatchAction* batch = actionQueue->createBatch(ACTION_DELETE_TILES);
		Action* action = actionQueue->createAction(batch);

		for(TileVector::iterator it = selection.begin(); it != selection.end(); ++it) {
			tile_count++;

			Tile* tile = *it;
			Tile* newtile = tile->deepCopy(map);

			ItemVector tile_selection = newtile->popSelectedItems();
			for(ItemVector::iterator iit = tile_selection.begin();
				iit != tile_selection.end();
				++iit)
			{
				++item_count;
				// Delete the items from the tile
				delete *iit;
			}

			if(newtile->creature && newtile->creature->isSelected()) {
				delete newtile->creature;
				newtile->creature = NULL;
			}

			if(newtile->spawn && newtile->spawn->isSelected()) {
				delete newtile->spawn;
				newtile->spawn = NULL;
			}

			if(settings.getInteger(Config::USE_AUTOMAGIC)) {
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

		if(settings.getInteger(Config::USE_AUTOMAGIC)) {
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
		ss << wxT("Deleted ") << tile_count << wxT(" tile") << (tile_count > 1 ? wxT("s") : wxT("")) <<  wxT(" (") << item_count << wxT(" item") << (item_count > 1? wxT("s") : wxT("")) << wxT(")");
		gui.SetStatusText(ss);
	}
}

	// Macro to avoid useless code repetition
void doSurroundingBorders(DoodadBrush* doodad_brush, PositionList& tilestoborder, Tile* buffer_tile, Tile* new_tile) {
	if(doodad_brush->doNewBorders() && settings.getInteger(Config::USE_AUTOMAGIC)) {
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

void removeDuplicateWalls(Tile* buffer, Tile* tile) {
	for(ItemVector::const_iterator iter = buffer->items.begin();
		iter != buffer->items.end();
		++iter)
	{
		if((*iter)->getWallBrush()) {
			tile->cleanWalls((*iter)->getWallBrush());
		}
	}
}


void Editor::drawInternal(Position offset, bool alt, bool dodraw)
{
	Brush* brush = gui.GetCurrentBrush();
	DoodadBrush* doodad_brush = dynamic_cast<DoodadBrush*>(brush);
	HouseExitBrush* house_exit_brush = dynamic_cast<HouseExitBrush*>(brush);
	WaypointBrush* waypoint_brush = dynamic_cast<WaypointBrush*>(brush);
	WallBrush* wall_brush = dynamic_cast<WallBrush*>(brush);
	SpawnBrush* spawn_brush = dynamic_cast<SpawnBrush*>(brush);
	CreatureBrush* creature_brush = dynamic_cast<CreatureBrush*>(brush);

	BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);

	if(doodad_brush) {
		Action* action = actionQueue->createAction(batch);
		BaseMap* buffer_map = gui.doodad_buffer_map;

		Position delta_pos = offset - Position(0x8000, 0x8000, 0x8);
		PositionList tilestoborder;

		for(MapIterator map_iter = buffer_map->begin();
				map_iter != buffer_map->end();
				++map_iter)
		{
			Tile* buffer_tile = (*map_iter)->get();
			Position pos = buffer_tile->getPosition() + delta_pos;
			if(pos.isValid() == false) {
				continue;
			}
			
			TileLocation* location = map.createTileL(pos);
			Tile* tile = location->get();

			if(doodad_brush->placeOnBlocking() || alt) {
				if(tile) {
					bool place = true;
					if(!doodad_brush->placeOnDuplicate() && !alt) {
						for(ItemVector::const_iterator iter = tile->items.begin();
							iter != tile->items.end();
							++iter)
						{
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
				if(tile && tile->isBlocking() == false) {
					bool place = true;
					if(!doodad_brush->placeOnDuplicate() && !alt) {
						for(ItemVector::const_iterator iter = tile->items.begin();
							iter != tile->items.end();
							++iter)
						{
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

			for(PositionList::const_iterator iter = tilestoborder.begin();
					iter != tilestoborder.end();
					++iter)
			{
				Tile* t = map.getTile(*iter);
				if(t) {
					Tile* new_tile = t->deepCopy(map);
					new_tile->borderize(&map);
					new_tile->wallize(&map);
					action->addChange(newd Change(new_tile));
				}
			}

			batch->addAndCommitAction(action);
		}
	} else if(house_exit_brush) {
		//
		Action* action = actionQueue->createAction(batch);
		House* house = map.houses.getHouse(house_exit_brush->getHouseID());
		if(house && house_exit_brush->canDraw(&map, offset)) {
			action->addChange(Change::Create(house, offset));
		} else {
			delete action;
			return;
		}
		batch->addAndCommitAction(action);
	} else if(waypoint_brush) {
		//
		Action* action = actionQueue->createAction(batch);
		Waypoint* wp = map.waypoints.getWaypoint(waypoint_brush->getWaypoint());
		if(wp && waypoint_brush->canDraw(&map, offset)) {
			action->addChange(Change::Create(wp, offset));
		} else {
			delete action;
			return;
		}
		batch->addAndCommitAction(action);
	} else if(wall_brush) {
		Action* action = actionQueue->createAction(batch);
		// This will only occur with a size 0, when clicking on a tile (not drawing)
		Tile* t = map.getTile(offset);
		Tile* new_tile = NULL;
		if(t) {
			new_tile = t->deepCopy(map);
		} else {
			new_tile = map.allocator(map.createTileL(offset));
		}
		if(dodraw) {
			bool b = true;
			wall_brush->draw(&map, new_tile, &b);
		} else {
			wall_brush->undraw(&map, new_tile);
		}
		action->addChange(newd Change(new_tile));
		batch->addAndCommitAction(action);
	} else if(spawn_brush || creature_brush) {
		Action* action = actionQueue->createAction(batch);

		Tile* t = map.getTile(offset);
		Tile* new_tile = NULL;
		if(t) {
			new_tile = t->deepCopy(map);
		} else {
			new_tile = map.allocator(map.createTileL(offset));
		}
		int param;
		if(creature_brush) {
			param = gui.GetSpawnTime();
		} else {
			param = gui.GetBrushSize();
		}
		if(dodraw) {
			brush->draw(&map, new_tile, &param);
		} else {
			brush->undraw(&map, new_tile);
		}
		action->addChange(newd Change(new_tile));
		batch->addAndCommitAction(action);
	}

	addBatch(batch, 2);
}

void Editor::drawInternal(const PositionVector& tilestodraw, bool alt, bool dodraw)
{
	Brush* brush = gui.GetCurrentBrush();
	OptionalBorderBrush* gravel_brush = dynamic_cast<OptionalBorderBrush*>(brush);
#ifdef __DEBUG__
	GroundBrush* border_brush = dynamic_cast<GroundBrush*>(brush);
	WallBrush* wall_brush = dynamic_cast<WallBrush*>(brush);

	if(border_brush || wall_brush) {
		// Wrong function, end call
		return;
	}
#endif
	Action* action = actionQueue->createAction(ACTION_DRAW);

	if(gravel_brush) {
		// We actually need to do borders, but on the same tiles we draw to
		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			Position pos = *it;
			TileLocation* location = map.createTileL(pos);
			Tile* tile = location->get();

			if(tile) {
				if(dodraw) {
					Tile* new_tile = tile->deepCopy(map);
					brush->draw(&map, new_tile);
					new_tile->borderize(&map);
					action->addChange(newd Change(new_tile));
				} else if(dodraw == false && tile->hasOptionalBorder()) {
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
			Position pos = *it;
			TileLocation* location = map.createTileL(pos);
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
	Brush* brush = gui.GetCurrentBrush();
	GroundBrush* border_brush = dynamic_cast<GroundBrush*>(brush);
	WallBrush* wall_brush = dynamic_cast<WallBrush*>(brush);
	DoorBrush* door_brush = dynamic_cast<DoorBrush*>(brush);
	TableBrush* table_brush = dynamic_cast<TableBrush*>(brush);
	CarpetBrush* carpet_brush = dynamic_cast<CarpetBrush*>(brush);
	EraserBrush* eraser = dynamic_cast<EraserBrush*>(brush);

	if(border_brush || eraser) {
		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);

		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			Position pos = *it;
			TileLocation* location = map.createTileL(pos);
			Tile* tile = location->get();

			if(tile) {
				Tile* new_tile = tile->deepCopy(map);
				if(settings.getInteger(Config::USE_AUTOMAGIC)) {
					new_tile->cleanBorders();
				}
				if(dodraw)
					if(border_brush && alt) {
						std::pair<bool, GroundBrush*> param;
						if(replace_brush) {
							param.first = false;
							param.second = replace_brush;
						} else {
							param.first = true;
							param.second = NULL;
						}
						gui.GetCurrentBrush()->draw(&map, new_tile, &param);
					} else {
						gui.GetCurrentBrush()->draw(&map, new_tile, NULL);
					}
				else {
					gui.GetCurrentBrush()->undraw(&map, new_tile);
					tilestoborder.push_back(pos);
				}
				action->addChange(newd Change(new_tile));
			} else if(dodraw) {
				Tile* new_tile = map.allocator(location);
				if(border_brush && alt) {
					std::pair<bool, GroundBrush*> param;
					if(replace_brush) {
						param.first = false;
						param.second = replace_brush;
					} else {
						param.first = true;
						param.second = NULL;
					}
					gui.GetCurrentBrush()->draw(&map, new_tile, &param);
				} else {
					gui.GetCurrentBrush()->draw(&map, new_tile, NULL);
				}
				action->addChange(newd Change(new_tile));
			}
		}

		// Commit changes to map
		batch->addAndCommitAction(action);

		if(settings.getInteger(Config::USE_AUTOMAGIC)) {
			// Do borders!
			action = actionQueue->createAction(batch);
			for(PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
				Position pos = *it;
				TileLocation* location = map.createTileL(pos);
				Tile* tile = location->get();

				if(tile) {
					Tile* new_tile = tile->deepCopy(map);
					if(eraser) {
						new_tile->wallize(&map);
						new_tile->tableize(&map);
						new_tile->carpetize(&map);
					}
					new_tile->borderize(&map);
					action->addChange(newd Change(new_tile));
				} else {
					Tile* new_tile = map.allocator(location);
					if(eraser) {
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
	} else if(table_brush || carpet_brush) {
		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);

		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			Position pos = *it;
			TileLocation* location = map.createTileL(pos);
			Tile* tile = location->get();

			if(tile) {
				Tile* new_tile = tile->deepCopy(map);
				if(dodraw)
					gui.GetCurrentBrush()->draw(&map, new_tile, NULL);
				else
					gui.GetCurrentBrush()->undraw(&map, new_tile);
				action->addChange(newd Change(new_tile));
			} else if(dodraw) {
				Tile* new_tile = map.allocator(location);
				gui.GetCurrentBrush()->draw(&map, new_tile, NULL);
				action->addChange(newd Change(new_tile));
			}
		}

		// Commit changes to map
		batch->addAndCommitAction(action);

		// Do borders!
		action = actionQueue->createAction(batch);
		for(PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
			Position pos = *it;
			Tile* tile = map.getTile(pos);
			if(table_brush) {
				if(tile && tile->hasTable()) {
					Tile* new_tile = tile->deepCopy(map);
					new_tile->tableize(&map);
					action->addChange(newd Change(new_tile));
				}
			} else if(carpet_brush) {
				if(tile && tile->hasCarpet()) {
					Tile* new_tile = tile->deepCopy(map);
					new_tile->carpetize(&map);
					action->addChange(newd Change(new_tile));
				}
			}
		}
		batch->addAndCommitAction(action);

		addBatch(batch, 2);
	} else if(wall_brush) {
		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);

		if(alt && dodraw) {
			// This is exempt from USE_AUTOMAGIC
			gui.doodad_buffer_map->clear();
			BaseMap* draw_map = gui.doodad_buffer_map;

			for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
				Position pos = *it;
				TileLocation* location = map.createTileL(pos);
				Tile* tile = location->get();

				if(tile) {
					Tile* new_tile = tile->deepCopy(map);
					new_tile->cleanWalls(wall_brush);
					gui.GetCurrentBrush()->draw(draw_map, new_tile);
					draw_map->setTile(pos, new_tile, true);
				} else if(dodraw) {
					Tile* new_tile = map.allocator(location);
					gui.GetCurrentBrush()->draw(draw_map, new_tile);
					draw_map->setTile(pos, new_tile, true);
				}
			}
			for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
				Position pos = *it;
				// Get the correct tiles from the draw map instead of the editor map
				Tile* tile = draw_map->getTile(pos);

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
				Position pos = *it;
				TileLocation* location = map.createTileL(pos);
				Tile* tile = location->get();
				
				if(tile) {
					Tile* new_tile = tile->deepCopy(map);
					// Wall cleaning is exempt from automagic
					new_tile->cleanWalls(wall_brush);
					if(dodraw)
						gui.GetCurrentBrush()->draw(&map, new_tile);
					else
						gui.GetCurrentBrush()->undraw(&map, new_tile);
					action->addChange(newd Change(new_tile));
				} else if(dodraw) {
					Tile* new_tile = map.allocator(location);
					gui.GetCurrentBrush()->draw(&map, new_tile);
					action->addChange(newd Change(new_tile));
				}
			}

			// Commit changes to map
			batch->addAndCommitAction(action);

			if(settings.getInteger(Config::USE_AUTOMAGIC)) {
				// Do borders!
				action = actionQueue->createAction(batch);
				for(PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
					Position pos = *it;
					Tile* tile = map.getTile(pos);

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
	} else if(door_brush) {
		BatchAction* batch = actionQueue->createBatch(ACTION_DRAW);
		Action* action = actionQueue->createAction(batch);

		// Loop is kind of redundant since there will only ever be one index.
		for(PositionVector::const_iterator it = tilestodraw.begin(); it != tilestodraw.end(); ++it) {
			Position pos = *it;
			TileLocation* location = map.createTileL(pos);
			Tile* tile = location->get();

			if(tile) {
				Tile* new_tile = tile->deepCopy(map);
				// Wall cleaning is exempt from automagic
				new_tile->cleanWalls(wall_brush);
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

		if(settings.getInteger(Config::USE_AUTOMAGIC)) {
			// Do borders!
			action = actionQueue->createAction(batch);
			for(PositionVector::const_iterator it = tilestoborder.begin(); it != tilestoborder.end(); ++it) {
				Position pos = *it;
				Tile* tile = map.getTile(pos);

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
			Position pos = *it;
			TileLocation* location = map.createTileL(pos);
			Tile* tile = location->get();

			if(tile) {
				Tile* new_tile = tile->deepCopy(map);
				if(dodraw)
					gui.GetCurrentBrush()->draw(&map, new_tile);
				else
					gui.GetCurrentBrush()->undraw(&map, new_tile);
				action->addChange(newd Change(new_tile));
			} else if(dodraw) {
				Tile* new_tile = map.allocator(location);
				gui.GetCurrentBrush()->draw(&map, new_tile);
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
	return live_client != NULL;
}

bool Editor::IsLiveServer() const
{
	return live_server != NULL;
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

void Editor::BroadcastNodes(DirtyList& dirty_list)
{
	if(IsLiveClient())
		live_client->SendChanges(dirty_list);
	else
		live_server->BroadcastNodes(dirty_list);
}

void Editor::CloseLiveServer()
{
	ASSERT(IsLive());
	
	if(live_client)
	{
		live_client->Close();

		live_client = NULL;
	}
	else if(live_server)
	{
		live_server->Close();
		live_server = NULL;

		delete actionQueue;
		actionQueue = newd ActionQueue(*this);
	}
}

void Editor::QueryNode(int ndx, int ndy, bool underground)
{
	ASSERT(live_client);
	live_client->QueryNode(ndx, ndy, underground);
}

void Editor::SendNodeRequests()
{
	if(live_client) {
		live_client->SendNodeRequests();
	}
}

