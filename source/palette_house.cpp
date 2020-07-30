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

#include "palette_house.h"

#include "settings.h"

#include "brush.h"
#include "editor.h"
#include "map.h"

#include "application.h"
#include "map_display.h"

#include "house_brush.h"
#include "house_exit_brush.h"
#include "spawn_brush.h"

// ============================================================================
// House palette

BEGIN_EVENT_TABLE(HousePalettePanel, PalettePanel)
	EVT_TIMER(PALETTE_LAYOUT_FIX_TIMER, HousePalettePanel::OnLayoutFixTimer)

	EVT_CHOICE(PALETTE_HOUSE_TOWN_CHOICE, HousePalettePanel::OnTownChange)

	EVT_LISTBOX(PALETTE_HOUSE_LISTBOX, HousePalettePanel::OnListBoxChange)
	EVT_LISTBOX_DCLICK(PALETTE_HOUSE_LISTBOX, HousePalettePanel::OnListBoxDoubleClick)

	EVT_BUTTON(PALETTE_HOUSE_ADD_HOUSE, HousePalettePanel::OnClickAddHouse)
	EVT_BUTTON(PALETTE_HOUSE_EDIT_HOUSE, HousePalettePanel::OnClickEditHouse)
	EVT_BUTTON(PALETTE_HOUSE_REMOVE_HOUSE, HousePalettePanel::OnClickRemoveHouse)

	EVT_TOGGLEBUTTON(PALETTE_HOUSE_BRUSH_BUTTON, HousePalettePanel::OnClickHouseBrushButton)
	EVT_TOGGLEBUTTON(PALETTE_HOUSE_SELECT_EXIT_BUTTON, HousePalettePanel::OnClickSelectExitButton)
END_EVENT_TABLE()

HousePalettePanel::HousePalettePanel(wxWindow* parent, wxWindowID id) :
	PalettePanel(parent, id),
	map(nullptr),
	do_resize_on_display(true),
	fix_size_timer(this, PALETTE_LAYOUT_FIX_TIMER)
{
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* tmpsizer;

	wxSizer* sidesizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Houses");
	town_choice = newd wxChoice(this, PALETTE_HOUSE_TOWN_CHOICE, wxDefaultPosition, wxDefaultSize, (int)0, (const wxString*)nullptr);
	sidesizer->Add(town_choice, 0, wxEXPAND);

	house_list = newd SortableListBox(this, PALETTE_HOUSE_LISTBOX);
	#ifdef __APPLE__
	//Used for detecting a deselect
	house_list->Bind(wxEVT_LEFT_UP, &HousePalettePanel::OnListBoxClick, this);
	#endif
	sidesizer->Add(house_list, 1, wxEXPAND);

	tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	wxSizerFlags sizerFlags(1);
	tmpsizer->Add(add_house_button = newd wxButton(this, PALETTE_HOUSE_ADD_HOUSE, "Add", wxDefaultPosition, wxSize(50, -1)), sizerFlags);
	tmpsizer->Add(edit_house_button = newd wxButton(this, PALETTE_HOUSE_EDIT_HOUSE, "Edit", wxDefaultPosition, wxSize(50, -1)), sizerFlags);
	tmpsizer->Add(remove_house_button = newd wxButton(this, PALETTE_HOUSE_REMOVE_HOUSE, "Remove", wxDefaultPosition, wxSize(70, -1)), sizerFlags);
	sidesizer->Add(tmpsizer, wxSizerFlags(0).Right());

	topsizer->Add(sidesizer, 1, wxEXPAND);

	// Temple position
	sidesizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, "Brushes", wxDefaultPosition, wxSize(150, 200)), wxVERTICAL);

	//sidesizer->Add(180, 1, wxEXPAND);

	tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	house_brush_button = newd wxToggleButton(this, PALETTE_HOUSE_BRUSH_BUTTON, "House tiles");
	tmpsizer->Add(house_brush_button);
	sidesizer->Add(tmpsizer, wxSizerFlags(1).Center());

	tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	select_position_button = newd wxToggleButton(this, PALETTE_HOUSE_SELECT_EXIT_BUTTON, "Select Exit");
	tmpsizer->Add(select_position_button);
	sidesizer->Add(tmpsizer, wxSizerFlags(1).Center());

	topsizer->Add(sidesizer, 0, wxEXPAND);

	SetSizerAndFit(topsizer);
}

HousePalettePanel::~HousePalettePanel()
{
	////
}

void HousePalettePanel::SetMap(Map* m)
{
	g_gui.house_brush->setHouse(nullptr);
	map = m;
	OnUpdate();
}

void HousePalettePanel::OnSwitchIn()
{
	PalettePanel::OnSwitchIn();
	// Extremely ugly hack to fix layout issue
	if(do_resize_on_display) {
		fix_size_timer.Start(100, true);
		do_resize_on_display = false;
	}
}

