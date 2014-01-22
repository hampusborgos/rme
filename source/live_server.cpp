//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "live_server.h"
#include "live_tab.h"
#include "live_action.h"
#include "live_peer.h"
#include "editor.h"

LiveServer::LiveServer(Editor& editor) : 
	editor(&editor),
	serv(nullptr),
	client_mask(0)
{
	log = nullptr;
}

LiveServer::~LiveServer()
{
}

wxString LiveServer::GetHostName() const
{
	return ipaddr.Hostname() << wxT(":") << ipaddr.Service();
}

bool LiveServer::SetName(const wxString& nname)
{
	if(nname.length() < 1) {
		last_err = wxT("Must provide a name.");
		return false;
	}
	if(nname.length() > 32) {
		last_err = wxT("Name is too long.");
		return false;
	}
	name = nname;
	return true;
}

bool LiveServer::SetVerification(const wxString& nname)
{
	return true;
}


bool LiveServer::Bind()
{
	ipaddr.AnyAddress();
	serv = newd wxSocketServer(ipaddr, wxSOCKET_NOWAIT);
	bool ok = serv->IsOk();
	if(ok)
	{
		serv->SetEventHandler(*this, wxID_ANY);
		serv->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_OUTPUT_FLAG | wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG);
		serv->Notify(true);
		wxEvtHandler::Connect(wxID_ANY, wxEVT_SOCKET, wxSocketEventHandler(LiveServer::HandleEvent));
	}
	return ok;
}

void LiveServer::Close()
{
	// Notify clients!
	for(PeerList::iterator citer = connected_clients.begin(); citer != connected_clients.end(); ++citer)
	{
		(*citer)->Close();
		delete *citer;
	}
	connected_clients.clear();
	if(log) {
		log->Message(wxT("Server was shutdown."));
		log->Disconnect();
		log = nullptr;
	}
	if(serv)
	{
		serv->Destroy();
		serv = nullptr;
	}

	LiveSocket::Close();
}

LiveLogTab* LiveServer::CreateLogWindow(wxWindow* parent)
{
	MapTabbook* mtb = dynamic_cast<MapTabbook*>(parent);
	ASSERT(mtb);
	log = newd LiveLogTab(mtb, this);

	log->Message(wxT("New Live mapping session started."));
	wxString m;
	m << wxT("Hosted on server ") << ipaddr.Hostname() << wxT(":") << ipaddr.Service() << wxT(".");
	log->Message(m);
	log->UpdateClientList(connected_clients);

	return log;
}

void LiveServer::HandleEvent(wxSocketEvent& evt)
{
	LivePeer* connection = reinterpret_cast<LivePeer*>(evt.GetClientData());
	try {
		switch(evt.GetSocketEvent()) {
			case wxSOCKET_CONNECTION:
			// A client tries to connect!
			{
				wxSocketBase* client = serv->Accept(false);
				if(!client) {
					log->Message(wxT("Network error: Could not accept incoming connection."));
				}
				LivePeer* connection = newd LivePeer(this, client);
				connection->parser = &LiveServer::OnParseLoginPackets;
				connecting_clients.push_back(connection);
				client->SetClientData(connection);

				client->SetEventHandler(*this, wxID_ANY);
				client->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_OUTPUT_FLAG | wxSOCKET_CONNECTION_FLAG | wxSOCKET_LOST_FLAG);
				client->Notify(true);
			} break;
			case wxSOCKET_LOST:
			// Connection was lost, either client disconnecting, or our socket breaking
			{
				if(connection->GetClientID() != 0)
				{
					client_mask &= ~(1 << connection->GetClientID());
					editor->map.clearVisible(client_mask);
				}

				bool client = false;
				PeerList::iterator iter = connected_clients.begin();
				while(iter != connected_clients.end())
				{
					if(*iter == connection)
					{
						client = true;
						connected_clients.erase(iter);
						break;
					}
					else
						++iter;
				}

				if(client)
				{
					// It was a client disconnecting.
					wxString str;
					str << wxT("Lost remote connection ") << connection->GetNick() << wxT(" (") << connection->GetHost() << wxT(")");
					log->Message(str);
					log->UpdateClientList(connected_clients);
					delete connection;
				}
				else
				{
					// Our socket broke!
					log->Message(wxT("Network connection was interrupted. Can no longer continue to host."));
					delete connection;
					Close();
				}
			} break;
			case wxSOCKET_OUTPUT:
			// We're ready to write to a socket
			{
				//log->Message(wxT("Socket is writeable!"));
				if(connection)
					connection->Send();
			} break;
			case wxSOCKET_INPUT:
			// We got some data to be read.
			{
				NetworkMessage* nmsg = connection->Receive();
				if(nmsg)
				{
					((this)->*(connection->parser))(connection, nmsg);
					FreeMessage(nmsg);
				}
			} break;
		}
	}
	catch(std::runtime_error& err)
	{
		connection->Close();
		log->Message(wxT("Invalid packet received, the reported error was: ") + wxString(err.what(), wxConvUTF8));
	}
}

