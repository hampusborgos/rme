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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/iomap_otbm.hpp $
// $Id: iomap_otbm.hpp 310 2010-02-26 18:03:48Z admin $

#include "main.h"

#include <wx/wfstream.h>
#include <wx/tarstrm.h>
#include <wx/zstream.h>
#include <wx/mstream.h>
#include <wx/datstrm.h>

#include "settings.h"
#include "gui.h" // Loadbar

#include "creatures.h"
#include "creature.h"
#include "map.h"
#include "tile.h"
#include "item.h"
#include "complexitem.h"
#include "town.h"

#include "iomap_otbm.h"

typedef uint8_t attribute_t;
typedef uint32_t flags_t;


// H4X
void reform(Map* map, Tile* tile, Item* item)
{
	int aid = item->getActionID();
	int id = item->getID();
	int uid = item->getUniqueID();
	/*
	if (item->isDoor()) {
		item->eraseAttribute("aid");
		item->setAttribute("keyid", aid);
	}

	if ((item->isDoor()) && tile && tile->getHouseID()) {
		Door* self = static_cast<Door*>(item);
		House* house = map->houses.getHouse(tile->getHouseID());
		self->setDoorID(house->getEmptyDoorID());
	}
	*/
}


// ============================================================================
// Item

Item* Item::Create_OTBM(const IOMap& maphandle, BinaryNode* stream)
{
	uint16_t _id;
	if(!stream->getU16(_id))
		return NULL;

	uint8_t _count = 0;

	const ItemType& iType = item_db[_id];
	if(maphandle.version.otbm == MAP_OTBM_1)
	{
		if(iType.stackable || iType.isSplash() || iType.isFluidContainer())
		{
			if(!stream->getU8(_count))
			{
				// Do nothing, we can't fail on this
			}
		}
	}
	return Item::Create(_id, _count);
}

bool Item::readItemAttribute_OTBM(const IOMap& maphandle, OTBM_ItemAttribute attr, BinaryNode* stream)
{
	switch(attr)
	{
		case OTBM_ATTR_COUNT:
		{
			uint8_t subtype;
			if(!stream->getU8(subtype))
				return false;

			setSubtype(subtype);
		} break;
		case OTBM_ATTR_ACTION_ID:
		{
			uint16_t aid;
			if(!stream->getU16(aid))
				return false;

			setActionID(aid);
		} break;
		case OTBM_ATTR_UNIQUE_ID:
		{
			uint16_t uid;
			if(!stream->getU16(uid))
				return false;

			setUniqueID(uid);
		} break;
		case OTBM_ATTR_CHARGES:
		{
			uint16_t charges;
			if(!stream->getU16(charges))
				return false;

			setSubtype(charges);
		} break;
		case OTBM_ATTR_TEXT:
		{
			std::string text;
			if(!stream->getString(text))
				return false;

			setText(text);
		} break;
		case OTBM_ATTR_DESC:
		{
			std::string text;
			if(!stream->getString(text))
				return false;

			setDescription(text);
		} break;
		case OTBM_ATTR_RUNE_CHARGES:
		{
			uint8_t subtype;
			if(!stream->getU8(subtype))
				return false;

			setSubtype(subtype);
		} break;
		
		// The following *should* be handled in the derived classes
		// However, we still need to handle them here since otherwise things
		// will break horribly
		case OTBM_ATTR_DEPOT_ID:
		{
			return stream->skip(2);
		} break;
		case OTBM_ATTR_HOUSEDOORID:
		{
			return stream->skip(1);
		} break;
		case OTBM_ATTR_TELE_DEST:
		{
			return stream->skip(5);
		} break;
		default:
		{
			return false;
		} break;
	}

	return true;
}

bool Item::unserializeAttributes_OTBM(const IOMap& maphandle, BinaryNode* stream)
{
	uint8_t attribute;
	while(stream->getU8(attribute))
	{
		if(attribute == OTBM_ATTR_ATTRIBUTE_MAP)
		{
			if(!ItemAttributes::unserializeAttributeMap(maphandle, stream))
				return false;
		}
		else if(!readItemAttribute_OTBM(maphandle, OTBM_ItemAttribute(attribute), stream))
		{
			return false;
		}
	}
	return true;
}

bool Item::unserializeItemNode_OTBM(const IOMap& maphandle, BinaryNode* node)
{
	return unserializeAttributes_OTBM(maphandle, node);
}

void Item::serializeItemAttributes_OTBM(const IOMap& maphandle, NodeFileWriteHandle& stream) const
{
	if(maphandle.version.otbm >= MAP_OTBM_2)
	{
		const ItemType& iType = item_db[id];

		if(iType.stackable || iType.isSplash() || iType.isFluidContainer())
		{
			stream.addU8(OTBM_ATTR_COUNT);
			stream.addU8(getSubtype());
		}
	}

	if(maphandle.version.otbm >= MAP_OTBM_4)
	{
		if(attributes && attributes->size())
		{
			stream.addU8(OTBM_ATTR_ATTRIBUTE_MAP);
			serializeAttributeMap(maphandle, stream);
		}
	}
	else
	{
		if(item_db.MinorVersion >= CLIENT_VERSION_820 && isCharged())
		{
			stream.addU8(OTBM_ATTR_CHARGES);
			stream.addU16(getSubtype());
		}

		if(getActionID() > 0)
		{
			stream.addU8(OTBM_ATTR_ACTION_ID);
			stream.addU16(getActionID());
		}

		if(getUniqueID() > 0)
		{
			stream.addU8(OTBM_ATTR_UNIQUE_ID);
			stream.addU16(getUniqueID());
		}

		if(getText().length() > 0)
		{
			stream.addU8(OTBM_ATTR_TEXT);
			stream.addString(getText());
		}

		if(getDescription().length() > 0)
		{
			stream.addU8(OTBM_ATTR_DESC);
			stream.addString(getDescription());
		}
	}
}

void Item::serializeItemCompact_OTBM(const IOMap& maphandle, NodeFileWriteHandle& stream) const
{
	stream.addU16(id);

	/* This is impossible
	const ItemType& iType = item_db[id];

	if(iType.stackable || iType.isSplash() || iType.isFluidContainer()){
		stream.addU8(getSubtype());
	}
	*/
}


bool Item::serializeItemNode_OTBM(const IOMap& maphandle, NodeFileWriteHandle& f) const
{
	f.addNode(OTBM_ITEM);
	f.addU16(id);

	if(maphandle.version.otbm == MAP_OTBM_1) {
		const ItemType& iType = item_db[id];

		if(iType.stackable || iType.isSplash() || iType.isFluidContainer()){
			f.addU8(getSubtype());
		}
	}

	serializeItemAttributes_OTBM(maphandle, f);
	f.endNode();

	return true;
}

