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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/palette_house.h $
// $Id: palette_house.h 298 2010-02-23 17:09:13Z admin $

#ifndef RME_PALETTE_HOUSE_H_
#define RME_PALETTE_HOUSE_H_

#include "palette_common.h"

class House;

class HousePalettePanel : public PalettePanel {
public:
	HousePalettePanel(wxWindow* parent, wxWindowID id = wxID_ANY);
	~HousePalettePanel();
	
	PaletteType GetType() const;

	// Select the first brush
	void SelectFirstBrush();
	// Returns the currently selected brush (first brush if panel is not loaded)
	Brush* GetSelectedBrush() const;
	// Returns the currently selected brush size
	int GetSelectedBrushSize() const;
	// Select the brush in the parameter, this only changes the look of the panel
	bool SelectBrush(const Brush* whatbrush);

	// Called sometimes?
	void OnUpdate();
	// Called when this page is about to be displayed
	void OnSwitchIn();

	void OnLayoutFixTimer(wxTimerEvent& event);

	void SetMap(Map* map);
protected:
	// Internal use
	void SaveHouse();
	void SelectTown(size_t index);
	void SelectHouse(size_t index);

	House* GetCurrentlySelectedHouse() const;

	void SelectHouseBrush();
	void SelectExitBrush();
public:
	// wxWidgets event handling
	void OnTownChange(wxCommandEvent& event);
	void OnListBoxChange(wxCommandEvent& event);
	void OnListBoxDoubleClick(wxCommandEvent& event);
	void OnClickHouseBrushButton(wxCommandEvent& event);
	void OnClickSelectExitButton(wxCommandEvent& event);
	void OnClickAddHouse(wxCommandEvent& event);
	void OnClickEditHouse(wxCommandEvent& event);
	void OnClickRemoveHouse(wxCommandEvent& event);

protected:
	Map* map;
	wxChoice* town_choice;
	wxListBox* house_list;
	wxToggleButton* house_brush_button;
	wxToggleButton* select_position_button;
	wxButton* add_house_button;
	wxButton* edit_house_button;
	wxButton* remove_house_button;

	// Used for ugly hack
	bool do_resize_on_display;
	wxTimer fix_size_timer;

	DECLARE_EVENT_TABLE()
};

class EditHouseDialog : public wxDialog
{
public:
	EditHouseDialog(wxWindow* parent, Map* map, House* house);
	virtual ~EditHouseDialog();

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);
protected:
	Map* map;
	House* what_house;

	wxString house_name, house_id, house_rent;
	
	wxTextCtrl* name_field;
	wxTextCtrl* id_field;
	wxTextCtrl* rent_field;
	wxCheckBox* guildhall_field;

	DECLARE_EVENT_TABLE();
};

#endif
