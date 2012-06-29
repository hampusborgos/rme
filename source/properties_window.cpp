//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "properties_window.h"

#include "gui_ids.h"
#include "complexitem.h"

#include <wx/grid.h>

BEGIN_EVENT_TABLE(PropertiesWindow, wxDialog)
	EVT_BUTTON(wxID_OK, PropertiesWindow::OnClickOK)
	EVT_BUTTON(wxID_CANCEL, PropertiesWindow::OnClickCancel)

	EVT_SIZE(PropertiesWindow::OnResize)

	EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY, PropertiesWindow::OnNotebookPageChanged)
END_EVENT_TABLE()

PropertiesWindow::PropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile_parent, Item* item, wxPoint pos) :
	wxDialog(parent, wxID_ANY, wxT("Object properties"), pos, wxDefaultSize, wxDEFAULT_DIALOG_STYLE),
	currentPanel(NULL),
	edit_map(map),
	edit_tile(tile_parent),
	edit_item(item)
{
	ASSERT(edit_item);
	
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);

	notebook = newd wxNotebook(this, wxID_ANY, wxDefaultPosition, wxSize(600, 300));

	notebook->AddPage(createGeneralPanel(notebook), wxT("General"), true);
	notebook->AddPage(createAttributesPanel(notebook), wxT("Advanced"));

	topsizer->Add(notebook, wxSizerFlags(1).Expand().DoubleBorder());

	wxSizer* optsizer = newd wxBoxSizer(wxHORIZONTAL);
	optsizer->Add(newd wxButton(this, wxID_OK, wxT("OK")), wxSizerFlags(1).Center());
	optsizer->Add(newd wxButton(this, wxID_CANCEL, wxT("Cancel")), wxSizerFlags(1).Center());
	topsizer->Add(optsizer, wxSizerFlags(0).Center().DoubleBorder());

	SetSizerAndFit(topsizer);
}

PropertiesWindow::~PropertiesWindow()
{
	;
}

