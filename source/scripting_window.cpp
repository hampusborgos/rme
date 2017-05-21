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
#include "../icons/run.xpm"

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

ScriptingWindow::ScriptingWindow(wxWindow* parent) :
	wxDialog(parent, wxID_ANY, "Scripting", wxDefaultPosition, wxSize(800, 600), wxRESIZE_BORDER | wxCAPTION | wxMINIMIZE_BOX | wxMAXIMIZE_BOX | wxCLOSE_BOX)
{
	wxColor defaultBackgroundColor(39, 40, 34);

	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);

	// Tool Bar
	tool_bar = new wxToolBar(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxTB_HORIZONTAL);
	run_button = tool_bar->AddTool(wxID_ANY, "Run", wxBitmap(run_xpm), wxNullBitmap, wxITEM_NORMAL, "Run Script", wxEmptyString, NULL);
	tool_bar->Realize();
	sizer->Add(tool_bar, 0, wxALL, 5);

	// Code editor
	code_text = newd wxStyledTextCtrl(this, wxID_ANY, wxDefaultPosition, wxSize(600, 350), wxTE_MULTILINE);
	code_text->SetMinSize(wxSize(500, 300));
	code_text->StyleSetBackground(wxSTC_STYLE_DEFAULT, defaultBackgroundColor);
	code_text->SetSelBackground(true, wxColour(73, 72, 62));
	code_text->SetCaretForeground(wxColour(255, 255, 255));

	// Font
	wxFont font = wxFont(11, wxFontFamily::wxFONTFAMILY_SCRIPT, wxFontStyle::wxFONTSTYLE_NORMAL, wxFontWeight::wxFONTWEIGHT_NORMAL);
	code_text->StyleSetFont(wxSTC_STYLE_DEFAULT, font);
	
	// Lua Highlights
	code_text->SetLexer(wxSTC_LEX_LUA);
	code_text->StyleSetBackground(wxSTC_LUA_DEFAULT, defaultBackgroundColor);
	code_text->StyleSetBackground(wxSTC_LUA_COMMENT, defaultBackgroundColor);
	code_text->StyleSetForeground(wxSTC_LUA_COMMENT, wxColour(117, 113, 94));
	code_text->StyleSetBackground(wxSTC_LUA_COMMENTLINE, defaultBackgroundColor);
	code_text->StyleSetForeground(wxSTC_LUA_COMMENTLINE, wxColour(117, 113, 94));
	code_text->StyleSetBackground(wxSTC_LUA_COMMENTDOC, defaultBackgroundColor);
	code_text->StyleSetForeground(wxSTC_LUA_COMMENTDOC, wxColour(117, 113, 94));
	code_text->StyleSetBackground(wxSTC_LUA_NUMBER, defaultBackgroundColor);
	code_text->StyleSetForeground(wxSTC_LUA_NUMBER, wxColour(174, 129, 255));
	code_text->StyleSetBackground(wxSTC_LUA_WORD, defaultBackgroundColor);
	code_text->StyleSetForeground(wxSTC_LUA_WORD, wxColour(249, 38, 114));
	code_text->StyleSetBackground(wxSTC_LUA_STRING, defaultBackgroundColor);
	code_text->StyleSetForeground(wxSTC_LUA_STRING, wxColour(230, 219, 116));
	code_text->StyleSetBackground(wxSTC_LUA_CHARACTER, defaultBackgroundColor);
	code_text->StyleSetForeground(wxSTC_LUA_CHARACTER, wxColour(230, 219, 116));
	code_text->StyleSetBackground(wxSTC_LUA_LITERALSTRING, defaultBackgroundColor);
	code_text->StyleSetForeground(wxSTC_LUA_LITERALSTRING, wxColour(230, 219, 116));
	code_text->StyleSetBackground(wxSTC_LUA_PREPROCESSOR, defaultBackgroundColor);
	code_text->StyleSetForeground(wxSTC_LUA_PREPROCESSOR, wxColour(230, 219, 116));
	code_text->StyleSetBackground(wxSTC_LUA_OPERATOR, defaultBackgroundColor);
	code_text->StyleSetForeground(wxSTC_LUA_OPERATOR, wxColour(255, 255, 255));
	code_text->StyleSetBackground(wxSTC_LUA_IDENTIFIER, defaultBackgroundColor);
	code_text->StyleSetForeground(wxSTC_LUA_IDENTIFIER, wxColour(255, 255, 255));
	code_text->StyleSetBackground(wxSTC_LUA_WORD2, defaultBackgroundColor);
	code_text->StyleSetForeground(wxSTC_LUA_WORD2, wxColour(174, 129, 255));
	code_text->StyleSetBackground(wxSTC_LUA_WORD3, defaultBackgroundColor);
	code_text->StyleSetForeground(wxSTC_LUA_WORD3, wxColour(102, 217, 239));
	code_text->StyleSetBackground(wxSTC_LUA_LABEL, defaultBackgroundColor);
	code_text->StyleSetForeground(wxSTC_LUA_LABEL, wxColour(255, 255, 255));

	// Margin numbers
	code_text->SetMarginType(MARGIN_LINE_NUMBERS, wxSTC_MARGIN_NUMBER);
	code_text->SetMarginWidth(MARGIN_LINE_NUMBERS, 50);
	code_text->StyleSetBackground(wxSTC_STYLE_LINENUMBER, wxColour(24, 24, 24));
	code_text->StyleSetForeground(wxSTC_STYLE_LINENUMBER, wxColour(144, 144, 138));

	// Margin fold
	code_text->SetMarginType(MARGIN_FOLD, wxSTC_MARGIN_SYMBOL);
	code_text->SetMarginWidth(MARGIN_FOLD, 15);
	code_text->SetMarginMask(MARGIN_FOLD, wxSTC_MASK_FOLDERS);
	code_text->StyleSetBackground(MARGIN_FOLD, wxColour(255, 0, 0));
	code_text->SetFoldMarginColour(true, wxColour(24, 24, 24));
	code_text->SetFoldMarginHiColour(true, wxColour(24, 24, 24));
	code_text->SetMarginSensitive(MARGIN_FOLD, true);

	code_text->SetProperty("fold", "1");
	code_text->SetProperty("fold.comment", "1");
	code_text->SetProperty("fold.compact", "1");

	wxColor folderColor(134, 134, 134);
	code_text->MarkerDefine(wxSTC_MARKNUM_FOLDER, wxSTC_MARK_BOXPLUSCONNECTED);
	code_text->MarkerSetForeground(wxSTC_MARKNUM_FOLDER, folderColor);
	code_text->MarkerSetBackground(wxSTC_MARKNUM_FOLDER, defaultBackgroundColor);

	code_text->MarkerDefine(wxSTC_MARKNUM_FOLDEROPEN, wxSTC_MARK_BOXMINUSCONNECTED);
	code_text->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPEN, folderColor);
	code_text->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPEN, defaultBackgroundColor);

	code_text->MarkerDefine(wxSTC_MARKNUM_FOLDERSUB, wxSTC_MARK_VLINE);
	code_text->MarkerSetForeground(wxSTC_MARKNUM_FOLDERSUB, folderColor);
	code_text->MarkerSetBackground(wxSTC_MARKNUM_FOLDERSUB, defaultBackgroundColor);

	code_text->MarkerDefine(wxSTC_MARKNUM_FOLDEREND, wxSTC_MARK_VLINE);
	code_text->MarkerSetForeground(wxSTC_MARKNUM_FOLDEREND, folderColor);
	code_text->MarkerSetBackground(wxSTC_MARKNUM_FOLDEREND, defaultBackgroundColor);

	code_text->MarkerDefine(wxSTC_MARKNUM_FOLDEROPENMID, wxSTC_MARK_VLINE);
	code_text->MarkerSetForeground(wxSTC_MARKNUM_FOLDEROPENMID, folderColor);
	code_text->MarkerSetBackground(wxSTC_MARKNUM_FOLDEROPENMID, defaultBackgroundColor);

	code_text->MarkerDefine(wxSTC_MARKNUM_FOLDERMIDTAIL, wxSTC_MARK_VLINE);
	code_text->MarkerSetForeground(wxSTC_MARKNUM_FOLDERMIDTAIL, folderColor);
	code_text->MarkerSetBackground(wxSTC_MARKNUM_FOLDERMIDTAIL, defaultBackgroundColor);

	code_text->MarkerDefine(wxSTC_MARKNUM_FOLDERTAIL, wxSTC_MARK_LCORNER);
	code_text->MarkerSetForeground(wxSTC_MARKNUM_FOLDERTAIL, folderColor);
	code_text->MarkerSetBackground(wxSTC_MARKNUM_FOLDERTAIL, defaultBackgroundColor);

	// Indentation
	code_text->SetIndentationGuides(2);
	code_text->SetHighlightGuide(1);
	code_text->SetIndent(4);
	code_text->StyleSetBackground(wxSTC_STYLE_INDENTGUIDE, defaultBackgroundColor);
	code_text->StyleSetForeground(wxSTC_STYLE_INDENTGUIDE, wxColour(70, 71, 65));

	// Keywords
	code_text->SetKeyWords(0, wxString(LuaKeywords1));
	code_text->SetKeyWords(1, wxString(LuaKeywords2));
	code_text->SetKeyWords(2, wxString(LuaKeywords3));
	sizer->Add(code_text, wxSizerFlags(5).Expand());

	output_text = newd wxTextCtrl(this, wxID_ANY, wxEmptyString, wxDefaultPosition, wxSize(600, 250), wxTE_MULTILINE | wxTE_READONLY);
	output_text->SetMinSize(wxSize(500, 200));
	output_text->SetBackgroundColour(defaultBackgroundColor);
	output_text->SetForegroundColour(wxColour(255, 255, 255));
	sizer->Add(output_text, wxSizerFlags(0).Expand());

	SetSizerAndFit(sizer);
	Centre(wxBOTH);

	// Connect Events
	Connect(run_button->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(ScriptingWindow::OnToolClicked));
	code_text->Connect(wxEVT_STC_MARGINCLICK, wxStyledTextEventHandler(ScriptingWindow::OnMarginClick), NULL, this);
}

ScriptingWindow::~ScriptingWindow()
{
	// Disconnect Events
	Disconnect(run_button->GetId(), wxEVT_COMMAND_TOOL_CLICKED, wxCommandEventHandler(ScriptingWindow::OnToolClicked));
	code_text->Disconnect(wxEVT_STC_MARGINCLICK, wxStyledTextEventHandler(ScriptingWindow::OnMarginClick), NULL, this);
}

void ScriptingWindow::AppendLog(const wxString& text)
{
	output_text->AppendText(text + "\n");
}

void ScriptingWindow::OnToolClicked(wxCommandEvent&)
{
	output_text->SetValue(wxEmptyString);
	g_lua.runScript(code_text->GetValue());
}

void ScriptingWindow::OnMarginClick(wxStyledTextEvent &event)
{
	if(event.GetMargin() == MARGIN_FOLD) {
		int lineClick = code_text->LineFromPosition(event.GetPosition());
		int levelClick = code_text->GetFoldLevel(lineClick);
		if(levelClick == (wxSTC_FOLDLEVELBASE | wxSTC_FOLDLEVELHEADERFLAG))
			code_text->ToggleFold(lineClick);
	}
}
