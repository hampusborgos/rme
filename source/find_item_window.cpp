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
#include "find_item_window.h"
#include "common_windows.h"
#include "gui.h"
#include "items.h"
#include "brush.h"
#include "raw_brush.h"

BEGIN_EVENT_TABLE(FindItemDialog, wxDialog)
	EVT_TIMER(wxID_ANY, FindItemDialog::OnInputTimer)
	EVT_BUTTON(wxID_OK, FindItemDialog::OnClickOK)
	EVT_BUTTON(wxID_CANCEL, FindItemDialog::OnClickCancel)
END_EVENT_TABLE()

FindItemDialog::FindItemDialog(wxWindow* parent, const wxString& title, bool onlyPickupables/* = false*/) :
	wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxSize(800, 600), wxDEFAULT_DIALOG_STYLE),
	input_timer(this),
	result_brush(nullptr),
	result_id(0),
	only_pickupables(onlyPickupables)
{
	this->SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* box_sizer = newd wxBoxSizer(wxHORIZONTAL);

	wxBoxSizer* options_box_sizer = newd wxBoxSizer(wxVERTICAL);

	wxString radio_boxChoices[] = { "Find by Server ID",
									"Find by Client ID",
									"Find by Name",
									"Find by Types",
									"Find by Properties" };

	int radio_boxNChoices = sizeof(radio_boxChoices) / sizeof(wxString);
	options_radio_box = newd wxRadioBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, radio_boxNChoices, radio_boxChoices, 1, wxRA_SPECIFY_COLS);
	options_radio_box->SetSelection(SearchMode::ServerIDs);
	options_box_sizer->Add(options_radio_box, 0, wxALL | wxEXPAND, 5);

	wxStaticBoxSizer* server_id_box_sizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, "Server ID"), wxVERTICAL);
	server_id_spin = newd wxSpinCtrl(server_id_box_sizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 100, g_items.getMaxID(), 100);
	server_id_box_sizer->Add(server_id_spin, 0, wxALL | wxEXPAND, 5);

	invalid_item = newd wxCheckBox(server_id_box_sizer->GetStaticBox(), wxID_ANY, "Force select", wxDefaultPosition, wxDefaultSize, 0);
	invalid_item->SetToolTip("Force choose item ID that does not appear on the list.");
	server_id_box_sizer->Add(invalid_item, 1, wxALL | wxEXPAND, 5);

	options_box_sizer->Add(server_id_box_sizer, 1, wxALL | wxEXPAND, 5);

	wxStaticBoxSizer* client_id_box_sizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, "Client ID"), wxVERTICAL);
	client_id_spin = newd wxSpinCtrl(client_id_box_sizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 100, g_gui.gfx.getItemSpriteMaxID(), 100);
	client_id_spin->Enable(false);
	client_id_box_sizer->Add(client_id_spin, 0, wxALL | wxEXPAND, 5);
	options_box_sizer->Add(client_id_box_sizer, 1, wxALL | wxEXPAND, 5);

	wxStaticBoxSizer* name_box_sizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, "Name"), wxVERTICAL);
	name_text_input = newd wxTextCtrl(name_box_sizer->GetStaticBox(), wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, 0);
	name_text_input->Enable(false);
	name_box_sizer->Add(name_text_input, 0, wxALL | wxEXPAND, 5);
	options_box_sizer->Add(name_box_sizer, 1, wxALL | wxEXPAND, 5);

	// spacer
	options_box_sizer->Add(0, 0, 4, wxALL | wxEXPAND, 5);

	buttons_box_sizer = newd wxStdDialogButtonSizer();
	ok_button = newd wxButton(this, wxID_OK);
	buttons_box_sizer->AddButton(ok_button);
	cancel_button = newd wxButton(this, wxID_CANCEL);
	buttons_box_sizer->AddButton(cancel_button);
	buttons_box_sizer->Realize();
	options_box_sizer->Add(buttons_box_sizer, 0, wxALIGN_CENTER | wxALL, 5);

	box_sizer->Add(options_box_sizer, 1, wxALL, 5);

	// --------------- Types ---------------

	wxStaticBoxSizer* type_box_sizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, "Types"), wxVERTICAL);

	wxString types_choices[] = { "Depot",
								 "Mailbox",
								 "Trash Holder",
								 "Container",
								 "Door",
								 "Magic Field",
								 "Teleport",
								 "Bed",
								 "Key",
								 "Podium" };

	int types_choices_count = sizeof(types_choices) / sizeof(wxString);
	types_radio_box = newd wxRadioBox(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxDefaultSize, types_choices_count, types_choices, 1, wxRA_SPECIFY_COLS);
	types_radio_box->SetSelection(0);
	types_radio_box->Enable(false);
	type_box_sizer->Add(types_radio_box, 0, wxALL | wxEXPAND, 5);

	box_sizer->Add(type_box_sizer, 1, wxALL | wxEXPAND, 5);

	// --------------- Properties ---------------

	wxStaticBoxSizer* properties_box_sizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, "Properties"), wxVERTICAL);

	unpassable = newd wxCheckBox(properties_box_sizer->GetStaticBox(), wxID_ANY, "Unpassable", wxDefaultPosition, wxDefaultSize, 0);
	properties_box_sizer->Add(unpassable, 0, wxALL, 5);

	unmovable = newd wxCheckBox(properties_box_sizer->GetStaticBox(), wxID_ANY, "Unmovable", wxDefaultPosition, wxDefaultSize, 0);
	properties_box_sizer->Add(unmovable, 0, wxALL, 5);

	block_missiles = newd wxCheckBox(properties_box_sizer->GetStaticBox(), wxID_ANY, "Block Missiles", wxDefaultPosition, wxDefaultSize, 0);
	properties_box_sizer->Add(block_missiles, 0, wxALL, 5);

	block_pathfinder = newd wxCheckBox(properties_box_sizer->GetStaticBox(), wxID_ANY, "Block Pathfinder", wxDefaultPosition, wxDefaultSize, 0);
	properties_box_sizer->Add(block_pathfinder, 0, wxALL, 5);

	readable = newd wxCheckBox(properties_box_sizer->GetStaticBox(), wxID_ANY, "Readable", wxDefaultPosition, wxDefaultSize, 0);
	properties_box_sizer->Add(readable, 0, wxALL, 5);

	writeable = newd wxCheckBox(properties_box_sizer->GetStaticBox(), wxID_ANY, "Writeable", wxDefaultPosition, wxDefaultSize, 0);
	properties_box_sizer->Add(writeable, 0, wxALL, 5);

	pickupable = newd wxCheckBox(properties_box_sizer->GetStaticBox(), wxID_ANY, "Pickupable", wxDefaultPosition, wxDefaultSize, 0);
	pickupable->SetValue(only_pickupables);
	pickupable->Enable(!only_pickupables);
	properties_box_sizer->Add(pickupable, 0, wxALL, 5);

	stackable = newd wxCheckBox(properties_box_sizer->GetStaticBox(), wxID_ANY, "Stackable", wxDefaultPosition, wxDefaultSize, 0);
	properties_box_sizer->Add(stackable, 0, wxALL, 5);

	rotatable = newd wxCheckBox(properties_box_sizer->GetStaticBox(), wxID_ANY, "Rotatable", wxDefaultPosition, wxDefaultSize, 0);
	properties_box_sizer->Add(rotatable, 0, wxALL, 5);

	hangable = newd wxCheckBox(properties_box_sizer->GetStaticBox(), wxID_ANY, "Hangable", wxDefaultPosition, wxDefaultSize, 0);
	properties_box_sizer->Add(hangable, 0, wxALL, 5);

	hook_east = newd wxCheckBox(properties_box_sizer->GetStaticBox(), wxID_ANY, "Hook East", wxDefaultPosition, wxDefaultSize, 0);
	properties_box_sizer->Add(hook_east, 0, wxALL, 5);

	hook_south = newd wxCheckBox(properties_box_sizer->GetStaticBox(), wxID_ANY, "Hook South", wxDefaultPosition, wxDefaultSize, 0);
	properties_box_sizer->Add(hook_south, 0, wxALL, 5);

	has_elevation = newd wxCheckBox(properties_box_sizer->GetStaticBox(), wxID_ANY, "Has Elevation", wxDefaultPosition, wxDefaultSize, 0);
	properties_box_sizer->Add(has_elevation, 0, wxALL, 5);

	ignore_look = newd wxCheckBox(properties_box_sizer->GetStaticBox(), wxID_ANY, "Ignore Look", wxDefaultPosition, wxDefaultSize, 0);
	properties_box_sizer->Add(ignore_look, 0, wxALL, 5);

	floor_change = newd wxCheckBox(properties_box_sizer->GetStaticBox(), wxID_ANY, "Floor Change", wxDefaultPosition, wxDefaultSize, 0);
	properties_box_sizer->Add(floor_change, 0, wxALL, 5);

	box_sizer->Add(properties_box_sizer, 1, wxALL | wxEXPAND, 5);

	// --------------- Items list ---------------

	wxStaticBoxSizer* result_box_sizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, "Result"), wxVERTICAL);
	items_list = newd FindDialogListBox(result_box_sizer->GetStaticBox(), wxID_ANY);
	items_list->SetMinSize(wxSize(230, 512));
	result_box_sizer->Add(items_list, 0, wxALL, 5);
	box_sizer->Add(result_box_sizer, 1, wxALL | wxEXPAND, 5);

	this->SetSizer(box_sizer);
	this->Layout();
	this->Centre(wxBOTH);
	this->EnableProperties(false);
	this->RefreshContentsInternal();

	// Connect Events
	options_radio_box->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnOptionChange), NULL, this);
	server_id_spin->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(FindItemDialog::OnServerIdChange), NULL, this);
	server_id_spin->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(FindItemDialog::OnServerIdChange), NULL, this);
	client_id_spin->Connect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(FindItemDialog::OnClientIdChange), NULL, this);
	client_id_spin->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(FindItemDialog::OnClientIdChange), NULL, this);
	name_text_input->Connect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(FindItemDialog::OnText), NULL, this);

	types_radio_box->Connect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnTypeChange), NULL, this);

	unpassable->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	unmovable->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	block_missiles->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	block_pathfinder->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	readable->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	writeable->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	pickupable->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	stackable->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	rotatable->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	hangable->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	hook_east->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	hook_south->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	has_elevation->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	ignore_look->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	floor_change->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	invalid_item->Connect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
}

