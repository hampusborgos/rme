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

#if 0

#include "rmenet.h"

const wxEventType EVT_RMENET_CONNECTION_ESTABLISHED = wxNewEventType();
const wxEventType EVT_RMENET_CONNECTION_LOST = wxNewEventType();
const wxEventType EVT_RMENET_SHOW_SERVLIST = wxNewEventType();


RMENet::RMENet(wxEvtHandler* event_dump) :
	event_dump(event_dump)
{
}

RMENet::~RMENet()
{
}

bool RMENet::Connect()
{
	wxIPV4address ipaddr;

	ipaddr.Hostname("127.0.0.1");//90.230.54.138"));
	ipaddr.Service(31312);

	socket = newd wxSocketClient(wxSOCKET_NOWAIT);

	connection = newd NetworkConnection(this, socket);
	socket->SetClientData(connection);

	socket->SetEventHandler(*this, wxID_ANY);
	socket->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_OUTPUT_FLAG | wxSOCKET_LOST_FLAG);
	socket->Notify(true);

	wxEvtHandler::Connect(wxID_ANY, wxEVT_SOCKET, wxSocketEventHandler(RMENet::HandleEvent));

	socket->Connect(ipaddr, false);
	if(!socket || !socket->WaitOnConnect(5, 0) ||
		!socket || !socket->IsConnected())
	{
		if(socket)
			socket->Destroy();
		socket = nullptr;
		delete connection;
		connection = nullptr;
		return false;
	}

	NetworkMessage* nmsg = AllocMessage();
	nmsg->AddByte(0x00); // Hello!
	nmsg->AddU32(__LIVE_NET_VERSION__);
	nmsg->AddString(g_settings.getString(Config::LIVE_USERNAME));
	nmsg->AddString(g_settings.getString(Config::LIVE_PASSWORD));
	connection->Send(nmsg);

	return true;
}

void RMENet::Close()
{
	delete connection;
	connection = nullptr;
	if(socket) {
		socket->Destroy();
		socket = nullptr;
	}

	// Delayed destruction: the socket will be deleted during the next
	// idle loop iteration. This ensures that all pending events have
	// been processed.
	//m_beingDeleted = true;

	// schedule this object for deletion
	wxAppTraits *traits = wxTheApp ? wxTheApp->GetTraits() : nullptr;
	if( traits ) {
		// let the traits object decide what to do with us
		traits->ScheduleForDestroy(this);
	} else { // no app or no traits
		// in wxBase we might have no app object at all, don't leak memory
		delete this;
	}
}

void RMENet::HandleEvent(wxSocketEvent& evt)
{
	NetworkConnection* connection = reinterpret_cast<NetworkConnection*>(evt.GetClientData());
	switch(evt.GetSocketEvent())
	{
		case wxSOCKET_LOST:
		// Connection was lost, either client disconnecting, or our socket breaking
		{
			// Generate event
			wxCommandEvent event(EVT_RMENET_CONNECTION_LOST);
			event.SetInt(0);
			event.SetString("GOOOFU");
			event_dump->AddPendingEvent(event);
		} break;
		case wxSOCKET_OUTPUT:
		// We're ready to write to a socket
		{
			// Send waiting data
			connection->Send();
		} break;
		case wxSOCKET_INPUT:
		// We got some data to be read.
		{
			NetworkMessage* nmsg = connection->Receive();
			if(nmsg)
			{
				try
				{
					OnParsePacket(nmsg);
					FreeMessage(nmsg);
				}
				catch(std::runtime_error&)
				{
					FreeMessage(nmsg);
					Close();
				}
			}
		} break;
	}
}



void RMENet::OnParsePacket(NetworkMessage* nmsg)
{
	switch(nmsg->ReadByte()) {
		case 0x00: // welcome
		{
			OnReceiveWelcome(nmsg);
		} break;
		case 0x01: // Disconnect
		{
			OnReceiveDisconnect(nmsg);
		} break;
		case 0x02: // Ping
		{
			OnReceivePing(nmsg);
		} break;
	}
}


void RMENet::OnReceiveWelcome(NetworkMessage* nmsg)
{
	can_host = nmsg->ReadU8() == 1;

	wxCommandEvent event(EVT_RMENET_CONNECTION_ESTABLISHED);
	event_dump->AddPendingEvent(event);
}

void RMENet::OnReceiveDisconnect(NetworkMessage *nmsg)
{
	wxCommandEvent event(EVT_RMENET_CONNECTION_LOST, wxID_ANY);
	event.SetString(wxstr(nmsg->ReadString()));
	event.SetInt(1);
	event_dump->AddPendingEvent(event);
}

void RMENet::OnReceivePing(NetworkMessage *nmsg)
{
	// Send pong
	NetworkMessage* omsg = AllocMessage();
	omsg->AddByte(0x02);
	connection->Send(omsg);
}

#endif
