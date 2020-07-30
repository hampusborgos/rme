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
#include "numbertextctrl.h"

BEGIN_EVENT_TABLE(NumberTextCtrl, wxTextCtrl)
	EVT_KILL_FOCUS(NumberTextCtrl::OnKillFocus)
	EVT_TEXT_ENTER(wxID_ANY, NumberTextCtrl::OnTextEnter)
END_EVENT_TABLE()

NumberTextCtrl::NumberTextCtrl(wxWindow* parent, wxWindowID id,
		long value, long minvalue, long maxvalue,
		const wxPoint& pos, const wxSize& sz,
		long style, const wxString& name) :
	wxTextCtrl(parent, id, (wxString() << value), pos, sz, style, wxTextValidator(wxFILTER_NUMERIC), name),
	minval(minvalue), maxval(maxvalue), lastval(value)
{
	////
}

NumberTextCtrl::NumberTextCtrl(wxWindow* parent, wxWindowID id,
		long value, long minvalue, long maxvalue,
		long style, const wxString& name,
		const wxPoint& pos, const wxSize& sz) :
	wxTextCtrl(parent, id, (wxString() << value), pos, sz, style, wxTextValidator(wxFILTER_NUMERIC), name),
	minval(minvalue), maxval(maxvalue), lastval(value)
{
	////
}

NumberTextCtrl::~NumberTextCtrl()
{
	////
}

void NumberTextCtrl::OnKillFocus(wxFocusEvent& evt)
{
	CheckRange();
	evt.Skip();
}

void NumberTextCtrl::OnTextEnter(wxCommandEvent& evt)
{
	CheckRange();
}

void NumberTextCtrl::SetIntValue(long value)
{
	wxString sv;
	sv << value;
	// Will generate events
	SetValue(sv);
}

long NumberTextCtrl::GetIntValue()
{
	long l;
	if(GetValue().ToLong(&l))
		return l;
	return 0;
}

void NumberTextCtrl::SetMinValue(long value)
{
	if(value == minval)
		return;
	minval = value;
	CheckRange();
}

void NumberTextCtrl::SetMaxValue(long value)
{
	if(value == maxval)
		return;
	maxval = value;
	CheckRange();
}

void NumberTextCtrl::CheckRange()
{
	wxString text = GetValue();
	wxString ntext;

	for(size_t s = 0; s < text.size(); ++s) {
		if(text[s] >= '0' && text[s] <= '9')
			ntext.Append(text[s]);
	}

	// Check that value is in range
	long v;
	if(ntext.size() != 0 && ntext.ToLong(&v)) {
		if(v < minval)
			v = minval;
		else if(v > maxval)
			v = maxval;

		ntext.clear();
		ntext << v;
		lastval = v;
	} else {
		ntext.clear();
		ntext << lastval;
	}

	// Check if there was any change
	if(ntext != text) {
		// ChangeValue doesn't generate events
		ChangeValue(ntext);
	}
}

