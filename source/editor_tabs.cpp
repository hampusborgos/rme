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

#include "editor_tabs.h"

#include "editor.h"
#include "live_tab.h"
#include "main.h"

EditorTab::EditorTab() { ; }

EditorTab::~EditorTab() { ; }

BEGIN_EVENT_TABLE(MapTabbook, wxPanel)
EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY, MapTabbook::OnNotebookPageClose)
EVT_AUINOTEBOOK_PAGE_CHANGED(wxID_ANY, MapTabbook::OnNotebookPageChanged)
END_EVENT_TABLE()

MapTabbook::MapTabbook(wxWindow* parent, wxWindowID id) : wxPanel(parent, id, wxDefaultPosition, wxDefaultSize)
{
	wxSizer* wxz = newd wxBoxSizer(wxHORIZONTAL);
	notebook = newd wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	wxz->Add(notebook, 1, wxEXPAND);
	SetSizerAndFit(wxz);
}

void MapTabbook::CycleTab(bool forward)
{
	if (!notebook) {
		return;
	}

	int32_t pageCount = notebook->GetPageCount();
	int32_t currentSelection = notebook->GetSelection();

	int32_t selection;
	if (forward) {
		selection = (currentSelection + 1) % pageCount;
	} else {
		selection = (currentSelection - 1 + pageCount) % pageCount;
	}
	notebook->SetSelection(selection);
}

void MapTabbook::OnNotebookPageClose(wxAuiNotebookEvent& event)
{
	EditorTab* editor_tab = GetTab(event.GetInt());

	MapTab* map_tab = dynamic_cast<MapTab*>(editor_tab);
	if (map_tab && map_tab->IsUniqueReference() && map_tab->GetMap()) {
		bool need_refresh = true;
		Editor* editor = map_tab->GetEditor();
		if (editor->IsLive()) {
			if (editor->hasChanges()) {
				SetFocusedTab(event.GetInt());
				if (!g_gui.root->DoQuerySave(false)) {
					need_refresh = false;
					event.Veto();
				}
			}
		} else if (editor->hasChanges()) {
			SetFocusedTab(event.GetInt());
			if (!g_gui.root->DoQuerySave()) {
				need_refresh = false;
				event.Veto();
			}
		}

		if (need_refresh) {
			g_gui.RefreshPalettes(nullptr, false);
			g_gui.UpdateMenus();
		}
		return;
	}

	LiveLogTab* live_tab = dynamic_cast<LiveLogTab*>(editor_tab);
	if (live_tab && live_tab->IsConnected()) {
		event.Veto();
	}
}

void MapTabbook::OnNotebookPageChanged(wxAuiNotebookEvent& evt)
{
	g_gui.UpdateMinimap();

	int32_t oldSelection = evt.GetOldSelection();
	int32_t newSelection = evt.GetSelection();

	MapTab* oldMapTab;
	if (oldSelection != -1) {
		oldMapTab = dynamic_cast<MapTab*>(GetTab(oldSelection));
	} else {
		oldMapTab = nullptr;
	}

	MapTab* newMapTab;
	if (newSelection != -1) {
		newMapTab = dynamic_cast<MapTab*>(GetTab(newSelection));
	} else {
		newMapTab = nullptr;
	}

	if (!newMapTab) {
		g_gui.RefreshPalettes(nullptr);
	} else if (!oldMapTab || !oldMapTab->HasSameReference(newMapTab)) {
		g_gui.RefreshPalettes(newMapTab->GetMap());
		g_gui.UpdateMenus();
	}

	if (oldMapTab) oldMapTab->VisibilityCheck();
	if (newMapTab) newMapTab->VisibilityCheck();
}

void MapTabbook::OnAllowNotebookDND(wxAuiNotebookEvent& evt) { evt.Allow(); }

// Wrappers

void MapTabbook::AddTab(EditorTab* tab, bool select)
{
	tab->GetWindow()->Reparent(notebook);
	notebook->AddPage(tab->GetWindow(), tab->GetTitle(), select);
	conv[tab->GetWindow()] = tab;
}

void MapTabbook::SetFocusedTab(int idx) { notebook->SetSelection(idx); }

EditorTab* MapTabbook::GetInternalTab(int idx) { return conv[notebook->GetPage(idx)]; }

EditorTab* MapTabbook::GetCurrentTab()
{
	if (GetTabCount() == 0 || GetSelection() == -1) {
		return nullptr;
	}
	return dynamic_cast<EditorTab*>(GetInternalTab(GetSelection()));
}

EditorTab* MapTabbook::GetTab(int index) { return GetInternalTab(index); }

wxWindow* MapTabbook::GetCurrentPage()
{
	if (GetTabCount() == 0) {
		return nullptr;
	}
	return GetCurrentTab()->GetWindow();
}

void MapTabbook::OnSwitchEditorMode(EditorMode mode)
{
	for (int32_t i = 0; i < GetTabCount(); ++i) {
		EditorTab* editorTab = GetTab(i);
		if (editorTab) {
			editorTab->OnSwitchEditorMode(mode);
		}
	}
}

void MapTabbook::SetTabLabel(int idx, wxString label)
{
	if (notebook) {
		notebook->SetPageText(idx, label);
	}
}

void MapTabbook::DeleteTab(int idx)
{
	if (notebook) {
		notebook->DeletePage(idx);
	}
}

int MapTabbook::GetTabCount()
{
	if (notebook) {
		return notebook->GetPageCount();
	}
	return 0;
}

int MapTabbook::GetTabIndex(wxWindow* w)
{
	if (notebook) {
		return notebook->GetPageIndex(w);
	}
	return 0;
}

int MapTabbook::GetSelection()
{
	if (notebook) {
		return notebook->GetSelection();
	}
	return 0;
}
