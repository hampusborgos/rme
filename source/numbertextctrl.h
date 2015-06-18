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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/numbertextctrl.h $
// $Id: numbertextctrl.h 264 2009-10-05 06:36:21Z remere $

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

	void OnEnterText(wxCommandEvent&);

	long GetIntValue();
	void SetIntValue(long v);
protected:
	long minval, maxval, lastval;
	DECLARE_EVENT_TABLE();
};

#endif
