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

#include "duplicated_items_window.h"
#include "gui.h"
#include "map.h"
#include "tile.h"
#include "item.h"
#include "editor.h"

DuplicatedItemsWindow::DuplicatedItemsWindow(wxWindow* parent) :
	wxPanel(parent, wxID_ANY),
	map_tab(nullptr)
{
	wxSize icon_size = FROM_DIP(parent, wxSize(16, 16));
	wxBitmap save_bitmap = wxArtProvider::GetBitmap(wxART_FILE_SAVE, wxART_TOOLBAR, icon_size);

	wxSizer* sizer = new wxBoxSizer(wxVERTICAL);
	items_list = new wxListBox(this, wxID_ANY, wxDefaultPosition, wxSize(200, 330), 0, nullptr, wxLB_SINGLE | wxLB_ALWAYS_SB);
	sizer->Add(items_list, wxSizerFlags(1).Expand());

	wxSizer* buttonsSizer = new wxBoxSizer(wxHORIZONTAL);

	remove_button = new wxButton(this, wxID_DELETE, "Remove");
	remove_button->Enable(false);
	remove_all_button = new wxButton(this, wxID_DELETE, "Remove All");
	remove_all_button->Enable(false);
	export_button = new wxBitmapButton(this, wxID_ANY, save_bitmap, wxDefaultPosition, wxDefaultSize, wxBU_AUTODRAW);
	export_button->SetToolTip("Export");
	export_button->Enable(false);

	buttonsSizer->Add(remove_button, wxSizerFlags(0).Center());
	buttonsSizer->AddSpacer(10);
	buttonsSizer->Add(remove_all_button, wxSizerFlags(0).Center());
	buttonsSizer->AddSpacer(20);
	buttonsSizer->Add(export_button, wxSizerFlags(0).Center());
	sizer->Add(buttonsSizer, wxSizerFlags(0).Center().DoubleBorder());

	SetSizerAndFit(sizer);

	items_list->Bind(wxEVT_LISTBOX, &DuplicatedItemsWindow::OnClickResult, this);
	remove_button->Bind(wxEVT_BUTTON, &DuplicatedItemsWindow::OnClickRemove, this);
	remove_all_button->Bind(wxEVT_BUTTON, &DuplicatedItemsWindow::OnClickRemoveAll, this);
	export_button->Bind(wxEVT_BUTTON, &DuplicatedItemsWindow::OnClickExport, this);
}

DuplicatedItemsWindow::~DuplicatedItemsWindow()
{
	Clear();

	items_list->Unbind(wxEVT_LISTBOX, &DuplicatedItemsWindow::OnClickResult, this);
	remove_button->Unbind(wxEVT_BUTTON, &DuplicatedItemsWindow::OnClickRemove, this);
	remove_all_button->Unbind(wxEVT_BUTTON, &DuplicatedItemsWindow::OnClickRemoveAll, this);
	export_button->Unbind(wxEVT_BUTTON, &DuplicatedItemsWindow::OnClickExport, this);
}

void DuplicatedItemsWindow::StartSearch(MapTab* tab, bool selection)
{
	Clear();

	map_tab = tab;
	if(!map_tab) {
		return;
	}

	auto message = wxString::Format("Searching on %s...", selection ? "selected area" : "map");
	g_gui.CreateLoadBar(message);

	Map* map = map_tab->GetMap();
	auto ibegin = map->begin();
	auto iend = map->end();
	long progress = 0;
	std::vector<DuplicatedItem*> result;

	while(ibegin != iend) {
		++progress;
		Tile* tile = (*ibegin)->get();
		if(selection && !tile->isSelected()) {
			++ibegin;
			continue;
		}

		if(progress % 0x8000 == 0) {
			g_gui.SetLoadDone((int)(100 * progress / map->getTileCount()));
		}

		auto& position = tile->getPosition();
		uint16_t prevId = 0;
		uint16_t count = 0;

		for(Item* item : tile->items) {
			uint16_t id = item->getID();
			if(id == prevId) {
				count++;
			} else if(count > 0) {
				result.push_back(new DuplicatedItem(position, prevId, count));
				count = 0;
			}
			prevId = id;
		}
		// Check for the last item
		if(count > 0) {
			result.push_back(new DuplicatedItem(position, prevId, count));
		}

		++ibegin;
	}

	g_gui.DestroyLoadBar();

	for(auto item : result) {
		auto label = wxString::Format("item: %d, count: %d, pos: (%d,%d,%d)",
			item->itemId, item->count, item->position.x, item->position.y, item->position.z);
		items_list->Append(label, item);
	}

	UpdateButtons();
}

