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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/main_menubar.h $
// $Id: main_menubar.h 323 2010-04-12 20:59:41Z admin $


#ifndef RME_MAIN_BAR_H_
#define RME_MAIN_BAR_H_

#include <wx/docview.h>

namespace MenuBar
{
	struct Action;

	enum ActionID {
		NEW,
		OPEN,
		SAVE,
		SAVE_AS,
		GENERATE_MAP,
		CLOSE,
		IMPORT_MAP,
		IMPORT_MONSTERS,
		IMPORT_MINIMAP,
		EXPORT_MINIMAP,
		RELOAD_DATA,
		RECENT_FILES,
		PREFERENCES,
		EXIT,
		UNDO,
		REDO,
		FIND_ITEM,
		REPLACE_ITEM,
		SEARCH_ON_MAP_EVERYTHING,
		SEARCH_ON_MAP_UNIQUE,
		SEARCH_ON_MAP_ACTION,
		SEARCH_ON_MAP_CONTAINER,
		SEARCH_ON_MAP_WRITEABLE,
		SEARCH_ON_SELECTION_EVERYTHING,
		SEARCH_ON_SELECTION_UNIQUE,
		SEARCH_ON_SELECTION_ACTION,
		SEARCH_ON_SELECTION_CONTAINER,
		SEARCH_ON_SELECTION_WRITEABLE,
		SELECT_MODE_COMPENSATE,
		SELECT_MODE_CURRENT,
		SELECT_MODE_LOWER,
		SELECT_MODE_VISIBLE,
		AUTOMAGIC,
		BORDERIZE_SELECTION,
		BORDERIZE_MAP,
		RANDOMIZE_SELECTION,
		RANDOMIZE_MAP,
		GOTO_PREVIOUS_POSITION,
		GOTO_POSITION,
		JUMP_TO_BRUSH,
		JUMP_TO_ITEM_BRUSH,
		CLEAR_INVALID_HOUSES,
		CLEAR_MODIFIED_STATE,
		CUT,
		COPY,
		PASTE,
		EDIT_TOWNS,
		EDIT_ITEMS,
		EDIT_MONSTERS,
		MAP_CLEANUP,
		MAP_REMOVE_ITEMS,
		MAP_REMOVE_CORPSES,
		MAP_REMOVE_UNREACHABLE_TILES,
		MAP_CLEAN_HOUSE_ITEMS,
		MAP_PROPERTIES,
		MAP_STATISTICS,
		NEW_VIEW,
		TOGGLE_FULLSCREEN,
		ZOOM_IN,
		ZOOM_OUT,
		ZOOM_NORMAL,
		SHOW_SHADE,
		SHOW_ALL_FLOORS,
		GHOST_ITEMS,
		GHOST_HIGHER_FLOORS,
		HIGHLIGHT_ITEMS,
		SHOW_INGAME_BOX,
		SHOW_GRID,
		SHOW_EXTRA,
		SHOW_CREATURES,
		SHOW_SPAWNS,
		SHOW_SPECIAL,
		SHOW_ONLY_COLORS,
		SHOW_ONLY_MODIFIED,
		SHOW_HOUSES,
		SHOW_PATHING,
		SHOW_TOOLTIPS,
		SHOW_PREVIEW,
		SHOW_WALL_HOOKS,
		WIN_MINIMAP,
		NEW_PALETTE,
		TAKE_SCREENSHOT,
		LIVE_START,
		LIVE_JOIN,
		LIVE_CLOSE,
		SELECT_TERRAIN,
		SELECT_DOODAD,
		SELECT_ITEM,
		SELECT_CREATURE,
		SELECT_HOUSE,
		SELECT_WAYPOINT,
		SELECT_RAW,
		FLOOR_0,
		FLOOR_1,
		FLOOR_2,
		FLOOR_3,
		FLOOR_4,
		FLOOR_5,
		FLOOR_6,
		FLOOR_7,
		FLOOR_8,
		FLOOR_9,
		FLOOR_10,
		FLOOR_11,
		FLOOR_12,
		FLOOR_13,
		FLOOR_14,
		FLOOR_15,
		DEBUG_VIEW_DAT,
		EXTENSIONS,
		GOTO_WEBSITE,
		ABOUT,
	};
}

class MainFrame;

class MainMenuBar : wxEvtHandler
{
public:
	MainMenuBar(MainFrame* frame);
	virtual ~MainMenuBar();

	bool Load(const FileName&, wxArrayString& warnings, wxString& error);

	// Update
	// Turn on/off all buttons according to current editor state
	void Update();
	void UpdateFloorMenu(); // Only concerns the floor menu

	void AddRecentFile(FileName file);
	void LoadRecentFiles();
	void SaveRecentFiles();

	// Interface
	void EnableItem(MenuBar::ActionID id, bool enable);
	void CheckItem(MenuBar::ActionID id, bool enable);
	bool IsItemChecked(MenuBar::ActionID id) const;

	// Event handlers for all menu buttons
	// File Menu
	void OnNew(wxCommandEvent& event);
	void OnOpen(wxCommandEvent& event);
	void OnGenerateMap(wxCommandEvent& event);
	void OnOpenRecent(wxCommandEvent& event);
	void OnSave(wxCommandEvent& event);
	void OnSaveAs(wxCommandEvent& event);
	void OnClose(wxCommandEvent& event);
	void OnPreferences(wxCommandEvent& event);
	void OnQuit(wxCommandEvent& event);

