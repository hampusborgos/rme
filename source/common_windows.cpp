//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "materials.h"
#include "brush.h"
#include "editor.h"

#include "items.h"
#include "map.h"
#include "item.h"
#include "complexitem.h"
#include "raw_brush.h"

#include "palette_window.h"
#include "gui.h"
#include "application.h"
#include "common_windows.h"
#include "positionctrl.h"

#include <boost/lexical_cast.hpp>

#ifdef _MSC_VER
	#pragma warning(disable:4018) // signed/unsigned mismatch
#endif

// ============================================================================
// Map Properties Window

BEGIN_EVENT_TABLE(MapPropertiesWindow, wxDialog)
	EVT_CHOICE(MAP_PROPERTIES_VERSION, MapPropertiesWindow::OnChangeVersion)
	EVT_BUTTON(wxID_OK, MapPropertiesWindow::OnClickOK)
	EVT_BUTTON(wxID_CANCEL, MapPropertiesWindow::OnClickCancel)
END_EVENT_TABLE()

MapPropertiesWindow::MapPropertiesWindow(wxWindow* parent, MapTab* view, Editor& editor) :
	wxDialog(parent, wxID_ANY, wxT("Map Properties"), wxDefaultPosition, wxSize(300, 200), wxRESIZE_BORDER | wxCAPTION),
	view(view),
	editor(editor)
{
	// Setup data variabels
	Map& map = editor.map;

	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer* grid_sizer = newd wxFlexGridSizer(2, 10, 10);
	grid_sizer->AddGrowableCol(1);

	// Description
	grid_sizer->Add(newd wxStaticText(this, wxID_ANY, wxT("Map Description")));
	description_ctrl = newd wxTextCtrl(this, wxID_ANY, wxstr(map.getMapDescription()), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE);
	grid_sizer->Add(description_ctrl, wxSizerFlags(1).Expand());

	// Map version
	grid_sizer->Add(newd wxStaticText(this, wxID_ANY, wxT("Map Version")));
	version_choice = newd wxChoice(this, MAP_PROPERTIES_VERSION);
	version_choice->Append(wxT("OTServ 0.5.0"));
	version_choice->Append(wxT("OTServ 0.6.0"));
	version_choice->Append(wxT("OTServ 0.6.1"));
	version_choice->Append(wxT("OTServ 0.7.0 (revscriptsys)"));

	switch(map.getVersion().otbm) {
		case MAP_OTBM_1:
			version_choice->SetSelection(0);
			break;
		case MAP_OTBM_2:
			version_choice->SetSelection(1);
			break;
		case MAP_OTBM_3:
			version_choice->SetSelection(2);
			break;
		case MAP_OTBM_4:
			version_choice->SetSelection(3);
			break;
		default:
			version_choice->SetSelection(0);
	}
	
	grid_sizer->Add(version_choice, wxSizerFlags(1).Expand());

	// Version
	grid_sizer->Add(newd wxStaticText(this, wxID_ANY, wxT("Client Version")));
	protocol_choice = newd wxChoice(this, wxID_ANY);
	
	protocol_choice->SetStringSelection(wxstr(gui.GetCurrentVersion().getName()));

	grid_sizer->Add(protocol_choice, wxSizerFlags(1).Expand());

	// Dimensions
	grid_sizer->Add(newd wxStaticText(this, wxID_ANY, wxT("Map Dimensions")));
	{
		wxSizer* subsizer = newd wxBoxSizer(wxHORIZONTAL);
		subsizer->Add(
			width_spin = 
				newd wxSpinCtrl(this, wxID_ANY, wxstr(i2s(map.getWidth())),
					wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 256, 65000), wxSizerFlags(1).Expand()
			);
		subsizer->Add(
			height_spin = 
				newd wxSpinCtrl(this, wxID_ANY, wxstr(i2s(map.getHeight())),
				wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 256, 65000), wxSizerFlags(1).Expand()
			);
		grid_sizer->Add(subsizer, 1, wxEXPAND);
	}

	// External files
	grid_sizer->Add(
		newd wxStaticText(this, wxID_ANY, wxT("External Housefile"))
		);

	grid_sizer->Add(
		house_filename_ctrl = 
			newd wxTextCtrl(this, wxID_ANY, wxstr(map.getHouseFilename())), 1, wxEXPAND
		);

	grid_sizer->Add(
		newd wxStaticText(this, wxID_ANY, wxT("External Spawnfile"))
		);

	grid_sizer->Add(
		spawn_filename_ctrl =
			newd wxTextCtrl(this, wxID_ANY, wxstr(map.getSpawnFilename())), 1, wxEXPAND
		);

	topsizer->Add(grid_sizer, wxSizerFlags(1).Expand().Border(wxALL, 20));

	wxSizer* subsizer = newd wxBoxSizer(wxHORIZONTAL);
	subsizer->Add(newd wxButton(this, wxID_OK, wxT("OK")), wxSizerFlags(1).Center());
	subsizer->Add(newd wxButton(this, wxID_CANCEL, wxT("Cancel")), wxSizerFlags(1).Center());
	topsizer->Add(subsizer, wxSizerFlags(0).Center().Border(wxLEFT | wxRIGHT | wxBOTTOM, 20));

	SetSizerAndFit(topsizer);
	
	UpdateProtocolList();

	ClientVersion* current_version = ClientVersion::get(map.getVersion().client);
	protocol_choice->SetStringSelection(wxstr(current_version->getName()));
}

void MapPropertiesWindow::UpdateProtocolList()
{
	wxString ver = version_choice->GetStringSelection();
	wxString client = protocol_choice->GetStringSelection();

	protocol_choice->Clear();
	
	ClientVersionList versions;
	if(settings.getInteger(Config::USE_OTBM_4_FOR_ALL_MAPS)) {
		versions = ClientVersion::getAllVisible();
	} else {
		MapVersionID map_version = MAP_OTBM_1;
		if(ver.Contains(wxT("0.5.0")))
			map_version = MAP_OTBM_1;
		else if(ver.Contains(wxT("0.6.0")))
			map_version = MAP_OTBM_2;
		else if(ver.Contains(wxT("0.6.1")))
			map_version = MAP_OTBM_3;
		else if(ver.Contains(wxT("0.7.0")))
			map_version = MAP_OTBM_4;

		ClientVersionList protocols = ClientVersion::getAllForOTBMVersion(map_version);
		for(ClientVersionList::const_iterator p = protocols.begin(); p != protocols.end(); ++p)
			protocol_choice->Append(wxstr((*p)->getName()));
	}
	protocol_choice->SetSelection(0);
	protocol_choice->SetStringSelection(client);
}

void MapPropertiesWindow::OnChangeVersion(wxCommandEvent&)
{
	UpdateProtocolList();
}

struct MapConversionContext
{
	struct CreatureInfo
	{
		std::string name;
		bool is_npc;
		Outfit outfit;
	};
	typedef std::map<std::string, CreatureInfo> CreatureMap;
	CreatureMap creature_types;

	void operator()(Map& map, Tile* tile, long long done)
	{
		if(tile->creature) {
			CreatureMap::iterator f = creature_types.find(tile->creature->getName());
			if(f == creature_types.end()) {
				CreatureInfo info = {
					tile->creature->getName(),
					tile->creature->isNpc(),
					tile->creature->getLookType()
				};
				creature_types[tile->creature->getName()] = info;
			}
		}
	}
};

