
#ifndef _RME_NET_CONNECTION_H_
#define _RME_NET_CONNECTION_H_

class LiveSocket;
class LiveServer;
class NetworkConnection;

class NetworkMessage {
public:
	NetworkMessage();
	~NetworkMessage();

	void Reset();
	
	void OnSend();
	void OnReceive();

	void AddByte(uint8_t u8);
	void AddU8(uint8_t u8);
	void AddU16(uint16_t u16);
	void AddU32(uint32_t u32);
	void AddString(const std::string& str);
	void AddPosition(const Position& pos);

	uint8_t ReadByte();
	uint8_t ReadU8();
	uint16_t ReadU16();
	uint32_t ReadU32();
	std::string ReadString();
	Position ReadPosition();
protected:
	std::vector<uint8_t> buffer;
	union {
		uint32_t sent;
		uint32_t read;
	};

	friend class NetworkConnection;
};

class NetSocket {
public:
	NetSocket() {}
	virtual ~NetSocket() {}

	NetworkMessage* AllocMessage();
	void FreeMessage(NetworkMessage* msg);
	
	virtual void Log(wxString message) = 0;
protected:
	std::vector<NetworkMessage*> message_pool;
};

class NetworkConnection {
public:
	NetworkConnection(NetSocket* nsocket, wxSocketBase* socket);
	~NetworkConnection();

	void Close();

	wxString GetHost() const;

	NetworkMessage* Receive();
	void Send(NetworkMessage* nmsg); // Put a message on the send queue (might send immedietly)
	void Send(); // Send waiting messages
protected:
	NetworkMessage* receiving_message;
	wxSocketBase* socket;
	NetSocket* nsocket;
	std::deque<NetworkMessage*> waiting_messages;
};

#endif
