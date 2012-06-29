//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "live_client.h"
#include "live_tab.h"
#include "live_action.h"
#include "editor.h"

LiveClient::LiveClient() :
	connection(NULL),
	client(NULL),
	editor(NULL)
{
}

LiveClient::~LiveClient() {

}

wxString LiveClient::GetHostName() const
{
	return ipaddr.Hostname();
}

bool LiveClient::Connect()
{
	client = newd wxSocketClient(wxSOCKET_NOWAIT);

	connection = newd NetworkConnection(this, client);
	client->SetClientData(connection);

	client->SetEventHandler(*this, wxID_ANY);
	client->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_OUTPUT_FLAG | wxSOCKET_LOST_FLAG);
	client->Notify(true);
	
	wxEvtHandler::Connect(wxID_ANY, wxEVT_SOCKET, wxSocketEventHandler(LiveClient::HandleEvent));
	
	client->Connect(ipaddr, false);
	if(!client->WaitOnConnect(5, 0))
	{
		if(log)
			log->Disconnect();
		last_err = wxT("Connection timed out.");
		client->Destroy();
		client = NULL;
		delete connection;
		return false;
	}
	
	if(!client->IsConnected())
	{
		if(log)
			log->Disconnect();
		last_err = wxT("Connection refused by peer.");
		client->Destroy();
		client = NULL;
		delete connection;
		return false;
	}
	
	if(log)
		log->Message(wxT("Connection established!"));

	// Send HELLO
	NetworkMessage* nmsg = AllocMessage();
	nmsg->AddByte(PACKET_HELLO_FROM_CLIENT); // Hello!
	nmsg->AddU32(__RME_VERSION_ID__);
	nmsg->AddU32(__LIVE_NET_VERSION__);
	nmsg->AddU32(gui.GetCurrentVersionID());
	nmsg->AddString("User");
	nmsg->AddString((const char*)password.mb_str(wxConvUTF8));
	connection->Send(nmsg);

	return true;
}

void LiveClient::Close()
{
	if(log)
	{
		log->Message(wxT("Disconnected from server."));
		log->Disconnect();
		log = NULL;
	}
	if(connection)
	{
		connection->Close();
		connection = NULL;
	}
	delete reinterpret_cast<NetworkConnection*>(client->GetClientData());
	LiveSocket::Close();
}

LiveLogTab* LiveClient::CreateLogWindow(wxWindow* parent)
{
	MapTabbook* mtb = dynamic_cast<MapTabbook*>(parent);
	ASSERT(mtb);
	log = newd LiveLogTab(mtb, this);

	log->Message(wxT("New Live mapping session started."));
	wxString m;
	m << wxT("Joined server ") << ipaddr.Hostname() << wxT(":") << ipaddr.Service() << wxT(".");
	log->Message(m);

	return log;
}

MapTab* LiveClient::CreateEditorWindow()
{
	MapTabbook* mtb = dynamic_cast<MapTabbook*>(gui.tabbook);
	ASSERT(mtb);

	MapTab* edit = newd MapTab(mtb, editor);
	edit->OnSwitchEditorMode(gui.IsSelectionMode()? SELECTION_MODE : DRAWING_MODE);

	return edit;
}

void LiveClient::HandleEvent(wxSocketEvent& evt)
{
	NetworkConnection* connection = reinterpret_cast<NetworkConnection*>(evt.GetClientData());
	switch(evt.GetSocketEvent())
	{
		case wxSOCKET_LOST:
		// Connection was lost, either client disconnecting, or our socket breaking
		{
			Log(wxT("Disconnected from server."));
			gui.CloseLiveEditors(this);
			DisconnectLog();
            break;
		}
		case wxSOCKET_OUTPUT:
		// We're ready to write to a socket
		{
			// Send waiting data
			connection->Send();
            break;
		}
		case wxSOCKET_INPUT:
		// We got some data to be read.
		{
			NetworkMessage* nmsg = connection->Receive();
			if(nmsg)
			{
				try {
					OnParsePacket(connection, nmsg);
					FreeMessage(nmsg);
				}
				catch(std::runtime_error&)
				{
					FreeMessage(nmsg);
					Close();
				}
			}
            break;
		}
        default:
            break;
	}
}