///////////////////////////////////////////////////////////////////////////////
// Common interface
///////////////////////////////////////////////////////////////////////////////

void LiveServer::UpdateCursor(Position pos)
{
	LiveCursor cursor;
	cursor.id = 0;
	cursor.pos = pos;
	cursor.color = wxColor(
		settings.getInteger(Config::CURSOR_RED),
		settings.getInteger(Config::CURSOR_GREEN),
		settings.getInteger(Config::CURSOR_BLUE),
		settings.getInteger(Config::CURSOR_ALPHA)
	);
	BroadcastCursor(cursor);
}

///////////////////////////////////////////////////////////////////////////////
// Broadcast interface
///////////////////////////////////////////////////////////////////////////////

void LiveServer::BroadcastNodes(DirtyList& dirty_list)
{
	if(dirty_list.Empty())
		return;

	DirtyList::SetType ds = dirty_list.GetPosList();
	for(DirtyList::SetType::const_iterator iter = ds.begin(); iter != ds.end(); ++iter)
	{
		const DirtyList::ValueType& ind = *iter;

		int ndx = ind.pos >> 18;
		int ndy = (ind.pos >> 4) & 0x3FFF;
		uint32_t floors = ind.floors;

		QTreeNode* node = editor->map.getLeaf(ndx*4, ndy*4);
		if(!node)
			continue;

		for(PeerList::iterator citer = connected_clients.begin(); citer != connected_clients.end(); ++citer)
		{
			if(dirty_list.owner != 0 && dirty_list.owner == (*citer)->GetClientID())
			{
				continue;
			}

			bool seen_ug = node->isVisible((*citer)->GetClientID(), true);
			bool seen_og = node->isVisible((*citer)->GetClientID(), false);
			if(seen_ug)
			{
				SendNode(*citer, node, ndx, ndy, floors & 0xff00);
			}
			if(seen_og)
			{
				SendNode(*citer, node, ndx, ndy, floors & 0x00ff);
			}
		}
	}
}

void LiveServer::BroadcastCursor(const LiveCursor& cursor)
{
	for(PeerList::iterator citer = connected_clients.begin(); citer != connected_clients.end(); ++citer)
	{
		if((*citer)->GetClientID() != cursor.id)
		{
			NetworkMessage* msg = AllocMessage();
			msg->AddByte(PACKET_CURSOR_UPDATE);
			AddCursor(msg, cursor);
			(*citer)->Send(msg);
		}
	}
}

void LiveServer::BroadcastChat(wxString speaker, wxString message)
{
	for(PeerList::iterator citer = connected_clients.begin(); citer != connected_clients.end(); ++citer)
	{
		NetworkMessage* nmsg = AllocMessage();
		nmsg->AddByte(PACKET_SERVER_TALK);
		nmsg->AddString(nstr(speaker));
		nmsg->AddString(nstr(message));
		(*citer)->Send(nmsg);
	}
	log->Chat(name, message);
}

void LiveServer::StartOperation(wxString msg)
{
	for(PeerList::iterator citer = connected_clients.begin(); citer != connected_clients.end(); ++citer)
	{
		NetworkMessage* nmsg = AllocMessage();
		nmsg->AddByte(PACKET_START_OPERATION);
		nmsg->AddString(nstr(msg));
		(*citer)->Send(nmsg);
	}
}

