//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include <wx/apptrait.h>
#include "live_socket.h"
#include "live_peer.h"
#include "map_region.h"
#include "iomap_otbm.h"
#include "live_tab.h"
#include "editor.h"

LiveSocket::LiveSocket() : bn_reader(NULL, 0), map_version(MapVersion(MAP_OTBM_4, CLIENT_VERSION_NONE)), log(NULL) {
}

LiveSocket::~LiveSocket() {
	while(message_pool.size() > 0) {
		delete message_pool.back();
		message_pool.pop_back();
	}
}

wxString LiveSocket::GetLastError() const {
	return last_err;
}

void LiveSocket::Log(wxString message) {
	if(log)
		log->Message(message);
}

void LiveSocket::DisconnectLog() {
	if(log)
		log->Disconnect();
}

void LiveSocket::Close() {	
	// Delayed destruction: the socket will be deleted during the next
	// idle loop iteration. This ensures that all pending events have
	// been processed.

	// schedule this object for deletion
	if(wxTheApp)
	{
		// let the traits object decide what to do with us
		//wxTheApp->ScheduleForDestruction(this);
		// ScheduleForDestruction(this);
	}
	else // no app or no traits
	{
		// in wxBase we might have no app object at all, don't leak memory
		delete this;
	}
}

bool LiveSocket::SetPassword(const wxString& npassword) {
	if(npassword.length() > 32) {
		last_err = wxT("Port is not a number.");
		return false;
	}
	password = npassword;
	return true;
}

bool LiveSocket::SetPort(long nport) {
	if(nport < 1 || nport > 65535) {
		last_err = wxT("Port must be a number in the range 1-65535.");
		return false;
	}
	ipaddr.Service((uint16_t)nport);
	return true;
}