FindItemDialog::~FindItemDialog()
{
	// Disconnect Events
	options_radio_box->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnOptionChange), NULL, this);
	server_id_spin->Disconnect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(FindItemDialog::OnServerIdChange), NULL, this);
	server_id_spin->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(FindItemDialog::OnServerIdChange), NULL, this);
	client_id_spin->Disconnect(wxEVT_COMMAND_SPINCTRL_UPDATED, wxCommandEventHandler(FindItemDialog::OnClientIdChange), NULL, this);
	client_id_spin->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(FindItemDialog::OnClientIdChange), NULL, this);
	name_text_input->Disconnect(wxEVT_COMMAND_TEXT_UPDATED, wxCommandEventHandler(FindItemDialog::OnText), NULL, this);

	types_radio_box->Disconnect(wxEVT_COMMAND_RADIOBOX_SELECTED, wxCommandEventHandler(FindItemDialog::OnTypeChange), NULL, this);

	unpassable->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	unmovable->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	block_missiles->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	block_pathfinder->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	readable->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	writeable->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	pickupable->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	stackable->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	rotatable->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	hangable->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	hook_east->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	hook_south->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	has_elevation->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	ignore_look->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
	floor_change->Disconnect(wxEVT_COMMAND_CHECKBOX_CLICKED, wxCommandEventHandler(FindItemDialog::OnPropertyChange), NULL, this);
}

