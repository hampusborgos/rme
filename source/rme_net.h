//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#if 0

#ifndef _RME_NET_H_
#define _RME_NET_H_

#include "net_connection.h"

class RMENet : public wxEvtHandler, public NetSocket
{
	~RMENet();
public:
	RMENet(wxEvtHandler* event_dump);

	bool Connect();
	void Close();
	void Log(wxString message) {};

	void HandleEvent(wxSocketEvent& evt);

	bool CanHostLive() {return can_host;}

protected:
	void OnParsePacket(NetworkMessage* nmsg);

	void OnReceiveWelcome(NetworkMessage* nmsg);
	void OnReceiveDisconnect(NetworkMessage* nmsg);
	void OnReceivePing(NetworkMessage* nmsg);

	wxSocketClient* socket;
	NetworkConnection* connection;
	bool can_host;

	wxEvtHandler* event_dump;
};

//=============================================================================
// Events

extern const wxEventType EVT_RMENET_CONNECTION_ESTABLISHED;

#define EVT_RMENET_CONNECTION_ESTABLISHED(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        EVT_RMENET_CONNECTION_ESTABLISHED, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),

extern const wxEventType EVT_RMENET_CONNECTION_LOST;

#define EVT_RMENET_CONNECTION_LOST(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        EVT_RMENET_CONNECTION_LOST, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),

extern const wxEventType EVT_RMENET_SHOW_SERVLIST;

#define EVT_RMENET_SHOW_SERVLIST(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        EVT_RMENET_SHOW_SERVLIST, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) NULL \
    ),

#endif

#endif
