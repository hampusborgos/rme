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

protected:
	typedef std::map<int, Item*> ItemsMap;
	ItemsMap items;
};

BrowseTileListBox::BrowseTileListBox(wxWindow* parent, wxWindowID id, Tile* tile) :
wxVListBox(parent, id, wxDefaultPosition, wxSize(200, 250), wxLB_SINGLE)
{
	int n = 0;
	for (ItemVector::reverse_iterator it = tile->items.rbegin(); it != tile->items.rend(); ++it)
	{
		items[n] = (*it);
		++n;
	}

	if (tile->ground)
	{
		items[n] = tile->ground;
		++n;
	}

	SetItemCount(n);
}

BrowseTileListBox::~BrowseTileListBox()
{
}

void BrowseTileListBox::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const
{
	ItemsMap::const_iterator item_iterator = items.find(int(n));
	
	Sprite* sprite = gui.gfx.getSprite(item_iterator->second->getClientID());
	if (sprite)
		sprite->DrawTo(&dc, SPRITE_SIZE_32x32, rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());

	if (int(n) == GetSelection())
	{
		dc.SetTextForeground(wxColor(0xFF, 0xFF, 0xFF));
	}
	else
	{
		dc.SetTextForeground(wxColor(0x00, 0x00, 0x00));
	}
	
	dc.DrawText(item_iterator->second->getName(), rect.GetX() + 40, rect.GetY() + 6);
}

wxCoord BrowseTileListBox::OnMeasureItem(size_t n) const
{
	return 32;
}

// ============================================================================
//

BrowseTileWindow::BrowseTileWindow(wxWindow* parent, const Map* map, Tile* tile, wxPoint position /* = wxDefaultPosition */) :
wxDialog(parent, wxID_ANY, "Browse Field", position, wxSize(600, 400), wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER)
{
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	item_list = newd BrowseTileListBox(this, wxID_ANY, tile);
	sizer->Add(item_list, wxSizerFlags(1).Expand());

	wxString pos;
	pos << wxT("x=") << tile->getX() << wxT(",  y=") << tile->getY() << wxT(",  z=") << tile->getZ();

	wxSizer* infoSizer = newd wxBoxSizer(wxVERTICAL);
	infoSizer->Add(newd wxStaticText(this, wxID_ANY, wxT("Position: ") + pos), wxSizerFlags(0).Center());
	infoSizer->Add(newd wxStaticText(this, wxID_ANY, wxT("Item count: ") + i2ws(item_list->GetItemCount())), wxSizerFlags(0).Left());
	sizer->Add(infoSizer, wxSizerFlags(0).Center().DoubleBorder());

	SetSizerAndFit(sizer);
}

BrowseTileWindow::~BrowseTileWindow()
{
}

