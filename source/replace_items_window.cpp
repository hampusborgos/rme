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

#include "main.h"
#include "replace_items_window.h"
#include "find_item_window.h"
#include "graphics.h"
#include "gui.h"
#include "artprovider.h"
#include "items.h"

// ============================================================================
// ReplaceItemsButton

ReplaceItemsButton::ReplaceItemsButton(wxWindow* parent) :
	DCButton(parent, wxID_ANY, wxDefaultPosition, DC_BTN_TOGGLE, RENDER_SIZE_32x32, 0),
	m_id(0)
{
	////
}

ItemGroup_t ReplaceItemsButton::GetGroup() const
{
	if(m_id != 0) {
		const ItemType& it = g_items.getItemType(m_id);
		if(it.id != 0)
			return it.group;
	}
	return ITEM_GROUP_NONE;
}

void ReplaceItemsButton::SetItemId(uint16_t id)
{
	if(m_id == id)
		return;

	m_id = id;

	if(m_id != 0) {
		const ItemType& it = g_items.getItemType(m_id);
		if(it.id != 0) {
			SetSprite(it.clientID);
			return;
		}
	}

	SetSprite(0);
}

// ============================================================================
// ReplaceItemsListBox

ReplaceItemsListBox::ReplaceItemsListBox(wxWindow* parent) :
	wxVListBox(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxLB_SINGLE)
{
	m_arrow_bitmap = wxArtProvider::GetBitmap(ART_POSITION_GO, wxART_TOOLBAR, wxSize(16, 16));
	m_flag_bitmap = wxArtProvider::GetBitmap(ART_PZ_BRUSH, wxART_TOOLBAR, wxSize(16, 16));
}

bool ReplaceItemsListBox::AddItem(const ReplacingItem& item)
{
	if(item.replaceId == 0 || item.withId == 0 || item.replaceId == item.withId)
		return false;

	SetItemCount(GetItemCount() + 1);
	m_items.push_back(item);
	Refresh();

	return true;
}

void ReplaceItemsListBox::MarkAsComplete(const ReplacingItem& item, uint32_t total)
{
	auto it = std::find(m_items.begin(), m_items.end(), item);
	if(it != m_items.end()) {
		it->total = total;
		it->complete = true;
		Refresh();
	}
}

void ReplaceItemsListBox::RemoveSelected()
{
	if(m_items.empty())
		return;

	const int index = GetSelection();
	if(index == wxNOT_FOUND)
		return;

	m_items.erase(m_items.begin() + index);
	SetItemCount(GetItemCount() - 1);
	Refresh();
}

bool ReplaceItemsListBox::CanAdd(uint16_t replaceId, uint16_t withId) const
{
	if(replaceId == 0 || withId == 0 || replaceId == withId)
		return false;

	for(const ReplacingItem& item : m_items) {
		if(replaceId == item.replaceId)
			return false;
	}
	return true;
}

void ReplaceItemsListBox::OnDrawItem(wxDC& dc, const wxRect& rect, size_t index) const
{
	ASSERT(index < m_items.size());

	const ReplacingItem& item = m_items.at(index);
	const ItemType& type1 = g_items.getItemType(item.replaceId);
	Sprite* sprite1 = g_gui.gfx.getSprite(type1.clientID);
	const ItemType& type2 = g_items.getItemType(item.withId);
	Sprite* sprite2 = g_gui.gfx.getSprite(type2.clientID);

	if(sprite1 && sprite2) {
		int x = rect.GetX();
		int y = rect.GetY();
		sprite1->DrawTo(&dc, SPRITE_SIZE_32x32, x + 4, y + 4, rect.GetWidth(), rect.GetHeight());
		dc.DrawBitmap(m_arrow_bitmap, x + 38, y + 10, true);
		sprite2->DrawTo(&dc, SPRITE_SIZE_32x32, x + 56, y + 4, rect.GetWidth(), rect.GetHeight());
		dc.DrawText(wxString::Format("Replace: %d With: %d", item.replaceId, item.withId), x + 104, y + 10);

		if(item.complete) {
			x = rect.GetWidth() - 100;
			dc.DrawBitmap(m_flag_bitmap, x + 70, y + 10, true);
			dc.DrawText(wxString::Format("Total: %d", item.total), x, y + 10);
		}
	}

	if(IsSelected(index)) {
		if(HasFocus())
			dc.SetTextForeground(wxColor(0xFF, 0xFF, 0xFF));
		else
			dc.SetTextForeground(wxColor(0x00, 0x00, 0xFF));
	} else {
		dc.SetTextForeground(wxColor(0x00, 0x00, 0x00));
	}
}

wxCoord ReplaceItemsListBox::OnMeasureItem(size_t WXUNUSED(index)) const
{
	return 40;
}