	// Import Menu
	// Export Menu
	void OnImportMap(wxCommandEvent& event);
	void OnImportMonsterData(wxCommandEvent& event);
	void OnImportMinimap(wxCommandEvent& event);
	void OnExportMinimap(wxCommandEvent& event);
	void OnReloadDataFiles(wxCommandEvent& event);

	// Edit Menu
	void OnUndo(wxCommandEvent& event);
	void OnRedo(wxCommandEvent& event);
	void OnBorderizeSelection(wxCommandEvent& event);
	void OnBorderizeMap(wxCommandEvent& event);
	void OnRandomizeSelection(wxCommandEvent& event);
	void OnRandomizeMap(wxCommandEvent& event);
	void OnJumpToBrush(wxCommandEvent& event);
	void OnJumpToItemBrush(wxCommandEvent& event);
	void OnGotoPreviousPosition(wxCommandEvent& event);
	void OnGotoPosition(wxCommandEvent& event);
	void OnMapRemoveItems(wxCommandEvent& event);
	void OnMapRemoveCorpses(wxCommandEvent& event);
	void OnMapRemoveUnreachable(wxCommandEvent& event);
	void OnClearHouseTiles(wxCommandEvent& event);
	void OnClearModifiedState(wxCommandEvent& event);
	void OnToggleAutomagic(wxCommandEvent& event);
	void OnSelectionTypeChange(wxCommandEvent& event);
	void OnCut(wxCommandEvent& event);
	void OnCopy(wxCommandEvent& event);
	void OnPaste(wxCommandEvent& event);
	void OnSearchForItem(wxCommandEvent& event);
	void OnReplaceItem(wxCommandEvent& event);
	void OnSearchForStuffOnMap(wxCommandEvent& event);
	void OnSearchForUniqueOnMap(wxCommandEvent& event);
	void OnSearchForActionOnMap(wxCommandEvent& event);
	void OnSearchForContainerOnMap(wxCommandEvent& event);
	void OnSearchForWriteableOnMap(wxCommandEvent& event);
	void OnSearchForStuffOnSelection(wxCommandEvent& event);
	void OnSearchForUniqueOnSelection(wxCommandEvent& event);
	void OnSearchForActionOnSelection(wxCommandEvent& event);
	void OnSearchForContainerOnSelection(wxCommandEvent& event);
	void OnSearchForWriteableOnSelection(wxCommandEvent& event);

	// Map menu
	void OnMapEditTowns(wxCommandEvent& event);
	void OnMapEditItems(wxCommandEvent& event);
	void OnMapEditMonsters(wxCommandEvent& event);
	void OnMapCleanHouseItems(wxCommandEvent& event);
	void OnMapCleanup(wxCommandEvent& event);
	void OnMapProperties(wxCommandEvent& event);
	void OnMapStatistics(wxCommandEvent& event);

	// View Menu
	void OnNewView(wxCommandEvent& event);
	void OnToggleFullscreen(wxCommandEvent& event);
	void OnZoomIn(wxCommandEvent& event);
	void OnZoomOut(wxCommandEvent& event);
	void OnZoomNormal(wxCommandEvent& event);
	void OnChangeViewSettings(wxCommandEvent& event);

	// Network menu
	void OnStartLive(wxCommandEvent& event);
	void OnJoinLive(wxCommandEvent& event);
	void OnCloseLive(wxCommandEvent& event);

	// Window Menu
	void OnMinimapWindow(wxCommandEvent& event);
	void OnNewPalette(wxCommandEvent& event);
	void OnTakeScreenshot(wxCommandEvent& event);
	void OnSelectTerrainPalette(wxCommandEvent& event);
	void OnSelectDoodadPalette(wxCommandEvent& event);
	void OnSelectItemPalette(wxCommandEvent& event);
	void OnSelectHousePalette(wxCommandEvent& event);
	void OnSelectCreaturePalette(wxCommandEvent& event);
	void OnSelectWaypointPalette(wxCommandEvent& event);
	void OnSelectRawPalette(wxCommandEvent& event);

	// Floor menu
	void OnChangeFloor(wxCommandEvent& event);

	// About Menu
	void OnDebugViewDat(wxCommandEvent& event);
	void OnListExtensions(wxCommandEvent& event);
	void OnGotoWebsite(wxCommandEvent& event);
	void OnAbout(wxCommandEvent& event);

protected:
	// Load and returns a menu item, also sets accelerator
	wxObject* LoadItem(pugi::xml_node node, wxMenu* parent, wxArrayString& warnings, wxString& error);
	// Checks the items in the menus according to the settings (in config)
	void LoadValues();
	void SearchItems(bool unique, bool action, bool container, bool writable, bool onSelection = false);
protected:

	MainFrame* frame;
	wxMenuBar* menubar;

	// Used so that calling Check on menu items don't trigger events (avoids infinite recursion)
	bool checking_programmaticly;

	std::map<MenuBar::ActionID, std::list<wxMenuItem*> > items;

	// Hardcoded recent files
	wxFileHistory recentFiles;

	std::map<std::string, MenuBar::Action*> actions;

	DECLARE_EVENT_TABLE();
};

namespace MenuBar
{
	struct Action
	{
		Action() : id(0), kind(wxITEM_NORMAL) {}
		Action(std::string s, int id, wxItemKind kind, wxCommandEventFunction handler)
			: id(id), setting(0), name(s), kind(kind), handler(handler) {}

		int id;
		int setting;
		std::string name;
		wxItemKind kind;
		wxCommandEventFunction handler;
	};
}

#endif
