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

// ============================================================================
// Waypoint palette

#include "main.h"

#include "gui.h"
#include "palette_waypoints.h"
#include "waypoint_brush.h"
#include "map.h"

BEGIN_EVENT_TABLE(WaypointPalettePanel, PalettePanel)
	EVT_BUTTON(PALETTE_WAYPOINT_ADD_WAYPOINT, WaypointPalettePanel::OnClickAddWaypoint)
	EVT_BUTTON(PALETTE_WAYPOINT_REMOVE_WAYPOINT, WaypointPalettePanel::OnClickRemoveWaypoint)

	EVT_LIST_BEGIN_LABEL_EDIT(PALETTE_WAYPOINT_LISTBOX, WaypointPalettePanel::OnBeginEditWaypointLabel)
	EVT_LIST_END_LABEL_EDIT(PALETTE_WAYPOINT_LISTBOX, WaypointPalettePanel::OnEditWaypointLabel)
	EVT_LIST_ITEM_SELECTED(PALETTE_WAYPOINT_LISTBOX, WaypointPalettePanel::OnClickWaypoint)
END_EVENT_TABLE()

WaypointPalettePanel::WaypointPalettePanel(wxWindow* parent, wxWindowID id) :
	PalettePanel(parent, id),
	map(nullptr)
{
	wxSizer* sidesizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Waypoints");

	waypoint_list = newd wxListCtrl(this, PALETTE_WAYPOINT_LISTBOX,
		wxDefaultPosition, wxDefaultSize,
		wxLC_REPORT | wxLC_SINGLE_SEL | wxLC_EDIT_LABELS | wxLC_NO_HEADER);
	waypoint_list->InsertColumn(0, "UNNAMED", wxLIST_FORMAT_LEFT, 200);
	sidesizer->Add(waypoint_list, 1, wxEXPAND);

	wxSizer* tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	tmpsizer->Add(add_waypoint_button = newd wxButton(this, PALETTE_WAYPOINT_ADD_WAYPOINT, "Add", wxDefaultPosition, wxSize(50, -1)), 1, wxEXPAND);
	tmpsizer->Add(remove_waypoint_button = newd wxButton(this, PALETTE_WAYPOINT_REMOVE_WAYPOINT, "Remove", wxDefaultPosition, wxSize(70, -1)), 1, wxEXPAND);
	sidesizer->Add(tmpsizer, 0, wxEXPAND);

	SetSizerAndFit(sidesizer);
}

WaypointPalettePanel::~WaypointPalettePanel()
{
	////
}


void WaypointPalettePanel::OnSwitchIn()
{
	PalettePanel::OnSwitchIn();
}

void WaypointPalettePanel::OnSwitchOut()
{
	PalettePanel::OnSwitchOut();
}

void WaypointPalettePanel::SetMap(Map* m)
{
	map = m;
	this->Enable(m && m->getVersion().otbm >= MAP_OTBM_3);
}

void WaypointPalettePanel::SelectFirstBrush()
{
	//SelectWaypointBrush();
}

Brush* WaypointPalettePanel::GetSelectedBrush() const
{
    long item = waypoint_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	g_gui.waypoint_brush->setWaypoint(
		item == -1?
			nullptr :
			map->waypoints.getWaypoint(nstr(waypoint_list->GetItemText(item)))
	);
	return g_gui.waypoint_brush;
}

bool WaypointPalettePanel::SelectBrush(const Brush* whatbrush)
{
	ASSERT(whatbrush == g_gui.waypoint_brush);
	return false;
}

int WaypointPalettePanel::GetSelectedBrushSize() const
{
	return 0;
}

PaletteType WaypointPalettePanel::GetType() const
{
	return TILESET_WAYPOINT;
}

wxString WaypointPalettePanel::GetName() const
{
	return "Waypoint Palette";
}

