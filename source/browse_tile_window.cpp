//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "map.h"
#include "tile.h"
#include "graphics.h"
#include "gui.h"
#include "browse_tile_window.h"

// ============================================================================
//

class BrowseTileListBox : public wxVListBox
{
public:
	BrowseTileListBox(wxWindow* parent, wxWindowID id, Tile* tile);
	~BrowseTileListBox();

	void OnDrawItem(wxDC& dc, const wxRect& rect, size_t index) const;
	wxCoord OnMeasureItem(size_t index) const;
	void RemoveSelected();

protected:
	void UpdateItems();

	typedef std::map<int, Item*> ItemsMap;
	ItemsMap items;
	Tile* edit_tile;
};

BrowseTileListBox::BrowseTileListBox(wxWindow* parent, wxWindowID id, Tile* tile) :
wxVListBox(parent, id, wxDefaultPosition, wxSize(200, 250), wxLB_MULTIPLE), edit_tile(tile)
{
	edit_tile->select();
	UpdateItems();
}

BrowseTileListBox::~BrowseTileListBox()
{
}

void BrowseTileListBox::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const
{
	ItemsMap::const_iterator item_iterator = items.find(int(n));
	Item* item = item_iterator->second;
	
	Sprite* sprite = gui.gfx.getSprite(item->getClientID());
	if (sprite)
		sprite->DrawTo(&dc, SPRITE_SIZE_32x32, rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());

	if (IsSelected(n))
	{
		item->select();
		dc.SetTextForeground(wxColor(0xFF, 0xFF, 0xFF));
	}
	else {
		item->deselect();
		dc.SetTextForeground(wxColor(0x00, 0x00, 0x00));
	}
	
	wxString label;
	label << item->getID() << wxT(" - ") << item->getName();
	dc.DrawText(label, rect.GetX() + 40, rect.GetY() + 6);
}

wxCoord BrowseTileListBox::OnMeasureItem(size_t n) const
{
	return 32;
}

void BrowseTileListBox::RemoveSelected()
{
	if (GetItemCount() == 0 || GetSelectedCount() == 0) return;

	Clear();
	items.clear();

	// Delete the items from the tile
	ItemVector tile_selection = edit_tile->popSelectedItems();
	for (ItemVector::iterator iit = tile_selection.begin(); iit != tile_selection.end(); ++iit)
	{
		delete *iit;
	}

	edit_tile->select();
	UpdateItems();
	Refresh();
}

void BrowseTileListBox::UpdateItems()
{
	int n = 0;
	for (ItemVector::reverse_iterator it = edit_tile->items.rbegin(); it != edit_tile->items.rend(); ++it)
	{
		items[n] = (*it);
		++n;
	}

	if (edit_tile->ground)
	{
		items[n] = edit_tile->ground;
		++n;
	}

	SetItemCount(n);
}

// ============================================================================
//

BEGIN_EVENT_TABLE(BrowseTileWindow, wxDialog)
	EVT_BUTTON(wxID_REMOVE, BrowseTileWindow::OnClickDelete)
	EVT_BUTTON(wxID_OK, BrowseTileWindow::OnClickOK)
	EVT_BUTTON(wxID_CANCEL, BrowseTileWindow::OnClickCancel)
END_EVENT_TABLE()

BrowseTileWindow::BrowseTileWindow(wxWindow* parent, const Map* map, Tile* tile, wxPoint position /* = wxDefaultPosition */) :
wxDialog(parent, wxID_ANY, "Browse Field", position, wxSize(600, 400), wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER)
{
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	item_list = newd BrowseTileListBox(this, wxID_ANY, tile);
	sizer->Add(item_list, wxSizerFlags(1).Expand());

	wxString pos;
	pos << wxT("x=") << tile->getX() << wxT(",  y=") << tile->getY() << wxT(",  z=") << tile->getZ();

	wxSizer* infoSizer = newd wxBoxSizer(wxVERTICAL);
	infoSizer->Add(newd wxButton(this, wxID_REMOVE, wxT("Delete")), wxSizerFlags(0).Left());
	infoSizer->AddSpacer(5);
	infoSizer->Add(newd wxStaticText(this, wxID_ANY, wxT("Position: ") + pos), wxSizerFlags(0).Left());
	item_count_txt = newd wxStaticText(this, wxID_ANY, wxT("Item count: ") + i2ws(item_list->GetItemCount()));
	infoSizer->Add(item_count_txt, wxSizerFlags(0).Left());
	sizer->Add(infoSizer, wxSizerFlags(0).Left().DoubleBorder());

	// OK/Cancel buttons
	wxSizer* btnSizer = newd wxBoxSizer(wxHORIZONTAL);
	btnSizer->Add(newd wxButton(this, wxID_OK, wxT("OK")), wxSizerFlags(0).Center());
	btnSizer->Add(newd wxButton(this, wxID_CANCEL, wxT("Cancel")), wxSizerFlags(0).Center());
	sizer->Add(btnSizer, wxSizerFlags(0).Center().DoubleBorder());

	SetSizerAndFit(sizer);
}

BrowseTileWindow::~BrowseTileWindow()
{
}

void BrowseTileWindow::OnClickDelete(wxCommandEvent& WXUNUSED(event))
{
	item_list->RemoveSelected();
	item_count_txt->SetLabelText(wxT("Item count: ") + i2ws(item_list->GetItemCount()));
}

void BrowseTileWindow::OnClickOK(wxCommandEvent& WXUNUSED(event))
{
	EndModal(1);
}

void BrowseTileWindow::OnClickCancel(wxCommandEvent& WXUNUSED(event))
{
	EndModal(0);
}

