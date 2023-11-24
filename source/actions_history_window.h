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

#ifndef RME_ACTIONS_HISTORY_WINDOW_H_
#define RME_ACTIONS_HISTORY_WINDOW_H_

#include "action.h"
#include "main.h"

class HistoryListBox : public wxVListBox
{
public:
	HistoryListBox(wxWindow* parent);

	void OnDrawItem(wxDC& dc, const wxRect& rect, size_t index) const override;
	wxCoord OnMeasureItem(size_t index) const override;

private:
	const wxBitmap& getIconBitmap(ActionIdentifier identifier) const;

	wxBitmap open_bitmap;
	wxBitmap move_bitmap;
	wxBitmap remote_bitmap;
	wxBitmap select_bitmap;
	wxBitmap unselect_bitmap;
	wxBitmap delete_bitmap;
	wxBitmap cut_bitmap;
	wxBitmap paste_bitmap;
	wxBitmap randomize_bitmap;
	wxBitmap borderize_bitmap;
	wxBitmap draw_bitmap;
	wxBitmap erase_bitmap;
	wxBitmap switch_bitmap;
	wxBitmap rotate_bitmap;
	wxBitmap replace_bitmap;
	wxBitmap change_bitmap;
};

class ActionsHistoryWindow : public wxPanel
{
public:
	ActionsHistoryWindow(wxWindow* parent);
	virtual ~ActionsHistoryWindow();

	void RefreshActions();

	void OnListSelected(wxCommandEvent& event);

protected:
	HistoryListBox* list;
};

#endif
