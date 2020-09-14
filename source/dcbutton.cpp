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

#include "settings.h"

#include "dcbutton.h"
#include "sprites.h"
#include "gui.h"

BEGIN_EVENT_TABLE(DCButton, wxPanel)
	EVT_PAINT(DCButton::OnPaint)
	EVT_LEFT_DOWN(DCButton::OnClick)
END_EVENT_TABLE()

IMPLEMENT_DYNAMIC_CLASS(DCButton, wxPanel)

DCButton::DCButton() :
	wxPanel(nullptr, wxID_ANY, wxDefaultPosition, wxSize(36, 36)),
	type(DC_BTN_NORMAL),
	state(false),
	size(RENDER_SIZE_16x16),
	sprite(nullptr),
	overlay(nullptr)
{
	SetSprite(0);
}

DCButton::DCButton(wxWindow* parent, wxWindowID id, wxPoint pos, int type, RenderSize sz, int sprite_id) :
	wxPanel(parent, id, pos, (sz == RENDER_SIZE_64x64? wxSize(68, 68) : sz == RENDER_SIZE_32x32? wxSize(36, 36) : wxSize(20, 20))),
	type(type),
	state(false),
	size(sz),
	sprite(nullptr),
	overlay(nullptr)
{
	SetSprite(sprite_id);
}

DCButton::~DCButton()
{
	////
}

void DCButton::SetSprite(int _sprid)
{
	if(_sprid != 0) {
		sprite = g_gui.gfx.getSprite(_sprid);
	} else {
		sprite = nullptr;
	}
	Refresh();
}

void DCButton::SetOverlay(Sprite* espr)
{
	overlay = espr;
	Refresh();
}

void DCButton::SetValue(bool val)
{
	ASSERT(type == DC_BTN_TOGGLE);
	bool oldval = val;
	state = val;
	if(state == oldval) {
		// Cheap to change value to the old one (which is done ALOT)
		if(GetValue() && g_settings.getInteger(Config::USE_GUI_SELECTION_SHADOW)) {
			SetOverlay(g_gui.gfx.getSprite(EDITOR_SPRITE_SELECTION_MARKER));
		} else {
			SetOverlay(nullptr);
		}
		Refresh();
	}
}

bool DCButton::GetValue() const
{
	ASSERT(type == DC_BTN_TOGGLE);
	return state;
}

void DCButton::OnPaint(wxPaintEvent& event)
{
	wxBufferedPaintDC pdc(this);

	if(g_gui.gfx.isUnloaded()) {
		return;
	}

	static std::unique_ptr<wxPen> highlight_pen;
	static std::unique_ptr<wxPen> dark_highlight_pen;
	static std::unique_ptr<wxPen> light_shadow_pen;
	static std::unique_ptr<wxPen> shadow_pen;

	if(highlight_pen.get() == nullptr)      highlight_pen.reset(newd wxPen(wxColor(0xFF,0xFF,0xFF), 1, wxSOLID));
	if(dark_highlight_pen.get() == nullptr) dark_highlight_pen.reset(newd wxPen(wxColor(0xD4,0xD0,0xC8), 1, wxSOLID));
	if(light_shadow_pen.get() == nullptr)   light_shadow_pen.reset(newd wxPen(wxColor(0x80,0x80,0x80), 1, wxSOLID));
	if(shadow_pen.get() == nullptr)         shadow_pen.reset(newd wxPen(wxColor(0x40,0x40,0x40), 1, wxSOLID));

	int size_x = 20, size_y = 20;

	if(size == RENDER_SIZE_16x16) {
		size_x = 20;
		size_y = 20;
	} else if(size == RENDER_SIZE_32x32) {
		size_x = 36;
		size_y = 36;
	}

	pdc.SetBrush(*wxBLACK);
	pdc.DrawRectangle(0,0,size_x,size_y);
	if(type == DC_BTN_TOGGLE && GetValue()) {
		pdc.SetPen(*shadow_pen);
		pdc.DrawLine(0 ,0 ,size_x-1,0 );
		pdc.DrawLine(0 ,1 ,0 ,size_y-1);
		pdc.SetPen(*light_shadow_pen);
		pdc.DrawLine(1 ,1 ,size_x-2,1 );
		pdc.DrawLine(1 ,2 ,1 ,size_y-2);
		pdc.SetPen(*dark_highlight_pen);
		pdc.DrawLine(size_x-2,1 ,size_x-2,size_y-2);
		pdc.DrawLine(1 ,size_y-2,size_x-1,size_y-2);
		pdc.SetPen(*highlight_pen);
		pdc.DrawLine(size_x-1,0 ,size_x-1,size_y-1);
		pdc.DrawLine(0 ,size_y-1,size_y,size_y-1);
	} else {
		pdc.SetPen(*highlight_pen);
		pdc.DrawLine(0 ,0 ,size_x-1,0 );
		pdc.DrawLine(0 ,1 ,0 ,size_y-1);
		pdc.SetPen(*dark_highlight_pen);
		pdc.DrawLine(1 ,1 ,size_x-2,1 );
		pdc.DrawLine(1 ,2 ,1 ,size_y-2);
		pdc.SetPen(*light_shadow_pen);
		pdc.DrawLine(size_x-2,1 ,size_x-2,size_y-2);
		pdc.DrawLine(1 ,size_y-2,size_x-1,size_y-2);
		pdc.SetPen(*shadow_pen);
		pdc.DrawLine(size_x-1,0 ,size_x-1,size_y-1);
		pdc.DrawLine(0 ,size_y-1,size_y,size_y-1);
	}

	if(sprite) {
		if(size == RENDER_SIZE_16x16) {
			// Draw the picture!
			sprite->DrawTo(&pdc, SPRITE_SIZE_16x16, 2, 2);

			if(overlay && type == DC_BTN_TOGGLE && GetValue()) {
				overlay->DrawTo(&pdc, SPRITE_SIZE_16x16, 2, 2);
			}
		} else if(size == RENDER_SIZE_32x32) {
			// Draw the picture!
			sprite->DrawTo(&pdc, SPRITE_SIZE_32x32, 2, 2);

			if(overlay && type == DC_BTN_TOGGLE && GetValue()) {
				overlay->DrawTo(&pdc, SPRITE_SIZE_32x32, 2, 2);
			}
		} else if(size == RENDER_SIZE_64x64) {
			////
		}
	}
}

void DCButton::OnClick(wxMouseEvent& WXUNUSED(evt))
{
	wxCommandEvent event(type == DC_BTN_TOGGLE? wxEVT_COMMAND_TOGGLEBUTTON_CLICKED : wxEVT_COMMAND_BUTTON_CLICKED, GetId());
	event.SetEventObject(this);

	if(type == DC_BTN_TOGGLE) {
		SetValue(!GetValue());
	}
	SetFocus();

	GetEventHandler()->ProcessEvent(event);
}

