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

#ifndef RME_PALETTE_WAYPOINTS_H_
#define RME_PALETTE_WAYPOINTS_H_

#include <wx/listctrl.h>

#include "waypoints.h"
#include "palette_common.h"

class WaypointPalettePanel : public PalettePanel {
public:
	WaypointPalettePanel(wxWindow* parent, wxWindowID id = wxID_ANY);
	~WaypointPalettePanel();

	wxString GetName() const;
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
	// Called when this page is hidden
	void OnSwitchOut();
public:
	// wxWidgets event handling
	void OnClickWaypoint(wxListEvent& event);
	void OnBeginEditWaypointLabel(wxListEvent& event);
	void OnEditWaypointLabel(wxListEvent& event);
	void OnClickAddWaypoint(wxCommandEvent& event);
	void OnClickRemoveWaypoint(wxCommandEvent& event);

	void SetMap(Map* map);
protected:
	Map* map;
	wxListCtrl* waypoint_list;
	wxButton* add_waypoint_button;
	wxButton* remove_waypoint_button;

	DECLARE_EVENT_TABLE()
};

#endif