FindItemDialog::SearchMode FindItemDialog::getSearchMode() const
{
	return (SearchMode)options_radio_box->GetSelection();
}

void FindItemDialog::setSearchMode(FindItemDialog::SearchMode mode)
{
	if((SearchMode)options_radio_box->GetSelection() != mode)
		options_radio_box->SetSelection(mode);

	server_id_spin->Enable(mode == SearchMode::ServerIDs);
	invalid_item->Enable(mode == SearchMode::ServerIDs);
	client_id_spin->Enable(mode == SearchMode::ClientIDs);
	name_text_input->Enable(mode == SearchMode::Names);
	types_radio_box->Enable(mode == SearchMode::Types);
	EnableProperties(mode == SearchMode::Properties);
	RefreshContentsInternal();

	if(mode == SearchMode::ServerIDs) {
		server_id_spin->SetFocus();
		server_id_spin->SetSelection(-1, -1);
	} else if(mode == SearchMode::ClientIDs) {
		client_id_spin->SetFocus();
		client_id_spin->SetSelection(-1, -1);
	} else if(mode == SearchMode::Names) {
		name_text_input->SetFocus();
	}
}

void FindItemDialog::EnableProperties(bool enable)
{
	unpassable->Enable(enable);
	unmovable->Enable(enable);
	block_missiles->Enable(enable);
	block_pathfinder->Enable(enable);
	readable->Enable(enable);
	writeable->Enable(enable);
	pickupable->Enable(!only_pickupables && enable);
	stackable->Enable(enable);
	rotatable->Enable(enable);
	hangable->Enable(enable);
	hook_east->Enable(enable);
	hook_south->Enable(enable);
	has_elevation->Enable(enable);
	ignore_look->Enable(enable);
	floor_change->Enable(enable);
}

