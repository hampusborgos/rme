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


#ifndef RME_PREFERENCES_WINDOW_H_
#define RME_PREFERENCES_WINDOW_H_

class PreferencesWindow : public wxDialog
{
public:
	explicit PreferencesWindow(wxWindow* parent);
	virtual ~PreferencesWindow();

	void OnClickDefaults(wxCommandEvent&);
	void OnClickApply(wxCommandEvent&);
	void OnClickOK(wxCommandEvent&);
	void OnClickCancel(wxCommandEvent&);

	void SelectNewAssetsFolder(wxCommandEvent& event);

	void OnCollapsiblePane(wxCollapsiblePaneEvent&);

	wxBookCtrl& getBookCtrl() {
		return *book;
	}

protected:
	void SetDefaults();
	void Apply();

	wxBookCtrl* book;

	// General
	wxCheckBox* always_make_backup_chkbox;
	wxCheckBox* create_on_startup_chkbox;
	wxCheckBox* update_check_on_startup_chkbox;
	wxCheckBox* only_one_instance_chkbox;
	wxCheckBox* show_welcome_dialog_chkbox;
	wxSpinCtrl* undo_size_spin;
	wxSpinCtrl* undo_mem_size_spin;
	wxSpinCtrl* worker_threads_spin;
	wxSpinCtrl* replace_size_spin;
	wxRadioBox* position_format;

	// Editor
	wxCheckBox* group_actions_chkbox;
	wxCheckBox* duplicate_id_warn_chkbox;
	wxCheckBox* house_remove_chkbox;
	wxCheckBox* auto_assign_doors_chkbox;
	wxCheckBox* eraser_leave_unique_chkbox;
	wxCheckBox* doodad_erase_same_chkbox;
	wxCheckBox* auto_create_spawn_chkbox;
	wxCheckBox* auto_create_spawn_npc_chkbox;
	wxCheckBox* allow_multiple_orderitems_chkbox;
	wxCheckBox* merge_move_chkbox;
	wxCheckBox* merge_paste_chkbox;

	// Graphics
	wxCheckBox* icon_selection_shadow_chkbox;
	wxChoice* icon_background_choice;
	wxDirPickerCtrl* screenshot_directory_picker;
	wxChoice* screenshot_format_choice;
	wxCheckBox* hide_items_when_zoomed_chkbox;
	wxColourPickerCtrl* cursor_color_pick;
	wxColourPickerCtrl* cursor_alt_color_pick;
	/*
	wxCheckBox* texture_managment_chkbox;
	wxSpinCtrl* clean_interval_spin;
	wxSpinCtrl* texture_longevity_spin;
	wxSpinCtrl* texture_threshold_spin;
	wxSpinCtrl* software_threshold_spin;
	wxSpinCtrl* software_clean_amount_spin;
	*/

	// Interface
	wxChoice* terrain_palette_style_choice;
	wxChoice* doodad_palette_style_choice;
	wxChoice* item_palette_style_choice;
	wxChoice* raw_palette_style_choice;

	wxCheckBox* large_terrain_tools_chkbox;
	wxCheckBox* large_doodad_sizebar_chkbox;
	wxCheckBox* large_item_sizebar_chkbox;
	wxCheckBox* large_house_sizebar_chkbox;
	wxCheckBox* large_raw_sizebar_chkbox;
	wxCheckBox* large_container_icons_chkbox;
	wxCheckBox* large_pick_item_icons_chkbox;

	wxCheckBox* switch_mousebtn_chkbox;
	wxCheckBox* doubleclick_properties_chkbox;
	wxCheckBox* inversed_scroll_chkbox;
	wxSlider* scroll_speed_slider;
	wxSlider* zoom_speed_slider;

	// Client info
	wxChoice* default_version_choice;
	wxDirPickerCtrl* version_dir_picker;
	wxCheckBox* check_sigs_chkbox;

	// Create controls
	wxChoice* AddPaletteStyleChoice(wxWindow* parent, wxSizer* sizer, const wxString& short_description, const wxString& description, const std::string& setting);
	void SetPaletteStyleChoice(wxChoice* ctrl, int key);

	// Create windows
	wxNotebookPage* CreateGeneralPage();
	wxNotebookPage* CreateGraphicsPage();
	wxNotebookPage* CreateUIPage();
	wxNotebookPage* CreateEditorPage();
	wxNotebookPage* CreateClientPage();

	DECLARE_EVENT_TABLE()
};

#endif
