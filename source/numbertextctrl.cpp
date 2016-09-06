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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/numbertextctrl.hpp $
// $Id: numbertextctrl.hpp 280 2010-02-14 23:46:31Z admin $

#include "main.h"

#include "numbertextctrl.h"

BEGIN_EVENT_TABLE(NumberTextCtrl, wxSpinCtrl)
	EVT_TEXT(wxID_ANY, NumberTextCtrl::OnEnterText)
END_EVENT_TABLE()

NumberTextCtrl::NumberTextCtrl(wxWindow* parent, wxWindowID id,
		long value, long minvalue, long maxvalue,
		const wxPoint& pos, const wxSize& sz,
		long style, const wxString& name) :
	wxSpinCtrl(parent, id, (wxString() << value), pos, sz, style, minvalue, maxvalue, value, name),
	minval(minvalue), maxval(maxvalue), lastval(value)
{
}

NumberTextCtrl::NumberTextCtrl(wxWindow* parent, wxWindowID id,
		long value, long minvalue, long maxvalue,
		long style, const wxString& name,
		const wxPoint& pos, const wxSize& sz) :
	wxSpinCtrl(parent, id, (wxString() << value), pos, sz, style, minvalue, maxvalue, value, name),
	minval(minvalue), maxval(maxvalue), lastval(value)
{
}

NumberTextCtrl::~NumberTextCtrl()
{
}

wxTextPos NumberTextCtrl::GetLastPosition() const {
	return wxstr(i2s(GetValue())).size();
}

void NumberTextCtrl::OnEnterText(wxCommandEvent& evt)
{
	//printf("%d\n", (int)GetInsertionPoint());
/*	wxString text = GetValue();
	wxString ntext;

	int p = GetInsertionPoint();
	if(text.size() && text[0] == wxT('-'))
		ntext.Append(wxT('-'));

	for(size_t s = 0; s < text.size(); ++s)
	{
		if(text[s] >= '0' && text[s] <= '9')
			ntext.Append(text[s]);
		else
			--p;
	}

	// Check that value is in range
	long v;
	if(ntext.ToLong(&v))
	{
		if(v < minval)
			v = minval;
		else if(v > maxval)
			v = maxval;

		ntext.clear();
		ntext << v;
		lastval = v;
	}
	else
	{
		ntext.clear();
		ntext << lastval;
	}

	// Check if there was any change
	if(ntext != text)
	{
		// User input was invalid, change the text to the cleaned one.

		// ChangeValue doesn't generate events
		ChangeValue(ntext);
		SetInsertionPoint(p);
	}*/
}

void NumberTextCtrl::SetIntValue(long v)
{
	wxString sv;
	sv << v;
	// Will generate events
	SetValue(sv);
}

long NumberTextCtrl::GetIntValue()
{
	return GetValue();
}

