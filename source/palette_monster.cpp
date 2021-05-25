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

#include "settings.h"
#include "brush.h"
#include "gui.h"
#include "palette_monster.h"
#include "monster_brush.h"
#include "spawn_monster_brush.h"
#include "materials.h"

// ============================================================================
// Monster palette

BEGIN_EVENT_TABLE(MonsterPalettePanel, PalettePanel)
	EVT_CHOICE(PALETTE_MONSTER_TILESET_CHOICE, MonsterPalettePanel::OnTilesetChange)

	EVT_LISTBOX(PALETTE_MONSTER_LISTBOX, MonsterPalettePanel::OnListBoxChange)

	EVT_TOGGLEBUTTON(PALETTE_MONSTER_BRUSH_BUTTON, MonsterPalettePanel::OnClickMonsterBrushButton)
	EVT_TOGGLEBUTTON(PALETTE_SPAWN_MONSTER_BRUSH_BUTTON, MonsterPalettePanel::OnClickSpawnMonsterBrushButton)

	EVT_SPINCTRL(PALETTE_MONSTER_SPAWN_TIME, MonsterPalettePanel::OnChangeSpawnMonsterTime)
	EVT_SPINCTRL(PALETTE_MONSTER_SPAWN_SIZE, MonsterPalettePanel::OnChangeSpawnMonsterSize)
END_EVENT_TABLE()

MonsterPalettePanel::MonsterPalettePanel(wxWindow* parent, wxWindowID id) :
	PalettePanel(parent, id),
	handling_event(false)
{
	
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);

	wxSizer* sidesizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Monsters");
	tileset_choice = newd wxChoice(this, PALETTE_MONSTER_TILESET_CHOICE, wxDefaultPosition, wxDefaultSize, (int)0, (const wxString*)nullptr);
	sidesizer->Add(tileset_choice, 0, wxEXPAND);

	monster_list = newd SortableListBox(this, PALETTE_MONSTER_LISTBOX);
	sidesizer->Add(monster_list, 1, wxEXPAND);
	topsizer->Add(sidesizer, 1, wxEXPAND);

	// Brush selection
	sidesizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, "Brushes", wxDefaultPosition, wxSize(150, 200)), wxVERTICAL);

	//sidesizer->Add(180, 1, wxEXPAND);

	wxFlexGridSizer* grid = newd wxFlexGridSizer(3, 10, 10);
	grid->AddGrowableCol(1);

	grid->Add(newd wxStaticText(this, wxID_ANY, "Spawntime"));
	monster_spawntime_spin = newd wxSpinCtrl(this, PALETTE_MONSTER_SPAWN_TIME, i2ws(g_settings.getInteger(Config::DEFAULT_SPAWN_MONSTER_TIME)), wxDefaultPosition, wxSize(50, 20), wxSP_ARROW_KEYS, 0, 3600, g_settings.getInteger(Config::DEFAULT_SPAWN_MONSTER_TIME));
	grid->Add(monster_spawntime_spin, 0, wxEXPAND);
	monster_brush_button = newd wxToggleButton(this, PALETTE_MONSTER_BRUSH_BUTTON, "Place Monster");
	grid->Add(monster_brush_button, 0, wxEXPAND);

	grid->Add(newd wxStaticText(this, wxID_ANY, "Spawn size"));
	spawn_monster_size_spin = newd wxSpinCtrl(this, PALETTE_MONSTER_SPAWN_SIZE, i2ws(5), wxDefaultPosition, wxSize(50, 20), wxSP_ARROW_KEYS, 1, g_settings.getInteger(Config::MAX_SPAWN_MONSTER_RADIUS), g_settings.getInteger(Config::CURRENT_SPAWN_MONSTER_RADIUS));
	grid->Add(spawn_monster_size_spin, 0, wxEXPAND);
	spawn_monster_brush_button = newd wxToggleButton(this, PALETTE_SPAWN_MONSTER_BRUSH_BUTTON, "Place Spawn");
	grid->Add(spawn_monster_brush_button, 0, wxEXPAND);

	sidesizer->Add(grid, 0, wxEXPAND);
	topsizer->Add(sidesizer, 0, wxEXPAND);
	SetSizerAndFit(topsizer);

	OnUpdate();
}

MonsterPalettePanel::~MonsterPalettePanel()
{
	////
}

PaletteType MonsterPalettePanel::GetType() const
{
	return TILESET_MONSTER;
}

void MonsterPalettePanel::SelectFirstBrush()
{
	SelectMonsterBrush();
}

Brush* MonsterPalettePanel::GetSelectedBrush() const
{
	if(monster_brush_button->GetValue()) {
		if(monster_list->GetCount() == 0) {
			return nullptr;
		}
		Brush* brush = reinterpret_cast<Brush*>(monster_list->GetClientData(monster_list->GetSelection()));
		if(brush && brush->isMonster()) {
			g_gui.SetSpawnMonsterTime(monster_spawntime_spin->GetValue());
			return brush;
		}
	} else if(spawn_monster_brush_button->GetValue()) {
		g_settings.setInteger(Config::CURRENT_SPAWN_MONSTER_RADIUS, spawn_monster_size_spin->GetValue());
		g_settings.setInteger(Config::DEFAULT_SPAWN_MONSTER_TIME, monster_spawntime_spin->GetValue());
		return g_gui.spawn_brush;
	}
	return nullptr;
}

