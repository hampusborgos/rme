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

#include <wx/collpane.h>

#include "settings.h"
#include "client_version.h"
#include "editor.h"

#include "gui.h"

#include "preferences.h"

BEGIN_EVENT_TABLE(PreferencesWindow, wxDialog)
	EVT_BUTTON(wxID_OK, PreferencesWindow::OnClickOK)
	EVT_BUTTON(wxID_CANCEL, PreferencesWindow::OnClickCancel)
	EVT_BUTTON(wxID_APPLY, PreferencesWindow::OnClickApply)
	EVT_COLLAPSIBLEPANE_CHANGED(wxID_ANY, PreferencesWindow::OnCollapsiblePane)
END_EVENT_TABLE()

PreferencesWindow::PreferencesWindow(wxWindow *parent, bool clientVersionSelected = false)
        : wxDialog(parent, wxID_ANY, "Preferences", wxDefaultPosition, wxSize(400, 400), wxCAPTION | wxCLOSE_BOX) {
    wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);

    book = newd wxNotebook(this, wxID_ANY, wxDefaultPosition, wxDefaultSize, wxBK_TOP);
    //book->SetPadding(4);

    book->AddPage(CreateGeneralPage(), "General", true);
    book->AddPage(CreateEditorPage(), "Editor");
    book->AddPage(CreateGraphicsPage(), "Graphics");
    book->AddPage(CreateUIPage(), "Interface");
    book->AddPage(CreateClientPage(), "Client Version", clientVersionSelected);

    sizer->Add(book, 1, wxEXPAND | wxALL, 10);

    wxSizer* subsizer = newd wxBoxSizer(wxHORIZONTAL);
    subsizer->Add(newd wxButton(this, wxID_OK, "OK"), wxSizerFlags(1).Center());
    subsizer->Add(newd wxButton(this, wxID_CANCEL, "Cancel"), wxSizerFlags(1).Border(wxALL, 5).Left().Center());
    subsizer->Add(newd wxButton(this, wxID_APPLY, "Apply"), wxSizerFlags(1).Center());
    sizer->Add(subsizer, 0, wxCENTER | wxLEFT | wxBOTTOM | wxRIGHT, 10);

    SetSizerAndFit(sizer);
    Centre(wxBOTH);
    // FindWindowById(PANE_ADVANCED_GRAPHICS, this)->GetParent()->Fit();
}

PreferencesWindow::~PreferencesWindow()
{
	////
}

wxNotebookPage* PreferencesWindow::CreateGeneralPage()
{
	wxNotebookPage* general_page = newd wxPanel(book, wxID_ANY);

	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	wxStaticText* tmptext;

	show_welcome_dialog_chkbox = newd wxCheckBox(general_page, wxID_ANY, "Show welcome dialog on startup");
	show_welcome_dialog_chkbox->SetValue(g_settings.getInteger(Config::WELCOME_DIALOG) == 1);
	show_welcome_dialog_chkbox->SetToolTip("Show welcome dialog when starting the editor.");
	sizer->Add(show_welcome_dialog_chkbox, 0, wxLEFT | wxTOP, 5);

	always_make_backup_chkbox = newd wxCheckBox(general_page, wxID_ANY, "Always make map backup");
	always_make_backup_chkbox->SetValue(g_settings.getInteger(Config::ALWAYS_MAKE_BACKUP) == 1);
	sizer->Add(always_make_backup_chkbox, 0, wxLEFT | wxTOP, 5);

	update_check_on_startup_chkbox = newd wxCheckBox(general_page, wxID_ANY, "Check for updates on startup");
	update_check_on_startup_chkbox->SetValue(g_settings.getInteger(Config::USE_UPDATER) == 1);
	sizer->Add(update_check_on_startup_chkbox, 0, wxLEFT | wxTOP, 5);

	only_one_instance_chkbox = newd wxCheckBox(general_page, wxID_ANY, "Open all maps in the same instance");
	only_one_instance_chkbox->SetValue(g_settings.getInteger(Config::ONLY_ONE_INSTANCE) == 1);
	only_one_instance_chkbox->SetToolTip("When checked, maps opened using the shell will all be opened in the same instance.");
	sizer->Add(only_one_instance_chkbox, 0, wxLEFT | wxTOP, 5);

	sizer->AddSpacer(10);

    auto * grid_sizer = newd wxFlexGridSizer(2, 10, 10);
	grid_sizer->AddGrowableCol(1);

	grid_sizer->Add(tmptext = newd wxStaticText(general_page, wxID_ANY, "Undo queue size: "), 0);
	undo_size_spin = newd wxSpinCtrl(general_page, wxID_ANY, i2ws(g_settings.getInteger(Config::UNDO_SIZE)), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0x10000000);
	grid_sizer->Add(undo_size_spin, 0);
	SetWindowToolTip(tmptext, undo_size_spin, "How many action you can undo, be aware that a high value will increase memory usage.");

	grid_sizer->Add(tmptext = newd wxStaticText(general_page, wxID_ANY, "Undo maximum memory size (MB): "), 0);
	undo_mem_size_spin = newd wxSpinCtrl(general_page, wxID_ANY, i2ws(g_settings.getInteger(Config::UNDO_MEM_SIZE)), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 4096);
	grid_sizer->Add(undo_mem_size_spin, 0);
	SetWindowToolTip(tmptext, undo_mem_size_spin, "The approximite limit for the memory usage of the undo queue.");

	grid_sizer->Add(tmptext = newd wxStaticText(general_page, wxID_ANY, "Worker Threads: "), 0);
	worker_threads_spin = newd wxSpinCtrl(general_page, wxID_ANY, i2ws(g_settings.getInteger(Config::WORKER_THREADS)), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 64);
	grid_sizer->Add(worker_threads_spin, 0);
	SetWindowToolTip(tmptext, worker_threads_spin, "How many threads the editor will use for intensive operations. This should be equivalent to the amount of logical processors in your system.");

	grid_sizer->Add(tmptext = newd wxStaticText(general_page, wxID_ANY, "Replace count: "), 0);
	replace_size_spin = newd wxSpinCtrl(general_page, wxID_ANY, i2ws(g_settings.getInteger(Config::REPLACE_SIZE)), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 100000);
	grid_sizer->Add(replace_size_spin, 0);
	SetWindowToolTip(tmptext, replace_size_spin, "How many items you can replace on the map using the Replace Item tool.");

	sizer->Add(grid_sizer, 0, wxALL, 5);
	sizer->AddSpacer(10);

	wxString position_choices[] = { "  {x = 0, y = 0, z = 0}",
                                    R"(  {"x":0,"y":0,"z":0})",
									"  x, y, z",
									"  (x, y, z)",
									"  Position(x, y, z)" };
	int radio_choices = sizeof(position_choices) / sizeof(wxString);
	position_format = newd wxRadioBox(general_page, wxID_ANY, "Copy Position Format", wxDefaultPosition, wxDefaultSize, radio_choices, position_choices, 1, wxRA_SPECIFY_COLS);
	position_format->SetSelection(g_settings.getInteger(Config::COPY_POSITION_FORMAT));
	sizer->Add(position_format, 0, wxALL | wxEXPAND, 5);
	SetWindowToolTip(tmptext, position_format, "The position format when copying from the map.");

	general_page->SetSizerAndFit(sizer);

	return general_page;
}

