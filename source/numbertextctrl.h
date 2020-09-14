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

#ifndef _RME_NUMBER_TEXT_CTRL_H_
#define _RME_NUMBER_TEXT_CTRL_H_

// Text ctrl that only allows number input
class NumberTextCtrl : public wxTextCtrl
{
public:
	NumberTextCtrl(wxWindow* parent, wxWindowID id = wxID_ANY,
		long value = 0, long minvalue = 0, long maxvalue = 100,
		const wxPoint& pos = wxDefaultPosition, const wxSize& sz = wxDefaultSize,
		long style = 0, const wxString& name = wxTextCtrlNameStr);
	NumberTextCtrl(wxWindow* parent, wxWindowID id = wxID_ANY,
		long value = 0, long minvalue = 0, long maxvalue = 100,
		long style = 0, const wxString& name = wxTextCtrlNameStr,
		const wxPoint& pos = wxDefaultPosition, const wxSize& sz = wxDefaultSize);
	~NumberTextCtrl();

	void OnKillFocus(wxFocusEvent&);
	void OnTextEnter(wxCommandEvent&);

	long GetIntValue();
	void SetIntValue(long value);

	void SetMinValue(long value);
	void SetMaxValue(long value);

protected:
	void CheckRange();

	long minval, maxval, lastval;
	DECLARE_EVENT_TABLE();
};

#endif