void MapPropertiesWindow::OnClickOK(wxCommandEvent& WXUNUSED(event)) 
{
	Map& map = editor.map;

	MapVersion old_ver = map.getVersion();
	MapVersion new_ver;
	
	wxString ver = version_choice->GetStringSelection();

	new_ver.client = ClientVersion::get(nstr(protocol_choice->GetStringSelection()))->getID();
	if(ver.Contains(wxT("0.5.0"))) {
		new_ver.otbm = MAP_OTBM_1;
	} else if(ver.Contains(wxT("0.6.0"))) {
		new_ver.otbm = MAP_OTBM_2;
	} else if(ver.Contains(wxT("0.6.1"))) {
		new_ver.otbm = MAP_OTBM_3;
	} else if(ver.Contains(wxT("0.7.0"))) {
		new_ver.otbm = MAP_OTBM_4;
	}

	if(new_ver.client != old_ver.client) {
		if(gui.GetOpenMapCount() > 1) {
			gui.PopupDialog(this, wxT("Error"), 
				wxT("You can not change editor version with multiple maps open"), wxOK);
			return;
		}
		wxString error;
		wxArrayString warnings;

		// Switch version
		gui.GetCurrentEditor()->selection.clear();
		gui.GetCurrentEditor()->actionQueue->clear();	

		if(new_ver.client < old_ver.client) {
			int ret = gui.PopupDialog(this, wxT("Notice"),
				wxT("Converting to a previous version may have serious side-effects, are you sure you want to do this?"), wxYES | wxNO);
			if(ret != wxID_YES) {
				return;
			}
			UnnamedRenderingLock();

			// Remember all creatures types on the map
			MapConversionContext conversion_context;
			foreach_TileOnMap(map, conversion_context);

			// Perform the conversion			
			map.convert(new_ver, true);

			// Load the new version
			if(!gui.LoadVersion(new_ver.client, error, warnings)) {
				gui.ListDialog(this, wxT("Warnings"), warnings);
				gui.PopupDialog(this, wxT("Map Loader Error"), error, wxOK);
				gui.PopupDialog(this, wxT("Conversion Error"), wxT("Could not convert map. The map will now be closed."), wxOK);

				EndModal(0);
				return;
			}

			// Remove all creatures that were present are present in the new version
			for(MapConversionContext::CreatureMap::iterator cs = conversion_context.creature_types.begin(); cs != conversion_context.creature_types.end();) {
				if(creature_db[cs->first])
					cs = conversion_context.creature_types.erase(cs);
				else
					++cs;
			}

			if(conversion_context.creature_types.size() > 0) {
				int add = gui.PopupDialog(this, wxT("Unrecognized creatures"), wxT("There were creatures on the old version that are not present in this and were on the map, do you want to add them to this version as well?"), wxYES | wxNO);
				if(add == wxID_YES) {
					for(MapConversionContext::CreatureMap::iterator cs = conversion_context.creature_types.begin(); cs != conversion_context.creature_types.end(); ++cs) {
						MapConversionContext::CreatureInfo info = cs->second;
						creature_db.addCreatureType(info.name, info.is_npc, info.outfit);
					}
				}
			}

			map.cleanInvalidTiles(true);
		} else  {
			UnnamedRenderingLock();
			if(!gui.LoadVersion(new_ver.client, error, warnings)) {
				gui.ListDialog(this, wxT("Warnings"), warnings);
				gui.PopupDialog(this, wxT("Map Loader Error"), error, wxOK);
				gui.PopupDialog(this, wxT("Conversion Error"), wxT("Could not convert map. The map will now be closed."), wxOK);

				EndModal(0);
				return;
			}
			map.convert(new_ver, true);
		}
	} else {
		map.convert(new_ver, true);
	}

	map.setMapDescription(nstr(description_ctrl->GetValue()));
	map.setHouseFilename(nstr(house_filename_ctrl->GetValue()));
	map.setSpawnFilename(nstr(spawn_filename_ctrl->GetValue()));

	// Only resize if we have to
	int new_map_width = width_spin->GetValue();
	int new_map_height = height_spin->GetValue();
	if(new_map_width != map.getWidth() || new_map_height != map.getHeight()) {
		map.setWidth(new_map_width);
		map.setHeight(new_map_height);
		gui.FitViewToMap(view);
	}
	gui.RefreshPalettes();

	EndModal(1);
}

void MapPropertiesWindow::OnClickCancel(wxCommandEvent& WXUNUSED(event)) 
{
	// Just close this window
	EndModal(1);
}

MapPropertiesWindow::~MapPropertiesWindow() 
{
	////
}

// ============================================================================
// Map Import Window

BEGIN_EVENT_TABLE(ImportMapWindow, wxDialog)
	EVT_BUTTON(MAP_WINDOW_FILE_BUTTON, ImportMapWindow::OnClickBrowse)
	EVT_BUTTON(wxID_OK, ImportMapWindow::OnClickOK)
	EVT_BUTTON(wxID_CANCEL, ImportMapWindow::OnClickCancel)
END_EVENT_TABLE()

ImportMapWindow::ImportMapWindow(wxWindow* parent, Editor& editor) :
	wxDialog(parent, wxID_ANY, wxT("Import map"), wxDefaultPosition, wxSize(280,227)),
	editor(editor)
{
	// Setup data variabels
	file_to_import = wxT("");
	x_offset = wxT("0");
	y_offset = wxT("0");

	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* tmpsizer;

	// File
	tmpsizer = newd wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Map file"));
	tmpsizer->Add(file_text_field = newd wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(185,20), 0, wxTextValidator(wxFILTER_ASCII, &file_to_import)));
	tmpsizer->Add(newd wxButton(this, MAP_WINDOW_FILE_BUTTON, wxT("Browse")));
	sizer->Add(tmpsizer);

	// Import offset
	tmpsizer = newd wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Import offset"));
	wxStaticText* width_field = newd wxStaticText(this, wxID_ANY, wxT("X offset"));
	tmpsizer->Add(width_field);
	wxTextCtrl* x_size_field = newd wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(85,20), 0, wxTextValidator(wxFILTER_NUMERIC, &x_offset));
	tmpsizer->Add(x_size_field);

	tmpsizer->Add(10, 20);

	wxStaticText* height_field = newd wxStaticText(this, wxID_ANY, wxT("Y offset"));
	tmpsizer->Add(height_field);
	wxTextCtrl* y_size_field = newd wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(85,20), 0, wxTextValidator(wxFILTER_NUMERIC, &y_offset));
	tmpsizer->Add(y_size_field);
	sizer->Add(tmpsizer);

	// Import options
	wxArrayString house_choices;
	house_choices.Add(wxT("Smart Merge"));
	house_choices.Add(wxT("Insert"));
	house_choices.Add(wxT("Merge"));
	house_choices.Add(wxT("Don't import"));

	// House options
	tmpsizer = newd wxStaticBoxSizer(wxHORIZONTAL, this, wxT("House import behaviour"));
	house_options = newd wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(260,20), house_choices);
	house_options->SetSelection(0);
	tmpsizer->Add(house_options);
	sizer->Add(tmpsizer);

	// Import options
	wxArrayString spawn_choices;
	spawn_choices.Add(wxT("Merge"));
	spawn_choices.Add(wxT("Don't import"));

	// Spawn options
	tmpsizer = newd wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Spawn import behaviour"));
	spawn_options = newd wxChoice(this, wxID_ANY, wxDefaultPosition, wxSize(260,20), spawn_choices);
	spawn_options->SetSelection(0);
	tmpsizer->Add(spawn_options);
	sizer->Add(tmpsizer);

	// OK/Cancel buttons
	tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	tmpsizer->Add(newd wxButton(this, wxID_OK, wxT("OK")), wxSizerFlags(1).Center());
	tmpsizer->Add(newd wxButton(this, wxID_CANCEL, wxT("Cancel")), wxSizerFlags(1).Center());
	sizer->Add(tmpsizer, wxSizerFlags(1).Center());

	SetSizerAndFit(sizer);
}

