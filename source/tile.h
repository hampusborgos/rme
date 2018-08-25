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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/tile.h $
// $Id: tile.h 298 2010-02-23 17:09:13Z admin $

#ifndef RME_TILE_H
#define RME_TILE_H

#include "position.h"
#include "item.h"
#include "map_region.h"

enum {
	TILESTATE_NONE           = 0x0000,
	TILESTATE_PROTECTIONZONE = 0x0001,
	TILESTATE_DEPRECATED     = 0x0002, // Reserved
	TILESTATE_NOPVP          = 0x0004,
	TILESTATE_NOLOGOUT       = 0x0008,
	TILESTATE_PVPZONE        = 0x0010,
	TILESTATE_REFRESH        = 0x0020,
	// Internal
	TILESTATE_SELECTED  = 0x0001,
	TILESTATE_UNIQUE    = 0x0002,
	TILESTATE_BLOCKING  = 0x0004,
	TILESTATE_OP_BORDER = 0x0008, // If this is true, gravel will be placed on the tile!
	TILESTATE_HAS_TABLE = 0x0010,
	TILESTATE_HAS_CARPET= 0x0020,
	TILESTATE_MODIFIED  = 0x0040,
};

enum : uint8_t {
	INVALID_MINIMAP_COLOR = 0xFF
};

class Tile
{
public: // Members
	TileLocation*  location;
	Item*          ground;
	ItemVector     items;
	Creature*      creature;
	Spawn*         spawn;
	uint32_t house_id; // House id for this tile (pointer not safe)

public:
	// ALWAYS use this constructor if the Tile is EVER going to be placed on a map
	Tile(TileLocation& location);
	// Use this when the tile is only used internally by the editor (like in certain brushes)
	Tile(int x, int y, int z);

	~Tile();

	// Argument is a the map to allocate the tile from
	Tile* deepCopy(BaseMap& map);

	// The location of the tile
	// Stores state that remains between the tile being moved (like house exits)
	void setLocation(TileLocation* where) {location = where;}
	TileLocation* getLocation() {return location;}
	const TileLocation* getLocation() const {return location;}

	// Position of the tile
	Position getPosition() {return location->getPosition();}
	const Position getPosition() const {return location->getPosition();}
	int getX() const {return location->getPosition().x;}
	int getY() const {return location->getPosition().y;}
	int getZ() const {return location->getPosition().z;}

public: //Functions
	// Absorb the other tile into this tile
	void merge(Tile* other);

	// Has tile been modified since the map was loaded/created?
	bool isModified() const { return testFlags(statflags, TILESTATE_MODIFIED); }
	void modify() { statflags |= TILESTATE_MODIFIED; }
	void unmodify() { statflags &= ~TILESTATE_MODIFIED; }

	// Get memory footprint size
	uint32_t memsize() const;
	// Get number of items on the tile
	bool empty() const { return size() == 0; }
	int size() const;

	// Blocking?
	bool isBlocking() const { return testFlags(statflags, TILESTATE_BLOCKING); }

	// PZ
	bool isPZ() const { return testFlags(mapflags, TILESTATE_PROTECTIONZONE); }
	void setPZ(bool pz) {
		if(pz) {
			mapflags |= TILESTATE_PROTECTIONZONE;
		} else {
			mapflags &= ~TILESTATE_PROTECTIONZONE;
		}
	}

	bool hasItemId(uint16_t itemId) const;
	bool hasProperty(enum ITEMPROPERTY prop) const;

	Item* getTopItem(); // Returns the topmost item, or nullptr if the tile is empty
	void addItem(Item* item);

	void select();
	void deselect();
	// This selects borders too
	void selectGround();
	void deselectGround();

	bool isSelected() const { return testFlags(statflags, TILESTATE_SELECTED); }
	bool hasUniqueItem() const { return testFlags(statflags, TILESTATE_UNIQUE); }

	ItemVector popSelectedItems();
	ItemVector getSelectedItems();
	Item* getTopSelectedItem();

	// Refresh internal flags (such as selected etc.)
	void update();

	uint8_t getMiniMapColor() const;

	// Does this tile have ground?
	bool hasGround() const { return ground != nullptr; }
	bool hasBorders() const {
		return items.size() && items[0]->isBorder();
	}

	// Get the border brush of this tile
	GroundBrush* getGroundBrush() const;

	// Remove all borders (for autoborder)
	void cleanBorders();

