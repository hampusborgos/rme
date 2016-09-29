//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "live_socket.h"
#include "map_region.h"
#include "iomap_otbm.h"
#include "live_tab.h"
#include "editor.h"

LiveSocket::LiveSocket() :
	cursors(), mapReader(nullptr, 0), mapWriter(),
	mapVersion(MapVersion(MAP_OTBM_4, CLIENT_VERSION_NONE)), log(nullptr),
	name(wxT("User")), password(wxT(""))
{
	//
}

LiveSocket::~LiveSocket()
{
	//
}

wxString LiveSocket::getName() const
{
	return name;
}

bool LiveSocket::setName(const wxString& newName)
{
	if(newName.empty()) {
		setLastError(wxT("Must provide a name."));
		return false;
	} else if(newName.length() > 32) {
		setLastError(wxT("Name is too long."));
		return false;
	}
	name = newName;
	return true;
}

wxString LiveSocket::getPassword() const
{
	return password;
}

bool LiveSocket::setPassword(const wxString& newPassword)
{
	if(newPassword.length() > 32) {
		setLastError(wxT("Password is too long."));
		return false;
	}
	password = newPassword;
	return true;
}

wxString LiveSocket::getLastError() const
{
	return lastError;
}

void LiveSocket::setLastError(const wxString& error)
{
	lastError = error;
}

std::string LiveSocket::getHostName() const
{
	return "?";
}

std::vector<LiveCursor> LiveSocket::getCursorList() const
{
	std::vector<LiveCursor> cursorList;
	for(auto& cursorEntry : cursors) {
		cursorList.push_back(cursorEntry.second);
	}
	return cursorList;
}

void LiveSocket::logMessage(const wxString& message)
{
	wxTheApp->CallAfter([this, message]() {
		if(log) {
			log->Message(message);
		}
	});
}

void LiveSocket::receiveNode(NetworkMessage& message, Editor& editor, Action* action, int32_t ndx, int32_t ndy, bool underground)
{
	QTreeNode* node = editor.map.getLeaf(ndx * 4, ndy * 4);
	if(!node) {
		log->Message(wxT("Warning: Received update for unknown tile (") + std::to_string(ndx * 4) + wxT("/") + std::to_string(ndy * 4) + wxT("/") + (underground ? "true" : "false") + wxT(")"));
		return;
	}

	node->setRequested(underground, false);
	node->setVisible(underground, true);

	uint16_t floorBits = message.read<uint16_t>();
	if(floorBits == 0) {
		return;
	}

	for(uint_fast8_t z = 0; z < 16; ++z) {
		if(testFlags(floorBits, 1 << z)) {
			receiveFloor(message, editor, action, ndx, ndy, z, node, node->getFloor(z));
		}
	}
}

void LiveSocket::sendNode(uint32_t clientId, QTreeNode* node, int32_t ndx, int32_t ndy, uint32_t floorMask)
{
	bool underground;
	if(floorMask & 0xFF00) {
		if(floorMask & 0x00FF) {
			underground = false;
		} else {
			underground = true;
		}
	} else {
		underground = false;
	}

	node->setVisible(clientId, underground, true);

	// Send message
	NetworkMessage message;
	message.write<uint8_t>(PACKET_NODE);
	message.write<uint32_t>((ndx << 18) | (ndy << 4) | ((floorMask & 0xFF00) ? 1 : 0));

	if(!node) {
		message.write<uint8_t>(0x00);
	} else {
		Floor** floors = node->getFloors();

		uint16_t sendMask = 0;
		for(uint32_t z = 0; z < 16; ++z) {
			uint32_t bit = 1 << z;
			if(floors[z] && testFlags(floorMask, bit)) {
				sendMask |= bit;
			}
		}

		message.write<uint16_t>(sendMask);
		for(uint32_t z = 0; z < 16; ++z) {
			if(testFlags(sendMask, 1 << z)) {
				sendFloor(message, floors[z]);
			}
		}
	}

	send(message);
}

