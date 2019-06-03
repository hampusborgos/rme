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

#include "main.h"
#include "main_toolbar.h"
#include "gui.h"
#include "editor.h"
#include "settings.h"
#include "brush.h"
#include "pngfiles.h"

#include <wx/artprov.h>
#include <wx/mstream.h>

const wxString MainToolBar::STANDARD_BAR_NAME = "standard_toolbar";
const wxString MainToolBar::BRUSHES_BAR_NAME = "brushes_toolbar";

#define loadPNGFile(name) _wxGetBitmapFromMemory(name, sizeof(name))
inline wxBitmap* _wxGetBitmapFromMemory(const unsigned char* data, int length)
{
	wxMemoryInputStream is(data, length);
	wxImage img(is, "image/png");
	if (!img.IsOk()) return nullptr;
	return newd wxBitmap(img, -1);
}

MainToolBar::MainToolBar(wxWindow* parent, wxAuiManager* manager)
{
	wxBitmap new_bitmap = wxArtProvider::GetBitmap(wxART_NEW, wxART_OTHER, wxSize(16, 16));
	wxBitmap open_bitmap = wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_OTHER, wxSize(16, 16));
	wxBitmap save_bitmap = wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_OTHER, wxSize(16, 16));
	wxBitmap saveas_bitmap = wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS, wxART_OTHER, wxSize(16, 16));
	wxBitmap undo_bitmap = wxArtProvider::GetBitmap(wxART_UNDO, wxART_OTHER, wxSize(16, 16));
	wxBitmap redo_bitmap = wxArtProvider::GetBitmap(wxART_REDO, wxART_OTHER, wxSize(16, 16));
	wxBitmap cut_bitmap = wxArtProvider::GetBitmap(wxART_CUT, wxART_OTHER, wxSize(16, 16));
	wxBitmap copy_bitmap = wxArtProvider::GetBitmap(wxART_COPY, wxART_OTHER, wxSize(16, 16));
	wxBitmap paste_bitmap = wxArtProvider::GetBitmap(wxART_PASTE, wxART_OTHER, wxSize(16, 16));

	standard_toolbar = newd wxAuiToolBar(parent, TOOLBAR_STANDARD, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	standard_toolbar->SetToolBitmapSize(wxSize(16, 16));
	standard_toolbar->AddTool(wxID_NEW, wxEmptyString, new_bitmap, wxNullBitmap, wxITEM_NORMAL, "New Map", wxEmptyString, NULL);
	standard_toolbar->AddTool(wxID_OPEN, wxEmptyString, open_bitmap, wxNullBitmap, wxITEM_NORMAL, "Open Map", wxEmptyString, NULL);
	standard_toolbar->AddTool(wxID_SAVE, wxEmptyString, save_bitmap, wxNullBitmap, wxITEM_NORMAL, "Save Map", wxEmptyString, NULL);
	standard_toolbar->AddTool(wxID_SAVEAS, wxEmptyString, saveas_bitmap, wxNullBitmap, wxITEM_NORMAL, "Save Map As...", wxEmptyString, NULL);
	standard_toolbar->AddSeparator();
	standard_toolbar->AddTool(wxID_UNDO, wxEmptyString, undo_bitmap, wxNullBitmap, wxITEM_NORMAL, "Undo", wxEmptyString, NULL);
	standard_toolbar->AddTool(wxID_REDO, wxEmptyString, redo_bitmap, wxNullBitmap, wxITEM_NORMAL, "Redo", wxEmptyString, NULL);
	standard_toolbar->AddSeparator();
	standard_toolbar->AddTool(wxID_CUT, wxEmptyString, cut_bitmap, wxNullBitmap, wxITEM_NORMAL, "Cut", wxEmptyString, NULL);
	standard_toolbar->AddTool(wxID_COPY, wxEmptyString, copy_bitmap, wxNullBitmap, wxITEM_NORMAL, "Copy", wxEmptyString, NULL);
	standard_toolbar->AddTool(wxID_PASTE, wxEmptyString, paste_bitmap, wxNullBitmap, wxITEM_NORMAL, "Paste", wxEmptyString, NULL);
	standard_toolbar->Realize();

	wxBitmap* border_bitmap = loadPNGFile(optional_border_small_png);
	wxBitmap* eraser_bitmap = loadPNGFile(eraser_small_png);
	wxBitmap* pz_bitmap = loadPNGFile(protection_zone_small_png);
	wxBitmap* nopvp_bitmap = loadPNGFile(no_pvp_small_png);
	wxBitmap* nologout_bitmap = loadPNGFile(no_logout_small_png);
	wxBitmap* pvp_bitmap = loadPNGFile(pvp_zone_small_png);
	wxBitmap* normal_bitmap = loadPNGFile(door_normal_small_png);
	wxBitmap* locked_bitmap = loadPNGFile(door_locked_small_png);
	wxBitmap* magic_bitmap = loadPNGFile(door_magic_small_png);
	wxBitmap* quest_bitmap = loadPNGFile(door_quest_small_png);
	wxBitmap* hatch_bitmap = loadPNGFile(window_hatch_small_png);
	wxBitmap* window_bitmap = loadPNGFile(window_normal_small_png);

	brushes_toolbar = newd wxAuiToolBar(parent, TOOLBAR_BRUSHES, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	brushes_toolbar->SetToolBitmapSize(wxSize(16, 16));
	brushes_toolbar->AddTool(PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL, wxEmptyString, *border_bitmap, wxNullBitmap, wxITEM_CHECK, "Border", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_ERASER, wxEmptyString, *eraser_bitmap, wxNullBitmap, wxITEM_CHECK, "Eraser", wxEmptyString, NULL);
	brushes_toolbar->AddSeparator();
	brushes_toolbar->AddTool(PALETTE_TERRAIN_PZ_TOOL, wxEmptyString, *pz_bitmap, wxNullBitmap, wxITEM_CHECK, "Protected Zone", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_NOPVP_TOOL, wxEmptyString, *nopvp_bitmap, wxNullBitmap, wxITEM_CHECK, "No PvP Zone", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_NOLOGOUT_TOOL, wxEmptyString, *nologout_bitmap, wxNullBitmap, wxITEM_CHECK, "No Logout Zone", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_PVPZONE_TOOL, wxEmptyString, *pvp_bitmap, wxNullBitmap, wxITEM_CHECK, "PvP Zone", wxEmptyString, NULL);
	brushes_toolbar->AddSeparator();
	brushes_toolbar->AddTool(PALETTE_TERRAIN_NORMAL_DOOR, wxEmptyString, *normal_bitmap, wxNullBitmap, wxITEM_CHECK, "Normal Door", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_LOCKED_DOOR, wxEmptyString, *locked_bitmap, wxNullBitmap, wxITEM_CHECK, "Locked Door", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_MAGIC_DOOR, wxEmptyString, *magic_bitmap, wxNullBitmap, wxITEM_CHECK, "Magic Door", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_QUEST_DOOR, wxEmptyString, *quest_bitmap, wxNullBitmap, wxITEM_CHECK, "Quest Door", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_HATCH_DOOR, wxEmptyString, *hatch_bitmap, wxNullBitmap, wxITEM_CHECK, "Hatch Window", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_WINDOW_DOOR, wxEmptyString, *window_bitmap, wxNullBitmap, wxITEM_CHECK, "Window", wxEmptyString, NULL);
	brushes_toolbar->Realize();

	manager->AddPane(standard_toolbar, wxAuiPaneInfo().Name(STANDARD_BAR_NAME).ToolbarPane().Top().Row(1).Position(1).Floatable(false));
	manager->AddPane(brushes_toolbar, wxAuiPaneInfo().Name(BRUSHES_BAR_NAME).ToolbarPane().Top().Row(1).Position(2).Floatable(false));

	// Connect Events
	standard_toolbar->Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainToolBar::OnStandardButtonClick), NULL, this);
	brushes_toolbar->Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainToolBar::OnBrushesButtonClick), NULL, this);

	HideAll();
}

