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

#ifndef RME_OTMM_IOMAP_H_
#define RME_OTMM_IOMAP_H_

#include "iomap.h"

#pragma pack(1)

enum OTMM_VERSION {
	OTMM_VERSION_1,
};

enum OTMM_TileAttributes {
	OTMM_ATTR_TILE_FLAGS = 1,
};

enum OTMM_ItemAttribute {
	OTMM_ATTR_DESCRIPTION = 1,
	OTMM_ATTR_ACTION_ID = 2,
	OTMM_ATTR_UNIQUE_ID = 3,
	OTMM_ATTR_TEXT = 4,
	OTMM_ATTR_DESC = 5,
	OTMM_ATTR_TELE_DEST = 6,
	OTMM_ATTR_ITEM = 7,
	OTMM_ATTR_SUBTYPE = 8,
	OTMM_ATTR_DEPOT_ID = 9,
	OTMM_ATTR_DOOR_ID = 10,
	OTMM_ATTR_DURATION = 11,
};

enum OTMM_NodeTypes {
	OTMM_ROOT = 1,
	OTMM_MAP_DATA = 2,
	OTMM_TILE_DATA = 3,
	OTMM_TILE = 4,
	OTMM_HOUSETILE = 5,
	OTMM_ITEM = 6,
	OTMM_SPAWN_DATA = 7,
	OTMM_SPAWN_AREA = 8,
	OTMM_MONSTER = 9,
	OTMM_NPC = 10,
	OTMM_TOWN_DATA = 11,
	OTMM_TOWN = 12,
	OTMM_HOUSE_DATA = 13,
	OTMM_HOUSE = 14,
	OTMM_DESCRIPTION = 15,
	OTMM_EDITOR = 16,
};

#pragma pack()

class NodeFileReadHandle;
class NodeFileWriteHandle;

class IOMapOTMM : public IOMap {
public:
	IOMapOTMM();
	~IOMapOTMM();

	virtual MapVersion getVersionInfo(const FileName& identifier);

	virtual bool loadMap(Map& map, NodeFileReadHandle& handle, const FileName& identifier, bool showdialog);
	virtual bool loadMap(Map& map, const FileName& identifier, bool showdialog);

	virtual bool saveMap(Map& map, NodeFileWriteHandle& handle, const FileName& identifier, bool showdialog);
	virtual bool saveMap(Map& map, const FileName& identifier, bool showdialog);
};

#endif