ImportMapWindow::~ImportMapWindow() 
{
	////
}

void ImportMapWindow::OnClickBrowse(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog file(this, wxT("Import..."), wxT(""), wxT(""), wxT("*.otbm"), wxFD_OPEN | wxFD_FILE_MUST_EXIST);
	int ok = file.ShowModal();

	if(ok == wxID_OK) {
		file_text_field->ChangeValue(file.GetPath());
	} else {
		return;
	}
}

void ImportMapWindow::OnClickOK(wxCommandEvent& WXUNUSED(event)) 
{
	if(Validate() && TransferDataFromWindow()) {
		wxFileName fn = file_to_import;
		if(!fn.FileExists()) {
			gui.PopupDialog(this, wxT("Error"), wxT("The specified map file doesn't exist"), wxOK);
		}

		long import_x_offset;
		x_offset.ToLong(&import_x_offset);
		long import_y_offset;
		y_offset.ToLong(&import_y_offset);

		if(import_x_offset < -65000 || import_x_offset > 65000) {
			gui.PopupDialog(this, wxT("Error"), wxT("The specified x offset needs to be in the range -65000 to 65000"), wxOK);
			return;
		}
		if(import_y_offset < -65000 || import_y_offset > 65000) {
			gui.PopupDialog(this, wxT("Error"), wxT("The specified y offset needs to be in the range -65000 to 65000"), wxOK);
			return;
		}

		ImportType spawn_import_type = IMPORT_DONT;
		ImportType house_import_type = IMPORT_DONT;

		switch(spawn_options->GetSelection()) {
			case 0: spawn_import_type = IMPORT_MERGE; break;
			case 1: spawn_import_type = IMPORT_DONT; break;
		}

		switch(house_options->GetSelection()) {
			case 0: house_import_type = IMPORT_SMART_MERGE; break;
			case 1: house_import_type = IMPORT_MERGE; break;
			case 2: house_import_type = IMPORT_INSERT; break;
			case 3: house_import_type = IMPORT_DONT; break;
		}

		EndModal(1);

		editor.importMap(fn, import_x_offset, import_y_offset, house_import_type, spawn_import_type);
	}
}

void ImportMapWindow::OnClickCancel(wxCommandEvent& WXUNUSED(event)) 
{
	// Just close this window
	EndModal(0);
}


// ============================================================================
// Export Minimap window

BEGIN_EVENT_TABLE(ExportMiniMapWindow, wxDialog)
	EVT_BUTTON(MAP_WINDOW_FILE_BUTTON, ExportMiniMapWindow::OnClickBrowse)
	EVT_BUTTON(wxID_OK, ExportMiniMapWindow::OnClickOK)
	EVT_BUTTON(wxID_CANCEL, ExportMiniMapWindow::OnClickCancel)
	EVT_CHOICE(wxID_ANY, ExportMiniMapWindow::OnExportTypeChange)
END_EVENT_TABLE()

ExportMiniMapWindow::ExportMiniMapWindow(wxWindow* parent, Editor& editor) :
	wxDialog(parent, wxID_ANY, wxT("Export minimap"), wxDefaultPosition, wxSize(260,150)),
	editor(editor)
{
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* tmpsizer;

	// File
	tmpsizer = newd wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Map file"));
	tmpsizer->Add(file_text_field = newd wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxDefaultSize), 5, wxEXPAND);
	tmpsizer->Add(newd wxButton(this, MAP_WINDOW_FILE_BUTTON, wxT("Browse")), 2);
	sizer->Add(tmpsizer, 1, wxEXPAND);

	// Import options
	wxArrayString choices;
	choices.Add(wxT("All floors"));
	choices.Add(wxT("Ground floor"));
	choices.Add(wxT("Specific floor"));

	// House options
	tmpsizer = newd wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Floor options"));
	floor_options = newd wxChoice(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, choices);
	floor_number = newd wxSpinCtrl(this, wxID_ANY, wxT("7"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 15, 7);
	floor_number->Enable(false);
	floor_options->SetSelection(0);
	tmpsizer->Add(floor_options, 5, wxEXPAND);
	tmpsizer->Add(floor_number, 2);
	sizer->Add(tmpsizer, 1, wxEXPAND);

	// OK/Cancel buttons
	tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	tmpsizer->Add(newd wxButton(this, wxID_OK, wxT("OK")), wxSizerFlags(1).Center());
	tmpsizer->Add(newd wxButton(this, wxID_CANCEL, wxT("Cancel")), wxSizerFlags(1).Center());
	sizer->Add(tmpsizer, 0, wxCENTER);

	SetSizerAndFit(sizer);
}

ExportMiniMapWindow::~ExportMiniMapWindow() 
{
	////
}

void ExportMiniMapWindow::OnExportTypeChange(wxCommandEvent& event) 
{
	if(event.GetSelection() == 2) {
		floor_number->Enable(true);
	} else {
		floor_number->Enable(false);
	}
}

void ExportMiniMapWindow::OnClickBrowse(wxCommandEvent& WXUNUSED(event)) 
{
	wxFileDialog file(this, wxT("Export..."), wxT(""), wxT(""), wxT("*.bmp"), wxFD_SAVE);
	int ok = file.ShowModal();

	if(ok == wxID_OK) {
		FileName fn(file.GetPath());
		fn.SetName(fn.GetName() + wxT("_%d"));
		file_text_field->ChangeValue(fn.GetFullPath());
	} else {
		return;
	}
}

void ExportMiniMapWindow::OnClickOK(wxCommandEvent& WXUNUSED(event)) 
{
	std::string filename = nstr(file_text_field->GetValue());
	size_t d_pos = filename.find("%d");
	if(d_pos == std::string::npos) {
		gui.PopupDialog(this, wxT("Error"), wxT("You need to put a %d in the filename (this will be replaced by the floor number)"), wxYES | wxNO);
		return;
	}
	size_t i_pos = filename.find_first_of("*?:/\"<>|");
	if(i_pos != std::string::npos) {
#ifdef __WINDOWS__
		if(!(filename[i_pos] == ':' && i_pos == 1))
#else
		if(filename[i_pos] != '/')
#endif
		{
			gui.PopupDialog(this, wxT("Error"), wxT("File name contains invalid characters."), wxOK);
			return;
		}
	}
#ifdef __WINDOWS__
	if(filename.size() <= 3 || filename[1] != ':' || filename[2] != '\\') 
#else
	if(filename[0] != '/') 
#endif
	{
		gui.PopupDialog(this, wxT("Error"), wxT("Output filename must be absolute."), wxOK);
		return;
	}

	std::string pre_sep = filename.substr(0, d_pos);
	std::string post_sep= filename.substr(d_pos+2);

	gui.CreateLoadBar(wxT("Exporting minimap"));
	try 
	{
		switch(floor_options->GetSelection()) {
			case 0: { // All floors
				for(int floor = 0; floor < 16; ++floor) {
					gui.SetLoadScale(int(floor*(100.f/16.f)), int((floor+1)*(100.f/16.f)));
					FileName fn = wxstr(pre_sep + i2s(floor) + post_sep);
					editor.exportMiniMap(fn, floor, true);
				}
				break;
			}
			case 1: { // Ground floor
				FileName fn = wxstr(pre_sep + "ground" + post_sep);
				editor.exportMiniMap(fn, 7, true);
				gui.DestroyLoadBar();
				break;
			}
			case 2: { // Specific floors
				int floor = floor_number->GetValue();
				FileName fn = wxstr(pre_sep + i2s(floor) + post_sep);
				editor.exportMiniMap(fn, floor, true);
				break;
			}
		}
	}
	catch(std::bad_alloc&) 
	{
		gui.PopupDialog(wxT("Error"), wxT("There is not enough memory available to complete the operation."), wxOK);
	}
	gui.DestroyLoadBar();
	EndModal(1);
}

