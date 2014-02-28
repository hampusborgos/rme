//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "editor_tabs.h"
#include "editor.h"
#include "live_tab.h"


EditorTab::EditorTab() {
	;
}

EditorTab::~EditorTab() {
	;
}

BEGIN_EVENT_TABLE(MapTabbook, wxPanel)
	EVT_AUINOTEBOOK_PAGE_CLOSE(wxID_ANY, MapTabbook::OnNotebookPageClose)
	EVT_AUINOTEBOOK_PAGE_CHANGED(wxID_ANY, MapTabbook::OnNotebookPageChanged)
END_EVENT_TABLE()

MapTabbook::MapTabbook(wxWindow *parent, wxWindowID id) :
	wxPanel(parent, id, wxDefaultPosition, wxDefaultSize)
{
	wxSizer* wxz = newd wxBoxSizer(wxHORIZONTAL);
	notebook = newd wxAuiNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	wxz->Add(notebook, 1, wxEXPAND);
	SetSizerAndFit(wxz);
}

MapTabbook::~MapTabbook() 
{
	;
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

void MapTabbook::OnNotebookPageClose(wxAuiNotebookEvent& evt)
{
	EditorTab* editorTab = GetTab(evt.GetInt());

	MapTab* mapTab = dynamic_cast<MapTab*>(editorTab);
	if (mapTab && mapTab->IsUniqueReference() && mapTab->GetMap()) {
		if (mapTab->GetEditor()->IsLive()) {
			if (mapTab->GetMap()->hasChanged()) {
				SetFocusedTab(evt.GetInt());
				if (gui.root->DoQuerySave(false)) {
					gui.RefreshPalettes(nullptr, false);
					gui.UpdateMenus();
				} else {
					evt.Veto();
				}
			} else {
				gui.RefreshPalettes(nullptr, false);
				gui.UpdateMenus();
			}
		}
		return;
	}

	LiveLogTab* lt = dynamic_cast<LiveLogTab*>(editorTab);
	if (lt && lt->IsConnected()) {
		evt.Veto();
	}
}

void MapTabbook::OnNotebookPageChanged(wxAuiNotebookEvent& evt)
{
	gui.UpdateMinimap();

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

	// std::cout << oldSelection << " " << newSelection;
	if (!newMapTab) {
		gui.RefreshPalettes(nullptr);
	} else if (!oldMapTab || !oldMapTab->HasSameReference(newMapTab)) {
		gui.RefreshPalettes(newMapTab->GetMap());
		gui.UpdateMenus();
	}
}

void MapTabbook::OnAllowNotebookDND(wxAuiNotebookEvent& evt)
{
	evt.Allow();
}

// Wrappers

void MapTabbook::AddTab(EditorTab* tab, bool select)
{
	tab->GetWindow()->Reparent(notebook);
	notebook->AddPage(tab->GetWindow(), tab->GetTitle(), select);
	conv[tab->GetWindow()] = tab;
}

void MapTabbook::SetFocusedTab(int idx)
{
	notebook->SetSelection(idx);
}

EditorTab* MapTabbook::GetInternalTab(int idx)
{
	return conv[notebook->GetPage(idx)];
}

EditorTab* MapTabbook::GetCurrentTab()
{
	if (GetTabCount() == 0 || GetSelection() == -1) {
		return nullptr;
	}
	return dynamic_cast<EditorTab*>(GetInternalTab(GetSelection()));
}

EditorTab* MapTabbook::GetTab(int idx)
{
	return GetInternalTab(idx);
}

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
