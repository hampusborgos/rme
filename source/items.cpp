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

#include "materials.h"
#include "gui.h"
#include <string.h> // memcpy

#include "items.h"
#include "item.h"

ItemDatabase g_items;

ItemType::ItemType() :
	sprite(nullptr),
	id(0),
	clientID(0),
	brush(nullptr),
	doodad_brush(nullptr),
	raw_brush(nullptr),
	is_metaitem(false),
	has_raw(false),
	in_other_tileset(false),
	group(ITEM_GROUP_NONE),
	type(ITEM_TYPE_NONE),
	volume(0),
	maxTextLen(0),
	//writeOnceItemID(0),
	ground_equivalent(0),
	border_group(0),
	has_equivalent(false),
	wall_hate_me(false),
	name(""),
	description(""),
	weight(0.0f),
	attack(0),
	defense(0),
	armor(0),
	charges(0),
	client_chargeable(false),
	extra_chargeable(false),
	ignoreLook(false),

	isHangable(false),
	hookEast(false),
	hookSouth(false),
	canReadText(false),
	canWriteText(false),
	replaceable(true),
	decays(false),
	stackable(false),
	moveable(true),
	alwaysOnBottom(false),
	pickupable(false),
	rotable(false),
	isBorder(false),
	isOptionalBorder(false),
	isWall(false),
	isBrushDoor(false),
	isOpen(false),
	isTable(false),
	isCarpet(false),

	floorChangeDown(false),
	floorChangeNorth(false),
	floorChangeSouth(false),
	floorChangeEast(false),
	floorChangeWest(false),
	floorChange(false),

	unpassable(false),
	blockPickupable(false),
	blockMissiles(false),
	blockPathfinder(false),
	hasElevation(false),

	alwaysOnTopOrder(0),
	rotateTo(0),
	border_alignment(BORDER_NONE)
{
	////
}

ItemType::~ItemType()
{
	////
}

bool ItemType::isFloorChange() const
{
	return floorChange || floorChangeDown || floorChangeNorth || floorChangeSouth || floorChangeEast || floorChangeWest;
}

ItemDatabase::ItemDatabase() :
	// Version information
	MajorVersion(0),
	MinorVersion(0),
	BuildNumber(0),

	// Count of GameSprite types
	item_count(0),
	effect_count(0),
	monster_count(0),
	distance_count(0),

	minclientID(0),
	maxclientID(0),

	max_item_id(0)
{
	////
}

ItemDatabase::~ItemDatabase()
{
	clear();
}

void ItemDatabase::clear()
{
	for(uint32_t i = 0; i < items.size(); i++) {
		delete items[i];
		items.set(i, nullptr);
	}
}