// ============================================================================
// Teleport

bool Teleport::readItemAttribute_OTBM(const IOMap& maphandle, OTBM_ItemAttribute attribute, BinaryNode* stream)
{
	if(OTBM_ATTR_TELE_DEST == attribute) {
		uint16_t x = 0;
		uint16_t y = 0;
		uint8_t z = 0;
		if(!stream->getU16(x) ||
				!stream->getU16(y) ||
				!stream->getU8(z))
		{
			return false;
		}

		destination = Position(x, y, z);
		return true;
	} else {
		return Item::readItemAttribute_OTBM(maphandle, attribute, stream);
	}
}

void Teleport::serializeItemAttributes_OTBM(const IOMap& maphandle, NodeFileWriteHandle& stream) const
{
	Item::serializeItemAttributes_OTBM(maphandle, stream);

	stream.addByte(OTBM_ATTR_TELE_DEST);
	stream.addU16(destination.x);
	stream.addU16(destination.y);
	stream.addU8(destination.z);
}

// ============================================================================
// Door

bool Door::readItemAttribute_OTBM(const IOMap& maphandle, OTBM_ItemAttribute attribute, BinaryNode* stream)
{
	if(OTBM_ATTR_HOUSEDOORID == attribute) {
		uint8_t id = 0;
		if(!stream->getU8(id)) {
			return false;
		}
		doorid = id;
		return true;
	} else {
		return Item::readItemAttribute_OTBM(maphandle, attribute, stream);
	}
}

void Door::serializeItemAttributes_OTBM(const IOMap& maphandle, NodeFileWriteHandle& stream) const
{
	Item::serializeItemAttributes_OTBM(maphandle, stream);
	if(doorid) {
		stream.addByte(OTBM_ATTR_HOUSEDOORID);
		stream.addU8(doorid);
	}
}

// ============================================================================
// Depots

bool Depot::readItemAttribute_OTBM(const IOMap& maphandle, OTBM_ItemAttribute attribute, BinaryNode* stream)
{
	if(OTBM_ATTR_DEPOT_ID == attribute) {
		uint16_t id = 0;
		if(!stream->getU16(id)) {
			return false;
		}
		depotid = id;
		return true;
	} else {
		return Item::readItemAttribute_OTBM(maphandle, attribute, stream);
	}
}

void Depot::serializeItemAttributes_OTBM(const IOMap& maphandle, NodeFileWriteHandle& stream) const
{
	Item::serializeItemAttributes_OTBM(maphandle, stream);
	if(depotid) {
		stream.addByte(OTBM_ATTR_DEPOT_ID);
		stream.addU16(depotid);
	}
}

// ============================================================================
// Container

bool Container::unserializeItemNode_OTBM(const IOMap& maphandle, BinaryNode* node)
{
	bool ret = Item::unserializeAttributes_OTBM(maphandle, node);

	if(ret)
	{
		BinaryNode* child = node->getChild();
		if(child)
		do { // Do always us to break...
			uint8_t type;
			if(!child->getByte(type))
				return false;

			//load container items
			if(type == OTBM_ITEM)
			{
				Item* item = Item::Create_OTBM(maphandle, child);
				if(!item)
					return false;

				if(!item->unserializeItemNode_OTBM(maphandle, child))
				{
					delete item;
					return false;
				}
				contents.push_back(item);
			}
			else
			{
				// z0mg corrupted file data!
				return false;
			}
		} while(child->advance());
		return true;
	}
	return false;
}

bool Container::serializeItemNode_OTBM(const IOMap& maphandle, NodeFileWriteHandle& f) const
{
	f.addNode(OTBM_ITEM);
	
	f.addU16(id);
	if(maphandle.version.otbm == MAP_OTBM_1)
	{
		// In the ludicrous event that an item is a container AND stackable, we have to do this. :p
		const ItemType& iType = item_db[id];
		if(iType.stackable || iType.isSplash() || iType.isFluidContainer())
			f.addU8(getSubtype());
	}

	serializeItemAttributes_OTBM(maphandle, f);

	for(ItemVector::const_iterator it = contents.begin(); it != contents.end(); ++it)
		(*it)->serializeItemNode_OTBM(maphandle, f);

	f.endNode();
	return true;
}

/*
	OTBM_ROOTV1
	|
	|--- OTBM_MAP_DATA
	|	|
	|	|--- OTBM_TILE_AREA
	|	|	|--- OTBM_TILE
	|	|	|--- OTBM_TILE_SQUARE (not implemented)
	|	|	|--- OTBM_TILE_REF (not implemented)
	|	|	|--- OTBM_HOUSETILE
	|	|
	|	|--- OTBM_SPAWNS (not implemented)
	|	|	|--- OTBM_SPAWN_AREA (not implemented)
	|	|	|--- OTBM_MONSTER (not implemented)
	|	|
	|	|--- OTBM_TOWNS
	|		|--- OTBM_TOWN
	|
	|--- OTBM_ITEM_DEF (not implemented)
*/

bool IOMapOTBM::getVersionInfo(const FileName& filename, MapVersion& out_ver)
{
	if (filename.GetExt() == wxT("otgz"))
	{
		// Open the archive
		std::shared_ptr<struct archive> a(archive_read_new(), archive_read_free);
		archive_read_support_filter_all(a.get());
		archive_read_support_format_all(a.get());
		if (archive_read_open_filename(a.get(), nstr(filename.GetFullPath()).c_str(), 10240) != ARCHIVE_OK)
			 return false;

		// Loop over the archive entries until we find the otbm file
		struct archive_entry* entry;
		while (archive_read_next_header(a.get(), &entry) == ARCHIVE_OK)
		{
			std::string entryName = archive_entry_pathname(entry);

			if (entryName == "world/map.otbm")
			{
				// Read the OTBM header into temporary memory
				uint8_t buffer[8096];
				memset(buffer, 0, 8096);

				// Read from the archive
				int read_bytes = archive_read_data(a.get(), buffer, 8096);
				
				// Check so it at least contains the 4-byte file id
				if (read_bytes < 4)
					return false;
				
				// Create a read handle on it
				std::shared_ptr<NodeFileReadHandle> f(new MemoryNodeFileReadHandle(buffer + 4, read_bytes - 4));

				// Read the version info
				return getVersionInfo(f.get(), out_ver);
			}
		}

		// Didn't find OTBM file, lame
		return false;
	}
	else
	{
		// Just open a disk-based read handle
		DiskNodeFileReadHandle f(nstr(filename.GetFullPath()), StringVector(1, "OTBM"));
		if(f.isOk() == false)
			return false;
		return getVersionInfo(&f, out_ver);
	}
}

