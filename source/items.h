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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/items.h $
// $Id: items.h 298 2010-02-23 17:09:13Z admin $

#ifndef RME_ITEMS_H_
#define RME_ITEMS_H_

#include "filehandle.h"
#include "brush_enums.h"

class Brush;
class GroundBrush;
class WallBrush;
class CarpetBrush;
class TableBrush;
class HouseBrush;
class HouseExitBrush;
class OptionalBorderBrush;
class EraserBrush;
class SpawnBrush;
class DoorBrush;
class FlagBrush;
class RAWBrush;

class ItemType;
class GameSprite;
class GameSprite;
class ItemDatabase;

extern ItemDatabase g_items;

typedef uint8_t attribute_t;
typedef uint32_t flags_t;
typedef uint16_t datasize_t;

enum ItemGroup_t {
	ITEM_GROUP_NONE = 0,
	ITEM_GROUP_GROUND,
	ITEM_GROUP_CONTAINER,
	ITEM_GROUP_WEAPON,
	ITEM_GROUP_AMMUNITION,
	ITEM_GROUP_ARMOR,
	ITEM_GROUP_RUNE,
	ITEM_GROUP_TELEPORT,
	ITEM_GROUP_MAGICFIELD,
	ITEM_GROUP_WRITEABLE,
	ITEM_GROUP_KEY,
	ITEM_GROUP_SPLASH,
	ITEM_GROUP_FLUID,
	ITEM_GROUP_DOOR,
	ITEM_GROUP_DEPRECATED,
	ITEM_GROUP_LAST
};

enum ItemTypes_t {
	ITEM_TYPE_NONE = 0,
	ITEM_TYPE_DEPOT,
	ITEM_TYPE_MAILBOX,
	ITEM_TYPE_TRASHHOLDER,
	ITEM_TYPE_CONTAINER,
	ITEM_TYPE_DOOR,
	ITEM_TYPE_MAGICFIELD,
	ITEM_TYPE_TELEPORT,
	ITEM_TYPE_BED,
	ITEM_TYPE_KEY,
	ITEM_TYPE_LAST
};

/////////OTB specific//////////////

enum rootattrib_t{
	ROOT_ATTR_VERSION = 0x01
};

enum itemattrib_t {
	ITEM_ATTR_FIRST = 0x10,
	ITEM_ATTR_SERVERID = ITEM_ATTR_FIRST,
	ITEM_ATTR_CLIENTID,
	ITEM_ATTR_NAME,
	ITEM_ATTR_DESCR,
	ITEM_ATTR_SPEED,
	ITEM_ATTR_SLOT,
	ITEM_ATTR_MAXITEMS,
	ITEM_ATTR_WEIGHT,
	ITEM_ATTR_WEAPON,
	ITEM_ATTR_AMU,
	ITEM_ATTR_ARMOR,
	ITEM_ATTR_MAGLEVEL,
	ITEM_ATTR_MAGFIELDTYPE,
	ITEM_ATTR_WRITEABLE,
	ITEM_ATTR_ROTATETO,
	ITEM_ATTR_DECAY,
	ITEM_ATTR_SPRITEHASH,
	ITEM_ATTR_MINIMAPCOLOR,
	ITEM_ATTR_07,
	ITEM_ATTR_08,
	ITEM_ATTR_LIGHT,

	//1-byte aligned
	ITEM_ATTR_DECAY2,
	ITEM_ATTR_WEAPON2,
	ITEM_ATTR_AMU2,
	ITEM_ATTR_ARMOR2,
	ITEM_ATTR_WRITEABLE2,
	ITEM_ATTR_LIGHT2,

	ITEM_ATTR_TOPORDER,

	ITEM_ATTR_WRITEABLE3,

	ITEM_ATTR_LAST
};