wxWindow* PropertiesWindow::createGeneralPanel(wxWindow* parent)
{
	wxPanel* panel = new wxPanel(parent, ITEM_PROPERTIES_GENERAL_TAB);
	wxFlexGridSizer* gridsizer = newd wxFlexGridSizer(2, 10, 10);
	gridsizer->AddGrowableCol(1);

	gridsizer->Add(newd wxStaticText(panel, wxID_ANY, wxT("ID ") + i2ws(edit_item->getID())));
	gridsizer->Add(newd wxStaticText(panel, wxID_ANY, wxT("\"") + wxstr(edit_item->getName()) + wxT("\"")));

	gridsizer->Add(newd wxStaticText(panel, wxID_ANY, wxT("Action ID")));
	wxSpinCtrl* action_id_field = newd wxSpinCtrl(panel, wxID_ANY, i2ws(edit_item->getActionID()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getActionID());
	gridsizer->Add(action_id_field, wxSizerFlags(1).Expand());

	gridsizer->Add(newd wxStaticText(panel, wxID_ANY, wxT("Unique ID")));
	wxSpinCtrl* unique_id_field = newd wxSpinCtrl(panel, wxID_ANY, i2ws(edit_item->getUniqueID()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getUniqueID());
	gridsizer->Add(unique_id_field, wxSizerFlags(1).Expand());

	panel->SetSizerAndFit(gridsizer);

	return panel;
}

wxWindow* PropertiesWindow::createContainerPanel(wxWindow* parent)
{
	wxPanel* panel = new wxPanel(parent, ITEM_PROPERTIES_CONTAINER_TAB);



	return panel;
}

wxWindow* PropertiesWindow::createAttributesPanel(wxWindow* parent)
{
	//wxPanel* panel = new wxPanel(parent, wxID_ANY);
	
	wxSizer* sizer = newd wxBoxSizer(wxHORIZONTAL);
	wxGrid *grid = newd wxGrid(parent, ITEM_PROPERTIES_ADVANCED_TAB, wxDefaultPosition, wxSize(-1, 160));

	wxFont time_font(*wxSWISS_FONT);
	grid->SetDefaultCellFont(time_font);
	grid->CreateGrid(0, 3);
	grid->DisableDragRowSize();
	grid->DisableDragColSize();
	grid->SetSelectionMode(wxGrid::wxGridSelectRows);
	grid->SetRowLabelSize(0);
	//log->SetColLabelSize(0);
	//log->EnableGridLines(false);
	grid->EnableEditing(true);

	grid->SetColLabelValue(0, wxT("Key"));
	grid->SetColSize(0, 100);
	grid->SetColLabelValue(1, wxT("Type"));
	grid->SetColSize(1, 80);
	grid->SetColLabelValue(2, wxT("Value"));
	grid->SetColSize(2, 410);

	wxArrayString types;
	types.Add(wxT("Number"));
	types.Add(wxT("Float"));
	types.Add(wxT("Boolean"));
	types.Add(wxT("String"));

	// contents
	ItemAttributeMap attrs = edit_item->getAttributes();
	grid->AppendRows(attrs.size());
	int i = 0;
	for (ItemAttributeMap::iterator aiter = attrs.begin(); aiter != attrs.end(); ++aiter, ++i)
	{
		const ItemAttribute &attr = aiter->second;
		grid->SetCellValue(wxstr(aiter->first), i, 0);
		switch (aiter->second.type)
		{
		case ItemAttribute::STRING:
			{
				grid->SetCellValue(wxT("String"), i, 1);
				grid->SetCellValue(wxstr(*attr.getString()), i, 2);
				break;
			}
		case ItemAttribute::INTEGER:
			{
				grid->SetCellValue(wxT("Number"), i, 1);
				grid->SetCellValue(i2ws(*attr.getInteger()), i, 2);
				grid->SetCellEditor(i, 2, new wxGridCellNumberEditor);
				break;
			}
		case ItemAttribute::FLOAT:
			{
				grid->SetCellValue(wxT("Float"), i, 1);
				wxString f;
				f << *attr.getFloat();
				grid->SetCellValue(f, i, 2);
				grid->SetCellEditor(i, 2, new wxGridCellFloatEditor);
				break;
			}
		case ItemAttribute::BOOLEAN:
			{
				grid->SetCellValue(wxT("Boolean"), i, 1);
				grid->SetCellValue(*attr.getBoolean() ? wxT("1") : wxT(""), i, 2);
				grid->SetCellRenderer(i, 2, new wxGridCellBoolRenderer);
				grid->SetCellEditor(i, 2, new wxGridCellBoolEditor);
				break;
			}
		default:
			{
				grid->SetCellValue(wxT("Unknown"), i, 1);
				grid->SetCellBackgroundColour(*wxLIGHT_GREY, i, 1);
				grid->SetCellBackgroundColour(*wxLIGHT_GREY, i, 2);
				grid->SetReadOnly(i, 1, true);
				grid->SetReadOnly(i, 2, true);
				break;
			}
		}
		grid->SetCellEditor(i, 1, new wxGridCellChoiceEditor(types));
	}

	return grid;
}

void PropertiesWindow::OnResize(wxSizeEvent& evt)
{
	/*
	if (wxGrid* grid = (wxGrid*)currentPanel->FindWindowByName(wxT("AdvancedGrid")))
	{
		int tWidth = 0;
		for (int i = 0; i < 3; ++i)
			tWidth += grid->GetColumnWidth(i);

		int wWidth = grid->GetParent()->GetSize().GetWidth();
		
		grid->SetColumnWidth(2, wWidth - 100 - 80);
	}
	*/
}

void PropertiesWindow::OnNotebookPageChanged(wxNotebookEvent& evt)
{
	wxWindow* page = notebook->GetCurrentPage();

	// TODO: Save

	switch (page->GetId())
	{
		case ITEM_PROPERTIES_GENERAL_TAB:
		{
			//currentPanel = createGeneralPanel(page);
			break;
		}
		case ITEM_PROPERTIES_ADVANCED_TAB:
		{
			//currentPanel = createAttributesPanel(page);
			break;
		}
		default:
			break;
	}
}

void PropertiesWindow::saveGeneralPanel(wxWindow* panel)
{

}

void PropertiesWindow::saveContainerPanel(wxWindow* panel)
{

}

void PropertiesWindow::saveAttributesPanel(wxWindow* panel)
{

}

void PropertiesWindow::OnClickOK(wxCommandEvent&)
{
	EndModal(1);
}

void PropertiesWindow::OnClickCancel(wxCommandEvent&)
{
	EndModal(0);
}