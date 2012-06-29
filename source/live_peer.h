//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef _RME_LIVE_PEER_H_
#define _RME_LIVE_PEER_H_

#include "live_server.h"

class LivePeer : public NetworkConnection
{
public:
	LivePeer(LiveServer* server, wxSocketBase* socket);
	~LivePeer();

	void SetNick(wxString nick);
	wxString GetNick() const;

	wxColor GetUsedColor() const;
	void SetUsedColor(const wxColor& color);

	// Returns 0 if local (the server itself)
	void SetClientID(uint32_t u) {clientid = u;}
	uint32_t GetClientID() const {return clientid;}

	LiveServerPacketParser parser;

protected:
	uint32_t clientid;
	wxString nick;
	wxColor lastColor;
};

#endif
