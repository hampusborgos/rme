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
	OldPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Monster* monster, wxPoint = wxDefaultPosition);
	OldPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, SpawnMonster* spawnMonster, wxPoint = wxDefaultPosition);
	OldPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Npc* npc, wxPoint = wxDefaultPosition);
	OldPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, SpawnNpc* spawnNpc, wxPoint = wxDefaultPosition);
	virtual ~OldPropertiesWindow();

	void OnFocusChange(wxFocusEvent&);

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);

	void Update();

protected:

protected:
	wxSpinCtrl* count_field;
	wxChoice* direction_field;
	wxSpinCtrl* action_id_field;
	wxSpinCtrl* unique_id_field;
	wxSpinCtrl* door_id_field;
	wxChoice* depot_id_field;
	wxSpinCtrl* x_field;
	wxSpinCtrl* y_field;
	wxSpinCtrl* z_field;
	wxChoice* splash_type_field;
	wxTextCtrl* text_field;
	wxTextCtrl* description_field;
	std::vector<ContainerItemButton*> container_items;

	friend class ContainerItemButton;
	friend class ContainerItemPopupMenu;

	DECLARE_EVENT_TABLE();
};

#endif

