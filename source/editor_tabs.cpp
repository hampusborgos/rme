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
	if(!notebook)
		return;

	int s;
	if (forward)
		s = (notebook->GetSelection() + 1) % notebook->GetPageCount();
	else
		s = (notebook->GetSelection() - 1 + notebook->GetPageCount()) % notebook->GetPageCount();
	notebook->SetSelection(s);
}

void MapTabbook::OnNotebookPageClose(wxAuiNotebookEvent& evt)
{
	MapTab* mt = dynamic_cast<MapTab*>(GetTab(evt.GetInt()));
	if(mt && mt->IsUniqueReference() && mt->GetMap())
	{
		if(mt->GetEditor()->IsLive()) {
			if(mt->GetMap()->hasChanged()) {
				SetFocusedTab(evt.GetInt());
				if(gui.root->DoQuerySave(false)) {
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

	LiveLogTab* lt = dynamic_cast<LiveLogTab*>(GetTab(evt.GetInt()));
	if(lt && lt->IsConnected()) {
		evt.Veto();
	}
}

void MapTabbook::OnNotebookPageChanged(wxAuiNotebookEvent& evt)
{
	gui.UpdateMinimap();
	MapTab* mt_old = dynamic_cast<MapTab*>(evt.GetOldSelection() != -1? GetTab(evt.GetOldSelection()) : nullptr);
	MapTab* mt_new = dynamic_cast<MapTab*>(evt.GetSelection() != -1? GetTab(evt.GetSelection()) : nullptr);

	int old = evt.GetOldSelection();
	int nnew = evt.GetSelection();

	//std::cout << old << nnew;

	if((mt_old && mt_new && mt_old->HasSameReference(mt_new) == false) || (!mt_old && mt_new))
	{
		gui.RefreshPalettes(mt_new->GetMap());
		gui.UpdateMenus();
	}
	else if(!mt_new)
	{
		gui.RefreshPalettes(nullptr);
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
	if(GetTabCount() == 0)
		return nullptr;

	return dynamic_cast<EditorTab*>(GetInternalTab(GetSelection()));
}

EditorTab* MapTabbook::GetTab(int idx)
{
	return GetInternalTab(idx);
}

wxWindow* MapTabbook::GetCurrentPage()
{
	if(GetTabCount() == 0)
		return nullptr;

	return GetCurrentTab()->GetWindow();
}

void MapTabbook::OnSwitchEditorMode(EditorMode mode)
{
	for(int i = 0; i < GetTabCount(); ++i)
		if(GetTab(i))
			GetTab(i)->OnSwitchEditorMode(mode);
}


void MapTabbook::SetTabLabel(int idx, wxString label)
{
	if(notebook)
		notebook->SetPageText(idx, label);
}

void MapTabbook::DeleteTab(int idx)
{
	if(notebook)
		notebook->DeletePage(idx);
}

int MapTabbook::GetTabCount()
{
	return notebook? notebook->GetPageCount() : 0;
}

int MapTabbook::GetTabIndex(wxWindow* w)
{
	return notebook? notebook->GetPageIndex(w) : 0;
}

int MapTabbook::GetSelection()
{
	return notebook? notebook->GetSelection() : 0;
}