wxNotebookPage* PreferencesWindow::CreateEditorPage()
{
	wxNotebookPage* editor_page = newd wxPanel(book, wxID_ANY);

	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);

	group_actions_chkbox = newd wxCheckBox(editor_page, wxID_ANY, "Group same-type actions");
	group_actions_chkbox->SetValue(g_settings.getBoolean(Config::GROUP_ACTIONS));
	group_actions_chkbox->SetToolTip("This will group actions of the same type (drawing, selection..) when several take place in consecutive order.");
	sizer->Add(group_actions_chkbox, 0, wxLEFT | wxTOP, 5);

	duplicate_id_warn_chkbox = newd wxCheckBox(editor_page, wxID_ANY, "Warn for duplicate IDs");
	duplicate_id_warn_chkbox->SetValue(g_settings.getBoolean(Config::WARN_FOR_DUPLICATE_ID));
	duplicate_id_warn_chkbox->SetToolTip("Warns for most kinds of duplicate IDs.");
	sizer->Add(duplicate_id_warn_chkbox, 0, wxLEFT | wxTOP, 5);

	house_remove_chkbox = newd wxCheckBox(editor_page, wxID_ANY, "House brush removes items");
	house_remove_chkbox->SetValue(g_settings.getBoolean(Config::HOUSE_BRUSH_REMOVE_ITEMS));
	house_remove_chkbox->SetToolTip("When this option is checked, the house brush will automaticly remove items that will respawn every time the map is loaded.");
	sizer->Add(house_remove_chkbox, 0, wxLEFT | wxTOP, 5);

	auto_assign_doors_chkbox = newd wxCheckBox(editor_page, wxID_ANY, "Auto-assign door ids");
	auto_assign_doors_chkbox->SetValue(g_settings.getBoolean(Config::AUTO_ASSIGN_DOORID));
	auto_assign_doors_chkbox->SetToolTip("This will auto-assign unique door ids to all doors placed with the door brush (or doors painted over with the house brush).\nDoes NOT affect doors placed using the RAW palette.");
	sizer->Add(auto_assign_doors_chkbox, 0, wxLEFT | wxTOP, 5);

	doodad_erase_same_chkbox = newd wxCheckBox(editor_page, wxID_ANY, "Doodad brush only erases same");
	doodad_erase_same_chkbox->SetValue(g_settings.getBoolean(Config::DOODAD_BRUSH_ERASE_LIKE));
	doodad_erase_same_chkbox->SetToolTip("The doodad brush will only erase items that belongs to the current brush.");
	sizer->Add(doodad_erase_same_chkbox, 0, wxLEFT | wxTOP, 5);

	eraser_leave_unique_chkbox = newd wxCheckBox(editor_page, wxID_ANY, "Eraser leaves unique items");
	eraser_leave_unique_chkbox->SetValue(g_settings.getBoolean(Config::ERASER_LEAVE_UNIQUE));
	eraser_leave_unique_chkbox->SetToolTip("The eraser will leave containers with items in them, items with unique or action id and items.");
	sizer->Add(eraser_leave_unique_chkbox, 0, wxLEFT | wxTOP, 5);

	auto_create_spawn_chkbox = newd wxCheckBox(editor_page, wxID_ANY, "Auto create spawn when placing creature");
	auto_create_spawn_chkbox->SetValue(g_settings.getBoolean(Config::AUTO_CREATE_SPAWN));
	auto_create_spawn_chkbox->SetToolTip("When this option is checked, you can place creatures without placing a spawn manually, the spawn will be place automatically.");
	sizer->Add(auto_create_spawn_chkbox, 0, wxLEFT | wxTOP, 5);

	allow_multiple_orderitems_chkbox = newd wxCheckBox(editor_page, wxID_ANY, "Prevent toporder conflict");
	allow_multiple_orderitems_chkbox->SetValue(g_settings.getBoolean(Config::RAW_LIKE_SIMONE));
	allow_multiple_orderitems_chkbox->SetToolTip("When this option is checked, you can not place several items with the same toporder on one tile using a RAW Brush.");
	sizer->Add(allow_multiple_orderitems_chkbox, 0, wxLEFT | wxTOP, 5);

	sizer->AddSpacer(10);

	merge_move_chkbox = newd wxCheckBox(editor_page, wxID_ANY, "Use merge move");
	merge_move_chkbox->SetValue(g_settings.getBoolean(Config::MERGE_MOVE));
	merge_move_chkbox->SetToolTip("Moved tiles won't replace already placed tiles.");
	sizer->Add(merge_move_chkbox, 0, wxLEFT | wxTOP, 5);

	merge_paste_chkbox = newd wxCheckBox(editor_page, wxID_ANY, "Use merge paste");
	merge_paste_chkbox->SetValue(g_settings.getBoolean(Config::MERGE_PASTE));
	merge_paste_chkbox->SetToolTip("Pasted tiles won't replace already placed tiles.");
	sizer->Add(merge_paste_chkbox, 0, wxLEFT | wxTOP, 5);

	editor_page->SetSizerAndFit(sizer);

	return editor_page;
}

