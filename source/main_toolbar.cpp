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

#include <wx/artprov.h>

const wxString MainToolBar::STANDARD_BAR_NAME = "standard_toolbar";

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

	manager->AddPane(standard_toolbar, wxAuiPaneInfo().Name(STANDARD_BAR_NAME).ToolbarPane().Top().Row(1).Position(1).Floatable(false));

	// Connect Events
	standard_toolbar->Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainToolBar::OnButtonClick), NULL, this);

	HideAll();
}

MainToolBar::~MainToolBar()
{
	// Disconnect Events
	standard_toolbar->Disconnect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainToolBar::OnButtonClick), NULL, this);
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
		manager->Update();
	} else {
		GetPane(TOOLBAR_STANDARD).Hide();
		manager->Update();
	}
		
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
}

void MainToolBar::OnButtonClick(wxCommandEvent& event)
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

wxAuiPaneInfo& MainToolBar::GetPane(ToolBarID id)
{
	wxAuiManager* manager = g_gui.GetAuiManager();
	if (!manager)
		return wxAuiNullPaneInfo;

	switch (id) {
		case TOOLBAR_STANDARD:
			return manager->GetPane(STANDARD_BAR_NAME);
			break;
		default:
			return wxAuiNullPaneInfo;
	}
}