//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
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
