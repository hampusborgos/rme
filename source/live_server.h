//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////


#ifndef _RME_LIVE_SERVER_H_
#define _RME_LIVE_SERVER_H_

#include "live_socket.h"
#include "net_connection.h"
#include "action.h"

class LiveLogTab;
class QTreeNode;

class LiveServer : public LiveSocket {
public:
	LiveServer(Editor& editor);
	virtual ~LiveServer();

	bool SetName(const wxString& name);
	bool SetVerification(const wxString& key);

	void Close();
	bool Bind();

	LiveLogTab* CreateLogWindow(wxWindow* parent);

	void HandleEvent(wxSocketEvent& evt);
	
	wxString GetHostName() const;

public:
	// Broadcast interface
	void BroadcastNodes(DirtyList& dirty_list);
	void BroadcastChat(wxString speaker, wxString message);
	void BroadcastCursor(const LiveCursor& cursor);

	// Operation displays load bar for all clients
	void StartOperation(wxString msg);
	void UpdateOperation(int percent);

	// Update interface
	virtual void UpdateCursor(Position pos);

	// Parsing function, set in LivePeer::parser
	void OnParseEditorPackets(LivePeer* connection, NetworkMessage* nmsg);
	void OnParseLoginPackets(LivePeer* connection, NetworkMessage* nmsg);

protected:
	// Packet handling functions
	void OnReceiveHello(LivePeer* connection, NetworkMessage* nmsg);
	void OnReceiveReady(LivePeer* connection, NetworkMessage* nmsg);
	
	void OnReceiveCursorUpdate(LivePeer* connection, NetworkMessage* nmsg);

	void OnReceiveChatMessage(LivePeer* connection, NetworkMessage* nmsg);

	void OnReceiveNodeRequest(LivePeer* connection, NetworkMessage* nmsg);
	void OnReceiveChanges(LivePeer* connection, NetworkMessage* nmsg);
	void OnReceiveAddHouse(LivePeer* connection, NetworkMessage* nmsg);
	void OnReceiveEditHouse(LivePeer* connection, NetworkMessage* nmsg);
	void OnReceiveRemoveHouse(LivePeer* connection, NetworkMessage* nmsg);
	
protected:
	// Read from packets
	void ReceiveChanges(NetworkConnection* connection, NetworkMessage* nmsg);

protected:
	bool IsServer() const {return true;}

	typedef std::vector<LivePeer*> PeerList;

	Editor* const editor;
	wxSocketServer* serv;
	uint32_t client_mask;

	wxString name;
	wxString cd_key;

	PeerList connecting_clients;
	PeerList connected_clients;
	LiveLogTab* log;
};

typedef void (LiveServer::*LiveServerPacketParser)(LivePeer* , NetworkMessage* );

#endif