void FindItemDialog::RefreshContentsInternal()
{
	items_list->Clear();
	ok_button->Enable(false);

	SearchMode selection = (SearchMode)options_radio_box->GetSelection();
	bool found_search_results = false;

	if(selection == SearchMode::ServerIDs) {
		result_id = std::min(server_id_spin->GetValue(), 0xFFFF);
		uint16_t serverID = static_cast<uint16_t>(result_id);
		if (serverID <= g_items.getMaxID()) {
			ItemType& item = g_items.getItemType(serverID);
			RAWBrush* raw_brush = item.raw_brush;
			if (raw_brush) {
				if (only_pickupables) {
					if (item.pickupable) {
						found_search_results = true;
						items_list->AddBrush(raw_brush);
					}
				} else {
					found_search_results = true;
					items_list->AddBrush(raw_brush);
				}
			}
		}

		if (invalid_item->GetValue()) {
			found_search_results = true;
		}
	} else if(selection == SearchMode::ClientIDs) {
		uint16_t clientID = (uint16_t)client_id_spin->GetValue();
		for (int id = 100; id <= g_items.getMaxID(); ++id) {
			ItemType& item = g_items.getItemType(id);
			if (item.id == 0 || item.clientID != clientID)
				continue;

			RAWBrush* raw_brush = item.raw_brush;
			if (!raw_brush)
				continue;

			if(only_pickupables && !item.pickupable)
				continue;

			found_search_results = true;
			items_list->AddBrush(raw_brush);
		}
	} else if(selection == SearchMode::Names) {
		std::string search_string = as_lower_str(nstr(name_text_input->GetValue()));
		if(search_string.size() >= 2) {
			for(int id = 100; id <= g_items.getMaxID(); ++id) {
				ItemType& item = g_items.getItemType(id);
				if(item.id == 0)
					continue;

				RAWBrush* raw_brush = item.raw_brush;
				if(!raw_brush)
					continue;

				if(only_pickupables && !item.pickupable)
					continue;

				if(as_lower_str(raw_brush->getName()).find(search_string) == std::string::npos)
					continue;

				found_search_results = true;
				items_list->AddBrush(raw_brush);
			}
		}
	} else if(selection == SearchMode::Types) {
		for(int id = 100; id <= g_items.getMaxID(); ++id) {
			ItemType& item = g_items.getItemType(id);
			if (item.id == 0)
				continue;

			RAWBrush* raw_brush = item.raw_brush;
			if(!raw_brush)
				continue;

			if(only_pickupables && !item.pickupable)
				continue;

			SearchItemType selection = (SearchItemType)types_radio_box->GetSelection();
			if ((selection == SearchItemType::Depot && !item.isDepot()) ||
				(selection == SearchItemType::Mailbox && !item.isMailbox()) ||
				(selection == SearchItemType::TrashHolder && !item.isTrashHolder()) ||
				(selection == SearchItemType::Container && !item.isContainer()) ||
				(selection == SearchItemType::Door && !item.isDoor()) ||
				(selection == SearchItemType::MagicField && !item.isMagicField()) ||
				(selection == SearchItemType::Teleport && !item.isTeleport()) ||
				(selection == SearchItemType::Bed && !item.isBed()) ||
				(selection == SearchItemType::Key && !item.isKey()) ||
				(selection == SearchItemType::Podium && !item.isPodium())) {
				continue;
			}

			found_search_results = true;
			items_list->AddBrush(raw_brush);
		}
	} else if(selection == SearchMode::Properties) {
		bool has_selected = (unpassable->GetValue() ||
			unmovable->GetValue() ||
			block_missiles->GetValue() ||
			block_pathfinder->GetValue() ||
			readable->GetValue() ||
			writeable->GetValue() ||
			pickupable->GetValue() ||
			stackable->GetValue() ||
			rotatable->GetValue() ||
			hangable->GetValue() ||
			hook_east->GetValue() ||
			hook_south->GetValue() ||
			has_elevation->GetValue() ||
			ignore_look->GetValue() ||
			floor_change->GetValue());

		if(has_selected) {
			for(int id = 100; id <= g_items.getMaxID(); ++id) {
				ItemType& item = g_items.getItemType(id);
				if(item.id == 0)
					continue;

				RAWBrush* raw_brush = item.raw_brush;
				if(!raw_brush)
					continue;

				if((unpassable->GetValue() && !item.unpassable) ||
					(unmovable->GetValue() && item.moveable) ||
					(block_missiles->GetValue() && !item.blockMissiles) ||
					(block_pathfinder->GetValue() && !item.blockPathfinder) ||
					(readable->GetValue() && !item.canReadText) ||
					(writeable->GetValue() && !item.canWriteText) ||
					(pickupable->GetValue() && !item.pickupable) ||
					(stackable->GetValue() && !item.stackable) ||
					(rotatable->GetValue() && !item.rotable) ||
					(hangable->GetValue() && !item.isHangable) ||
					(hook_east->GetValue() && !item.hookEast) ||
					(hook_south->GetValue() && !item.hookSouth) ||
					(has_elevation->GetValue() && !item.hasElevation) ||
					(ignore_look->GetValue() && !item.ignoreLook) ||
					(floor_change->GetValue() && !item.isFloorChange())) {
					continue;
				}

				found_search_results = true;
				items_list->AddBrush(raw_brush);
			}
		}
	}

	if(found_search_results) {
		items_list->SetSelection(0);
		ok_button->Enable(true);
	} else
		items_list->SetNoMatches();

	items_list->Refresh();
}

