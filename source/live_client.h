//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef _RME_LIVE_CLIENT_H_
#define _RME_LIVE_CLIENT_H_

#include "live_socket.h"
#include "net_connection.h"

#include <set>

class DirtyList;
class MapTab;

class LiveClient : public LiveSocket
{
	public:
		LiveClient();
		~LiveClient();

		//
		bool connect(const std::string& address, uint16_t port);
		void tryConnect(boost::asio::ip::tcp::resolver::iterator endpoint);

		void close();
		bool handleError(const boost::system::error_code& error);

		//
		std::string getHostName() const;

		//
		void receiveHeader();
		void receive(uint32_t packetSize);
		void send(NetworkMessage& message);

		//
		void updateCursor(const Position& position);

		LiveLogTab* createLogWindow(wxWindow* parent);
		MapTab* createEditorWindow();

		// send packets
		void sendHello();
		void sendNodeRequests();
		void sendChanges(DirtyList& dirtyList);
		void sendChat(const wxString& chatMessage);
		void sendReady();

		// Flags a node as queried and stores it, need to call SendNodeRequest to send it to server
		void queryNode(int32_t ndx, int32_t ndy, bool underground);

	protected:
		void parsePacket(NetworkMessage message);

		// parse packets
		void parseHello(NetworkMessage& message);
		void parseKick(NetworkMessage& message);
		void parseClientAccepted(NetworkMessage& message);
		void parseChangeClientVersion(NetworkMessage& message);
		void parseServerTalk(NetworkMessage& message);
		void parseNode(NetworkMessage& message);
		void parseCursorUpdate(NetworkMessage& message);
		void parseStartOperation(NetworkMessage& message);
		void parseUpdateOperation(NetworkMessage& message);

		//
		NetworkMessage readMessage;

		std::set<uint32_t> queryNodeList;
		wxString currentOperation;

		std::shared_ptr<boost::asio::ip::tcp::resolver> resolver;
		std::shared_ptr<boost::asio::ip::tcp::socket> socket;

		Editor* editor;

		bool stopped;
};

#endif