///////////////////////////////////////////////////////////////////////////////
// Interface for common requests
///////////////////////////////////////////////////////////////////////////////

void LiveClient::QueryNode(int ndx, int ndy, bool underground)
{
	uint32_t nd = 0;
	ndx >>= 2;
	ndy >>= 2;
	nd |= (ndx << 18);
	nd |= (ndy << 4);
	nd |= (underground? 1 : 0);
	query_node_list.insert(nd);
}

void LiveClient::UpdateCursor(Position pos)
{
	LiveCursor cursor;
	cursor.id = 77; // Unimportant, server fixes it for us
	cursor.pos = pos;
	cursor.color = wxColor(
		settings.getInteger(Config::CURSOR_RED),
		settings.getInteger(Config::CURSOR_GREEN),
		settings.getInteger(Config::CURSOR_BLUE),
		settings.getInteger(Config::CURSOR_ALPHA)
	);

	NetworkMessage* nmsg = AllocMessage();
	nmsg->AddByte(PACKET_CLIENT_UPDATE_CURSOR);
	AddCursor(nmsg, cursor);
	connection->Send(nmsg);
}

///////////////////////////////////////////////////////////////////////////////
// Parse incoming packets
///////////////////////////////////////////////////////////////////////////////

void LiveClient::OnParsePacket(NetworkConnection* connection, NetworkMessage* nmsg)
{
	switch(nmsg->ReadByte()) {
		case PACKET_HELLO_FROM_SERVER: // HELLO TO YOU TOO
		{
			OnReceiveHello(connection, nmsg);
			break;
		}
		case PACKET_KICK: // FAREWELL NOOB
		{
			OnReceiveFarewell(connection, nmsg);
			break;
		}
		case PACKET_ACCEPTED_CLIENT: // ACCEPTED
			{
				OnReceiveAccepted(connection, nmsg);
				break;
			}
		case PACKET_CHANGE_CLIENT_VERSION: // CLIENT VERSION CHANGE
		{
			OnReceiveClientVersion(connection, nmsg);
			break;
		}
		case PACKET_SERVER_TALK: // SERVER SAYS SOMETHING!
			{
				OnReceiveServerTalk(connection, nmsg);
				break;
			}
		case PACKET_NODE: // NEW NODE
		{
			OnReceiveNewNode(connection, nmsg);
			break;
		}
		case PACKET_CURSOR_UPDATE: // POINTER UPDATE
		{
			OnReceiveCursorUpdate(connection, nmsg);
			break;
		}
		case PACKET_START_OPERATION: // START OPERATION
		{
			OnReceiveStartOperation(connection, nmsg);
			break;
		}
		case PACKET_UPDATE_OPERATION: // OPERATION UPDATE
		{
			OnReceiveUpdateOperation(connection, nmsg);
			break;
		}
		default: {
			log->Message(wxT("Unknown packet receieved!"));
			//Close();
			break;
		}
	}
}

void LiveClient::OnReceiveAccepted(NetworkConnection* connection, NetworkMessage* nmsg)
{
	// We have been accepted with current client version, reply that we're ready for a 'Hello'
	NetworkMessage* omsg = AllocMessage();
	omsg->AddByte(PACKET_READY_CLIENT);
	connection->Send(omsg);
}

void LiveClient::OnReceiveHello(NetworkConnection* connection, NetworkMessage* nmsg)
{
	std::string hostname = nmsg->ReadString();
	uint16_t width = nmsg->ReadU16();
	uint16_t height = nmsg->ReadU16();

	ASSERT(editor == NULL);
	editor = newd Editor(gui.copybuffer, this);
	editor->map.setWidth(width);
	editor->map.setHeight(height);
	editor->map.setName("Live Map - " + hostname);
	CreateEditorWindow();
}

void LiveClient::OnReceiveFarewell(NetworkConnection* connection, NetworkMessage* nmsg)
{
	std::string message = nmsg->ReadString();
	gui.PopupDialog(wxT("Disconnected"), wxstr(message), wxOK);
}

