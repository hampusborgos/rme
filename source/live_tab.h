 //////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////


#ifndef _RME_LIVE_TAB_H_
#define _RME_LIVE_TAB_H_

#include "main.h"

#include "editor_tabs.h"
#include "application.h"

class wxGrid;

class MapTabbook;
class LiveSocket;
class LiveServer;
class LivePeer;

class LiveLogTab : public EditorTab, public wxPanel {
public:
	LiveLogTab(MapTabbook* aui, LiveSocket* socket);
	~LiveLogTab();

	void Message(const wxString& str);
	void Chat(const wxString& speaker, const wxString& str);

	virtual wxWindow* GetWindow() const {return (wxPanel*)this;}
	virtual wxString GetTitle() const;

	bool IsConnected() const {return socket != NULL;}
	void Disconnect();

	LiveSocket* GetSocket() {return socket;}

	void UpdateClientList(std::vector<LivePeer*> users);

	void OnSelectChatbox(wxFocusEvent& evt);
	void OnDeselectChatbox(wxFocusEvent& evt);

	void OnChat(wxCommandEvent& evt);
	void OnResizeChat(wxSizeEvent& evt);
	void OnResizeClientList(wxSizeEvent& evt);

protected:
	MapTabbook* aui;
	LiveSocket* socket;
	wxGrid* log;
	wxTextCtrl* input;
	wxGrid* user_list;

	std::vector<LivePeer*> peers;

	DECLARE_EVENT_TABLE();
};

#endif