bool IOMapOTBM::getVersionInfo(NodeFileReadHandle* f,  MapVersion& out_ver)
{
	BinaryNode* root = f->getRootNode();
	if(!root)
		return false;

	root->skip(1); // Skip the type byte

	uint16_t u16;
	uint32_t u32;

	if(!root->getU32(u32)) // Version
		return false;
	out_ver.otbm = (MapVersionID)u32;


	root->getU16(u16);
	root->getU16(u16);
	root->getU32(u32);

	if(!root->getU32(u32)) // OTB minor version
		return false;

	out_ver.client = ClientVersionID(u32);
	return true;
}

bool IOMapOTBM::loadMap(Map& map, const FileName& filename)
{
	if (filename.GetExt() == wxT("otgz"))
	{
		// Open the archive
		std::shared_ptr<struct archive> a(archive_read_new(), archive_read_free);
		archive_read_support_filter_all(a.get());
		archive_read_support_format_all(a.get());
		if (archive_read_open_filename(a.get(), nstr(filename.GetFullPath()).c_str(), 10240) != ARCHIVE_OK)
			 return false;

		// Memory buffers for the houses & spawns
		std::shared_ptr<uint8_t> house_buffer;
		std::shared_ptr<uint8_t> spawn_buffer;
		size_t house_buffer_size = 0;
		size_t spawn_buffer_size = 0;

		// See if the otbm file has been loaded
		bool otbm_loaded = false;

		// Loop over the archive entries until we find the otbm file
		gui.SetLoadDone(0, wxT("Decompressing archive..."));
		struct archive_entry* entry;
		while (archive_read_next_header(a.get(), &entry) == ARCHIVE_OK)
		{
			std::string entryName = archive_entry_pathname(entry);

			if (entryName == "world/map.otbm")
			{
				// Read the entire OTBM file into a memory region
				size_t otbm_size = archive_entry_size(entry);
				std::shared_ptr<uint8_t> otbm_buffer(new uint8_t[otbm_size], [](uint8_t* p) { delete[] p; });
				
				// Read from the archive
				size_t read_bytes = archive_read_data(a.get(), otbm_buffer.get(), otbm_size);
				
				// Check so it at least contains the 4-byte file id
				if (read_bytes < 4)
					return false;

				if (read_bytes < otbm_size)
				{
					error(wxT("Could not read file."));
					return false;
				}

				gui.SetLoadDone(0, wxT("Loading OTBM map..."));

				// Create a read handle on it
				std::shared_ptr<NodeFileReadHandle> f(
					new MemoryNodeFileReadHandle(otbm_buffer.get() + 4, otbm_size - 4));

				// Read the version info
				if (!loadMap(map, *f.get()))
				{
					error(wxT("Could not load OTBM file inside archive"));
					return false;
				}

				otbm_loaded = true;
			}
			else if (entryName == "world/houses.xml")
			{
				house_buffer_size = archive_entry_size(entry);
				house_buffer.reset(new uint8_t[house_buffer_size]);
				
				// Read from the archive
				size_t read_bytes = archive_read_data(a.get(), house_buffer.get(), house_buffer_size);
				
				// Check so it at least contains the 4-byte file id
				if (read_bytes < house_buffer_size)
				{
					house_buffer.reset();
					house_buffer_size = 0;
					warning(wxT("Failed to decompress houses."));
				}
			}
			else if (entryName == "world/spawns.xml")
			{
				spawn_buffer_size = archive_entry_size(entry);
				spawn_buffer.reset(new uint8_t[spawn_buffer_size]);
				
				// Read from the archive
				size_t read_bytes = archive_read_data(a.get(), spawn_buffer.get(), spawn_buffer_size);
				
				// Check so it at least contains the 4-byte file id
				if (read_bytes < spawn_buffer_size)
				{
					spawn_buffer.reset();
					spawn_buffer_size = 0;
					warning(wxT("Failed to decompress spawns."));
				}
			}
		}

		if (!otbm_loaded)
		{
			error(wxT("OTBM file not found inside archive."));
			return false;
		}

		// Load the houses from the stored buffer
		if (house_buffer.get() && house_buffer_size > 0)
		{
			xmlDocPtr doc = xmlParseMemory((const char*)house_buffer.get(), house_buffer_size);
			if (doc)
			{
				if (!loadHouses(map, doc))
					warning(wxT("Failed to load houses."));
			}
			else
				warning(wxT("Failed to load houses due to XML parse error."));
		}

		// Load the spawns from the stored buffer
		if (spawn_buffer.get() && spawn_buffer_size > 0)
		{
			xmlDocPtr doc = xmlParseMemory((const char*)spawn_buffer.get(), spawn_buffer_size);
			if (doc)
			{
				if (!loadSpawns(map, doc))
					warning(wxT("Failed to load spawns."));
			}
			else
				warning(wxT("Failed to load spawns due to XML parse error."));
		}

		return true;
	}
	else
	{
		DiskNodeFileReadHandle f(nstr(filename.GetFullPath()), StringVector(1, "OTBM"));
		if(f.isOk() == false)
		{
			error((wxT("Couldn't open file for reading\nThe error reported was: ") + wxstr(f.getErrorMessage())).wc_str());
			return false;
		}

		if (!loadMap(map, f))
			return false;
		
		// Read auxilliary files
		if(!loadHouses(map, filename))
		{
			warning(wxT("Failed to load houses."));
			map.housefile = nstr(filename.GetName()) + "-house.xml";
		}
		if(!loadSpawns(map, filename))
		{
			warning(wxT("Failed to load spawns."));
			map.spawnfile = nstr(filename.GetName())+ "-spawn.xml";
		}

		return true;
	}
}

