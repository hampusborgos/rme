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

#ifndef RME_PROPERTIES_WINDOW_H_
#define RME_PROPERTIES_WINDOW_H_

#include "main.h"

#include "common_windows.h"

class ContainerItemButton;
class ContainerItemPopupMenu;
class ItemAttribute;

class PropertiesWindow : public ObjectPropertiesWindowBase
{
public:
	PropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Item* item, wxPoint position = wxDefaultPosition);
	~PropertiesWindow();

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);
	void OnClickAddAttribute(wxCommandEvent&);
	void OnClickRemoveAttribute(wxCommandEvent&);

	void OnResize(wxSizeEvent&);
	void OnNotebookPageChanged(wxNotebookEvent&);
	void OnGridValueChanged(wxGridEvent&);

	void Update();

protected:
	// Simple pane
	wxWindow* createGeneralPanel(wxWindow* parent);
	void saveGeneralPanel();

	// Container pane
	std::vector<ContainerItemButton*> container_items;
	wxWindow* createContainerPanel(wxWindow* parent);
	void saveContainerPanel();

	// Advanced pane
	wxGrid* attributesGrid;
	wxWindow* createAttributesPanel(wxWindow *parent);
	void saveAttributesPanel();
	void SetGridValue(wxGrid* grid, int rowIndex, std::string name, const ItemAttribute& attr);

protected:
	wxNotebook* notebook;
	wxWindow* currentPanel;

	DECLARE_EVENT_TABLE()
};

#endif