MainToolBar::~MainToolBar()
{
	// Disconnect Events
	standard_toolbar->Disconnect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainToolBar::OnStandardButtonClick), NULL, this);
	brushes_toolbar->Disconnect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainToolBar::OnBrushesButtonClick), NULL, this);
}

void MainToolBar::UpdateButtons()
{
	Editor* editor = g_gui.GetCurrentEditor();
	if (editor) {
		standard_toolbar->EnableTool(wxID_UNDO, editor->actionQueue->canUndo());
		standard_toolbar->EnableTool(wxID_REDO, editor->actionQueue->canRedo());
		standard_toolbar->EnableTool(wxID_PASTE, editor->copybuffer.canPaste());
	} else {
		standard_toolbar->EnableTool(wxID_UNDO, false);
		standard_toolbar->EnableTool(wxID_REDO, false);
		standard_toolbar->EnableTool(wxID_PASTE, false);
	}

	bool has_map = editor != nullptr;
	bool is_host = has_map && !editor->IsLiveClient();

	standard_toolbar->EnableTool(wxID_SAVE, is_host);
	standard_toolbar->EnableTool(wxID_SAVEAS, is_host);
	standard_toolbar->EnableTool(wxID_CUT, has_map);
	standard_toolbar->EnableTool(wxID_COPY, has_map);

	brushes_toolbar->EnableTool(PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL, has_map);
	brushes_toolbar->EnableTool(PALETTE_TERRAIN_ERASER, has_map);
	brushes_toolbar->EnableTool(PALETTE_TERRAIN_PZ_TOOL, has_map);
	brushes_toolbar->EnableTool(PALETTE_TERRAIN_NOPVP_TOOL, has_map);
	brushes_toolbar->EnableTool(PALETTE_TERRAIN_NOLOGOUT_TOOL, has_map);
	brushes_toolbar->EnableTool(PALETTE_TERRAIN_PVPZONE_TOOL, has_map);
	brushes_toolbar->EnableTool(PALETTE_TERRAIN_NORMAL_DOOR, has_map);
	brushes_toolbar->EnableTool(PALETTE_TERRAIN_LOCKED_DOOR, has_map);
	brushes_toolbar->EnableTool(PALETTE_TERRAIN_MAGIC_DOOR, has_map);
	brushes_toolbar->EnableTool(PALETTE_TERRAIN_QUEST_DOOR, has_map);
	brushes_toolbar->EnableTool(PALETTE_TERRAIN_HATCH_DOOR, has_map);
	brushes_toolbar->EnableTool(PALETTE_TERRAIN_WINDOW_DOOR, has_map);
}

