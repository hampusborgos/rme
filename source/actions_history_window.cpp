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
#include "artprovider.h"
#include "editor.h"
#include "gui.h"

HistoryListBox::HistoryListBox(wxWindow* parent) :
	wxVListBox(parent, wxID_ANY)
{
	wxSize icon_size = FROM_DIP(parent, wxSize(16, 16));
	open_bitmap = wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR, icon_size);
	move_bitmap = wxArtProvider::GetBitmap(ART_MOVE, wxART_LIST, icon_size);
	remote_bitmap = wxArtProvider::GetBitmap(ART_REMOTE, wxART_LIST, icon_size);
	select_bitmap = wxArtProvider::GetBitmap(ART_SELECT, wxART_LIST, icon_size);
	unselect_bitmap = wxArtProvider::GetBitmap(ART_UNSELECT, wxART_LIST, icon_size);
	delete_bitmap = wxArtProvider::GetBitmap(ART_DELETE, wxART_LIST, icon_size);
	cut_bitmap = wxArtProvider::GetBitmap(ART_CUT, wxART_LIST, icon_size);
	paste_bitmap = wxArtProvider::GetBitmap(ART_PASTE, wxART_LIST, icon_size);
	randomize_bitmap = wxArtProvider::GetBitmap(ART_RANDOMIZE, wxART_LIST, icon_size);
	borderize_bitmap = wxArtProvider::GetBitmap(ART_BORDERIZE, wxART_LIST, icon_size);
	draw_bitmap = wxArtProvider::GetBitmap(ART_DRAW, wxART_LIST, icon_size);
	erase_bitmap = wxArtProvider::GetBitmap(ART_ERASE, wxART_LIST, icon_size);
	switch_bitmap = wxArtProvider::GetBitmap(ART_SWITCH, wxART_LIST, icon_size);
	rotate_bitmap = wxArtProvider::GetBitmap(ART_ROTATE, wxART_LIST, icon_size);
	replace_bitmap = wxArtProvider::GetBitmap(ART_REPLACE, wxART_LIST, icon_size);
	change_bitmap = wxArtProvider::GetBitmap(ART_CHANGE, wxART_LIST, icon_size);
}

void HistoryListBox::OnDrawItem(wxDC& dc, const wxRect& rect, size_t index) const
{
	const Editor* editor = g_gui.GetCurrentEditor();
	if(!editor) {
		return;
	}

	const ActionQueue* actions = editor->getHistoryActions();
	if(!actions) {
		return;
	}

	if(IsSelected(index)) {
		dc.SetTextForeground(*wxBLUE);
	} else {
		dc.SetTextForeground(*wxBLACK);
	}

	const BatchAction* action = actions->getAction(index - 1);
	if(action) {
		const wxBitmap& bitmap = getIconBitmap(action->getType());
		dc.DrawBitmap(bitmap, rect.GetX() + 4, rect.GetY() + 4, true);
		dc.DrawText(action->getLabel(), rect.GetX() + 28, rect.GetY() + 3);
	} else {
		dc.DrawBitmap(open_bitmap, rect.GetX() + 4, rect.GetY() + 4, true);
		dc.DrawText("Open Map", rect.GetX() + 28, rect.GetY() + 3);
	}
}

wxCoord HistoryListBox::OnMeasureItem(size_t index) const
{
	return 24;
}

const wxBitmap& HistoryListBox::getIconBitmap(ActionIdentifier identifier) const
{
	switch (identifier)
	{
		case ACTION_MOVE:
			return move_bitmap;
		case ACTION_REMOTE:
			return remote_bitmap;
		case ACTION_SELECT:
			return select_bitmap;
		case ACTION_UNSELECT:
			return unselect_bitmap;
		case ACTION_DELETE_TILES:
			return delete_bitmap;
		case ACTION_CUT_TILES:
			return cut_bitmap;
		case ACTION_PASTE_TILES:
			return paste_bitmap;
		case ACTION_RANDOMIZE:
			return randomize_bitmap;
		case ACTION_BORDERIZE:
			return borderize_bitmap;
		case ACTION_DRAW:
			return draw_bitmap;
		case ACTION_ERASE:
			return erase_bitmap;
		case ACTION_SWITCHDOOR:
			return switch_bitmap;
		case ACTION_ROTATE_ITEM:
			return rotate_bitmap;
		case ACTION_REPLACE_ITEMS:
			return replace_bitmap;
		case ACTION_CHANGE_PROPERTIES:
			return change_bitmap;
		default:
			return wxNullBitmap;
	}
}

ActionsHistoryWindow::ActionsHistoryWindow(wxWindow* parent) :
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(230, 250))
{
	SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	list = new HistoryListBox(this);
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

	const Editor* editor = g_gui.GetCurrentEditor();
	if(!editor) {
		list->SetItemCount(0);
		list->Refresh();
		return;
	}

	size_t count = 1;
	int selection = 0;

	const ActionQueue* actions = editor->getHistoryActions();
	if(actions) {
		count += actions->size();
		selection += actions->getCurrentIndex();
	}

	list->SetItemCount(count);
	list->SetSelection(selection);
	list->Refresh();
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
