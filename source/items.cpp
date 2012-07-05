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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/items.hpp $
// $Id: items.hpp 310 2010-02-26 18:03:48Z admin $

#include "main.h"

#include "materials.h"
#include "gui.h"
#include <string.h> // memcpy

#include "items.h"
#include "item.h"

ItemDatabase item_db;

ItemType::ItemType() :
	sprite(NULL),
	id(0),
	clientID(0),
	brush(NULL),
	doodad_brush(NULL),
	raw_brush(NULL),
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

	isVertical(false),
	isHorizontal(false),
	isHangable(false),
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

	floorChangeDown(true),
	floorChangeNorth(false),
	floorChangeSouth(false),
	floorChangeEast(false),
	floorChangeWest(false),

	blockSolid(false),
	blockPickupable(false),
	blockProjectile(false),
	blockPathFind(false),

	alwaysOnTopOrder(0),
	rotateTo(0),
	border_alignment(BORDER_NONE)
{
}

ItemType::~ItemType()
{
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
	//
}

ItemDatabase::~ItemDatabase()
{
	clear();
}

void ItemDatabase::clear()
{
	for(uint i = 0; i < items.size(); i++)
	{
		delete items[i];
		items.set(i, NULL);
	}
}

bool ItemDatabase::loadFromOtbVer1(BinaryNode* itemNode, wxString& error, wxArrayString& warnings)
{
	uint8_t u8;
	
	for( ; itemNode != NULL; itemNode = itemNode->advance())
	{
		if(!itemNode->getU8(u8))
		{
			// Invalid!
			warnings.push_back(wxT("Invalid item type encountered..."));
			continue;
		}

		if(u8 == ITEM_GROUP_DEPRECATED)
			continue;

		ItemType* t = newd ItemType();
		t->group = ItemGroup_t(u8);

		switch(t->group)
		{
			case  ITEM_GROUP_NONE:
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
				warnings.push_back(wxT("Unknown item group declaration"));
		}

		uint32_t flags;
		if(itemNode->getU32(flags))
		{
			t->blockSolid = ((flags & FLAG_BLOCK_SOLID) == FLAG_BLOCK_SOLID);
			t->blockProjectile = ((flags & FLAG_BLOCK_PROJECTILE) == FLAG_BLOCK_PROJECTILE);
			t->blockPathFind = ((flags & FLAG_BLOCK_PATHFIND) == FLAG_BLOCK_PATHFIND);
			// These are irrelevant
			//t->hasHeight = ((flags & FLAG_HAS_HEIGHT) == FLAG_HAS_HEIGHT);
			//t->useable = ((flags & FLAG_USEABLE) == FLAG_USEABLE);
			t->pickupable = ((flags & FLAG_PICKUPABLE) == FLAG_PICKUPABLE);
			t->moveable = ((flags & FLAG_MOVEABLE) == FLAG_MOVEABLE);
			t->stackable = ((flags & FLAG_STACKABLE) == FLAG_STACKABLE);
			t->floorChangeDown = ((flags & FLAG_FLOORCHANGEDOWN) == FLAG_FLOORCHANGEDOWN);
			t->floorChangeNorth = ((flags & FLAG_FLOORCHANGENORTH) == FLAG_FLOORCHANGENORTH);
			t->floorChangeEast = ((flags & FLAG_FLOORCHANGEEAST) == FLAG_FLOORCHANGEEAST);
			t->floorChangeSouth = ((flags & FLAG_FLOORCHANGESOUTH) == FLAG_FLOORCHANGESOUTH);
			t->floorChangeWest = ((flags & FLAG_FLOORCHANGEWEST) == FLAG_FLOORCHANGEWEST);
			// Now this is confusing, just accept that the ALWAYSONTOP flag means it's always on bottom, got it?!
			t->alwaysOnBottom = ((flags & FLAG_ALWAYSONTOP) == FLAG_ALWAYSONTOP);
			t->isVertical = ((flags & FLAG_VERTICAL) == FLAG_VERTICAL);
			t->isHorizontal = ((flags & FLAG_HORIZONTAL) == FLAG_HORIZONTAL);
			t->isHangable = ((flags & FLAG_HANGABLE) == FLAG_HANGABLE);
			t->allowDistRead = ((flags & FLAG_ALLOWDISTREAD) == FLAG_ALLOWDISTREAD);
			t->rotable = ((flags & FLAG_ROTABLE) == FLAG_ROTABLE);
			t->canReadText = ((flags & FLAG_READABLE) == FLAG_READABLE);
		}

		uint8_t attribute;
		while(itemNode->getU8(attribute))
		{
			uint16_t datalen;
			if(!itemNode->getU16(datalen))
			{
				warnings.push_back(wxT("Invalid item type property"));
				break;
			}
			switch(attribute)
			{
				case ITEM_ATTR_SERVERID:
				{
					if(datalen != sizeof(uint16_t))
					{
						error = wxT("items.otb: Unexpected data length of server id block (Should be 2 bytes)");
						return false;
					}
					if(!itemNode->getU16(t->id))
						warnings.push_back(wxT("Invalid item type property (2)"));

					if(max_item_id < t->id)
						max_item_id = t->id;

				} break;
				case ITEM_ATTR_CLIENTID:
				{
					if(datalen != sizeof(uint16_t))
					{
						error = wxT("items.otb: Unexpected data length of client id block (Should be 2 bytes)");
						return false;
					}

					if(!itemNode->getU16(t->clientID))
						warnings.push_back(wxT("Invalid item type property (2)"));

					t->sprite = static_cast<GameSprite*>(gui.gfx.getSprite(t->clientID));
				} break;
				case ITEM_ATTR_SPEED:
				{
					if(datalen != sizeof(uint16_t))
					{
						error = wxT("items.otb: Unexpected data length of speed block (Should be 2 bytes)");
						return false;
					}

					//t->speed = itemNode->getU16();
					if(!itemNode->skip(2)) // Just skip two bytes, we don't need speed
						warnings.push_back(wxT("Invalid item type property (3)"));

				} break;
				case ITEM_ATTR_LIGHT2:
				{
					if(datalen != sizeof(lightBlock2))
					{
						warnings.push_back(wxT("items.otb: Unexpected data length of item light (2) block (Should be ") + i2ws(sizeof(lightBlock2)) + wxT(" bytes)"));
						break;
					}

					if(!itemNode->skip(4)) // Just skip two bytes, we don't need light
						warnings.push_back(wxT("Invalid item type property (4)"));

					//t->lightLevel = itemNode->getU16();
					//t->lightColor = itemNode->getU16();
				} break;
				case ITEM_ATTR_TOPORDER:
				{
					if(datalen != sizeof(uint8_t))
					{
						warnings.push_back(wxT("items.otb: Unexpected data length of item toporder block (Should be 1 byte)"));
						break;
					}

					uint8_t u8 = 0;
					if(!itemNode->getU8(u8))
						warnings.push_back(wxT("Invalid item type property (5)"));
					
					t->alwaysOnTopOrder = u8;
				} break;			
				case ITEM_ATTR_NAME:
				{
					if(datalen >= 128)
					{
						warnings.push_back(wxT("items.otb: Unexpected data length of item name block (Should be 128 bytes)"));
						break;
					}
					
					uint8_t name[128];
					memset(&name, 0, 128);
					
					if(!itemNode->getRAW(name, datalen))
					{
						warnings.push_back(wxT("Invalid item type property (6)"));
						break;
					}
					t->name = (char*)name;
				} break;
				case ITEM_ATTR_DESCR:
				{
					if(datalen >= 128)
					{
						warnings.push_back(wxT("items.otb: Unexpected data length of item descr block (Should be 128 bytes)"));
						break;
					}

					uint8_t description[128];
					memset(&description, 0, 128);

					if(!itemNode->getRAW(description, datalen))
					{
						warnings.push_back(wxT("Invalid item type property (7)"));
						break;
					}

					t->description = (char*)description;
				} break;
				case ITEM_ATTR_MAXITEMS:
				{
					if(datalen != sizeof(unsigned short))
					{
						warnings.push_back(wxT("items.otb: Unexpected data length of item volume block (Should be 2 bytes)"));
						break;
					}

					if(!itemNode->getU16(t->volume))
						warnings.push_back(wxT("Invalid item type property (8)"));

				} break;
				case ITEM_ATTR_WEIGHT:
				{
					if(datalen != sizeof(double))
					{
						warnings.push_back(wxT("items.otb: Unexpected data length of item weight block (Should be 8 bytes)"));
						break;
					}
					uint8_t w[sizeof(double)];
					if(!itemNode->getRAW(w, sizeof(double)))
					{
						warnings.push_back(wxT("Invalid item type property (7)"));
						break;
					}

					double wi = *reinterpret_cast<double*>(&w);
					t->weight = wi;
				} break;
				case ITEM_ATTR_ROTATETO:
				{
					if(datalen != sizeof(unsigned short))
					{
						warnings.push_back(wxT("items.otb: Unexpected data length of item rotateTo block (Should be 2 bytes)"));
						break;
					}

					uint16_t rotate;
					if(!itemNode->getU16(rotate))
					{
						warnings.push_back(wxT("Invalid item type property (8)"));
						break;
					}

					t->rotateTo = rotate;
				} break;
				case ITEM_ATTR_WRITEABLE3:
				{
					if(datalen != sizeof(writeableBlock3))
					{
						warnings.push_back(wxT("items.otb: Unexpected data length of item toporder block (Should be 1 byte)"));
						break;
					}

					uint16_t readOnlyID;
					uint16_t maxTextLen;

					if(!itemNode->getU16(readOnlyID))
					{
						warnings.push_back(wxT("Invalid item type property (9)"));
						break;
					}

					if(!itemNode->getU16(maxTextLen))
					{
						warnings.push_back(wxT("Invalid item type property (10)"));
						break;
					}

					//t->readOnlyId = wb3->readOnlyId;
					t->maxTextLen = maxTextLen;
				} break;
				default:
				{
					//skip unknown attributes
					itemNode->skip(datalen);
					//warnings.push_back(wxT("items.otb: Skipped unknown attribute"));
				} break;
			}
		}
		if(t) 
		{
			if(items[t->id])
			{
				warnings.push_back(wxT("items.otb: Duplicate items"));
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
	for( ; itemNode != NULL; itemNode = itemNode->advance())
	{
		if(!itemNode->getU8(u8))
		{
			// Invalid!
			warnings.push_back(wxT("Invalid item type encountered..."));
			continue;
		}

		if(ItemGroup_t(u8) == ITEM_GROUP_DEPRECATED)
			continue;

		ItemType* t = newd ItemType();
		t->group = ItemGroup_t(u8);

		switch(t->group)
		{
			case  ITEM_GROUP_NONE:
			case ITEM_GROUP_GROUND:
			case ITEM_GROUP_SPLASH:
			case ITEM_GROUP_FLUID:
				break;
			case   ITEM_GROUP_DOOR: t->type = ITEM_TYPE_DOOR; break;
			case   ITEM_GROUP_CONTAINER: t->type = ITEM_TYPE_CONTAINER; break;
			case   ITEM_GROUP_RUNE: t->client_chargeable = true; break;
			case   ITEM_GROUP_TELEPORT: t->type = ITEM_TYPE_TELEPORT; break;
			case   ITEM_GROUP_MAGICFIELD: t->type = ITEM_TYPE_MAGICFIELD; break;
			default:
				warnings.push_back(wxT("Unknown item group declaration"));
		}

		uint32_t flags;
		if(itemNode->getU32(flags))
		{
			t->blockSolid = ((flags & FLAG_BLOCK_SOLID) == FLAG_BLOCK_SOLID);
			t->blockProjectile = ((flags & FLAG_BLOCK_PROJECTILE) == FLAG_BLOCK_PROJECTILE);
			t->blockPathFind = ((flags & FLAG_BLOCK_PATHFIND) == FLAG_BLOCK_PATHFIND);
			t->pickupable = ((flags & FLAG_PICKUPABLE) == FLAG_PICKUPABLE);
			t->moveable = ((flags & FLAG_MOVEABLE) == FLAG_MOVEABLE);
			t->stackable = ((flags & FLAG_STACKABLE) == FLAG_STACKABLE);
			t->floorChangeDown = ((flags & FLAG_FLOORCHANGEDOWN) == FLAG_FLOORCHANGEDOWN);
			t->floorChangeNorth = ((flags & FLAG_FLOORCHANGENORTH) == FLAG_FLOORCHANGENORTH);
			t->floorChangeEast = ((flags & FLAG_FLOORCHANGEEAST) == FLAG_FLOORCHANGEEAST);
			t->floorChangeSouth = ((flags & FLAG_FLOORCHANGESOUTH) == FLAG_FLOORCHANGESOUTH);
			t->floorChangeWest = ((flags & FLAG_FLOORCHANGEWEST) == FLAG_FLOORCHANGEWEST);
			// Now this is confusing, just accept that the ALWAYSONTOP flag means it's always on bottom, got it?!
			t->alwaysOnBottom = ((flags & FLAG_ALWAYSONTOP) == FLAG_ALWAYSONTOP);
			t->isVertical = ((flags & FLAG_VERTICAL) == FLAG_VERTICAL);
			t->isHorizontal = ((flags & FLAG_HORIZONTAL) == FLAG_HORIZONTAL);
			t->isHangable = ((flags & FLAG_HANGABLE) == FLAG_HANGABLE);
			t->allowDistRead = ((flags & FLAG_ALLOWDISTREAD) == FLAG_ALLOWDISTREAD);
			t->rotable = ((flags & FLAG_ROTABLE) == FLAG_ROTABLE);
			t->canReadText = ((flags & FLAG_READABLE) == FLAG_READABLE);
		}

		uint8_t attribute;
		while(itemNode->getU8(attribute))
		{
			uint16_t datalen;
			if(!itemNode->getU16(datalen))
			{
				warnings.push_back(wxT("Invalid item type property"));
				break;
			}

			switch(attribute)
			{
				case ITEM_ATTR_SERVERID:
				{
					if(datalen != sizeof(uint16_t))
					{
						error = wxT("items.otb: Unexpected data length of server id block (Should be 2 bytes)");
						return false;
					}

					if(!itemNode->getU16(t->id))
						warnings.push_back(wxT("Invalid item type property (2)"));

					if(max_item_id < t->id)
						max_item_id = t->id;

				} break;
				case ITEM_ATTR_CLIENTID:
				{
					if(datalen != sizeof(uint16_t))
					{
						error = wxT("items.otb: Unexpected data length of client id block (Should be 2 bytes)");
						return false;
					}

					if(!itemNode->getU16(t->clientID))
						warnings.push_back(wxT("Invalid item type property (2)"));

					t->sprite = static_cast<GameSprite*>(gui.gfx.getSprite(t->clientID));
				} break;
				case ITEM_ATTR_SPEED:
				{
					if(datalen != sizeof(uint16_t))
					{
						error = wxT("items.otb: Unexpected data length of speed block (Should be 2 bytes)");
						return false;
					}

					//t->speed = itemNode->getU16();
					if(!itemNode->skip(2)) // Just skip two bytes, we don't need speed
						warnings.push_back(wxT("Invalid item type property (3)"));

				} break;
				case ITEM_ATTR_LIGHT2:
				{
					if(datalen != sizeof(lightBlock2))
					{
						warnings.push_back(wxT("items.otb: Unexpected data length of item light (2) block (Should be ") + i2ws(sizeof(lightBlock2)) + wxT(" bytes)"));
						break;
					}

					if(!itemNode->skip(4)) // Just skip two bytes, we don't need light
						warnings.push_back(wxT("Invalid item type property (4)"));

					//t->lightLevel = itemNode->getU16();
					//t->lightColor = itemNode->getU16();
				} break;
				case ITEM_ATTR_TOPORDER: {
					if(datalen != sizeof(uint8_t))
					{
						warnings.push_back(wxT("items.otb: Unexpected data length of item toporder block (Should be 1 byte)"));
						break;
					}

					uint8_t u8 = 0;
					if(!itemNode->getU8(u8))
					{
						warnings.push_back(wxT("Invalid item type property (5)"));
					}
					t->alwaysOnTopOrder = u8;
				} break;
				default:
				{
					//skip unknown attributes
					itemNode->skip(datalen);
					//warnings.push_back(wxT("items.otb: Skipped unknown attribute"));
				} break;
			}
		}

		if(t)
		{
			if(items[t->id])
			{
				warnings.push_back(wxT("items.otb: Duplicate items"));
				delete items[t->id];
			}
			items.set(t->id, t);
		}
	}
	return true;
}

bool ItemDatabase::loadFromOtbVer3(BinaryNode* itemNode, wxString& error, wxArrayString& warnings) {
	uint8_t u8;
	for( ; itemNode != NULL; itemNode = itemNode->advance())
	{
		if(!itemNode->getU8(u8))
		{
			// Invalid!
			warnings.push_back(wxT("Invalid item type encountered..."));
			continue;
		}

		if(ItemGroup_t(u8) == ITEM_GROUP_DEPRECATED)
			continue;

		ItemType* t = newd ItemType();
		t->group = ItemGroup_t(u8);

		switch(t->group)
		{
			case  ITEM_GROUP_NONE:
			case ITEM_GROUP_GROUND:
			case ITEM_GROUP_SPLASH:
			case ITEM_GROUP_FLUID:
				break;
			case ITEM_GROUP_CONTAINER: t->type = ITEM_TYPE_CONTAINER; break;
				break;
			default:
				warnings.push_back(wxT("Unknown item group declaration"));
		}

		uint32_t flags;
		if(itemNode->getU32(flags))
		{
			t->blockSolid = ((flags & FLAG_BLOCK_SOLID) == FLAG_BLOCK_SOLID);
			t->blockProjectile = ((flags & FLAG_BLOCK_PROJECTILE) == FLAG_BLOCK_PROJECTILE);
			t->blockPathFind = ((flags & FLAG_BLOCK_PATHFIND) == FLAG_BLOCK_PATHFIND);
			t->pickupable = ((flags & FLAG_PICKUPABLE) == FLAG_PICKUPABLE);
			t->moveable = ((flags & FLAG_MOVEABLE) == FLAG_MOVEABLE);
			t->stackable = ((flags & FLAG_STACKABLE) == FLAG_STACKABLE);
			t->floorChangeDown = ((flags & FLAG_FLOORCHANGEDOWN) == FLAG_FLOORCHANGEDOWN);
			t->floorChangeNorth = ((flags & FLAG_FLOORCHANGENORTH) == FLAG_FLOORCHANGENORTH);
			t->floorChangeEast = ((flags & FLAG_FLOORCHANGEEAST) == FLAG_FLOORCHANGEEAST);
			t->floorChangeSouth = ((flags & FLAG_FLOORCHANGESOUTH) == FLAG_FLOORCHANGESOUTH);
			t->floorChangeWest = ((flags & FLAG_FLOORCHANGEWEST) == FLAG_FLOORCHANGEWEST);
			// Now this is confusing, just accept that the ALWAYSONTOP flag means it's always on bottom, got it?!
			t->alwaysOnBottom = ((flags & FLAG_ALWAYSONTOP) == FLAG_ALWAYSONTOP);
			t->isVertical = ((flags & FLAG_VERTICAL) == FLAG_VERTICAL);
			t->isHorizontal = ((flags & FLAG_HORIZONTAL) == FLAG_HORIZONTAL);
			t->isHangable = ((flags & FLAG_HANGABLE) == FLAG_HANGABLE);
			t->allowDistRead = ((flags & FLAG_ALLOWDISTREAD) == FLAG_ALLOWDISTREAD);
			t->rotable = ((flags & FLAG_ROTABLE) == FLAG_ROTABLE);
			t->canReadText = ((flags & FLAG_READABLE) == FLAG_READABLE);
			t->client_chargeable = ((flags & FLAG_CLIENTCHARGES) == FLAG_CLIENTCHARGES);
		}

		uint8_t attribute;
		while(itemNode->getU8(attribute))
		{
			uint16_t datalen;
			if(!itemNode->getU16(datalen))
			{
				warnings.push_back(wxT("Invalid item type property"));
				break;
			}

			switch(attribute)
			{
				case ITEM_ATTR_SERVERID:
				{
					if(datalen != sizeof(uint16_t)) 
					{
						error = wxT("items.otb: Unexpected data length of server id block (Should be 2 bytes)");
						return false;
					}

					if(!itemNode->getU16(t->id))
						warnings.push_back(wxT("Invalid item type property (2)"));

					if(max_item_id < t->id)
						max_item_id = t->id;

				} break;
				case ITEM_ATTR_CLIENTID:
				{
					if(datalen != sizeof(uint16_t))
					{
						error = wxT("items.otb: Unexpected data length of client id block (Should be 2 bytes)");
						return false;
					}

					if(!itemNode->getU16(t->clientID))
						warnings.push_back(wxT("Invalid item type property (2)"));

					t->sprite = static_cast<GameSprite*>(gui.gfx.getSprite(t->clientID));
				} break;
				case ITEM_ATTR_SPEED:
				{
					if(datalen != sizeof(uint16_t))
					{
						error = wxT("items.otb: Unexpected data length of speed block (Should be 2 bytes)");
						return false;
					}

					//t->speed = itemNode->getU16();
					if(!itemNode->skip(2)) // Just skip two bytes, we don't need speed
						warnings.push_back(wxT("Invalid item type property (3)"));

				} break;
				case ITEM_ATTR_LIGHT2:
				{
					if(datalen != sizeof(lightBlock2))
					{
						warnings.push_back(wxT("items.otb: Unexpected data length of item light (2) block (Should be ") + i2ws(sizeof(lightBlock2)) + wxT(" bytes)"));
						break;
					}
					if(!itemNode->skip(4)) // Just skip two bytes, we don't need light
						warnings.push_back(wxT("Invalid item type property (4)"));

					//t->lightLevel = itemNode->getU16();
					//t->lightColor = itemNode->getU16();
				} break;
				case ITEM_ATTR_TOPORDER:
				{
					if(datalen != sizeof(uint8_t))
					{
						warnings.push_back(wxT("items.otb: Unexpected data length of item toporder block (Should be 1 byte)"));
						break;
					}

					if(!itemNode->getU8(u8))
						warnings.push_back(wxT("Invalid item type property (5)"));

					t->alwaysOnTopOrder = u8;
				} break;
				default:
				{
					//skip unknown attributes
					itemNode->skip(datalen);
					//warnings.push_back(wxT("items.otb: Skipped unknown attribute"));
				} break;
			}
		}

		if(t)
		{
			if(items[t->id])
			{
				warnings.push_back(wxT("items.otb: Duplicate items"));
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

	if(f.isOk() == false)
	{
		error = wxT("Couldn't open file \"") + wxstr(filename) + wxT("\":") + wxstr(f.getErrorMessage());
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
	if(attr == ROOT_ATTR_VERSION)
	{
		uint16_t datalen;
		if(root->getU16(datalen) == false || datalen != 4 + 4 + 4 + 1*128)
		{
			error = wxT("items.otb: Size of version header is invalid, updated .otb version?");
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
	}
	else
	{
		error = wxT("Expected ROOT_ATTR_VERSION as first node of items.otb!");
	}

	if(settings.getInteger(Config::CHECK_SIGNATURES))
	{
		if(gui.GetCurrentVersion().getOTBVersion().format_version != MajorVersion)
		{
			error = wxT("Unsupported items.otb version (version ") + i2ws(MajorVersion) + wxT(")");
			return false;
		}
	}

	BinaryNode* itemNode = root->getChild();
	switch(MajorVersion)
	{
		case 1: return loadFromOtbVer1(itemNode, error, warnings);
		case 2: return loadFromOtbVer2(itemNode, error, warnings);
		case 3: return loadFromOtbVer3(itemNode, error, warnings);
	}
	return true;
}

bool ItemDatabase::loadItemFromGameXml(xmlNodePtr itemNode, int id)
{
	if(id > 20000 && id < 20100){
		// WTF is the use of this shit? :P

		itemNode = itemNode->next;

		return true;

#if 0
		// What OT does...
		id = id - 20000;

		ItemType* iType = newd ItemType();
		iType->id = id;
		items[id] = iType;
#endif
	}

	ItemType& it = getItemType(id);

	readXMLString(itemNode, "name", it.name);

	readXMLString(itemNode, "editorsuffix", it.editorsuffix);

	xmlNodePtr itemAttributesNode = itemNode->children;

	std::string strValue;
	int intValue;
	while(itemAttributesNode)
	{
		if(readXMLString(itemAttributesNode, "key", strValue))
		{
			if(strValue == "type")
			{
				if(readXMLString(itemAttributesNode, "value", strValue))
				{
					if(strValue == "magicfield")
					{
						it.group = ITEM_GROUP_MAGICFIELD;
						it.type = ITEM_TYPE_MAGICFIELD;
					}
					else if(strValue == "key")
					{
						it.type = ITEM_TYPE_KEY;
					}
					else if(strValue == "depot")
					{
						it.type = ITEM_TYPE_DEPOT;
					}
					else if(strValue == "teleport")
					{
						it.type = ITEM_TYPE_TELEPORT;
					}
					else if(strValue == "bed")
					{
						it.type = ITEM_TYPE_BED;
					}
					else if(strValue == "door")
					{
						it.type = ITEM_TYPE_DOOR;
					}
					else
					{
						// We ignore many types, no need to complain
						//warnings.push_back("items.xml: Unknown type " + strValue);
					}
				}
			}
			else if(strValue == "name")
			{
				if(readXMLString(itemAttributesNode, "value", strValue))
					it.name = strValue;
			}
			else if(strValue == "description")
			{
				if(readXMLString(itemAttributesNode, "value", strValue))
					it.description = strValue;
			}/*
			else if(strValue == "runeSpellName")
			{
				if(readXMLString(itemAttributesNode, "value", strValue))
					it.runeSpellName = strValue;
			}*/
			else if(strValue == "weight")
			{
				if(readXMLInteger(itemAttributesNode, "value", intValue))
					it.weight = intValue / 100.f;
			}
			else if(strValue == "armor")
			{
				if(readXMLInteger(itemAttributesNode, "value", intValue))
					it.armor = intValue;
			}
			else if(strValue == "defense")
			{
				if(readXMLInteger(itemAttributesNode, "value", intValue))
					it.defense = intValue;
			}
			else if(strValue == "attack")
			{
				if(readXMLInteger(itemAttributesNode, "value", intValue))
					it.attack = intValue;
			}
			else if(strValue == "rotateTo")
			{
				if(readXMLInteger(itemAttributesNode, "value", intValue))
					it.rotateTo = intValue;
			}
			else if(strValue == "containerSize")
			{
				if(readXMLInteger(itemAttributesNode, "value", intValue))
					it.volume = intValue;
			}
			/*
			else if(strValue == "readable")
			{
				if(readXMLInteger(itemAttributesNode, "value", intValue))
					it.canReadText = true;
			}
			*/
			else if(strValue == "writeable")
			{
				if(readXMLInteger(itemAttributesNode, "value", intValue))
					it.canWriteText = true;
					it.canReadText = true;
			}
			else if(strValue == "decayTo")
			{
				it.decays = true;
			}
			else if(strValue == "maxTextLen")
			{
				if(readXMLInteger(itemAttributesNode, "value", intValue))
					it.maxTextLen = intValue;
			}/*
			else if(strValue == "writeOnceItemId")
			{
				if(readXMLInteger(itemAttributesNode, "value", intValue))
					it.writeOnceItemId = intValue;
			}*/
			else if(strValue == "charges")
			{
				if(readXMLInteger(itemAttributesNode, "value", intValue))
				{
					it.charges = intValue;
					it.extra_chargeable = true;
				}
			}
		}

		itemAttributesNode = itemAttributesNode->next;
	}

	return true;
}

bool ItemDatabase::loadFromGameXml(const FileName& identifier, wxString& error, wxArrayString& warnings)
{
	xmlDocPtr doc = xmlParseFile(identifier.GetFullPath().mb_str());
	int intValue;
	std::string strValue;

	if(doc)
	{
		xmlNodePtr root = xmlDocGetRootElement(doc);

		if(xmlStrcmp(root->name,(const xmlChar*)"items") != 0)
		{
			xmlFreeDoc(doc);
			error = wxT("items.xml, invalid root node.");
			return false;
		}

		xmlNodePtr itemNode = root->children;
		while(itemNode)
		{
			if(xmlStrcmp(itemNode->name,(const xmlChar*)"item") == 0)
			{
				int fromid = 0, toid = 0;

				readXMLInteger(itemNode, "fromid", fromid);
				readXMLInteger(itemNode, "toid", toid);

				if(readXMLInteger(itemNode, "id", intValue))
					fromid = toid = intValue;

				if(fromid == 0 || toid == 0)
				{
					error = wxT("Could not read item id from item node.");
					return false;
				}

				for(int id = fromid; id <= toid; ++id)
					if(!loadItemFromGameXml(itemNode, id))
						return false;
			}

			itemNode = itemNode->next;
		}

		xmlFreeDoc(doc);
	}
	else
	{
		error = wxT("Could not load items.xml (Syntax error?)");
		return false;
	}
	return true;
}

bool ItemDatabase::loadMetaItem(xmlNodePtr node)
{
	int id;
	if(readXMLInteger(node, "id", id))
	{
		if(items[id] != NULL)
		{
			//std::cout << "Occupied ID " << id << " : " << items[id]->id << ":" << items[id]->name << std::endl;
			return false;
		}
		items.set(id, newd ItemType());
		items[id]->is_metaitem = true;
		items[id]->id = id;
	}
	else
	{
		return false;
	}
	return true;
}

ItemType& ItemDatabase::getItemType(int id)
{
	ItemType* it = items[id];
	if(it)
		return *it;
	else
	{
		static ItemType dummyItemType; // use this for invalid ids
		return dummyItemType;
	}
}

bool ItemDatabase::typeExists(int id) const
{
	ItemType* it = items[id];
	return it != NULL;
}