wxNotebookPage* PreferencesWindow::CreateGraphicsPage()
{
	wxWindow* tmp;
	wxNotebookPage* graphics_page = newd wxPanel(book, wxID_ANY);

	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);

	hide_items_when_zoomed_chkbox = newd wxCheckBox(graphics_page, wxID_ANY, "Hide items when zoomed out");
	hide_items_when_zoomed_chkbox->SetValue(g_settings.getBoolean(Config::HIDE_ITEMS_WHEN_ZOOMED));
	sizer->Add(hide_items_when_zoomed_chkbox, 0, wxLEFT | wxTOP, 5);
	SetWindowToolTip(hide_items_when_zoomed_chkbox, "When this option is checked, \"loose\" items will be hidden when you zoom very far out.");

	icon_selection_shadow_chkbox = newd wxCheckBox(graphics_page, wxID_ANY, "Use icon selection shadow");
	icon_selection_shadow_chkbox->SetValue(g_settings.getBoolean(Config::USE_GUI_SELECTION_SHADOW));
	sizer->Add(icon_selection_shadow_chkbox, 0, wxLEFT | wxTOP, 5);
	SetWindowToolTip(icon_selection_shadow_chkbox, "When this option is checked, selected items in the palette menu will be shaded.");

	use_memcached_chkbox = newd wxCheckBox(graphics_page, wxID_ANY, "Use memcached sprites");
	use_memcached_chkbox->SetValue(g_settings.getBoolean(Config::USE_MEMCACHED_SPRITES));
	sizer->Add(use_memcached_chkbox, 0, wxLEFT | wxTOP, 5);
	SetWindowToolTip(use_memcached_chkbox, "When this is checked, sprites will be loaded into memory at startup and unpacked at runtime. This is faster but consumes more memory.\nIf it is not checked, the editor will use less memory but there will be a performance decrease due to reading sprites from the disk.");

	sizer->AddSpacer(10);

    auto * subsizer = newd wxFlexGridSizer(2, 10, 10);
	subsizer->AddGrowableCol(1);

	// Icon background color
	icon_background_choice = newd wxChoice(graphics_page, wxID_ANY);
	icon_background_choice->Append("Black background");
	icon_background_choice->Append("Gray background");
	icon_background_choice->Append("White background");
	if(g_settings.getInteger(Config::ICON_BACKGROUND) == 255) {
		icon_background_choice->SetSelection(2);
	} else if(g_settings.getInteger(Config::ICON_BACKGROUND) == 88) {
		icon_background_choice->SetSelection(1);
	} else {
		icon_background_choice->SetSelection(0);
	}

	subsizer->Add(tmp = newd wxStaticText(graphics_page, wxID_ANY, "Icon background color: "), 0);
	subsizer->Add(icon_background_choice, 0);
	SetWindowToolTip(icon_background_choice, tmp, "This will change the background color on icons in all windows.");

	// Cursor colors
	subsizer->Add(tmp = newd wxStaticText(graphics_page, wxID_ANY, "Cursor color: "), 0);
	subsizer->Add(cursor_color_pick = newd wxColourPickerCtrl(graphics_page, wxID_ANY, wxColor(
		g_settings.getInteger(Config::CURSOR_RED),
		g_settings.getInteger(Config::CURSOR_GREEN),
		g_settings.getInteger(Config::CURSOR_BLUE),
		g_settings.getInteger(Config::CURSOR_ALPHA)
		)), 0);
	SetWindowToolTip(icon_background_choice, tmp, "The color of the main cursor on the map (while in drawing mode).");

	// Alternate cursor color
	subsizer->Add(tmp = newd wxStaticText(graphics_page, wxID_ANY, "Secondary cursor color: "), 0);
	subsizer->Add(cursor_alt_color_pick = newd wxColourPickerCtrl(graphics_page, wxID_ANY, wxColor(
		g_settings.getInteger(Config::CURSOR_ALT_RED),
		g_settings.getInteger(Config::CURSOR_ALT_GREEN),
		g_settings.getInteger(Config::CURSOR_ALT_BLUE),
		g_settings.getInteger(Config::CURSOR_ALT_ALPHA)
		)), 0);
	SetWindowToolTip(icon_background_choice, tmp, "The color of the secondary cursor on the map (for houses and flags).");

	// Screenshot dir
	subsizer->Add(tmp = newd wxStaticText(graphics_page, wxID_ANY, "Screenshot directory: "), 0);
	screenshot_directory_picker = newd wxDirPickerCtrl(graphics_page, wxID_ANY);
	subsizer->Add(screenshot_directory_picker, 1, wxEXPAND);
	wxString ss = wxstr(g_settings.getString(Config::SCREENSHOT_DIRECTORY));
	screenshot_directory_picker->SetPath(ss);
	SetWindowToolTip(screenshot_directory_picker, "Screenshot taken in the editor will be saved to this directory.");

	// Screenshot format
	screenshot_format_choice = newd wxChoice(graphics_page, wxID_ANY);
	screenshot_format_choice->Append("PNG");
	screenshot_format_choice->Append("JPG");
	screenshot_format_choice->Append("TGA");
	screenshot_format_choice->Append("BMP");
	if(g_settings.getString(Config::SCREENSHOT_FORMAT) == "png") {
		screenshot_format_choice->SetSelection(0);
	} else if(g_settings.getString(Config::SCREENSHOT_FORMAT) == "jpg") {
		screenshot_format_choice->SetSelection(1);
	} else if(g_settings.getString(Config::SCREENSHOT_FORMAT) == "tga") {
		screenshot_format_choice->SetSelection(2);
	} else if(g_settings.getString(Config::SCREENSHOT_FORMAT) == "bmp") {
		screenshot_format_choice->SetSelection(3);
	} else {
		screenshot_format_choice->SetSelection(0);
	}
	subsizer->Add(tmp = newd wxStaticText(graphics_page, wxID_ANY, "Screenshot format: "), 0);
	subsizer->Add(screenshot_format_choice, 0);
	SetWindowToolTip(screenshot_format_choice, tmp, "This will affect the screenshot format used by the editor.\nTo take a screenshot, press F11.");

	sizer->Add(subsizer, 1, wxEXPAND | wxALL, 5);

	// Advanced g_settings
	/*
	wxCollapsiblePane* pane = newd wxCollapsiblePane(graphics_page, PANE_ADVANCED_GRAPHICS, "Advanced g_settings");
	{
		wxSizer* pane_sizer = newd wxBoxSizer(wxVERTICAL);

		pane_sizer->Add(texture_managment_chkbox = newd wxCheckBox(pane->GetPane(), wxID_ANY, "Use texture managment"));
		if(g_settings.getInteger(Config::TEXTURE_MANAGEMENT)) {
			texture_managment_chkbox->SetValue(true);
		}
		pane_sizer->AddSpacer(8);

		wxFlexGridSizer* pane_grid_sizer = newd wxFlexGridSizer(2, 10, 10);
		pane_grid_sizer->AddGrowableCol(1);

		pane_grid_sizer->Add(tmp = newd wxStaticText(pane->GetPane(), wxID_ANY, "Texture clean interval: "), 0);
		clean_interval_spin = newd wxSpinCtrl(pane->GetPane(), wxID_ANY, i2ws(g_settings.getInteger(Config::TEXTURE_CLEAN_PULSE)), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0x1000000);
		pane_grid_sizer->Add(clean_interval_spin, 0);
		SetWindowToolTip(clean_interval_spin, tmp, "This controls how often the editor tries to free hardware texture resources.");

		pane_grid_sizer->Add(tmp = newd wxStaticText(pane->GetPane(), wxID_ANY, "Texture longevity: "), 0);
		texture_longevity_spin = newd wxSpinCtrl(pane->GetPane(), wxID_ANY, i2ws(g_settings.getInteger(Config::TEXTURE_LONGEVITY)), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 0, 0x1000000);
		pane_grid_sizer->Add(texture_longevity_spin, 0);
		SetWindowToolTip(texture_longevity_spin, tmp, "This controls for how long (in seconds) that the editor will keep textures in memory before it cleans them up.");

		pane_grid_sizer->Add(tmp = newd wxStaticText(pane->GetPane(), wxID_ANY, "Texture clean threshold: "), 0);
		texture_threshold_spin = newd wxSpinCtrl(pane->GetPane(), wxID_ANY, i2ws(g_settings.getInteger(Config::TEXTURE_CLEAN_THRESHOLD)), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 100, 0x1000000);
		pane_grid_sizer->Add(texture_threshold_spin, 0);
		SetWindowToolTip(texture_threshold_spin, tmp, "This controls how many textures the editor will hold in memory before it attempts to clean up old textures. However, an infinite amount MIGHT be loaded.");

		pane_grid_sizer->Add(tmp = newd wxStaticText(pane->GetPane(), wxID_ANY, "Software clean threshold: "), 0);
		software_threshold_spin = newd wxSpinCtrl(pane->GetPane(), wxID_ANY, i2ws(g_settings.getInteger(Config::SOFTWARE_CLEAN_THRESHOLD)), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 100, 0x1000000);
		pane_grid_sizer->Add(software_threshold_spin, 0);
		SetWindowToolTip(software_threshold_spin, tmp, "This controls how many GUI sprites (icons) the editor will hold in memory at the same time.");

		pane_grid_sizer->Add(tmp = newd wxStaticText(pane->GetPane(), wxID_ANY, "Software clean amount: "), 0);
		software_clean_amount_spin = newd wxSpinCtrl(pane->GetPane(), wxID_ANY, i2ws(g_settings.getInteger(Config::SOFTWARE_CLEAN_SIZE)), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 0x1000000);
		pane_grid_sizer->Add(software_clean_amount_spin, 0);
		SetWindowToolTip(software_clean_amount_spin, tmp, "How many sprites the editor will free at once when the limit is exceeded.");

		pane_sizer->Add(pane_grid_sizer, 0, wxEXPAND);

		pane->GetPane()->SetSizerAndFit(pane_sizer);

		pane->Collapse();
	}

	sizer->Add(pane, 0);
	*/

	graphics_page->SetSizerAndFit(sizer);

	return graphics_page;
}

