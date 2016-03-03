//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include <wx/splitter.h>
#include <wx/grid.h>

#include "live_tab.h"
#include "live_socket.h"
#include "live_peer.h"

class myGrid : public wxGrid
{
public:
	myGrid(wxWindow* parent, wxWindowID id, wxPoint pos, wxSize size) : wxGrid(parent, id, pos, size) {}
	virtual ~myGrid() {}

	void DrawCellHighlight(wxDC& dc, const wxGridCellAttr* attr) {
		//wxGrid::DrawCellHighlight(dc, attr);
	}

	DECLARE_CLASS(myGrid);
};

IMPLEMENT_CLASS(myGrid, wxGrid)

BEGIN_EVENT_TABLE(LiveLogTab, wxPanel)
	EVT_TEXT(LIVE_CHAT_TEXTBOX, LiveLogTab::OnChat)
END_EVENT_TABLE()

LiveLogTab::LiveLogTab(MapTabbook* aui, LiveSocket* server) :
	EditorTab(),
	wxPanel(aui),
	aui(aui),
	socket(server)
{
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	
	wxPanel* splitter = newd wxPanel(this);
	topsizer->Add(splitter, 1, wxEXPAND);

	// Setup left panel
	wxPanel* left_pane = newd wxPanel(splitter);
	wxSizer* left_sizer = newd wxBoxSizer(wxVERTICAL);

	wxFont time_font(*wxSWISS_FONT);

	log = newd myGrid(left_pane, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	log->SetDefaultCellFont(time_font);
	log->CreateGrid(0, 3);
	log->DisableDragRowSize();
	log->DisableDragColSize();
	log->SetSelectionMode(wxGrid::wxGridSelectRows);
	log->SetRowLabelSize(0);
	//log->SetColLabelSize(0);
	//log->EnableGridLines(false);
	log->EnableEditing(false);
	
	log->SetColLabelValue(0, wxT("Time"));
	log->SetColMinimalWidth(0, 60);
	log->SetColSize(0, 60);
	log->SetColLabelValue(1, wxT("User"));
	log->SetColMinimalWidth(1, 100);
	log->SetColSize(1, 100);
	log->SetColLabelValue(2, wxT("Message"));
	log->SetColMinimalWidth(2, 100);
	log->SetColSize(2, 100);

	log->Connect(wxEVT_SIZE, wxSizeEventHandler(LiveLogTab::OnResizeChat), nullptr, this);
	
	left_sizer->Add(log, 1, wxEXPAND);

	input = newd wxTextCtrl(left_pane, LIVE_CHAT_TEXTBOX, wxEmptyString, wxDefaultPosition, wxDefaultSize);
	left_sizer->Add(input, 0, wxEXPAND);
	
	input->Connect(wxEVT_SET_FOCUS, wxFocusEventHandler(LiveLogTab::OnSelectChatbox), nullptr, this);
	input->Connect(wxEVT_KILL_FOCUS, wxFocusEventHandler(LiveLogTab::OnDeselectChatbox), nullptr, this);

	left_pane->SetSizerAndFit(left_sizer);

	// Setup right panel
	user_list = newd myGrid(splitter, wxID_ANY, wxDefaultPosition, wxSize(280, 100));
	user_list->CreateGrid(5, 3);
	user_list->DisableDragRowSize();
	user_list->DisableDragColSize();
	user_list->SetSelectionMode(wxGrid::wxGridSelectRows);
	user_list->SetRowLabelSize(0);

	user_list->SetColLabelValue(0, wxT(""));
	user_list->SetColSize(0, 24);
	user_list->SetColLabelValue(1, wxT("#"));
	user_list->SetColSize(1, 36);
	user_list->SetColLabelValue(2, wxT("Name"));
	user_list->SetColSize(2, 200);

	//user_list->GetGridWindow()->

	// Finalize
	SetSizerAndFit(topsizer);
	
	wxSizer* split_sizer = newd wxBoxSizer(wxHORIZONTAL);
	split_sizer->Add(left_pane, wxSizerFlags(1).Expand());
	split_sizer->Add(user_list, wxSizerFlags(0).Expand());
	splitter->SetSizerAndFit(split_sizer);
	//splitter->SplitVertically(left_pane, user_list, max(this->GetSize().GetWidth() - 200, 0));

	aui->AddTab(this, true);
}

LiveLogTab::~LiveLogTab()
{

}

wxString LiveLogTab::GetTitle() const
{
	if(socket) {
		return wxT("Live Log - ") + socket->getHostName();
	}
	return wxT("Live Log - Disconnected");
}

void LiveLogTab::Disconnect()
{
	socket->log = nullptr;
	input->SetWindowStyle(input->GetWindowStyle() | wxTE_READONLY);
	socket = nullptr;
	Refresh();
}

wxString format00(wxDateTime::wxDateTime_t t)
{
	wxString str;
	if(t < 10) str << wxT("0");
	str << t;
	return str;
}

void LiveLogTab::Message(const wxString& str)
{
	wxDateTime t = wxDateTime::Now();
	wxString time, speaker;
	time << format00(t.GetHour()) << wxT(":")
		 << format00(t.GetMinute()) << wxT(":") 
		 << format00(t.GetSecond());

	speaker << wxT("Server");

	log->AppendRows(1);
	int n = log->GetNumberRows() - 1;
	log->SetCellValue(n, 0, time);
	log->SetCellValue(n, 1, speaker);
	log->SetCellValue(n, 2, str);
}

void LiveLogTab::Chat(const wxString& speaker, const wxString& str)
{
	wxDateTime t = wxDateTime::Now();
	wxString time;
	time << format00(t.GetHour()) << wxT(":")
		 << format00(t.GetMinute()) << wxT(":")
		 << format00(t.GetSecond());

	log->AppendRows(1);
	int n = log->GetNumberRows() - 1;
	log->SetCellValue(n, 0, time);
	log->SetCellValue(n, 1, speaker);
	log->SetCellValue(n, 2, str);
}

void LiveLogTab::OnChat(wxCommandEvent& evt)
{

}

void LiveLogTab::OnResizeChat(wxSizeEvent& evt)
{
	log->SetColSize(2, log->GetSize().GetWidth() - 160);
}

void LiveLogTab::OnResizeClientList(wxSizeEvent& evt)
{
}

void LiveLogTab::OnSelectChatbox(wxFocusEvent& evt)
{
	gui.DisableHotkeys();
}

void LiveLogTab::OnDeselectChatbox(wxFocusEvent& evt)
{
	gui.EnableHotkeys();
}

void LiveLogTab::UpdateClientList(const std::unordered_map<uint32_t, LivePeer*>& updatedClients)
{
	// Delete old rows
	if(user_list->GetNumberRows() > 0) {
		user_list->DeleteRows(0, user_list->GetNumberRows());
	}

	clients = updatedClients;
	user_list->AppendRows(clients.size());

	int32_t i = 0;
	for(auto& clientEntry : clients) {
		LivePeer* peer = clientEntry.second;
		user_list->SetCellBackgroundColour(i, 0, peer->getUsedColor());
		user_list->SetCellValue(i, 1, i2ws((peer->getClientId() >> 1) + 1));
		user_list->SetCellValue(i, 2, peer->getName());
		++i;
	}
}