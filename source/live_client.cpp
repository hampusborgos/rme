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

#include "live_client.h"
#include "live_tab.h"
#include "live_action.h"
#include "editor.h"

#include <wx/event.h>

LiveClient::LiveClient() : LiveSocket(),
	readMessage(), queryNodeList(), currentOperation(),
	resolver(nullptr), socket(nullptr), editor(nullptr), stopped(false)
{
	//
}

LiveClient::~LiveClient()
{
	//
}

bool LiveClient::connect(const std::string& address, uint16_t port)
{
	NetworkConnection& connection = NetworkConnection::getInstance();
	if(!connection.start()) {
		setLastError("The previous connection has not been terminated yet.");
		return false;
	}

	auto& service = connection.get_service();
	if(!resolver) {
		resolver = std::make_shared<boost::asio::ip::tcp::resolver>(service);
	}

	if(!socket) {
		socket = std::make_shared<boost::asio::ip::tcp::socket>(service);
	}

	boost::asio::ip::tcp::resolver::query query(address, std::to_string(port));
	resolver->async_resolve(query, [this](const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator endpoint_iterator) -> void
	{
		if(error) {
			logMessage("Error: " + error.message());
		} else {
			tryConnect(endpoint_iterator);
		}
	});

	/*
	if(!client->WaitOnConnect(5, 0)) {
		if(log)
			log->Disconnect();
		last_err = "Connection timed out.";
		client->Destroy();
		client = nullptr;
		delete connection;
		return false;
	}

	if(!client->IsConnected()) {
		if(log)
			log->Disconnect();
		last_err = "Connection refused by peer.";
		client->Destroy();
		client = nullptr;
		delete connection;
		return false;
	}

	if(log)
		log->Message("Connection established!");
	*/
	return true;
}

void LiveClient::tryConnect(boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
	if(stopped) {
		return;
	}

	if(endpoint_iterator == boost::asio::ip::tcp::resolver::iterator()) {
		return;
	}

	logMessage("Joining server " + endpoint_iterator->host_name() + ":" + endpoint_iterator->service_name() + "...");

	boost::asio::async_connect(*socket, endpoint_iterator, [this](boost::system::error_code error, boost::asio::ip::tcp::resolver::iterator endpoint_iterator) -> void
	{
		if(!socket->is_open()) {
			tryConnect(++endpoint_iterator);
		} else if(error) {
			if(handleError(error)) {
				tryConnect(++endpoint_iterator);
			} else {
				wxTheApp->CallAfter([this]() {
					close();
					g_gui.CloseLiveEditors(this);
				});
			}
		} else {
			socket->set_option(boost::asio::ip::tcp::no_delay(true), error);
			if(error) {
				wxTheApp->CallAfter([this]() {
					close();
				});
				return;
			}
			sendHello();
			receiveHeader();
		}
	});
}

void LiveClient::close()
{
	if(resolver) {
		resolver->cancel();
	}

	if(socket) {
		socket->close();
	}

	if(log) {
		log->Message("Disconnected from server.");
		log->Disconnect();
		log = nullptr;
	}

	stopped = true;
}

bool LiveClient::handleError(const boost::system::error_code& error)
{
	if(error == boost::asio::error::eof || error == boost::asio::error::connection_reset) {
		wxTheApp->CallAfter([this]() {
			log->Message(wxString() + getHostName() + ": disconnected.");
			close();
		});
		return true;
	} else if(error == boost::asio::error::connection_aborted) {
		logMessage("You have left the server.");
		return true;
	}
	return false;
}

std::string LiveClient::getHostName() const
{
	if(!socket) {
		return "not connected";
	}
	return socket->remote_endpoint().address().to_string();
}

void LiveClient::receiveHeader()
{
	readMessage.position = 0;
	boost::asio::async_read(*socket,
		boost::asio::buffer(readMessage.buffer, 4),
		[this](const boost::system::error_code& error, size_t bytesReceived) -> void {
			if(error) {
				if(!handleError(error)) {
					logMessage(wxString() + getHostName() + ": " + error.message());
				}
			} else if(bytesReceived < 4) {
				logMessage(wxString() + getHostName() + ": Could not receive header[size: " + std::to_string(bytesReceived) + "], disconnecting client.");
			} else {
				receive(readMessage.read<uint32_t>());
			}
		}
	);
}