bool ItemDatabase::loadFromOtbVer1(BinaryNode* itemNode, wxString& error, wxArrayString& warnings)
{
	uint8_t u8;

	for( ; itemNode != nullptr; itemNode = itemNode->advance()) {
		if(!itemNode->getU8(u8)) {
			// Invalid!
			warnings.push_back("Invalid item type encountered...");
			continue;
		}

		if(u8 == ITEM_GROUP_DEPRECATED)
			continue;

		ItemType* t = newd ItemType();
		t->group = ItemGroup_t(u8);

		switch(t->group) {
			case ITEM_GROUP_NONE:
			case ITEM_GROUP_GROUND:
			case ITEM_GROUP_SPLASH:
			case ITEM_GROUP_FLUID:
			case ITEM_GROUP_WEAPON:
			case ITEM_GROUP_AMMUNITION:
			case ITEM_GROUP_ARMOR:
			case ITEM_GROUP_WRITEABLE:
			case ITEM_GROUP_KEY:
				break;
			case ITEM_GROUP_DOOR: t->type = ITEM_TYPE_DOOR; break;
			case ITEM_GROUP_CONTAINER: t->type = ITEM_TYPE_CONTAINER; break;
			case ITEM_GROUP_RUNE: t->client_chargeable = true; break;
			case ITEM_GROUP_TELEPORT: t->type = ITEM_TYPE_TELEPORT; break;
			case ITEM_GROUP_MAGICFIELD: t->type = ITEM_TYPE_MAGICFIELD; break;
			default:
				warnings.push_back("Unknown item group declaration");
		}

		uint32_t flags;
		if(itemNode->getU32(flags)) {
			t->unpassable = ((flags & FLAG_UNPASSABLE) == FLAG_UNPASSABLE);
			t->blockMissiles = ((flags & FLAG_BLOCK_MISSILES) == FLAG_BLOCK_MISSILES);
			t->blockPathfinder = ((flags & FLAG_BLOCK_PATHFINDER) == FLAG_BLOCK_PATHFINDER);
			t->hasElevation = ((flags & FLAG_HAS_ELEVATION) == FLAG_HAS_ELEVATION);
			//t->useable = ((flags & FLAG_USEABLE) == FLAG_USEABLE);
			t->pickupable = ((flags & FLAG_PICKUPABLE) == FLAG_PICKUPABLE);
			t->moveable = ((flags & FLAG_MOVEABLE) == FLAG_MOVEABLE);
			t->stackable = ((flags & FLAG_STACKABLE) == FLAG_STACKABLE);
			t->floorChangeDown = ((flags & FLAG_FLOORCHANGEDOWN) == FLAG_FLOORCHANGEDOWN);
			t->floorChangeNorth = ((flags & FLAG_FLOORCHANGENORTH) == FLAG_FLOORCHANGENORTH);
			t->floorChangeEast = ((flags & FLAG_FLOORCHANGEEAST) == FLAG_FLOORCHANGEEAST);
			t->floorChangeSouth = ((flags & FLAG_FLOORCHANGESOUTH) == FLAG_FLOORCHANGESOUTH);
			t->floorChangeWest = ((flags & FLAG_FLOORCHANGEWEST) == FLAG_FLOORCHANGEWEST);
			t->floorChange = t->floorChangeDown || t->floorChangeNorth || t->floorChangeEast || t->floorChangeSouth || t->floorChangeWest;
			// Now this is confusing, just accept that the ALWAYSONTOP flag means it's always on bottom, got it?!
			t->alwaysOnBottom = ((flags & FLAG_ALWAYSONTOP) == FLAG_ALWAYSONTOP);
			t->isHangable = ((flags & FLAG_HANGABLE) == FLAG_HANGABLE);
			t->hookEast = ((flags & FLAG_HOOK_EAST) == FLAG_HOOK_EAST);
			t->hookSouth = ((flags & FLAG_HOOK_SOUTH) == FLAG_HOOK_SOUTH);
			t->allowDistRead = ((flags & FLAG_ALLOWDISTREAD) == FLAG_ALLOWDISTREAD);
			t->rotable = ((flags & FLAG_ROTABLE) == FLAG_ROTABLE);
			t->canReadText = ((flags & FLAG_READABLE) == FLAG_READABLE);
		}

		uint8_t attribute;
		while(itemNode->getU8(attribute)) {
			uint16_t datalen;
			if(!itemNode->getU16(datalen)) {
				warnings.push_back("Invalid item type property");
				break;
			}

			switch(attribute) {
				case ITEM_ATTR_SERVERID: {
					if(datalen != sizeof(uint16_t)) {
						error = "items.otb: Unexpected data length of server id block (Should be 2 bytes)";
						return false;
					}
					if(!itemNode->getU16(t->id))
						warnings.push_back("Invalid item type property (2)");

					if(max_item_id < t->id)
						max_item_id = t->id;
					break;
				}

				case ITEM_ATTR_CLIENTID: {
					if(datalen != sizeof(uint16_t)) {
						error = "items.otb: Unexpected data length of client id block (Should be 2 bytes)";
						return false;
					}

					if(!itemNode->getU16(t->clientID))
						warnings.push_back("Invalid item type property (2)");

					t->sprite = static_cast<GameSprite*>(g_gui.gfx.getSprite(t->clientID));
					break;
				}

				case ITEM_ATTR_SPEED: {
					if(datalen != sizeof(uint16_t)) {
						error = "items.otb: Unexpected data length of speed block (Should be 2 bytes)";
						return false;
					}

					//t->speed = itemNode->getU16();
					if(!itemNode->skip(2)) // Just skip two bytes, we don't need speed
						warnings.push_back("Invalid item type property (3)");
					break;
				}

				case ITEM_ATTR_LIGHT2: {
					if(datalen != sizeof(lightBlock2)) {
						warnings.push_back("items.otb: Unexpected data length of item light (2) block (Should be " + i2ws(sizeof(lightBlock2)) + " bytes)");
						break;
					}

					if(!itemNode->skip(4)) // Just skip two bytes, we don't need light
						warnings.push_back("Invalid item type property (4)");

					//t->lightLevel = itemNode->getU16();
					//t->lightColor = itemNode->getU16();
					break;
				}

				case ITEM_ATTR_TOPORDER: {
					if(datalen != sizeof(uint8_t)) {
						warnings.push_back("items.otb: Unexpected data length of item toporder block (Should be 1 byte)");
						break;
					}

					uint8_t u8 = 0;
					if(!itemNode->getU8(u8))
						warnings.push_back("Invalid item type property (5)");

					t->alwaysOnTopOrder = u8;
					break;
				}

				case ITEM_ATTR_NAME: {
					if(datalen >= 128) {
						warnings.push_back("items.otb: Unexpected data length of item name block (Should be 128 bytes)");
						break;
					}

					uint8_t name[128];
					memset(&name, 0, 128);

					if(!itemNode->getRAW(name, datalen)) {
						warnings.push_back("Invalid item type property (6)");
						break;
					}
					t->name = (char*)name;
					break;
				}

				case ITEM_ATTR_DESCR: {
					if(datalen >= 128) {
						warnings.push_back("items.otb: Unexpected data length of item descr block (Should be 128 bytes)");
						break;
					}

					uint8_t description[128];
					memset(&description, 0, 128);

					if(!itemNode->getRAW(description, datalen)) {
						warnings.push_back("Invalid item type property (7)");
						break;
					}

					t->description = (char*)description;
					break;
				}

				case ITEM_ATTR_MAXITEMS: {
					if(datalen != sizeof(unsigned short)) {
						warnings.push_back("items.otb: Unexpected data length of item volume block (Should be 2 bytes)");
						break;
					}

					if(!itemNode->getU16(t->volume))
						warnings.push_back("Invalid item type property (8)");
					break;
				}

				case ITEM_ATTR_WEIGHT: {
					if(datalen != sizeof(double)) {
						warnings.push_back("items.otb: Unexpected data length of item weight block (Should be 8 bytes)");
						break;
					}
					uint8_t w[sizeof(double)];
					if(!itemNode->getRAW(w, sizeof(double))) {
						warnings.push_back("Invalid item type property (7)");
						break;
					}

					double wi = *reinterpret_cast<double*>(&w);
					t->weight = wi;
					break;
				}

				case ITEM_ATTR_ROTATETO: {
					if(datalen != sizeof(unsigned short)) {
						warnings.push_back("items.otb: Unexpected data length of item rotateTo block (Should be 2 bytes)");
						break;
					}

					uint16_t rotate;
					if(!itemNode->getU16(rotate)) {
						warnings.push_back("Invalid item type property (8)");
						break;
					}

					t->rotateTo = rotate;
					break;
				}

				case ITEM_ATTR_WRITEABLE3: {
					if(datalen != sizeof(writeableBlock3)) {
						warnings.push_back("items.otb: Unexpected data length of item toporder block (Should be 1 byte)");
						break;
					}

					uint16_t readOnlyID;
					uint16_t maxTextLen;

					if(!itemNode->getU16(readOnlyID)) {
						warnings.push_back("Invalid item type property (9)");
						break;
					}

					if(!itemNode->getU16(maxTextLen)) {
						warnings.push_back("Invalid item type property (10)");
						break;
					}

					//t->readOnlyId = wb3->readOnlyId;
					t->maxTextLen = maxTextLen;
					break;
				}

				default: {
					//skip unknown attributes
					itemNode->skip(datalen);
					//warnings.push_back("items.otb: Skipped unknown attribute");
					break;
				}
			}
		}

		if(t) {
			if(items[t->id]) {
				warnings.push_back("items.otb: Duplicate items");
				delete items[t->id];
			}
			items.set(t->id, t);
		}
	}
	return true;
}