bool IOMapOTBM::loadMap(Map& map, NodeFileReadHandle& f)
{
	BinaryNode* root = f.getRootNode();
	if(!root)
	{
		error(wxT("Could not read root node."));
		return false;
	}
	root->skip(1); // Skip the type byte

	uint8_t u8;
	uint16_t u16;
	uint32_t u32;

	if(!root->getU32(u32))
		return false;

	version.otbm = (MapVersionID) u32;

	if(version.otbm > MAP_OTBM_4)
	{
		// Failed to read version
		if(gui.PopupDialog(wxT("Map error"), 
			wxT("The loaded map appears to be a OTBM format that is not supported by the editor.")
			wxT("Do you still want to attempt to load the map?"), wxYES | wxNO) == wxID_YES)
		{
			warning(wxT("Unsupported or damaged map version"));
		}
		else
		{
			error(wxT("Unsupported OTBM version, could not load map"));
			return false;
		}
	}

	if(!root->getU16(u16))
		return false;

	map.width = u16;
	if(!root->getU16(u16))
		return false;

	map.height = u16;

	if(!root->getU32(u32) || u32 > (unsigned long)item_db.MajorVersion) // OTB major version
	{ 
		if(gui.PopupDialog(wxT("Map error"), 
			wxT("The loaded map appears to be a items.otb format that deviates from the ")
			wxT("items.otb loaded by the editor. Do you still want to attempt to load the map?"), wxYES | wxNO) == wxID_YES)
		{
			warning(wxT("Unsupported or damaged map version"));
		}
		else
		{
			error(wxT("Outdated items.otb, could not load map"));
			return false;
		}
	}

	if(!root->getU32(u32) || u32 > (unsigned long)item_db.MinorVersion) // OTB minor version
	{
		warning(wxT("This editor needs an updated items.otb version"));
	}
	version.client = (ClientVersionID)u32;

	BinaryNode* mapHeaderNode = root->getChild();
	if(mapHeaderNode == NULL || !mapHeaderNode->getByte(u8) || u8 != OTBM_MAP_DATA)
	{
		error(wxT("Could not get root child node. Cannot recover from fatal error!"));
		return false;
	}

	uint8_t attribute;
	while(mapHeaderNode->getU8(attribute))
	{
		switch(attribute)
		{
			case OTBM_ATTR_DESCRIPTION:
			{
				if(!mapHeaderNode->getString(map.description))
				{
					warning(wxT("Invalid map description tag"));
				}
				//std::cout << "Map description: " << mapDescription << std::endl;
			} break;
			case OTBM_ATTR_EXT_SPAWN_FILE:
			{
				if(!mapHeaderNode->getString(map.spawnfile))
				{
					warning(wxT("Invalid map spawnfile tag"));
				}
			} break;
			case OTBM_ATTR_EXT_HOUSE_FILE:
			{
				if(!mapHeaderNode->getString(map.housefile))
				{
					warning(wxT("Invalid map housefile tag"));
				}
			} break;
			default:
			{
				warning(wxT("Unknown header node."));
			} break;
		}
	}

	
	int nodes_loaded = 0;

	for(BinaryNode* mapNode = mapHeaderNode->getChild(); mapNode != NULL; mapNode = mapNode->advance())
	{
		++nodes_loaded;
		if(nodes_loaded % 15 == 0)
			gui.SetLoadDone(int(100.0 * f.tell() / f.size()));
		
		uint8_t node_type;
		if(!mapNode->getByte(node_type))
		{
			warning(wxT("Invalid map node"));
			continue;
		}
		if(node_type == OTBM_TILE_AREA)
		{
			uint16_t base_x, base_y;
			uint8_t base_z;
			if(!mapNode->getU16(base_x) ||
					!mapNode->getU16(base_y) ||
					!mapNode->getU8(base_z))
			{
				warning(wxT("Invalid map node, no base coordinate"));
				continue;
			}

			for(BinaryNode* tileNode = mapNode->getChild(); tileNode != NULL; tileNode = tileNode->advance())
			{
				Tile* tile = NULL;
				uint8_t tile_type;
				if(!tileNode->getByte(tile_type))
				{
					warning(wxT("Invalid tile type"));
					continue;
				}
				if(tile_type == OTBM_TILE || tile_type == OTBM_HOUSETILE)
				{
					//printf("Start\n");
					uint8_t x_offset, y_offset;
					if(!tileNode->getU8(x_offset) || !tileNode->getU8(y_offset))
					{
						warning(wxT("Could not read position of tile"));
						continue;
					}
					const Position pos(base_x + x_offset, base_y + y_offset, base_z);
					
					if(map.getTile(pos))
					{
						warning(wxT("Duplicate tile at %d:%d:%d, discarding duplicate"), pos.x, pos.y, pos.z);
						continue;
					}
					
					tile = map.allocator(map.createTileL(pos));
					House* house = NULL;
					if(tile_type == OTBM_HOUSETILE)
					{
						uint32_t house_id;
						if(!tileNode->getU32(house_id))
						{
							warning(wxT("House tile without house data, discarding tile"));
							continue;
						}
						if(house_id)
						{
							house = map.houses.getHouse(house_id);
							if(!house)
							{
								house = newd House(map);
								house->id = house_id;
								map.houses.addHouse(house);
							}
						}
						else
						{
							warning(wxT("Invalid house id from tile %d:%d:%d"), pos.x, pos.y, pos.z);
						}
					}

					//printf("So far so good\n");

					uint8_t attribute;
					while(tileNode->getU8(attribute))
					{
						switch(attribute)
						{
							case OTBM_ATTR_TILE_FLAGS:
							{
								uint32_t flags = 0;
								if(!tileNode->getU32(flags)) {
									warning(wxT("Invalid tile flags of tile on %d:%d:%d"), pos.x, pos.y, pos.z);
								}
								tile->setMapFlags(flags);
							} break;
							case OTBM_ATTR_ITEM:
							{
								Item* item = Item::Create_OTBM(*this, tileNode);
								if(item == NULL)
								{
									warning(wxT("Invalid item at tile %d:%d:%d"), pos.x, pos.y, pos.z);
								}
								tile->addItem(item);
							} break;
							default:
							{
								warning(wxT("Unknown tile attribute at %d:%d:%d"), pos.x, pos.y, pos.z);
							} break;
						}
					}

					//printf("Didn't die in loop\n");

					
					for(BinaryNode* itemNode = tileNode->getChild(); itemNode != NULL; itemNode = itemNode->advance())
					{
						Item* item = NULL;
						uint8_t item_type;
						if(!itemNode->getByte(item_type))
						{
							warning(wxT("Unknown item type %d:%d:%d"), pos.x, pos.y, pos.z);
							continue;
						}
						if(item_type == OTBM_ITEM)
						{
							item = Item::Create_OTBM(*this, itemNode);
							if(item)
							{
								if(item->unserializeItemNode_OTBM(*this, itemNode) == false)
								{
									warning(wxT("Couldn't unserialize item attributes at %d:%d:%d"), pos.x, pos.y, pos.z);
								}
								//reform(&map, tile, item);
								tile->addItem(item);
							}
						}
						else
						{
							warning(wxT("Unknown type of tile child node"));
						}
					}

					tile->update();
					if(house)
						house->addTile(tile);

					map.setTile(pos.x, pos.y, pos.z, tile);
				}
				else
				{
					warning(wxT("Unknown type of tile node"));
				}
			}
		}
		else if(node_type == OTBM_TOWNS)
		{
			for(BinaryNode* townNode = mapNode->getChild(); townNode != NULL; townNode = townNode->advance())
			{
				Town* town = NULL;
				uint8_t town_type;
				if(!townNode->getByte(town_type))
				{
					warning(wxT("Invalid town type (1)"));
					continue;
				}
				if(town_type != OTBM_TOWN)
				{
					warning(wxT("Invalid town type (2)"));
					continue;
				}
				uint32_t town_id;
				if(!townNode->getU32(town_id))
				{
					warning(wxT("Invalid town id"));
					continue;
				}

				town = map.towns.getTown(town_id);
				if(town)
				{
					warning(wxT("Duplicate town id %d, discarding duplicate"), town_id);
					continue;
				}
				else
				{
					town = newd Town(town_id);
					if(!map.towns.addTown(town))
					{
						delete town;
						continue;
					}
				}
				std::string town_name;
				if(!townNode->getString(town_name))
				{
					warning(wxT("Invalid town name"));
					continue;
				}
				town->setName(town_name);
				Position pos;
				uint16_t x;
				uint16_t y;
				uint8_t z;
				if(!townNode->getU16(x) ||
						!townNode->getU16(y) ||
						!townNode->getU8(z))
				{
					warning(wxT("Invalid town temple position"));
					continue;
				}
				pos.x = x;
				pos.y = y;
				pos.z = z;
				town->setTemplePosition(pos);
			}
		}
		else if(node_type == OTBM_WAYPOINTS)
		{
			for(BinaryNode* waypointNode = mapNode->getChild(); waypointNode != NULL; waypointNode = waypointNode->advance())
			{
				uint8_t waypoint_type;
				if(!waypointNode->getByte(waypoint_type))
				{
					warning(wxT("Invalid waypoint type (1)"));
					continue;
				}
				if(waypoint_type != OTBM_WAYPOINT)
				{
					warning(wxT("Invalid waypoint type (2)"));
					continue;
				}
				
				Waypoint wp;

				if(!waypointNode->getString(wp.name))
				{
					warning(wxT("Invalid waypoint name"));
					continue;
				}
				uint16_t x;
				uint16_t y;
				uint8_t z;
				if(!waypointNode->getU16(x) ||
						!waypointNode->getU16(y) ||
						!waypointNode->getU8(z))
				{
					warning(wxT("Invalid waypoint position"));
					continue;
				}
				wp.pos.x = x;
				wp.pos.y = y;
				wp.pos.z = z;

				map.waypoints.addWaypoint(newd Waypoint(wp));
			}
		}
	}

	if(!f.isOk())
		warning(wxstr(f.getErrorMessage()).wc_str());
	return true;
}