void LiveClient::receive(uint32_t packetSize)
{
	readMessage.buffer.resize(readMessage.position + packetSize);
	boost::asio::async_read(*socket,
		boost::asio::buffer(&readMessage.buffer[readMessage.position], packetSize),
		[this](const boost::system::error_code& error, size_t bytesReceived) -> void {
			if(error) {
				if(!handleError(error)) {
					logMessage(wxString() + getHostName() + ": " + error.message());
				}
			} else if(bytesReceived < readMessage.buffer.size() - 4) {
				logMessage(wxString() + getHostName() + ": Could not receive packet[size: " + std::to_string(bytesReceived) + "], disconnecting client.");
			} else {
				wxTheApp->CallAfter([this]() {
					parsePacket(std::move(readMessage));
					receiveHeader();
				});
			}
		}
	);
}

void LiveClient::send(NetworkMessage& message)
{
	memcpy(&message.buffer[0], &message.size, 4);
	boost::asio::async_write(*socket,
		boost::asio::buffer(message.buffer, message.size + 4),
		[this](const boost::system::error_code& error, size_t bytesTransferred) -> void {
			if(error) {
				logMessage(wxString() + getHostName() + ": " + error.message());
			}
		}
	);
}

void LiveClient::updateCursor(const Position& position)
{
	LiveCursor cursor;
	cursor.id = 77; // Unimportant, server fixes it for us
	cursor.pos = position;
	cursor.color = wxColor(
		g_settings.getInteger(Config::CURSOR_RED),
		g_settings.getInteger(Config::CURSOR_GREEN),
		g_settings.getInteger(Config::CURSOR_BLUE),
		g_settings.getInteger(Config::CURSOR_ALPHA)
	);

	NetworkMessage message;
	message.write<uint8_t>(PACKET_CLIENT_UPDATE_CURSOR);
	writeCursor(message, cursor);

	send(message);
}

LiveLogTab* LiveClient::createLogWindow(wxWindow* parent)
{
	MapTabbook* mtb = dynamic_cast<MapTabbook*>(parent);
	ASSERT(mtb);

	log = newd LiveLogTab(mtb, this);
	log->Message("New Live mapping session started.");

	return log;
}

MapTab* LiveClient::createEditorWindow()
{
	MapTabbook* mtb = dynamic_cast<MapTabbook*>(g_gui.tabbook);
	ASSERT(mtb);

	MapTab* edit = newd MapTab(mtb, editor);
	edit->OnSwitchEditorMode(g_gui.IsSelectionMode() ? SELECTION_MODE : DRAWING_MODE);

	return edit;
}

void LiveClient::sendHello()
{
	NetworkMessage message;
	message.write<uint8_t>(PACKET_HELLO_FROM_CLIENT);
	message.write<uint32_t>(__RME_VERSION_ID__);
	message.write<uint32_t>(__LIVE_NET_VERSION__);
	message.write<uint32_t>(g_gui.GetCurrentVersionID());
	message.write<std::string>(nstr(name));
	message.write<std::string>(nstr(password));

	send(message);
}

void LiveClient::sendNodeRequests()
{
	if(queryNodeList.empty()) {
		return;
	}

	NetworkMessage message;
	message.write<uint8_t>(PACKET_REQUEST_NODES);

	message.write<uint32_t>(queryNodeList.size());
	for(uint32_t node : queryNodeList) {
		message.write<uint32_t>(node);
	}

	send(message);
	queryNodeList.clear();
}

void LiveClient::sendChanges(DirtyList& dirtyList)
{
	ChangeList& changeList = dirtyList.GetChanges();
	if(changeList.empty()) {
		return;
	}

	mapWriter.reset();
	for(Change* change : changeList) {
		switch (change->getType()) {
			case CHANGE_TILE: {
				const Position& position = static_cast<Tile*>(change->getData())->getPosition();
				sendTile(mapWriter, editor->map.getTile(position), &position);
				break;
			}
			default:
				break;
		}
	}
	mapWriter.endNode();

	NetworkMessage message;
	message.write<uint8_t>(PACKET_CHANGE_LIST);

	std::string data(reinterpret_cast<const char*>(mapWriter.getMemory()), mapWriter.getSize());
	message.write<std::string>(data);

	send(message);
}

void LiveClient::sendChat(const wxString& chatMessage)
{
	NetworkMessage message;
	message.write<uint8_t>(PACKET_CLIENT_TALK);
	message.write<std::string>(nstr(chatMessage));
	send(message);
}