wxChoice* PreferencesWindow::AddPaletteStyleChoice(wxWindow* parent, wxSizer* sizer, const wxString& short_description, const wxString& description, const std::string& setting)
{
	wxStaticText* text;
	sizer->Add(text = newd wxStaticText(parent, wxID_ANY, short_description), 0);

	wxChoice* choice = newd wxChoice(parent, wxID_ANY);
	sizer->Add(choice, 0);

	choice->Append("Large Icons");
	choice->Append("Small Icons");
	choice->Append("Listbox with Icons");

	text->SetToolTip(description);
	choice->SetToolTip(description);

	if(setting == "large icons") {
		choice->SetSelection(0);
	} else if(setting == "small icons") {
		choice->SetSelection(1);
	} else if(setting == "listbox") {
		choice->SetSelection(2);
	}

	return choice;
}

void PreferencesWindow::SetPaletteStyleChoice(wxChoice* ctrl, int key)
{
	if(ctrl->GetSelection() == 0) {
		g_settings.setString(key, "large icons");
	} else if(ctrl->GetSelection() == 1) {
		g_settings.setString(key, "small icons");
	} else if(ctrl->GetSelection() == 2) {
		g_settings.setString(key, "listbox");
	}
}

wxNotebookPage* PreferencesWindow::CreateUIPage()
{
	wxNotebookPage* ui_page = newd wxPanel(book, wxID_ANY);

	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);

    auto * subsizer = newd wxFlexGridSizer(2, 10, 10);
	subsizer->AddGrowableCol(1);
	terrain_palette_style_choice = AddPaletteStyleChoice(
		ui_page, subsizer,
		"Terrain Palette Style:",
		"Configures the look of the terrain palette.",
		g_settings.getString(Config::PALETTE_TERRAIN_STYLE));
	doodad_palette_style_choice = AddPaletteStyleChoice(
		ui_page, subsizer,
		"Doodad Palette Style:",
		"Configures the look of the doodad palette.",
		g_settings.getString(Config::PALETTE_DOODAD_STYLE));
	item_palette_style_choice = AddPaletteStyleChoice(
		ui_page, subsizer,
		"Item Palette Style:",
		"Configures the look of the item palette.",
		g_settings.getString(Config::PALETTE_ITEM_STYLE));
	raw_palette_style_choice = AddPaletteStyleChoice(
		ui_page, subsizer,
		"RAW Palette Style:",
		"Configures the look of the raw palette.",
		g_settings.getString(Config::PALETTE_RAW_STYLE));

	sizer->Add(subsizer, 0, wxALL, 5);

	sizer->AddSpacer(10);

	large_terrain_tools_chkbox = newd wxCheckBox(ui_page, wxID_ANY, "Use large terrain palette tool & size icons");
	large_terrain_tools_chkbox->SetValue(g_settings.getBoolean(Config::USE_LARGE_TERRAIN_TOOLBAR));
	sizer->Add(large_terrain_tools_chkbox, 0, wxLEFT | wxTOP, 5);

	large_doodad_sizebar_chkbox = newd wxCheckBox(ui_page, wxID_ANY, "Use large doodad size palette icons");
	large_doodad_sizebar_chkbox->SetValue(g_settings.getBoolean(Config::USE_LARGE_DOODAD_SIZEBAR));
	sizer->Add(large_doodad_sizebar_chkbox, 0, wxLEFT | wxTOP, 5);

	large_item_sizebar_chkbox = newd wxCheckBox(ui_page, wxID_ANY, "Use large item size palette icons");
	large_item_sizebar_chkbox->SetValue(g_settings.getBoolean(Config::USE_LARGE_ITEM_SIZEBAR));
	sizer->Add(large_item_sizebar_chkbox, 0, wxLEFT | wxTOP, 5);

	large_house_sizebar_chkbox = newd wxCheckBox(ui_page, wxID_ANY, "Use large house palette size icons");
	large_house_sizebar_chkbox->SetValue(g_settings.getBoolean(Config::USE_LARGE_HOUSE_SIZEBAR));
	sizer->Add(large_house_sizebar_chkbox, 0, wxLEFT | wxTOP, 5);

	large_raw_sizebar_chkbox = newd wxCheckBox(ui_page, wxID_ANY, "Use large raw palette size icons");
	large_raw_sizebar_chkbox->SetValue(g_settings.getBoolean(Config::USE_LARGE_RAW_SIZEBAR));
	sizer->Add(large_raw_sizebar_chkbox, 0, wxLEFT | wxTOP, 5);

	large_container_icons_chkbox = newd wxCheckBox(ui_page, wxID_ANY, "Use large container view icons");
	large_container_icons_chkbox->SetValue(g_settings.getBoolean(Config::USE_LARGE_CONTAINER_ICONS));
	sizer->Add(large_container_icons_chkbox, 0, wxLEFT | wxTOP, 5);

	large_pick_item_icons_chkbox = newd wxCheckBox(ui_page, wxID_ANY, "Use large item picker icons");
	large_pick_item_icons_chkbox->SetValue(g_settings.getBoolean(Config::USE_LARGE_CHOOSE_ITEM_ICONS));
	sizer->Add(large_pick_item_icons_chkbox, 0, wxLEFT | wxTOP, 5);

	sizer->AddSpacer(10);

	switch_mousebtn_chkbox = newd wxCheckBox(ui_page, wxID_ANY, "Switch mousebuttons");
	switch_mousebtn_chkbox->SetValue(g_settings.getBoolean(Config::SWITCH_MOUSEBUTTONS));
	switch_mousebtn_chkbox->SetToolTip("Switches the right and center mouse button.");
	sizer->Add(switch_mousebtn_chkbox, 0, wxLEFT | wxTOP, 5);

	doubleclick_properties_chkbox = newd wxCheckBox(ui_page, wxID_ANY, "Double click for properties");
	doubleclick_properties_chkbox->SetValue(g_settings.getBoolean(Config::DOUBLECLICK_PROPERTIES));
	doubleclick_properties_chkbox->SetToolTip("Double clicking on a tile will bring up the properties menu for the top item.");
	sizer->Add(doubleclick_properties_chkbox, 0, wxLEFT | wxTOP, 5);

	inversed_scroll_chkbox = newd wxCheckBox(ui_page, wxID_ANY, "Use inversed scroll");
	inversed_scroll_chkbox->SetValue(g_settings.getFloat(Config::SCROLL_SPEED) < 0);
	inversed_scroll_chkbox->SetToolTip("When this checkbox is checked, dragging the map using the center mouse button will be inversed (default RTS behaviour).");
	sizer->Add(inversed_scroll_chkbox, 0, wxLEFT | wxTOP, 5);

	sizer->AddSpacer(10);

	sizer->Add(newd wxStaticText(ui_page, wxID_ANY, "Scroll speed: "), 0, wxLEFT | wxTOP, 5);

    auto true_scrollspeed = int(std::abs(g_settings.getFloat(Config::SCROLL_SPEED)) * 10);
	scroll_speed_slider = newd wxSlider(ui_page, wxID_ANY, true_scrollspeed, 1, max(true_scrollspeed, 100));
	scroll_speed_slider->SetToolTip("This controls how fast the map will scroll when you hold down the center mouse button and move it around.");
	sizer->Add(scroll_speed_slider, 0, wxEXPAND, 5);

	sizer->Add(newd wxStaticText(ui_page, wxID_ANY, "Zoom speed: "), 0, wxLEFT | wxTOP, 5);

    auto true_zoomspeed = int(g_settings.getFloat(Config::ZOOM_SPEED) * 10);
	zoom_speed_slider = newd wxSlider(ui_page, wxID_ANY, true_zoomspeed, 1, max(true_zoomspeed, 100));
	zoom_speed_slider->SetToolTip("This controls how fast you will zoom when you scroll the center mouse button.");
	sizer->Add(zoom_speed_slider, 0, wxEXPAND, 5);

	ui_page->SetSizerAndFit(sizer);

	return ui_page;
}