void LiveServer::UpdateOperation(int percent)
{
	for(PeerList::iterator citer = connected_clients.begin(); citer != connected_clients.end(); ++citer)
	{
		NetworkMessage* msg = AllocMessage();
		msg->AddByte(PACKET_UPDATE_OPERATION);
		msg->AddU32(percent);
		(*citer)->Send(msg);
	}
}

///////////////////////////////////////////////////////////////////////////////
// Parse incoming packets
///////////////////////////////////////////////////////////////////////////////

void LiveServer::OnParseLoginPackets(LivePeer* connection, NetworkMessage* nmsg)
{
	switch(nmsg->ReadByte())
	{
		case PACKET_HELLO_FROM_CLIENT: // HELLO
		{
			OnReceiveHello(connection, nmsg);
			break;
		}
		case PACKET_READY_CLIENT: // READY FOR WORLD
		{
			OnReceiveReady(connection, nmsg);
			break;
		}
		default:
		{
			log->Message(wxT("Invalid login packet receieved, connection severed."));
			connection->Close();
			break;
		}
	}
}

void LiveServer::OnParseEditorPackets(LivePeer* connection, NetworkMessage* nmsg)
{
	switch(nmsg->ReadByte())
	{
		case PACKET_REQUEST_NODES: // REQUEST NODES
		{
			OnReceiveNodeRequest(connection, nmsg);
			break;
		}
		case PACKET_CHANGE_LIST: // SEND CHANGES
		{
			OnReceiveChanges(connection, nmsg);
			break;
		}
		case PACKET_ADD_HOUSE: // ADD NEW HOUSE
		{
			OnReceiveAddHouse(connection, nmsg);
			break;
		}
		case PACKET_EDIT_HOUSE: // EDIT HOUSE
		{
			OnReceiveEditHouse(connection, nmsg);
			break;
		}
		case PACKET_REMOVE_HOUSE: // REMOVE HOUSE
		{
			OnReceiveRemoveHouse(connection, nmsg);
			break;
		}
		case PACKET_CLIENT_UPDATE_CURSOR: // CURSOR UPDATE
		{
			OnReceiveCursorUpdate(connection, nmsg);
			break;
		}
		case PACKET_CLIENT_TALK: // CLIENT TALKS
		{
			OnReceiveChatMessage(connection, nmsg);
			break;
		}
		default:
		{
			log->Message(wxT("Invalid editor packet receieved, connection severed."));
			connection->Close();
			break;
		}
	}
}

void LiveServer::OnReceiveHello(LivePeer* connection, NetworkMessage* nmsg)
{
	uint32_t rvid = nmsg->ReadU32();
	if(rvid != __RME_VERSION_ID__)
	{
		NetworkMessage* omsg = AllocMessage();
		omsg->AddByte(PACKET_KICK); // FAREWELL
		omsg->AddString("Wrong editor version.");
		connection->Send(omsg);

		connection->Close();
		return;
	}

	uint32_t net_ver = nmsg->ReadU32();
	if(net_ver != __LIVE_NET_VERSION__)
	{
		NetworkMessage* omsg = AllocMessage();
		omsg->AddByte(PACKET_KICK); // FAREWELL
		omsg->AddString("Wrong protocol version.");
		connection->Send(omsg);

		connection->Close();
		return;
	}

	uint32_t client_version = nmsg->ReadU32();
	std::string rnick = nmsg->ReadString();
	std::string pass = nmsg->ReadString();
	wxString upass = wxString(pass.c_str(), wxConvUTF8);

	if(password != upass)
	{
		log->Message(wxT("Client tried to connect, but used the wrong password, connection refused."));
		connection->Close();
		return;
	}
	connection->SetNick(wxString(rnick.c_str(), wxConvUTF8));
	log->Message(connection->GetNick() << wxT(" (") << connection->GetHost() << wxT(") connected."));

	if((ClientVersionID)client_version != gui.GetCurrentVersionID())
	{
		NetworkMessage* omsg = AllocMessage();
		omsg->AddByte(PACKET_CHANGE_CLIENT_VERSION); // CHANGE CLIENT VER
		omsg->AddU32(gui.GetCurrentVersionID());
		connection->Send(omsg);
	}
	else
	{
		// Reply that we have accepted the remote connection
		NetworkMessage* omsg = AllocMessage();
		omsg->AddByte(PACKET_ACCEPTED_CLIENT); // CONNECTION ACCEPTED WITHOUT ISSUE
		connection->Send(omsg);
	}
}
	

