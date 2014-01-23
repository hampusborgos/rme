//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef _RME_LIVE_SOCKET_H_
#define _RME_LIVE_SOCKET_H_

#include "position.h"
#include "net_connection.h"
#include "live_packets.h"
#include "filehandle.h"
#include "iomap.h"

class LivePeer;
class LiveLogTab;
class NetworkMessage;
class Action;

struct LiveCursor
{
	uint32_t id;
	wxColor color;
	Position pos;
};

class LiveSocket : public wxEvtHandler, public NetSocket
{
public:
	LiveSocket();
	virtual ~LiveSocket();

	bool SetPassword(const wxString& password);
	bool SetPort(long port);
	bool SetIP(const wxString& IP);

	virtual void Close();
	
	virtual LiveLogTab* CreateLogWindow(wxWindow* parent) = 0;

	virtual wxString GetHostName() const = 0;
	wxString GetLastError() const;

	std::vector<LiveCursor> GetCursorList() const;

public:
	// Events that are common to both server and client
	virtual void UpdateCursor(Position pos) = 0;
	

protected:
	// Internal
	virtual bool IsServer() const = 0;
	bool IsClient() const {return ! IsServer();}

	void ReceiveNode(NetworkMessage* nmsg, Editor& editor, Action* action, int ndx, int ndy, bool underground);
	void ReceiveFloor(NetworkMessage* nmsg, Editor& editor, Action* action, int ndx, int ndy, int z, QTreeNode* nd, Floor* floor);
	void ReceiveTile(BinaryNode* tileNode, Editor& editor, Action* action, const Position* pos);
	Tile* ReadTile(BinaryNode* tileNode, Map& map, const Position* mpos = nullptr);

	void SendNode(LivePeer* connection, QTreeNode* nd, int ndx, int ndy, uint32_t floormask);
	void AddFloor(NetworkMessage* omsg, Floor* floor);
	void AddTile(MemoryNodeFileWriteHandle& writer, Tile* tile, const Position* pos);

	void AddCursor(NetworkMessage* nmsg, const LiveCursor& cursor);
	LiveCursor ReadCursor(NetworkMessage* nmsg);

public:
	void Log(wxString message);
	void DisconnectLog();

protected:
	MemoryNodeFileReadHandle bn_reader;
	MemoryNodeFileWriteHandle bn_writer;
	VirtualIOMap map_version;
	LiveLogTab* log;

	wxString last_err;

	wxString password;
	wxIPV4address ipaddr;

	std::map<uint32_t, LiveCursor> cursors;

	friend class LiveLogTab;
};

#endif