bool IOMapOTBM::loadSpawns(Map& map, const FileName& dir)
{
	std::string fn = (const char*)(dir.GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME).mb_str(wxConvUTF8));
	fn += map.spawnfile;
	FileName filename(wxstr(fn));
	if(filename.FileExists() == false)
		return false;

	xmlDocPtr doc = xmlParseFile(fn.c_str());
	if (!doc)
		return false;
	return loadSpawns(map, doc);
}

bool IOMapOTBM::loadSpawns(Map& map, xmlDocPtr doc)
{
	xmlNodePtr root = xmlDocGetRootElement(doc);

	if(xmlStrcmp(root->name,(const xmlChar*)"spawns") != 0)
		return false;


	int intVal;

	xmlNodePtr spawnNode = root->children;
	while(spawnNode) 
	{
		do if(xmlStrcmp(spawnNode->name,(const xmlChar*)"spawn") == 0)
		{
			bool posok = true;
			Position spawnpos;

			if(readXMLInteger(spawnNode, "centerx", intVal))
				spawnpos.x = intVal; else posok = false;
			if(readXMLInteger(spawnNode, "centery", intVal))
				spawnpos.y = intVal; else posok = false;
			if(readXMLInteger(spawnNode, "centerz", intVal))
				spawnpos.z = intVal; else posok = false;

			if(posok == false)
			{
				warning(wxT("Bad position data on one spawn, discarding..."));
				break;
			}

			int radius = 1;
			if(readXMLInteger(spawnNode, "radius", intVal))
			{
				radius = max(1, intVal);	
			}
			else
			{
				warning(wxT("Couldn't read radius of spawn.. discarding spawn..."));
				continue;
			}

			Tile* tile = map.getTile(spawnpos);
			if(tile && tile->spawn)
			{
				warning(wxT("Duplicate spawn on position %d:%d:%d\n"), tile->getX(), tile->getY(), tile->getZ());
				break;
			}

			Spawn* spawn = newd Spawn(radius);
			if(!tile)
			{
				tile = map.allocator(map.createTileL(spawnpos));
				map.setTile(spawnpos, tile);
			}
			tile->spawn = spawn;
			map.addSpawn(tile);

			xmlNodePtr creatureNode = spawnNode->children;
			while(creatureNode)
			{
				if(xmlStrcmp(creatureNode->name,(const xmlChar*)"monster") == 0 || xmlStrcmp(creatureNode->name,(const xmlChar*)"npc") == 0) 
				do {
					std::string name;
					int spawntime = settings.getInteger(Config::DEFAULT_SPAWNTIME);
					bool isNpc = (xmlStrcmp(creatureNode->name,(const xmlChar*)"npc") == 0);
					if(!readXMLValue(creatureNode, "name", name))
					{
						wxString err;
						err <<  "Bad creature position data, discarding creature at spawn " << spawnpos.x << ":" << spawnpos.y << ":" << spawnpos.z << " due missing name.";
						warnings.Add(err);
						break;
					}
					if(name == "")
						break;

					readXMLValue(creatureNode, "spawntime", spawntime);
					Position creaturepos(spawnpos);
					posok = true;

					if(readXMLInteger(creatureNode, "x", intVal))
						creaturepos.x += intVal;
					else posok = false;

					if(readXMLInteger(creatureNode, "y", intVal))
						creaturepos.y += intVal;
					else posok = false;

					if(posok == false)
					{
						wxString err;
						err <<  "Bad creature position data, discarding creature \"" << name << "\" at spawn " << spawnpos.x << ":" << spawnpos.y << ":" << spawnpos.z << " due to invalid position.";
						warnings.Add(err);
						break;
					}

					if(abs(creaturepos.x - spawnpos.x) > radius)
						radius = abs(creaturepos.x - spawnpos.x);
					if(abs(creaturepos.y - spawnpos.y) > radius)
						radius = abs(creaturepos.y - spawnpos.y);

					radius = min(radius, settings.getInteger(Config::MAX_SPAWN_RADIUS));

					Tile* creature_tile;
					if(creaturepos == spawnpos)
						creature_tile = tile;
					else
						creature_tile = map.getTile(creaturepos);

					if(!creature_tile)
					{
						wxString err;
						err <<  "Discarding creature \"" << name << "\" at " << creaturepos.x << ":" << creaturepos.y << ":" << creaturepos.z << " due to invalid position.";
						warnings.Add(err);
						break;
					}
					if(creature_tile->creature)
					{
						wxString err;
						err <<  "Duplicate creature \"" << name << "\" at " << creaturepos.x << ":" << creaturepos.y << ":" << creaturepos.z << " was discarded.";
						warnings.Add(err);
						break;
					}
					CreatureType* type = creature_db[name];
					if(!type)
						type = creature_db.addMissingCreatureType(name, isNpc);

					Creature* creature = newd Creature(type);
					creature->setSpawnTime(spawntime);
					creature_tile->creature = creature;

					if(creature_tile->getLocation()->getSpawnCount() == 0)
					{
						// No spawn, create a newd one
						ASSERT(creature_tile->spawn == NULL);
						Spawn* spawn = newd Spawn(5);
						creature_tile->spawn = spawn;
						map.addSpawn(creature_tile);
					}
				} while(false);
				creatureNode = creatureNode->next;
			}
		} while(false);
		spawnNode = spawnNode->next;
	}

	return true;
}

