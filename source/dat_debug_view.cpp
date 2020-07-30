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

#include "dat_debug_view.h"

#include "graphics.h"
#include "gui.h"

// ============================================================================
//

class DatDebugViewListBox : public wxVListBox
{
public:
	DatDebugViewListBox(wxWindow* parent, wxWindowID id);
	~DatDebugViewListBox();

	void OnDrawItem(wxDC& dc, const wxRect& rect, size_t index) const;
	wxCoord OnMeasureItem(size_t index) const;

protected:
	typedef std::map<int, Sprite*> SpriteMap;
	SpriteMap sprites;
};

DatDebugViewListBox::DatDebugViewListBox(wxWindow* parent, wxWindowID id) :
	wxVListBox(parent, id, wxDefaultPosition, wxDefaultSize, wxLB_SINGLE)
{
	int n = 0;
	for(int id = 0; id < g_gui.gfx.getItemSpriteMaxID(); ++id) {
		Sprite* spr = g_gui.gfx.getSprite(id);
		if(spr) {
			sprites[n] = spr;
			++n;
		}
	}
	SetItemCount(n);
}

DatDebugViewListBox::~DatDebugViewListBox()
{
	////
}

void DatDebugViewListBox::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const
{
	SpriteMap::const_iterator spr_iter = sprites.find(int(n));
	if(spr_iter != sprites.end())
		spr_iter->second->DrawTo(&dc, SPRITE_SIZE_32x32, rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());

	if(IsSelected(n)) {
		if(HasFocus())
			dc.SetTextForeground(wxColor(0xFF, 0xFF, 0xFF));
		else
			dc.SetTextForeground(wxColor(0x00, 0x00, 0xFF));
	} else {
		dc.SetTextForeground(wxColor(0x00, 0x00, 0x00));
	}

	dc.DrawText(wxString() << n, rect.GetX() + 40, rect.GetY() + 6);
}

wxCoord DatDebugViewListBox::OnMeasureItem(size_t n) const
{
	return 32;
}

// ============================================================================
//

BEGIN_EVENT_TABLE(DatDebugView, wxPanel)
	EVT_TEXT(wxID_ANY, DatDebugView::OnTextChange)
	EVT_LISTBOX_DCLICK(wxID_ANY, DatDebugView::OnClickList)
END_EVENT_TABLE()

DatDebugView::DatDebugView(wxWindow* parent) : wxPanel(parent)
{
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);

	search_field = newd wxTextCtrl(this, wxID_ANY, "", wxDefaultPosition, wxDefaultSize);
	search_field->SetFocus();
	sizer->Add(search_field, 0, wxEXPAND, 2);

	item_list = newd DatDebugViewListBox(this, wxID_ANY);
	item_list->SetMinSize(wxSize(470, 400));
	sizer->Add(item_list, 1, wxEXPAND | wxALL, 2);

	SetSizerAndFit(sizer);
	Centre(wxBOTH);
}

DatDebugView::~DatDebugView()
{
	////
}

void DatDebugView::OnTextChange(wxCommandEvent& evt)
{
	////
}

void DatDebugView::OnClickList(wxCommandEvent& evt)
{
	////
}

