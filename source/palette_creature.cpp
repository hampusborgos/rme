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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/palette_creature.hpp $
// $Id: palette_creature.hpp 310 2010-02-26 18:03:48Z admin $

#include "main.h"

#include "settings.h"
#include "brush.h"
#include "gui.h"
#include "palette_creature.h"
#include "creature_brush.h"
#include "spawn_brush.h"
#include "materials.h"

// ============================================================================
// Creature palette

BEGIN_EVENT_TABLE(CreaturePalettePanel, PalettePanel)
	EVT_CHOICE(PALETTE_CREATURE_TILESET_CHOICE, CreaturePalettePanel::OnTilesetChange)

	EVT_LISTBOX(PALETTE_CREATURE_LISTBOX, CreaturePalettePanel::OnListBoxChange)

	EVT_TOGGLEBUTTON(PALETTE_CREATURE_BRUSH_BUTTON, CreaturePalettePanel::OnClickCreatureBrushButton)
	EVT_TOGGLEBUTTON(PALETTE_SPAWN_BRUSH_BUTTON, CreaturePalettePanel::OnClickSpawnBrushButton)

	EVT_SPINCTRL(PALETTE_CREATURE_SPAWN_TIME, CreaturePalettePanel::OnChangeSpawnTime)
	EVT_SPINCTRL(PALETTE_CREATURE_SPAWN_SIZE, CreaturePalettePanel::OnChangeSpawnSize)
END_EVENT_TABLE()

CreaturePalettePanel::CreaturePalettePanel(wxWindow* parent, wxWindowID id) :
	PalettePanel(parent, id),
	handling_event(false)
{
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);

	wxSizer* sidesizer = newd wxStaticBoxSizer(wxVERTICAL, this, wxT("Creatures"));
	tileset_choice = newd wxChoice(this, PALETTE_CREATURE_TILESET_CHOICE, wxDefaultPosition, wxDefaultSize, (int)0, (const wxString*)nullptr);
	sidesizer->Add(tileset_choice, 0, wxEXPAND);

	creature_list = newd wxListBox(this, PALETTE_CREATURE_LISTBOX, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE | wxLB_NEEDED_SB | wxLB_SORT);
	sidesizer->Add(creature_list, 1, wxEXPAND);
	topsizer->Add(sidesizer, 1, wxEXPAND);

	// Brush selection
	sidesizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, wxT("Brushes"), wxDefaultPosition, wxSize(150, 200)), wxVERTICAL);

	//sidesizer->Add(180, 1, wxEXPAND);

	wxFlexGridSizer* grid = newd wxFlexGridSizer(3, 10, 10);
	grid->AddGrowableCol(1);

	grid->Add(newd wxStaticText(this, wxID_ANY, wxT("Spawntime")));
	creature_spawntime_spin = newd wxSpinCtrl(this, PALETTE_CREATURE_SPAWN_TIME, i2ws(settings.getInteger(Config::DEFAULT_SPAWNTIME)), wxDefaultPosition, wxSize(50, 20), wxSP_ARROW_KEYS, 0, 3600, settings.getInteger(Config::DEFAULT_SPAWNTIME));
	grid->Add(creature_spawntime_spin, 0, wxEXPAND);
	creature_brush_button = newd wxToggleButton(this, PALETTE_CREATURE_BRUSH_BUTTON, wxT("Place Creature"));
	grid->Add(creature_brush_button, 0, wxEXPAND);

	grid->Add(newd wxStaticText(this, wxID_ANY, wxT("Spawn size")));
	spawn_size_spin = newd wxSpinCtrl(this, PALETTE_CREATURE_SPAWN_SIZE, i2ws(5), wxDefaultPosition, wxSize(50, 20), wxSP_ARROW_KEYS, 1, settings.getInteger(Config::MAX_SPAWN_RADIUS), settings.getInteger(Config::CURRENT_SPAWN_RADIUS));
	grid->Add(spawn_size_spin, 0, wxEXPAND);
	spawn_brush_button = newd wxToggleButton(this, PALETTE_SPAWN_BRUSH_BUTTON, wxT("Place Spawn"));
	grid->Add(spawn_brush_button, 0, wxEXPAND);

	sidesizer->Add(grid, 0, wxEXPAND);
	topsizer->Add(sidesizer, 0, wxEXPAND);
	SetSizerAndFit(topsizer);

	OnUpdate();
}

CreaturePalettePanel::~CreaturePalettePanel()
{
	////
}

PaletteType CreaturePalettePanel::GetType() const
{
	return TILESET_CREATURE;
}

void CreaturePalettePanel::SelectFirstBrush()
{
	SelectCreatureBrush();
}

Brush* CreaturePalettePanel::GetSelectedBrush() const
{
	if(creature_brush_button->GetValue()) {
		if(creature_list->GetCount() == 0) {
			return nullptr;
		}
		Brush* creature_brush = reinterpret_cast<Brush*>(creature_list->GetClientData(creature_list->GetSelection()));
		if(dynamic_cast<CreatureBrush*>(creature_brush)) {
			gui.SetSpawnTime(creature_spawntime_spin->GetValue());
			return creature_brush;
		}
	} else if(spawn_brush_button->GetValue()) {
		settings.setInteger(Config::CURRENT_SPAWN_RADIUS, spawn_size_spin->GetValue());
		settings.setInteger(Config::DEFAULT_SPAWNTIME, creature_spawntime_spin->GetValue());
		return gui.spawn_brush;
	}
	return nullptr;
}

