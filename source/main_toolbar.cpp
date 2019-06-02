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

#include <wx/artprov.h>

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

	default_toolbar = newd wxAuiToolBar(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxAUI_TB_DEFAULT_STYLE);
	default_toolbar->SetToolBitmapSize(wxSize(16, 16));
	default_toolbar->AddTool(wxID_NEW, wxEmptyString, new_bitmap, wxNullBitmap, wxITEM_NORMAL, "New Map", wxEmptyString, NULL);
	default_toolbar->AddTool(wxID_OPEN, wxEmptyString, open_bitmap, wxNullBitmap, wxITEM_NORMAL, "Open Map", wxEmptyString, NULL);
	default_toolbar->AddTool(wxID_SAVE, wxEmptyString, save_bitmap, wxNullBitmap, wxITEM_NORMAL, "Save Map", wxEmptyString, NULL);
	default_toolbar->AddTool(wxID_SAVEAS, wxEmptyString, saveas_bitmap, wxNullBitmap, wxITEM_NORMAL, "Save Map As...", wxEmptyString, NULL);
	default_toolbar->AddSeparator();
	default_toolbar->AddTool(wxID_UNDO, wxEmptyString, undo_bitmap, wxNullBitmap, wxITEM_NORMAL, "Undo", wxEmptyString, NULL);
	default_toolbar->AddTool(wxID_REDO, wxEmptyString, redo_bitmap, wxNullBitmap, wxITEM_NORMAL, "Redo", wxEmptyString, NULL);
	default_toolbar->AddSeparator();
	default_toolbar->AddTool(wxID_CUT, wxEmptyString, cut_bitmap, wxNullBitmap, wxITEM_NORMAL, "Cut", wxEmptyString, NULL);
	default_toolbar->AddTool(wxID_COPY, wxEmptyString, copy_bitmap, wxNullBitmap, wxITEM_NORMAL, "Copy", wxEmptyString, NULL);
	default_toolbar->AddTool(wxID_PASTE, wxEmptyString, paste_bitmap, wxNullBitmap, wxITEM_NORMAL, "Paste", wxEmptyString, NULL);
	default_toolbar->Realize();

	manager->AddPane(default_toolbar, wxAuiPaneInfo().Name("default_toolbar").ToolbarPane().Top().Row(1).Position(1).Floatable(false));

	// Connect Events
	default_toolbar->Connect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainToolBar::OnButtonClick), NULL, this);
}

MainToolBar::~MainToolBar()
{
	// Disconnect Events
	default_toolbar->Disconnect(wxEVT_COMMAND_MENU_SELECTED, wxCommandEventHandler(MainToolBar::OnButtonClick), NULL, this);
}

void MainToolBar::UpdateButtons()
{
	Editor* editor = g_gui.GetCurrentEditor();
	if (editor) {
		default_toolbar->EnableTool(wxID_UNDO, editor->actionQueue->canUndo());
		default_toolbar->EnableTool(wxID_REDO, editor->actionQueue->canRedo());
		default_toolbar->EnableTool(wxID_PASTE, editor->copybuffer.canPaste());
	} else {
		default_toolbar->EnableTool(wxID_UNDO, false);
		default_toolbar->EnableTool(wxID_REDO, false);
		default_toolbar->EnableTool(wxID_PASTE, false);
	}

	bool has_map = editor != nullptr;
	bool is_host = has_map && !editor->IsLiveClient();

	default_toolbar->EnableTool(wxID_SAVE, is_host);
	default_toolbar->EnableTool(wxID_SAVEAS, is_host);
	default_toolbar->EnableTool(wxID_CUT, has_map);
	default_toolbar->EnableTool(wxID_COPY, has_map);
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
