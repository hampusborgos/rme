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
#include "artprovider.h"

#include <wx/artprov.h>
#include <wx/mstream.h>

const wxString MainToolBar::STANDARD_BAR_NAME = "standard_toolbar";
const wxString MainToolBar::BRUSHES_BAR_NAME = "brushes_toolbar";
const wxString MainToolBar::POSITION_BAR_NAME = "position_toolbar";
const wxString MainToolBar::SIZES_BAR_NAME = "sizes_toolbar";

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
	wxSize icon_size = wxSize(16, 16);
	wxBitmap new_bitmap = wxArtProvider::GetBitmap(wxART_NEW, wxART_TOOLBAR, icon_size);
	wxBitmap open_bitmap = wxArtProvider::GetBitmap(wxART_FILE_OPEN, wxART_TOOLBAR, icon_size);
	wxBitmap save_bitmap = wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR, icon_size);
	wxBitmap saveas_bitmap = wxArtProvider::GetBitmap(wxART_FILE_SAVE_AS, wxART_TOOLBAR, icon_size);
	wxBitmap undo_bitmap = wxArtProvider::GetBitmap(wxART_UNDO, wxART_TOOLBAR, icon_size);
	wxBitmap redo_bitmap = wxArtProvider::GetBitmap(wxART_REDO, wxART_TOOLBAR, icon_size);
	wxBitmap cut_bitmap = wxArtProvider::GetBitmap(wxART_CUT, wxART_TOOLBAR, icon_size);
	wxBitmap copy_bitmap = wxArtProvider::GetBitmap(wxART_COPY, wxART_TOOLBAR, icon_size);
	wxBitmap paste_bitmap = wxArtProvider::GetBitmap(wxART_PASTE, wxART_TOOLBAR, icon_size);

	standard_toolbar = newd wxAuiToolBar(parent, TOOLBAR_STANDARD, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	standard_toolbar->SetToolBitmapSize(icon_size);
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
	wxBitmap pz_bitmap = wxArtProvider::GetBitmap(ART_PZ_BRUSH, wxART_TOOLBAR, icon_size);
	wxBitmap nopvp_bitmap = wxArtProvider::GetBitmap(ART_NOPVP_BRUSH, wxART_TOOLBAR, icon_size);
	wxBitmap nologout_bitmap = wxArtProvider::GetBitmap(ART_NOLOOUT_BRUSH, wxART_TOOLBAR, icon_size);
	wxBitmap pvp_bitmap = wxArtProvider::GetBitmap(ART_PVP_BRUSH, wxART_TOOLBAR, icon_size);
	wxBitmap* normal_bitmap = loadPNGFile(door_normal_small_png);
	wxBitmap* locked_bitmap = loadPNGFile(door_locked_small_png);
	wxBitmap* magic_bitmap = loadPNGFile(door_magic_small_png);
	wxBitmap* quest_bitmap = loadPNGFile(door_quest_small_png);
	wxBitmap* hatch_bitmap = loadPNGFile(window_hatch_small_png);
	wxBitmap* window_bitmap = loadPNGFile(window_normal_small_png);

	brushes_toolbar = newd wxAuiToolBar(parent, TOOLBAR_BRUSHES, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	brushes_toolbar->SetToolBitmapSize(icon_size);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_OPTIONAL_BORDER_TOOL, wxEmptyString, *border_bitmap, wxNullBitmap, wxITEM_CHECK, "Border", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_ERASER, wxEmptyString, *eraser_bitmap, wxNullBitmap, wxITEM_CHECK, "Eraser", wxEmptyString, NULL);
	brushes_toolbar->AddSeparator();
	brushes_toolbar->AddTool(PALETTE_TERRAIN_PZ_TOOL, wxEmptyString, pz_bitmap, wxNullBitmap, wxITEM_CHECK, "Protected Zone", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_NOPVP_TOOL, wxEmptyString, nopvp_bitmap, wxNullBitmap, wxITEM_CHECK, "No PvP Zone", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_NOLOGOUT_TOOL, wxEmptyString, nologout_bitmap, wxNullBitmap, wxITEM_CHECK, "No Logout Zone", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_PVPZONE_TOOL, wxEmptyString, pvp_bitmap, wxNullBitmap, wxITEM_CHECK, "PvP Zone", wxEmptyString, NULL);
	brushes_toolbar->AddSeparator();
	brushes_toolbar->AddTool(PALETTE_TERRAIN_NORMAL_DOOR, wxEmptyString, *normal_bitmap, wxNullBitmap, wxITEM_CHECK, "Normal Door", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_LOCKED_DOOR, wxEmptyString, *locked_bitmap, wxNullBitmap, wxITEM_CHECK, "Locked Door", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_MAGIC_DOOR, wxEmptyString, *magic_bitmap, wxNullBitmap, wxITEM_CHECK, "Magic Door", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_QUEST_DOOR, wxEmptyString, *quest_bitmap, wxNullBitmap, wxITEM_CHECK, "Quest Door", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_HATCH_DOOR, wxEmptyString, *hatch_bitmap, wxNullBitmap, wxITEM_CHECK, "Hatch Window", wxEmptyString, NULL);
	brushes_toolbar->AddTool(PALETTE_TERRAIN_WINDOW_DOOR, wxEmptyString, *window_bitmap, wxNullBitmap, wxITEM_CHECK, "Window", wxEmptyString, NULL);
	brushes_toolbar->Realize();

	wxBitmap go_bitmap = wxArtProvider::GetBitmap(ART_POSITION_GO, wxART_TOOLBAR, icon_size);

	position_toolbar = newd wxAuiToolBar(parent, TOOLBAR_POSITION, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE | wxAUI_TB_HORZ_TEXT);
	position_toolbar->SetToolBitmapSize(icon_size);
	x_control = newd NumberTextCtrl(position_toolbar, wxID_ANY, 0, 0, MAP_MAX_WIDTH, wxTE_PROCESS_ENTER, "X", wxDefaultPosition, wxSize(60, 20));
	x_control->SetToolTip("X Coordinate");
	y_control = newd NumberTextCtrl(position_toolbar, wxID_ANY, 0, 0, MAP_MAX_HEIGHT, wxTE_PROCESS_ENTER, "Y", wxDefaultPosition, wxSize(60, 20));
	y_control->SetToolTip("Y Coordinate");
	z_control = newd NumberTextCtrl(position_toolbar, wxID_ANY, 0, 0, MAP_MAX_LAYER, wxTE_PROCESS_ENTER, "Z", wxDefaultPosition, wxSize(35, 20));
	z_control->SetToolTip("Z Coordinate");
	go_button = newd wxButton(position_toolbar, TOOLBAR_POSITION_GO, wxEmptyString, wxDefaultPosition, wxSize(22, 20));
	go_button->SetBitmap(go_bitmap);
	go_button->SetToolTip("Go To Position");
	position_toolbar->AddControl(x_control);
	position_toolbar->AddControl(y_control);
	position_toolbar->AddControl(z_control);
	position_toolbar->AddControl(go_button);
	position_toolbar->Realize();

	wxBitmap circular_bitmap = wxArtProvider::GetBitmap(ART_CIRCULAR, wxART_TOOLBAR, icon_size);
	wxBitmap rectangular_bitmap = wxArtProvider::GetBitmap(ART_RECTANGULAR, wxART_TOOLBAR, icon_size);
	wxBitmap size1_bitmap = wxArtProvider::GetBitmap(ART_RECTANGULAR_1, wxART_TOOLBAR, icon_size);
	wxBitmap size2_bitmap = wxArtProvider::GetBitmap(ART_RECTANGULAR_2, wxART_TOOLBAR, icon_size);
	wxBitmap size3_bitmap = wxArtProvider::GetBitmap(ART_RECTANGULAR_3, wxART_TOOLBAR, icon_size);
	wxBitmap size4_bitmap = wxArtProvider::GetBitmap(ART_RECTANGULAR_4, wxART_TOOLBAR, icon_size);
	wxBitmap size5_bitmap = wxArtProvider::GetBitmap(ART_RECTANGULAR_5, wxART_TOOLBAR, icon_size);
	wxBitmap size6_bitmap = wxArtProvider::GetBitmap(ART_RECTANGULAR_6, wxART_TOOLBAR, icon_size);
	wxBitmap size7_bitmap = wxArtProvider::GetBitmap(ART_RECTANGULAR_7, wxART_TOOLBAR, icon_size);

	sizes_toolbar = newd wxAuiToolBar(parent, TOOLBAR_SIZES, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	sizes_toolbar->SetToolBitmapSize(icon_size);
	sizes_toolbar->AddTool(TOOLBAR_SIZES_RECTANGULAR, wxEmptyString, rectangular_bitmap, wxNullBitmap, wxITEM_CHECK, "Rectangular Brush", wxEmptyString, NULL);
	sizes_toolbar->AddTool(TOOLBAR_SIZES_CIRCULAR, wxEmptyString, circular_bitmap, wxNullBitmap, wxITEM_CHECK, "Circular Brush", wxEmptyString, NULL);
	sizes_toolbar->AddSeparator();
	sizes_toolbar->AddTool(TOOLBAR_SIZES_1, wxEmptyString, size1_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 1", wxEmptyString, NULL);
	sizes_toolbar->AddTool(TOOLBAR_SIZES_2, wxEmptyString, size2_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 2", wxEmptyString, NULL);
	sizes_toolbar->AddTool(TOOLBAR_SIZES_3, wxEmptyString, size3_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 3", wxEmptyString, NULL);
	sizes_toolbar->AddTool(TOOLBAR_SIZES_4, wxEmptyString, size4_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 4", wxEmptyString, NULL);
	sizes_toolbar->AddTool(TOOLBAR_SIZES_5, wxEmptyString, size5_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 5", wxEmptyString, NULL);
	sizes_toolbar->AddTool(TOOLBAR_SIZES_6, wxEmptyString, size6_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 6", wxEmptyString, NULL);
	sizes_toolbar->AddTool(TOOLBAR_SIZES_7, wxEmptyString, size7_bitmap, wxNullBitmap, wxITEM_CHECK, "Size 7", wxEmptyString, NULL);
	sizes_toolbar->Realize();
	sizes_toolbar->ToggleTool(TOOLBAR_SIZES_RECTANGULAR, true);
	sizes_toolbar->ToggleTool(TOOLBAR_SIZES_1, true);

	manager->AddPane(standard_toolbar, wxAuiPaneInfo().Name(STANDARD_BAR_NAME).ToolbarPane().Top().Row(1).Position(1).Floatable(false));
	manager->AddPane(brushes_toolbar, wxAuiPaneInfo().Name(BRUSHES_BAR_NAME).ToolbarPane().Top().Row(1).Position(2).Floatable(false));
	manager->AddPane(position_toolbar, wxAuiPaneInfo().Name(POSITION_BAR_NAME).ToolbarPane().Top().Row(1).Position(4).Floatable(false));
	manager->AddPane(sizes_toolbar, wxAuiPaneInfo().Name(SIZES_BAR_NAME).ToolbarPane().Top().Row(1).Position(3).Floatable(false));

	standard_toolbar->Bind(wxEVT_COMMAND_MENU_SELECTED, &MainToolBar::OnStandardButtonClick, this);
	brushes_toolbar->Bind(wxEVT_COMMAND_MENU_SELECTED, &MainToolBar::OnBrushesButtonClick, this);
	x_control->Bind(wxEVT_TEXT_PASTE, &MainToolBar::OnPastePositionText, this);
	x_control->Bind(wxEVT_KEY_UP, &MainToolBar::OnPositionKeyUp, this);
	y_control->Bind(wxEVT_TEXT_PASTE, &MainToolBar::OnPastePositionText, this);
	y_control->Bind(wxEVT_KEY_UP, &MainToolBar::OnPositionKeyUp, this);
	z_control->Bind(wxEVT_TEXT_PASTE, &MainToolBar::OnPastePositionText, this);
	z_control->Bind(wxEVT_KEY_UP, &MainToolBar::OnPositionKeyUp, this);
	go_button->Bind(wxEVT_BUTTON, &MainToolBar::OnPositionButtonClick, this);
	sizes_toolbar->Bind(wxEVT_COMMAND_MENU_SELECTED, &MainToolBar::OnSizesButtonClick, this);

	HideAll();
}

MainToolBar::~MainToolBar()
{
	standard_toolbar->Unbind(wxEVT_COMMAND_MENU_SELECTED, &MainToolBar::OnStandardButtonClick, this);
	brushes_toolbar->Unbind(wxEVT_COMMAND_MENU_SELECTED, &MainToolBar::OnBrushesButtonClick, this);
	x_control->Unbind(wxEVT_TEXT_PASTE, &MainToolBar::OnPastePositionText, this);
	x_control->Unbind(wxEVT_KEY_UP, &MainToolBar::OnPositionKeyUp, this);
	y_control->Unbind(wxEVT_TEXT_PASTE, &MainToolBar::OnPastePositionText, this);
	y_control->Unbind(wxEVT_KEY_UP, &MainToolBar::OnPositionKeyUp, this);
	z_control->Unbind(wxEVT_TEXT_PASTE, &MainToolBar::OnPastePositionText, this);
	z_control->Unbind(wxEVT_KEY_UP, &MainToolBar::OnPositionKeyUp, this);
	go_button->Unbind(wxEVT_BUTTON, &MainToolBar::OnPositionButtonClick, this);
	sizes_toolbar->Unbind(wxEVT_COMMAND_MENU_SELECTED, &MainToolBar::OnSizesButtonClick, this);
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

	position_toolbar->EnableTool(TOOLBAR_POSITION_GO, has_map);
	x_control->Enable(has_map);
	y_control->Enable(has_map);
	z_control->Enable(has_map);

	if (has_map) {
		x_control->SetMaxValue(editor->getMapWidth());
		y_control->SetMaxValue(editor->getMapHeight());
	}

	sizes_toolbar->EnableTool(TOOLBAR_SIZES_CIRCULAR, has_map);
	sizes_toolbar->EnableTool(TOOLBAR_SIZES_RECTANGULAR, has_map);
	sizes_toolbar->EnableTool(TOOLBAR_SIZES_1, has_map);
	sizes_toolbar->EnableTool(TOOLBAR_SIZES_2, has_map);
	sizes_toolbar->EnableTool(TOOLBAR_SIZES_3, has_map);
	sizes_toolbar->EnableTool(TOOLBAR_SIZES_4, has_map);
	sizes_toolbar->EnableTool(TOOLBAR_SIZES_5, has_map);
	sizes_toolbar->EnableTool(TOOLBAR_SIZES_6, has_map);
	sizes_toolbar->EnableTool(TOOLBAR_SIZES_7, has_map);
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

void MainToolBar::UpdateBrushSize(BrushShape shape, int size)
{
	if (shape == BRUSHSHAPE_CIRCLE) {
		sizes_toolbar->ToggleTool(TOOLBAR_SIZES_CIRCULAR, true);
		sizes_toolbar->ToggleTool(TOOLBAR_SIZES_RECTANGULAR, false);

		wxSize icon_size = wxSize(16, 16);
		sizes_toolbar->SetToolBitmap(TOOLBAR_SIZES_1, wxArtProvider::GetBitmap(ART_CIRCULAR_1, wxART_TOOLBAR, icon_size));
		sizes_toolbar->SetToolBitmap(TOOLBAR_SIZES_2, wxArtProvider::GetBitmap(ART_CIRCULAR_2, wxART_TOOLBAR, icon_size));
		sizes_toolbar->SetToolBitmap(TOOLBAR_SIZES_3, wxArtProvider::GetBitmap(ART_CIRCULAR_3, wxART_TOOLBAR, icon_size));
		sizes_toolbar->SetToolBitmap(TOOLBAR_SIZES_4, wxArtProvider::GetBitmap(ART_CIRCULAR_4, wxART_TOOLBAR, icon_size));
		sizes_toolbar->SetToolBitmap(TOOLBAR_SIZES_5, wxArtProvider::GetBitmap(ART_CIRCULAR_5, wxART_TOOLBAR, icon_size));
		sizes_toolbar->SetToolBitmap(TOOLBAR_SIZES_6, wxArtProvider::GetBitmap(ART_CIRCULAR_6, wxART_TOOLBAR, icon_size));
		sizes_toolbar->SetToolBitmap(TOOLBAR_SIZES_7, wxArtProvider::GetBitmap(ART_CIRCULAR_7, wxART_TOOLBAR, icon_size));
	} else {
		sizes_toolbar->ToggleTool(TOOLBAR_SIZES_CIRCULAR, false);
		sizes_toolbar->ToggleTool(TOOLBAR_SIZES_RECTANGULAR, true);

		wxSize icon_size = wxSize(16, 16);
		sizes_toolbar->SetToolBitmap(TOOLBAR_SIZES_1, wxArtProvider::GetBitmap(ART_RECTANGULAR_1, wxART_TOOLBAR, icon_size));
		sizes_toolbar->SetToolBitmap(TOOLBAR_SIZES_2, wxArtProvider::GetBitmap(ART_RECTANGULAR_2, wxART_TOOLBAR, icon_size));
		sizes_toolbar->SetToolBitmap(TOOLBAR_SIZES_3, wxArtProvider::GetBitmap(ART_RECTANGULAR_3, wxART_TOOLBAR, icon_size));
		sizes_toolbar->SetToolBitmap(TOOLBAR_SIZES_4, wxArtProvider::GetBitmap(ART_RECTANGULAR_4, wxART_TOOLBAR, icon_size));
		sizes_toolbar->SetToolBitmap(TOOLBAR_SIZES_5, wxArtProvider::GetBitmap(ART_RECTANGULAR_5, wxART_TOOLBAR, icon_size));
		sizes_toolbar->SetToolBitmap(TOOLBAR_SIZES_6, wxArtProvider::GetBitmap(ART_RECTANGULAR_6, wxART_TOOLBAR, icon_size));
		sizes_toolbar->SetToolBitmap(TOOLBAR_SIZES_7, wxArtProvider::GetBitmap(ART_RECTANGULAR_7, wxART_TOOLBAR, icon_size));
	}

	sizes_toolbar->ToggleTool(TOOLBAR_SIZES_1, size == 0);
	sizes_toolbar->ToggleTool(TOOLBAR_SIZES_2, size == 1);
	sizes_toolbar->ToggleTool(TOOLBAR_SIZES_3, size == 2);
	sizes_toolbar->ToggleTool(TOOLBAR_SIZES_4, size == 4);
	sizes_toolbar->ToggleTool(TOOLBAR_SIZES_5, size == 6);
	sizes_toolbar->ToggleTool(TOOLBAR_SIZES_6, size == 8);
	sizes_toolbar->ToggleTool(TOOLBAR_SIZES_7, size == 11);

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
		std::string info = g_settings.getString(Config::TOOLBAR_STANDARD_LAYOUT);
		if (!info.empty())
			manager->LoadPaneInfo(wxString(info), GetPane(TOOLBAR_STANDARD));
		GetPane(TOOLBAR_STANDARD).Show();
	} else
		GetPane(TOOLBAR_STANDARD).Hide();
		
	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_BRUSHES)) {
		std::string info = g_settings.getString(Config::TOOLBAR_BRUSHES_LAYOUT);
		if (!info.empty())
			manager->LoadPaneInfo(wxString(info), GetPane(TOOLBAR_BRUSHES));
		GetPane(TOOLBAR_BRUSHES).Show();
	} else
		GetPane(TOOLBAR_BRUSHES).Hide();

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_POSITION)) {
		std::string info = g_settings.getString(Config::TOOLBAR_POSITION_LAYOUT);
		if (!info.empty())
			manager->LoadPaneInfo(wxString(info), GetPane(TOOLBAR_POSITION));
		GetPane(TOOLBAR_POSITION).Show();
	} else
		GetPane(TOOLBAR_POSITION).Hide();

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_SIZES)) {
		std::string info = g_settings.getString(Config::TOOLBAR_SIZES_LAYOUT);
		if (!info.empty())
			manager->LoadPaneInfo(wxString(info), GetPane(TOOLBAR_SIZES));
		GetPane(TOOLBAR_SIZES).Show();
	} else
		GetPane(TOOLBAR_SIZES).Hide();

	manager->Update();
}

void MainToolBar::SavePerspective()
{
	wxAuiManager* manager = g_gui.GetAuiManager();
	if (!manager)
		return;

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_STANDARD)) {
		wxString info = manager->SavePaneInfo(GetPane(TOOLBAR_STANDARD));
		g_settings.setString(Config::TOOLBAR_STANDARD_LAYOUT, info.ToStdString());
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_BRUSHES)) {
		wxString info = manager->SavePaneInfo(GetPane(TOOLBAR_BRUSHES));
		g_settings.setString(Config::TOOLBAR_BRUSHES_LAYOUT, info.ToStdString());
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_POSITION)) {
		wxString info = manager->SavePaneInfo(GetPane(TOOLBAR_POSITION));
		g_settings.setString(Config::TOOLBAR_POSITION_LAYOUT, info.ToStdString());
	}

	if (g_settings.getBoolean(Config::SHOW_TOOLBAR_SIZES)) {
		wxString info = manager->SavePaneInfo(GetPane(TOOLBAR_SIZES));
		g_settings.setString(Config::TOOLBAR_SIZES_LAYOUT, info.ToStdString());
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

void MainToolBar::OnPositionButtonClick(wxCommandEvent& event)
{
	if (!g_gui.IsEditorOpen())
		return;

	if (event.GetId() == TOOLBAR_POSITION_GO) {
		Position pos(x_control->GetIntValue(), y_control->GetIntValue(), z_control->GetIntValue());
		if (pos.isValid())
			g_gui.SetScreenCenterPosition(pos);
	}
}

void MainToolBar::OnPositionKeyUp(wxKeyEvent& event)
{
	if (event.GetKeyCode() == WXK_TAB) {
		if (x_control->HasFocus()) {
			y_control->SelectAll();
			y_control->SetFocus();
		} else if (y_control->HasFocus()) {
			z_control->SelectAll();
			z_control->SetFocus();
		} else if (z_control->HasFocus()) {
			go_button->SetFocus();
		}
	} else if (event.GetKeyCode() == WXK_NUMPAD_ENTER || event.GetKeyCode() == WXK_RETURN) {
		Position pos(x_control->GetIntValue(), y_control->GetIntValue(), z_control->GetIntValue());
		if (pos.isValid())
			g_gui.SetScreenCenterPosition(pos);
	}
	event.Skip();
}

void MainToolBar::OnPastePositionText(wxClipboardTextEvent& event)
{
	Position position;
	if (posFromClipboard(position.x, position.y, position.z)) {
		x_control->SetIntValue(position.x);
		y_control->SetIntValue(position.y);
		z_control->SetIntValue(position.z);
	} else
		event.Skip();
}

void MainToolBar::OnSizesButtonClick(wxCommandEvent& event)
{
	if (!g_gui.IsEditorOpen())
		return;

	switch (event.GetId()) {
		case TOOLBAR_SIZES_CIRCULAR:
			g_gui.SetBrushShape(BRUSHSHAPE_CIRCLE);
			break;
		case TOOLBAR_SIZES_RECTANGULAR:
			g_gui.SetBrushShape(BRUSHSHAPE_SQUARE);
			break;
		case TOOLBAR_SIZES_1:
			g_gui.SetBrushSize(0);
			break;
		case TOOLBAR_SIZES_2:
			g_gui.SetBrushSize(1);
			break;
		case TOOLBAR_SIZES_3:
			g_gui.SetBrushSize(2);
			break;
		case TOOLBAR_SIZES_4:
			g_gui.SetBrushSize(4);
			break;
		case TOOLBAR_SIZES_5:
			g_gui.SetBrushSize(6);
			break;
		case TOOLBAR_SIZES_6:
			g_gui.SetBrushSize(8);
			break;
		case TOOLBAR_SIZES_7:
			g_gui.SetBrushSize(11);
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
		case TOOLBAR_POSITION:
			return manager->GetPane(POSITION_BAR_NAME);
		case TOOLBAR_SIZES:
			return manager->GetPane(SIZES_BAR_NAME);
		default:
			return wxAuiNullPaneInfo;
	}
}