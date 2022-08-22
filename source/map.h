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
#include "spawn.h"
#include "complexitem.h"
#include "waypoints.h"
#include "templates.h"

class Map : public BaseMap
{
public:
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

	const MapVersion& getVersion() const { return m_version; }
	// Returns true if any change has been done since last save
	bool hasChanged() const { return m_hasChanged; }
	// Makes a change, doesn't matter what. Just so that it asks when saving (Also adds a * to the window title)
	bool doChange();
	// Clears any changes
	bool clearChanges();

	// Errors/warnings
	bool hasWarnings() const { return !m_warnings.empty(); }
	const wxArrayString& getWarnings() const noexcept { return m_warnings; }
	bool hasError() const { return !m_error.empty(); }
	const wxString& getError() const noexcept { return m_error; }

	// Mess with spawns
	bool addSpawn(Tile* spawn);
	void removeSpawn(Tile* tile);
	void removeSpawn(const Position& position) { removeSpawn(getTile(position)); }

	// Returns all possible spawns on the target tile
	SpawnList getSpawnList(Tile* t);
	SpawnList getSpawnList(const Position& position) { return getSpawnList(getTile(position)); }
	SpawnList getSpawnList(int32_t x, int32_t y, int32_t z) { return getSpawnList(getTile(x, y, z)); }

	// Returns true if the map has been saved
	// ie. it knows which file it should be saved to
	bool hasFile() const { return !m_filename.empty(); }
	const std::string& getFile() const noexcept { return m_filename; }
	const std::string& getName() const noexcept { return m_name; }
	void setName(const std::string& name) { m_name = name; }
	void setFile(const std::string& file) { m_filename = file; }

	// Get map data
	uint16_t getWidth() const noexcept { return m_width; }
	uint16_t getHeight() const noexcept { return m_height; }
	const std::string& getDescription() const noexcept { return m_description; }
	const std::string& getHouseFile() const noexcept { return m_housefile; }
	const std::string& getSpawnFile() const noexcept { return m_spawnfile; }

	Towns& getTowns() { return m_towns; }
	const Towns& getTowns() const { return m_towns; }
	Houses& getHouses() { return m_houses; }
	const Houses& getHouses() const { return m_houses; }
	Spawns& getSpawns() { return m_spawns; }
	const Spawns& getSpawns() const { return m_spawns; }
	Waypoints& getWaypoints() { return m_waypoints; }
	const Waypoints& getWaypoints() const { return m_waypoints; }

	// Set some map data
	void setWidth(int width);
	void setHeight(int height);
	void setDescription(const std::string& description);
	void setHouseFile(const std::string& file);
	void setSpawnFile(const std::string& file);
	void setUnnamed(bool unnamed) { m_unnamed = unnamed; }

	bool isUnnamed() const noexcept { return m_unnamed; }

protected:
	// Loads a map
	bool open(const std::string identifier);

protected:
	void removeSpawnInternal(Tile* tile);
	
	friend class IOMapOTBM;
	friend class IOMapOTMM;
	friend class Editor;

private:
	std::string m_name; // The map name, NOT the same as filename
	std::string m_filename; // the maps filename
	std::string m_description; // The description of the map

	MapVersion m_version;

	// Map Width and Height - for info purposes
	uint16_t m_width, m_height;

	std::string m_spawnfile; // The maps spawnfile
	std::string m_housefile; // The housefile

	bool m_hasChanged; // If the map has changed
	bool m_unnamed; // If the map has yet to receive a name

	Towns m_towns;
	Houses m_houses;
	Spawns m_spawns;
	Waypoints m_waypoints;

	wxArrayString m_warnings;
	wxString m_error;
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
