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

static const char* LuaKeywords1 =
	"and break do else elseif end for function if "
	"in local not or repeat return then until while";
static const char* LuaKeywords2 =
	"false true nil";
static const char* LuaKeywords3 =
	"print type tonumber tostring dofile unpack "
	"ipairs pairs next pcall "
	"io.close io.flush io.input io.lines io.open io.output "
	"io.read io.tmpfile io.type io.write io.stdin io.stdout io.stderr "
	"os.clock os.date os.difftime os.execute os.exit os.getenv os.remove "
	"os.rename os.setlocale os.time os.tmpname "
	"string.byte string.char string.dump string.find string.len "
	"string.lower string.rep string.sub string.upper string.format "
	"string.gfind string.gsub table.concat table.foreach table.foreachi "
	"table.getn table.sort table.insert table.remove table.setn "
	"math.abs math.acos math.asin math.atan math.atan2 math.ceil math.cos math.deg "
	"math.exp math.floor math.frexp math.ldexp math.log math.log10 math.max "
	"math.min math.mod math.pi math.pow math.rad math.random math.randomseed "
	"math.sin math.sqrt math.tan";

enum
{
	MARGIN_LINE_NUMBERS,
	MARGIN_FOLD
};

BEGIN_EVENT_TABLE(ScriptingWindow, wxDialog)
	EVT_BUTTON(wxID_APPLY, ScriptingWindow::OnRunScript)
END_EVENT_TABLE()

