//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_PROPERTIES_WINDOW_H_
#define RME_PROPERTIES_WINDOW_H_

#include "main.h"

#include "common_windows.h"

class ContainerItemButton;
class ContainerItemPopupMenu;

class PropertiesWindow : public wxDialog
{
public:
	PropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Item* item, wxPoint = wxDefaultPosition);
	virtual ~PropertiesWindow();

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);

	void OnResize(wxSizeEvent&);
	void OnNotebookPageChanged(wxNotebookEvent&);

protected:
	wxWindow* createGeneralPanel(wxWindow* parent);
	void saveGeneralPanel(wxWindow* panel);

	wxWindow* createContainerPanel(wxWindow* parent);
	void saveContainerPanel(wxWindow* panel);

	wxWindow* createAttributesPanel(wxWindow *parent);
	void saveAttributesPanel(wxWindow* panel);

protected:
	wxNotebook* notebook;
	wxWindow* currentPanel;
	const Map* edit_map;
	const Tile* edit_tile;
	Item* edit_item;

	DECLARE_EVENT_TABLE();
};

#endif