void LiveClient::sendReady()
{
	NetworkMessage message;
	message.write<uint8_t>(PACKET_READY_CLIENT);
	send(message);
}

void LiveClient::queryNode(int32_t ndx, int32_t ndy, bool underground)
{
	uint32_t nd = 0;
	nd |= ((ndx >> 2) << 18);
	nd |= ((ndy >> 2) << 4);
	nd |= (underground ? 1 : 0);
	queryNodeList.insert(nd);
}

void LiveClient::parsePacket(NetworkMessage message)
{
	uint8_t packetType;
	while(message.position < message.buffer.size()) {
		packetType = message.read<uint8_t>();
		switch (packetType) {
			case PACKET_HELLO_FROM_SERVER:
				parseHello(message);
				break;
			case PACKET_KICK:
				parseKick(message);
				break;
			case PACKET_ACCEPTED_CLIENT:
				parseClientAccepted(message);
				break;
			case PACKET_CHANGE_CLIENT_VERSION:
				parseChangeClientVersion(message);
				break;
			case PACKET_SERVER_TALK:
				parseServerTalk(message);
				break;
			case PACKET_NODE:
				parseNode(message);
				break;
			case PACKET_CURSOR_UPDATE:
				parseCursorUpdate(message);
				break;
			case PACKET_START_OPERATION:
				parseStartOperation(message);
				break;
			case PACKET_UPDATE_OPERATION:
				parseUpdateOperation(message);
				break;
			default: {
				log->Message("Unknown packet receieved!");
				close();
				break;
			}
		}
	}
}

void LiveClient::parseHello(NetworkMessage& message)
{
	ASSERT(editor == nullptr);
	editor = newd Editor(g_gui.copybuffer, this);

	Map& map = editor->map;
	map.setName("Live Map - " + message.read<std::string>());
	map.setWidth(message.read<uint16_t>());
	map.setHeight(message.read<uint16_t>());

	createEditorWindow();
}

void LiveClient::parseKick(NetworkMessage& message)
{
	const std::string& kickMessage = message.read<std::string>();
	close();

	g_gui.PopupDialog("Disconnected", wxstr(kickMessage), wxOK);
}

void LiveClient::parseClientAccepted(NetworkMessage& message)
{
	sendReady();
}

void LiveClient::parseChangeClientVersion(NetworkMessage& message)
{
	ClientVersionID clientVersion = static_cast<ClientVersionID>(message.read<uint32_t>());
	if(!g_gui.CloseAllEditors()) {
		close();
		return;
	}

	wxString error;
	wxArrayString warnings;
	g_gui.LoadVersion(clientVersion, error, warnings);

	sendReady();
}

void LiveClient::parseServerTalk(NetworkMessage& message)
{
	const std::string& speaker = message.read<std::string>();
	const std::string& chatMessage = message.read<std::string>();
	log->Chat(
		wxstr(speaker),
		wxstr(chatMessage)
	);
}

void LiveClient::parseNode(NetworkMessage& message)
{
	uint32_t ind = message.read<uint32_t>();

	// Extract node position
	int32_t ndx = ind >> 18;
	int32_t ndy = (ind >> 4) & 0x3FFF;
	bool underground = ind & 1;

	Action* action = editor->actionQueue->createAction(ACTION_REMOTE);
	receiveNode(message, *editor, action, ndx, ndy, underground);
	editor->actionQueue->addAction(action);

	g_gui.RefreshView();
	g_gui.UpdateMinimap();
}

void LiveClient::parseCursorUpdate(NetworkMessage& message)
{
	LiveCursor cursor = readCursor(message);
	cursors[cursor.id] = cursor;

	g_gui.RefreshView();
}

void LiveClient::parseStartOperation(NetworkMessage& message)
{
	const std::string& operation = message.read<std::string>();

	currentOperation = wxstr(operation);
	g_gui.SetStatusText("Server Operation in Progress: " + currentOperation + "... (0%)");
}

void LiveClient::parseUpdateOperation(NetworkMessage& message)
{
	int32_t percent = message.read<uint32_t>();
	if(percent >= 100) {
		g_gui.SetStatusText("Server Operation Finished.");
	} else {
		g_gui.SetStatusText("Server Operation in Progress: " + currentOperation + "... (" + std::to_string(percent) + "%)");
	}
}