void DuplicatedItemsWindow::Clear()
{
	uint32_t count = items_list->GetCount();
	if(count == 0) {
		return;
	}

	for(uint32_t i = 0; i < count; ++i) {
		delete reinterpret_cast<DuplicatedItem*>(items_list->GetClientData(i));
	}

	items_list->Clear();
	UpdateButtons();

	map_tab = nullptr;
}

void DuplicatedItemsWindow::OnClickResult(wxCommandEvent& event)
{
	if(!map_tab || !map_tab->IsCurrent()) {
		return;
	}

	DuplicatedItem* data = reinterpret_cast<DuplicatedItem*>(event.GetClientData());
	if(data) {
		g_gui.SetScreenCenterPosition(data->position);
		remove_button->Enable(true);
	} else {
		remove_button->Enable(false);
	}
}

void DuplicatedItemsWindow::OnClickRemove(wxCommandEvent& WXUNUSED(event))
{
	if(!map_tab || !map_tab->IsCurrent()) {
		return;
	}

	int32_t index = items_list->GetSelection();
	if (index == wxNOT_FOUND) {
		return;
	}

	Editor* editor = map_tab->GetEditor();
	ActionQueue* history = editor->getHistoryActions();
	BatchAction* batch = history->createBatch(ACTION_DELETE_TILES);
	Action* action = history->createAction(batch);

	DuplicatedItem* data = reinterpret_cast<DuplicatedItem*>(items_list->GetClientData(index));
	removeItem(data, action);

	batch->addAndCommitAction(action);
	editor->addBatch(batch);
	editor->updateActions();

	items_list->Delete(index);
	delete data;

	UpdateButtons();
}

void DuplicatedItemsWindow::OnClickRemoveAll(wxCommandEvent& WXUNUSED(event))
{
	if(!map_tab || !map_tab->IsCurrent()) {
		return;
	}

	uint32_t count = items_list->GetCount();
	if (count == 0) {
		return;
	}

	g_gui.CreateLoadBar("Removing items...");

	Map* map = map_tab->GetMap();
	Editor* editor = map_tab->GetEditor();
	ActionQueue* history = editor->getHistoryActions();
	BatchAction* batch = history->createBatch(ACTION_DELETE_TILES);
	Action* action = history->createAction(batch);

	for(uint32_t i = 0; i < count; ++i) {
		DuplicatedItem* data = reinterpret_cast<DuplicatedItem*>(items_list->GetClientData(i));
		removeItem(data, action);
		g_gui.SetLoadScale((int32_t)i, (int32_t)count);
	}

	batch->addAndCommitAction(action);
	editor->addBatch(batch);
	editor->updateActions();

	g_gui.DestroyLoadBar();
	Clear();
}

void DuplicatedItemsWindow::OnClickExport(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog dialog(this, "Save file...", "", "", "Text Documents (*.txt) | *.txt", wxFD_SAVE);
	if(dialog.ShowModal() == wxID_OK) {
		wxFile file(dialog.GetPath(), wxFile::write);
		if(file.IsOpened()) {
			g_gui.CreateLoadBar("Exporting result...");

			file.Write("Generated by Remere's Map Editor " + __RME_VERSION__);
			file.Write("\n=============================================\n\n");
			wxArrayString lines = items_list->GetStrings();
			size_t count = lines.Count();
			for(size_t i = 0; i < count; ++i) {
				file.Write(lines[i] + "\n");
				g_gui.SetLoadScale((int32_t)i, (int32_t)count);
			}
			file.Close();

			g_gui.DestroyLoadBar();
		}
	}
}

void DuplicatedItemsWindow::UpdateButtons()
{
	if(!IsShownOnScreen()) {
		return;
	}

	bool enable = items_list->GetCount() != 0 && map_tab && map_tab->IsCurrent();
	remove_button->Enable(enable);
	remove_all_button->Enable(enable);
	export_button->Enable(enable);
}

bool DuplicatedItemsWindow::removeItem(DuplicatedItem* data, Action* action)
{
	Map* map = map_tab->GetMap();
	Tile* tile = map->getTile(data->position);
	if(!tile) {
		return false;
	}

	uint16_t count = 0;
	Tile* new_tile = tile->deepCopy(*map);
	for(auto it = new_tile->items.begin(); it != new_tile->items.end();) {
		Item* item = *it;
		if (item->getID() != data->itemId || ++count == 1) {
			++it;
			continue;
		}
		delete item;
		it = new_tile->items.erase(it);
	}

	action->addChange(new Change(new_tile));
	return true;
}
