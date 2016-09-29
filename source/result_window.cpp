//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/result_window.hpp $
// $Id: result_window.hpp 310 2010-02-26 18:03:48Z admin $

#include "main.h"

#include "result_window.h"
#include "gui.h"
#include "position.h"

BEGIN_EVENT_TABLE(SearchResultWindow, wxPanel)
	EVT_LISTBOX(wxID_ANY, SearchResultWindow::OnClickResult)
END_EVENT_TABLE()

SearchResultWindow::SearchResultWindow(wxWindow* parent) :
	wxPanel(parent, wxID_ANY)
{
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	result_list = newd wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(200, 330), 0, nullptr, wxLB_SINGLE | wxLB_ALWAYS_SB | wxLB_SORT);
	sizer->Add(result_list, wxSizerFlags(1).Expand());
	SetSizerAndFit(sizer);
}

SearchResultWindow::~SearchResultWindow()
{
	Clear();
}

void SearchResultWindow::Clear()
{
	for(uint32_t n = 0; n < result_list->GetCount(); ++n) {
		delete reinterpret_cast<Position*>(result_list->GetClientData(n));
	}
	result_list->Clear();
}

void SearchResultWindow::AddPosition(wxString description, Position pos)
{
	result_list->Append(description << wxT(" (") << pos.x << wxT(",") << pos.y << wxT(",") << pos.z << wxT(")"), newd Position(pos));
}

void SearchResultWindow::OnClickResult(wxCommandEvent& event)
{
	Position* pos = reinterpret_cast<Position*>(event.GetClientData());
	if(pos) {
		g_gui.SetScreenCenterPosition(*pos);
	}
}
