//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////


#ifndef _RME_LIVE_SERVER_H_
#define _RME_LIVE_SERVER_H_

#include "live_socket.h"
#include "net_connection.h"
#include "action.h"

class LivePeer;
class LiveLogTab;
class QTreeNode;

class LiveServer : public LiveSocket
{
	public:
		LiveServer(Editor& editor);
		~LiveServer();

		//
		bool bind();
		void close();

		void acceptClient();
		void removeClient(uint32_t id);

		//
		void receiveHeader() {}
		void receive(uint32_t packetSize) {}
		void send(NetworkMessage& message) {}

		//
		void updateCursor(const Position& position);
		void updateClientList() const;

		//
		LiveLogTab* createLogWindow(wxWindow* parent);

		//
		uint16_t getPort() const;
		bool setPort(int32_t newPort);

		Editor* getEditor() const {
			return editor;
		}

		uint32_t getFreeClientId();
		std::string getHostName() const;

		//
		void broadcastNodes(DirtyList& dirtyList);
		void broadcastChat(const wxString& speaker, const wxString& chatMessage);
		void broadcastCursor(const LiveCursor& cursor);

		void startOperation(const wxString& operationMessage);
		void updateOperation(int32_t percent);

	protected:
		std::unordered_map<uint32_t, LivePeer*> clients;

		std::shared_ptr<boost::asio::ip::tcp::acceptor> acceptor;
		std::shared_ptr<boost::asio::ip::tcp::socket> socket;

		Editor* editor;

		uint32_t clientIds;
		uint16_t port;

		bool stopped;
};

#endif