bool LiveSocket::SetIP(const wxString& nip) {
	if(!ipaddr.Hostname(nip)) {
		last_err = wxT("Not a valid IP Hostname.");
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////
// Interface for common requests
///////////////////////////////////////////////////////////////////////////////

std::vector<LiveCursor> LiveSocket::GetCursorList() const
{
	std::vector<LiveCursor> l;
	for(std::map<uint32_t, LiveCursor>::const_iterator cursor = cursors.begin(); cursor != cursors.end(); ++cursor)
		l.push_back(cursor->second);
	return l;
}


///////////////////////////////////////////////////////////////////////////////
// Network code after this point
///////////////////////////////////////////////////////////////////////////////

void LiveSocket::SendNode(LivePeer* connection, QTreeNode* nd, int ndx, int ndy, uint32_t floormask)
{
	// Update the node list with their knowledge of this node
	bool underground = false;
	if(floormask & 0xff00) // Underground floors included
		underground = true;
	if(floormask & 0x00ff) // Overground floors included
		underground = false;
	
	nd->setVisible(connection->GetClientID(), underground, true);

	// Message...
	NetworkMessage* omsg = AllocMessage();
	omsg->AddByte(PACKET_NODE); // NEW NODE

	// We send it back with the reply.
	omsg->AddU32((ndx << 18) | (ndy << 4) | (floormask & 0xff00? 1 : 0));

	if(!nd) {
		omsg->AddByte(0);
	} else {
		// Compensate for underground
		// First byte tells what floors are included
		Floor** floor_p = nd->getFloors();
		uint16_t floors = 0;
		for(int z = 0; z <= 15; ++z) {
			if(floor_p[z] != NULL && (floormask & (1 << z)))
				floors |= 1 << z;
		}
		omsg->AddU16(floors);

		// Now we tell, for each floor, which tiles are included, followed by the tiles themselves.
		for(int z = 0; z <= 15; ++z) {
			if(floors & (1 << z))
				AddFloor(omsg, floor_p[z]);
		}
	}
	connection->Send(omsg);
}

void LiveSocket::AddFloor(NetworkMessage* omsg, Floor* floor)
{
	uint16_t tiles = 0;
	for(int x = 0; x < 4; ++x) {
		for(int y = 0; y < 4; ++y) {
			Tile* tile = floor->locs[x*4 + y].get();
			if(tile && tile->size() > 0) {
				tiles |= (1 << (x*4+y));
			}
		}
	}
	omsg->AddU16(tiles);
	if(tiles == 0x0000)
		return;

	bn_writer.reset();
	// Skip beginning node start
	//bn_writer.addNode(0); // root node
	for(int x = 0; x < 4; ++x) {
		for(int y = 0; y < 4; ++y) {
			if((tiles & (1 << (x*4+y))) == false) {
				continue;
			}
			Tile* tile = floor->locs[x*4 + y].get();
			AddTile(bn_writer, tile, NULL);
		}
	}
	bn_writer.endNode(); // end root

	std::string s(
		(const char*)bn_writer.getMemory(), 
		bn_writer.getSize());
	omsg->AddString(s);
}

void LiveSocket::AddTile(MemoryNodeFileWriteHandle& writer, Tile* tile, const Position* pos)
{
	writer.addNode(tile->isHouseTile()? OTBM_HOUSETILE : OTBM_TILE);

	if(pos) {
		writer.addU16(pos->x);
		writer.addU16(pos->y);
		writer.addU8 (pos->z);
	}

	if(tile->isHouseTile())
		writer.addU32(tile->getHouseID());
	
	if(tile->getMapFlags()) {
		writer.addByte(OTBM_ATTR_TILE_FLAGS);
		writer.addU32(tile->getMapFlags());
	}

	if(tile->ground) {
		Item* ground = tile->ground;
		if(ground->isComplex()) {
			ground->serializeItemNode_OTBM(map_version, writer);
		} else {
			bn_writer.addByte(OTBM_ATTR_ITEM);
			ground->serializeItemCompact_OTBM(map_version, writer);
		}
	}

	for(ItemVector::iterator it = tile->items.begin(); it != tile->items.end(); ++it) {
		(*it)->serializeItemNode_OTBM(map_version, writer);
	}
	writer.endNode();
}


void LiveSocket::ReceiveNode(NetworkMessage* nmsg, Editor& editor, Action* action, int ndx, int ndy, bool underground) {
	Map& map = editor.map;

	//
	QTreeNode* nd = map.getLeaf(ndx*4, ndy*4);
	
	if(!nd) {
		wxString str;
		str << wxT("Warning: Received update for unknown tile (") << ndx*4 << wxT("/") << ndy*4 << wxT("/") << underground << wxT(")");
		log->Message(str);
		return;
	}

	// Update node state
	nd->setRequested(underground, false);
	nd->setVisible(underground, true);

	// Figure out what floors are relevant
	uint16_t floors = nmsg->ReadU16();

	if(floors == 0x0000) {
		return;
	}

	for(int z = 0; z <= 15; ++z)
		if(floors & (1 << (z)))
			ReceiveFloor(nmsg, editor, action, ndx, ndy, z, nd, nd->getFloor(z));
}

void LiveSocket::ReceiveFloor(NetworkMessage* nmsg, Editor& editor, Action* action, int ndx, int ndy, int z, QTreeNode* nd, Floor* floor) {
	Map& map = editor.map;

	//
	uint16_t tiles = nmsg->ReadU16();
	if(tiles == 0x0000) {
		// No tiles on this floor
		for(int x = 0; x < 4; ++x) {
			for(int y = 0; y < 4; ++y) {
				action->addChange(new Change(map.allocator(nd->createTile(ndx * 4 + x, ndy * 4 + y, z))));
			}
		}
		return;
	}
	
	std::string data = nmsg->ReadString();
	// -1 on address since we skip the first START_NODE when sending
	bn_reader.assign((uint8_t*)data.c_str() - 1, data.size());
	BinaryNode* rootNode = bn_reader.getRootNode();
	BinaryNode* tileNode = rootNode->getChild();

	for(int x = 0; x < 4; ++x) {
		for(int y = 0; y < 4; ++y) {
			if((tiles & (1 << (x*4+y))) == false) {
				action->addChange(new Change(map.allocator(nd->createTile(ndx * 4 + x, ndy * 4 + y, z))));
				continue;
			}
			Position pos(ndx*4 + x, ndy*4 + y, z);
			ReceiveTile(tileNode, editor, action, &pos);
			tileNode->advance();
		}
	}
	bn_reader.close();
}

void LiveSocket::ReceiveTile(BinaryNode* tileNode, Editor& editor, Action* action, const Position* pos)
{
	Map& map = editor.map;
	// We are receiving a tile! :)
	ASSERT(tileNode);
	Tile* tile = ReadTile(tileNode, map, pos);

	if(tile)
		action->addChange(newd Change(tile));
}


Tile* LiveSocket::ReadTile(BinaryNode* tileNode, Map& map, const Position* mpos)
{
	// We are receiving a tile! :)
	ASSERT(tileNode);
	Tile* tile = NULL;
	uint8_t tile_type;
	tileNode->getByte(tile_type);

	if(tile_type == OTBM_TILE || tile_type == OTBM_HOUSETILE)
	{
		Position pos;
		if(mpos)
		{
			pos = *mpos;
		}
		else
		{
			uint16_t x; tileNode->getU16(x); pos.x = x;
			uint16_t y; tileNode->getU16(y); pos.y = y;
			uint8_t  z; tileNode->getU8 (z); pos.z = z;
		}

		TileLocation* loc = map.createTileL(pos);
		tile = map.allocator(loc);

		if(tile_type == OTBM_HOUSETILE)
		{
			uint32_t house_id;
			if(!tileNode->getU32(house_id))
			{
				//warning(wxT("House tile without house data, discarding tile"));
				delete tile;
				return NULL;
			}
			if(house_id)
			{
				House* house = map.houses.getHouse(house_id);
				if(house)
					tile->setHouse(house);
			}
			else
			{
				//warning(wxT("Invalid house id from tile %d:%d:%d"), pos.x, pos.y, pos.z);
			}
		}

		uint8_t attribute;
		while(tileNode->getU8(attribute))
		{
			switch(attribute)
			{
				case OTBM_ATTR_TILE_FLAGS:
				{
					uint32_t flags = 0;
					if(!tileNode->getU32(flags))
					{
						//warning(wxT("Invalid tile flags of tile on %d:%d:%d"), pos.x, pos.y, pos.z);
					}
					tile->setMapFlags(flags);
					break;
				}
				case OTBM_ATTR_ITEM:
				{
					Item* item = Item::Create_OTBM(map_version, tileNode);
					if(item == NULL)
					{
						//warning(wxT("Invalid item at tile %d:%d:%d"), pos.x, pos.y, pos.z);
					}
					tile->addItem(item);
					break;
				}
				default:
					{
					//warning(wxT("Unknown tile attribute at %d:%d:%d"), pos.x, pos.y, pos.z);
				}
			}
		}

		
		BinaryNode* itemNode = tileNode->getChild();
		if(itemNode) do
		{
			Item* item = NULL;
			uint8_t item_type;
			if(!itemNode->getByte(item_type))
			{
				//warning(wxT("Unknown item type %d:%d:%d"), pos.x, pos.y, pos.z);
				delete tile;
				return NULL;
			}
			if(item_type == OTBM_ITEM)
			{
				item = Item::Create_OTBM(map_version, itemNode);
				if(item)
				{
					if(item->unserializeItemNode_OTBM(map_version, itemNode) == false)
					{
						//warning(wxT("Couldn't unserialize item attributes at %d:%d:%d"), pos.x, pos.y, pos.z);
					}
					tile->addItem(item);
				}
			}
			else
			{
				//warning(wxT("Unknown type of tile child node"));
			}
		} while(itemNode->advance());

		return tile;
	}
	else
	{
		return NULL;
	}
}

void LiveSocket::AddCursor(NetworkMessage* nmsg, const LiveCursor& cursor)
{
	nmsg->AddU32(cursor.id);
	nmsg->AddU8(cursor.color.Red());
	nmsg->AddU8(cursor.color.Green());
	nmsg->AddU8(cursor.color.Blue());
	nmsg->AddU8(cursor.color.Alpha());
	nmsg->AddPosition(cursor.pos);
}

LiveCursor LiveSocket::ReadCursor(NetworkMessage* nmsg)
{
	LiveCursor c;
	c.id = nmsg->ReadU32();
	uint8_t r, g, b, a;
	r = nmsg->ReadByte();
	g = nmsg->ReadByte();
	b = nmsg->ReadByte();
	a = nmsg->ReadByte();
	c.color.Set(r, g, b, a);
	c.pos = nmsg->ReadPosition();

	return c;
}
