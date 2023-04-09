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

#include "main.h"

#include "settings.h"
#include "brush.h"
#include "gui.h"
#include "palette_npc.h"
#include "npc_brush.h"
#include "spawn_npc_brush.h"
#include "materials.h"

// ============================================================================
// Npc palette

BEGIN_EVENT_TABLE(NpcPalettePanel, PalettePanel)
	EVT_CHOICE(PALETTE_NPC_TILESET_CHOICE, NpcPalettePanel::OnTilesetChange)

	EVT_LISTBOX(PALETTE_NPC_LISTBOX, NpcPalettePanel::OnListBoxChange)

	EVT_TOGGLEBUTTON(PALETTE_NPC_BRUSH_BUTTON, NpcPalettePanel::OnClickNpcBrushButton)
	EVT_TOGGLEBUTTON(PALETTE_SPAWN_NPC_BRUSH_BUTTON, NpcPalettePanel::OnClickSpawnNpcBrushButton)

	EVT_SPINCTRL(PALETTE_SPAWN_NPC_TIME, NpcPalettePanel::OnChangeSpawnNpcTime)
	EVT_SPINCTRL(PALETTE_SPAWN_NPC_SIZE, NpcPalettePanel::OnChangeSpawnNpcSize)
END_EVENT_TABLE()

NpcPalettePanel::NpcPalettePanel(wxWindow* parent, wxWindowID id) :
	PalettePanel(parent, id),
	handling_event(false)
{
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);

	wxSizer* sidesizer = newd wxStaticBoxSizer(wxVERTICAL, this, "Npcs");
	tileset_choice = newd wxChoice(this, PALETTE_NPC_TILESET_CHOICE, wxDefaultPosition, wxDefaultSize, (int)0, (const wxString*)nullptr);
	sidesizer->Add(tileset_choice, 0, wxEXPAND);

	npc_list = newd SortableListBox(this, PALETTE_NPC_LISTBOX);
	sidesizer->Add(npc_list, 1, wxEXPAND);
	topsizer->Add(sidesizer, 1, wxEXPAND);

	// Brush selection
	sidesizer = newd wxStaticBoxSizer(newd wxStaticBox(this, wxID_ANY, "Brushes", wxDefaultPosition, wxSize(150, 200)), wxVERTICAL);

	//sidesizer->Add(180, 1, wxEXPAND);

	wxFlexGridSizer* grid = newd wxFlexGridSizer(3, 10, 10);
	grid->AddGrowableCol(1);

	grid->Add(newd wxStaticText(this, wxID_ANY, "Spawntime"));
	npc_spawntime_spin = newd wxSpinCtrl(this, PALETTE_SPAWN_NPC_TIME, i2ws(g_settings.getInteger(Config::DEFAULT_SPAWN_NPC_TIME)), wxDefaultPosition, wxSize(50, 20), wxSP_ARROW_KEYS, 0, 3600, g_settings.getInteger(Config::DEFAULT_SPAWN_NPC_TIME));
	grid->Add(npc_spawntime_spin, 0, wxEXPAND);
	npc_brush_button = newd wxToggleButton(this, PALETTE_NPC_BRUSH_BUTTON, "Place Npc");
	grid->Add(npc_brush_button, 0, wxEXPAND);

	grid->Add(newd wxStaticText(this, wxID_ANY, "Spawn size"));
	spawn_npc_size_spin = newd wxSpinCtrl(this, PALETTE_SPAWN_NPC_SIZE, i2ws(1), wxDefaultPosition, wxSize(50, 20), wxSP_ARROW_KEYS, 1, g_settings.getInteger(Config::MAX_SPAWN_NPC_RADIUS), g_settings.getInteger(Config::CURRENT_SPAWN_NPC_RADIUS));
	grid->Add(spawn_npc_size_spin, 0, wxEXPAND);
	spawn_npc_brush_button = newd wxToggleButton(this, PALETTE_SPAWN_NPC_BRUSH_BUTTON, "Place Spawn");
	grid->Add(spawn_npc_brush_button, 0, wxEXPAND);

	sidesizer->Add(grid, 0, wxEXPAND);
	topsizer->Add(sidesizer, 0, wxEXPAND);
	SetSizerAndFit(topsizer);

	OnUpdate();
}

NpcPalettePanel::~NpcPalettePanel()
{
	////
}

PaletteType NpcPalettePanel::GetType() const
{
	return TILESET_NPC;
}

void NpcPalettePanel::SelectFirstBrush()
{
	SelectNpcBrush();
}

Brush* NpcPalettePanel::GetSelectedBrush() const
{
	if(npc_brush_button->GetValue()) {
		if(npc_list->GetCount() == 0) {
			return nullptr;
		}
		Brush* brush = reinterpret_cast<Brush*>(npc_list->GetClientData(npc_list->GetSelection()));
		if(brush && brush->isNpc()) {
			g_gui.SetSpawnNpcTime(npc_spawntime_spin->GetValue());
			return brush;
		}
	} else if(spawn_npc_brush_button->GetValue()) {
		g_settings.setInteger(Config::CURRENT_SPAWN_NPC_RADIUS, spawn_npc_size_spin->GetValue());
		g_settings.setInteger(Config::DEFAULT_SPAWN_NPC_TIME, npc_spawntime_spin->GetValue());
		return g_gui.spawn_npc_brush;
	}
	return nullptr;
}

