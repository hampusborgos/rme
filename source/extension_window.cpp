//////////////////////////////////////////////////////////////////////
// This file is part of Canary Map Editor
//////////////////////////////////////////////////////////////////////
// Canary Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Canary Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "extension_window.h"

#include "gui.h"
#include "materials.h"

extern Materials g_materials;

BEGIN_EVENT_TABLE(ExtensionsDialog, wxDialog)
	EVT_BUTTON(wxID_OK, ExtensionsDialog::OnClickOK)
	EVT_BUTTON(EXTENSIONS_OPEN_FOLDER_BUTTON, ExtensionsDialog::OnClickOpenFolder)
END_EVENT_TABLE()

ExtensionsDialog::ExtensionsDialog(wxWindow* parent) :
	wxDialog(parent, wxID_ANY, "Extensions", wxDefaultPosition, wxSize(600, 500), wxRESIZE_BORDER | wxCAPTION)
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

	wxSizer* buttonSizer = newd wxBoxSizer(wxHORIZONTAL);
	buttonSizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center());
	buttonSizer->Add(newd wxButton(this, EXTENSIONS_OPEN_FOLDER_BUTTON, "Open Extensions Folder"), wxSizerFlags(1).Center());
	topSizer->Add(buttonSizer, 0, wxCENTER | wxLEFT | wxRIGHT | wxBOTTOM, 20);

	SetSizerAndFit(topSizer);
	Centre(wxBOTH);
}

ExtensionsDialog::~ExtensionsDialog()
{
	////
}

void ExtensionsDialog::OnClickOK(wxCommandEvent& evt)
{
	EndModal(0);
}

void ExtensionsDialog::OnClickOpenFolder(wxCommandEvent& evt)
{
	wxString cmd, extensionsDir = g_gui.GetExtensionsDirectory();
#if defined __WINDOWS__
	cmd << "explorer";
#elif defined __APPLE__
	cmd << "open";
	extensionsDir.Replace(" ", "\\ "); //Escape spaces
#elif defined __linux
	cmd << "xdg-open";
#else
#error "NOT IMPLEMENTED"
#endif

	cmd << " " << extensionsDir;

	wxExecute(cmd);
}
