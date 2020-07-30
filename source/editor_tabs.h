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

#ifndef RME_EDITOR_TABS_H_
#define RME_EDITOR_TABS_H_

#include "gui_ids.h"

class EditorTab;

class MapTabbook : public wxPanel
{
public:
	MapTabbook(wxWindow* parent, wxWindowID id);
	~MapTabbook();

	// Wrapper functions
	void AddTab(EditorTab* tab, bool select);
	void SetTabLabel(int idx, wxString label);
	void SetFocusedTab(int idx);
	void DeleteTab(int idx);
	int GetTabCount();
	int GetTabIndex(wxWindow* tab);
	int GetSelection();
	void CycleTab(bool forward = true);
	wxWindow* GetCurrentPage();
	EditorTab* GetCurrentTab();
	EditorTab* GetTab(int idx);

	// Events
	void OnAllowNotebookDND(wxAuiNotebookEvent& evt);
	void OnNotebookPageClose(wxAuiNotebookEvent& evt);
	void OnNotebookPageChanged(wxAuiNotebookEvent& evt);
	void OnSwitchEditorMode(EditorMode mode);

protected:
	EditorTab* GetInternalTab(int idx);
	wxAuiNotebook* notebook;
	std::map<wxWindow*, EditorTab*> conv;

	DECLARE_EVENT_TABLE();
};

class EditorTab
{
public:
	EditorTab();
	virtual ~EditorTab();

	// Properties
	virtual wxWindow* GetWindow() const = 0;
	virtual wxString GetTitle() const = 0;

	//
	virtual void OnSwitchEditorMode(EditorMode mode) {}
};

#endif
