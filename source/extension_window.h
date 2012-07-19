//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_EXTENSION_WINDOW_H
#define RME_EXTENSION_WINDOW_H

class MaterialsExtension;

class ExtensionsDialog : public wxDialog
{
public:
	ExtensionsDialog(wxWindow* parent);
	virtual ~ExtensionsDialog();

	void OnClickOK(wxCommandEvent& evt);
	void OnClickOpenFolder(wxCommandEvent& evt);
	void OnClickLink(wxHtmlLinkEvent& evt);

	DECLARE_EVENT_TABLE();

private:
	wxString HTML() const;
	wxString HTMLForExtension(MaterialsExtension* me) const;
};


#endif
