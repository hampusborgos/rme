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

#ifndef RME_SCRIPTING_WINDOW_H_
#define RME_SCRIPTING_WINDOW_H_

#include <wx/stc/stc.h>

class ScriptingWindow : public wxDialog
{
public:
	ScriptingWindow(wxWindow* parent);
	~ScriptingWindow();

	void AppendLog(const wxString& text);

	void OnToolClicked(wxCommandEvent&);
	void OnMarginClick(wxStyledTextEvent &event);

private:
	wxToolBar* tool_bar;
	wxToolBarToolBase* run_button;

	wxStyledTextCtrl* code_text;
	wxTextCtrl* output_text;
};

#endif