void ExportMiniMapWindow::OnClickCancel(wxCommandEvent& WXUNUSED(event)) 
{
	// Just close this window
	EndModal(0);
}

// ============================================================================
// Numkey forwarding text control

BEGIN_EVENT_TABLE(KeyForwardingTextCtrl, wxTextCtrl)
	EVT_KEY_DOWN(KeyForwardingTextCtrl::OnKeyDown)
END_EVENT_TABLE()

void KeyForwardingTextCtrl::OnKeyDown(wxKeyEvent& event) 
{
	if(event.GetKeyCode() == WXK_UP || event.GetKeyCode() == WXK_DOWN || 
		event.GetKeyCode() == WXK_PAGEDOWN || event.GetKeyCode() == WXK_PAGEUP) {
		GetParent()->GetEventHandler()->AddPendingEvent(event);
	} else {
		event.Skip();
	}
}

// ============================================================================
// Find Item Dialog (Jump to item)

BEGIN_EVENT_TABLE(FindDialog, wxDialog)
	EVT_TIMER(wxID_ANY, FindDialog::OnTextIdle)
	EVT_TEXT(JUMP_DIALOG_TEXT, FindDialog::OnTextChange)
	EVT_KEY_DOWN(FindDialog::OnKeyDown)
	EVT_TEXT_ENTER(JUMP_DIALOG_TEXT, FindDialog::OnClickOK)
	EVT_LISTBOX_DCLICK(JUMP_DIALOG_LIST, FindDialog::OnClickList)
	EVT_BUTTON(wxID_OK, FindDialog::OnClickOK)
	EVT_BUTTON(wxID_CANCEL, FindDialog::OnClickCancel)
END_EVENT_TABLE()