bool NpcPalettePanel::SelectBrush(const Brush* whatbrush)
{
	if(!whatbrush)
		return false;

	if(whatbrush->isNpc()) {
		int current_index = tileset_choice->GetSelection();
		if(current_index != wxNOT_FOUND) {
			const TilesetCategory* tsc = reinterpret_cast<const TilesetCategory*>(tileset_choice->GetClientData(current_index));
			// Select first house
			for(BrushVector::const_iterator iter = tsc->brushlist.begin(); iter != tsc->brushlist.end(); ++iter) {
				if(*iter == whatbrush) {
					SelectNpc(whatbrush->getName());
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
						SelectNpc(whatbrush->getName());
						return true;
					}
				}
			}
		}
	} else if(whatbrush->isSpawnNpc()) {
		SelectSpawnNpcBrush();
		return true;
	}
	return false;
}

int NpcPalettePanel::GetSelectedBrushSize() const
{
	return spawn_npc_size_spin->GetValue();
}

void NpcPalettePanel::OnUpdate()
{
	tileset_choice->Clear();
	g_materials.createNpcTileset();

	for(TilesetContainer::const_iterator iter = g_materials.tilesets.begin(); iter != g_materials.tilesets.end(); ++iter) {
		const TilesetCategory* tsc = iter->second->getCategory(TILESET_NPC);
		if(tsc && tsc->size() > 0) {
			tileset_choice->Append(wxstr(iter->second->name), const_cast<TilesetCategory*>(tsc));
		} else if(iter->second->name == "NPCs") {
			Tileset* ts = const_cast<Tileset*>(iter->second);
			TilesetCategory* rtsc = ts->getCategory(TILESET_NPC);
			tileset_choice->Append(wxstr(ts->name), rtsc);
		}
	}
	SelectTileset(0);
}

void NpcPalettePanel::OnUpdateBrushSize(BrushShape shape, int size)
{
	return spawn_npc_size_spin->SetValue(size);
}

void NpcPalettePanel::OnSwitchIn()
{
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SetBrushSize(spawn_npc_size_spin->GetValue());
}

void NpcPalettePanel::SelectTileset(size_t index)
{
	ASSERT(tileset_choice->GetCount() >= index);

	npc_list->Clear();
	if(tileset_choice->GetCount() == 0) {
		// No tilesets :(
		npc_brush_button->Enable(false);
	} else {
		const TilesetCategory* tsc = reinterpret_cast<const TilesetCategory*>(tileset_choice->GetClientData(index));
		// Select first house
		for(BrushVector::const_iterator iter = tsc->brushlist.begin();
				iter != tsc->brushlist.end();
				++iter)
		{
			npc_list->Append(wxstr((*iter)->getName()), *iter);
		}
		npc_list->Sort();
		SelectNpc(0);

		tileset_choice->SetSelection(index);
	}
}

void NpcPalettePanel::SelectNpc(size_t index)
{
	// Save the old g_settings
	ASSERT(npc_list->GetCount() >= index);

	if(npc_list->GetCount() > 0) {
		npc_list->SetSelection(index);
	}

	SelectNpcBrush();
}

void NpcPalettePanel::SelectNpc(std::string name)
{
	if(npc_list->GetCount() > 0) {
		if(!npc_list->SetStringSelection(wxstr(name))) {
			npc_list->SetSelection(0);
		}
	}

	SelectNpcBrush();
}

void NpcPalettePanel::SelectNpcBrush()
{
	if(npc_list->GetCount() > 0) {
		npc_brush_button->Enable(true);
		npc_brush_button->SetValue(true);
		spawn_npc_brush_button->SetValue(false);
	} else {
		npc_brush_button->Enable(false);
		SelectSpawnNpcBrush();
	}
}

void NpcPalettePanel::SelectSpawnNpcBrush()
{
	//g_gui.house_exit_brush->setHouse(house);
	npc_brush_button->SetValue(false);
	spawn_npc_brush_button->SetValue(true);
}

void NpcPalettePanel::OnTilesetChange(wxCommandEvent& event)
{
	SelectTileset(event.GetSelection());
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SelectBrush();
}

void NpcPalettePanel::OnListBoxChange(wxCommandEvent& event)
{
	SelectNpc(event.GetSelection());
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SelectBrush();
}

void NpcPalettePanel::OnClickNpcBrushButton(wxCommandEvent& event)
{
	SelectNpcBrush();
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SelectBrush();
}

void NpcPalettePanel::OnClickSpawnNpcBrushButton(wxCommandEvent& event)
{
	SelectSpawnNpcBrush();
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SelectBrush();
}

void NpcPalettePanel::OnChangeSpawnNpcTime(wxSpinEvent& event)
{
	g_gui.ActivatePalette(GetParentPalette());
	g_gui.SetSpawnNpcTime(event.GetPosition());
}

void NpcPalettePanel::OnChangeSpawnNpcSize(wxSpinEvent& event)
{
	if(!handling_event) {
		handling_event = true;
		g_gui.ActivatePalette(GetParentPalette());
		g_gui.SetBrushSize(event.GetPosition());
		handling_event = false;
	}
}