bool ItemDatabase::loadFromOtbVer2(BinaryNode* itemNode, wxString& error, wxArrayString& warnings)
{
	uint8_t u8;
	for( ; itemNode != nullptr; itemNode = itemNode->advance()) {
		if(!itemNode->getU8(u8)) {
			// Invalid!
			warnings.push_back("Invalid item type encountered...");
			continue;
		}

		if(ItemGroup_t(u8) == ITEM_GROUP_DEPRECATED)
			continue;

		ItemType* t = newd ItemType();
		t->group = ItemGroup_t(u8);

		switch(t->group) {
			case ITEM_GROUP_NONE:
			case ITEM_GROUP_GROUND:
			case ITEM_GROUP_SPLASH:
			case ITEM_GROUP_FLUID:
				break;
			case ITEM_GROUP_DOOR: t->type = ITEM_TYPE_DOOR; break;
			case ITEM_GROUP_CONTAINER: t->type = ITEM_TYPE_CONTAINER; break;
			case ITEM_GROUP_RUNE: t->client_chargeable = true; break;
			case ITEM_GROUP_TELEPORT: t->type = ITEM_TYPE_TELEPORT; break;
			case ITEM_GROUP_MAGICFIELD: t->type = ITEM_TYPE_MAGICFIELD; break;
			default:
				warnings.push_back("Unknown item group declaration");
		}

		uint32_t flags;
		if(itemNode->getU32(flags)) {
			t->unpassable = ((flags & FLAG_UNPASSABLE) == FLAG_UNPASSABLE);
			t->blockMissiles = ((flags & FLAG_BLOCK_MISSILES) == FLAG_BLOCK_MISSILES);
			t->blockPathfinder = ((flags & FLAG_BLOCK_PATHFINDER) == FLAG_BLOCK_PATHFINDER);
			t->hasElevation = ((flags & FLAG_HAS_ELEVATION) == FLAG_HAS_ELEVATION);
			t->pickupable = ((flags & FLAG_PICKUPABLE) == FLAG_PICKUPABLE);
			t->moveable = ((flags & FLAG_MOVEABLE) == FLAG_MOVEABLE);
			t->stackable = ((flags & FLAG_STACKABLE) == FLAG_STACKABLE);
			t->floorChangeDown = ((flags & FLAG_FLOORCHANGEDOWN) == FLAG_FLOORCHANGEDOWN);
			t->floorChangeNorth = ((flags & FLAG_FLOORCHANGENORTH) == FLAG_FLOORCHANGENORTH);
			t->floorChangeEast = ((flags & FLAG_FLOORCHANGEEAST) == FLAG_FLOORCHANGEEAST);
			t->floorChangeSouth = ((flags & FLAG_FLOORCHANGESOUTH) == FLAG_FLOORCHANGESOUTH);
			t->floorChangeWest = ((flags & FLAG_FLOORCHANGEWEST) == FLAG_FLOORCHANGEWEST);
			t->floorChange = t->floorChangeDown || t->floorChangeNorth || t->floorChangeEast || t->floorChangeSouth || t->floorChangeWest;
			// Now this is confusing, just accept that the ALWAYSONTOP flag means it's always on bottom, got it?!
			t->alwaysOnBottom = ((flags & FLAG_ALWAYSONTOP) == FLAG_ALWAYSONTOP);
			t->isHangable = ((flags & FLAG_HANGABLE) == FLAG_HANGABLE);
			t->hookEast = ((flags & FLAG_HOOK_EAST) == FLAG_HOOK_EAST);
			t->hookSouth = ((flags & FLAG_HOOK_SOUTH) == FLAG_HOOK_SOUTH);
			t->allowDistRead = ((flags & FLAG_ALLOWDISTREAD) == FLAG_ALLOWDISTREAD);
			t->rotable = ((flags & FLAG_ROTABLE) == FLAG_ROTABLE);
			t->canReadText = ((flags & FLAG_READABLE) == FLAG_READABLE);
		}

		uint8_t attribute;
		while(itemNode->getU8(attribute)) {
			uint16_t datalen;
			if(!itemNode->getU16(datalen)) {
				warnings.push_back("Invalid item type property");
				break;
			}

			switch(attribute) {
				case ITEM_ATTR_SERVERID: {
					if(datalen != sizeof(uint16_t)) {
						error = "items.otb: Unexpected data length of server id block (Should be 2 bytes)";
						return false;
					}

					if(!itemNode->getU16(t->id))
						warnings.push_back("Invalid item type property (2)");

					if(max_item_id < t->id)
						max_item_id = t->id;
					break;
				}

				case ITEM_ATTR_CLIENTID: {
					if(datalen != sizeof(uint16_t)) {
						error = "items.otb: Unexpected data length of client id block (Should be 2 bytes)";
						return false;
					}

					if(!itemNode->getU16(t->clientID))
						warnings.push_back("Invalid item type property (2)");

					t->sprite = static_cast<GameSprite*>(g_gui.gfx.getSprite(t->clientID));
					break;
				}

				case ITEM_ATTR_SPEED: {
					if(datalen != sizeof(uint16_t)) {
						error = "items.otb: Unexpected data length of speed block (Should be 2 bytes)";
						return false;
					}

					//t->speed = itemNode->getU16();
					if(!itemNode->skip(2)) // Just skip two bytes, we don't need speed
						warnings.push_back("Invalid item type property (3)");
					break;
				}

				case ITEM_ATTR_LIGHT2: {
					if(datalen != sizeof(lightBlock2)) {
						warnings.push_back("items.otb: Unexpected data length of item light (2) block (Should be " + i2ws(sizeof(lightBlock2)) + " bytes)");
						break;
					}

					if(!itemNode->skip(4)) // Just skip two bytes, we don't need light
						warnings.push_back("Invalid item type property (4)");

					//t->lightLevel = itemNode->getU16();
					//t->lightColor = itemNode->getU16();
					break;
				}

				case ITEM_ATTR_TOPORDER: {
					if(datalen != sizeof(uint8_t)) {
						warnings.push_back("items.otb: Unexpected data length of item toporder block (Should be 1 byte)");
						break;
					}

					uint8_t u8 = 0;
					if(!itemNode->getU8(u8)) {
						warnings.push_back("Invalid item type property (5)");
					}
					t->alwaysOnTopOrder = u8;
					break;
				}

				default: {
					//skip unknown attributes
					itemNode->skip(datalen);
					//warnings.push_back("items.otb: Skipped unknown attribute");
					break;
				}
			}
		}

		if(t) {
			if(items[t->id]) {
				warnings.push_back("items.otb: Duplicate items");
				delete items[t->id];
			}
			items.set(t->id, t);
		}
	}
	return true;
}

