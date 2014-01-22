//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "container_properties_window.h"

#include "old_properties_window.h"
#include "properties_window.h"
#include "gui.h"
#include "complexitem.h"
#include "map.h"

// ============================================================================
// Container Item Button
// Displayed in the container object properties menu, needs some
// custom event handling for the right-click menu etcetera so we
// need to define a custom class for it.

std::auto_ptr<ContainerItemPopupMenu> ContainerItemButton::popup_menu;

BEGIN_EVENT_TABLE(ContainerItemButton, ItemButton)
	EVT_LEFT_DOWN(ContainerItemButton::OnMouseDoubleLeftClick)
	EVT_RIGHT_UP(ContainerItemButton::OnMouseRightRelease)

	EVT_MENU(CONTAINER_POPUP_MENU_ADD, ContainerItemButton::OnAddItem)
	EVT_MENU(CONTAINER_POPUP_MENU_EDIT, ContainerItemButton::OnEditItem)
	EVT_MENU(CONTAINER_POPUP_MENU_REMOVE, ContainerItemButton::OnRemoveItem)
END_EVENT_TABLE()

ContainerItemButton::ContainerItemButton(wxWindow* parent, bool large, int _index, const Map* map, Item* item) :
	ItemButton(parent, (large? RENDER_SIZE_32x32 : RENDER_SIZE_16x16), (item? item->getClientID() : 0)),
	edit_map(map),
	edit_item(item),
	index(_index)
{
	// ...
}

ContainerItemButton::~ContainerItemButton()
{
	// ...
}

void ContainerItemButton::OnMouseDoubleLeftClick(wxMouseEvent& WXUNUSED(event))
{
	Container* container = getParentContainer();

	bool can_add = container->getVolume() > (int)container->getVector().size();

	if(edit_item) {
		wxCommandEvent unused;
		OnEditItem(unused);
	} else if(can_add) {
		wxCommandEvent unused;
		OnAddItem(unused);
	}
}

void ContainerItemButton::OnMouseRightRelease(wxMouseEvent& WXUNUSED(event))
{
	getMenu()->Update(this);
	PopupMenu(getMenu());
}

void ContainerItemButton::OnAddItem(wxCommandEvent& WXUNUSED(event))
{
	struct IsItem
	{
		static bool Pickupable(const ItemType& it)
		{
			return it.pickupable;
		}
	};

	FindItemDialog* d = newd FindItemDialog(this->GetParent(), wxT("Choose Item to add"));
	d->setCondition(IsItem::Pickupable);
	int id = d->ShowModal();
	d->Destroy();

	if(id != 0)
	{
		Container* container = getParentContainer();

		// Find the position where we should insert the item
		ItemVector& v = container->getVector();
		ItemVector::iterator item_index = v.begin();
		int i = 0;
		while(true) {
			if(item_index == v.end()) {
				// Insert at end of the vector
				break;
			}
			if(i == index) {
				// We found where to insert
				break;
			}
			++i;
			++item_index;
		}

		// Create and insert the item
		Item* item = Item::Create(id);
		v.insert(item_index, item);

		// Update view
		UpdateParentContainerWindow();
	}
}

void ContainerItemButton::OnEditItem(wxCommandEvent& WXUNUSED(event))
{
	ASSERT(edit_item);

	wxPoint newDialogAt;
	wxWindow* w = this;
	while (w = w->GetParent())
	{
		if (ObjectPropertiesWindowBase* o = dynamic_cast<ObjectPropertiesWindowBase*>(w))
		{
			newDialogAt = o->GetPosition();
			break;
		}
	}

	newDialogAt += wxPoint(20, 20);

	wxDialog* d;

	if (edit_map->getVersion().otbm >= MAP_OTBM_4)
		d = newd PropertiesWindow(
			this, edit_map, nullptr, edit_item,
			newDialogAt
		);
	else
		d = newd OldPropertiesWindow(
			this, edit_map, nullptr, edit_item,
			newDialogAt
		);
	d->ShowModal();
	d->Destroy();
}

void ContainerItemButton::OnRemoveItem(wxCommandEvent& WXUNUSED(event))
{
	ASSERT(edit_item);
	int ret = gui.PopupDialog(GetParent(),
		wxT("Remove Item"),
		wxT("Are you sure you want to remove this item from the container?"),
		wxYES | wxNO);

	if(ret == wxID_YES)
	{
		Container* container = getParentContainer();
		ItemVector& v = container->getVector();
		ItemVector::iterator item_index = v.begin();
		while(item_index != v.end())
		{
			if(*item_index == edit_item)
			{
				break;
			}
			++item_index;
		}
		ASSERT(item_index != v.end());

		v.erase(item_index);
		delete edit_item;

		UpdateParentContainerWindow();
	}
}

void ContainerItemButton::setItem(Item* item)
{
	edit_item = item;
	if(edit_item)
		SetSprite(edit_item->getClientID());
	else
		SetSprite(0);
}

Container* ContainerItemButton::getParentContainer()
{
	Container* parentContainer = nullptr;

	wxWindow* w = this;
	while (w = w->GetParent())
	{
		if (ObjectPropertiesWindowBase* o = dynamic_cast<ObjectPropertiesWindowBase*>(w))
			return dynamic_cast<Container*>(o->getItemBeingEdited());
	}
	return nullptr;
}

void ContainerItemButton::UpdateParentContainerWindow()
{
	wxWindow* w = this;
	while (w = w->GetParent())
	{
		if (ObjectPropertiesWindowBase* o = dynamic_cast<ObjectPropertiesWindowBase*>(w))
		{
			o->Update();
			return;
		}
	}
}

// ============================================================================
// Container Popup Menu

ContainerItemPopupMenu::ContainerItemPopupMenu() : wxMenu(wxT(""))
{
	// ...
}

ContainerItemPopupMenu::~ContainerItemPopupMenu()
{
	// ...
}

void ContainerItemPopupMenu::Update(ContainerItemButton* btn)
{
	// Clear the menu of all items
	while(GetMenuItemCount() > 0)
	{
		wxMenuItem* m_item = FindItemByPosition(0);
		// If you add a submenu, this won't delete it.
		Delete(m_item);
	}

	wxMenuItem* addItem = nullptr;
	if(btn->edit_item)
	{
		Append( CONTAINER_POPUP_MENU_EDIT, wxT("&Edit Item"), wxT("Open the properties menu for this item"));
		addItem = Append( CONTAINER_POPUP_MENU_ADD, wxT("&Add Item"), wxT("Add a newd item to the container"));
		Append( CONTAINER_POPUP_MENU_REMOVE, wxT("&Remove Item"), wxT("Remove this item from the container"));
	}
	else
	{
		addItem = Append( CONTAINER_POPUP_MENU_ADD, wxT("&Add Item"), wxT("Add a newd item to the container"));
	}

	Container* parentContainer = btn->getParentContainer();
	if(parentContainer->getVolume() <= (int)parentContainer->getVector().size())
		addItem->Enable(false);
}