void WaypointPalettePanel::OnUpdate()
{
	if(wxTextCtrl* tc = waypoint_list->GetEditControl()) {
		Waypoint* wp = map->waypoints.getWaypoint(nstr(tc->GetValue()));
		if(wp && !wp->pos.isValid()) {
			if(map->getTile(wp->pos))
				map->getTileL(wp->pos)->decreaseWaypointCount();
			map->waypoints.removeWaypoint(wp->name);
		}
	}
	waypoint_list->DeleteAllItems();

	if(!map){
		waypoint_list->Enable(false);
		add_waypoint_button->Enable(false);
		remove_waypoint_button->Enable(false);
	} else {
		waypoint_list->Enable(true);
		add_waypoint_button->Enable(true);
		remove_waypoint_button->Enable(true);

		Waypoints& waypoints = map->waypoints;

		for(WaypointMap::const_iterator iter = waypoints.begin(); iter != waypoints.end(); ++iter) {
			waypoint_list->InsertItem(0, wxstr(iter->second->name));
		}
	}
}

void WaypointPalettePanel::OnClickWaypoint(wxListEvent& event)
{
	if(!map)
		return;

	std::string wpname = nstr(event.GetText());
	Waypoint* wp = map->waypoints.getWaypoint(wpname);
	if(wp) {
		g_gui.SetScreenCenterPosition(wp->pos);
		g_gui.waypoint_brush->setWaypoint(wp);
	}
}

void WaypointPalettePanel::OnBeginEditWaypointLabel(wxListEvent& event)
{
	// We need to disable all hotkeys, so we can type properly
	g_gui.DisableHotkeys();
}

void WaypointPalettePanel::OnEditWaypointLabel(wxListEvent& event)
{
	std::string wpname = nstr(event.GetLabel());
	std::string oldwpname = nstr(waypoint_list->GetItemText(event.GetIndex()));
	Waypoint* wp = map->waypoints.getWaypoint(oldwpname);

	if(event.IsEditCancelled())
		return;

	if(wpname == "") {
		map->waypoints.removeWaypoint(oldwpname);
		g_gui.RefreshPalettes();
	} else if(wp) {
		if(wpname == oldwpname) {
			; // do nothing
		} else {
			if(map->waypoints.getWaypoint(wpname)) {
				// Already exists a waypoint with this name!
				g_gui.SetStatusText("There already is a waypoint with this name.");
				event.Veto();
				if(oldwpname == "") {
					map->waypoints.removeWaypoint(oldwpname);
					g_gui.RefreshPalettes();
				}
			} else {
				Waypoint* nwp = newd Waypoint(*wp);
				nwp->name = wpname;

				Waypoint* rwp = map->waypoints.getWaypoint(oldwpname);
				if(rwp) {
					if(map->getTile(rwp->pos))
						map->getTileL(rwp->pos)->decreaseWaypointCount();
					map->waypoints.removeWaypoint(rwp->name);
				}

				map->waypoints.addWaypoint(nwp);
				g_gui.waypoint_brush->setWaypoint(nwp);

				// Refresh other palettes
				refresh_timer.Start(300, true);
			}
		}
	}

	if(event.IsAllowed())
		g_gui.EnableHotkeys();
}

void WaypointPalettePanel::OnClickAddWaypoint(wxCommandEvent& event)
{
	if(map) {
		map->waypoints.addWaypoint(newd Waypoint());
		long i = waypoint_list->InsertItem(0, "");
		waypoint_list->EditLabel(i);

		//g_gui.RefreshPalettes();
	}
}

void WaypointPalettePanel::OnClickRemoveWaypoint(wxCommandEvent& event)
{
	if(!map)
		return;

    long item = waypoint_list->GetNextItem(-1, wxLIST_NEXT_ALL, wxLIST_STATE_SELECTED);
	if(item != -1) {
		Waypoint* wp = map->waypoints.getWaypoint(nstr(waypoint_list->GetItemText(item)));
		if(wp) {
			if(map->getTile(wp->pos))
				map->getTileL(wp->pos)->decreaseWaypointCount();
			map->waypoints.removeWaypoint(wp->name);
		}
		waypoint_list->DeleteItem(item);
		refresh_timer.Start(300, true);
	}
}