bool ItemDatabase::loadFromOtbVer3(BinaryNode* itemNode, wxString& error, wxArrayString& warnings) {
	uint8_t u8;
	for( ; itemNode != nullptr; itemNode = itemNode->advance()) {
		if(!itemNode->getU8(u8)) {
			// Invalid!
			warnings.push_back("Invalid item type encountered...");
			continue;
		}

		if(ItemGroup_t(u8) == ITEM_GROUP_DEPRECATED)
			continue;

		ItemType* t = newd ItemType();
		t->group = ItemGroup_t(u8);

		switch(t->group) {
			case  ITEM_GROUP_NONE:
			case ITEM_GROUP_GROUND:
			case ITEM_GROUP_SPLASH:
			case ITEM_GROUP_FLUID:
				break;
			case ITEM_GROUP_CONTAINER: t->type = ITEM_TYPE_CONTAINER; break;
				break;
			default:
				warnings.push_back("Unknown item group declaration");
		}

		uint32_t flags;
		if(itemNode->getU32(flags)) {
			t->unpassable = ((flags & FLAG_UNPASSABLE) == FLAG_UNPASSABLE);
			t->blockMissiles = ((flags & FLAG_BLOCK_MISSILES) == FLAG_BLOCK_MISSILES);
			t->blockPathfinder = ((flags & FLAG_BLOCK_PATHFINDER) == FLAG_BLOCK_PATHFINDER);
			t->hasElevation = ((flags & FLAG_HAS_ELEVATION) == FLAG_HAS_ELEVATION);
			t->pickupable = ((flags & FLAG_PICKUPABLE) == FLAG_PICKUPABLE);
			t->moveable = ((flags & FLAG_MOVEABLE) == FLAG_MOVEABLE);
			t->stackable = ((flags & FLAG_STACKABLE) == FLAG_STACKABLE);
			t->floorChangeDown = ((flags & FLAG_FLOORCHANGEDOWN) == FLAG_FLOORCHANGEDOWN);
			t->floorChangeNorth = ((flags & FLAG_FLOORCHANGENORTH) == FLAG_FLOORCHANGENORTH);
			t->floorChangeEast = ((flags & FLAG_FLOORCHANGEEAST) == FLAG_FLOORCHANGEEAST);
			t->floorChangeSouth = ((flags & FLAG_FLOORCHANGESOUTH) == FLAG_FLOORCHANGESOUTH);
			t->floorChangeWest = ((flags & FLAG_FLOORCHANGEWEST) == FLAG_FLOORCHANGEWEST);
			t->floorChange = t->floorChangeDown || t->floorChangeNorth || t->floorChangeEast || t->floorChangeSouth || t->floorChangeWest;
			// Now this is confusing, just accept that the ALWAYSONTOP flag means it's always on bottom, got it?!
			t->alwaysOnBottom = ((flags & FLAG_ALWAYSONTOP) == FLAG_ALWAYSONTOP);
			t->isHangable = ((flags & FLAG_HANGABLE) == FLAG_HANGABLE);
			t->hookEast = ((flags & FLAG_HOOK_EAST) == FLAG_HOOK_EAST);
			t->hookSouth = ((flags & FLAG_HOOK_SOUTH) == FLAG_HOOK_SOUTH);
			t->allowDistRead = ((flags & FLAG_ALLOWDISTREAD) == FLAG_ALLOWDISTREAD);
			t->rotable = ((flags & FLAG_ROTABLE) == FLAG_ROTABLE);
			t->canReadText = ((flags & FLAG_READABLE) == FLAG_READABLE);
			t->client_chargeable = ((flags & FLAG_CLIENTCHARGES) == FLAG_CLIENTCHARGES);
			t->ignoreLook = ((flags & FLAG_IGNORE_LOOK) == FLAG_IGNORE_LOOK);
		}

		uint8_t attribute;
		while(itemNode->getU8(attribute)) {
			uint16_t datalen;
			if(!itemNode->getU16(datalen)) {
				warnings.push_back("Invalid item type property");
				break;
			}

			switch(attribute) {
				case ITEM_ATTR_SERVERID: {
					if(datalen != sizeof(uint16_t)) {
						error = "items.otb: Unexpected data length of server id block (Should be 2 bytes)";
						return false;
					}

					if(!itemNode->getU16(t->id))
						warnings.push_back("Invalid item type property (2)");

					if(max_item_id < t->id)
						max_item_id = t->id;
					break;
				}

				case ITEM_ATTR_CLIENTID: {
					if(datalen != sizeof(uint16_t)) {
						error = "items.otb: Unexpected data length of client id block (Should be 2 bytes)";
						return false;
					}

					if(!itemNode->getU16(t->clientID))
						warnings.push_back("Invalid item type property (2)");

					t->sprite = static_cast<GameSprite*>(g_gui.gfx.getSprite(t->clientID));
					break;
				}

				case ITEM_ATTR_SPEED: {
					if(datalen != sizeof(uint16_t)) {
						error = "items.otb: Unexpected data length of speed block (Should be 2 bytes)";
						return false;
					}

					//t->speed = itemNode->getU16();
					if(!itemNode->skip(2)) // Just skip two bytes, we don't need speed
						warnings.push_back("Invalid item type property (3)");
					break;
				}

				case ITEM_ATTR_LIGHT2: {
					if(datalen != sizeof(lightBlock2))
					{
						warnings.push_back("items.otb: Unexpected data length of item light (2) block (Should be " + i2ws(sizeof(lightBlock2)) + " bytes)");
						break;
					}
					if(!itemNode->skip(4)) // Just skip two bytes, we don't need light
						warnings.push_back("Invalid item type property (4)");

					//t->lightLevel = itemNode->getU16();
					//t->lightColor = itemNode->getU16();
					break;
				}

				case ITEM_ATTR_TOPORDER: {
					if(datalen != sizeof(uint8_t)) {
						warnings.push_back("items.otb: Unexpected data length of item toporder block (Should be 1 byte)");
						break;
					}

					if(!itemNode->getU8(u8))
						warnings.push_back("Invalid item type property (5)");

					t->alwaysOnTopOrder = u8;
					break;
				}

				default: {
					//skip unknown attributes
					itemNode->skip(datalen);
					//warnings.push_back("items.otb: Skipped unknown attribute");
					break;
				}
			}
		}

		if(t) {
			if(items[t->id]) {
				warnings.push_back("items.otb: Duplicate items");
				delete items[t->id];
			}
			items.set(t->id, t);
		}
	}
	return true;
}

