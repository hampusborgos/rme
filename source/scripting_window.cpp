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

#include "scripting_window.h"
#include "luascript.h"

BEGIN_EVENT_TABLE(ScriptingWindow, wxDialog)
	EVT_BUTTON(wxID_APPLY, ScriptingWindow::OnRunScript)
END_EVENT_TABLE()

ScriptingWindow::ScriptingWindow(wxWindow* parent) :
	wxDialog(parent, wxID_ANY, "Scripting", wxDefaultPosition, wxSize(600, 400), wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX)
{
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	script_text_field = newd wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(600, 200), wxTE_MULTILINE);
	script_text_field->SetMinSize(wxSize(400, 200));
	sizer->Add(script_text_field, wxSizerFlags(5).Expand());

	sizer->AddSpacer(5);

	run_button = newd wxButton(this, wxID_APPLY, wxT("Run"));
	sizer->Add(run_button, 0, wxLEFT, 5);

	sizer->AddSpacer(5);

	log_text_field = newd wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(600, 150), wxTE_MULTILINE | wxTE_READONLY);
	log_text_field->SetMinSize(wxSize(400, 100));
	sizer->Add(log_text_field, wxSizerFlags(0).Expand());

	SetSizerAndFit(sizer);
	Centre(wxBOTH);
}

ScriptingWindow::~ScriptingWindow()
{
	////
}

void ScriptingWindow::OnRunScript(wxCommandEvent&)
{
	log_text_field->SetValue(wxEmptyString);
	g_lua.runScript(script_text_field->GetValue());
}

void ScriptingWindow::AppendLog(const wxString& text)
{
	log_text_field->AppendText(text + wxT("\n"));
}