void HousePalettePanel::OnLayoutFixTimer(wxTimerEvent& WXUNUSED(event))
{
	wxWindow* w = this;
	while((w = w->GetParent()) && dynamic_cast<PaletteWindow*>(w) == nullptr);

	if(w) {
		w->SetSize(w->GetSize().GetWidth(), w->GetSize().GetHeight() + 1);
		w->SetSize(w->GetSize().GetWidth(), w->GetSize().GetHeight() - 1);
	}
}

void HousePalettePanel::SelectFirstBrush()
{
	SelectHouseBrush();
}

Brush* HousePalettePanel::GetSelectedBrush() const
{
	if(select_position_button->GetValue()) {
		House* house = GetCurrentlySelectedHouse();
		if(house)
			g_gui.house_exit_brush->setHouse(house);
		return (g_gui.house_exit_brush->getHouseID() != 0? g_gui.house_exit_brush : nullptr);
	} else if(house_brush_button->GetValue()) {
		g_gui.house_brush->setHouse(GetCurrentlySelectedHouse());
		return (g_gui.house_brush->getHouseID() != 0? g_gui.house_brush : nullptr);
	}
	return nullptr;
}

bool HousePalettePanel::SelectBrush(const Brush* whatbrush)
{
	if(!whatbrush)
		return false;

	if(whatbrush->isHouse() && map) {
		const HouseBrush* house_brush = static_cast<const HouseBrush*>(whatbrush);
		for(HouseMap::iterator house_iter = map->houses.begin(); house_iter != map->houses.end(); ++house_iter) {
			if(house_iter->second->id == house_brush->getHouseID()) {
				for(uint32_t i = 0; i < town_choice->GetCount(); ++i) {
					Town* town = reinterpret_cast<Town*>(town_choice->GetClientData(i));
					// If it's "No Town" (nullptr) select it, or if it has the same town ID as the house
					if(town == nullptr || town->getID() == house_iter->second->townid) {
						SelectTown(i);
						for(uint32_t j = 0; j < house_list->GetCount(); ++j) {
							if(house_iter->second->id == reinterpret_cast<House*>(house_list->GetClientData(j))->id) {
								SelectHouse(j);
								return true;
							}
						}
						return true;
					}
				}
			}
		}
	} else if (whatbrush->isSpawn()) {
		SelectExitBrush();
	}
	return false;
}

int HousePalettePanel::GetSelectedBrushSize() const
{
	return 0;
}

PaletteType HousePalettePanel::GetType() const
{
	return TILESET_HOUSE;
}

void HousePalettePanel::SelectTown(size_t index)
{
	ASSERT(town_choice->GetCount() >= index);

	if(map == nullptr || town_choice->GetCount() == 0) {
		// No towns :(
		add_house_button->Enable(false);
	} else {
		Town* what_town = reinterpret_cast<Town*>(town_choice->GetClientData(index));

		// Clear the old houselist
		house_list->Clear();

		for(HouseMap::iterator house_iter = map->houses.begin(); house_iter != map->houses.end(); ++house_iter) {
			if(what_town) {
				if(house_iter->second->townid == what_town->getID()) {
					house_list->Append(wxstr(house_iter->second->getDescription()), house_iter->second);
				}
			} else {
				// "No Town" selected!
				if(map->towns.getTown(house_iter->second->townid) == nullptr) {
					// The town doesn't exist
					house_list->Append(wxstr(house_iter->second->getDescription()), house_iter->second);
				}
			}
		}
		house_list->Sort();

		// Select first house
		SelectHouse(0);
		town_choice->SetSelection(index);
		add_house_button->Enable(what_town != nullptr);
		ASSERT(what_town == nullptr || add_house_button->IsEnabled() || !IsEnabled());
	}
}

void HousePalettePanel::SelectHouse(size_t index)
{
	ASSERT(house_list->GetCount() >= index);

	if(house_list->GetCount() > 0) {
		edit_house_button->Enable(true);
		remove_house_button->Enable(true);
		select_position_button->Enable(true);
		house_brush_button->Enable(true);
		// Select the house
		house_list->SetSelection(index);
		SelectHouseBrush();
	} else {
		// No houses :(
		edit_house_button->Enable(false);
		remove_house_button->Enable(false);
		select_position_button->Enable(false);
		house_brush_button->Enable(false);
	}

	SelectHouseBrush();
	g_gui.RefreshView();
}

House* HousePalettePanel::GetCurrentlySelectedHouse() const
{
	int selection = house_list->GetSelection();
	if(house_list->GetCount() > 0 && selection != wxNOT_FOUND) {
		return reinterpret_cast<House*>(house_list->GetClientData(selection));
	}
	return nullptr;
}