bool ItemDatabase::loadFromOtb(const FileName& datafile, wxString& error, wxArrayString& warnings)
{
	std::string filename = nstr((datafile.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + datafile.GetFullName()));
	DiskNodeFileReadHandle f(filename, StringVector(1, "OTBI"));

	if(!f.isOk()) {
		error = "Couldn't open file \"" + wxstr(filename) + "\":" + wxstr(f.getErrorMessage());
		return false;
	}

	BinaryNode* root = f.getRootNode();

#define safe_get(node, func, ...) do {\
		if(!node->get##func(__VA_ARGS__)) {\
			error = wxstr(f.getErrorMessage()); \
			return false; \
		} \
	} while(false)

	// Read root flags
	root->skip(1); // Type info
	//uint32_t flags =

	root->skip(4); // Unused?

	uint8_t attr;
	safe_get(root, U8, attr);
	if(attr == ROOT_ATTR_VERSION) {
		uint16_t datalen;
		if(!root->getU16(datalen) || datalen != 4 + 4 + 4 + 1*128) {
			error = "items.otb: Size of version header is invalid, updated .otb version?";
			return false;
		}
		safe_get(root, U32, MajorVersion);	// items otb format file version
		safe_get(root, U32, MinorVersion);	// client version
		safe_get(root, U32, BuildNumber);	// revision
		std::string csd;
		csd.resize(128);

		if(!root->getRAW((uint8_t*)csd.data(), 128)) { // CSDVersion ??
			error = wxstr(f.getErrorMessage());
			return false;
		}
	} else {
		error = "Expected ROOT_ATTR_VERSION as first node of items.otb!";
	}

	if(g_settings.getInteger(Config::CHECK_SIGNATURES)) {
		if(g_gui.GetCurrentVersion().getOTBVersion().format_version != MajorVersion) {
			error = "Unsupported items.otb version (version " + i2ws(MajorVersion) + ")";
			return false;
		}
	}

	BinaryNode* itemNode = root->getChild();
	switch(MajorVersion) {
		case 1: return loadFromOtbVer1(itemNode, error, warnings);
		case 2: return loadFromOtbVer2(itemNode, error, warnings);
		case 3: return loadFromOtbVer3(itemNode, error, warnings);
	}
	return true;
}

