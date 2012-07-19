//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "extension_window.h"

#include "gui.h"
#include "materials.h"

extern Materials materials;

BEGIN_EVENT_TABLE(ExtensionsDialog, wxDialog)
	EVT_HTML_LINK_CLICKED(wxID_ANY, ExtensionsDialog::OnClickLink)
	EVT_BUTTON(wxID_OK, ExtensionsDialog::OnClickOK)
	EVT_BUTTON(EXTENSIONS_OPEN_FOLDER_BUTTON, ExtensionsDialog::OnClickOpenFolder)
END_EVENT_TABLE()

ExtensionsDialog::ExtensionsDialog(wxWindow* parent) :
	wxDialog(parent, wxID_ANY, wxT("Extensions"), wxDefaultPosition, wxSize(600, 500), wxRESIZE_BORDER | wxCAPTION)
{
	wxBoxSizer* topSizer = new wxBoxSizer(wxVERTICAL);

	wxHtmlWindow* htmlWindow = new wxHtmlWindow(this, wxID_ANY, wxDefaultPosition, wxSize(550, 400));
	htmlWindow->SetPage(HTML());
	topSizer->Add(htmlWindow, wxSizerFlags(1).DoubleBorder().Expand());

	wxSizer* buttonSizer = newd wxBoxSizer(wxHORIZONTAL);
	buttonSizer->Add(newd wxButton(this, wxID_OK, wxT("OK")), wxSizerFlags(1).Center());
	buttonSizer->Add(newd wxButton(this, EXTENSIONS_OPEN_FOLDER_BUTTON, wxT("Open Extensions Folder")), wxSizerFlags(1).Center());
	topSizer->Add(buttonSizer, 0, wxCENTER | wxLEFT | wxRIGHT | wxBOTTOM, 20);

	SetSizerAndFit(topSizer);
}

ExtensionsDialog::~ExtensionsDialog()
{
}

void ExtensionsDialog::OnClickLink(wxHtmlLinkEvent& evt)
{
	::wxLaunchDefaultBrowser(evt.GetLinkInfo().GetHref(), wxBROWSER_NEW_WINDOW);
}

void ExtensionsDialog::OnClickOK(wxCommandEvent& evt)
{
	EndModal(0);
}

void ExtensionsDialog::OnClickOpenFolder(wxCommandEvent& evt)
{
	wxString cmd;
#if defined __WINDOWS__
	cmd << "explorer";
#elif defined __OSX__
	cmd << "open";
#else
#error "NOT IMPLEMENTED"
#endif

	cmd << " " << gui.GetExtensionsDirectory();
	wxExecute(cmd);
}

wxString ExtensionsDialog::HTML() const
{
	wxString markup;
	for (MaterialsExtensionList::const_iterator me = materials.getExtensions().begin();
			me != materials.getExtensions().end();
			++me)
	{
		markup << HTMLForExtension(*me);
		markup << "<hr>";
	}
	return markup;
}

wxString ExtensionsDialog::HTMLForExtension(MaterialsExtension* me) const
{
	wxString markup;
	markup
		<< "<table>"

		<< "< background='#ff0000'>"
		<< "<td width='100px'><b>Extension</b></td>"
		<< "<td>";
	
	if (me->url.empty())
		markup << me->name;
	else
		markup << "<a href='" << me->url << "'>" << me->name << "</a>";
	
	markup
		<< "</td>"
		<< "</tr>"

		<< "<tr>"
		<< "<td width='100px'><b>Author</b></td>"
		<< "<td>";

	if (me->author_url.empty())
		markup << me->author;
	else
		markup << "<a href='" << me->author_url << "'>" << me->author << "</a>";

	markup
		<< "</td>"
		<< "</tr>"

		<< "<tr>"
		<< "<td width='100px'><b>Description</b></td>"
		<< "<td>" << me->description << "</td>"
		<< "</tr>"

		<< "<tr>"
		<< "<td width='100px'><b>Clients</b></td>"
		<< "<td>" << me->getVersionString() << "</td>"
		<< "</tr>"

		<< "</table>"
	;

	return markup;
}