void LiveSocket::receiveFloor(NetworkMessage& message, Editor& editor, Action* action, int32_t ndx, int32_t ndy, int32_t z, QTreeNode* node, Floor* floor)
{
	Map& map = editor.map;

	uint16_t tileBits = message.read<uint16_t>();
	if(tileBits == 0) {
		for(uint_fast8_t x = 0; x < 4; ++x) {
			for(uint_fast8_t y = 0; y < 4; ++y) {
				action->addChange(new Change(map.allocator(node->createTile(ndx * 4 + x, ndy * 4 + y, z))));
			}
		}
		return;
	}

	// -1 on address since we skip the first START_NODE when sending
	const std::string& data = message.read<std::string>();
	mapReader.assign(reinterpret_cast<const uint8_t*>(data.c_str() - 1), data.size());

	BinaryNode* rootNode = mapReader.getRootNode();
	BinaryNode* tileNode = rootNode->getChild();

	Position position(0, 0, z);
	for(uint_fast8_t x = 0; x < 4; ++x) {
		for(uint_fast8_t y = 0; y < 4; ++y) {
			position.x = (ndx * 4) + x;
			position.y = (ndy * 4) + y;

			if(testFlags(tileBits, 1 << ((x * 4) + y))) {
				receiveTile(tileNode, editor, action, &position);
				tileNode->advance();
			} else {
				action->addChange(new Change(map.allocator(node->createTile(position.x, position.y, z))));
			}
		}
	}
	mapReader.close();
}

void LiveSocket::sendFloor(NetworkMessage& message, Floor* floor)
{
	uint16_t tileBits = 0;
	for(uint_fast8_t x = 0; x < 4; ++x) {
		for(uint_fast8_t y = 0; y < 4; ++y) {
			uint_fast8_t index = (x * 4) + y;

			Tile* tile = floor->locs[index].get();
			if(tile && tile->size() > 0) {
				tileBits |= (1 << index);
			}
		}
	}

	message.write<uint16_t>(tileBits);
	if(tileBits == 0) {
		return;
	}

	mapWriter.reset();
	for(uint_fast8_t x = 0; x < 4; ++x) {
		for(uint_fast8_t y = 0; y < 4; ++y) {
			uint_fast8_t index = (x * 4) + y;
			if(testFlags(tileBits, 1 << index)) {
				sendTile(mapWriter, floor->locs[index].get(), nullptr);
			}
		}
	}
	mapWriter.endNode();

	std::string stream(
		reinterpret_cast<char*>(mapWriter.getMemory()),
		mapWriter.getSize()
	);
	message.write<std::string>(stream);
}

void LiveSocket::receiveTile(BinaryNode* node, Editor& editor, Action* action, const Position* position)
{
	ASSERT(node != nullptr);

	Tile* tile = readTile(node, editor, position);
	if(tile) {
		action->addChange(newd Change(tile));
	}
}

void LiveSocket::sendTile(MemoryNodeFileWriteHandle& writer, Tile* tile, const Position* position)
{
	writer.addNode(tile->isHouseTile() ? OTBM_HOUSETILE : OTBM_TILE);
	if(position) {
		writer.addU16(position->x);
		writer.addU16(position->y);
		writer.addU8(position->z);
	}

	if(tile->isHouseTile()) {
		writer.addU32(tile->getHouseID());
	}

	if(tile->getMapFlags()) {
		writer.addByte(OTBM_ATTR_TILE_FLAGS);
		writer.addU32(tile->getMapFlags());
	}

	Item* ground = tile->ground;
	if(ground) {
		if(ground->isComplex()) {
			ground->serializeItemNode_OTBM(mapVersion, writer);
		} else {
			writer.addByte(OTBM_ATTR_ITEM);
			ground->serializeItemCompact_OTBM(mapVersion, writer);
		}
	}

	for(Item* item : tile->items) {
		item->serializeItemNode_OTBM(mapVersion, writer);
	}

	writer.endNode();
}

