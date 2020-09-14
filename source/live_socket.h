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

#ifndef _RME_LIVE_SOCKET_H_
#define _RME_LIVE_SOCKET_H_

#include "position.h"
#include "net_connection.h"
#include "live_packets.h"
#include "filehandle.h"
#include "iomap.h"

#include <memory>
#include <unordered_map>

class LiveLogTab;
class Action;

struct LiveCursor
{
	uint32_t id;
	wxColor color;
	Position pos;
};

class LiveSocket
{
	public:
		LiveSocket();
		virtual ~LiveSocket();

		//
		wxString getName() const;
		bool setName(const wxString& newName);

		wxString getPassword() const;
		bool setPassword(const wxString& newPassword);

		wxString getLastError() const;
		void setLastError(const wxString& error);

		std::string getHostName() const;
		std::vector<LiveCursor> getCursorList() const;

		//
		void logMessage(const wxString& message);

		//
		virtual void receiveHeader() = 0;
		virtual void receive(uint32_t packetSize) = 0;
		virtual void send(NetworkMessage& message) = 0;

		//
		virtual void updateCursor(const Position& position) = 0;

	protected:
		// receive / send methods
		void receiveNode(NetworkMessage& message, Editor& editor, Action* action, int32_t ndx, int32_t ndy, bool underground);
		void sendNode(uint32_t clientId, QTreeNode* node, int32_t ndx, int32_t ndy, uint32_t floorMask);

		void receiveFloor(NetworkMessage& message, Editor& editor, Action* action, int32_t ndx, int32_t ndy, int32_t z, QTreeNode* node, Floor* floor);
		void sendFloor(NetworkMessage& message, Floor* floor);

		void receiveTile(BinaryNode* node, Editor& editor, Action* action, const Position* position);
		void sendTile(MemoryNodeFileWriteHandle& writer, Tile* tile, const Position* position);

		// read / write types
		Tile* readTile(BinaryNode* node, Editor& editor, const Position* position);

		LiveCursor readCursor(NetworkMessage& message);
		void writeCursor(NetworkMessage& message, const LiveCursor& cursor);

		//
		std::unordered_map<uint32_t, LiveCursor> cursors;

		MemoryNodeFileReadHandle mapReader;
		MemoryNodeFileWriteHandle mapWriter;
		VirtualIOMap mapVersion;

		LiveLogTab* log;

		wxString name;
		wxString password;
		wxString lastError;

		friend class LiveLogTab;
};

#endif