void LiveClient::OnReceiveClientVersion(NetworkConnection* connection, NetworkMessage* nmsg)
{
	// Server wants us to switch server version before it sends 'Hello'
	int ver = nmsg->ReadU32();

	if (!gui.CloseAllEditors())
	{
		Disconnect();
		return;
	}

	wxString error;
	wxArrayString warnings;
	gui.LoadVersion((ClientVersionID)ver, error, warnings);

	// Now tell the server we are ready for the world
	NetworkMessage* omsg = AllocMessage();
	omsg->AddByte(PACKET_READY_CLIENT);
	connection->Send(omsg);
}

void LiveClient::OnReceiveCursorUpdate(NetworkConnection* connection, NetworkMessage* nmsg)
{
	LiveCursor cursor = ReadCursor(nmsg);
	cursors[cursor.id] = cursor;

	gui.RefreshView();
}

void LiveClient::OnReceiveServerTalk(NetworkConnection* connection, NetworkMessage* nmsg)
{
	std::string speaker = nmsg->ReadString();
	std::string msg = nmsg->ReadString();
	
	log->Chat(wxstr(speaker), wxstr(msg));
}

void LiveClient::OnReceiveStartOperation(NetworkConnection* connection, NetworkMessage* nmsg)
{
	std::string operation = nmsg->ReadString();
	current_operation = wxstr(operation);

	gui.SetStatusText(wxString(wxT("Server Operation in Progress: ")) + current_operation + wxT("... (0%)"));
}

void LiveClient::OnReceiveUpdateOperation(NetworkConnection* connection, NetworkMessage* nmsg)
{
	int percent = nmsg->ReadU32();

	if (percent >= 100)
		gui.SetStatusText(wxT("Server Operation Finished."));
	else
		gui.SetStatusText(wxString(wxT("Server Operation in Progress: ")) + current_operation + wxT("... (") + wxstr(i2s(percent)) + wxT("%)"));
}

void LiveClient::OnReceiveNewNode(NetworkConnection* connection, NetworkMessage* nmsg)
{
	// Read position
	uint32_t ind = nmsg->ReadU32();

	// Extract node position
	int ndx = ind >> 18;
	int ndy = (ind >> 4) & 0x3FFF;
	bool underground = ind & 1;

	Action* action = editor->actionQueue->createAction(ACTION_REMOTE);
	ReceiveNode(nmsg, *editor, action, ndx, ndy, underground);
	editor->actionQueue->addAction(action);

	gui.RefreshView();
}


///////////////////////////////////////////////////////////////////////////////
// Send functions
///////////////////////////////////////////////////////////////////////////////

void LiveClient::SendNodeRequests()
{
	if(query_node_list.size())
	{
		NetworkMessage* nmsg = AllocMessage();
		nmsg->AddByte(PACKET_REQUEST_NODES); // REQUEST NODES
		nmsg->AddU32(query_node_list.size());
		for(std::set<uint32_t>::const_iterator iter = query_node_list.begin(); iter != query_node_list.end(); ++iter)
			nmsg->AddU32(*iter);

		connection->Send(nmsg);

		query_node_list.clear();
	}
}

void LiveClient::SendChanges(DirtyList& dirty_list)
{
	ChangeList& cl = dirty_list.GetChanges();
	if(cl.empty())
		return;
	
	bn_writer.reset();

	ChangeList::iterator iter = dirty_list.GetChanges().begin();
	ChangeList::iterator end = dirty_list.GetChanges().end();
	for( ; iter != end; ++iter)
	{
		Change* c = *iter;
		// That this logic is here is kinda ugly
		
		switch(c->getType())
		{
			case CHANGE_TILE:
			{
				Tile* tile = reinterpret_cast<Tile*>(c->getData());
				Position pos = tile->getPosition();
				Tile* real_tile = editor->map.getTile(pos);
				AddTile(bn_writer, real_tile, &pos);
			} break;
			case CHANGE_MOVE_HOUSE_EXIT:
			{
			} break;
			default:
				break;
		}
	}
	bn_writer.endNode();

	NetworkMessage* omsg = AllocMessage();
	omsg->AddByte(PACKET_CHANGE_LIST);
	std::string s(
		(const char*)bn_writer.getMemory(), 
		bn_writer.getSize()
	);
	omsg->AddString(s);
	connection->Send(omsg);
}

void LiveClient::SendChat(wxString message)
{
	NetworkMessage* omsg = AllocMessage();
	omsg->AddByte(PACKET_CLIENT_TALK);
	omsg->AddString(nstr(message));
}