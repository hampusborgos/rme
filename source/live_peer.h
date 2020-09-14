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