void MainToolBar::UpdateBrushButtons()
{
	Brush* brush = g_gui.GetCurrentBrush();
	if (brush) {
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL, brush == g_gui.optional_brush);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_ERASER, brush == g_gui.eraser);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_PZ_TOOL, brush == g_gui.pz_brush);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_NOPVP_TOOL, brush == g_gui.rook_brush);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_NOLOGOUT_TOOL, brush == g_gui.nolog_brush);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_PVPZONE_TOOL, brush == g_gui.pvp_brush);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_NORMAL_DOOR, brush == g_gui.normal_door_brush);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_LOCKED_DOOR, brush == g_gui.locked_door_brush);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_MAGIC_DOOR, brush == g_gui.magic_door_brush);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_QUEST_DOOR, brush == g_gui.quest_door_brush);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_HATCH_DOOR, brush == g_gui.hatch_door_brush);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_WINDOW_DOOR, brush == g_gui.window_door_brush);
	} else {
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL, false);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_ERASER, false);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_PZ_TOOL, false);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_NOPVP_TOOL, false);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_NOLOGOUT_TOOL, false);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_PVPZONE_TOOL, false);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_NORMAL_DOOR, false);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_LOCKED_DOOR, false);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_MAGIC_DOOR, false);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_QUEST_DOOR, false);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_HATCH_DOOR, false);
		brushes_toolbar->ToggleTool(PALETTE_TERRAIN_WINDOW_DOOR, false);
	}
	g_gui.GetAuiManager()->Update();
}

void MainToolBar::Show(ToolBarID id, bool show)
{
	wxAuiManager* manager = g_gui.GetAuiManager();
	if (manager) {
		wxAuiPaneInfo& pane = GetPane(id);
		if (pane.IsOk()) {
			pane.Show(show);
			manager->Update();
		}
	}
}

void MainToolBar::HideAll(bool update)
{
	wxAuiManager* manager = g_gui.GetAuiManager();
	if (!manager)
		return;

	wxAuiPaneInfoArray& panes = manager->GetAllPanes();
	for (int i = 0, count = panes.GetCount(); i < count; ++i) {
		if (!panes.Item(i).IsToolbar())
			panes.Item(i).Hide();
	}

	if (update)
		manager->Update();
}

