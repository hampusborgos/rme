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

#ifndef _RME_LIVE_TAB_H_
#define _RME_LIVE_TAB_H_

#include "main.h"

#include "editor_tabs.h"
#include "application.h"
#include "live_server.h"

class wxGrid;

class MapTabbook;
class LiveSocket;
class LiveServer;

class LiveLogTab : public EditorTab, public wxPanel {
public:
	LiveLogTab(MapTabbook* aui, LiveSocket* socket);
	~LiveLogTab();

	void Message(const wxString& str);
	void Chat(const wxString& speaker, const wxString& str);

	virtual wxWindow* GetWindow() const {return (wxPanel*)this;}
	virtual wxString GetTitle() const;

	bool IsConnected() const {return socket != nullptr;}
	void Disconnect();

	LiveSocket* GetSocket() {return socket;}

	void UpdateClientList(const std::unordered_map<uint32_t, LivePeer*>& updatedClients);

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

	std::unordered_map<uint32_t, LivePeer*> clients;

	DECLARE_EVENT_TABLE();
};

#endif
