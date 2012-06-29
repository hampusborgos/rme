//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "net_connection.h"

#include "tile.h"

NetworkConnection::NetworkConnection(NetSocket* nsocket, wxSocketBase* socket) :
	socket(socket),
	nsocket(nsocket),
	receiving_message(NULL)
{
}

NetworkConnection::~NetworkConnection()
{
	while(waiting_messages.size() > 0)
	{
		delete waiting_messages.back();
		waiting_messages.pop_back();
	}
}

void NetworkConnection::Close()
{
	if(socket)
	{
		socket->Destroy();
		socket = NULL;
	}
}

wxString NetworkConnection::GetHost() const
{
	wxIPV4address addr;
	socket->GetPeer(addr);
	wxString str;
	str << addr.Hostname();
	return str;
}

void NetworkConnection::Send(NetworkMessage* nmsg)
{
	waiting_messages.push_back(nmsg);
	
	nmsg->OnSend();

	Send();
}

void NetworkConnection::Send()
{
	if(!socket)
		return;

	while(waiting_messages.size() > 0)
	{
		NetworkMessage* current_msg = waiting_messages.front();
		std::vector<uint8_t>& buffer = current_msg->buffer;
		uint32_t& sent = current_msg->sent;
		if(sent == ~0)
		{
			// Gotta add 4 bytes size
			uint32_t msgsize = buffer.size() - 4;
			uint8_t* t = &buffer[0];
			memcpy(t, &msgsize, 4);
			sent = 0;
			//std::cout << "Sending message ...\n";
			//for(uint8_t* c = &buffer.front(); c != &buffer.back(); ++c) {
//				printf("\t%d\t=%.2x\n", c - &buffer.front() - 4, *c);
//			}
		}
		socket->Write(&buffer[0] + sent, buffer.size() - sent);
//		std::cout << "Sent " << socket->LastCount() << " bytes!\n";
		if(socket->Error())
		{
			if(socket->LastError() == wxSOCKET_WOULDBLOCK)
			{
				// Socket is full, wait 'til it's open to send more
			}
			else
			{
				// Fail :(
				nsocket->Log(wxT("SOCKET WRITE ERROR!"));
				//nsocket->Close();
			}
			return;
		}

		if(socket->LastCount() == 0)
		{
			// ?
		}

		sent += socket->LastCount();
		if(sent == buffer.size())
		{
			nsocket->FreeMessage(current_msg);
			waiting_messages.pop_front();
		}
	}
}

NetworkMessage* NetworkConnection::Receive()
{
	if(!receiving_message)
	{
		uint32_t sz;
		socket->Peek(&sz, 4);
		if(socket->LastCount() < 4)
		{
			return NULL;
		}
		if(socket->Error())
		{
			nsocket->Log(wxT("SOCKET READ ERROR (1)!"));
			return NULL;
			//lsocket->Close();
		}
		if(sz > 256*1024) {
			nsocket->Log(wxT("SOCKET OVERFLOW ERROR (2)!"));
			return NULL;
		}
		socket->Read(&sz, 4);


		receiving_message = nsocket->AllocMessage();
		receiving_message->buffer.resize(sz);
		receiving_message->read = 0;
//		std::cout << "Received 4 bytes (size)" << std::endl;
	}	
	std::vector<uint8_t>& buffer = receiving_message->buffer;
	uint32_t& read = receiving_message->read;
	if(buffer.size())
	{
		socket->Read(&buffer[read], buffer.size() - read);
		
		if(socket->Error()) {
			nsocket->Log(wxT("SOCKET READ ERROR (2)!"));

			//lsocketz->Close();
			return NULL;
		}

		read += socket->LastCount();
	}

	if(read == buffer.size()) {
//		std::cout << "Received message ...\n";
//		for(uint8_t* c = &buffer.front(); c != &buffer.back(); ++c) {
//			printf("\t%d\t=%.2x\n", c - &buffer.front(), *c);
//		}
		NetworkMessage* tmp = receiving_message;
		read = 0;
		receiving_message = NULL;
		tmp->OnReceive();
		return tmp;
	}
	// Complete message hasn't been read yet.
	return NULL;
}


NetworkMessage* NetSocket::AllocMessage() {
	if(message_pool.size()) {
		NetworkMessage* msg = message_pool.back();
		message_pool.pop_back();
		return msg;
	}
	return newd NetworkMessage();
}

void NetSocket::FreeMessage(NetworkMessage* msg) {
	msg->Reset();
	message_pool.push_back(msg);
}

NetworkMessage::NetworkMessage() {
	Reset();
}

NetworkMessage::~NetworkMessage() {
}

void NetworkMessage::Reset() {
	buffer.resize(4);
	sent = ~0;
}

void NetworkMessage::OnSend()
{
	for(std::vector<uint8_t>::iterator b = buffer.begin(); b != buffer.end(); ++b)
	{
		*b = ((*b ^ 134) << 5) | (*b >> 3); *b ^= 9;
	}
}

void NetworkMessage::OnReceive()
{
	for(std::vector<uint8_t>::iterator b = buffer.begin(); b != buffer.end(); ++b)
	{
		*b ^= 9; *b = (((*b  >> 5) ^ 134) & 0x1f) | ((*b << 3));
	}
}

void NetworkMessage::AddByte(uint8_t u8) {
	buffer.push_back(u8);
}

void NetworkMessage::AddU8(uint8_t u8) {
	buffer.push_back(u8);
}

void NetworkMessage::AddU16(uint16_t u16) {
	buffer.resize(buffer.size() + 2);
	memcpy(&buffer[buffer.size() - 2], &u16, 2);
}

void NetworkMessage::AddU32(uint32_t u32) {
	buffer.resize(buffer.size() + 4);
	memcpy(&buffer[buffer.size() - 4], &u32, 4);
}

void NetworkMessage::AddString(const std::string& str) {
	size_t sz = min(str.size(), 0xFFFFul);
	AddU16(sz);
	if(sz > 0) {
		buffer.resize(buffer.size() + sz);
		memcpy(&buffer[buffer.size() - sz], str.c_str(), sz);
	}
}

void NetworkMessage::AddPosition(const Position& pos) {
	AddU16(pos.x);
	AddU16(pos.y);
	AddU8(pos.z);
}

uint8_t NetworkMessage::ReadU8() {
	if(read+1 > buffer.size())
		throw std::runtime_error("Read beyond end of network buffer!");
	return buffer[read++];
}

uint8_t NetworkMessage::ReadByte() {
	if(read+1 > buffer.size())
		throw std::runtime_error("Read beyond end of network buffer!");
	return buffer[read++];
}

uint16_t NetworkMessage::ReadU16() {
	if(read+2 > buffer.size())
		throw std::runtime_error("Read beyond end of network buffer!");
	read += 2;
	return *reinterpret_cast<uint16_t*>(&buffer[read - 2]);
}

uint32_t NetworkMessage::ReadU32() {
	if(read+4 > buffer.size())
		throw std::runtime_error("Read beyond end of network buffer!");
	read += 4;
	return *reinterpret_cast<uint32_t*>(&buffer[read - 4]);
}

std::string NetworkMessage::ReadString() {
	uint16_t sz = ReadU16();
	if(sz == 0)
		return "";
	if(read+sz > buffer.size())
		throw std::runtime_error("Read beyond end of network buffer!");
	std::string str((const char*)&buffer[read], sz);
	read += sz;
	return str;
}

Position NetworkMessage::ReadPosition() {
	Position pos;
	pos.x = ReadU16();
	pos.y = ReadU16();
	pos.z = ReadU8();
	return pos;
}