bool IOMapOTBM::loadHouses(Map& map, const FileName& dir)
{
	std::string fn = (const char*)(dir.GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME).mb_str(wxConvUTF8));
	fn += map.housefile;
	FileName filename(wxstr(fn));
	if(filename.FileExists() == false)
		return false;

	xmlDocPtr doc = xmlParseFile(fn.c_str());
	if (!doc)
		return false;
	return loadHouses(map, doc);
}

bool IOMapOTBM::loadHouses(Map& map, xmlDocPtr doc)
{
	xmlNodePtr root = xmlDocGetRootElement(doc);

	if(xmlStrcmp(root->name,(const xmlChar*)"houses") != 0)
		return false;

	int intVal;
	std::string strVal;

	xmlNodePtr houseNode = root->children;
	while(houseNode)
	{
		do if(xmlStrcmp(houseNode->name,(const xmlChar*)"house") == 0)
		{
			House* house = NULL;
			if(readXMLInteger(houseNode, "houseid", intVal))
			{
				house = map.houses.getHouse(intVal);
				if(!house)
					break;
			}

			if(readXMLString(houseNode, "name", strVal))
				house->name = strVal;
			else
				house->name = "House #" + house->id;

			bool posok = true;
			Position houseexit;

			if(readXMLInteger(houseNode, "entryx", intVal))
				houseexit.x = intVal; else posok = false;
			if(readXMLInteger(houseNode, "entryy", intVal))
				houseexit.y = intVal; else posok = false;
			if(readXMLInteger(houseNode, "entryz", intVal))
				houseexit.z = intVal; else posok = false;
			if(posok)
				house->setExit(houseexit);

			if(readXMLInteger(houseNode, "rent", intVal))
				house->rent = intVal;

			if(readXMLInteger(houseNode, "guildhall", intVal))
				house->guildhall = (intVal != 0);

			if(readXMLInteger(houseNode, "townid", intVal))
			{
				house->townid = intVal;
			}
			else
			{
				warning(wxT("House %d has no town! House was removed."), house->id);
				map.houses.removeHouse(house);
			}
		} while(false);
		houseNode = houseNode->next;
	}

	return true;
}

struct xmlToMemoryContext
{
	uint8_t* data;
	size_t position;
	size_t length;
};

int writeXmlToMemoryWriteCallback(void* context, const char* buffer, int len)
{
	xmlToMemoryContext* ctx = (xmlToMemoryContext*)context;
	if (ctx->position + len > ctx->length)
	{
		// Quite aggressive growth, but we don't want too many moves here
		ctx->data = (uint8_t*)realloc(ctx->data, ctx->length * 4);
		ctx->length *= 4;
	}
	memcpy(ctx->data + ctx->position, buffer, len);
	ctx->position += len;
	return len;
}

int writeXmlToMemoryCloseCallback(void* context)
{
	return 0;
}

bool IOMapOTBM::saveMap(Map& map, const FileName& identifier)
{
	if (identifier.GetExt() == "otgz")
	{

		// Create the archive
		struct archive* a = archive_write_new();
		struct archive_entry* entry = NULL;

		archive_write_set_compression_gzip(a);
		archive_write_set_format_pax_restricted(a);
		archive_write_open_filename(a, nstr(identifier.GetFullPath()).c_str());

		// Start out at 100kb memory for the XML files
		xmlToMemoryContext xmlSaveData = {
			(uint8_t*)malloc(1024*100),
			0,
			1024*100
		};

		gui.SetLoadDone(0, wxT("Saving spawns..."));
		if(xmlDocPtr spawnDoc = saveSpawns(map))
		{
			// Write the data
			xmlSaveData.position = 0;
			xmlSaveCtxt* xmlContext = xmlSaveToIO(writeXmlToMemoryWriteCallback, writeXmlToMemoryCloseCallback, &xmlSaveData, "UTF-8", XML_SAVE_FORMAT);
			xmlSaveDoc(xmlContext, spawnDoc);
			xmlSaveClose(xmlContext);
			xmlFreeDoc(spawnDoc);
			spawnDoc = NULL;

			// Write to the arhive
			entry = archive_entry_new();
			archive_entry_set_pathname(entry, "world/spawns.xml");
			archive_entry_set_size(entry, xmlSaveData.position);
			archive_entry_set_filetype(entry, AE_IFREG);
			archive_entry_set_perm(entry, 0644);
			archive_write_header(a, entry);

			// Write to the archive
			archive_write_data(a, xmlSaveData.data, xmlSaveData.position);
			
			// Free the entry
			archive_entry_free(entry);
		}
		
		gui.SetLoadDone(0, wxT("Saving houses..."));
		if (xmlDocPtr houseDoc = saveHouses(map))
		{
			// Write the data
			xmlSaveData.position = 0;
			xmlSaveCtxt* xmlContext = xmlSaveToIO(writeXmlToMemoryWriteCallback, writeXmlToMemoryCloseCallback, &xmlSaveData, "UTF-8", XML_SAVE_FORMAT);
			xmlSaveDoc(xmlContext, houseDoc);
			xmlSaveClose(xmlContext);
			xmlFreeDoc(houseDoc);
			houseDoc = NULL;

			// Write to the arhive
			entry = archive_entry_new();
			archive_entry_set_pathname(entry, "world/houses.xml");
			archive_entry_set_size(entry, xmlSaveData.position);
			archive_entry_set_filetype(entry, AE_IFREG);
			archive_entry_set_perm(entry, 0644);
			archive_write_header(a, entry);

			// Write to the archive
			archive_write_data(a, xmlSaveData.data, xmlSaveData.position);
			
			// Free the entry
			archive_entry_free(entry);
		}
		
		// Free the xml context
		free(xmlSaveData.data);

		gui.SetLoadDone(0, wxT("Saving OTBM map..."));
		MemoryNodeFileWriteHandle otbmWriter;
		saveMap(map, otbmWriter);

		gui.SetLoadDone(100, wxT("Compressing..."));
		
		// Create an archive entry for the otbm file
		entry = archive_entry_new();
		archive_entry_set_pathname(entry, "world/map.otbm");
		archive_entry_set_size(entry, otbmWriter.getSize() + 4); // 4 bytes extra for header
		archive_entry_set_filetype(entry, AE_IFREG);
		archive_entry_set_perm(entry, 0644);
		archive_write_header(a, entry);
		
		// Write the version header
		char otbm_identifier[] = "OTBM";
		archive_write_data(a, otbm_identifier, 4);

		// Write the OTBM data
		archive_write_data(a, otbmWriter.getMemory(), otbmWriter.getSize());

		archive_entry_free(entry);

		// Free / close the archive
		archive_write_close(a); 
		archive_write_free(a);

		return true;
	}
	else
	{
		DiskNodeFileWriteHandle f(
			nstr(identifier.GetFullPath()),
			(settings.getInteger(Config::SAVE_WITH_OTB_MAGIC_NUMBER) ? "OTBM" : std::string(4, '\0'))
		);
	
		if(f.isOk() == false)
		{
			error(wxT("Can not open file %s for writing"), (const char*)identifier.GetFullPath().mb_str(wxConvUTF8));
			return false;
		}

		if (!saveMap(map, f))
			return false;

		gui.SetLoadDone(99, wxT("Saving spawns..."));
		saveSpawns(map, identifier);

		gui.SetLoadDone(99, wxT("Saving houses..."));
		saveHouses(map, identifier);

		return true;
	}

	// No way to save with this extension
	return false;
}

