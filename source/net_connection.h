
#ifndef _RME_NET_CONNECTION_H_
#define _RME_NET_CONNECTION_H_

#include "position.h"

#include <string>
#include <vector>
#include <cstdint>
#include <thread>
#include <mutex>

struct NetworkMessage
{
	NetworkMessage();

	void clear();
	void expand(const size_t length);

	//
	template<typename T> T read()
	{
		T& value = *reinterpret_cast<T*>(&buffer[position]);
		position += sizeof(T);
		return value;
	}

	template<typename T> void write(const T& value)
	{
		expand(sizeof(T));
		memcpy(&buffer[position], &value, sizeof(T));
		position += sizeof(T);
	}

	//
	std::vector<uint8_t> buffer;
	size_t position;
	size_t size;
};

template<> std::string NetworkMessage::read<std::string>();
template<> Position NetworkMessage::read<Position>();
template<> void NetworkMessage::write<std::string>(const std::string& value);
template<> void NetworkMessage::write<Position>(const Position& value);

class NetworkConnection
{
	private:
		NetworkConnection();
		NetworkConnection(const NetworkConnection& copy) = delete;

	public:
		~NetworkConnection();

		static NetworkConnection& getInstance();

		bool start();
		void stop();

		boost::asio::io_service& get_service();

	private:
		boost::asio::io_service* service;
		std::thread thread;
		bool stopped;
};

#endif
