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

#ifndef RME_FIND_ITEM_WINDOW_H_
#define RME_FIND_ITEM_WINDOW_H_

#include <wx/radiobox.h>
#include <wx/spinctrl.h>
#include <wx/sizer.h>
#include <wx/statbox.h>
#include <wx/textctrl.h>
#include <wx/checkbox.h>
#include <wx/button.h>
#include <wx/dialog.h>

class FindDialogListBox;

class FindItemDialog : public wxDialog
{
public:
	enum SearchMode {
		ServerIDs = 0,
		ClientIDs,
		Names,
		Types,
		Properties,
	};

	enum SearchItemType {
		Depot,
		Mailbox,
		TrashHolder,
		Container,
		Door,
		MagicField,
		Teleport,
		Bed,
		Key,
		Podium
	};

	FindItemDialog(wxWindow* parent, const wxString& title, bool onlyPickupables = false);
	~FindItemDialog();

	Brush* getResult() const { return result_brush; }
	uint16_t getResultID() const { return result_id; }

	SearchMode getSearchMode() const;
	void setSearchMode(SearchMode mode);

private:
	void EnableProperties(bool enable);
	void RefreshContentsInternal();

	void OnOptionChange(wxCommandEvent& event);
	void OnServerIdChange(wxCommandEvent& event);
	void OnClientIdChange(wxCommandEvent& event);
	void OnText(wxCommandEvent& event);
	void OnTypeChange(wxCommandEvent& event);
	void OnPropertyChange(wxCommandEvent& event);
	void OnInputTimer(wxTimerEvent& event);
	void OnClickOK(wxCommandEvent& event);
	void OnClickCancel(wxCommandEvent& event);

	wxRadioBox* options_radio_box;

	wxRadioBox* types_radio_box;

	wxSpinCtrl* server_id_spin;
	wxSpinCtrl* client_id_spin;
	wxTextCtrl* name_text_input;
	wxTimer input_timer;
	wxCheckBox* unpassable;
	wxCheckBox* unmovable;
	wxCheckBox* block_missiles;
	wxCheckBox* block_pathfinder;
	wxCheckBox* readable;
	wxCheckBox* writeable;
	wxCheckBox* pickupable;
	wxCheckBox* stackable;
	wxCheckBox* rotatable;
	wxCheckBox* hangable;
	wxCheckBox* hook_east;
	wxCheckBox* hook_south;
	wxCheckBox* has_elevation;
	wxCheckBox* ignore_look;
	wxCheckBox* floor_change;
	wxCheckBox* invalid_item;

	FindDialogListBox* items_list;
	wxStdDialogButtonSizer* buttons_box_sizer;
	wxButton* ok_button;
	wxButton* cancel_button;
	Brush* result_brush;
	uint16_t result_id;
	bool only_pickupables;

	DECLARE_EVENT_TABLE()
};

#endif // RME_FIND_ITEM_WINDOW_H_
