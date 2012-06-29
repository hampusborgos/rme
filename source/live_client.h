//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef _RME_LIVE_CLIENT_H_
#define _RME_LIVE_CLIENT_H_

#include "live_socket.h"
#include "net_connection.h"

class DirtyList;
class MapTab;

class LiveClient : public LiveSocket
{
public:
	LiveClient();
	virtual ~LiveClient();

	bool Connect();
	void Close();
	
	LiveLogTab* CreateLogWindow(wxWindow* parent);
	MapTab* CreateEditorWindow();

	void HandleEvent(wxSocketEvent& evt);

	wxString GetHostName() const;

	// Flags a node as queried and stores it, need to call SendNodeRequest to send it to server
	void QueryNode(int ndx, int ndy, bool underground);
	
	virtual void UpdateCursor(Position pos);

	// Send pending stuff
	void SendNodeRequests();
	void SendChanges(DirtyList& dirty_list);
	void SendChat(wxString message);

protected:
	bool IsServer() const {return false;}

	void OnParsePacket(NetworkConnection* connection, NetworkMessage* nmsg);

protected:
	// Packet handling functions
	void OnReceiveHello(NetworkConnection* connection, NetworkMessage* nmsg);
	void OnReceiveFarewell(NetworkConnection* connection, NetworkMessage* nmsg);
	void OnReceiveAccepted(NetworkConnection* connection, NetworkMessage* nmsg);
	void OnReceiveServerTalk(NetworkConnection* connection, NetworkMessage* nmsg);
	void OnReceiveClientVersion(NetworkConnection* connection, NetworkMessage* nmsg);
	void OnReceiveStartOperation(NetworkConnection* connection, NetworkMessage* nmsg);
	void OnReceiveUpdateOperation(NetworkConnection* connection, NetworkMessage* nmsg);
	void OnReceiveCursorUpdate(NetworkConnection* connection, NetworkMessage* nmsg);
	void OnReceiveNewNode(NetworkConnection* connection, NetworkMessage* nmsg);

	NetworkConnection* connection;
	std::set<uint32_t> query_node_list;
	wxSocketClient* client;
	wxString current_operation;
	Editor* editor;
};

#endif