enum itemflags_t {
	FLAG_UNPASSABLE = 1 << 0,
	FLAG_BLOCK_MISSILES = 1 << 1,
	FLAG_BLOCK_PATHFINDER = 1 << 2,
	FLAG_HAS_ELEVATION = 1 << 3,
	FLAG_USEABLE = 1 << 4,
	FLAG_PICKUPABLE = 1 << 5,
	FLAG_MOVEABLE = 1 << 6,
	FLAG_STACKABLE = 1 << 7,
	FLAG_FLOORCHANGEDOWN = 1 << 8,
	FLAG_FLOORCHANGENORTH = 1 << 9,
	FLAG_FLOORCHANGEEAST = 1 << 10,
	FLAG_FLOORCHANGESOUTH = 1 << 11,
	FLAG_FLOORCHANGEWEST = 1 << 12,
	FLAG_ALWAYSONTOP = 1 << 13,
	FLAG_READABLE = 1 << 14,
	FLAG_ROTABLE = 1 << 15,
	FLAG_HANGABLE = 1 << 16,
	FLAG_HOOK_EAST = 1 << 17,
	FLAG_HOOK_SOUTH = 1 << 18,
	FLAG_CANNOTDECAY = 1 << 19,
	FLAG_ALLOWDISTREAD = 1 << 20,
	FLAG_UNUSED = 1 << 21,
	FLAG_CLIENTCHARGES = 1 << 22,
	FLAG_IGNORE_LOOK = 1 << 23
};

enum slotsOTB_t{
	OTB_SLOT_DEFAULT,
	OTB_SLOT_HEAD,
	OTB_SLOT_BODY,
	OTB_SLOT_LEGS,
	OTB_SLOT_BACKPACK,
	OTB_SLOT_WEAPON,
	OTB_SLOT_2HAND,
	OTB_SLOT_FEET,
	OTB_SLOT_AMULET,
	OTB_SLOT_RING,
	OTB_SLOT_HAND,
};

enum ShootTypeOtb_t {
	OTB_SHOOT_NONE          = 0,
	OTB_SHOOT_BOLT          = 1,
	OTB_SHOOT_ARROW         = 2,
	OTB_SHOOT_FIRE          = 3,
	OTB_SHOOT_ENERGY        = 4,
	OTB_SHOOT_POISONARROW   = 5,
	OTB_SHOOT_BURSTARROW    = 6,
	OTB_SHOOT_THROWINGSTAR  = 7,
	OTB_SHOOT_THROWINGKNIFE = 8,
	OTB_SHOOT_SMALLSTONE    = 9,
	OTB_SHOOT_SUDDENDEATH   = 10,
	OTB_SHOOT_LARGEROCK     = 11,
	OTB_SHOOT_SNOWBALL      = 12,
	OTB_SHOOT_POWERBOLT     = 13,
	OTB_SHOOT_SPEAR         = 14,
	OTB_SHOOT_POISONFIELD   = 15,
	OTB_SHOOT_INFERNALBOLT  = 16
};

//1-byte aligned structs
#pragma pack(1)

struct VERSIONINFO {
	uint32_t dwMajorVersion;
	uint32_t dwMinorVersion;
	uint32_t dwBuildNumber;
	uint8_t CSDVersion[128];
};

struct decayBlock2 {
	uint16_t decayTo;
	uint16_t decayTime;
};

struct weaponBlock2 {
	uint8_t weaponType;
	uint8_t amuType;
	uint8_t shootType;
	uint8_t attack;
	uint8_t defence;
};

struct amuBlock2 {
	uint8_t amuType;
	uint8_t shootType;
	uint8_t attack;
};

struct armorBlock2 {
	uint16_t armor;
	double weight;
	uint16_t slot_position;
};

struct writeableBlock2 {
	uint16_t readOnlyId;
};

struct lightBlock2 {
	uint16_t lightLevel;
	uint16_t lightColor;
};

struct writeableBlock3 {
	uint16_t readOnlyId;
	uint16_t maxTextLen;
};

#pragma pack()

class ItemType {
private:
	ItemType(const ItemType&) {}

public:
	ItemType();
	~ItemType();

	bool isGroundTile() const { return (group == ITEM_GROUP_GROUND); }
	bool isSplash() const { return (group == ITEM_GROUP_SPLASH); }
	bool isFluidContainer() const { return (group == ITEM_GROUP_FLUID); }
	
	bool isClientCharged() const { return client_chargeable; }
	bool isExtraCharged() const { return !client_chargeable && extra_chargeable; }