void MainToolBar::LoadPerspective()
{
	wxAuiManager* manager = g_gui.GetAuiManager();
	if (!manager)
		return;

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_STANDARD)) {
		std::string standard = g_settings.getString(Config::TOOLBAR_STANDARD_LAYOUT);
		if (!standard.empty())
			manager->LoadPaneInfo(wxString(standard), GetPane(TOOLBAR_STANDARD));
		GetPane(TOOLBAR_STANDARD).Show();
	} else
		GetPane(TOOLBAR_STANDARD).Hide();
		
	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_BRUSHES)) {
		std::string brushes = g_settings.getString(Config::TOOLBAR_BRUSHES_LAYOUT);
		if (!brushes.empty())
			manager->LoadPaneInfo(wxString(brushes), GetPane(TOOLBAR_BRUSHES));
		GetPane(TOOLBAR_BRUSHES).Show();
	} else
		GetPane(TOOLBAR_BRUSHES).Hide();

	manager->Update();
}

void MainToolBar::SavePerspective()
{
	wxAuiManager* manager = g_gui.GetAuiManager();
	if (!manager)
		return;

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_STANDARD)) {
		wxString standard = manager->SavePaneInfo(GetPane(TOOLBAR_STANDARD));
		g_settings.setString(Config::TOOLBAR_STANDARD_LAYOUT, standard.ToStdString());
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_BRUSHES)) {
		wxString brushes = manager->SavePaneInfo(GetPane(TOOLBAR_BRUSHES));
		g_settings.setString(Config::TOOLBAR_BRUSHES_LAYOUT, brushes.ToStdString());
	}
}

void MainToolBar::OnStandardButtonClick(wxCommandEvent& event)
{
	switch (event.GetId()) {
		case wxID_NEW:
			g_gui.NewMap();
			break;
		case wxID_OPEN:
			g_gui.OpenMap();
			break;
		case wxID_SAVE:
			g_gui.SaveMap();
			break;
		case wxID_SAVEAS:
			g_gui.SaveMapAs();
			break;
		case wxID_UNDO:
			g_gui.DoUndo();
			break;
		case wxID_REDO:
			g_gui.DoRedo();
			break;
		case wxID_CUT:
			g_gui.DoCut();
			break;
		case wxID_COPY:
			g_gui.DoCopy();
			break;
		case wxID_PASTE:
			g_gui.PreparePaste();
			break;
		default:
			break;
	}
}

void MainToolBar::OnBrushesButtonClick(wxCommandEvent& event)
{
	if (!g_gui.IsEditorOpen())
		return;

	switch (event.GetId()) {
		case PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL:
			g_gui.SelectBrush(g_gui.optional_brush);
			break;
		case PALETTE_TERRAIN_ERASER:
			g_gui.SelectBrush(g_gui.eraser);
			break;
		case PALETTE_TERRAIN_PZ_TOOL:
			g_gui.SelectBrush(g_gui.pz_brush);
			break;
		case PALETTE_TERRAIN_NOPVP_TOOL:
			g_gui.SelectBrush(g_gui.rook_brush);
			break;
		case PALETTE_TERRAIN_NOLOGOUT_TOOL:
			g_gui.SelectBrush(g_gui.nolog_brush);
			break;
		case PALETTE_TERRAIN_PVPZONE_TOOL:
			g_gui.SelectBrush(g_gui.pvp_brush);
			break;
		case PALETTE_TERRAIN_NORMAL_DOOR:
			g_gui.SelectBrush(g_gui.normal_door_brush);
			break;
		case PALETTE_TERRAIN_LOCKED_DOOR:
			g_gui.SelectBrush(g_gui.locked_door_brush);
			break;
		case PALETTE_TERRAIN_MAGIC_DOOR:
			g_gui.SelectBrush(g_gui.magic_door_brush);
			break;
		case PALETTE_TERRAIN_QUEST_DOOR:
			g_gui.SelectBrush(g_gui.quest_door_brush);
			break;
		case PALETTE_TERRAIN_HATCH_DOOR:
			g_gui.SelectBrush(g_gui.hatch_door_brush);
			break;
		case PALETTE_TERRAIN_WINDOW_DOOR:
			g_gui.SelectBrush(g_gui.window_door_brush);
			break;
		default:
			break;
	}
}

wxAuiPaneInfo& MainToolBar::GetPane(ToolBarID id)
{
	wxAuiManager* manager = g_gui.GetAuiManager();
	if (!manager)
		return wxAuiNullPaneInfo;

	switch (id) {
		case TOOLBAR_STANDARD:
			return manager->GetPane(STANDARD_BAR_NAME);
		case TOOLBAR_BRUSHES:
			return manager->GetPane(BRUSHES_BAR_NAME);
		default:
			return wxAuiNullPaneInfo;
	}
}