void HousePalettePanel::SelectHouseBrush()
{
	if(house_list->GetCount() > 0) {
		house_brush_button->SetValue(true);
		select_position_button->SetValue(false);
	} else {
		house_brush_button->SetValue(false);
		select_position_button->SetValue(false);
	}
}

void HousePalettePanel::SelectExitBrush()
{
	if(house_list->GetCount() > 0) {
		house_brush_button->SetValue(false);
		select_position_button->SetValue(true);
	}
}

void HousePalettePanel::OnUpdate()
{
	int old_town_selection = town_choice->GetSelection();

	town_choice->Clear();
	house_list->Clear();

	if(map == nullptr)
		return;

	if(map->towns.count() != 0) {
		// Create choice control
		for(TownMap::iterator town_iter = map->towns.begin(); town_iter != map->towns.end(); ++town_iter) {
			town_choice->Append(wxstr(town_iter->second->getName()), town_iter->second);
		}
		town_choice->Append("No Town", (void*)(nullptr));
		if(old_town_selection <= 0)
			SelectTown(0);
		else if((size_t)old_town_selection <= town_choice->GetCount())
			SelectTown(old_town_selection);
		else
			SelectTown(old_town_selection-1);

		house_list->Enable(true);
	} else {
		town_choice->Append("No Town", (void*)(nullptr));
		select_position_button->Enable(false);
		select_position_button->SetValue(false);
		house_brush_button->Enable(false);
		house_brush_button->SetValue(false);
		add_house_button->Enable(false);
		edit_house_button->Enable(false);
		remove_house_button->Enable(false);

		SelectTown(0);
	}
}

void HousePalettePanel::OnTownChange(wxCommandEvent& event)
{
	SelectTown(event.GetSelection());
	g_gui.SelectBrush();
}

void HousePalettePanel::OnListBoxChange(wxCommandEvent& event)
{
	SelectHouse(event.GetSelection());
	g_gui.SelectBrush();
}

void HousePalettePanel::OnListBoxDoubleClick(wxCommandEvent& event)
{
	House* house = reinterpret_cast<House*>(event.GetClientData());
	// I find it extremly unlikely that one actually wants the exit at 0,0,0, so just treat it as the null value
	if(house && house->getExit() != Position(0,0,0)) {
		g_gui.SetScreenCenterPosition(house->getExit());
	}
}

void HousePalettePanel::OnClickHouseBrushButton(wxCommandEvent& event)
{
	SelectHouseBrush();
	g_gui.SelectBrush();
}

void HousePalettePanel::OnClickSelectExitButton(wxCommandEvent& event)
{
	SelectExitBrush();
	g_gui.SelectBrush();
}

void HousePalettePanel::OnClickAddHouse(wxCommandEvent& event)
{
	if(map == nullptr)
		return;

	House* new_house = newd House(*map);
	new_house->id = map->houses.getEmptyID();

	std::ostringstream os;
	os << "Unnamed House #" << new_house->id;
	new_house->name = os.str();
	Town* town = reinterpret_cast<Town*>(town_choice->GetClientData(town_choice->GetSelection()));

	ASSERT(town);
	new_house->townid = town->getID();

	map->houses.addHouse(new_house);
	house_list->Append(wxstr(new_house->getDescription()), new_house);
	SelectHouse(house_list->FindString(wxstr(new_house->getDescription())));
	g_gui.SelectBrush();
	refresh_timer.Start(300, true);
}

void HousePalettePanel::OnClickEditHouse(wxCommandEvent& event)
{
	if(house_list->GetCount() == 0)
		return;
	if(map == nullptr)
		return;
	int selection = house_list->GetSelection();
	House* house = reinterpret_cast<House*>(house_list->GetClientData(selection));
	if(house) {
		wxDialog* d = newd EditHouseDialog(g_gui.root, map, house);
		int ret = d->ShowModal();
		if(ret == 1) {
			// Something changed, change name of house
			house_list->SetString(selection, wxstr(house->getDescription()));
			house_list->Sort();
			refresh_timer.Start(300, true);
		}
	}
}

void HousePalettePanel::OnClickRemoveHouse(wxCommandEvent& event)
{
	int selection = house_list->GetSelection();
	if(selection != wxNOT_FOUND) {
		House* house = reinterpret_cast<House*>(house_list->GetClientData(selection));
		map->houses.removeHouse(house);
		house_list->Delete(selection);
		refresh_timer.Start(300, true);

		if(int(house_list->GetCount()) <= selection) {
			selection -= 1;
		}

		if(selection >= 0 && house_list->GetCount()) {
			house_list->SetSelection(selection);
		} else {
			select_position_button->Enable(false);
			select_position_button->SetValue(false);
			house_brush_button->Enable(false);
			house_brush_button->SetValue(false);
			edit_house_button->Enable(false);
			remove_house_button->Enable(false);
		}
		g_gui.SelectBrush();
	}
	g_gui.RefreshView();
}