	// Add a border item (added at the bottom of all items)
	void addBorderItem(Item* item);

	// Borderize this tile
	void borderize(BaseMap* parent);

	bool hasTable() const { return testFlags(statflags, TILESTATE_HAS_TABLE); }
	Item* getTable() const;

	bool hasCarpet() const { return testFlags(statflags, TILESTATE_HAS_CARPET); }
	Item* getCarpet() const;

	bool hasOptionalBorder() const { return testFlags(statflags, TILESTATE_OP_BORDER); }
	void setOptionalBorder(bool b) {
		if(b) {
			statflags |= TILESTATE_OP_BORDER;
		} else {
			statflags &= ~TILESTATE_OP_BORDER;
		}
	}

	// Get the (first) wall of this tile
	Item* getWall() const;
	bool hasWall() const;
	// Remove all walls from the tile (for autowall) (only of those belonging to the specified brush
	void cleanWalls(WallBrush* wb);
	// Remove all walls from the tile
	void cleanWalls(bool dontdelete = false);
	// Add a wall item (same as just addItem, but an additional check to verify that it is a wall)
	void addWallItem(Item* item);
	// Wallize (name sucks, I know) this tile
	void wallize(BaseMap* parent);
	// Remove all tables from this tile
	void cleanTables(bool dontdelete = false);
	// Tableize (name sucks even worse, I know) this tile
	void tableize(BaseMap* parent);
	// Carpetize (name sucks even worse than last one, I know) this tile
	void carpetize(BaseMap* parent);

	// Has to do with houses
	bool isHouseTile() const;
	uint32_t getHouseID() const;
	void addHouseExit(House* h);
	void removeHouseExit(House* h);
	bool isHouseExit() const;
	const HouseExitList* getHouseExits() const;
	HouseExitList* getHouseExits();
	bool hasHouseExit(uint32_t exit) const;
	void setHouse(House* house);

	// Mapflags (PZ, PVPZONE etc.)
	void setMapFlags(uint16_t _flags);
	void unsetMapFlags(uint16_t _flags);
	uint16_t getMapFlags() const;

	// Statflags (You really ought not to touch this)
	void setStatFlags(uint16_t _flags);
	void unsetStatFlags(uint16_t _flags);
	uint16_t getStatFlags() const;

protected:
	union {
		struct {
			uint16_t mapflags;
			uint16_t statflags;
		};
		uint32_t flags;
	};

private:
	uint8_t minimapColor;

	Tile(const Tile& tile); // No copy
	Tile& operator=(const Tile& i);// Can't copy
	Tile& operator==(const Tile& i);// Can't compare
};

bool tilePositionLessThan(const Tile* a, const Tile* b);
// This sorts them by draw order
bool tilePositionVisualLessThan(const Tile* a, const Tile* b);

typedef std::vector<Tile*> TileVector;
typedef std::list<Tile*> TileList;

inline bool Tile::hasWall() const {
	return getWall() != nullptr;
}

inline bool Tile::isHouseTile() const {
	return house_id != 0;
}

inline uint32_t Tile::getHouseID() const {
	return house_id;
}

inline HouseExitList* Tile::getHouseExits() {
	return location->getHouseExits();
}

inline const HouseExitList* Tile::getHouseExits() const {
	return location->getHouseExits();
}

inline bool Tile::isHouseExit() const {
	const HouseExitList* house_exits = getHouseExits();
	if(house_exits)
		return !house_exits->empty();
	return false;
}

inline bool Tile::hasHouseExit(uint32_t exit) const {
	const HouseExitList* house_exits = getHouseExits();
	if(house_exits) {
		for(HouseExitList::const_iterator iter = house_exits->begin(); iter != house_exits->end(); ++iter) {
			if(*iter == exit) {
				return true;
			}
		}
	}
	return false;
}

inline void Tile::setMapFlags(uint16_t _flags) {
	mapflags = _flags | mapflags;
}

inline void Tile::unsetMapFlags(uint16_t _flags) {
	mapflags &= ~_flags;
}

inline uint16_t Tile::getMapFlags() const {
	return mapflags;
}

inline void Tile::setStatFlags(uint16_t _flags) {
	statflags = _flags | statflags;
}

inline void Tile::unsetStatFlags(uint16_t _flags) {
	statflags &= ~_flags;
}

inline uint16_t Tile::getStatFlags() const {
	return statflags;
}

#endif
