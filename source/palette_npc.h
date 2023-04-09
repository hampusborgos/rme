//////////////////////////////////////////////////////////////////////
// This file is part of Canary Map Editor
//////////////////////////////////////////////////////////////////////
// Canary Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Canary Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////


#ifndef RME_TILESET_NPC_H_
#define RME_TILESET_NPC_H_

#include "palette_common.h"

class NpcPalettePanel : public PalettePanel {
public:
	NpcPalettePanel(wxWindow* parent, wxWindowID id = wxID_ANY);
	virtual ~NpcPalettePanel();

	PaletteType GetType() const;

	// Select the first brush
	void SelectFirstBrush();
	// Returns the currently selected brush (first brush if panel is not loaded)
	Brush* GetSelectedBrush() const;
	// Returns the currently selected brush size
	int GetSelectedBrushSize() const;
	// Select the brush in the parameter, this only changes the look of the panel
	bool SelectBrush(const Brush* whatbrush);

	// Updates the palette window to use the current brush size
	void OnUpdateBrushSize(BrushShape shape, int size);
	// Called when this page is displayed
	void OnSwitchIn();
	// Called sometimes?
	void OnUpdate();

protected:
	void SelectTileset(size_t index);
	void SelectNpc(size_t index);
	void SelectNpc(std::string name);
public:
	// Event handling
	void OnChangeSpawnNpcTime(wxSpinEvent& event);
	void OnChangeSpawnNpcSize(wxSpinEvent& event);

	void OnTilesetChange(wxCommandEvent& event);
	void OnListBoxChange(wxCommandEvent& event);
	void OnClickNpcBrushButton(wxCommandEvent& event);
	void OnClickSpawnNpcBrushButton(wxCommandEvent& event);
protected:
	void SelectNpcBrush();
	void SelectSpawnNpcBrush();

	wxChoice* tileset_choice;
	SortableListBox* npc_list;
	wxToggleButton* npc_brush_button;
	wxToggleButton* spawn_npc_brush_button;
	wxSpinCtrl* npc_spawntime_spin;
	wxSpinCtrl* spawn_npc_size_spin;

	bool handling_event;

	DECLARE_EVENT_TABLE();
};

#endif
