//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"
#include "net_connection.h"

NetworkMessage::NetworkMessage()
{
	clear();
}

void NetworkMessage::clear()
{
	buffer.resize(4);
	position = 4;
	size = 0;
}

void NetworkMessage::expand(const size_t length)
{
	if(position + length >= buffer.size()) {
		buffer.resize(position + length + 1);
	}
	size += length;
}

template<> std::string NetworkMessage::read<std::string>()
{
	const uint16_t length = read<uint16_t>();
	char* strBuffer = reinterpret_cast<char*>(&buffer[position]);
	position += length;
	return std::string(strBuffer, length);
}

template<> Position NetworkMessage::read<Position>()
{
	Position position;
	position.x = read<uint16_t>();
	position.y = read<uint16_t>();
	position.z = read<uint8_t>();
	return position;
}

template<> void NetworkMessage::write<std::string>(const std::string& value)
{
	const size_t length = value.length();
	write<uint16_t>(length);

	expand(length);
	memcpy(&buffer[position], &value[0], length);
	position += length;
}

template<> void NetworkMessage::write<Position>(const Position& value)
{
	write<uint16_t>(value.x);
	write<uint16_t>(value.y);
	write<uint8_t>(value.z);
}

// NetworkConnection
NetworkConnection::NetworkConnection() :
	service(nullptr), thread(), stopped(false)
{
	//
}

NetworkConnection::~NetworkConnection()
{
	stop();
}

NetworkConnection& NetworkConnection::getInstance()
{
	static NetworkConnection connection;
	return connection;
}

bool NetworkConnection::start()
{
	if(thread.joinable()) {
		if(stopped) {
			return false;
		}
		return true;
	}

	stopped = false;
	if(!service) {
		service = new boost::asio::io_service;
	}

	thread = std::thread([this]() -> void {
		boost::asio::io_service& serviceRef = *service;
		try {
			while(!stopped) {
				serviceRef.run_one();
				serviceRef.reset();
			}
		} catch (std::exception& e) {
			std::cout << e.what() << std::endl;
		}
	});
	return true;
}

void NetworkConnection::stop()
{
	if(!service) {
		return;
	}

	service->stop();
	stopped = true;
	thread.join();

	delete service;
	service = nullptr;
}

boost::asio::io_service& NetworkConnection::get_service()
{
	return *service;
}
