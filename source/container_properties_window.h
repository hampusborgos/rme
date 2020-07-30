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

#ifndef _RME_CONTAINER_PROPS_H_
#define _RME_CONTAINER_PROPS_H_

#include "common_windows.h"

class Container;
class ContainerItemButton;

// Right-click popup menu
class ContainerItemPopupMenu : public wxMenu {
public:
	ContainerItemPopupMenu();
	virtual ~ContainerItemPopupMenu();

	void Update(ContainerItemButton* what);
};

// Container Item Button
class ContainerItemButton : public ItemButton
{
	DECLARE_EVENT_TABLE()
	public:
		ContainerItemButton(wxWindow* parent, bool large, int index, const Map* map, Item* item);
		~ContainerItemButton();

		void OnMouseDoubleLeftClick(wxMouseEvent& event);
		void OnMouseRightRelease(wxMouseEvent& event);

		void OnAddItem(wxCommandEvent& event);
		void OnEditItem(wxCommandEvent& event);
		void OnRemoveItem(wxCommandEvent& event);

		ObjectPropertiesWindowBase* getParentContainerWindow();
		Container* getParentContainer();

		void setItem(Item* item);

	private:
		static std::unique_ptr<ContainerItemPopupMenu> popup_menu;

		const Map* edit_map;
		Item* edit_item;

		size_t index;

		friend class ContainerItemPopupMenu;
};

#endif