	bool isDepot() const { return (type == ITEM_TYPE_DEPOT); }
	bool isMailbox() const { return (type == ITEM_TYPE_MAILBOX); }
	bool isTrashHolder() const { return (type == ITEM_TYPE_TRASHHOLDER); }
	bool isContainer() const { return (type == ITEM_TYPE_CONTAINER); }
	bool isDoor() const { return (type == ITEM_TYPE_DOOR); }
	bool isMagicField() const { return (type == ITEM_TYPE_MAGICFIELD); }
	bool isTeleport() const { return (type == ITEM_TYPE_TELEPORT); }
	bool isBed() const { return (type == ITEM_TYPE_BED); }
	bool isKey() const { return (type == ITEM_TYPE_KEY); }

	bool isStackable() const { return stackable; }
	bool isMetaItem() const { return is_metaitem; }

	bool isFloorChange() const;

	GameSprite* sprite;
	uint16_t id;
	uint16_t clientID;
	Brush* brush;
	Brush* doodad_brush;
	RAWBrush* raw_brush;
	bool is_metaitem;
	// This is needed as a consequence of the item palette & the raw palette
	// using the same brushes ("others" category consists of items with this
	// flag set to false)
	bool has_raw;
	bool in_other_tileset;

	ItemGroup_t group;
	ItemTypes_t type;

	uint16_t volume;
	uint16_t maxTextLen;
	//uint16_t writeOnceItemId;
	uint16_t ground_equivalent;
	uint32_t border_group;
	bool has_equivalent; // True if any item has this as ground_equivalent
	bool wall_hate_me; // (For wallbrushes, regard this as not part of the wall)

	std::string name;
	std::string editorsuffix;
	std::string description;

	float weight;
	// It might be useful to be able to extrapolate this information in the future
	int attack;
	int defense;
	int armor;
	uint32_t charges;
	bool client_chargeable;
	bool extra_chargeable;
	bool ignoreLook;

	bool isHangable;
	bool hookEast;
	bool hookSouth;
	bool canReadText;
	bool canWriteText;
	bool allowDistRead;
	bool replaceable;
	bool decays;

	bool stackable;
	bool moveable;
	bool alwaysOnBottom;
	bool pickupable;
	bool rotable;
	bool isBorder;
	bool isOptionalBorder;
	bool isWall;
	bool isBrushDoor;
	bool isOpen;
	bool isTable;
	bool isCarpet;

	bool floorChangeDown;
	bool floorChangeNorth;
	bool floorChangeSouth;
	bool floorChangeEast;
	bool floorChangeWest;
	bool floorChange;

	bool unpassable;
	bool blockPickupable;
	bool blockMissiles;
	bool blockPathfinder;
	bool hasElevation;

	int alwaysOnTopOrder;
	int rotateTo;
	BorderType border_alignment;
};

class ItemDatabase
{
public:
	ItemDatabase();
	~ItemDatabase();

	void clear();

	ItemType& operator[](size_t id) {return getItemType(id);}
	uint16_t getMaxID() const {return max_item_id;}

	bool typeExists(int id) const;
	ItemType& getItemType(int id);
	ItemType& getItemIdByClientID(int spriteId);

	bool loadFromOtb(const FileName& datafile, wxString& error, wxArrayString& warnings);
	bool loadFromGameXml(const FileName& datafile, wxString& error, wxArrayString& warnings);
	bool loadItemFromGameXml(pugi::xml_node itemNode, int id);
	bool loadMetaItem(pugi::xml_node node);

	//typedef std::map<int32_t, ItemType*> ItemMap;
	typedef contigous_vector<ItemType*> ItemMap;
	typedef std::map<std::string, ItemType*> ItemNameMap;
	ItemMap items;

	// Version information
	uint32_t MajorVersion;
	uint32_t MinorVersion;
	uint32_t BuildNumber;

protected:
	bool loadFromOtbVer1(BinaryNode* itemNode, wxString& error, wxArrayString& warnings);
	bool loadFromOtbVer2(BinaryNode* itemNode, wxString& error, wxArrayString& warnings);
	bool loadFromOtbVer3(BinaryNode* itemNode, wxString& error, wxArrayString& warnings);

protected:
	// Count of GameSprite types
	uint16_t item_count;
	uint16_t effect_count;
	uint16_t monster_count;
	uint16_t distance_count;

	uint16_t minclientID;
	uint16_t maxclientID;
	uint16_t max_item_id;

	friend class GameSprite;
	friend class Item;
};

#endif