bool IOMapOTBM::saveMap(Map& map, NodeFileWriteHandle& f)
{
	/* STOP!
	 * Before you even think about modifying this, please reconsider.
	 * while adding stuff to the binary format may be "cool", you'll
	 * inevitably make it incompatible with any future releases of
	 * the map editor, meaning you cannot reuse your map. Before you
	 * try to modify this, PLEASE consider using an external file
	 * like spawns.xml or houses.xml, as that will be MUCH easier
	 * to port to newer versions of the editor than a custom binary
	 * format.
	 */
	
	MapVersion mapver = map.getVersion();

	f.addNode(0);
	{
		f.addU32(mapver.otbm); // Version
		f.addU16(map.width);
		f.addU16(map.height);
		f.addU32(item_db.MajorVersion);
		f.addU32(item_db.MinorVersion);

		f.addNode(OTBM_MAP_DATA);
		{
			f.addByte(OTBM_ATTR_DESCRIPTION);
			// Neither SimOne's nor OpenTibia cares for additional description tags
			f.addString("Saved with Remere's Map Editor " + __RME_VERSION__);

			f.addU8(OTBM_ATTR_DESCRIPTION);
			f.addString(map.description);

			f.addU8(OTBM_ATTR_EXT_SPAWN_FILE);
			FileName fn(wxstr(map.spawnfile));
			f.addString(std::string((const char*)fn.GetFullName().mb_str(wxConvUTF8)));
			
			f.addU8(OTBM_ATTR_EXT_HOUSE_FILE);
			fn.Assign(wxstr(map.housefile));
			f.addString(std::string((const char*)fn.GetFullName().mb_str(wxConvUTF8)));

			// Start writing tiles
			uint tiles_saved = 0;
			bool first = true;

			int local_x = -1, local_y = -1, local_z = -1;

			MapIterator map_iterator = map.begin();
			while(map_iterator != map.end())
			{
				// Update progressbar
				++tiles_saved;
				if(tiles_saved % 8192 == 0)
					gui.SetLoadDone(int(tiles_saved / double(map.getTileCount()) * 100.0));

				// Get tile
				Tile* save_tile = (*map_iterator)->get();

				// Is it an empty tile that we can skip? (Leftovers...)
				if(!save_tile || save_tile->size() == 0)
				{
					++map_iterator;
					continue;
				}

				const Position& pos = save_tile->getPosition();

				// Decide if newd node should be created
				if(pos.x < local_x || pos.x >= local_x + 256 ||
				   pos.y < local_y || pos.y >= local_y + 256 ||
				   pos.z != local_z)
				{
					// End last node
					if(!first)
					{
						f.endNode();
					}
					first = false;

					// Start newd node
					f.addNode(OTBM_TILE_AREA);
					f.addU16(local_x = pos.x & 0xFF00);
					f.addU16(local_y = pos.y & 0xFF00);
					f.addU8( local_z = pos.z);
				}
				f.addNode(save_tile->isHouseTile()? OTBM_HOUSETILE : OTBM_TILE);
				
				f.addU8(save_tile->getX() & 0xFF);
				f.addU8(save_tile->getY() & 0xFF);

				if(save_tile->isHouseTile())
				{
					f.addU32(save_tile->getHouseID());
				}

				if(save_tile->getMapFlags())
				{
					f.addByte(OTBM_ATTR_TILE_FLAGS);
					f.addU32(save_tile->getMapFlags());
				}

				if(save_tile->ground)
				{
					Item* ground = save_tile->ground;
					if(ground->isMetaItem())
					{
						// Do nothing, we don't save metaitems...
					}
					else if(ground->hasBorderEquivalent())
					{
						bool found = false;
						for(ItemVector::iterator it = save_tile->items.begin(); it != save_tile->items.end(); ++it)
						{
							if((*it)->getGroundEquivalent() == ground->getID())
							{
								// Do nothing
								// Found equivalent
								found = true;
								break;
							}
						}

						if(found == false)
						{
							ground->serializeItemNode_OTBM(*this, f);
						}

					}
					else if(ground->isComplex())
					{
						ground->serializeItemNode_OTBM(*this, f);
					}
					else
					{
						f.addByte(OTBM_ATTR_ITEM);
						ground->serializeItemCompact_OTBM(*this, f);
					}
				}

				for(ItemVector::iterator it = save_tile->items.begin(); it != save_tile->items.end(); ++it)
				{
					if(!(*it)->isMetaItem())
					{
						(*it)->serializeItemNode_OTBM(*this, f);
					}
				}
				f.endNode();

				++map_iterator;
			}
			// Only close the last node if one has actually been created
			if(!first)
			{
				f.endNode();
			}

			f.addNode(OTBM_TOWNS);
			{
				for(TownMap::const_iterator it = map.towns.begin(); it != map.towns.end(); ++it)
				{
					Town* town = it->second;
					f.addNode(OTBM_TOWN);

					f.addU32(town->getID());
					f.addString(town->getName());
					f.addU16(town->getTemplePosition().x);
					f.addU16(town->getTemplePosition().y);
					f.addU8 (town->getTemplePosition().z);
					f.endNode();
				}
			} f.endNode();

			if(version.otbm >= MAP_OTBM_3)
			{
				f.addNode(OTBM_WAYPOINTS);
				{
					for(WaypointMap::const_iterator it = map.waypoints.begin(); it != map.waypoints.end(); ++it)
					{
						Waypoint* waypoint = it->second;
						f.addNode(OTBM_WAYPOINT);

						f.addString(waypoint->name);
						f.addU16(waypoint->pos.x);
						f.addU16(waypoint->pos.y);
						f.addU8 (waypoint->pos.z);
						f.endNode();
					}
				} f.endNode();
			}
		} f.endNode();
	} f.endNode();

	return true;
}