void FindItemDialog::OnOptionChange(wxCommandEvent& WXUNUSED(event))
{
	setSearchMode((SearchMode)options_radio_box->GetSelection());
}

void FindItemDialog::OnServerIdChange(wxCommandEvent& WXUNUSED(event))
{
	RefreshContentsInternal();
}

void FindItemDialog::OnClientIdChange(wxCommandEvent& WXUNUSED(event))
{
	RefreshContentsInternal();
}

void FindItemDialog::OnText(wxCommandEvent& WXUNUSED(event))
{
	input_timer.Start(800, true);
}

void FindItemDialog::OnTypeChange(wxCommandEvent& WXUNUSED(event))
{
	RefreshContentsInternal();
}

void FindItemDialog::OnPropertyChange(wxCommandEvent& WXUNUSED(event))
{
	RefreshContentsInternal();
}

void FindItemDialog::OnInputTimer(wxTimerEvent& WXUNUSED(event))
{
	RefreshContentsInternal();
}

void FindItemDialog::OnClickOK(wxCommandEvent& WXUNUSED(event))
{
	if (invalid_item->GetValue() && (SearchMode)options_radio_box->GetSelection() == SearchMode::ServerIDs && result_id != 0) {
		EndModal(wxID_OK);
		return;
	}

	if(items_list->GetItemCount() != 0) {
		Brush* brush = items_list->GetSelectedBrush();
		if(brush) {
			result_brush = brush;
			result_id = brush->asRaw()->getItemID();
			EndModal(wxID_OK);
		}
	}
}

void FindItemDialog::OnClickCancel(wxCommandEvent& WXUNUSED(event))
{
	EndModal(wxID_CANCEL);
}