bool ItemDatabase::loadItemFromGameXml(pugi::xml_node itemNode, uint16_t id)
{
	ClientVersionID clientVersion = g_gui.GetCurrentVersionID();
	if(clientVersion < CLIENT_VERSION_980 && id > 20000 && id < 20100) {
		itemNode = itemNode.next_sibling();
		return true;
	} else if(id > 30000 && id < 30100) {
		itemNode = itemNode.next_sibling();
		return true;
	}

	ItemType& item = getItemType(id);

	item.name = itemNode.attribute("name").as_string();
	item.editorsuffix = itemNode.attribute("editorsuffix").as_string();

	pugi::xml_attribute attribute;
	for(pugi::xml_node itemAttributesNode = itemNode.first_child(); itemAttributesNode; itemAttributesNode = itemAttributesNode.next_sibling()) {
		if(!(attribute = itemAttributesNode.attribute("key"))) {
			continue;
		}

		std::string key = attribute.as_string();
		to_lower_str(key);
		if(key == "type") {
			if(!(attribute = itemAttributesNode.attribute("value"))) {
				continue;
			}

			std::string typeValue = attribute.as_string();
			to_lower_str(key);
			if(typeValue == "depot") {
				item.type = ITEM_TYPE_DEPOT;
			} else if(typeValue == "mailbox") {
				item.type = ITEM_TYPE_MAILBOX;
			} else if(typeValue == "trashholder") {
				item.type = ITEM_TYPE_TRASHHOLDER;
			} else if (typeValue == "container") {
				item.type = ITEM_TYPE_CONTAINER;
			} else if (typeValue == "door") {
				item.type = ITEM_TYPE_DOOR;
			} else if (typeValue == "magicfield") {
				item.group = ITEM_GROUP_MAGICFIELD;
				item.type = ITEM_TYPE_MAGICFIELD;
			} else if (typeValue == "teleport") {
				item.type = ITEM_TYPE_TELEPORT;
			} else if (typeValue == "bed") {
				item.type = ITEM_TYPE_BED;
			} else if (typeValue == "key") {
				item.type = ITEM_TYPE_KEY;
			}
		} else if(key == "name") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				item.name = attribute.as_string();
			}
		} else if(key == "description") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				item.description = attribute.as_string();
			}
		}else if(key == "runespellName") {
			/*if((attribute = itemAttributesNode.attribute("value"))) {
				item.runeSpellName = attribute.as_string();
			}*/
		} else if(key == "weight") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				item.weight = attribute.as_int() / 100.f;
			}
		} else if(key == "armor") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				item.armor = attribute.as_int();
			}
		} else if(key == "defense") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				item.defense = attribute.as_int();
			}
		} else if(key == "rotateto") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				item.rotateTo = attribute.as_ushort();
			}
		} else if(key == "containersize") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				item.volume = attribute.as_ushort();
			}
		} else if(key == "readable") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				item.canReadText = attribute.as_bool();
			}
		} else if(key == "writeable") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				item.canWriteText = item.canReadText = attribute.as_bool();
			}
		} else if(key == "decayto") {
			item.decays = true;
		} else if(key == "maxtextlen" || key == "maxtextlength") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				item.maxTextLen = attribute.as_ushort();
				item.canReadText = item.maxTextLen > 0;
			}
		} else if(key == "writeonceitemid") {
			/*if((attribute = itemAttributesNode.attribute("value"))) {
				item.writeOnceItemId = pugi::cast<int32_t>(attribute.value());
			}*/
		} else if(key == "allowdistread") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				item.allowDistRead = attribute.as_bool();
			}
		} else if(key == "charges") {
			if((attribute = itemAttributesNode.attribute("value"))) {
				item.charges = attribute.as_uint();
				item.extra_chargeable = true;
			}
		} else if(key == "floorchange") {
			if ((attribute = itemAttributesNode.attribute("value"))) {
				std::string value = attribute.as_string();
				if(value == "down") {
					item.floorChangeDown = true;
					item.floorChange = true;
				} else if (value == "north") {
					item.floorChangeNorth = true;
					item.floorChange = true;
				} else if (value == "south") {
					item.floorChangeSouth = true;
					item.floorChange = true;
				} else if (value == "west") {
					item.floorChangeWest = true;
					item.floorChange = true;
				} else if (value == "east") {
					item.floorChangeEast = true;
					item.floorChange = true;
				} else if(value == "northex")
					item.floorChange = true;
				else if(value == "southex")
					item.floorChange = true;
				else if(value == "westex")
					item.floorChange = true;
				else if(value == "eastex")
					item.floorChange = true;
				else if (value == "southalt")
					item.floorChange = true;
				else if (value == "eastalt")
					item.floorChange = true;
			}
		}
	}
	return true;
}