wxNotebookPage* PreferencesWindow::CreateClientPage()
{
	wxNotebookPage* client_page = newd wxPanel(book, wxID_ANY);

	// Refresh g_settings
	ClientVersion::saveVersions();
	ClientVersionList versions = ClientVersion::getAllVisible();

	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);

    auto * options_sizer = newd wxFlexGridSizer(2, 10, 10);
	options_sizer->AddGrowableCol(1);

	// Default client version choice control
	default_version_choice = newd wxChoice(client_page, wxID_ANY);
	wxStaticText* default_client_tooltip = newd wxStaticText(client_page, wxID_ANY, "Default client version:");
	options_sizer->Add(default_client_tooltip, 0, wxLEFT | wxTOP, 5);
	options_sizer->Add(default_version_choice, 0, wxTOP, 5);
	SetWindowToolTip(default_client_tooltip, default_version_choice, "This will decide what client version will be used when new maps are created.");

	// Check file sigs checkbox
	check_sigs_chkbox = newd wxCheckBox(client_page, wxID_ANY, "Check file signatures");
	check_sigs_chkbox->SetValue(g_settings.getBoolean(Config::CHECK_SIGNATURES));
	check_sigs_chkbox->SetToolTip("When this option is not checked, the editor will load any OTB/DAT/SPR combination without complaints. This may cause graphics bugs.");
	options_sizer->Add(check_sigs_chkbox, 0, wxLEFT | wxRIGHT | wxTOP, 5);

	// Add the grid sizer
	topsizer->Add(options_sizer, wxSizerFlags(0).Expand());
	topsizer->AddSpacer(10);

	wxScrolledWindow *client_list_window = newd wxScrolledWindow(client_page, wxID_ANY, wxDefaultPosition, wxDefaultSize);
	client_list_window->SetMinSize(FromDIP(wxSize(450, 450)));
    auto * client_list_sizer = newd wxFlexGridSizer(2, 10, 10);
	client_list_sizer->AddGrowableCol(1);

    int version_counter = 0;
	for (auto version : versions) {
        if(!version->isVisible())
			continue;

		default_version_choice->Append(wxstr(version->getName()));

		wxStaticText *tmp_text = newd wxStaticText(client_list_window, wxID_ANY, wxString(version->getName()));
		client_list_sizer->Add(tmp_text, wxSizerFlags(0).Expand());

		wxDirPickerCtrl* dir_picker = newd wxDirPickerCtrl(client_list_window, wxID_ANY, version->getClientPath().GetFullPath());
		version_dir_pickers.push_back(dir_picker);
		client_list_sizer->Add(dir_picker, wxSizerFlags(0).Border(wxRIGHT, 10).Expand());

		wxString tooltip;
		tooltip << "The editor will look for " << wxstr(version->getName()) << " DAT & SPR here.";
		tmp_text->SetToolTip(tooltip);
		dir_picker->SetToolTip(tooltip);

		if(version->getID() == g_settings.getInteger(Config::DEFAULT_CLIENT_VERSION))
			default_version_choice->SetSelection(version_counter);

		version_counter++;
	}

	// Set the sizers
	client_list_window->SetSizer(client_list_sizer);
	client_list_window->FitInside();
	client_list_window->SetScrollRate(5, 5);
	topsizer->Add(client_list_window, 0, wxALL, 5);
	client_page->SetSizerAndFit(topsizer);

	return client_page;
}

