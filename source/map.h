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

#ifndef RME_MAP_H_
#define RME_MAP_H_

#include "basemap.h"
#include "tile.h"
#include "town.h"
#include "house.h"
#include "spawn_monster.h"
#include "complexitem.h"
#include "waypoints.h"
#include "templates.h"
#include "spawn_npc.h"

class Map : public BaseMap
{
public:
	// ctor and dtor
	Map();
	virtual ~Map();

	// Operations on the entire map
	void cleanInvalidTiles(bool showdialog = false);
	// Save a bmp image of the minimap
	bool exportMinimap(FileName filename, int floor = GROUND_LAYER, bool showdialog = false);
	//
	bool convert(MapVersion to, bool showdialog = false);
	bool convert(const ConversionMap& cm, bool showdialog = false);


	// Query information about the map

	MapVersion getVersion() const;
	// Returns true if any change has been done since last save
	bool hasChanged() const;
	// Makes a change, doesn't matter what. Just so that it asks when saving (Also adds a * to the window title)
	bool doChange();
	// Clears any changes
	bool clearChanges();

	// Errors/warnings
	bool hasWarnings() const {return warnings.size() != 0;}
	const wxArrayString& getWarnings() const {return warnings;}
	bool hasError() const {return error.size() != 0;}
	wxString getError() const {return error;}

	// Mess with spawnsMonster
	bool addSpawnMonster(Tile* spawnMonster);
	void removeSpawnMonster(Tile* tile);
	void removeSpawnMonster(const Position& position) { removeSpawnMonster(getTile(position)); }

	// Returns all possible spawnsMonster on the target tile
	SpawnMonsterList getSpawnMonsterList(Tile* t);
	SpawnMonsterList getSpawnMonsterList(const Position& position) { return getSpawnMonsterList(getTile(position)); }
	SpawnMonsterList getSpawnMonsterList(int32_t x, int32_t y, int32_t z) { return getSpawnMonsterList(getTile(x, y, z)); }

	// Mess with npc spawns
	bool addSpawnNpc(Tile* spawnMonster);
	void removeSpawnNpc(Tile* tile);
	void removeSpawnNpc(const Position& position) { removeSpawnNpc(getTile(position)); }

	// Returns all possible npc spawns on the target tile
	SpawnNpcList getSpawnNpcList(Tile* t);
	SpawnNpcList getSpawnNpcList(const Position& position) { return getSpawnNpcList(getTile(position)); }
	SpawnNpcList getSpawnNpcList(int32_t x, int32_t y, int32_t z) { return getSpawnNpcList(getTile(x, y, z)); }

	// Returns true if the map has been saved
	// ie. it knows which file it should be saved to
	bool hasFile() const;
	std::string getFilename() const {return filename;}
	std::string getName() const {return name;}
	void setName(const std::string& n) {name = n;}

	// Get map data
	int getWidth() const {return width;}
	int getHeight() const {return height;}
	std::string getMapDescription() const {return description;}
	std::string getHouseFilename() const {return housefile;}
	std::string getSpawnFilename() const {return spawnmonsterfile;}
	std::string getSpawnNpcFilename() const {return spawnnpcfile;}

	// Set some map data
	void setWidth(int new_width);
	void setHeight(int new_height);
	void setMapDescription(const std::string& new_description);
	void setHouseFilename(const std::string& new_housefile);
	void setSpawnMonsterFilename(const std::string& new_spawnmonsterfile);
	void setSpawnNpcFilename(const std::string& new_npcfile);

	void flagAsNamed() {unnamed = false;}

protected:
	// Loads a map
	bool open(const std::string identifier);

protected:
	void removeSpawnMonsterInternal(Tile* tile);
	void removeSpawnNpcInternal(Tile* tile);

	wxArrayString warnings;
	wxString error;

	std::string name; // The map name, NOT the same as filename
	std::string filename; // the maps filename
	std::string description; // The description of the map

	MapVersion mapVersion;

	// Map Width and Height - for info purposes
	uint16_t width, height;

	std::string spawnmonsterfile; // The maps spawnmonsterfile
	std::string spawnnpcfile; // The maps spawnnpcfile
	std::string housefile; // The housefile

public:
	Towns towns;
	Houses houses;
	SpawnsMonster spawnsMonster;
	SpawnsNpc spawnsNpc;

protected:
	bool has_changed; // If the map has changed
	bool unnamed; // If the map has yet to receive a name

	friend class IOMapOTBM;
	friend class IOMapOTMM;
	friend class Editor;

public:
	Waypoints waypoints;
};

template <typename ForeachType>
inline void foreach_ItemOnMap(Map& map, ForeachType& foreach, bool selectedTiles)
{
	MapIterator tileiter = map.begin();
	MapIterator end = map.end();
	long long done = 0;

	while(tileiter != end) {
		++done;
		Tile* tile = (*tileiter)->get();
		if(selectedTiles && !tile->isSelected()) {
			++tileiter;
			continue;
		}

		if(tile->ground) {
			foreach(map, tile, tile->ground, done);
		}

		std::queue<Container*> containers;
		for(ItemVector::iterator itemiter = tile->items.begin(); itemiter != tile->items.end(); ++itemiter) {
			Item* item = *itemiter;
			Container* container = dynamic_cast<Container*>(item);
			foreach(map, tile, item, done);
			if(container) {
				containers.push(container);

				do {
					container = containers.front();
					ItemVector& v = container->getVector();
					for(ItemVector::iterator containeriter = v.begin(); containeriter != v.end(); ++containeriter) {
						Item* i = *containeriter;
						Container* c = dynamic_cast<Container*>(i);
						foreach(map, tile, i, done);
						if(c) {
							containers.push(c);
						}
					}
					containers.pop();
				} while(containers.size());
			}
		}
		++tileiter;
	}
}

template <typename ForeachType>
inline void foreach_TileOnMap(Map& map, ForeachType& foreach)
{
	MapIterator tileiter = map.begin();
	MapIterator end = map.end();
	long long done = 0;

	while(tileiter != end)
		foreach(map, (*tileiter++)->get(), ++done);
}

template <typename RemoveIfType>
inline long long remove_if_TileOnMap(Map& map, RemoveIfType& remove_if)
{
	MapIterator tileiter = map.begin();
	MapIterator end = map.end();
	long long done = 0;
	long long removed = 0;
	long long total = map.getTileCount();

	while(tileiter != end) {
		Tile* tile = (*tileiter)->get();
		if(remove_if(map, tile, removed, done, total)) {
			map.setTile(tile->getPosition(), nullptr, true);
			++removed;
		}
		++tileiter;
		++done;
	}

	return removed;
}

template <typename RemoveIfType>
inline int64_t RemoveItemOnMap(Map& map, RemoveIfType& condition, bool selectedOnly) {
	int64_t done = 0;
	int64_t removed = 0;

	MapIterator it = map.begin();
	MapIterator end = map.end();

	while(it != end) {
		++done;
		Tile* tile = (*it)->get();
		if(selectedOnly && !tile->isSelected()) {
			++it;
			continue;
		}

		if(tile->ground) {
			if(condition(map, tile->ground, removed, done)) {
				delete tile->ground;
				tile->ground = nullptr;
				++removed;
			}
		}

		for(auto iit = tile->items.begin(); iit != tile->items.end();) {
			Item* item = *iit;
			if(condition(map, item, removed, done)) {
				iit = tile->items.erase(iit);
				delete item;
				++removed;
			}
			else
				++iit;
		}
		++it;
	}
	return removed;
}

#endif
