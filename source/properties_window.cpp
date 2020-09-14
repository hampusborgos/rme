//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "properties_window.h"

#include "gui_ids.h"
#include "complexitem.h"
#include "container_properties_window.h"

#include <wx/grid.h>

BEGIN_EVENT_TABLE(PropertiesWindow, wxDialog)
	EVT_BUTTON(wxID_OK, PropertiesWindow::OnClickOK)
	EVT_BUTTON(wxID_CANCEL, PropertiesWindow::OnClickCancel)

	EVT_BUTTON(ITEM_PROPERTIES_ADD_ATTRIBUTE, PropertiesWindow::OnClickAddAttribute)
	EVT_BUTTON(ITEM_PROPERTIES_REMOVE_ATTRIBUTE, PropertiesWindow::OnClickRemoveAttribute)

	EVT_NOTEBOOK_PAGE_CHANGED(wxID_ANY, PropertiesWindow::OnNotebookPageChanged)

	EVT_GRID_CELL_CHANGED(PropertiesWindow::OnGridValueChanged)
END_EVENT_TABLE()

PropertiesWindow::PropertiesWindow(wxWindow* parent, const Map* map, const Tile* tile_parent, Item* item, wxPoint pos) :
	ObjectPropertiesWindowBase(parent, "Item Properties", map, tile_parent, item, pos),
	currentPanel(nullptr)
{
	ASSERT(edit_item);
	notebook = newd wxNotebook(this, wxID_ANY, wxDefaultPosition, wxSize(600, 300));

	notebook->AddPage(createGeneralPanel(notebook), "Simple", true);
	if(dynamic_cast<Container*>(item)) {
		notebook->AddPage(createContainerPanel(notebook), "Contents");
	}
	notebook->AddPage(createAttributesPanel(notebook), "Advanced");

	wxSizer* topSizer = newd wxBoxSizer(wxVERTICAL);
	topSizer->Add(notebook, wxSizerFlags(1).DoubleBorder());

	wxSizer* optSizer = newd wxBoxSizer(wxHORIZONTAL);
	optSizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(0).Center());
	optSizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(0).Center());
	topSizer->Add(optSizer, wxSizerFlags(0).Center().DoubleBorder());

	SetSizerAndFit(topSizer);
	Centre(wxBOTH);
}

PropertiesWindow::~PropertiesWindow()
{
	;
}

void PropertiesWindow::Update()
{
	Container* container = dynamic_cast<Container*>(edit_item);
	if(container) {
		for(uint32_t i = 0; i < container->getVolume(); ++i) {
			container_items[i]->setItem(container->getItem(i));
		}
	}
	wxDialog::Update();
}

wxWindow* PropertiesWindow::createGeneralPanel(wxWindow* parent)
{
	wxPanel* panel = newd wxPanel(parent, ITEM_PROPERTIES_GENERAL_TAB);
	wxFlexGridSizer* gridsizer = newd wxFlexGridSizer(2, 10, 10);
	gridsizer->AddGrowableCol(1);

	gridsizer->Add(newd wxStaticText(panel, wxID_ANY, "ID " + i2ws(edit_item->getID())));
	gridsizer->Add(newd wxStaticText(panel, wxID_ANY, "\"" + wxstr(edit_item->getName()) + "\""));

	gridsizer->Add(newd wxStaticText(panel, wxID_ANY, "Action ID"));
	wxSpinCtrl* action_id_field = newd wxSpinCtrl(panel, wxID_ANY, i2ws(edit_item->getActionID()), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getActionID());
	gridsizer->Add(action_id_field, wxSizerFlags(1).Expand());

	gridsizer->Add(newd wxStaticText(panel, wxID_ANY, "Unique ID"));
	wxSpinCtrl* unique_id_field = newd wxSpinCtrl(panel, wxID_ANY, i2ws(edit_item->getUniqueID()), wxDefaultPosition, wxSize(-1, 20), wxSP_ARROW_KEYS, 0, 0xFFFF, edit_item->getUniqueID());
	gridsizer->Add(unique_id_field, wxSizerFlags(1).Expand());

	panel->SetSizerAndFit(gridsizer);

	return panel;
}