// ============================================================================
// ReplaceItemsDialog

ReplaceItemsDialog::ReplaceItemsDialog(wxWindow* parent, bool selectionOnly) :
	wxDialog(parent, wxID_ANY, (selectionOnly ? "Replace Items on Selection" : "Replace Items"),
			 wxDefaultPosition, wxSize(500, 480), wxDEFAULT_DIALOG_STYLE),
	selectionOnly(selectionOnly)
{
	SetSizeHints(wxDefaultSize, wxDefaultSize);

	wxBoxSizer* sizer = new wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer* list_sizer = new wxFlexGridSizer(0, 2, 0, 0);
	list_sizer->SetFlexibleDirection(wxBOTH);
	list_sizer->SetNonFlexibleGrowMode(wxFLEX_GROWMODE_SPECIFIED);
	list_sizer->SetMinSize(wxSize(-1, 300));

	list = new ReplaceItemsListBox(this);
	list->SetMinSize(wxSize(480, 320));

	list_sizer->Add(list, 0, wxALL | wxEXPAND, 5);
	sizer->Add(list_sizer, 1, wxALL | wxEXPAND, 5);

	wxBoxSizer* items_sizer = new wxBoxSizer(wxHORIZONTAL);
	items_sizer->SetMinSize(wxSize(-1, 40));

	replace_button = new ReplaceItemsButton(this);
	items_sizer->Add(replace_button, 0, wxALL, 5);

	wxBitmap bitmap = wxArtProvider::GetBitmap(ART_POSITION_GO, wxART_TOOLBAR, wxSize(16, 16));
	arrow_bitmap = new wxStaticBitmap(this, wxID_ANY, bitmap);
	items_sizer->Add(arrow_bitmap, 0, wxTOP, 15);

	with_button = new ReplaceItemsButton(this);
	items_sizer->Add(with_button, 0, wxALL, 5);

	items_sizer->Add(0, 0, 1, wxEXPAND, 5);

	progress = new wxGauge(this, wxID_ANY, 100);
	progress->SetValue(0);
	items_sizer->Add(progress, 0, wxALL, 5);

	sizer->Add(items_sizer, 1, wxALL | wxEXPAND, 5);

	wxBoxSizer* buttons_sizer = new wxBoxSizer(wxHORIZONTAL);

	add_button = new wxButton(this, wxID_ANY, wxT("Add"));
	add_button->Enable(false);
	buttons_sizer->Add(add_button, 0, wxALL, 5);

	remove_button = new wxButton(this, wxID_ANY, wxT("Remove"));
	remove_button->Enable(false);
	buttons_sizer->Add(remove_button, 0, wxALL, 5);

	buttons_sizer->Add(0, 0, 1, wxEXPAND, 5);

	execute_button = new wxButton(this, wxID_ANY, wxT("Execute"));
	execute_button->Enable(false);
	buttons_sizer->Add(execute_button, 0, wxALL, 5);

	close_button = new wxButton(this, wxID_ANY, wxT("Close"));
	buttons_sizer->Add(close_button, 0, wxALL, 5);

	sizer->Add(buttons_sizer, 1, wxALL | wxLEFT | wxRIGHT | wxSHAPED, 5);

	SetSizer(sizer);
	Layout();
	Centre(wxBOTH);

	// Connect Events
	list->Connect(wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler(ReplaceItemsDialog::OnListSelected), NULL, this);
	replace_button->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(ReplaceItemsDialog::OnReplaceItemClicked), NULL, this);
	with_button->Connect(wxEVT_LEFT_DOWN, wxMouseEventHandler(ReplaceItemsDialog::OnWithItemClicked), NULL, this);
	add_button->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ReplaceItemsDialog::OnAddButtonClicked), NULL, this);
	remove_button->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ReplaceItemsDialog::OnRemoveButtonClicked), NULL, this);
	execute_button->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ReplaceItemsDialog::OnExecuteButtonClicked), NULL, this);
	close_button->Connect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ReplaceItemsDialog::OnCancelButtonClicked), NULL, this);
}

ReplaceItemsDialog::~ReplaceItemsDialog()
{
	// Disconnect Events
	list->Disconnect(wxEVT_COMMAND_LISTBOX_SELECTED, wxCommandEventHandler(ReplaceItemsDialog::OnListSelected), NULL, this);
	replace_button->Disconnect(wxEVT_LEFT_DOWN, wxMouseEventHandler(ReplaceItemsDialog::OnReplaceItemClicked), NULL, this);
	with_button->Disconnect(wxEVT_LEFT_DOWN, wxMouseEventHandler(ReplaceItemsDialog::OnWithItemClicked), NULL, this);
	add_button->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ReplaceItemsDialog::OnAddButtonClicked), NULL, this);
	remove_button->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ReplaceItemsDialog::OnRemoveButtonClicked), NULL, this);
	execute_button->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ReplaceItemsDialog::OnExecuteButtonClicked), NULL, this);
	close_button->Disconnect(wxEVT_COMMAND_BUTTON_CLICKED, wxCommandEventHandler(ReplaceItemsDialog::OnCancelButtonClicked), NULL, this);
}