bool MonsterPalettePanel::SelectBrush(const Brush* whatbrush)
{
	if(!whatbrush)
		return false;

	if(whatbrush->isMonster()) {
		int current_index = tileset_choice->GetSelection();
		if(current_index != wxNOT_FOUND) {
			const TilesetCategory* tsc = reinterpret_cast<const TilesetCategory*>(tileset_choice->GetClientData(current_index));
			// Select first house
			for(BrushVector::const_iterator iter = tsc->brushlist.begin(); iter != tsc->brushlist.end(); ++iter) {
				if(*iter == whatbrush) {
					SelectMonster(whatbrush->getName());
					return true;
				}
			}
		}
		// Not in the current display, search the hidden one's
		for(size_t i = 0; i < tileset_choice->GetCount(); ++i) {
			if(current_index != (int)i) {
				const TilesetCategory* tsc = reinterpret_cast<const TilesetCategory*>(tileset_choice->GetClientData(i));
				for(BrushVector::const_iterator iter = tsc->brushlist.begin();
						iter != tsc->brushlist.end();
						++iter)
				{
					if(*iter == whatbrush) {
						SelectTileset(i);
						SelectMonster(whatbrush->getName());
						return true;
					}
				}
			}
		}
	} else if(whatbrush->isSpawnMonster()) {
		SelectSpawnBrush();
		return true;
	}
	return false;
}

int MonsterPalettePanel::GetSelectedBrushSize() const
{
	return spawn_monster_size_spin->GetValue();
}

void MonsterPalettePanel::OnUpdate()
{
	tileset_choice->Clear();
	g_materials.createOtherTileset();

	for(TilesetContainer::const_iterator iter = g_materials.tilesets.begin(); iter != g_materials.tilesets.end(); ++iter) {
		const TilesetCategory* tsc = iter->second->getCategory(TILESET_MONSTER);
		if(tsc && tsc->size() > 0) {
			tileset_choice->Append(wxstr(iter->second->name), const_cast<TilesetCategory*>(tsc));
		} else if(iter->second->name == "Others") {
			Tileset* ts = const_cast<Tileset*>(iter->second);
			TilesetCategory* rtsc = ts->getCategory(TILESET_MONSTER);
			tileset_choice->Append(wxstr(ts->name), rtsc);
		}
	}
	SelectTileset(0);
}

void MonsterPalettePanel::OnUpdateBrushSize(BrushShape shape, int size)
{
	return spawn_monster_size_spin->SetValue(size);
}

void MonsterPalettePanel::OnSwitchIn()
{
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SetBrushSize(spawn_monster_size_spin->GetValue());
}

void MonsterPalettePanel::SelectTileset(size_t index)
{
	ASSERT(tileset_choice->GetCount() >= index);

	monster_list->Clear();
	if(tileset_choice->GetCount() == 0) {
		// No tilesets :(
		monster_brush_button->Enable(false);
	} else {
		const TilesetCategory* tsc = reinterpret_cast<const TilesetCategory*>(tileset_choice->GetClientData(index));
		// Select first house
		for(BrushVector::const_iterator iter = tsc->brushlist.begin();
				iter != tsc->brushlist.end();
				++iter)
		{
			monster_list->Append(wxstr((*iter)->getName()), *iter);
		}
		monster_list->Sort();
		SelectMonster(0);

		tileset_choice->SetSelection(index);
	}
}

void MonsterPalettePanel::SelectMonster(size_t index)
{
	// Save the old g_settings
	ASSERT(monster_list->GetCount() >= index);

	if(monster_list->GetCount() > 0) {
		monster_list->SetSelection(index);
	}

	SelectMonsterBrush();
}

void MonsterPalettePanel::SelectMonster(std::string name)
{
	if(monster_list->GetCount() > 0) {
		if(!monster_list->SetStringSelection(wxstr(name))) {
			monster_list->SetSelection(0);
		}
	}

	SelectMonsterBrush();
}

void MonsterPalettePanel::SelectMonsterBrush()
{
	if(monster_list->GetCount() > 0) {
		monster_brush_button->Enable(true);
		monster_brush_button->SetValue(true);
		spawn_monster_brush_button->SetValue(false);
	} else {
		monster_brush_button->Enable(false);
		SelectSpawnBrush();
	}
}

void MonsterPalettePanel::SelectSpawnBrush()
{
	//g_gui.house_exit_brush->setHouse(house);
	monster_brush_button->SetValue(false);
	spawn_monster_brush_button->SetValue(true);
}

void MonsterPalettePanel::OnTilesetChange(wxCommandEvent& event)
{
	SelectTileset(event.GetSelection());
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SelectBrush();
}

void MonsterPalettePanel::OnListBoxChange(wxCommandEvent& event)
{
	SelectMonster(event.GetSelection());
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SelectBrush();
}

void MonsterPalettePanel::OnClickMonsterBrushButton(wxCommandEvent& event)
{
	SelectMonsterBrush();
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SelectBrush();
}

void MonsterPalettePanel::OnClickSpawnMonsterBrushButton(wxCommandEvent& event)
{
	SelectSpawnBrush();
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SelectBrush();
}

void MonsterPalettePanel::OnChangeSpawnMonsterTime(wxSpinEvent& event)
{
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SetSpawnMonsterTime(event.GetPosition());
}

void MonsterPalettePanel::OnChangeSpawnMonsterSize(wxSpinEvent& event)
{
	if(!handling_event) {
		handling_event = true;
		g_gui.ActivatePalette(GetParentPalette());
		g_gui.SetBrushSize(event.GetPosition());
		handling_event = false;
	}
}
