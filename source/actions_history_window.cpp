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

#include "main.h"
#include "actions_history_window.h"
#include "editor.h"
#include "gui.h"

ActionsHistoryWindow::ActionsHistoryWindow(wxWindow* parent) :
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(230, 250))
{
	SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	list = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE | wxLB_ALWAYS_SB);
	list->SetCanFocus(false);
	sizer->Add(list, 1, wxEXPAND, 5);

	SetSizer(sizer);
	Layout();

	// Connect Events
	list->Connect(wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler(ActionsHistoryWindow::OnListSelected), NULL, this);
}

ActionsHistoryWindow::~ActionsHistoryWindow()
{
	// Disconnect Events
	list->Disconnect(wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler(ActionsHistoryWindow::OnListSelected), NULL, this);
}

void ActionsHistoryWindow::RefreshActions()
{
	if(!IsShownOnScreen())
		return;

	list->Clear();

	Editor* editor = g_gui.GetCurrentEditor();
	if(editor && editor->getHistoryActions()) {
		list->Append("Open Map");

		const ActionQueue::ActionList& actions = editor->getHistoryActions()->getActions();
		for(BatchAction* batch : actions) {
			list->Append(batch->getLabel());
		}

		int index = editor->getHistoryActions()->getCurrentIndex();
		if(index >= 0 && index < (int)actions.size())
			list->Select(index);
		else if(!list->IsEmpty())
			list->Select(list->GetCount() - 1);
	}
}

void ActionsHistoryWindow::OnListSelected(wxCommandEvent& event)
{
	int index = list->GetSelection();
	if(index == wxNOT_FOUND)
		return;

	Editor* editor = g_gui.GetCurrentEditor();
	if(editor && editor->getHistoryActions()) {
		int current = editor->getHistoryActions()->getCurrentIndex();
		if(index > current) {
			editor->redo(index - current);
		} else if (index < current) {
			editor->undo(current - index);
		}
	}
}
