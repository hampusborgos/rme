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

static const char LuaKeywords[] =	"and break do else elseif end false for function if in "
									"local nil not or repeat return then true  until while pairs";

BEGIN_EVENT_TABLE(ScriptingWindow, wxDialog)
	EVT_BUTTON(wxID_APPLY, ScriptingWindow::OnRunScript)
END_EVENT_TABLE()

ScriptingWindow::ScriptingWindow(wxWindow* parent) :
	wxDialog(parent, wxID_ANY, "Scripting", wxDefaultPosition, wxSize(800, 600), wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX)
{
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	script_text_field = newd wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(600, 350), wxTE_MULTILINE);
	script_text_field->SetMinSize(wxSize(500, 300));
	script_text_field->SetMarginWidth(30, 14);
	script_text_field->SetLexer(wxSTC_LEX_LUA);
	script_text_field->StyleSetSpec(wxSTC_P_DEFAULT, wxT("fore:#808080"));
	script_text_field->StyleSetSpec(wxSTC_P_COMMENTLINE, wxT("fore:#007F00"));
	script_text_field->StyleSetSpec(wxSTC_P_NUMBER, wxT("fore:#007F7F"));
	script_text_field->StyleSetSpec(wxSTC_P_STRING, wxT("fore:#7F007F"));
	script_text_field->StyleSetSpec(wxSTC_P_CHARACTER, wxT("fore:#7F007F"));
	script_text_field->StyleSetSpec(wxSTC_P_WORD, wxT("bold,fore:#004EFF"));
	script_text_field->StyleSetSpec(wxSTC_P_TRIPLE, wxT("fore:#7F0000"));
	script_text_field->StyleSetSpec(wxSTC_P_TRIPLEDOUBLE, wxT("fore:#7F0000"));
	script_text_field->StyleSetSpec(wxSTC_P_CLASSNAME, wxT("bold,fore:#0000FF"));
	script_text_field->StyleSetSpec(wxSTC_P_DEFNAME, wxT("bold,fore:#007F7F"));
	script_text_field->StyleSetSpec(wxSTC_P_OPERATOR, wxT("bold"));
	script_text_field->StyleSetSpec(wxSTC_P_IDENTIFIER, wxT(""));
	script_text_field->StyleSetSpec(wxSTC_P_COMMENTBLOCK, wxT("fore:#7F7F7F"));
	script_text_field->StyleSetSpec(wxSTC_P_STRINGEOL, wxT("fore:#000000,back:#E0C0E0,eol"));
	script_text_field->StyleSetSpec(wxSTC_P_WORD2, wxT("fore:#203040,back:#FFFFC0"));
	script_text_field->StyleSetSpec(wxSTC_P_DECORATOR, wxT("fore:#805000"));
	script_text_field->StyleSetSpec(wxSTC_STYLE_BRACELIGHT, wxT("fore:#0000FF,bold"));
	script_text_field->StyleSetSpec(wxSTC_STYLE_BRACEBAD, wxT("fore:#FF0000,bold"));

	script_text_field->SetMarginType(0, wxSTC_MARGIN_NUMBER);
	script_text_field->SetMarginWidth(2, 16);
	script_text_field->SetIndent(4);
	script_text_field->SetKeyWords(0, wxString(LuaKeywords));
	script_text_field->SetKeyWords(3, wxT("Tile Selection Editor"));
	sizer->Add(script_text_field, wxSizerFlags(5).Expand());

	sizer->AddSpacer(5);

	run_button = newd wxButton(this, wxID_APPLY, wxT("Run"));
	sizer->Add(run_button, 0, wxLEFT, 5);

	sizer->AddSpacer(5);

	log_text_field = newd wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(600, 250), wxTE_MULTILINE | wxTE_READONLY);
	log_text_field->SetMinSize(wxSize(500, 200));
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