bool CreaturePalettePanel::SelectBrush(const Brush* whatbrush)
{
	bool isCreatureBrush = dynamic_cast<const CreatureBrush*>(whatbrush)? true : false;
	if(isCreatureBrush) {
		int current_index = tileset_choice->GetSelection();
		if(current_index != wxNOT_FOUND) {
			const TilesetCategory* tsc = reinterpret_cast<const TilesetCategory*>(tileset_choice->GetClientData(current_index));
			// Select first house
			for(BrushVector::const_iterator iter = tsc->brushlist.begin(); iter != tsc->brushlist.end(); ++iter) {
				if(*iter == whatbrush) {
					SelectCreature(whatbrush->getName());
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
						SelectCreature(whatbrush->getName());
						return true;
					}
				}
			}
		}
	} else if(dynamic_cast<const SpawnBrush*>(whatbrush)) {
		SelectSpawnBrush();
		return true;
	}
	return false;
}

int CreaturePalettePanel::GetSelectedBrushSize() const
{
	return spawn_size_spin->GetValue();
}

void CreaturePalettePanel::OnUpdate()
{
	tileset_choice->Clear();
	materials.createOtherTileset();

	for(TilesetContainer::const_iterator iter = materials.tilesets.begin(); iter != materials.tilesets.end(); ++iter) {
		const TilesetCategory* tsc = iter->second->getCategory(TILESET_CREATURE);
		if(tsc && tsc->size() > 0) {
			tileset_choice->Append(wxstr(iter->second->name), const_cast<TilesetCategory*>(tsc));
		} else if(iter->second->name == "NPCs" || iter->second->name == "Others") {
			Tileset* ts = const_cast<Tileset*>(iter->second);
			TilesetCategory* rtsc = ts->getCategory(TILESET_CREATURE);
			tileset_choice->Append(wxstr(ts->name), rtsc);
		}
	}
	SelectTileset(0);
}

void CreaturePalettePanel::OnUpdateBrushSize(BrushShape shape, int size)
{
	return spawn_size_spin->SetValue(size);
}

void CreaturePalettePanel::OnSwitchIn()
{
	gui.ActivatePalette(GetParentPalette());
	gui.SetBrushSize(spawn_size_spin->GetValue());
}

void CreaturePalettePanel::SelectTileset(size_t index)
{
	ASSERT(tileset_choice->GetCount() >= index);

	creature_list->Clear();
	if(tileset_choice->GetCount() == 0) {
		// No tilesets :(
		creature_brush_button->Enable(false);
	} else {
		const TilesetCategory* tsc = reinterpret_cast<const TilesetCategory*>(tileset_choice->GetClientData(index));
		// Select first house
		for(BrushVector::const_iterator iter = tsc->brushlist.begin();
				iter != tsc->brushlist.end();
				++iter)
		{
			creature_list->Append(wxstr((*iter)->getName()), *iter);
		}
		SelectCreature(0);

		tileset_choice->SetSelection(index);
	}
}

void CreaturePalettePanel::SelectCreature(size_t index)
{
	// Save the old settings
	ASSERT(creature_list->GetCount() >= index);

	if(creature_list->GetCount() > 0) {
		creature_list->SetSelection(index);
	}

	SelectCreatureBrush();
}

void CreaturePalettePanel::SelectCreature(std::string name)
{
	if(creature_list->GetCount() > 0) {
		if(!creature_list->SetStringSelection(wxstr(name))) {
			creature_list->SetSelection(0);
		}
	}

	SelectCreatureBrush();
}

void CreaturePalettePanel::SelectCreatureBrush()
{
	if(creature_list->GetCount() > 0) {
		creature_brush_button->Enable(true);
		creature_brush_button->SetValue(true);
		spawn_brush_button->SetValue(false);
	} else {
		creature_brush_button->Enable(false);
		SelectSpawnBrush();
	}
}

void CreaturePalettePanel::SelectSpawnBrush()
{
	//gui.house_exit_brush->setHouse(house);
	creature_brush_button->SetValue(false);
	spawn_brush_button->SetValue(true);
}

void CreaturePalettePanel::OnTilesetChange(wxCommandEvent& event)
{
	SelectTileset(event.GetSelection());
	gui.ActivatePalette(GetParentPalette());
	gui.SelectBrush();
}

void CreaturePalettePanel::OnListBoxChange(wxCommandEvent& event)
{
	SelectCreature(event.GetSelection());
	gui.ActivatePalette(GetParentPalette());
	gui.SelectBrush();
}

void CreaturePalettePanel::OnClickCreatureBrushButton(wxCommandEvent& event)
{
	SelectCreatureBrush();
	gui.ActivatePalette(GetParentPalette());
	gui.SelectBrush();
}

void CreaturePalettePanel::OnClickSpawnBrushButton(wxCommandEvent& event)
{
	SelectSpawnBrush();
	gui.ActivatePalette(GetParentPalette());
	gui.SelectBrush();
}

void CreaturePalettePanel::OnChangeSpawnTime(wxSpinEvent& event)
{
	gui.ActivatePalette(GetParentPalette());
	gui.SetSpawnTime(event.GetPosition());
}

void CreaturePalettePanel::OnChangeSpawnSize(wxSpinEvent& event)
{
	if(!handling_event) {
		handling_event = true;
		gui.ActivatePalette(GetParentPalette());
		gui.SetBrushSize(event.GetPosition());
		handling_event = false;
	}
}