FindDialog::FindDialog(wxWindow* parent, wxString title) :
	wxDialog(gui.root, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX),
	idle_input_timer(this),
	result_brush(nullptr),
	result_id(0)
{
	sizer = newd wxBoxSizer(wxVERTICAL);

	search_field = newd KeyForwardingTextCtrl(this, JUMP_DIALOG_TEXT, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	search_field->SetFocus();
	sizer->Add(search_field, 0, wxEXPAND);

	item_list = newd FindDialogListBox(this, JUMP_DIALOG_LIST);
	item_list->SetMinSize(wxSize(470, 400));
	sizer->Add(item_list, wxSizerFlags(1).Expand().Border());

	wxSizer* stdsizer = newd wxBoxSizer(wxHORIZONTAL);
	stdsizer->Add(newd wxButton(this, wxID_OK, wxT("OK")), wxSizerFlags(1).Center());
	stdsizer->Add(newd wxButton(this, wxID_CANCEL, wxT("Cancel")), wxSizerFlags(1).Center());
	sizer->Add(stdsizer, wxSizerFlags(0).Center().Border());

	SetSizerAndFit(sizer);
	// We can't call it here since it calls an abstract function, call in child constructors instead.
	// RefreshContents();
}

FindDialog::~FindDialog() 
{
	////
}

void FindDialog::OnKeyDown(wxKeyEvent& event) 
{
	int w, h;
	item_list->GetSize(&w, &h);
	size_t amount = 1; 

	switch(event.GetKeyCode()) {
		case WXK_PAGEUP:
			amount = h / 32 + 1;
		case WXK_UP: {
			if(item_list->GetItemCount() > 0) {
				ssize_t n = item_list->GetSelection();
				if(n == wxNOT_FOUND) 
					n = 0;
				else if(n != amount && n - amount < n) // latter is needed for unsigned overflow
					n -= amount;
				else
					n = 0;
				item_list->SetSelection(n);
			}
			break;
		}

		case WXK_PAGEDOWN:
			amount = h / 32 + 1;
		case WXK_DOWN: {
			if(item_list->GetItemCount() > 0) {
				ssize_t n = item_list->GetSelection();
				size_t itemcount = item_list->GetItemCount();
				if(n == wxNOT_FOUND)
					n = 0;
				else if(static_cast<uint32_t>(n) < itemcount - amount && itemcount - amount < itemcount)
					n += amount;
				else
					n = item_list->GetItemCount() - 1;

				item_list->SetSelection(n);
			}
			break;
		}
		default:
			event.Skip();
			break;
	}
}

void FindDialog::OnTextIdle(wxTimerEvent& WXUNUSED(event)) 
{
	RefreshContents();
}

void FindDialog::OnTextChange(wxCommandEvent& WXUNUSED(event)) 
{
	idle_input_timer.Start(800, true);
}

void FindDialog::OnClickList(wxCommandEvent& event) 
{
	OnClickListInternal(event);
}

void FindDialog::OnClickOK(wxCommandEvent& WXUNUSED(event)) 
{
	// This is to get virtual callback
	OnClickOKInternal();
}

void FindDialog::OnClickCancel(wxCommandEvent& WXUNUSED(event)) 
{
	EndModal(0);
}

void FindDialog::RefreshContents() 
{
	// This is to get virtual callback
	RefreshContentsInternal();
}

// ============================================================================
// Find Item Dialog (Jump to item)

FindItemDialog::FindItemDialog(wxWindow* parent, wxString title) : FindDialog(parent, title)
{
	// Labels
	sizer->Insert(1, newd wxStaticText(this, wxID_ANY, wxT("Action ID")), 0, wxEXPAND);
	action_field = newd KeyForwardingTextCtrl(this, JUMP_DIALOG_TEXT, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	sizer->Insert(2, action_field, 0, wxEXPAND);

	sizer->Insert(3, newd wxStaticText(this, wxID_ANY, wxT("Unique ID")), 0, wxEXPAND);
	unique_field = newd KeyForwardingTextCtrl(this, JUMP_DIALOG_TEXT, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	sizer->Insert(4, unique_field, 0, wxEXPAND);

	actionid = 0;
	uniqueid = 0;

	extra_condition = nullptr;
	RefreshContents();
}

FindItemDialog::~FindItemDialog() 
{
	////
}

void FindItemDialog::setCondition(bool condition(const ItemType&))
{
	extra_condition = condition;
}

void FindItemDialog::OnClickListInternal(wxCommandEvent& event)
{
	RAWBrush* brush = dynamic_cast<RAWBrush*>(item_list->GetSelectedBrush());
	if(brush)
	{
		result_brush = brush;
		result_id = brush->getItemID();
		EndModal(result_id);
	}
}

void FindItemDialog::OnClickOKInternal()
{
	// This is kind of stupid as it would fail unless the "Please enter a search string" wasn't there
	if(item_list->GetItemCount() > 0) {
		if(item_list->GetSelection() == wxNOT_FOUND) {
			item_list->SetSelection(0);
		}

		RAWBrush* brush = dynamic_cast<RAWBrush*>(item_list->GetSelectedBrush());
		if(brush == nullptr) {
			// It's either "Please enter a search string" or "No matches"
			// Perhaps we can refresh now?
			std::string search_string = as_lower_str(nstr(search_field->GetValue()));
			try {
				actionid = boost::lexical_cast<int>(nstr(action_field->GetValue()).c_str());
			} catch(boost::bad_lexical_cast &) {
				actionid = -1;
			}

			try {
				uniqueid = boost::lexical_cast<int>(nstr(unique_field->GetValue()).c_str());
			} catch(boost::bad_lexical_cast &) {
				uniqueid = -1;
			}

			bool do_search = (search_string.size() >= 2) || actionid > -1 || uniqueid > -1;
			if(do_search) {
				for(int id = 0; id <= item_db.getMaxID(); ++id) {
					ItemType& it = item_db[id];
					if(it.id == 0 || (extra_condition && !extra_condition(it)))
						continue;

					RAWBrush* raw = it.raw_brush;
					if(raw == nullptr)
						continue;

					if(as_lower_str(raw->getName()).find(search_string) == std::string::npos)
						continue;

					// Found one!
					result_brush = raw;
					result_id = raw->getItemID();
					break;
				}
			}
		} else {
			result_brush = brush;
			result_id = brush->getItemID();
		}
	}
	EndModal(result_id);
}

void FindItemDialog::RefreshContentsInternal() 
{
	item_list->Clear();

	std::string search_string = as_lower_str(nstr(search_field->GetValue()));
	bool do_search = (search_string.size() >= 2);

	if(do_search) {
		for(int id = 0; id <= item_db.getMaxID(); ++id) {
			ItemType& it = item_db[id];
			if(it.id == 0 || (extra_condition && !extra_condition(it)))
				continue;

			RAWBrush* raw = it.raw_brush;
			if(raw == nullptr)
				continue;

			if(as_lower_str(raw->getName()).find(search_string) == std::string::npos)
				continue;

			item_list->AddBrush(raw);
		}

		if(item_list->GetItemCount() > 0) {
			item_list->SetSelection(0);
		}
		else {
			item_list->SetNoMatches();
		}
	}
	item_list->RefreshAll();
}

// ============================================================================
// Find Brush Dialog (Jump to brush)

FindBrushDialog::FindBrushDialog(wxWindow* parent, wxString title) : FindDialog(parent, title) 
{
	RefreshContents();
}

FindBrushDialog::~FindBrushDialog() 
{
	////
}

void FindBrushDialog::OnClickListInternal(wxCommandEvent& event) 
{
	Brush* brush = item_list->GetSelectedBrush();
	if(brush) {
		result_brush = brush;
		EndModal(1);
	}
}

void FindBrushDialog::OnClickOKInternal() 
{
	// This is kind of stupid as it would fail unless the "Please enter a search string" wasn't there
	if(item_list->GetItemCount() > 0) {
		if(item_list->GetSelection() == wxNOT_FOUND) {
			item_list->SetSelection(0);
		}
		Brush* brush = item_list->GetSelectedBrush();
		if(brush == nullptr) {
			// It's either "Please enter a search string" or "No matches"
			// Perhaps we can refresh now?
			std::string search_string = as_lower_str(nstr(search_field->GetValue()));
			bool do_search = (search_string.size() >= 2);

			if(do_search) {
				const BrushMap& map = brushes.getMap();
				for(BrushMap::const_iterator iter = map.begin(); iter != map.end(); ++iter) {
					const Brush* brush = iter->second;
					if(as_lower_str(brush->getName()).find(search_string) == std::string::npos)
						continue;

					// Don't match RAWs now.
					if(dynamic_cast<const RAWBrush*>(brush))
						continue;

					// Found one!
					result_brush = brush;
					break;
				}

				// Did we not find a matching brush?
				if(!result_brush) {
					// Then let's search the RAWs
					for(int id = 0; id <= item_db.getMaxID(); ++id) {
						ItemType& it = item_db[id];
						if(it.id == 0)
							continue;

						RAWBrush* raw = it.raw_brush;
						if(raw == nullptr)
							continue;

						if(as_lower_str(raw->getName()).find(search_string) == std::string::npos)
							continue;

						// Found one!
						result_brush = raw;
						break;
					}
				}
				// Done!
			}
		} else {
			result_brush = brush;
		}
	}
	EndModal(1);
}

void FindBrushDialog::RefreshContentsInternal() 
{
	item_list->Clear();

	std::string search_string = as_lower_str(nstr(search_field->GetValue()));
	bool do_search = (search_string.size() >= 2);

	if(do_search) {
		const BrushMap& brushes_map = brushes.getMap();
		
		// We store the raws so they display last of all results
		std::deque<const RAWBrush*> raws;

		for(BrushMap::const_iterator iter = brushes_map.begin(); iter != brushes_map.end(); ++iter) {
			const Brush* brush = iter->second;

			if(as_lower_str(brush->getName()).find(search_string) == std::string::npos)
				continue;

			if(dynamic_cast<const RAWBrush*>(brush))
				continue;

			item_list->AddBrush(const_cast<Brush*>(brush));
		}

		for(int id = 0; id <= item_db.getMaxID(); ++id) {
			ItemType& it = item_db[id];
			if(it.id == 0)
				continue;

			RAWBrush* raw = it.raw_brush;
			if(raw == nullptr)
				continue;

			if(as_lower_str(raw->getName()).find(search_string) == std::string::npos)
				continue;

			item_list->AddBrush(raw);
		}

		while(raws.size() > 0) {
			item_list->AddBrush(const_cast<RAWBrush*>(raws.front()));
			raws.pop_front();
		}

		if(item_list->GetItemCount() > 0) {
			item_list->SetSelection(0);
		} else {
			item_list->SetNoMatches();
		}
	}
}

// ============================================================================
// Replace item dialog

BEGIN_EVENT_TABLE(ReplaceItemDialog, wxDialog)
	EVT_TIMER(wxID_ANY, ReplaceItemDialog::OnTextIdle)

	EVT_KEY_DOWN(ReplaceItemDialog::OnKeyDown)
	EVT_TEXT(REPLACE_DIALOG_FIND_TEXT, ReplaceItemDialog::OnTextChange)
	EVT_TEXT_ENTER(REPLACE_DIALOG_FIND_TEXT, ReplaceItemDialog::OnClickOK)
	EVT_TEXT(REPLACE_DIALOG_WITH_TEXT, ReplaceItemDialog::OnTextChange)
	EVT_TEXT_ENTER(REPLACE_DIALOG_WITH_TEXT, ReplaceItemDialog::OnClickOK)

	EVT_BUTTON(wxID_OK, ReplaceItemDialog::OnClickOK)
	EVT_BUTTON(wxID_CANCEL, ReplaceItemDialog::OnClickCancel)
END_EVENT_TABLE()

ReplaceItemDialog::ReplaceItemDialog(wxWindow* parent, wxString title) :
	wxDialog(gui.root, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX),
	find_idle_input_timer(this),
	with_idle_input_timer(this)
{
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);

	wxFlexGridSizer *gridsizer = newd wxFlexGridSizer(2, 10, 10);

	// Labels
	gridsizer->Add(newd wxStaticText(this, wxID_ANY, wxT("Replace this item:")), 0);
	gridsizer->Add(newd wxStaticText(this, wxID_ANY, wxT("With this item:")), 0);

	// The text fields
	find_item_field = newd KeyForwardingTextCtrl(this, REPLACE_DIALOG_FIND_TEXT, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	find_item_field->SetFocus();
	gridsizer->Add(find_item_field, wxSizerFlags(0).Expand());

	with_item_field = newd KeyForwardingTextCtrl(this, REPLACE_DIALOG_WITH_TEXT, wxT(""), wxDefaultPosition, wxDefaultSize, wxTE_PROCESS_ENTER);
	gridsizer->Add(with_item_field, wxSizerFlags(0).Expand());

	// The lists
	find_item_list = newd FindDialogListBox(this, REPLACE_DIALOG_FIND_LIST);
	find_item_list->SetMinSize(wxSize(250, 400));
	gridsizer->Add(find_item_list, wxSizerFlags(1).Expand());

	with_item_list = newd FindDialogListBox(this, REPLACE_DIALOG_WITH_LIST);
	with_item_list->SetMinSize(wxSize(250, 400));
	gridsizer->Add(with_item_list, wxSizerFlags(1).Expand());

	topsizer->Add(gridsizer, wxSizerFlags(1).Expand().Border());

	// Buttons
	wxSizer* stdsizer = newd wxBoxSizer(wxHORIZONTAL);
	stdsizer->Add(newd wxButton(this, wxID_OK, wxT("Replace")), wxSizerFlags(1).Center());
	stdsizer->Add(newd wxButton(this, wxID_CANCEL, wxT("Cancel")), wxSizerFlags(1).Center());
	topsizer->Add(stdsizer, wxSizerFlags(0).Center().Border());

	SetSizerAndFit(topsizer);
	
	RefreshContents(find_item_list);
	RefreshContents(with_item_list);
}
	

ReplaceItemDialog::~ReplaceItemDialog() 
{
	////
}

void ReplaceItemDialog::OnKeyDown(wxKeyEvent& event) 
{
	FindDialogListBox *item_list = (event.GetEventObject() == find_item_field ? find_item_list : with_item_list);
	int w, h;
	item_list->GetSize(&w, &h);
	size_t amount = 1; 

	switch(event.GetKeyCode()) {
		case WXK_PAGEUP:
			amount = h / 32 + 1;
		case WXK_UP: {
			if(item_list->GetItemCount() > 0) {
				ssize_t n = item_list->GetSelection();
				if(n == wxNOT_FOUND) 
					n = 0;
				else if(n != amount && n - amount < n) // latter is needed for unsigned overflow
					n -= amount;
				else
					n = 0;
				item_list->SetSelection(n);
			}
			break;
		}
		case WXK_PAGEDOWN:
			amount = h / 32 + 1;
		case WXK_DOWN: {
			if(item_list->GetItemCount() > 0) {
				ssize_t n = item_list->GetSelection();
				size_t itemcount = item_list->GetItemCount();
				if(n == wxNOT_FOUND)
					n = 0;
				else if(static_cast<uint32_t>(n) < itemcount - amount && itemcount - amount < itemcount)
					n += amount;
				else
					n = item_list->GetItemCount() - 1;

				item_list->SetSelection(n);
			}
			break;
		}
		default:
			event.Skip();
			break;
	}
}

void ReplaceItemDialog::OnTextIdle(wxTimerEvent& event) 
{
	if(&event.GetTimer() == &find_idle_input_timer)
		RefreshContents(find_item_list);
	else
		RefreshContents(with_item_list);
}

void ReplaceItemDialog::OnTextChange(wxCommandEvent& event) 
{
	if(event.GetEventObject() == find_item_field)
		find_idle_input_timer.Start(800, true);
	else
		with_idle_input_timer.Start(800, true);
}

void ReplaceItemDialog::OnClickOK(wxCommandEvent& WXUNUSED(event)) 
{
	// This is kind of stupid as it would fail unless the "Please enter a search string" wasn't there
	if(find_item_list->GetItemCount() > 0 && with_item_list->GetItemCount() > 0) {
		if(find_item_list->GetSelection() == wxNOT_FOUND)
			find_item_list->SetSelection(0);
		if(with_item_list->GetSelection() == wxNOT_FOUND)
			with_item_list->SetSelection(0);
		
		result_find_brush = dynamic_cast<RAWBrush*>(find_item_list->GetSelectedBrush());
		result_with_brush = dynamic_cast<RAWBrush*>(with_item_list->GetSelectedBrush());
	}

	if(result_find_brush == nullptr || result_with_brush == nullptr) {
		result_find_brush = nullptr;
		result_with_brush = nullptr;

		gui.PopupDialog(wxT("Select both items"), wxT("You must select two items for the replacement to work."), wxOK);
		return;
	}

	EndModal(1);
}

void ReplaceItemDialog::OnClickCancel(wxCommandEvent& WXUNUSED(event)) 
{
	EndModal(0);
}

void ReplaceItemDialog::RefreshContents(FindDialogListBox *which_list) 
{
	which_list->Clear();

	wxTextCtrl *search_field = (which_list == find_item_list ? find_item_field : with_item_field);

	std::string search_string = as_lower_str(nstr(search_field->GetValue()));
	bool do_search = (search_string.size() >= 2);

	if(do_search) {
		for(int id = 0; id <= item_db.getMaxID(); ++id) 
		{
			ItemType& it = item_db[id];
			if(it.id == 0)
				continue;

			RAWBrush* raw = it.raw_brush;
			if(raw == nullptr)
				continue;

			if(as_lower_str(raw->getName()).find(search_string) == std::string::npos)
				continue;

			which_list->AddBrush(raw);
		}

		if(which_list->GetItemCount() > 0)
			which_list->SetSelection(0);
		else
			which_list->SetNoMatches();
	}
}

uint16_t ReplaceItemDialog::GetResultFindID() const
{
	return result_find_brush ? static_cast<RAWBrush*>(result_find_brush)->getItemID() : 0;
}

uint16_t ReplaceItemDialog::GetResultWithID() const
{
	return result_with_brush ? static_cast<RAWBrush*>(result_with_brush)->getItemID() : 0;
}

// ============================================================================
// Listbox in find item / brush stuff

FindDialogListBox::FindDialogListBox(wxWindow* parent, wxWindowID id) :
	wxVListBox(parent, id, wxDefaultPosition, wxDefaultSize, wxLB_SINGLE),
	cleared(false),
	no_matches(false)
{
	Clear();
}

FindDialogListBox::~FindDialogListBox() 
{
	////
}

void FindDialogListBox::Clear() 
{
	cleared = true;
	no_matches = false;
	brushlist.clear();
	SetItemCount(1);
}

void FindDialogListBox::SetNoMatches() 
{
	cleared = false;
	no_matches = true;
	brushlist.clear();
	SetItemCount(1);
}

void FindDialogListBox::AddBrush(Brush* brush) 
{
	if(cleared || no_matches)
		SetItemCount(0);
	
	cleared = false;
	no_matches = false;

	SetItemCount(GetItemCount() + 1);
	brushlist.push_back(brush);
}

Brush* FindDialogListBox::GetSelectedBrush() 
{
	ssize_t n = GetSelection();
	if(n == wxNOT_FOUND || no_matches || cleared)
		return nullptr;
	return brushlist[n];
}

void FindDialogListBox::OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const
{
	if(no_matches) {
		dc.DrawText(wxT("No matches for your search."), rect.GetX() + 40, rect.GetY() + 6);
	} else if(cleared) {
		dc.DrawText(wxT("Please enter your search string."), rect.GetX() + 40, rect.GetY() + 6);
	} else {
		ASSERT(n < brushlist.size());
		Sprite* spr = gui.gfx.getSprite(brushlist[n]->getLookID());
		if(spr) {
			spr->DrawTo(&dc, SPRITE_SIZE_32x32, rect.GetX(), rect.GetY(), rect.GetWidth(), rect.GetHeight());
		}

		if(int(n) == GetSelection()) {
			dc.SetTextForeground(wxColor(0xFF, 0xFF, 0xFF));
		} else {
			dc.SetTextForeground(wxColor(0x00, 0x00, 0x00));
		}

		dc.DrawText(wxstr(brushlist[n]->getName()), rect.GetX() + 40, rect.GetY() + 6);
	}
}

wxCoord FindDialogListBox::OnMeasureItem(size_t n) const
{
	return 32;
}

// ============================================================================
// Object properties base

ObjectPropertiesWindowBase::ObjectPropertiesWindowBase(wxWindow* parent, wxString title, const Map* map, const Tile* tile, Item* item, wxPoint position /* = wxDefaultPosition */) :
wxDialog(parent, wxID_ANY, title,
	position, wxSize(600, 400), wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER),
	edit_map(map),
	edit_tile(tile),
	edit_item(item),
	edit_creature(nullptr),
	edit_spawn(nullptr)
{
	////
}

ObjectPropertiesWindowBase::ObjectPropertiesWindowBase(wxWindow* parent, wxString title, const Map* map, const Tile* tile, Creature* creature, wxPoint position /* = wxDefaultPosition */) :
wxDialog(parent, wxID_ANY, title,
	position, wxSize(600, 400), wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER),
	edit_map(map),
	edit_tile(tile),
	edit_item(nullptr),
	edit_creature(creature),
	edit_spawn(nullptr)
{
	////
}

ObjectPropertiesWindowBase::ObjectPropertiesWindowBase(wxWindow* parent, wxString title, const Map* map, const Tile* tile, Spawn* spawn, wxPoint position /* = wxDefaultPosition */) :
wxDialog(parent, wxID_ANY, title,
	position, wxSize(600, 400), wxCAPTION | wxCLOSE_BOX | wxRESIZE_BORDER),
	edit_map(map),
	edit_tile(tile),
	edit_item(nullptr),
	edit_creature(nullptr),
	edit_spawn(spawn)
{
	////
}

Item* ObjectPropertiesWindowBase::getItemBeingEdited()
{
	return edit_item;
}

// ============================================================================
// Edit Towns Dialog

BEGIN_EVENT_TABLE(EditTownsDialog, wxDialog)
	EVT_LISTBOX(EDIT_TOWNS_LISTBOX, EditTownsDialog::OnListBoxChange)

	EVT_BUTTON(EDIT_TOWNS_SELECT_TEMPLE, EditTownsDialog::OnClickSelectTemplePosition)
	EVT_BUTTON(EDIT_TOWNS_ADD, EditTownsDialog::OnClickAdd)
	EVT_BUTTON(EDIT_TOWNS_REMOVE, EditTownsDialog::OnClickRemove)
	EVT_BUTTON(wxID_OK, EditTownsDialog::OnClickOK)
	EVT_BUTTON(wxID_CANCEL, EditTownsDialog::OnClickCancel)
END_EVENT_TABLE()

EditTownsDialog::EditTownsDialog(wxWindow* parent, Editor& editor) :
	wxDialog(parent, wxID_ANY, wxT("Towns"), wxDefaultPosition, wxSize(280,330)),
	editor(editor)
{
	Map& map = editor.map;

	// Create topsizer
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	wxSizer* tmpsizer;

	for(TownMap::const_iterator town_iter = map.towns.begin(); town_iter != map.towns.end(); ++town_iter) {
		Town* town = town_iter->second;
		town_list.push_back(newd Town(*town));
		if(max_town_id < town->getID()) {
			max_town_id = town->getID();
		}
	}

	// Town list
	town_listbox = newd wxListBox(this, EDIT_TOWNS_LISTBOX, wxDefaultPosition, wxSize(240, 100));
	sizer->Add(town_listbox, 1, wxEXPAND | wxTOP | wxLEFT | wxRIGHT, 10);

	tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	tmpsizer->Add(newd wxButton(this, EDIT_TOWNS_ADD, wxT("Add")), 0, wxTOP, 5);
	tmpsizer->Add(remove_button = newd wxButton(this, EDIT_TOWNS_REMOVE, wxT("Remove")), 0, wxRIGHT | wxTOP, 5);
	sizer->Add(tmpsizer, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

	// House options
	tmpsizer = newd wxStaticBoxSizer(wxHORIZONTAL, this, wxT("Name / ID"));
	name_field = newd wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(190,20), 0, wxTextValidator(wxFILTER_ASCII, &town_name));
	tmpsizer->Add(name_field, 2, wxEXPAND | wxLEFT | wxBOTTOM, 5);

	id_field = newd wxTextCtrl(this, wxID_ANY, wxT(""), wxDefaultPosition, wxSize(40,20), 0, wxTextValidator(wxFILTER_NUMERIC, &town_id));
	id_field->Enable(false);
	tmpsizer->Add(id_field, 1, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 5);
	sizer->Add(tmpsizer, 0, wxEXPAND | wxALL, 10);

	// Temple position
	temple_position = newd PositionCtrl(this, wxT("Temple Position"), 0, 0, 0, map.getWidth(), map.getHeight());
	select_position_button = newd wxButton(this, EDIT_TOWNS_SELECT_TEMPLE, wxT("Go To"));
	temple_position->Add(select_position_button, 0, wxLEFT | wxRIGHT | wxBOTTOM, 5);
	sizer->Add(temple_position, 0, wxEXPAND | wxLEFT | wxRIGHT, 10);

	// OK/Cancel buttons
	tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	tmpsizer->Add(newd wxButton(this, wxID_OK, wxT("OK")), wxSizerFlags(1).Center());
	tmpsizer->Add(newd wxButton(this, wxID_CANCEL, wxT("Cancel")), wxSizerFlags(1).Center());
	sizer->Add(tmpsizer, 0, wxCENTER | wxALL, 10);

	SetSizerAndFit(sizer);

	BuildListBox(true);
}

EditTownsDialog::~EditTownsDialog()
{
	for(std::vector<Town*>::iterator town_iter = town_list.begin(); town_iter != town_list.end(); ++town_iter) {
		delete *town_iter;
	}
}

void EditTownsDialog::BuildListBox(bool doselect)
{
	long tmplong = 0;
	max_town_id = 0;
	wxArrayString town_name_list;
	uint32_t selection_before = 0;

	if(doselect && id_field->GetValue().ToLong(&tmplong)) {
		uint32_t old_town_id = tmplong;

		for(std::vector<Town*>::iterator town_iter = town_list.begin(); town_iter != town_list.end(); ++town_iter) {
			if(old_town_id == (*town_iter)->getID()) {
				selection_before = (*town_iter)->getID();
				break;
			}
		}
	}

	for(std::vector<Town*>::iterator town_iter = town_list.begin(); town_iter != town_list.end(); ++town_iter) {
		Town* town = *town_iter;
		town_name_list.Add(wxstr(town->getName()));
		if(max_town_id < town->getID()) {
			max_town_id = town->getID();
		}
	}

	town_listbox->Set(town_name_list);
	remove_button->Enable(town_listbox->GetCount() != 0);
	select_position_button->Enable(false);

	if(doselect) {
		if(selection_before) {
			int i = 0;
			for(std::vector<Town*>::iterator town_iter = town_list.begin(); town_iter != town_list.end(); ++town_iter) {
				if(selection_before == (*town_iter)->getID()) {
					town_listbox->SetSelection(i);
					return;
				}
				++i;
			}
		}
		UpdateSelection(0);
	}
}

void EditTownsDialog::UpdateSelection(int new_selection)
{
	long tmplong;

	// Save old values
	if(town_list.size() > 0) {
		if(id_field->GetValue().ToLong(&tmplong)) {
			uint32_t old_town_id = tmplong;

			Town* old_town = nullptr;

			for(std::vector<Town*>::iterator town_iter = town_list.begin(); town_iter != town_list.end(); ++town_iter) {
				if(old_town_id == (*town_iter)->getID()) {
					old_town = *town_iter;
					break;
				}
			}

			if(old_town) {
				Position templepos = temple_position->GetPosition();

				//printf("Changed town %d:%s\n", old_town_id, old_town->getName().c_str());
				//printf("New values %d:%s:%d:%d:%d\n", town_id, town_name.c_str(), templepos.x, templepos.y, templepos.z);
				old_town->setTemplePosition(templepos);

				wxString new_name = name_field->GetValue();
				wxString old_name = wxstr(old_town->getName());

				old_town->setName(nstr(new_name));
				if(new_name != old_name) {
					// Name has changed, update list
					BuildListBox(false);
				}
			}
		}
	}

	// Clear fields
	town_name.Clear();
	town_id.Clear();

	if(town_list.size() > size_t(new_selection)) {
		name_field->Enable(true);
		temple_position->Enable(true);
		select_position_button->Enable(true);

		// Change the values to reflect the newd selection
		Town* town = town_list[new_selection];
		ASSERT(town);

		//printf("Selected %d:%s\n", new_selection, town->getName().c_str());
		town_name << wxstr(town->getName());
		name_field->SetValue(town_name);
		town_id << long(town->getID());
		id_field->SetValue(town_id);
		temple_position->SetPosition(town->getTemplePosition());
		town_listbox->SetSelection(new_selection);
	} else {
		name_field->Enable(false);
		temple_position->Enable(false);
		select_position_button->Enable(false);
	}
	Refresh();
}

void EditTownsDialog::OnListBoxChange(wxCommandEvent& event)
{
	UpdateSelection(event.GetSelection());
}

void EditTownsDialog::OnClickSelectTemplePosition(wxCommandEvent& WXUNUSED(event))
{
	Position templepos = temple_position->GetPosition();
	gui.CenterOnPosition(templepos);
}

void EditTownsDialog::OnClickAdd(wxCommandEvent& WXUNUSED(event))
{
	Town* new_town = newd Town(++max_town_id);
	new_town->setName("Unnamed Town");
	new_town->setTemplePosition(Position(0,0,0));
	town_list.push_back(new_town);

	BuildListBox(false);
	UpdateSelection(town_list.size()-1);
	town_listbox->SetSelection(town_list.size()-1);
}

void EditTownsDialog::OnClickRemove(wxCommandEvent& WXUNUSED(event))
{
	long tmplong;
	if(id_field->GetValue().ToLong(&tmplong)) {
		uint32_t old_town_id = tmplong;

		Town* town = nullptr;

		std::vector<Town*>::iterator town_iter = town_list.begin();

		int selection_index = 0;
		while(town_iter != town_list.end()) {
			if(old_town_id == (*town_iter)->getID()) {
				town = *town_iter;
				break;
			}
			++selection_index;
			++town_iter;
		}
		if(!town) return;

		Map& map = editor.map;
		for(HouseMap::iterator house_iter = map.houses.begin(); house_iter != map.houses.end(); ++house_iter) {
			House* house = house_iter->second;
			if(house->townid == town->getID()) {
				gui.PopupDialog(this, wxT("Error"), wxT("You cannot delete a town which still has houses associated with it."), wxOK);
				return;
			}
		}

		delete town;
		town_list.erase(town_iter);
		BuildListBox(false);
		UpdateSelection(selection_index-1);
	}
}

void EditTownsDialog::OnClickOK(wxCommandEvent& WXUNUSED(event))
{
	long tmplong = 0;

	if(Validate() && TransferDataFromWindow()) {
		// Save old values
		if(town_list.size() > 0 && id_field->GetValue().ToLong(&tmplong)) {
			uint32_t old_town_id = tmplong;

			Town* old_town = nullptr;

			for(std::vector<Town*>::iterator town_iter = town_list.begin(); town_iter != town_list.end(); ++town_iter) {
				if(old_town_id == (*town_iter)->getID()) {
					old_town = *town_iter;
					break;
				}
			}

			if(old_town) {
				Position templepos = temple_position->GetPosition();

				//printf("Changed town %d:%s\n", old_town_id, old_town->getName().c_str());
				//printf("New values %d:%s:%d:%d:%d\n", town_id, town_name.c_str(), templepos.x, templepos.y, templepos.z);
				old_town->setTemplePosition(templepos);

				wxString new_name = name_field->GetValue();
				wxString old_name = wxstr(old_town->getName());

				old_town->setName(nstr(new_name));
				if(new_name != old_name) {
					// Name has changed, update list
					BuildListBox(true);
				}
			}
		}

		Towns& towns = editor.map.towns;

		// Verify the newd information
		for(std::vector<Town*>::iterator town_iter = town_list.begin(); town_iter != town_list.end(); ++town_iter) {
			Town* town = *town_iter;
			if(town->getName() == "") {
				gui.PopupDialog(this, wxT("Error"), wxT("You can't have a town with an empty name."), wxOK);
				return;
			}
			if(!town->getTemplePosition().isValid() ||
				town->getTemplePosition().x > editor.map.getWidth() ||
				town->getTemplePosition().y > editor.map.getHeight()) {
				wxString msg;
				msg << wxT("The town ") << wxstr(town->getName()) << wxT(" has an invalid temple position.");
				gui.PopupDialog(this, wxT("Error"), msg, wxOK);
				return;
			}
		}

		// Clear old towns
		towns.clear();

		// Build the newd town map
		for(std::vector<Town*>::iterator town_iter = town_list.begin(); town_iter != town_list.end(); ++town_iter) {
			towns.addTown(*town_iter);
		}
		town_list.clear();
		editor.map.doChange();

		EndModal(1);
		gui.RefreshPalettes();
	}
}

void EditTownsDialog::OnClickCancel(wxCommandEvent& WXUNUSED(event))
{
	// Just close this window
	EndModal(0);
}

// ============================================================================
// Go To Position Dialog
// Jump to a position on the map by entering XYZ coordinates

BEGIN_EVENT_TABLE(GotoPositionDialog, wxDialog)
	EVT_BUTTON(wxID_OK, GotoPositionDialog::OnClickOK)
	EVT_BUTTON(wxID_CANCEL, GotoPositionDialog::OnClickCancel)
END_EVENT_TABLE()

GotoPositionDialog::GotoPositionDialog(wxWindow* parent, Editor& editor) :
	wxDialog(parent, wxID_ANY, wxT("Go To Position"), wxDefaultPosition, wxDefaultSize),
	editor(editor)
{
	Map& map = editor.map;

	// create topsizer
	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);

	posctrl = newd PositionCtrl(this, wxT("Destination"), map.getWidth() / 2, map.getHeight() / 2, 7, map.getWidth(), map.getHeight());
	sizer->Add(posctrl, 0, wxTOP | wxLEFT | wxRIGHT, 20);

	// OK/Cancel buttons
	wxSizer* tmpsizer = newd wxBoxSizer(wxHORIZONTAL);
	tmpsizer->Add(newd wxButton(this, wxID_OK, wxT("OK")), wxSizerFlags(1).Center());
	tmpsizer->Add(newd wxButton(this, wxID_CANCEL, wxT("Cancel")), wxSizerFlags(1).Center());
	sizer->Add(tmpsizer, 0, wxALL | wxCENTER, 20); // Border to top too

	SetSizerAndFit(sizer);
}

void GotoPositionDialog::OnClickCancel(wxCommandEvent &)
{
	EndModal(0);
}

void GotoPositionDialog::OnClickOK(wxCommandEvent &)
{
	gui.CenterOnPosition(posctrl->GetPosition());
	EndModal(1);
}