ScriptingWindow::ScriptingWindow(wxWindow* parent) :
	wxDialog(parent, wxID_ANY, "Scripting", wxDefaultPosition, wxSize(800, 600), wxRESIZE_BORDER | wxCAPTION | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX)
{
	wxColor defaultBackgroundColor(39, 40, 34);

	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	script_text_field = newd wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(600, 350), wxTE_MULTILINE);
	script_text_field->SetMinSize(wxSize(500, 300));
	script_text_field->StyleSetBackground(wxSTC_STYLE_DEFAULT, defaultBackgroundColor);
	script_text_field->SetSelBackground(true, wxColour(73, 72, 62));
	script_text_field->SetCaretForeground(wxColour(255, 255, 255));

	// Font
	wxFont font = wxFont(11, wxFontFamily::wxFONTFAMILY_SCRIPT, wxFontStyle::wxFONTSTYLE_NORMAL, wxFontWeight::wxFONTWEIGHT_NORMAL);
	script_text_field->StyleSetFont(wxSTC_STYLE_DEFAULT, font);
	
	// Lua Highlights
	script_text_field->SetLexer(wxSTC_LEX_LUA);
	script_text_field->StyleSetBackground(wxSTC_LUA_DEFAULT, defaultBackgroundColor);
	script_text_field->StyleSetBackground(wxSTC_LUA_COMMENT, defaultBackgroundColor);
	script_text_field->StyleSetForeground(wxSTC_LUA_COMMENT, wxColour(117, 113, 94));
	script_text_field->StyleSetBackground(wxSTC_LUA_COMMENTLINE, defaultBackgroundColor);
	script_text_field->StyleSetForeground(wxSTC_LUA_COMMENTLINE, wxColour(117, 113, 94));
	script_text_field->StyleSetBackground(wxSTC_LUA_COMMENTDOC, defaultBackgroundColor);
	script_text_field->StyleSetForeground(wxSTC_LUA_COMMENTDOC, wxColour(117, 113, 94));
	script_text_field->StyleSetBackground(wxSTC_LUA_NUMBER, defaultBackgroundColor);
	script_text_field->StyleSetForeground(wxSTC_LUA_NUMBER, wxColour(174, 129, 255));
	script_text_field->StyleSetBackground(wxSTC_LUA_WORD, defaultBackgroundColor);
	script_text_field->StyleSetForeground(wxSTC_LUA_WORD, wxColour(249, 38, 114));
	script_text_field->StyleSetBackground(wxSTC_LUA_STRING, defaultBackgroundColor);
	script_text_field->StyleSetForeground(wxSTC_LUA_STRING, wxColour(230, 219, 116));
	script_text_field->StyleSetBackground(wxSTC_LUA_CHARACTER, defaultBackgroundColor);
	script_text_field->StyleSetForeground(wxSTC_LUA_CHARACTER, wxColour(230, 219, 116));
	script_text_field->StyleSetBackground(wxSTC_LUA_LITERALSTRING, defaultBackgroundColor);
	script_text_field->StyleSetForeground(wxSTC_LUA_LITERALSTRING, wxColour(230, 219, 116));
	script_text_field->StyleSetBackground(wxSTC_LUA_PREPROCESSOR, defaultBackgroundColor);
	script_text_field->StyleSetForeground(wxSTC_LUA_PREPROCESSOR, wxColour(230, 219, 116));
	script_text_field->StyleSetBackground(wxSTC_LUA_OPERATOR, defaultBackgroundColor);
	script_text_field->StyleSetForeground(wxSTC_LUA_OPERATOR, wxColour(255, 255, 255));
	script_text_field->StyleSetBackground(wxSTC_LUA_IDENTIFIER, defaultBackgroundColor);
	script_text_field->StyleSetForeground(wxSTC_LUA_IDENTIFIER, wxColour(255, 255, 255));
	script_text_field->StyleSetBackground(wxSTC_LUA_WORD2, defaultBackgroundColor);
	script_text_field->StyleSetForeground(wxSTC_LUA_WORD2, wxColour(174, 129, 255));
	script_text_field->StyleSetBackground(wxSTC_LUA_WORD3, defaultBackgroundColor);
	script_text_field->StyleSetForeground(wxSTC_LUA_WORD3, wxColour(102, 217, 239));
	script_text_field->StyleSetBackground(wxSTC_LUA_LABEL, defaultBackgroundColor);
	script_text_field->StyleSetForeground(wxSTC_LUA_LABEL, wxColour(255, 255, 255));

	// Margin numbers
	script_text_field->SetMarginType(MARGIN_LINE_NUMBERS, wxSTC_MARGIN_NUMBER);
	script_text_field->SetMarginWidth(MARGIN_LINE_NUMBERS, 50);
	script_text_field->StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour(24, 24, 24));
	script_text_field->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(144, 144, 138));

	// Margin fold
	script_text_field->SetMarginType(MARGIN_FOLD, wxSTC_MARGIN_SYMBOL);
	script_text_field->SetMarginWidth(MARGIN_FOLD, 15);
	script_text_field->SetMarginMask(MARGIN_FOLD, wxSTC_MASK_FOLDERS);
	script_text_field->StyleSetBackground(MARGIN_FOLD, wxColour(255, 0, 0));
	script_text_field->SetFoldMarginColour(true, wxColour(24, 24, 24));
	script_text_field->SetFoldMarginHiColour(true, wxColour(24, 24, 24));
	script_text_field->SetMarginSensitive(MARGIN_FOLD, true);

	script_text_field->SetProperty("fold", "1");
	script_text_field->SetProperty("fold.comment", "1");
	script_text_field->SetProperty("fold.compact", "1");

	wxColor folderColor(134, 134, 134);
	script_text_field->MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXPLUSCONNECTED);
	script_text_field->MarkerSetForeground(wxSTC_MARKNUM_FOLDER, folderColor);
	script_text_field->MarkerSetBackground(wxSTC_MARKNUM_FOLDER, defaultBackgroundColor);

	script_text_field->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXMINUSCONNECTED);
	script_text_field->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPEN, folderColor);
	script_text_field->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPEN, defaultBackgroundColor);

	script_text_field->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_VLINE);
	script_text_field->MarkerSetForeground(wxSTC_MARKNUM_FOLDERSUB, folderColor);
	script_text_field->MarkerSetBackground(wxSTC_MARKNUM_FOLDERSUB, defaultBackgroundColor);

	script_text_field->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_VLINE);
	script_text_field->MarkerSetForeground(wxSTC_MARKNUM_FOLDEREND, folderColor);
	script_text_field->MarkerSetBackground(wxSTC_MARKNUM_FOLDEREND, defaultBackgroundColor);

	script_text_field->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_VLINE);
	script_text_field->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPENMID, folderColor);
	script_text_field->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPENMID, defaultBackgroundColor);

	script_text_field->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_VLINE);
	script_text_field->MarkerSetForeground(wxSTC_MARKNUM_FOLDERMIDTAIL, folderColor);
	script_text_field->MarkerSetBackground(wxSTC_MARKNUM_FOLDERMIDTAIL, defaultBackgroundColor);

	script_text_field->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_LCORNER);
	script_text_field->MarkerSetForeground(wxSTC_MARKNUM_FOLDERTAIL, folderColor);
	script_text_field->MarkerSetBackground(wxSTC_MARKNUM_FOLDERTAIL, defaultBackgroundColor);

	// Indentation
	script_text_field->SetIndentationGuides(2);
	script_text_field->SetHighlightGuide(1);
	script_text_field->SetIndent(4);
	script_text_field->StyleSetBackground(wxSTC_STYLE_INDENTGUIDE, defaultBackgroundColor);
	script_text_field->StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour(70, 71, 65));

	// Keywords
	script_text_field->SetKeyWords(0, wxString(LuaKeywords1));
	script_text_field->SetKeyWords(1, wxString(LuaKeywords2));
	script_text_field->SetKeyWords(2, wxString(LuaKeywords3));
	sizer->Add(script_text_field, wxSizerFlags(5).Expand());

	sizer->AddSpacer(5);

	run_button = newd wxButton(this, wxID_APPLY, "Run");
	sizer->Add(run_button, 0, wxLEFT, 5);

	sizer->AddSpacer(5);

	log_text_field = newd wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(600, 250), wxTE_MULTILINE | wxTE_READONLY);
	log_text_field->SetMinSize(wxSize(500, 200));
	sizer->Add(log_text_field, wxSizerFlags(0).Expand());

	SetSizerAndFit(sizer);
	Centre(wxBOTH);

	script_text_field->Connect(wxEVT_STC_MARGINCLICK, wxStyledTextEventHandler(ScriptingWindow::OnMarginClick), NULL, this);
}

ScriptingWindow::~ScriptingWindow()
{
	script_text_field->Disconnect(wxEVT_STC_MARGINCLICK, wxStyledTextEventHandler(ScriptingWindow::OnMarginClick), NULL, this);
}

void ScriptingWindow::AppendLog(const wxString& text)
{
	log_text_field->AppendText(text + "\n");
}

void ScriptingWindow::OnRunScript(wxCommandEvent&)
{
	log_text_field->SetValue(wxEmptyString);
	g_lua.runScript(script_text_field->GetValue());
}

void ScriptingWindow::OnMarginClick(wxStyledTextEvent &event)
{
	if(event.GetMargin() == MARGIN_FOLD) {
		int lineClick = script_text_field->LineFromPosition(event.GetPosition());
		int levelClick = script_text_field->GetFoldLevel(lineClick);
		if(levelClick == (wxSTC_FOLDLEVELBASE | wxSTC_FOLDLEVELHEADERFLAG))
			script_text_field->ToggleFold(lineClick);
	}
}
