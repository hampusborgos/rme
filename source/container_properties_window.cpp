//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "container_properties_window.h"

#include "old_properties_window.h"
#include "gui.h"
#include "complexitem.h"

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
	OldPropertiesWindow* parent = dynamic_cast<OldPropertiesWindow*>(GetParent());
	ASSERT(parent);
	Container* container = dynamic_cast<Container*>(parent->edit_item);
	ASSERT(container);

	bool can_add = container->getVolume() > container->getVector().size();

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
		// We got an item!
		// Fetch some important data
		OldPropertiesWindow* parent = dynamic_cast<OldPropertiesWindow*>(this->GetParent());
		ASSERT(parent);
		Container* container = dynamic_cast<Container*>(parent->edit_item);
		ASSERT(container);

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
		for(uint i = 1; i <= container->getVolume(); ++i)
		{
			Item* item = NULL;
			if(container->getItem(i - 1))
			{
				item = container->getItem(i - 1);
			}

			parent->container_items[i - 1]->setItem(item);
		}
	}
}

void ContainerItemButton::OnEditItem(wxCommandEvent& WXUNUSED(event))
{
	ASSERT(edit_item);
	wxDialog* w = newd OldPropertiesWindow(this, edit_map, NULL, edit_item, this->GetParent()->GetPosition() + wxPoint(10,10));
	w->ShowModal();
	w->Destroy();
}

void ContainerItemButton::OnRemoveItem(wxCommandEvent& WXUNUSED(event))
{
	ASSERT(edit_item);
	int ret = gui.PopupDialog(GetParent(), wxT("Remove Item"), wxT("Are you sure you want to remove this item from the container?"), wxYES | wxNO);
	if(ret == wxID_YES)
	{
		OldPropertiesWindow* parent = dynamic_cast<OldPropertiesWindow*>(this->GetParent());
		ASSERT(parent);
		Container* container = dynamic_cast<Container*>(parent->edit_item);
		ASSERT(container);

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

		for(uint i = 1; i <= container->getVolume(); ++i)
		{
			Item* item = NULL;
			if(container->getItem(i - 1))
			{
				item = container->getItem(i - 1);
			}

			parent->container_items[i - 1]->setItem(item);
		}
	}
	else
	{
		// ...
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

	OldPropertiesWindow* parent = dynamic_cast<OldPropertiesWindow*>(btn->GetParent());
	ASSERT(parent);
	Container* container = dynamic_cast<Container*>(parent->edit_item);
	ASSERT(container);

	wxMenuItem* addItem = NULL;
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
	if(container->getVolume() <= container->getVector().size())
	{
		addItem->Enable(false);
	}
}
