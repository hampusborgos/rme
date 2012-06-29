
#ifndef RME_DAT_DEBUG_VIEW_H_
#define RME_DAT_DEBUG_VIEW_H_

class DatDebugViewListBox;

class DatDebugView : wxPanel
{
public:
	DatDebugView(wxWindow* parent);
	~DatDebugView();

	void OnTextChange(wxCommandEvent&);
	void OnClickList(wxCommandEvent&);
	
protected:

	DatDebugViewListBox* item_list;
	wxTextCtrl* search_field;

	DECLARE_EVENT_TABLE()
};

#endif