Tile* LiveSocket::readTile(BinaryNode* node, Editor& editor, const Position* position)
{
	ASSERT(node != nullptr);

	Map& map = editor.map;

	uint8_t tileType;
	node->getByte(tileType);

	if(tileType != OTBM_TILE && tileType != OTBM_HOUSETILE) {
		return nullptr;
	}

	Position pos;
	if(position) {
		pos = *position;
	} else {
		uint16_t x; node->getU16(x); pos.x = x;
		uint16_t y; node->getU16(y); pos.y = y;
		uint8_t z; node->getU8(z); pos.z = z;
	}

	Tile* tile = map.allocator(
		map.createTileL(pos)
	);

	if(tileType == OTBM_HOUSETILE) {
		uint32_t houseId;
		if(!node->getU32(houseId)) {
			//warning(wxT("House tile without house data, discarding tile"));
			delete tile;
			return nullptr;
		}

		if(houseId) {
			House* house = map.houses.getHouse(houseId);
			if(house) {
				tile->setHouse(house);
			}
		} else {
			//warning(wxT("Invalid house id from tile %d:%d:%d"), pos.x, pos.y, pos.z);
		}
	}

	uint8_t attribute;
	while(node->getU8(attribute)) {
		switch (attribute) {
			case OTBM_ATTR_TILE_FLAGS: {
				uint32_t flags = 0;
				if(!node->getU32(flags)) {
					//warning(wxT("Invalid tile flags of tile on %d:%d:%d"), pos.x, pos.y, pos.z);
				}
				tile->setMapFlags(flags);
				break;
			}
			case OTBM_ATTR_ITEM: {
				Item* item = Item::Create_OTBM(mapVersion, node);
				if(!item) {
					//warning(wxT("Invalid item at tile %d:%d:%d"), pos.x, pos.y, pos.z);
				}
				tile->addItem(item);
				break;
			}
			default:
				//warning(wxT("Unknown tile attribute at %d:%d:%d"), pos.x, pos.y, pos.z);
				break;
		}
	}

	//for(BinaryNode* itemNode = node->getChild(); itemNode; itemNode->advance()) {
	BinaryNode* itemNode = node->getChild();
	if(itemNode) do {
		uint8_t itemType;
		if(!itemNode->getByte(itemType)) {
			//warning(wxT("Unknown item type %d:%d:%d"), pos.x, pos.y, pos.z);
			delete tile;
			return nullptr;
		}

		if(itemType == OTBM_ITEM) {
			Item* item = Item::Create_OTBM(mapVersion, itemNode);
			if(item) {
				if(!item->unserializeItemNode_OTBM(mapVersion, itemNode)) {
					//warning(wxT("Couldn't unserialize item attributes at %d:%d:%d"), pos.x, pos.y, pos.z);
				}
				tile->addItem(item);
			}
		} else {
			//warning(wxT("Unknown type of tile child node"));
		}
	//}
	} while(itemNode->advance());

	return tile;
}

LiveCursor LiveSocket::readCursor(NetworkMessage& message)
{
	LiveCursor cursor;
	cursor.id = message.read<uint32_t>();

	uint8_t r = message.read<uint8_t>();
	uint8_t g = message.read<uint8_t>();
	uint8_t b = message.read<uint8_t>();
	uint8_t a = message.read<uint8_t>();
	cursor.color = wxColor(r, g, b, a);

	cursor.pos = message.read<Position>();
	return cursor;
}

void LiveSocket::writeCursor(NetworkMessage& message, const LiveCursor& cursor)
{
	message.write<uint32_t>(cursor.id);
	message.write<uint8_t>(cursor.color.Red());
	message.write<uint8_t>(cursor.color.Green());
	message.write<uint8_t>(cursor.color.Blue());
	message.write<uint8_t>(cursor.color.Alpha());
	message.write<Position>(cursor.pos);
}