bool IOMapOTBM::saveSpawns(Map& map, const FileName& dir)
{
	wxString wpath = dir.GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME);
	std::string filename = std::string(wpath.mb_str(wxConvUTF8)) + map.spawnfile;

	// Create the XML file
	if (xmlDocPtr doc = saveSpawns(map))
	{
		// Store it on disk
		bool result = xmlSaveFormatFileEnc(filename.c_str(), doc, "UTF-8", 1);
		xmlFreeDoc(doc);
		return result;
	}

	return false;
}

xmlDocPtr IOMapOTBM::saveSpawns(Map& map)
{
	xmlDocPtr doc = xmlNewDoc((const xmlChar*)"1.0");
	doc->children = xmlNewDocNode(doc, NULL, (const xmlChar*)"spawns", NULL);

	xmlNodePtr root = doc->children;

	Spawns& spawns = map.spawns;
	CreatureList creature_list;
	for(SpawnPositionList::const_iterator iter = spawns.begin();
			iter != spawns.end();
			++iter)
	{
		Tile* tile = map.getTile(*iter);
		ASSERT(tile);
		Spawn* spawn = tile->spawn;
		ASSERT(spawn);
		xmlNodePtr spawn_child = xmlNewNode(NULL,(const xmlChar*)"spawn");

		xmlSetProp(spawn_child, (const xmlChar*)"centerx", (const xmlChar*)i2s(iter->x).c_str());
		xmlSetProp(spawn_child, (const xmlChar*)"centery", (const xmlChar*)i2s(iter->y).c_str());
		xmlSetProp(spawn_child, (const xmlChar*)"centerz", (const xmlChar*)i2s(iter->z).c_str());
		xmlSetProp(spawn_child, (const xmlChar*)"radius", (const xmlChar*)i2s(spawn->getSize()).c_str());

		for(int y = -tile->spawn->getSize(); y <= tile->spawn->getSize(); ++y)
		{
			for(int x = -tile->spawn->getSize(); x <= tile->spawn->getSize(); ++x)
			{
				Tile* creature_tile = map.getTile(*iter + Position(x, y, 0));
				if(creature_tile)
				{
					Creature* c = creature_tile->creature;
					if(c && c->isSaved() == false)
					{
						xmlNodePtr creature_child = xmlNewNode(NULL,(const xmlChar*)(c->isNpc()? "npc" : "monster"));

						xmlSetProp(creature_child, (const xmlChar*)"name", (const xmlChar*)c->getName().c_str());
						xmlSetProp(creature_child, (const xmlChar*)"x", (const xmlChar*)i2s(x).c_str());
						xmlSetProp(creature_child, (const xmlChar*)"y", (const xmlChar*)i2s(y).c_str());
						xmlSetProp(creature_child, (const xmlChar*)"z", (const xmlChar*)i2s(iter->z).c_str());
						xmlSetProp(creature_child, (const xmlChar*)"spawntime", (const xmlChar*)i2s(c->getSpawnTime()).c_str());

						xmlAddChild(spawn_child, creature_child);

						// Mark as saved
						c->save();
						creature_list.push_back(c);
					}
				}
			}
		}
		xmlAddChild(root, spawn_child);
	}

	for(CreatureList::iterator iter = creature_list.begin();
			iter != creature_list.end();
			++iter)
	{
		(*iter)->reset();
	}

	return doc;
}

bool IOMapOTBM::saveHouses(Map& map, const FileName& dir)
{
	wxString wpath = dir.GetPath(wxPATH_GET_SEPARATOR | wxPATH_GET_VOLUME);
	std::string filename = std::string(wpath.mb_str(wxConvUTF8)) + map.housefile;

	// Create the XML file
	if (xmlDocPtr doc = saveHouses(map))
	{
		// Store it on disk
		bool result = xmlSaveFormatFileEnc(filename.c_str(), doc, "UTF-8", 1);
		xmlFreeDoc(doc);
		return result;
	}

	return false;
}

xmlDocPtr IOMapOTBM::saveHouses(Map& map)
{
	xmlDocPtr doc = xmlNewDoc((const xmlChar*)"1.0");
	doc->children = xmlNewDocNode(doc, NULL, (const xmlChar*)"houses", NULL);

	xmlNodePtr root = doc->children;

	for(HouseMap::const_iterator house_iter = map.houses.begin();
			house_iter != map.houses.end();
			++house_iter)
	{
		const House* house = house_iter->second;
		xmlNodePtr child = xmlNewNode(NULL,(const xmlChar*)"house");

		xmlSetProp(child, (const xmlChar*)"name", (const xmlChar*)house->name.c_str());
		xmlSetProp(child, (const xmlChar*)"houseid", (const xmlChar*)i2s(house->id).c_str());

		xmlSetProp(child, (const xmlChar*)"entryx", (const xmlChar*)i2s(house->getExit().x).c_str());
		xmlSetProp(child, (const xmlChar*)"entryy", (const xmlChar*)i2s(house->getExit().y).c_str());
		xmlSetProp(child, (const xmlChar*)"entryz", (const xmlChar*)i2s(house->getExit().z).c_str());

		xmlSetProp(child, (const xmlChar*)"rent", (const xmlChar*)i2s(house->rent).c_str());
		if(house->guildhall)
			xmlSetProp(child, (const xmlChar*)"guildhall", (const xmlChar*)"1");

		xmlSetProp(child, (const xmlChar*)"townid", (const xmlChar*)i2s(house->townid).c_str());
		xmlSetProp(child, (const xmlChar*)"size", (const xmlChar*)i2s(house->size()).c_str());

		xmlAddChild(root, child);
	}

	return doc;
}
