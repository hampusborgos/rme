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

class PropertiesWindow : public wxDialog
{
public:
	PropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Item* item, wxPoint position = wxDefaultPosition);
	virtual ~PropertiesWindow();

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);
	void OnClickAddAttribute(wxCommandEvent&);
	void OnClickRemoveAttribute(wxCommandEvent&);

	void OnResize(wxSizeEvent&);
	void OnNotebookPageChanged(wxNotebookEvent&);
	void OnGridValueChanged(wxGridEvent&);

protected:
	wxWindow* createGeneralPanel(wxWindow* parent);
	void saveGeneralPanel();

	wxWindow* createContainerPanel(wxWindow* parent);
	void saveContainerPanel();

	wxWindow* createAttributesPanel(wxWindow *parent);
	void saveAttributesPanel();

	void SetGridValue(wxGrid* grid, int rowIndex, std::string name, const ItemAttribute& attr);

protected:
	wxNotebook* notebook;
	wxWindow* currentPanel;
	wxGrid* attributesGrid;
	const Map* edit_map;
	const Tile* edit_tile;
	Item* edit_item;

	DECLARE_EVENT_TABLE();
};

#endif
