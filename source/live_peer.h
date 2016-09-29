//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef _RME_LIVE_PEER_H_
#define _RME_LIVE_PEER_H_

#include "live_socket.h"
#include "net_connection.h"

class LiveServer;
class LivePeer : public LiveSocket
{
	public:
		LivePeer(LiveServer* server, boost::asio::ip::tcp::socket socket);
		~LivePeer();

		void close();
		bool handleError(const boost::system::error_code& error);

		//
		uint32_t getId() const { return id; }
		uint32_t getClientId() const { return clientId; }

		std::string getHostName() const;

		wxColor getUsedColor() const { return color; }
		void setUsedColor(const wxColor& newColor) { color = newColor; }

		//
		void receiveHeader();
		void receive(uint32_t packetSize);
		void send(NetworkMessage& message);

		//
		void updateCursor(const Position& position) {}

	protected:
		void parseLoginPacket(NetworkMessage message);
		void parseEditorPacket(NetworkMessage message);

		// login packets
		void parseHello(NetworkMessage& message);
		void parseReady(NetworkMessage& message);

		// editor packets
		void parseNodeRequest(NetworkMessage& message);
		void parseReceiveChanges(NetworkMessage& message);
		void parseAddHouse(NetworkMessage& message);
		void parseEditHouse(NetworkMessage& message);
		void parseRemoveHouse(NetworkMessage& message);
		void parseCursorUpdate(NetworkMessage& message);
		void parseChatMessage(NetworkMessage& message);

		//
		NetworkMessage readMessage;

		LiveServer* server;
		boost::asio::ip::tcp::socket socket;

		wxColor color;

		uint32_t id;
		uint32_t clientId;

		bool connected;

		friend class LiveLogTab;
		friend class LiveServer;
};

#endif