#ifdef __APPLE__
//On wxMac it is possible to deselect a wxListBox. (Unlike on the other platforms)
//EVT_LISTBOX is not triggered when the deselection is happening. http://trac.wxwidgets.org/ticket/15603
//Here we find out if the listbox was deselected using a normal mouse up event so we know when to disable the buttons and brushes.
void HousePalettePanel::OnListBoxClick(wxMouseEvent& event) {
	if (house_list->GetSelection() == wxNOT_FOUND) {
		select_position_button->Enable(false);
		select_position_button->SetValue(false);
		house_brush_button->Enable(false);
		house_brush_button->SetValue(false);
		edit_house_button->Enable(false);
		remove_house_button->Enable(false);
		g_gui.SelectBrush();
	}
}
#endif

// ============================================================================
// House Edit Dialog

BEGIN_EVENT_TABLE(EditHouseDialog, wxDialog)
	EVT_BUTTON(wxID_OK, EditHouseDialog::OnClickOK)
	EVT_BUTTON(wxID_CANCEL, EditHouseDialog::OnClickCancel)
END_EVENT_TABLE()

EditHouseDialog::EditHouseDialog(wxWindow* parent, Map* map, House* house) :
	wxDialog(parent, wxID_ANY, "House", wxDefaultPosition, wxSize(250,160)),
	map(map),
	what_house(house)
{
	ASSERT(map);
	ASSERT(house);

	// Create topsizer
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* tmpsizer;

	house_name = wxstr(house->name);
	house_id = i2ws(house->id);
	house_rent = i2ws(house->rent);

	// House options
	tmpsizer = newd wxStaticBoxSizer(wxHORIZONTAL, this, "Name");
	name_field = newd wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(230,20), 0, wxTextValidator(wxFILTER_ASCII, &house_name));
	tmpsizer->Add(name_field);

	sizer->Add(tmpsizer, wxSizerFlags().Border(wxALL, 20));

	tmpsizer = newd wxStaticBoxSizer(wxHORIZONTAL, this, "Rent / ID");
	rent_field = newd wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(160,20), 0, wxTextValidator(wxFILTER_NUMERIC, &house_rent));
	tmpsizer->Add(rent_field);
	id_field = newd wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxSize(70,20), 0, wxTextValidator(wxFILTER_NUMERIC, &house_id));
	id_field->Enable(false);
	tmpsizer->Add(id_field);
	sizer->Add(tmpsizer, wxSizerFlags().Border(wxALL, 20));

	// House options
	guildhall_field = newd wxCheckBox(this, wxID_ANY, "Guildhall", wxDefaultPosition);

	sizer->Add(guildhall_field, wxSizerFlags().Border(wxRIGHT | wxLEFT | wxBOTTOM, 20));
	guildhall_field->SetValue(house->guildhall);

	// OK/Cancel buttons
	tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	tmpsizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center());
	tmpsizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Center());
	sizer->Add(tmpsizer, wxSizerFlags(1).Center().Border(wxRIGHT | wxLEFT | wxBOTTOM, 20));

	SetSizerAndFit(sizer);
}

EditHouseDialog::~EditHouseDialog()
{
	////
}

void EditHouseDialog::OnClickOK(wxCommandEvent& WXUNUSED(event))
{
	if(Validate() && TransferDataFromWindow()) {
		// Verify the newd information
		long new_house_rent;
		house_rent.ToLong(&new_house_rent);

		if(new_house_rent < 0) {
			g_gui.PopupDialog(this, "Error", "House rent cannot be less than 0.", wxOK);
			return;
		}

		if(house_name.length() == 0) {
			g_gui.PopupDialog(this, "Error", "House name cannot be nil.", wxOK);
			return;
		}

		if(g_settings.getInteger(Config::WARN_FOR_DUPLICATE_ID)) {
			Houses& houses = map->houses;
			for(HouseMap::const_iterator house_iter = houses.begin(); house_iter != houses.end(); ++house_iter) {
				House* house = house_iter->second;
				ASSERT(house);
				if(wxstr(house->name) == house_name && house->id != what_house->id) {
					int ret = g_gui.PopupDialog(this, "Warning", "This house name is already in use, are you sure you want to continue?", wxYES | wxNO);
					if(ret == wxID_NO) {
						return;
					}
				}
			}
		}

		// Transfer to house
		what_house->name = nstr(house_name);
		what_house->rent = new_house_rent;
		what_house->guildhall = guildhall_field->GetValue();

		EndModal(1);
	}
}

void EditHouseDialog::OnClickCancel(wxCommandEvent& WXUNUSED(event))
{
	// Just close this window
	EndModal(0);
}