bool ItemDatabase::loadFromGameXml(const FileName& identifier, wxString& error, wxArrayString& warnings)
{
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(identifier.GetFullPath().mb_str());
	if(!result) {
		error = "Could not load items.xml (Syntax error?)";
		return false;
	}

	pugi::xml_node node = doc.child("items");
	if(!node) {
		error = "items.xml, invalid root node.";
		return false;
	}

	for(pugi::xml_node itemNode = node.first_child(); itemNode; itemNode = itemNode.next_sibling()) {
		if(itemNode.name() != "item") {
			continue;
		}

		uint16_t fromId = 0;
		uint16_t toId = 0;
		if(const pugi::xml_attribute attribute = itemNode.attribute("id")) {
			fromId = toId = attribute.as_ushort();
		} else {
			fromId = itemNode.attribute("fromid").as_ushort();
			toId = itemNode.attribute("toid").as_ushort();
		}

		if(fromId == 0 || toId == 0) {
			error = "Could not read item id from item node.";
			return false;
		}

		for(uint16_t id = fromId; id <= toId; ++id) {
			if(!loadItemFromGameXml(itemNode, id)) {
				return false;
			}
		}
	}
	return true;
}

bool ItemDatabase::loadMetaItem(pugi::xml_node node)
{
	if(const pugi::xml_attribute attribute = node.attribute("id")) {
		const uint16_t id = attribute.as_ushort();
		if(id == 0 || items[id]) {
			return false;
		}
		items.set(id, newd ItemType());
		items[id]->is_metaitem = true;
		items[id]->id = id;
		return true;
	}
	return false;
}

ItemType& ItemDatabase::getItemType(uint16_t id)
{
	ItemType* item = items[id];
	if(item)
		return *item;
	else {
		static ItemType dummyItemType; // use this for invalid ids
		return dummyItemType;
	}
}

bool ItemDatabase::typeExists(uint16_t id) const
{
	ItemType* item = items[id];
	return item != nullptr;
}
