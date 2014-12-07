//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_OLD_PROPERTIES_WINDOW_H_
#define RME_OLD_PROPERTIES_WINDOW_H_

#include "main.h"

#include "common_windows.h"

class ContainerItemButton;
class ContainerItemPopupMenu;

class OldPropertiesWindow : public ObjectPropertiesWindowBase
{
public:
	OldPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Item* item, wxPoint = wxDefaultPosition);
	OldPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Creature* creature, wxPoint = wxDefaultPosition);
	OldPropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile, Spawn* spawn, wxPoint = wxDefaultPosition);
	virtual ~OldPropertiesWindow();
	
	void OnFocusChange(wxFocusEvent&);

	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);

	void Update();

protected:

protected:
	wxSpinCtrl* count_field;
	wxSpinCtrl* direction_field;
	wxSpinCtrl* action_id_field;
	wxSpinCtrl* unique_id_field;
	wxSpinCtrl* door_id_field;
	wxChoice* depot_id_field;
	wxSpinCtrl* x_field;
	wxSpinCtrl* y_field;
	wxSpinCtrl* z_field;
	wxChoice* splash_type_field;
	wxTextCtrl* text_field;
	wxTextCtrl* description_field;
	std::vector<ContainerItemButton*> container_items;

	friend class ContainerItemButton;
	friend class ContainerItemPopupMenu;

	DECLARE_EVENT_TABLE();
};

#endif

