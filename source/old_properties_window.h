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

#ifndef RME_OLD_PROPERTIES_WINDOW_H_
#define RME_OLD_PROPERTIES_WINDOW_H_

#include "main.h"

#include "common_windows.h"

class ContainerItemButton;
class ContainerItemPopupMenu;

class OldPropertiesWindow : public ObjectPropertiesWindowBase
{
public:
	OldPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Item* item, wxPoint = wxDefaultPosition);
	OldPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Creature* creature, wxPoint = wxDefaultPosition);
	OldPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Spawn* spawn, wxPoint = wxDefaultPosition);
	virtual ~OldPropertiesWindow();

	void OnFocusChange(wxFocusEvent&);

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);

	void Update();

protected:
	// item
	wxSpinCtrl* count_field;
	wxSpinCtrl* action_id_field;
	wxSpinCtrl* unique_id_field;
	wxSpinCtrl* door_id_field;
	wxSpinCtrl* tier_field;
	wxChoice* depot_id_field;
	wxChoice* splash_type_field;
	wxTextCtrl* text_field;
	wxTextCtrl* description_field;

	// teleport
	wxSpinCtrl* x_field;
	wxSpinCtrl* y_field;
	wxSpinCtrl* z_field;

	// podium
	wxCheckBox* show_outfit;
	wxCheckBox* show_mount;
	wxCheckBox* show_platform;
	wxSpinCtrl* look_type;
	wxSpinCtrl* look_head;
	wxSpinCtrl* look_body;
	wxSpinCtrl* look_legs;
	wxSpinCtrl* look_feet;
	wxSpinCtrl* look_addon;
	wxSpinCtrl* look_mount;
	wxSpinCtrl* look_mounthead;
	wxSpinCtrl* look_mountbody;
	wxSpinCtrl* look_mountlegs;
	wxSpinCtrl* look_mountfeet;

	// podium and creature
	wxChoice* direction_field;

	// container
	std::vector<ContainerItemButton*> container_items;

	friend class ContainerItemButton;
	friend class ContainerItemPopupMenu;

	DECLARE_EVENT_TABLE();
};

#endif