void ReplaceItemsDialog::UpdateWidgets()
{
	const uint16_t replaceId = replace_button->GetItemId();
	const uint16_t withId = with_button->GetItemId();
	add_button->Enable(list->CanAdd(replaceId, withId));
	remove_button->Enable(list->GetCount() != 0 && list->GetSelection() != wxNOT_FOUND);
	execute_button->Enable(list->GetCount() != 0);
}

void ReplaceItemsDialog::OnListSelected(wxCommandEvent& WXUNUSED(event))
{
	remove_button->Enable(list->GetCount() != 0 && list->GetSelection() != wxNOT_FOUND);
}

void ReplaceItemsDialog::OnReplaceItemClicked(wxMouseEvent& WXUNUSED(event))
{
	FindItemDialog dialog(this, "Replace Item");
	if(dialog.ShowModal() == wxID_OK) {
		uint16_t id = dialog.getResultID();
		if(id != with_button->GetItemId()) {
			replace_button->SetItemId(id);
			UpdateWidgets();
		}
	}
	dialog.Destroy();
}

void ReplaceItemsDialog::OnWithItemClicked(wxMouseEvent& WXUNUSED(event))
{
	if(replace_button->GetItemId() == 0)
		return;

	FindItemDialog dialog(this, "With Item");
	if(dialog.ShowModal() == wxID_OK) {
		uint16_t id = dialog.getResultID();
		if(id != replace_button->GetItemId()) {
			with_button->SetItemId(id);
			UpdateWidgets();
		}
	}
	dialog.Destroy();
}

void ReplaceItemsDialog::OnAddButtonClicked(wxCommandEvent& WXUNUSED(event))
{
	const uint16_t replaceId = replace_button->GetItemId();
	const uint16_t withId = with_button->GetItemId();
	if(list->CanAdd(replaceId, withId)) {
		ReplacingItem item;
		item.replaceId = replaceId;
		item.withId = withId;
		if(list->AddItem(item)) {
			replace_button->SetItemId(0);
			with_button->SetItemId(0);
			UpdateWidgets();
		}
	}
}

void ReplaceItemsDialog::OnRemoveButtonClicked(wxCommandEvent& WXUNUSED(event))
{
	list->RemoveSelected();
	UpdateWidgets();
}

void ReplaceItemsDialog::OnExecuteButtonClicked(wxCommandEvent& WXUNUSED(event))
{
	if(!g_gui.IsEditorOpen())
		return;

	const auto& items = list->GetItems();
	if(items.empty())
		return;

	replace_button->Enable(false);
	with_button->Enable(false);
	add_button->Enable(false);
	remove_button->Enable(false);
	execute_button->Enable(false);
	close_button->Enable(false);
	progress->SetValue(0);

	MapTab* tab = dynamic_cast<MapTab*>(GetParent());
	if(!tab)
		return;

	Editor* editor = tab->GetEditor();

	int done = 0;
	for(const ReplacingItem& info : items) {
		ItemFinder finder(info.replaceId, (uint32_t)g_settings.getInteger(Config::REPLACE_SIZE));

		// search on map
		foreach_ItemOnMap(editor->map, finder, selectionOnly);

		uint32_t total = 0;
		std::vector<std::pair<Tile*, Item*>>& result = finder.result;

		if(!result.empty()) {
			Action* action = editor->actionQueue->createAction(ACTION_REPLACE_ITEMS);
			for(std::vector<std::pair<Tile*, Item*>>::const_iterator rit = result.begin(); rit != result.end(); ++rit) {
				Tile* new_tile = rit->first->deepCopy(editor->map);
				int index = rit->first->getIndexOf(rit->second);
				ASSERT(index != wxNOT_FOUND);
				Item* item = new_tile->getItemAt(index);
				ASSERT(item && item->getID() == rit->second->getID());
				transformItem(item, info.withId, new_tile);
				action->addChange(new Change(new_tile));
				total++;
			}
			editor->actionQueue->addAction(action);
		}

		done++;
		const int value = static_cast<int>((done / items.size()) * 100);
		progress->SetValue(std::clamp<int>(value, 0, 100));
		list->MarkAsComplete(info, total);
	}

	tab->Refresh();
	close_button->Enable(true);
	UpdateWidgets();
}

void ReplaceItemsDialog::OnCancelButtonClicked(wxCommandEvent& WXUNUSED(event))
{
	Close();
}
