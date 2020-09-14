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
#include "positionctrl.h"
#include "numbertextctrl.h"
#include "position.h"

PositionCtrl::PositionCtrl(wxWindow* parent, const wxString& label, int x, int y, int z,
	int maxx /*= MAP_MAX_WIDTH*/, int maxy /*= MAP_MAX_HEIGHT*/, int maxz /*= MAP_MAX_LAYER*/) :
wxStaticBoxSizer(wxHORIZONTAL, parent, label)
{
	x_field = newd NumberTextCtrl(parent, wxID_ANY, x, 0, maxx, wxTE_PROCESS_ENTER, "X", wxDefaultPosition, wxSize(60, 20));
	x_field->Bind(wxEVT_TEXT_PASTE, &PositionCtrl::OnClipboardText, this);
	Add(x_field, 2, wxEXPAND | wxLEFT | wxBOTTOM, 5);

	y_field = newd NumberTextCtrl(parent, wxID_ANY, y, 0, maxy, wxTE_PROCESS_ENTER, "Y", wxDefaultPosition, wxSize(60, 20));
	y_field->Bind(wxEVT_TEXT_PASTE, &PositionCtrl::OnClipboardText, this);
	Add(y_field, 2, wxEXPAND | wxLEFT | wxBOTTOM, 5);

	z_field = newd NumberTextCtrl(parent, wxID_ANY, z, 0, maxz, wxTE_PROCESS_ENTER, "Z", wxDefaultPosition, wxSize(35, 20));
	z_field->Bind(wxEVT_TEXT_PASTE, &PositionCtrl::OnClipboardText, this);
	Add(z_field, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
}

PositionCtrl::~PositionCtrl()
{
	////
}

Position PositionCtrl::GetPosition() const
{
	Position pos;
	pos.x = x_field->GetIntValue();
	pos.y = y_field->GetIntValue();
	pos.z = z_field->GetIntValue();
	return pos;
}

void PositionCtrl::SetPosition(Position pos)
{
	x_field->SetIntValue(pos.x);
	y_field->SetIntValue(pos.y);
	z_field->SetIntValue(pos.z);
}

bool PositionCtrl::Enable(bool enable)
{
	return (x_field->Enable(enable) && y_field->Enable(enable) && z_field->Enable(enable));
}

void PositionCtrl::OnClipboardText(wxClipboardTextEvent& evt)
{
	Position position;
	if(posFromClipboard(position.x, position.y, position.z)) {
		x_field->SetIntValue(position.x);
		y_field->SetIntValue(position.y);
		z_field->SetIntValue(position.z);
	} else {
		evt.Skip();
	}
}
