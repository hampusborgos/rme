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


#ifndef RME_TILESET_MONSTER_H_
#define RME_TILESET_MONSTER_H_

#include "palette_common.h"

class MonsterPalettePanel : public PalettePanel {
public:
	MonsterPalettePanel(wxWindow* parent, wxWindowID id = wxID_ANY);
	virtual ~MonsterPalettePanel();

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
	void SelectMonster(size_t index);
	void SelectMonster(std::string name);
public:
	// Event handling
	void OnChangeSpawnMonsterTime(wxSpinEvent& event);
	void OnChangeSpawnMonsterSize(wxSpinEvent& event);

	void OnTilesetChange(wxCommandEvent& event);
	void OnListBoxChange(wxCommandEvent& event);
	void OnClickMonsterBrushButton(wxCommandEvent& event);
	void OnClickSpawnMonsterBrushButton(wxCommandEvent& event);
protected:
	void SelectMonsterBrush();
	void SelectSpawnBrush();

	wxChoice* tileset_choice;
	SortableListBox* monster_list;
	wxToggleButton* monster_brush_button;
	wxToggleButton* spawn_monster_brush_button;
	wxSpinCtrl* monster_spawntime_spin;
	wxSpinCtrl* spawn_monster_size_spin;

	bool handling_event;

	DECLARE_EVENT_TABLE();
};

#endif
