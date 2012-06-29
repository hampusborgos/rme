//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "live_peer.h"
#include "live_server.h"


LivePeer::LivePeer(LiveServer *server, wxSocketBase *socket) :
	NetworkConnection(server, socket),
	clientid(0),
	parser(0)
{
	lastColor = wxColor(0, 160, 0, 128);
}

LivePeer::~LivePeer()
{
	Close();
}

void LivePeer::SetNick(wxString _nick) {
	nick = _nick;
}

wxString LivePeer::GetNick() const {
	return nick;
}

void LivePeer::SetUsedColor(const wxColor& color)
{
	lastColor = color;
}

wxColor LivePeer::GetUsedColor() const
{
	return lastColor;
}