// Event handlers!

void PreferencesWindow::OnClickOK(wxCommandEvent& WXUNUSED(event))
{
	Apply();
	EndModal(0);
}

void PreferencesWindow::OnClickCancel(wxCommandEvent& WXUNUSED(event))
{
	EndModal(0);
}

void PreferencesWindow::OnClickApply(wxCommandEvent& WXUNUSED(event))
{
	Apply();
}

void PreferencesWindow::OnCollapsiblePane(wxCollapsiblePaneEvent& event)
{
    auto * win = (wxWindow*)event.GetEventObject();
	win->GetParent()->Fit();
}

// Stuff

void PreferencesWindow::Apply()
{
	bool must_restart = false;
	// General
	g_settings.setInteger(Config::WELCOME_DIALOG, show_welcome_dialog_chkbox->GetValue());
	g_settings.setInteger(Config::ALWAYS_MAKE_BACKUP, always_make_backup_chkbox->GetValue());
	g_settings.setInteger(Config::USE_UPDATER, update_check_on_startup_chkbox->GetValue());
	g_settings.setInteger(Config::ONLY_ONE_INSTANCE, only_one_instance_chkbox->GetValue());
	g_settings.setInteger(Config::UNDO_SIZE, undo_size_spin->GetValue());
	g_settings.setInteger(Config::UNDO_MEM_SIZE, undo_mem_size_spin->GetValue());
	g_settings.setInteger(Config::WORKER_THREADS, worker_threads_spin->GetValue());
	g_settings.setInteger(Config::REPLACE_SIZE, replace_size_spin->GetValue());
	g_settings.setInteger(Config::COPY_POSITION_FORMAT, position_format->GetSelection());

	// Editor
	g_settings.setInteger(Config::GROUP_ACTIONS, group_actions_chkbox->GetValue());
	g_settings.setInteger(Config::WARN_FOR_DUPLICATE_ID, duplicate_id_warn_chkbox->GetValue());
	g_settings.setInteger(Config::HOUSE_BRUSH_REMOVE_ITEMS, house_remove_chkbox->GetValue());
	g_settings.setInteger(Config::AUTO_ASSIGN_DOORID, auto_assign_doors_chkbox->GetValue());
	g_settings.setInteger(Config::ERASER_LEAVE_UNIQUE, eraser_leave_unique_chkbox->GetValue());
	g_settings.setInteger(Config::DOODAD_BRUSH_ERASE_LIKE, doodad_erase_same_chkbox->GetValue());
	g_settings.setInteger(Config::AUTO_CREATE_SPAWN, auto_create_spawn_chkbox->GetValue());
	g_settings.setInteger(Config::RAW_LIKE_SIMONE, allow_multiple_orderitems_chkbox->GetValue());
	g_settings.setInteger(Config::MERGE_MOVE, merge_move_chkbox->GetValue());
	g_settings.setInteger(Config::MERGE_PASTE, merge_paste_chkbox->GetValue());

	// Graphics
	g_settings.setInteger(Config::USE_GUI_SELECTION_SHADOW, icon_selection_shadow_chkbox->GetValue());
	if(g_settings.getBoolean(Config::USE_MEMCACHED_SPRITES) != use_memcached_chkbox->GetValue()) {
		must_restart = true;
	}
	g_settings.setInteger(Config::USE_MEMCACHED_SPRITES_TO_SAVE, use_memcached_chkbox->GetValue());
	if(icon_background_choice->GetSelection() == 0) {
		if(g_settings.getInteger(Config::ICON_BACKGROUND) != 0) {
			g_gui.gfx.cleanSoftwareSprites();
		}
		g_settings.setInteger(Config::ICON_BACKGROUND, 0);
	} else if(icon_background_choice->GetSelection() == 1) {
		if(g_settings.getInteger(Config::ICON_BACKGROUND) != 88) {
			g_gui.gfx.cleanSoftwareSprites();
		}
		g_settings.setInteger(Config::ICON_BACKGROUND, 88);
	} else if(icon_background_choice->GetSelection() == 2) {
		if(g_settings.getInteger(Config::ICON_BACKGROUND) != 255) {
			g_gui.gfx.cleanSoftwareSprites();
		}
		g_settings.setInteger(Config::ICON_BACKGROUND, 255);
	}

	// Screenshots
	g_settings.setString(Config::SCREENSHOT_DIRECTORY, nstr(screenshot_directory_picker->GetPath()));

	std::string new_format = nstr(screenshot_format_choice->GetStringSelection());
	if(new_format == "PNG") {
		g_settings.setString(Config::SCREENSHOT_FORMAT, "png");
	} else if(new_format == "TGA") {
		g_settings.setString(Config::SCREENSHOT_FORMAT, "tga");
	} else if(new_format == "JPG") {
		g_settings.setString(Config::SCREENSHOT_FORMAT, "jpg");
	} else if(new_format == "BMP") {
		g_settings.setString(Config::SCREENSHOT_FORMAT, "bmp");
	}

	wxColor clr = cursor_color_pick->GetColour();
		g_settings.setInteger(Config::CURSOR_RED, clr.Red());
		g_settings.setInteger(Config::CURSOR_GREEN, clr.Green());
		g_settings.setInteger(Config::CURSOR_BLUE, clr.Blue());
		//g_settings.setInteger(Config::CURSOR_ALPHA, clr.Alpha());

	clr = cursor_alt_color_pick->GetColour();
		g_settings.setInteger(Config::CURSOR_ALT_RED, clr.Red());
		g_settings.setInteger(Config::CURSOR_ALT_GREEN, clr.Green());
		g_settings.setInteger(Config::CURSOR_ALT_BLUE, clr.Blue());
		//g_settings.setInteger(Config::CURSOR_ALT_ALPHA, clr.Alpha());

	g_settings.setInteger(Config::HIDE_ITEMS_WHEN_ZOOMED, hide_items_when_zoomed_chkbox->GetValue());
	/*
	g_settings.setInteger(Config::TEXTURE_MANAGEMENT, texture_managment_chkbox->GetValue());
	g_settings.setInteger(Config::TEXTURE_CLEAN_PULSE, clean_interval_spin->GetValue());
	g_settings.setInteger(Config::TEXTURE_LONGEVITY, texture_longevity_spin->GetValue());
	g_settings.setInteger(Config::TEXTURE_CLEAN_THRESHOLD, texture_threshold_spin->GetValue());
	g_settings.setInteger(Config::SOFTWARE_CLEAN_THRESHOLD, software_threshold_spin->GetValue());
	g_settings.setInteger(Config::SOFTWARE_CLEAN_SIZE, software_clean_amount_spin->GetValue());
	*/

	// Interface
	SetPaletteStyleChoice(terrain_palette_style_choice, Config::PALETTE_TERRAIN_STYLE);
	SetPaletteStyleChoice(doodad_palette_style_choice, Config::PALETTE_DOODAD_STYLE);
	SetPaletteStyleChoice(item_palette_style_choice, Config::PALETTE_ITEM_STYLE);
	SetPaletteStyleChoice(raw_palette_style_choice, Config::PALETTE_RAW_STYLE);
	g_settings.setInteger(Config::USE_LARGE_TERRAIN_TOOLBAR, large_terrain_tools_chkbox->GetValue());
	g_settings.setInteger(Config::USE_LARGE_DOODAD_SIZEBAR, large_doodad_sizebar_chkbox->GetValue());
	g_settings.setInteger(Config::USE_LARGE_ITEM_SIZEBAR, large_item_sizebar_chkbox->GetValue());
	g_settings.setInteger(Config::USE_LARGE_HOUSE_SIZEBAR, large_house_sizebar_chkbox->GetValue());
	g_settings.setInteger(Config::USE_LARGE_RAW_SIZEBAR, large_raw_sizebar_chkbox->GetValue());
	g_settings.setInteger(Config::USE_LARGE_CONTAINER_ICONS, large_container_icons_chkbox->GetValue());
	g_settings.setInteger(Config::USE_LARGE_CHOOSE_ITEM_ICONS, large_pick_item_icons_chkbox->GetValue());


	g_settings.setInteger(Config::SWITCH_MOUSEBUTTONS, switch_mousebtn_chkbox->GetValue());
	g_settings.setInteger(Config::DOUBLECLICK_PROPERTIES, doubleclick_properties_chkbox->GetValue());

	float scroll_mul = 1.0;
	if(inversed_scroll_chkbox->GetValue()) {
		scroll_mul = -1.0;
	}
	g_settings.setFloat(Config::SCROLL_SPEED, scroll_mul * scroll_speed_slider->GetValue()/10.f);
	g_settings.setFloat(Config::ZOOM_SPEED, zoom_speed_slider->GetValue()/10.f);

	// Client
	ClientVersionList versions = ClientVersion::getAllVisible();
	int version_counter = 0;
	for (auto version : versions) {
        wxString dir = version_dir_pickers[version_counter]->GetPath();
		if(dir.Length() > 0 && dir.Last() != '/' && dir.Last() != '\\')
			dir.Append("/");
		version->setClientPath(FileName(dir));

		if(version->getName() == default_version_choice->GetStringSelection())
			g_settings.setInteger(Config::DEFAULT_CLIENT_VERSION, version->getID());

		version_counter++;
	}
	g_settings.setInteger(Config::CHECK_SIGNATURES, check_sigs_chkbox->GetValue());

	// Make sure to reload client paths
	ClientVersion::saveVersions();
	ClientVersion::loadVersions();

	g_settings.save();

	if(must_restart) {
		g_gui.PopupDialog(this, "Notice", "You must restart the editor for the changes to take effect.", wxOK);
	}
	g_gui.RebuildPalettes();
}