void LiveServer::OnReceiveReady(LivePeer* connection, NetworkMessage* nmsg)
{
	// Client has changed server version to the proper one
	PeerList::iterator piter = std::find(connecting_clients.begin(), connecting_clients.end(), connection);
	if (piter == connecting_clients.end())
	{
		connection->Close();
		return;
	}
	connecting_clients.erase(piter);

	// Find free client id
	bool f = false;
	for(size_t s = 1; s < 16; ++s)
	{
		if((1 << s) & ~client_mask)
		{
			connection->SetClientID(s);
			client_mask |= (1 << s);
			f = true;
			break;
		}
	}

	if(!f)
	{
		// Not enough room, disconnect!
		NetworkMessage* omsg = AllocMessage();
		omsg->AddByte(0x81); // FAREWELL 
		omsg->AddString("Server is full.");
		connection->Send(omsg);

		connection->Close();
		return;
	}
	connected_clients.push_back(connection);

	log->UpdateClientList(connected_clients);

	// Let's reply
	NetworkMessage* omsg = AllocMessage();
	omsg->AddByte(0x80); // HELLO TO YOU TOO!
	omsg->AddString(editor->map.getName());
	omsg->AddU16(editor->map.getWidth());
	omsg->AddU16(editor->map.getHeight());

	connection->Send(omsg);

	// Change parser
	connection->parser = &LiveServer::OnParseEditorPackets;
}

void LiveServer::OnReceiveCursorUpdate(LivePeer* connection, NetworkMessage* nmsg)
{
	LiveCursor cursor = ReadCursor(nmsg);

	cursor.id = connection->GetClientID();
	connection->SetUsedColor(cursor.color);
	cursors[cursor.id] = cursor;
	BroadcastCursor(cursor);
	gui.RefreshView();
}

void LiveServer::OnReceiveNodeRequest(LivePeer* connection, NetworkMessage* nmsg)
{
	uint32_t nodes = nmsg->ReadU32();
	while(nodes--)
	{
		// Read position
		uint32_t ind = nmsg->ReadU32();			

		// Extract node position
		int ndx = ind >> 18;
		int ndy = (ind >> 4) & 0x3FFF;
		bool underground = ind & 1;

		QTreeNode* node = editor->map.createLeaf(ndx * 4, ndy * 4);
		if (node) {
			SendNode(connection, node, ndx, ndy, underground? 0xff00 : 0x00ff);
		}
	}
}

void LiveServer::OnReceiveChanges(LivePeer* connection, NetworkMessage* nmsg)
{
	std::string data = nmsg->ReadString();
	// -1 on address since we skip the first START_NODE when sending
	bn_reader.assign((uint8_t*)data.c_str() - 1, data.size());
	BinaryNode* rootNode = bn_reader.getRootNode();
	BinaryNode* tileNode = rootNode->getChild();

	NetworkedAction* action = dynamic_cast<NetworkedAction*>(editor->actionQueue->createAction(ACTION_REMOTE));
	action->owner = connection->GetClientID();

	if (tileNode) do
	{
		Tile* t = ReadTile(tileNode, editor->map);
		if(!t) continue;
		action->addChange(newd Change(t));
	} while (tileNode->advance());
	
	bn_reader.close();

	editor->actionQueue->addAction(action);

	gui.RefreshView();
}

void LiveServer::OnReceiveAddHouse(LivePeer* connection, NetworkMessage* nmsg)
{
}

void LiveServer::OnReceiveEditHouse(LivePeer* connection, NetworkMessage* nmsg)
{
}

void LiveServer::OnReceiveRemoveHouse(LivePeer* connection, NetworkMessage* nmsg)
{
}

void LiveServer::OnReceiveChatMessage(LivePeer* connection, NetworkMessage* nmsg)
{
	wxString message = wxstr(nmsg->ReadString());
	BroadcastChat(connection->GetNick(), message);
}