wxWindow* PropertiesWindow::createContainerPanel(wxWindow* parent)
{
	Container* container = (Container*)edit_item;
	wxPanel* panel = newd wxPanel(parent, ITEM_PROPERTIES_CONTAINER_TAB);
	wxSizer* topSizer = newd wxBoxSizer(wxVERTICAL);

	wxSizer* gridSizer = newd wxGridSizer(6, 5, 5);

	bool use_large_sprites = g_settings.getBoolean(Config::USE_LARGE_CONTAINER_ICONS);
	for(uint32_t i = 0; i < container->getVolume(); ++i) {
		Item* item = container->getItem(i);
		ContainerItemButton* containerItemButton = newd ContainerItemButton(panel, use_large_sprites, i, edit_map, item);

		container_items.push_back(containerItemButton);
		gridSizer->Add(containerItemButton, wxSizerFlags(0));
	}

	topSizer->Add(gridSizer, wxSizerFlags(1).Expand());

	/*
	wxSizer* optSizer = newd wxBoxSizer(wxHORIZONTAL);
	optSizer->Add(newd wxButton(panel, ITEM_PROPERTIES_ADD_ATTRIBUTE, "Add Item"), wxSizerFlags(0).Center());
	// optSizer->Add(newd wxButton(panel, ITEM_PROPERTIES_REMOVE_ATTRIBUTE, "Remove Attribute"), wxSizerFlags(0).Center());
	topSizer->Add(optSizer, wxSizerFlags(0).Center().DoubleBorder());
	*/

	panel->SetSizer(topSizer);
	return panel;
}

wxWindow* PropertiesWindow::createAttributesPanel(wxWindow* parent)
{
	wxPanel* panel = newd wxPanel(parent, wxID_ANY);
	wxSizer* topSizer = newd wxBoxSizer(wxVERTICAL);

	attributesGrid = newd wxGrid(panel, ITEM_PROPERTIES_ADVANCED_TAB, wxDefaultPosition, wxSize(-1, 160));
	topSizer->Add(attributesGrid, wxSizerFlags(1).Expand());

	wxFont time_font(*wxSWISS_FONT);
	attributesGrid->SetDefaultCellFont(time_font);
	attributesGrid->CreateGrid(0, 3);
	attributesGrid->DisableDragRowSize();
	attributesGrid->DisableDragColSize();
	attributesGrid->SetSelectionMode(wxGrid::wxGridSelectRows);
	attributesGrid->SetRowLabelSize(0);
	//log->SetColLabelSize(0);
	//log->EnableGridLines(false);
	attributesGrid->EnableEditing(true);

	attributesGrid->SetColLabelValue(0, "Key");
	attributesGrid->SetColSize(0, 100);
	attributesGrid->SetColLabelValue(1, "Type");
	attributesGrid->SetColSize(1, 80);
	attributesGrid->SetColLabelValue(2, "Value");
	attributesGrid->SetColSize(2, 410);

	// contents
	ItemAttributeMap attrs = edit_item->getAttributes();
	attributesGrid->AppendRows(attrs.size());
	int i = 0;
	for(ItemAttributeMap::iterator aiter = attrs.begin(); aiter != attrs.end(); ++aiter, ++i)
		SetGridValue(attributesGrid, i, aiter->first, aiter->second);

	wxSizer* optSizer = newd wxBoxSizer(wxHORIZONTAL);
	optSizer->Add(newd wxButton(panel, ITEM_PROPERTIES_ADD_ATTRIBUTE, "Add Attribute"), wxSizerFlags(0).Center());
	optSizer->Add(newd wxButton(panel, ITEM_PROPERTIES_REMOVE_ATTRIBUTE, "Remove Attribute"), wxSizerFlags(0).Center());
	topSizer->Add(optSizer, wxSizerFlags(0).Center().DoubleBorder());

	panel->SetSizer(topSizer);

	return panel;
}

