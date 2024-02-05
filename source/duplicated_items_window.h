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

#ifndef _RME_DUPLICATED_ITEMS_WINDOW_H_
#define _RME_DUPLICATED_ITEMS_WINDOW_H_

#include "main.h"

#include "position.h"

class MapTab;

class DuplicatedItemsWindow : public wxPanel
{
	struct DuplicatedItem {
		DuplicatedItem(const Position& position, uint16_t itemId, uint16_t count) :
			position(position), itemId(itemId), count(count) {}

		Position position;
		uint16_t itemId;
		uint16_t count;
	};

public:
	DuplicatedItemsWindow(wxWindow* parent);
	virtual ~DuplicatedItemsWindow();

	MapTab* GetMapTab() const noexcept { return map_tab; }

	void StartSearch(MapTab* tab, bool selection);
	void Clear();
	void UpdateButtons();

	void OnClickResult(wxCommandEvent&);
	void OnClickRemove(wxCommandEvent&);
	void OnClickRemoveAll(wxCommandEvent&);
	void OnClickExport(wxCommandEvent&);

protected:
	bool removeItem(DuplicatedItem* data, Action* action);

	MapTab* map_tab;
	wxListBox* items_list;
	wxButton* remove_button;
	wxButton* remove_all_button;
	wxBitmapButton* export_button;
};

#endif