void PropertiesWindow::SetGridValue(wxGrid* grid, int rowIndex, std::string label, const ItemAttribute& attr)
{
	wxArrayString types;
	types.Add("Number");
	types.Add("Float");
	types.Add("Boolean");
	types.Add("String");

	grid->SetCellValue(rowIndex, 0, label);
	switch (attr.type) {
		case ItemAttribute::STRING: {
			grid->SetCellValue(rowIndex, 1, "String");
			grid->SetCellValue(rowIndex, 2, wxstr(*attr.getString()));
			break;
		}
		case ItemAttribute::INTEGER: {
			grid->SetCellValue(rowIndex, 1, "Number");
			grid->SetCellValue(rowIndex, 2, i2ws(*attr.getInteger()));
			grid->SetCellEditor(rowIndex, 2, new wxGridCellNumberEditor);
			break;
		}
		case ItemAttribute::DOUBLE:
		case ItemAttribute::FLOAT: {
			grid->SetCellValue(rowIndex, 1, "Float");
			wxString f;
			f << *attr.getFloat();
			grid->SetCellValue(rowIndex, 2, f);
			grid->SetCellEditor(rowIndex, 2, new wxGridCellFloatEditor);
			break;
		}
		case ItemAttribute::BOOLEAN: {
			grid->SetCellValue(rowIndex, 1, "Boolean");
			grid->SetCellValue(rowIndex, 2, *attr.getBoolean() ? "1" : "");
			grid->SetCellRenderer(rowIndex, 2, new wxGridCellBoolRenderer);
			grid->SetCellEditor(rowIndex, 2, new wxGridCellBoolEditor);
			break;
		}
		default: {
			grid->SetCellValue(rowIndex, 1, "Unknown");
			grid->SetCellBackgroundColour(rowIndex, 1, *wxLIGHT_GREY);
			grid->SetCellBackgroundColour(rowIndex, 2, *wxLIGHT_GREY);
			grid->SetReadOnly(rowIndex, 1, true);
			grid->SetReadOnly(rowIndex, 2, true);
			break;
		}
	}
	grid->SetCellEditor(rowIndex, 1, new wxGridCellChoiceEditor(types));
}

void PropertiesWindow::OnResize(wxSizeEvent& evt)
{
	/*
	if(wxGrid* grid = (wxGrid*)currentPanel->FindWindowByName("AdvancedGrid")) {
		int tWidth = 0;
		for(int i = 0; i < 3; ++i)
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

	switch (page->GetId()) {
		case ITEM_PROPERTIES_GENERAL_TAB: {
			//currentPanel = createGeneralPanel(page);
			break;
		}
		case ITEM_PROPERTIES_ADVANCED_TAB: {
			//currentPanel = createAttributesPanel(page);
			break;
		}
		default:
			break;
	}
}

void PropertiesWindow::saveGeneralPanel()
{
	////
}

void PropertiesWindow::saveContainerPanel()
{
	////
}

void PropertiesWindow::saveAttributesPanel()
{
	edit_item->clearAllAttributes();
	for(int32_t rowIndex = 0; rowIndex < attributesGrid->GetNumberRows(); ++rowIndex) {
		ItemAttribute attr;
		wxString type = attributesGrid->GetCellValue(rowIndex, 1);
		if(type == "String") {
			attr.set(nstr(attributesGrid->GetCellValue(rowIndex, 2)));
		} else if(type == "Float") {
			double value;
			if(attributesGrid->GetCellValue(rowIndex, 2).ToDouble(&value)) {
				attr.set(value);
			}
		} else if(type == "Number") {
			long value;
			if(attributesGrid->GetCellValue(rowIndex, 2).ToLong(&value)) {
				attr.set(static_cast<int32_t>(value));
			}
		} else if(type == "Boolean") {
			attr.set(attributesGrid->GetCellValue(rowIndex, 2) == "1");
		} else {
			continue;
		}
		edit_item->setAttribute(nstr(attributesGrid->GetCellValue(rowIndex, 0)), attr);
	}
}

void PropertiesWindow::OnGridValueChanged(wxGridEvent& event)
{
	if(event.GetCol() == 1) {
		wxString newType = attributesGrid->GetCellValue(event.GetRow(), 1);
		if(newType == event.GetString()) {
			return;
		}

		ItemAttribute attr;
		if(newType == "String") {
			attr.set("");
		} else if(newType == "Float") {
			attr.set(0.0f);
		} else if(newType == "Number") {
			attr.set(0);
		} else if(newType == "Boolean") {
			attr.set(false);
		}
		SetGridValue(attributesGrid, event.GetRow(), nstr(attributesGrid->GetCellValue(event.GetRow(), 0)), attr);
	}
}

void PropertiesWindow::OnClickOK(wxCommandEvent&)
{
	saveAttributesPanel();
	EndModal(1);
}

void PropertiesWindow::OnClickAddAttribute(wxCommandEvent&)
{
	attributesGrid->AppendRows(1);
	ItemAttribute attr(0);
	SetGridValue(attributesGrid, attributesGrid->GetNumberRows() - 1, "", attr);
}

void PropertiesWindow::OnClickRemoveAttribute(wxCommandEvent&)
{
	wxArrayInt rowIndexes = attributesGrid->GetSelectedRows();
	if(rowIndexes.Count() != 1)
		return;

	int rowIndex = rowIndexes[0];
	attributesGrid->DeleteRows(rowIndex, 1);
}

void PropertiesWindow::OnClickCancel(wxCommandEvent&)
{
	EndModal(0);
}
