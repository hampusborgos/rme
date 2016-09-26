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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/main_menubar.hpp $
// $Id: main_menubar.hpp 323 2010-04-12 20:59:41Z admin $

#include "main.h"

#include "main_menubar.h"
#include "application.h"
#include "preferences.h"
#include "about_window.h"
#include "minimap_window.h"
#include "dat_debug_view.h"
#include "result_window.h"
#include "extension_window.h"

#include "gui.h"

#include <wx/chartype.h>

#include "editor.h"
#include "materials.h"
#include "live_client.h"
#include "live_server.h"

#define MAP_LOAD_FILE_WILDCARD_OTGZ wxT("OpenTibia Binary Map (*.otbm;*.otgz)|*.otbm;*.otgz")
#define MAP_SAVE_FILE_WILDCARD_OTGZ wxT("OpenTibia Binary Map (*.otbm)|*.otbm|Compressed OpenTibia Binary Map (*.otgz)|*.otgz")

#define MAP_LOAD_FILE_WILDCARD wxT("OpenTibia Binary Map (*.otbm)|*.otbm")
#define MAP_SAVE_FILE_WILDCARD wxT("OpenTibia Binary Map (*.otbm)|*.otbm")

BEGIN_EVENT_TABLE(MainMenuBar, wxEvtHandler)
END_EVENT_TABLE()

MainMenuBar::MainMenuBar(MainFrame *frame) : frame(frame)
{
	using namespace MenuBar;
	checking_programmaticly = false;

#define MAKE_ACTION(id, kind, handler) actions[#id] = new MenuBar::Action(#id, id, kind, wxCommandEventFunction(&MainMenuBar::handler))
#define MAKE_SET_ACTION(id, kind, setting_, handler) actions[#id] = new MenuBar::Action(#id, id, kind, wxCommandEventFunction(&MainMenuBar::handler)); actions[#id].setting = setting_

	MAKE_ACTION(NEW, wxITEM_NORMAL, OnNew);
	MAKE_ACTION(OPEN, wxITEM_NORMAL, OnOpen);
	MAKE_ACTION(SAVE, wxITEM_NORMAL, OnSave);
	MAKE_ACTION(SAVE_AS, wxITEM_NORMAL, OnSaveAs);
	MAKE_ACTION(GENERATE_MAP, wxITEM_NORMAL, OnGenerateMap);
	MAKE_ACTION(CLOSE, wxITEM_NORMAL, OnClose);

	MAKE_ACTION(IMPORT_MAP, wxITEM_NORMAL, OnImportMap);
	MAKE_ACTION(IMPORT_MONSTERS, wxITEM_NORMAL, OnImportMonsterData);
	MAKE_ACTION(IMPORT_MINIMAP, wxITEM_NORMAL, OnImportMinimap);
	MAKE_ACTION(EXPORT_MINIMAP, wxITEM_NORMAL, OnExportMinimap);

	MAKE_ACTION(RELOAD_DATA, wxITEM_NORMAL, OnReloadDataFiles);
	//MAKE_ACTION(RECENT_FILES, wxITEM_NORMAL, OnRecent);
	MAKE_ACTION(PREFERENCES, wxITEM_NORMAL, OnPreferences);
	MAKE_ACTION(EXIT, wxITEM_NORMAL, OnQuit);

	MAKE_ACTION(UNDO, wxITEM_NORMAL, OnUndo);
	MAKE_ACTION(REDO, wxITEM_NORMAL, OnRedo);

	MAKE_ACTION(FIND_ITEM, wxITEM_NORMAL, OnSearchForItem);
	MAKE_ACTION(REPLACE_ITEM, wxITEM_NORMAL, OnReplaceItem);
	MAKE_ACTION(SEARCH_EVERYTHING, wxITEM_NORMAL, OnSearchForStuff);
	MAKE_ACTION(SEARCH_UNIQUE, wxITEM_NORMAL, OnSearchForUnique);
	MAKE_ACTION(SEARCH_ACTION, wxITEM_NORMAL, OnSearchForAction);
	MAKE_ACTION(SEARCH_CONTAINER, wxITEM_NORMAL, OnSearchForContainer);
	MAKE_ACTION(SEARCH_WRITEABLE, wxITEM_NORMAL, OnSearchForWriteable);
	MAKE_ACTION(SELECT_MODE_COMPENSATE, wxITEM_RADIO, OnSelectionTypeChange);
	MAKE_ACTION(SELECT_MODE_LOWER, wxITEM_RADIO, OnSelectionTypeChange);
	MAKE_ACTION(SELECT_MODE_CURRENT, wxITEM_RADIO, OnSelectionTypeChange);
	MAKE_ACTION(SELECT_MODE_VISIBLE, wxITEM_RADIO, OnSelectionTypeChange);

	MAKE_ACTION(AUTOMAGIC, wxITEM_CHECK, OnToggleAutomagic);
	MAKE_ACTION(BORDERIZE_SELECTION, wxITEM_NORMAL, OnBorderizeSelection);
	MAKE_ACTION(BORDERIZE_MAP, wxITEM_NORMAL, OnBorderizeMap);
	MAKE_ACTION(RANDOMIZE_SELECTION, wxITEM_NORMAL, OnRandomizeSelection);
	MAKE_ACTION(RANDOMIZE_MAP, wxITEM_NORMAL, OnRandomizeMap);
	MAKE_ACTION(GOTO_POSITION, wxITEM_NORMAL, OnGotoPosition);
	MAKE_ACTION(JUMP_TO_BRUSH, wxITEM_NORMAL, OnJumpToBrush);
	MAKE_ACTION(JUMP_TO_ITEM_BRUSH, wxITEM_NORMAL, OnJumpToItemBrush);

	MAKE_ACTION(CUT, wxITEM_NORMAL, OnCut);
	MAKE_ACTION(COPY, wxITEM_NORMAL, OnCopy);
	MAKE_ACTION(PASTE, wxITEM_NORMAL, OnPaste);

	MAKE_ACTION(EDIT_TOWNS, wxITEM_NORMAL, OnMapEditTowns);
	MAKE_ACTION(EDIT_ITEMS, wxITEM_NORMAL, OnMapEditItems);
	MAKE_ACTION(EDIT_MONSTERS, wxITEM_NORMAL, OnMapEditMonsters);

	MAKE_ACTION(CLEAR_INVALID_HOUSES, wxITEM_NORMAL, OnClearHouseTiles);
	MAKE_ACTION(CLEAR_MODIFIED_STATE, wxITEM_NORMAL, OnClearModifiedState);
	MAKE_ACTION(MAP_REMOVE_ITEMS, wxITEM_NORMAL, OnMapRemoveItems);
	MAKE_ACTION(MAP_REMOVE_CORPSES, wxITEM_NORMAL, OnMapRemoveCorpses);
	MAKE_ACTION(MAP_REMOVE_UNREACHABLE_TILES, wxITEM_NORMAL, OnMapRemoveUnreachable);
	MAKE_ACTION(MAP_CLEANUP, wxITEM_NORMAL, OnMapCleanup);
	MAKE_ACTION(MAP_CLEAN_HOUSE_ITEMS, wxITEM_NORMAL, OnMapCleanHouseItems);
	MAKE_ACTION(MAP_PROPERTIES, wxITEM_NORMAL, OnMapProperties);
	MAKE_ACTION(MAP_STATISTICS, wxITEM_NORMAL, OnMapStatistics);

	MAKE_ACTION(NEW_VIEW, wxITEM_NORMAL, OnNewView);
	MAKE_ACTION(TOGGLE_FULLSCREEN, wxITEM_NORMAL, OnToggleFullscreen);

	MAKE_ACTION(ZOOM_IN, wxITEM_NORMAL, OnZoomIn);
	MAKE_ACTION(ZOOM_OUT, wxITEM_NORMAL, OnZoomOut);
	MAKE_ACTION(ZOOM_NORMAL, wxITEM_NORMAL, OnZoomNormal);

	MAKE_ACTION(SHOW_SHADE, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_ALL_FLOORS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(GHOST_ITEMS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(GHOST_HIGHER_FLOORS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(HIGHLIGHT_ITEMS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_EXTRA, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_INGAME_BOX, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_GRID, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_CREATURES, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_SPAWNS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_SPECIAL, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_ONLY_COLORS, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_ONLY_MODIFIED, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_HOUSES, wxITEM_CHECK, OnChangeViewSettings);
	MAKE_ACTION(SHOW_PATHING, wxITEM_CHECK, OnChangeViewSettings);

	MAKE_ACTION(WIN_MINIMAP, wxITEM_NORMAL, OnMinimapWindow);
	MAKE_ACTION(NEW_PALETTE, wxITEM_NORMAL, OnNewPalette);
	MAKE_ACTION(TAKE_SCREENSHOT, wxITEM_NORMAL, OnTakeScreenshot);

	MAKE_ACTION(LIVE_START, wxITEM_NORMAL, OnStartLive);
	MAKE_ACTION(LIVE_JOIN, wxITEM_NORMAL, OnJoinLive);
	MAKE_ACTION(LIVE_CLOSE, wxITEM_NORMAL, OnCloseLive);

	MAKE_ACTION(SELECT_TERRAIN, wxITEM_NORMAL, OnSelectTerrainPalette);
	MAKE_ACTION(SELECT_DOODAD, wxITEM_NORMAL, OnSelectDoodadPalette);
	MAKE_ACTION(SELECT_ITEM, wxITEM_NORMAL, OnSelectItemPalette);
	MAKE_ACTION(SELECT_CREATURE, wxITEM_NORMAL, OnSelectCreaturePalette);
	MAKE_ACTION(SELECT_HOUSE, wxITEM_NORMAL, OnSelectHousePalette);
	MAKE_ACTION(SELECT_WAYPOINT, wxITEM_NORMAL, OnSelectWaypointPalette);
	MAKE_ACTION(SELECT_RAW, wxITEM_NORMAL, OnSelectRawPalette);

	MAKE_ACTION(FLOOR_0, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_1, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_2, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_3, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_4, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_5, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_6, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_7, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_8, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_9, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_10, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_11, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_12, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_13, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_14, wxITEM_RADIO, OnChangeFloor);
	MAKE_ACTION(FLOOR_15, wxITEM_RADIO, OnChangeFloor);

	MAKE_ACTION(DEBUG_VIEW_DAT, wxITEM_NORMAL, OnDebugViewDat);
	MAKE_ACTION(EXTENSIONS, wxITEM_NORMAL, OnListExtensions);
	MAKE_ACTION(GOTO_WEBSITE, wxITEM_NORMAL, OnGotoWebsite);
	MAKE_ACTION(ABOUT, wxITEM_NORMAL, OnAbout);


	// A deleter, this way the frame does not need
	// to bother deleting us.
	class CustomMenuBar : public wxMenuBar
	{
	public:
		CustomMenuBar(MainMenuBar* mb) : mb(mb) {}
		~CustomMenuBar()
		{
			delete mb;
		}
	private:
		MainMenuBar* mb;
	};

	menubar = newd CustomMenuBar(this);
	frame->SetMenuBar(menubar);

	// Tie all events to this handler!

	for(std::map<std::string, MenuBar::Action*>::iterator ai = actions.begin(); ai != actions.end(); ++ai) {
		frame->Connect(MAIN_FRAME_MENU + ai->second->id, wxEVT_COMMAND_MENU_SELECTED,
			(wxObjectEventFunction)(wxEventFunction)(ai->second->handler), nullptr, this);
	}
	for(size_t i = 0; i < 10; ++i) {
		frame->Connect(recentFiles.GetBaseId() + i, wxEVT_COMMAND_MENU_SELECTED,
			wxCommandEventHandler(MainMenuBar::OnOpenRecent), nullptr, this);
	}
}

MainMenuBar::~MainMenuBar()
{
	// Don't need to delete menubar, it's owned by the frame

	for(std::map<std::string, MenuBar::Action*>::iterator ai = actions.begin(); ai != actions.end(); ++ai) {
		delete ai->second;
	}
}

void MainMenuBar::EnableItem(MenuBar::ActionID id, bool enable)
{
	std::map<MenuBar::ActionID, std::list<wxMenuItem*> >::iterator fi = items.find(id);
	if(fi == items.end())
		return;

	std::list<wxMenuItem*>& li = fi->second;

	for(std::list<wxMenuItem*>::iterator i = li.begin(); i !=li.end(); ++i)
		(*i)->Enable(enable);
}

void MainMenuBar::CheckItem(MenuBar::ActionID id, bool enable)
{
	std::map<MenuBar::ActionID, std::list<wxMenuItem*> >::iterator fi = items.find(id);
	if(fi == items.end())
		return;

	std::list<wxMenuItem*>& li = fi->second;

	checking_programmaticly = true;
	for(std::list<wxMenuItem*>::iterator i = li.begin(); i !=li.end(); ++i)
		(*i)->Check(enable);
	checking_programmaticly = false;
}

bool MainMenuBar::IsItemChecked(MenuBar::ActionID id) const
{
	std::map<MenuBar::ActionID, std::list<wxMenuItem*> >::const_iterator fi = items.find(id);
	if(fi == items.end())
		return false;

	const std::list<wxMenuItem*>& li = fi->second;

	for(std::list<wxMenuItem*>::const_iterator i = li.begin(); i !=li.end(); ++i)
		if((*i)->IsChecked())
			return true;

	return false;
}

void MainMenuBar::Update()
{
	using namespace MenuBar;
	// This updates all buttons and sets them to proper enabled/disabled state

	Editor* editor = gui.GetCurrentEditor();
	if(editor) {
		EnableItem(UNDO, editor->actionQueue->canUndo());
		EnableItem(REDO, editor->actionQueue->canRedo());
		EnableItem(PASTE, editor->copybuffer.canPaste());
	} else {
		EnableItem(UNDO, false);
		EnableItem(REDO, false);
		EnableItem(PASTE, false);
	}

	bool loaded = gui.IsVersionLoaded();
	bool has_map = editor != nullptr;
	bool is_live = editor && editor->IsLive();
	bool is_host = has_map && !editor->IsLiveClient();
	bool is_local = has_map && !is_live;

	EnableItem(CLOSE, is_local);
	EnableItem(SAVE, is_host);
	EnableItem(SAVE_AS, is_host);
	EnableItem(GENERATE_MAP, false);

	EnableItem(IMPORT_MAP, is_local);
	EnableItem(IMPORT_MONSTERS, is_local);
	EnableItem(IMPORT_MINIMAP, false);
	EnableItem(EXPORT_MINIMAP, is_local);

	EnableItem(FIND_ITEM, is_host);
	EnableItem(REPLACE_ITEM, is_local);
	EnableItem(SEARCH_EVERYTHING, is_host);
	EnableItem(SEARCH_UNIQUE, is_host);
	EnableItem(SEARCH_ACTION, is_host);
	EnableItem(SEARCH_CONTAINER, is_host);
	EnableItem(SEARCH_WRITEABLE, is_host);

	EnableItem(CUT, has_map);
	EnableItem(COPY, has_map);

	EnableItem(BORDERIZE_SELECTION, has_map);
	EnableItem(BORDERIZE_MAP, is_local);
	EnableItem(RANDOMIZE_SELECTION, has_map);
	EnableItem(RANDOMIZE_MAP, is_local);

	EnableItem(GOTO_POSITION, has_map);
	EnableItem(JUMP_TO_BRUSH, loaded);
	EnableItem(JUMP_TO_ITEM_BRUSH, loaded);

	EnableItem(MAP_REMOVE_ITEMS, is_host);
	EnableItem(MAP_REMOVE_CORPSES, is_local);
	EnableItem(MAP_REMOVE_UNREACHABLE_TILES, is_local);
	EnableItem(CLEAR_INVALID_HOUSES, is_local);
	EnableItem(CLEAR_MODIFIED_STATE, is_local);

	EnableItem(EDIT_TOWNS, is_local);
	EnableItem(EDIT_ITEMS, false);
	EnableItem(EDIT_MONSTERS, false);

	EnableItem(MAP_CLEANUP, is_local);
	EnableItem(MAP_PROPERTIES, is_local);
	EnableItem(MAP_STATISTICS, is_local);

	EnableItem(NEW_VIEW, has_map);
	EnableItem(ZOOM_IN, has_map);
	EnableItem(ZOOM_OUT, has_map);
	EnableItem(ZOOM_NORMAL, has_map);

	EnableItem(WIN_MINIMAP, loaded);
	EnableItem(NEW_PALETTE, loaded);
	EnableItem(SELECT_TERRAIN, loaded);
	EnableItem(SELECT_DOODAD, loaded);
	EnableItem(SELECT_ITEM, loaded);
	EnableItem(SELECT_HOUSE, loaded);
	EnableItem(SELECT_CREATURE, loaded);
	EnableItem(SELECT_WAYPOINT, loaded);
	EnableItem(SELECT_RAW, loaded);

	EnableItem(LIVE_START, is_local);
	EnableItem(LIVE_JOIN, loaded);
	EnableItem(LIVE_CLOSE, is_live);

	EnableItem(DEBUG_VIEW_DAT, loaded);

	UpdateFloorMenu();
}

void MainMenuBar::LoadValues()
{
	using namespace MenuBar;

	CheckItem(SELECT_MODE_COMPENSATE, settings.getBoolean(Config::COMPENSATED_SELECT));

	if(IsItemChecked(MenuBar::SELECT_MODE_CURRENT))
		settings.setInteger(Config::SELECTION_TYPE, SELECT_CURRENT_FLOOR);
	else if(IsItemChecked(MenuBar::SELECT_MODE_LOWER))
		settings.setInteger(Config::SELECTION_TYPE, SELECT_ALL_FLOORS);
	else if(IsItemChecked(MenuBar::SELECT_MODE_VISIBLE))
		settings.setInteger(Config::SELECTION_TYPE, SELECT_VISIBLE_FLOORS);

	switch(settings.getInteger(Config::SELECTION_TYPE)) {
		case SELECT_CURRENT_FLOOR:
			CheckItem(SELECT_MODE_CURRENT, true);
			break;
		case SELECT_ALL_FLOORS:
			CheckItem(SELECT_MODE_LOWER, true);
			break;
		default:
		case SELECT_VISIBLE_FLOORS:
			CheckItem(SELECT_MODE_VISIBLE, true);
			break;
	}

	CheckItem(AUTOMAGIC, settings.getBoolean(Config::USE_AUTOMAGIC));

	CheckItem(SHOW_SHADE, settings.getBoolean(Config::SHOW_SHADE));
	CheckItem(SHOW_INGAME_BOX, settings.getBoolean(Config::SHOW_INGAME_BOX));
	CheckItem(SHOW_ALL_FLOORS, settings.getBoolean(Config::SHOW_ALL_FLOORS));
	CheckItem(GHOST_ITEMS, settings.getBoolean(Config::TRANSPARENT_ITEMS));
	CheckItem(GHOST_HIGHER_FLOORS, settings.getBoolean(Config::TRANSPARENT_FLOORS));
	CheckItem(SHOW_EXTRA, !settings.getBoolean(Config::SHOW_EXTRA));
	CheckItem(SHOW_GRID, settings.getBoolean(Config::SHOW_GRID));
	CheckItem(HIGHLIGHT_ITEMS, settings.getBoolean(Config::HIGHLIGHT_ITEMS));
	CheckItem(SHOW_CREATURES, settings.getBoolean(Config::SHOW_CREATURES));
	CheckItem(SHOW_SPAWNS, settings.getBoolean(Config::SHOW_SPAWNS));
	CheckItem(SHOW_SPECIAL, settings.getBoolean(Config::SHOW_SPECIAL_TILES));
	CheckItem(SHOW_ONLY_COLORS, settings.getBoolean(Config::SHOW_ONLY_TILEFLAGS));
	CheckItem(SHOW_ONLY_MODIFIED, settings.getBoolean(Config::SHOW_ONLY_MODIFIED_TILES));
	CheckItem(SHOW_HOUSES, settings.getBoolean(Config::SHOW_HOUSES));
}

void MainMenuBar::LoadRecentFiles()
{
	recentFiles.Load(settings.getConfigObject());
}

void MainMenuBar::SaveRecentFiles()
{
	recentFiles.Save(settings.getConfigObject());
}

void MainMenuBar::AddRecentFile(FileName file)
{
	recentFiles.AddFileToHistory(file.GetFullPath());
}

void MainMenuBar::UpdateFloorMenu()
{
	if(gui.IsEditorOpen()) {
		for(int i = 0; i < MAP_LAYERS; ++i)
			CheckItem(MenuBar::ActionID(MenuBar::FLOOR_0 + i), false);
		CheckItem(MenuBar::ActionID(MenuBar::FLOOR_0 + gui.GetCurrentFloor()), true);
	}
}

bool MainMenuBar::Load(const FileName& path, wxArrayString& warnings, wxString& error)
{
	// Open the XML file
	pugi::xml_document doc;
	pugi::xml_parse_result result = doc.load_file(path.GetFullPath().mb_str());
	if(!result) {
		error = wxT("Could not open ") + path.GetFullName() + wxT(" (file not found or syntax error)");
		return false;
	}

	pugi::xml_node node = doc.child("menubar");
	if(!node) {
		error = path.GetFullName() + wxT(": Invalid rootheader.");
		return false;
	}

	// Clear the menu
	while(menubar->GetMenuCount() > 0) {
		menubar->Remove(0);
	}

	// Load succeded
	for(pugi::xml_node menuNode = node.first_child(); menuNode; menuNode = menuNode.next_sibling()) {
		// For each child node, load it
		wxObject* i = LoadItem(menuNode, nullptr, warnings, error);
		wxMenu* m = dynamic_cast<wxMenu*>(i);
		if(m) {
			menubar->Append(m, m->GetTitle());
#ifdef __APPLE__
			m->SetTitle(m->GetTitle());
#else
			m->SetTitle(wxT(""));
#endif
		} else if(i) {
			delete i;
			warnings.push_back(path.GetFullName() + wxT(": Only menus can be subitems of main menu"));
		}
	}

#ifdef __LINUX__
	const int count = 36;
	wxAcceleratorEntry entries[count];
	// Edit
	entries[0].Set(wxACCEL_CTRL, (int)'Z', MAIN_FRAME_MENU + MenuBar::UNDO);
	entries[1].Set(wxACCEL_CTRL | wxACCEL_SHIFT, (int)'Z', MAIN_FRAME_MENU + MenuBar::REDO);
	entries[2].Set(wxACCEL_CTRL, (int)'F', MAIN_FRAME_MENU + MenuBar::FIND_ITEM);
	entries[3].Set(wxACCEL_CTRL | wxACCEL_SHIFT, (int)'F', MAIN_FRAME_MENU + MenuBar::REPLACE_ITEM);
	entries[4].Set(wxACCEL_NORMAL, (int)'A', MAIN_FRAME_MENU + MenuBar::AUTOMAGIC);
	entries[5].Set(wxACCEL_CTRL, (int)'B', MAIN_FRAME_MENU + MenuBar::BORDERIZE_SELECTION);
	entries[6].Set(wxACCEL_CTRL, (int)'G', MAIN_FRAME_MENU + MenuBar::GOTO_POSITION);
	entries[7].Set(wxACCEL_NORMAL, (int)'J', MAIN_FRAME_MENU + MenuBar::JUMP_TO_BRUSH);
	entries[8].Set(wxACCEL_CTRL, (int)'X', MAIN_FRAME_MENU + MenuBar::CUT);
	entries[9].Set(wxACCEL_CTRL, (int)'C', MAIN_FRAME_MENU + MenuBar::COPY);
	entries[10].Set(wxACCEL_CTRL, (int)'V', MAIN_FRAME_MENU + MenuBar::PASTE);
	// View
	entries[11].Set(wxACCEL_CTRL, (int)'=', MAIN_FRAME_MENU + MenuBar::ZOOM_IN);
	entries[12].Set(wxACCEL_CTRL, (int)'-', MAIN_FRAME_MENU + MenuBar::ZOOM_OUT);
	entries[13].Set(wxACCEL_CTRL, (int)'0', MAIN_FRAME_MENU + MenuBar::ZOOM_NORMAL);
	entries[14].Set(wxACCEL_NORMAL, (int)'Q', MAIN_FRAME_MENU + MenuBar::SHOW_SHADE);
	entries[15].Set(wxACCEL_CTRL, (int)'W', MAIN_FRAME_MENU + MenuBar::SHOW_ALL_FLOORS);
	entries[16].Set(wxACCEL_NORMAL, (int)'Q', MAIN_FRAME_MENU + MenuBar::GHOST_ITEMS);
	entries[17].Set(wxACCEL_CTRL, (int)'L', MAIN_FRAME_MENU + MenuBar::GHOST_HIGHER_FLOORS);
	entries[18].Set(wxACCEL_SHIFT, (int)'I', MAIN_FRAME_MENU + MenuBar::SHOW_INGAME_BOX);
	entries[19].Set(wxACCEL_SHIFT, (int)'G', MAIN_FRAME_MENU + MenuBar::SHOW_GRID);
	entries[20].Set(wxACCEL_NORMAL, (int)'V', MAIN_FRAME_MENU + MenuBar::HIGHLIGHT_ITEMS);
	entries[21].Set(wxACCEL_NORMAL, (int)'F', MAIN_FRAME_MENU + MenuBar::SHOW_CREATURES);
	entries[22].Set(wxACCEL_NORMAL, (int)'S', MAIN_FRAME_MENU + MenuBar::SHOW_SPAWNS);
	entries[23].Set(wxACCEL_NORMAL, (int)'E', MAIN_FRAME_MENU + MenuBar::SHOW_SPECIAL);
	entries[24].Set(wxACCEL_CTRL, (int)'E', MAIN_FRAME_MENU + MenuBar::SHOW_ONLY_COLORS);
	entries[25].Set(wxACCEL_CTRL, (int)'M', MAIN_FRAME_MENU + MenuBar::SHOW_ONLY_MODIFIED);
	entries[26].Set(wxACCEL_CTRL, (int)'H', MAIN_FRAME_MENU + MenuBar::SHOW_HOUSES);
	entries[27].Set(wxACCEL_NORMAL, (int)'O', MAIN_FRAME_MENU + MenuBar::SHOW_PATHING);
	// Window
	entries[28].Set(wxACCEL_NORMAL, (int)'M', MAIN_FRAME_MENU + MenuBar::WIN_MINIMAP);
	entries[29].Set(wxACCEL_NORMAL, (int)'T', MAIN_FRAME_MENU + MenuBar::SELECT_TERRAIN);
	entries[30].Set(wxACCEL_NORMAL, (int)'D', MAIN_FRAME_MENU + MenuBar::SELECT_DOODAD);
	entries[31].Set(wxACCEL_NORMAL, (int)'I', MAIN_FRAME_MENU + MenuBar::SELECT_ITEM);
	entries[32].Set(wxACCEL_NORMAL, (int)'H', MAIN_FRAME_MENU + MenuBar::SELECT_HOUSE);
	entries[33].Set(wxACCEL_NORMAL, (int)'C', MAIN_FRAME_MENU + MenuBar::SELECT_CREATURE);
	entries[34].Set(wxACCEL_NORMAL, (int)'W', MAIN_FRAME_MENU + MenuBar::SELECT_WAYPOINT);
	entries[35].Set(wxACCEL_NORMAL, (int)'R', MAIN_FRAME_MENU + MenuBar::SELECT_RAW);

	wxAcceleratorTable accelerator(count, entries);
	frame->SetAcceleratorTable(accelerator);
#endif

	/*
	// Create accelerator table
	accelerator_table = newd wxAcceleratorTable(accelerators.size(), &accelerators[0]);

	// Tell all clients of the renewed accelerators
	RenewClients();
	*/

	recentFiles.AddFilesToMenu();
	Update();
	LoadValues();
	return true;
}

wxObject* MainMenuBar::LoadItem(pugi::xml_node node, wxMenu* parent, wxArrayString& warnings, wxString& error)
{
	pugi::xml_attribute attribute;

	const std::string& nodeName = as_lower_str(node.name());
	if(nodeName == "menu") {
		if(!(attribute = node.attribute("name"))) {
			return nullptr;
		}

		std::string name = attribute.as_string();
		std::replace(name.begin(), name.end(), '$', '&');

		wxMenu* menu = newd wxMenu;
		if((attribute = node.attribute("special")) && std::string(attribute.as_string()) == "RECENT_FILES") {
			recentFiles.UseMenu(menu);
		} else {
			for(pugi::xml_node menuNode = node.first_child(); menuNode; menuNode = menuNode.next_sibling()) {
				// Load an add each item in order
				LoadItem(menuNode, menu, warnings, error);
			}
		}

		// If we have a parent, add ourselves.
		// If not, we just return the item and the parent function
		// is responsible for adding us to wherever
		if(parent) {
			parent->AppendSubMenu(menu, wxstr(name));
		} else {
			menu->SetTitle((name));
		}
		return menu;
	} else if(nodeName == "item") {
		// We must have a parent when loading items
		if(!parent) {
			return nullptr;
		} else if(!(attribute = node.attribute("name"))) {
			return nullptr;
		}

		std::string name = attribute.as_string();
		std::replace(name.begin(), name.end(), '$', '&');
		if(!(attribute = node.attribute("action"))) {
			return nullptr;
		}

		const std::string& action = attribute.as_string();
		std::string hotkey = node.attribute("hotkey").as_string();
		if(!hotkey.empty()) {
			hotkey = '\t' + hotkey;
		}

		const std::string& help = node.attribute("help").as_string();
		name += hotkey;

		auto it = actions.find(action);
		if(it == actions.end()) {
			warnings.push_back(wxT("Invalid action type '") + wxstr(action) + wxT("'."));
			return nullptr;
		}

		const MenuBar::Action& act = *it->second;
		wxAcceleratorEntry* entry = wxAcceleratorEntry::Create(wxstr(hotkey));
		if(entry) {
			delete entry; // accelerators.push_back(entry);
		} else {
			warnings.push_back(wxT("Invalid hotkey."));
		}

		wxMenuItem* tmp = parent->Append(
			MAIN_FRAME_MENU + act.id, // ID
			wxstr(name), // Title of button
			wxstr(help), // Help text
			act.kind // Kind of item
		);
		items[MenuBar::ActionID(act.id)].push_back(tmp);
		return tmp;
	} else if(nodeName == "separator") {
		// We must have a parent when loading items
		if(!parent) {
			return nullptr;
		}
		return parent->AppendSeparator();
	}
	return nullptr;
}

void MainMenuBar::OnNew(wxCommandEvent& WXUNUSED(event))
{
	gui.NewMap();
}

void MainMenuBar::OnGenerateMap(wxCommandEvent& WXUNUSED(event))
{
	/*
	if(!DoQuerySave()) return;

	std::ostringstream os;
	os << "Untitled-" << untitled_counter << ".otbm";
	++untitled_counter;

	editor.generateMap(wxstr(os.str()));

	gui.SetStatusText(wxT("Generated newd map"));

	gui.UpdateTitle();
	gui.RefreshPalettes();
	gui.UpdateMinimap();
	gui.FitViewToMap();
	UpdateMenubar();
	Refresh();
	*/
}

void MainMenuBar::OnOpenRecent(wxCommandEvent& event)
{
	FileName fn(recentFiles.GetHistoryFile(event.GetId() - recentFiles.GetBaseId()));
	frame->LoadMap(fn);
}

void MainMenuBar::OnOpen(wxCommandEvent& WXUNUSED(event))
{
	wxString wildcard = (settings.getInteger(Config::USE_OTGZ) != 0 ? MAP_LOAD_FILE_WILDCARD_OTGZ : MAP_LOAD_FILE_WILDCARD);
	wxFileDialog filedlg(frame, wxT("Open map file"), wxT(""), wxT(""), wildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	int ok = filedlg.ShowModal();

	if(ok == wxID_OK)
		frame->LoadMap(filedlg.GetPath());
}

void MainMenuBar::OnClose(wxCommandEvent& WXUNUSED(event))
{
	frame->DoQuerySave(true); // It closes the editor too
}

void MainMenuBar::OnSave(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsEditorOpen())
		return;

	if(gui.GetCurrentMap().hasFile()) {
		gui.SaveCurrentMap(true);
	} else {
		wxString wildcard = (settings.getInteger(Config::USE_OTGZ) != 0 ? MAP_SAVE_FILE_WILDCARD_OTGZ : MAP_SAVE_FILE_WILDCARD);
		wxFileDialog file(frame, wxT("Save..."), wxT(""), wxT(""), wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
		int ok = file.ShowModal();

		if(ok == wxID_OK)
			gui.SaveCurrentMap(file.GetPath(), true);
	}
}

void MainMenuBar::OnSaveAs(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsEditorOpen())
		return;

	wxString wildcard = (settings.getInteger(Config::USE_OTGZ) != 0 ? MAP_SAVE_FILE_WILDCARD_OTGZ : MAP_SAVE_FILE_WILDCARD);
	wxFileDialog file(frame, wxT("Save As..."), wxT(""), wxT(""), wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);
	int ok = file.ShowModal();

	if(ok == wxID_OK) {
		gui.SaveCurrentMap(file.GetPath(), true);
		AddRecentFile(file.GetPath());
	}

	gui.UpdateTitle();
	Update();
}

void MainMenuBar::OnPreferences(wxCommandEvent& WXUNUSED(event))
{
	PreferencesWindow win(frame);
	win.ShowModal();
}

void MainMenuBar::OnQuit(wxCommandEvent& WXUNUSED(event))
{
	/*
	while(gui.IsEditorOpen())
		if(!frame->DoQuerySave(true))
			return;
			*/
	//((Application*)wxTheApp)->Unload();
	gui.root->Close();
}

void MainMenuBar::OnImportMap(wxCommandEvent& WXUNUSED(event))
{
	ASSERT(gui.GetCurrentEditor());
	wxDialog* importmap = newd ImportMapWindow(frame, *gui.GetCurrentEditor());
	importmap->ShowModal();
}

void MainMenuBar::OnImportMonsterData(wxCommandEvent& WXUNUSED(event))
{
	wxFileDialog dlg(gui.root, wxT("Import monster/npc file"), wxT(""),wxT(""),wxT("*.xml"), wxFD_OPEN | wxFD_MULTIPLE | wxFD_FILE_MUST_EXIST);
	if(dlg.ShowModal() == wxID_OK) {
		wxArrayString paths;
		dlg.GetPaths(paths);
		for(uint32_t i = 0; i < paths.GetCount(); ++i) {
			wxString error;
			wxArrayString warnings;
			bool ok = creature_db.importXMLFromOT(FileName(paths[i]), error, warnings);
			if(ok)
				gui.ListDialog(wxT("Monster loader errors"), warnings);
			else
				wxMessageBox(wxT("Error OT data file \"") + paths[i] + wxT("\".\n") + error, wxT("Error"), wxOK | wxICON_INFORMATION, gui.root);
		}
	}
}

void MainMenuBar::OnImportMinimap(wxCommandEvent& WXUNUSED(event))
{
	ASSERT(gui.IsEditorOpen());
	//wxDialog* importmap = newd ImportMapWindow();
	//importmap->ShowModal();
}

void MainMenuBar::OnExportMinimap(wxCommandEvent& WXUNUSED(event))
{
	if(gui.GetCurrentEditor()) {
		ExportMiniMapWindow dlg(frame, *gui.GetCurrentEditor());
		dlg.ShowModal();
	}
}

void MainMenuBar::OnDebugViewDat(wxCommandEvent& WXUNUSED(event))
{
	wxDialog dlg(frame, wxID_ANY, wxT("Debug .dat file"), wxDefaultPosition, wxDefaultSize, wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
	new DatDebugView(&dlg);
	dlg.ShowModal();
}

void MainMenuBar::OnReloadDataFiles(wxCommandEvent& WXUNUSED(event))
{
	wxString error;
	wxArrayString warnings;
	gui.LoadVersion(gui.GetCurrentVersionID(), error, warnings, true);
	gui.PopupDialog(wxT("Error"), error, wxOK);
	gui.ListDialog(wxT("Warnings"), warnings);
}

void MainMenuBar::OnListExtensions(wxCommandEvent& WXUNUSED(event))
{
	ExtensionsDialog exts(frame);
	exts.ShowModal();
}

void MainMenuBar::OnGotoWebsite(wxCommandEvent& WXUNUSED(event))
{
	::wxLaunchDefaultBrowser(wxT("http://www.remeresmapeditor.com/"),  wxBROWSER_NEW_WINDOW);
}

void MainMenuBar::OnAbout(wxCommandEvent& WXUNUSED(event))
{
	AboutWindow about(frame);
	about.ShowModal();
}

void MainMenuBar::OnUndo(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsEditorOpen())
		return;

	if(gui.GetCurrentEditor()->actionQueue->canUndo()) {
		gui.GetCurrentEditor()->actionQueue->undo();
		if(gui.GetCurrentEditor()->selection.size() > 0)
			gui.SetSelectionMode();
		gui.SetStatusText(wxT("Undo action"));
		gui.UpdateMinimap();
		Update();
		frame->Refresh();
	}
}

void MainMenuBar::OnRedo(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsEditorOpen())
		return;

	if(gui.GetCurrentEditor()->actionQueue->canRedo()) {
		gui.GetCurrentEditor()->actionQueue->redo();
		if(gui.GetCurrentEditor()->selection.size() > 0)
			gui.SetSelectionMode();
		gui.SetStatusText(wxT("Redo action"));
		gui.UpdateMinimap();
		Update();
		frame->Refresh();
	}
}

namespace OnSearchForItem
{
	struct Finder
	{
		Finder(uint16_t itemid)
			: more_than_value(false), itemid(itemid), actionid(0), uniqueid(0) {}

		Finder(uint16_t itemid, uint16_t actionid)
			: more_than_value(false), itemid(itemid), actionid(actionid), uniqueid(0) {}

		Finder(uint16_t itemid, uint16_t actionid, uint16_t uniqueid)
			: more_than_value(false), itemid(itemid), actionid(actionid), uniqueid(uniqueid) {}

		bool more_than_value;
		uint16_t itemid;
		uint16_t actionid;
		uint16_t uniqueid;

		std::vector<std::pair<Tile*, Item*> > found;

		void operator()(Map& map, Tile* tile, Item* item, long long done)
		{
			if(more_than_value) return;
			if(done % 0x8000 == 0) {
				gui.SetLoadDone((unsigned int)(100 * done / map.getTileCount()));
			}

			uint16_t itemId = item->getID();
			uint16_t aid = item->getActionID();
			uint16_t uid = item->getUniqueID();

			if((itemid > 100 && itemId == itemid) || (actionid > 0 && aid == actionid) || (uniqueid > 0 && uid == uniqueid)) {
				found.push_back(std::make_pair(tile, item));
				if(found.size() >= size_t(settings.getInteger(Config::REPLACE_SIZE)))
					more_than_value = true;
			}
		}

		wxString desc(Item* item)
		{
			wxString label;
			if(item->getUniqueID() > 0)
				label << wxT("UID:") << item->getUniqueID() << wxT(" ");

			if(item->getActionID() > 0)
				label << wxT("AID:") << item->getActionID() << wxT(" ");

			label << wxstr(item->getName());

			if(dynamic_cast<Container*>(item))
				label << wxT(" (Container) ");

			if(item->getText().length() > 0)
				label << wxT(" (Text: ") << wxstr(item->getText()) << wxT(") ");

			return label;
		}
	};
}

void MainMenuBar::OnSearchForItem(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsEditorOpen())
		return;

	FindItemDialog finder(frame, wxT("Search for Item"));
	if(finder.ShowModal() != 0) {
		OnSearchForItem::Finder func(finder.getResultID(), finder.getActionId(), finder.getUniqueId());
		gui.CreateLoadBar(wxT("Searching map..."));

		foreach_ItemOnMap(gui.GetCurrentMap(), func);
		std::vector<std::pair<Tile*, Item*> >& found = func.found;

		gui.DestroyLoadBar();

		if(func.more_than_value) {
			wxString msg;
			msg << wxT("Only the first ") << size_t(settings.getInteger(Config::REPLACE_SIZE)) << wxT(" results will be displayed.");
			gui.PopupDialog(wxT("Notice"), msg, wxOK);
		}

		SearchResultWindow* result = gui.ShowSearchWindow();
		result->Clear();
		for(std::vector<std::pair<Tile*, Item*> >::const_iterator iter = found.begin(); iter != found.end(); ++iter) {
			Tile* tile = iter->first;
			Item* item = iter->second;
			result->AddPosition(func.desc(item), tile->getPosition());
		}
	}
}
void MainMenuBar::OnReplaceItem(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsEditorOpen())
		return;

	ReplaceItemDialog dlg(frame, wxT("Replace Item"));

	if(dlg.ShowModal() != 0) {
		uint16_t find_id = dlg.GetResultFindID();
		uint16_t with_id = dlg.GetResultWithID();

		OnSearchForItem::Finder finder(find_id);
		gui.GetCurrentEditor()->actionQueue->clear();
		gui.CreateLoadBar(wxT("Searching & replacing map..."));

		// Search the map
		foreach_ItemOnMap(gui.GetCurrentMap(), finder);

		// Replace the items in a second step (can't replace while iterating)
		for(std::vector<std::pair<Tile*, Item*> >::const_iterator replace_iter = finder.found.begin();
				replace_iter != finder.found.end();
				++replace_iter)
		{
			transformItem(replace_iter->second, with_id, replace_iter->first);
		}

		wxString msg;
		msg << wxT("Replaced ") << finder.found.size() << wxT(" items.");
		gui.SetStatusText(msg);

		gui.DestroyLoadBar();
	}

	gui.RefreshView();
}

namespace OnSearchForStuff
{
	struct Searcher
	{
		Searcher() :
			search_unique(-1),
			search_action(-1),
			search_container(false),
			search_writeable(false) {}

		int search_unique;
		int search_action;
		bool search_container;
		bool search_writeable;
		std::vector<std::pair<Tile*, Item*> > found;

		void operator()(Map& map, Tile* tile, Item* item, long long done)
		{
			if(done % 0x8000 == 0) {
				gui.SetLoadDone((unsigned int)(100 * done / map.getTileCount()));
			}
			Container* container;

			bool exactUnique = search_unique > 0;
			bool exactAction = search_action > 0;

			int uid = item->getUniqueID();
			int aid = item->getActionID();

			if((search_unique == 0 || (exactUnique && uid == search_unique)) ||
				(search_action == 0 || (exactAction && aid == search_action)) ||
				(search_container && ((container = dynamic_cast<Container*>(item)) && container->getItemCount())) ||
				(search_writeable && item->getText().length() > 0)) {
				found.push_back(std::make_pair(tile, item));
			}
		}

		wxString desc(Item* item)
		{
			wxString label;
			if(item->getUniqueID() > 0)
				label << wxT("UID:") << item->getUniqueID() << wxT(" ");

			if(item->getActionID() > 0)
				label << wxT("AID:") << item->getActionID() << wxT(" ");

			label << wxstr(item->getName());

			if(dynamic_cast<Container*>(item))
				label << wxT(" (Container) ");

			if(item->getText().length() > 0)
				label << wxT(" (Text: ") << wxstr(item->getText()) << wxT(") ");

			return label;
		}
	};
}

void MainMenuBar::OnSearchForStuff(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsEditorOpen())
		return;

	gui.CreateLoadBar(wxT("Searching map..."));

	OnSearchForStuff::Searcher searcher;
	searcher.search_unique = 0;
	searcher.search_action = 0;
	searcher.search_container = true;
	searcher.search_writeable = true;

	foreach_ItemOnMap(gui.GetCurrentMap(), searcher);
	std::vector<std::pair<Tile*, Item*> >& found = searcher.found;

	gui.DestroyLoadBar();

	SearchResultWindow* result = gui.ShowSearchWindow();
	result->Clear();
	for(std::vector<std::pair<Tile*, Item*> >::iterator iter = found.begin(); iter != found.end(); ++iter) {
		result->AddPosition(searcher.desc(iter->second), iter->first->getPosition());
	}
}

void MainMenuBar::OnSearchForUnique(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsEditorOpen())
		return;

	gui.CreateLoadBar(wxT("Searching map..."));

	OnSearchForStuff::Searcher searcher;
	searcher.search_unique = 0;
	foreach_ItemOnMap(gui.GetCurrentMap(), searcher);
	std::vector<std::pair<Tile*, Item*> >& found = searcher.found;

	gui.DestroyLoadBar();

	SearchResultWindow* result = gui.ShowSearchWindow();
	result->Clear();
	for(std::vector<std::pair<Tile*, Item*> >::iterator iter = found.begin(); iter != found.end(); ++iter) {
		result->AddPosition(searcher.desc(iter->second), iter->first->getPosition());
	}
}

void MainMenuBar::OnSearchForAction(wxCommandEvent& WXUNUSED(event)) {
	if(!gui.IsEditorOpen())
		return;

	gui.CreateLoadBar(wxT("Searching map..."));

	OnSearchForStuff::Searcher searcher;
	searcher.search_action = 0;
	foreach_ItemOnMap(gui.GetCurrentMap(), searcher);
	std::vector<std::pair<Tile*, Item*> >& found = searcher.found;

	gui.DestroyLoadBar();

	SearchResultWindow* result = gui.ShowSearchWindow();
	result->Clear();
	for(std::vector<std::pair<Tile*, Item*> >::iterator iter = found.begin(); iter != found.end(); ++iter) {
		result->AddPosition(searcher.desc(iter->second), iter->first->getPosition());
	}
}

void MainMenuBar::OnSearchForContainer(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsEditorOpen())
		return;

	gui.CreateLoadBar(wxT("Searching map..."));

	OnSearchForStuff::Searcher searcher;
	searcher.search_container = true;
	foreach_ItemOnMap(gui.GetCurrentMap(), searcher);
	std::vector<std::pair<Tile*, Item*> >& found = searcher.found;

	gui.DestroyLoadBar();

	SearchResultWindow* result = gui.ShowSearchWindow();
	result->Clear();
	for(std::vector<std::pair<Tile*, Item*> >::iterator iter = found.begin(); iter != found.end(); ++iter) {
		result->AddPosition(searcher.desc(iter->second), iter->first->getPosition());
	}
}

void MainMenuBar::OnSearchForWriteable(wxCommandEvent& WXUNUSED(event)) {
	if(!gui.IsEditorOpen())
		return;

	gui.CreateLoadBar(wxT("Searching map..."));

	OnSearchForStuff::Searcher searcher;
	searcher.search_writeable = true;
	foreach_ItemOnMap(gui.GetCurrentMap(), searcher);
	std::vector<std::pair<Tile*, Item*> >& found = searcher.found;

	gui.DestroyLoadBar();

	SearchResultWindow* result = gui.ShowSearchWindow();
	result->Clear();
	for(std::vector<std::pair<Tile*, Item*> >::iterator iter = found.begin(); iter != found.end(); ++iter) {
		result->AddPosition(searcher.desc(iter->second), iter->first->getPosition());
	}
}

void MainMenuBar::OnSelectionTypeChange(wxCommandEvent& WXUNUSED(event))
{
	settings.setInteger(Config::COMPENSATED_SELECT, IsItemChecked(MenuBar::SELECT_MODE_COMPENSATE));

	if(IsItemChecked(MenuBar::SELECT_MODE_CURRENT))
		settings.setInteger(Config::SELECTION_TYPE, SELECT_CURRENT_FLOOR);
	else if(IsItemChecked(MenuBar::SELECT_MODE_LOWER))
		settings.setInteger(Config::SELECTION_TYPE, SELECT_ALL_FLOORS);
	else if(IsItemChecked(MenuBar::SELECT_MODE_VISIBLE))
		settings.setInteger(Config::SELECTION_TYPE, SELECT_VISIBLE_FLOORS);
}


void MainMenuBar::OnCopy(wxCommandEvent& WXUNUSED(event))
{
	if(gui.IsSelectionMode()) {
		if(gui.GetCurrentEditor()) {
			gui.GetCurrentEditor()->copybuffer.copy(*gui.GetCurrentEditor(), gui.GetCurrentFloor());
			gui.RefreshView();
			Update();
		}
	}
}

void MainMenuBar::OnCut(wxCommandEvent& WXUNUSED(event))
{
	if(gui.IsSelectionMode()) {
		if(gui.GetCurrentEditor()) {
			gui.GetCurrentEditor()->copybuffer.cut(*gui.GetCurrentEditor(), gui.GetCurrentFloor());
			gui.RefreshView();
			Update();
		}
	}
}

void MainMenuBar::OnPaste(wxCommandEvent& WXUNUSED(event))
{
	gui.SetSelectionMode();
	// Clear any old selection
	gui.GetCurrentEditor()->selection.start();
	gui.GetCurrentEditor()->selection.clear();
	gui.GetCurrentEditor()->selection.finish();

	// Start a pasting session
	gui.StartPasting();

	gui.RefreshView();
}

void MainMenuBar::OnToggleAutomagic(wxCommandEvent& WXUNUSED(event))
{
	settings.setInteger(Config::USE_AUTOMAGIC, IsItemChecked(MenuBar::AUTOMAGIC));
	settings.setInteger(Config::BORDER_IS_GROUND, IsItemChecked(MenuBar::AUTOMAGIC));
	if(settings.getInteger(Config::USE_AUTOMAGIC))
		gui.SetStatusText(wxT("Automagic enabled."));
	else
		gui.SetStatusText(wxT("Automagic disabled."));
}

void MainMenuBar::OnBorderizeSelection(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsEditorOpen())
		return;

	gui.GetCurrentEditor()->borderizeSelection();
	gui.RefreshView();
}

void MainMenuBar::OnBorderizeMap(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsEditorOpen())
		return;

	int ret = gui.PopupDialog(wxT("Borderize Map"), wxT("Are you sure you want to borderize the entire map (this action cannot be undone)?"), wxYES | wxNO);
	if(ret == wxID_YES)
		gui.GetCurrentEditor()->borderizeMap(true);

	gui.RefreshView();
}

void MainMenuBar::OnRandomizeSelection(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsEditorOpen())
		return;

	gui.GetCurrentEditor()->randomizeSelection();
	gui.RefreshView();
}

void MainMenuBar::OnRandomizeMap(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsEditorOpen())
		return;

	int ret = gui.PopupDialog(wxT("Randomize Map"), wxT("Are you sure you want to randomize the entire map (this action cannot be undone)?"), wxYES | wxNO);
	if(ret == wxID_YES)
		gui.GetCurrentEditor()->randomizeMap(true);

	gui.RefreshView();
}

void MainMenuBar::OnJumpToBrush(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsVersionLoaded())
		return;

	// Create the jump to dialog
	FindDialog* dlg = newd FindBrushDialog(frame);

	// Display dialog to user
	dlg->ShowModal();

	// Retrieve result, if null user canceled
	const Brush* brush = dlg->getResult();
	if(brush) {
		gui.SelectBrush(brush, TILESET_UNKNOWN);
	}
	delete dlg;
}

void MainMenuBar::OnJumpToItemBrush(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsVersionLoaded())
		return;

	// Create the jump to dialog
	FindDialog* dlg = newd FindItemDialog(frame);

	// Display dialog to user
	dlg->ShowModal();

	// Retrieve result, if null user canceled
	const Brush* brush = dlg->getResult();
	if(brush) {
		gui.SelectBrush(brush, TILESET_RAW);
	}
	delete dlg;
}

void MainMenuBar::OnGotoPosition(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsEditorOpen())
		return;

	// Display dialog, it also controls the actual jump
	GotoPositionDialog dlg(frame, *gui.GetCurrentEditor());
	dlg.ShowModal();
}

namespace OnMapRemoveItems
{
	struct condition
	{
		condition(uint16_t itemid) : itemid(itemid) {}
		uint16_t itemid;

		bool operator()(Map& map, Item* item, long long removed, long long done){
			if(done % 0x8000 == 0)
				gui.SetLoadDone((unsigned int)(100 * done / map.getTileCount()));

			return item->getID() == itemid && !item->isComplex();
		}
	};
}

void MainMenuBar::OnMapRemoveItems(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsEditorOpen())
		return;

	FindItemDialog finder(frame, wxT("Item Type to Remove"));
	finder.ShowModal();
	uint16_t itemid = finder.getResultID();

	if(itemid != 0) {
		gui.GetCurrentEditor()->selection.clear();
		gui.GetCurrentEditor()->actionQueue->clear();

		OnMapRemoveItems::condition func(itemid);
		gui.CreateLoadBar(wxT("Searching map for items to remove..."));

		long long removed = remove_if_ItemOnMap(gui.GetCurrentMap(), func);

		gui.DestroyLoadBar();

		wxString msg;
		msg << removed << wxT(" items deleted.");

		gui.PopupDialog(wxT("Search completed"), msg, wxOK);

		gui.GetCurrentMap().doChange();
	}
}

namespace OnMapRemoveCorpses
{
	struct condition
	{
		condition() {}

		bool operator()(Map& map, Item* item, long long removed, long long done){
			if(done % 0x800 == 0)
				gui.SetLoadDone((unsigned int)(100 * done / map.getTileCount()));

			return materials.isInTileset(item, "Corpses") & !item->isComplex();
		}
	};
}

void MainMenuBar::OnMapRemoveCorpses(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsEditorOpen())
		return;

	int ok = gui.PopupDialog(wxT("Remove Corpses"), wxT("Do you want to remove all corpses from the map?"), wxYES | wxNO);

	if(ok == wxID_YES) {
		gui.GetCurrentEditor()->selection.clear();
		gui.GetCurrentEditor()->actionQueue->clear();

		OnMapRemoveCorpses::condition func;
		gui.CreateLoadBar(wxT("Searching map for items to remove..."));

		long long removed = remove_if_ItemOnMap(gui.GetCurrentMap(), func);

		gui.DestroyLoadBar();

		wxString msg;
		msg << removed << wxT(" items deleted.");

		gui.PopupDialog(wxT("Search completed"), msg, wxOK);

		gui.GetCurrentMap().doChange();
	}
}

namespace OnMapRemoveUnreachable
{
	struct condition
	{
		condition() {}

		bool isReachable(Tile* tile)
		{
			if(tile == nullptr)
				return false;
			if(!tile->isBlocking())
				return true;
			return false;
		}

		bool operator()(Map& map, Tile* tile, long long removed, long long done, long long total)
		{
			if(done % 0x1000 == 0)
				gui.SetLoadDone((unsigned int)(100 * done / total));

			Position pos = tile->getPosition();
			int sx = std::max(pos.x - 10, 0);
			int ex = std::min(pos.x + 10, 65535);
			int sy = std::max(pos.y - 8,  0);
			int ey = std::min(pos.y + 8,  65535);
			int sz, ez;

			if(pos.z < 8) {
				sz = 0;
				ez = 9;
			} else {
				// underground
				sz = std::max(pos.z - 2, GROUND_LAYER);
				ez = std::min(pos.z + 2, MAP_MAX_LAYER);
			}

			for(int z = sz; z <= ez; ++z) {
				for(int y = sy; y <= ey; ++y) {
					for(int x = sx; x <= ex; ++x) {
						if(isReachable(map.getTile(x, y, z)))
							return false;
					}
				}
			}
			return true;
		}
	};
}

void MainMenuBar::OnMapRemoveUnreachable(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsEditorOpen())
		return;

	int ok = gui.PopupDialog(wxT("Remove Unreachable Tiles"), wxT("Do you want to remove all unreachable items from the map?"), wxYES | wxNO);

	if(ok == wxID_YES) {
		gui.GetCurrentEditor()->selection.clear();
		gui.GetCurrentEditor()->actionQueue->clear();

		OnMapRemoveUnreachable::condition func;
		gui.CreateLoadBar(wxT("Searching map for tiles to remove..."));

		long long removed = remove_if_TileOnMap(gui.GetCurrentMap(), func);

		gui.DestroyLoadBar();

		wxString msg;
		msg << removed << wxT(" tiles deleted.");

		gui.PopupDialog(wxT("Search completed"), msg, wxOK);

		gui.GetCurrentMap().doChange();
	}
}

void MainMenuBar::OnClearHouseTiles(wxCommandEvent& WXUNUSED(event))
{
	Editor* editor = gui.GetCurrentEditor();
	if(!editor)
		return;

	int ret = gui.PopupDialog(
		wxT("Clear Invalid House Tiles"),
		wxT("Are you sure you want to remove all house tiles that do not belong to a house (this action cannot be undone)?"),
		wxYES | wxNO
	);

	if(ret == wxID_YES) {
		// Editor will do the work
		editor->clearInvalidHouseTiles(true);
	}

	gui.RefreshView();
}

void MainMenuBar::OnClearModifiedState(wxCommandEvent& WXUNUSED(event))
{
	Editor* editor = gui.GetCurrentEditor();
	if(!editor)
		return;

	int ret = gui.PopupDialog(
		wxT("Clear Modified State"),
		wxT("This will have the same effect as closing the map and opening it again. Do you want to proceed?"),
		wxYES | wxNO
	);

	if(ret == wxID_YES) {
		// Editor will do the work
		editor->clearModifiedTileState(true);
	}

	gui.RefreshView();
}

void MainMenuBar::OnMapCleanHouseItems(wxCommandEvent& WXUNUSED(event))
{
	Editor* editor = gui.GetCurrentEditor();
	if(!editor)
		return;

	int ret = gui.PopupDialog(
		wxT("Clear Moveable House Items"),
		wxT("Are you sure you want to remove all items inside houses that can be moved (this action cannot be undone)?"),
		wxYES | wxNO
	);

	if(ret == wxID_YES) {
		// Editor will do the work
		//editor->removeHouseItems(true);
	}

	gui.RefreshView();
}

void MainMenuBar::OnMapEditTowns(wxCommandEvent& WXUNUSED(event))
{
	if(gui.GetCurrentEditor()) {
		wxDialog* town_dialog = newd EditTownsDialog(frame, *gui.GetCurrentEditor());
		town_dialog->ShowModal();
		town_dialog->Destroy();
	}
}

void MainMenuBar::OnMapEditItems(wxCommandEvent& WXUNUSED(event))
{
	;
}

void MainMenuBar::OnMapEditMonsters(wxCommandEvent& WXUNUSED(event))
{
	;
}

void MainMenuBar::OnMapStatistics(wxCommandEvent& WXUNUSED(event))
{
	if(!gui.IsEditorOpen())
		return;

	gui.CreateLoadBar(wxT("Collecting data..."));

	Map* map = &gui.GetCurrentMap();

	int load_counter = 0;

	uint64_t tile_count = 0;
	uint64_t detailed_tile_count = 0;
	uint64_t blocking_tile_count = 0;
	uint64_t walkable_tile_count = 0;
	double percent_pathable = 0.0;
	double percent_detailed = 0.0;
	uint64_t spawn_count = 0;
	uint64_t creature_count = 0;
	double creatures_per_spawn = 0.0;

	uint64_t item_count = 0;
	uint64_t loose_item_count = 0;
	uint64_t depot_count = 0;
	uint64_t action_item_count = 0;
	uint64_t unique_item_count = 0;
	uint64_t container_count = 0; // Only includes containers containing more than 1 item

	int town_count = map->towns.count();
	int house_count = map->houses.count();
	std::map<uint32_t, uint32_t> town_sqm_count;
	const Town* largest_town = nullptr;
	uint64_t largest_town_size = 0;
	uint64_t total_house_sqm = 0;
	const House* largest_house = nullptr;
	uint64_t largest_house_size = 0;
	double houses_per_town = 0.0;
	double sqm_per_house = 0.0;
	double sqm_per_town = 0.0;

	for(MapIterator mit = map->begin(); mit != map->end(); ++mit) {
		Tile* tile = (*mit)->get();
		if(load_counter % 8192 == 0) {
			gui.SetLoadDone((unsigned int)(int64_t(load_counter) * 95ll / int64_t(map->getTileCount())));
		}

		if(tile->empty())
			continue;

		tile_count += 1;

		bool is_detailed = false;
#define ANALYZE_ITEM(_item) {\
	item_count += 1; \
	if(!(_item)->isGroundTile() && !(_item)->isBorder()) { \
		is_detailed = true; \
		ItemType& it = item_db[(_item)->getID()]; \
		if(it.moveable) { \
			loose_item_count += 1; \
		} \
		if(it.isDepot()) { \
			depot_count += 1; \
		} \
		if((_item)->getActionID() > 0) { \
			action_item_count += 1; \
		} \
		if((_item)->getUniqueID() > 0) { \
			unique_item_count += 1; \
		} \
		if(Container* c = dynamic_cast<Container*>((_item))) { \
			if(c->getVector().size()) { \
				container_count += 1; \
			} \
		} \
	} \
}

		if(tile->ground) {
			ANALYZE_ITEM(tile->ground);
		}

		for(ItemVector::const_iterator item_iter = tile->items.begin(); item_iter != tile->items.end(); ++item_iter) {
			Item* item = *item_iter;
			ANALYZE_ITEM(item);
		}
#undef ANALYZE_ITEM

		if(tile->spawn)
			spawn_count += 1;

		if(tile->creature)
			creature_count += 1;

		if(tile->isBlocking())
			blocking_tile_count += 1;
		else
			walkable_tile_count += 1;

		if(is_detailed)
			detailed_tile_count += 1;

		load_counter += 1;
	}

	creatures_per_spawn =       (spawn_count != 0? double(creature_count) /      double(spawn_count) : -1.0);
	percent_pathable    = 100.0*(tile_count != 0?  double(walkable_tile_count) / double(tile_count) : -1.0);
	percent_detailed    = 100.0*(tile_count != 0?  double(detailed_tile_count) / double(tile_count) : -1.0);

	load_counter = 0;
	Houses& houses = map->houses;
	for(HouseMap::const_iterator hit = houses.begin(); hit != houses.end(); ++hit) {
		const House* house = hit->second;

		if(load_counter % 64)
			gui.SetLoadDone((unsigned int)(95ll + int64_t(load_counter) * 5ll / int64_t(house_count)));

		if(house->size() > largest_house_size) {
			largest_house = house;
			largest_house_size = house->size();
		}
		total_house_sqm += house->size();
		town_sqm_count[house->townid] += house->size();
	}

	houses_per_town = (town_count != 0?  double(house_count) /     double(town_count)  : -1.0);
	sqm_per_house   = (house_count != 0? double(total_house_sqm) / double(house_count) : -1.0);
	sqm_per_town    = (town_count != 0?  double(total_house_sqm) / double(town_count)  : -1.0);

	Towns& towns = map->towns;
	for(std::map<uint32_t, uint32_t>::iterator town_iter = town_sqm_count.begin();
			town_iter != town_sqm_count.end();
			++town_iter)
	{
		// No load bar for this, load is non-existant
		uint32_t town_id = town_iter->first;
		uint32_t town_sqm = town_iter->second;
		Town* town = towns.getTown(town_id);
		if(town && town_sqm > largest_town_size) {
			largest_town = town;
			largest_town_size = town_sqm;
		} else {
			// Non-existant town!
		}
	}

	gui.DestroyLoadBar();

	std::ostringstream os;
	os.setf(std::ios::fixed, std::ios::floatfield);
	os.precision(2);
	os << "Map statistics for the map \"" << map->getMapDescription() << "\"\n";
	os << "\tTile data:\n";
	os << "\t\tTotal number of tiles: " << tile_count << "\n";
	os << "\t\tNumber of pathable tiles: " << walkable_tile_count << "\n";
	os << "\t\tNumber of unpathable tiles: " << blocking_tile_count << "\n";
	if(percent_pathable >= 0.0)
		os << "\t\tPercent walkable tiles: " << percent_pathable << "%\n";
	os << "\t\tDetailed tiles: " << detailed_tile_count << "\n";
	if(percent_detailed >= 0.0)
		os << "\t\tPercent detailed tiles: " << percent_detailed << "%\n";

	os << "\tItem data:\n";
	os << "\t\tTotal number of items: " << item_count << "\n";
	os << "\t\tNumber of moveable tiles: " << loose_item_count << "\n";
	os << "\t\tNumber of depots: " << depot_count << "\n";
	os << "\t\tNumber of containers: " << container_count << "\n";
	os << "\t\tNumber of items with Action ID: " << action_item_count << "\n";
	os << "\t\tNumber of items with Unique ID: " << unique_item_count << "\n";

	os << "\tCreature data:\n";
	os << "\t\tTotal creature count: " << creature_count << "\n";
	os << "\t\tTotal spawn count: " << spawn_count << "\n";
	if(creatures_per_spawn >= 0)
		os << "\t\tMean creatures per spawn: " << creatures_per_spawn << "\n";

	os << "\tTown/House data:\n";
	os << "\t\tTotal number of towns: " << town_count << "\n";
	os << "\t\tTotal number of houses: " << house_count << "\n";
	if(houses_per_town >= 0)
		os << "\t\tMean houses per town: " << houses_per_town << "\n";
	os << "\t\tTotal amount of housetiles: " << total_house_sqm << "\n";
	if(sqm_per_house >= 0)
		os << "\t\tMean tiles per house: " << sqm_per_house << "\n";
	if(sqm_per_town >= 0)
		os << "\t\tMean tiles per town: " << sqm_per_town << "\n";

	if(largest_town)
		os << "\t\tLargest Town: \"" << largest_town->getName() << "\" (" << largest_town_size << " sqm)\n";
	if(largest_house)
		os << "\t\tLargest House: \"" << largest_house->name << "\" (" << largest_house_size << " sqm)\n";

	os << "\n";
	os << "Generated by Remere's Map Editor version " + __RME_VERSION__ + "\n";


    wxDialog* dg = newd wxDialog(frame, wxID_ANY, wxT("Map Statistics"), wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX);
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxTextCtrl* text_field = newd wxTextCtrl(dg, wxID_ANY, wxstr(os.str()), wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
	text_field->SetMinSize(wxSize(400, 300));
	topsizer->Add(text_field, wxSizerFlags(5).Expand());

	wxSizer* choicesizer = newd wxBoxSizer(wxHORIZONTAL);
	wxButton* export_button = newd wxButton(dg, wxID_OK, wxT("Export as XML"));
	choicesizer->Add(export_button, wxSizerFlags(1).Center());
	export_button->Enable(false);
	choicesizer->Add(newd wxButton(dg, wxID_CANCEL, wxT("OK")), wxSizerFlags(1).Center());
	topsizer->Add(choicesizer, wxSizerFlags(1).Center());
	dg->SetSizerAndFit(topsizer);
	dg->Centre(wxBOTH);

	int ret = dg->ShowModal();

	if(ret == wxID_OK) {
		//std::cout << "XML EXPORT";
	} else if(ret == wxID_CANCEL) {
		//std::cout << "OK";
	}
}

void MainMenuBar::OnMapCleanup(wxCommandEvent& WXUNUSED(event))
{
	int ok = gui.PopupDialog(wxT("Clean map"), wxT("Do you want to remove all invalid items from the map?"), wxYES | wxNO);

	if(ok == wxID_YES)
		gui.GetCurrentMap().cleanInvalidTiles(true);
}

void MainMenuBar::OnMapProperties(wxCommandEvent& WXUNUSED(event))
{
	wxDialog* properties = newd MapPropertiesWindow(
		frame,
		static_cast<MapTab*>(gui.GetCurrentTab()),
		*gui.GetCurrentEditor());

	if(properties->ShowModal() == 0) {
		// FAIL!
		gui.CloseAllEditors();
	}
	properties->Destroy();
}

void MainMenuBar::OnNewView(wxCommandEvent& WXUNUSED(event))
{
	gui.NewMapView();
}

void MainMenuBar::OnToggleFullscreen(wxCommandEvent& WXUNUSED(event))
{
	if(frame->IsFullScreen())
		frame->ShowFullScreen(false);
	else
		frame->ShowFullScreen(true, wxFULLSCREEN_NOBORDER | wxFULLSCREEN_NOCAPTION);
}

void MainMenuBar::OnTakeScreenshot(wxCommandEvent& WXUNUSED(event))
{
	wxString path = wxstr(settings.getString(Config::SCREENSHOT_DIRECTORY));
	if(path.size() > 0 && (path.Last() == wxT('/') || path.Last() == wxT('\\')))
		path = path + wxT("/");

	gui.GetCurrentMapTab()->GetView()->GetCanvas()->TakeScreenshot(
		path, wxstr(settings.getString(Config::SCREENSHOT_FORMAT))
	);

}

void MainMenuBar::OnZoomIn(wxCommandEvent& event)
{
	double zoom = gui.GetCurrentZoom();
	gui.SetCurrentZoom(zoom - 0.1);
}

void MainMenuBar::OnZoomOut(wxCommandEvent& event)
{
	double zoom = gui.GetCurrentZoom();
	gui.SetCurrentZoom(zoom + 0.1);
}

void MainMenuBar::OnZoomNormal(wxCommandEvent& event)
{
	gui.SetCurrentZoom(1.0);
}

void MainMenuBar::OnChangeViewSettings(wxCommandEvent& event)
{
	settings.setInteger(Config::SHOW_ALL_FLOORS, IsItemChecked(MenuBar::SHOW_ALL_FLOORS));
	if(IsItemChecked(MenuBar::SHOW_ALL_FLOORS)) {
		EnableItem(MenuBar::SELECT_MODE_VISIBLE, true);
		EnableItem(MenuBar::SELECT_MODE_LOWER, true);
	} else {
		EnableItem(MenuBar::SELECT_MODE_VISIBLE, false);
		EnableItem(MenuBar::SELECT_MODE_LOWER, false);
		CheckItem(MenuBar::SELECT_MODE_CURRENT, true);
		settings.setInteger(Config::SELECTION_TYPE, SELECT_CURRENT_FLOOR);
	}
	settings.setInteger(Config::TRANSPARENT_FLOORS, IsItemChecked(MenuBar::GHOST_HIGHER_FLOORS));
	settings.setInteger(Config::TRANSPARENT_ITEMS, IsItemChecked(MenuBar::GHOST_ITEMS));
	settings.setInteger(Config::SHOW_INGAME_BOX, IsItemChecked(MenuBar::SHOW_INGAME_BOX));
	settings.setInteger(Config::SHOW_GRID, IsItemChecked(MenuBar::SHOW_GRID));
	settings.setInteger(Config::SHOW_EXTRA, !IsItemChecked(MenuBar::SHOW_EXTRA));

	settings.setInteger(Config::SHOW_SHADE, IsItemChecked(MenuBar::SHOW_SHADE));
	settings.setInteger(Config::SHOW_SPECIAL_TILES, IsItemChecked(MenuBar::SHOW_SPECIAL));
	settings.setInteger(Config::SHOW_ONLY_TILEFLAGS, IsItemChecked(MenuBar::SHOW_ONLY_COLORS));
	settings.setInteger(Config::SHOW_ONLY_MODIFIED_TILES, IsItemChecked(MenuBar::SHOW_ONLY_MODIFIED));
	settings.setInteger(Config::SHOW_CREATURES, IsItemChecked(MenuBar::SHOW_CREATURES));
	settings.setInteger(Config::SHOW_SPAWNS, IsItemChecked(MenuBar::SHOW_SPAWNS));
	settings.setInteger(Config::SHOW_HOUSES, IsItemChecked(MenuBar::SHOW_HOUSES));
	settings.setInteger(Config::HIGHLIGHT_ITEMS, IsItemChecked(MenuBar::HIGHLIGHT_ITEMS));
	settings.setInteger(Config::SHOW_BLOCKING, IsItemChecked(MenuBar::SHOW_PATHING));

	gui.RefreshView();
}

void MainMenuBar::OnChangeFloor(wxCommandEvent& event)
{
	// Workaround to stop events from looping
	if(checking_programmaticly)
		return;

	for(int i = 0; i < 16; ++i) {
		if(IsItemChecked(MenuBar::ActionID(MenuBar::FLOOR_0 + i))) {
			gui.ChangeFloor(i);
		}
	}
}

void MainMenuBar::OnMinimapWindow(wxCommandEvent& event)
{
	gui.CreateMinimap();
}

void MainMenuBar::OnNewPalette(wxCommandEvent& event)
{
	gui.NewPalette();
}

void MainMenuBar::OnSelectTerrainPalette(wxCommandEvent& WXUNUSED(event))
{
	gui.SelectPalettePage(TILESET_TERRAIN);
}

void MainMenuBar::OnSelectDoodadPalette(wxCommandEvent& WXUNUSED(event))
{
	gui.SelectPalettePage(TILESET_DOODAD);
}

void MainMenuBar::OnSelectItemPalette(wxCommandEvent& WXUNUSED(event))
{
	gui.SelectPalettePage(TILESET_ITEM);
}

void MainMenuBar::OnSelectHousePalette(wxCommandEvent& WXUNUSED(event))
{
	gui.SelectPalettePage(TILESET_HOUSE);
}

void MainMenuBar::OnSelectCreaturePalette(wxCommandEvent& WXUNUSED(event))
{
	gui.SelectPalettePage(TILESET_CREATURE);
}

void MainMenuBar::OnSelectWaypointPalette(wxCommandEvent& WXUNUSED(event))
{
	gui.SelectPalettePage(TILESET_WAYPOINT);
}

void MainMenuBar::OnSelectRawPalette(wxCommandEvent& WXUNUSED(event))
{
	gui.SelectPalettePage(TILESET_RAW);
}

void MainMenuBar::OnStartLive(wxCommandEvent& event)
{
	Editor* editor = gui.GetCurrentEditor();
	if(!editor) {
		gui.PopupDialog(wxT("Error"), wxT("You need to have a map open to start a live mapping session."), wxOK);
		return;
	}
	if(editor->IsLive()) {
		gui.PopupDialog(wxT("Error"), wxT("You can not start two live servers on the same map (or a server using a remote map)."), wxOK);
		return;
	}

	wxDialog* live_host_dlg = newd wxDialog(frame, wxID_ANY, wxT("Host Live Server"), wxDefaultPosition, wxDefaultSize);

	wxSizer* top_sizer = newd wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer* gsizer = newd wxFlexGridSizer(2, 10, 10);
	gsizer->AddGrowableCol(0, 2);
	gsizer->AddGrowableCol(1, 3);

	// Data fields
	wxTextCtrl* hostname;
	wxSpinCtrl* port;
	wxTextCtrl* password;
	wxCheckBox* allow_copy;

	gsizer->Add(newd wxStaticText(live_host_dlg, wxID_ANY, wxT("Server Name:")));
	gsizer->Add(hostname = newd wxTextCtrl(live_host_dlg, wxID_ANY, wxT("RME Live Server")), 0, wxEXPAND);

	gsizer->Add(newd wxStaticText(live_host_dlg, wxID_ANY, wxT("Port:")));
	gsizer->Add(port = newd wxSpinCtrl(live_host_dlg, wxID_ANY, wxT("31313"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535, 31313), 0, wxEXPAND);

	gsizer->Add(newd wxStaticText(live_host_dlg, wxID_ANY, wxT("Password:")));
	gsizer->Add(password = newd wxTextCtrl(live_host_dlg, wxID_ANY), 0, wxEXPAND);

	top_sizer->Add(gsizer, 0, wxALL, 20);

	top_sizer->Add(allow_copy = newd wxCheckBox(live_host_dlg, wxID_ANY, wxT("Allow copy & paste between maps.")), 0, wxRIGHT | wxLEFT, 20);
	allow_copy->SetToolTip(wxT("Allows remote clients to copy & paste from the hosted map to local maps."));

	wxSizer* ok_sizer = newd wxBoxSizer(wxHORIZONTAL);
	ok_sizer->Add(newd wxButton(live_host_dlg, wxID_OK, wxT("OK")), 1, wxCENTER);
	ok_sizer->Add(newd wxButton(live_host_dlg, wxID_CANCEL, wxT("Cancel")), wxCENTER, 1);
	top_sizer->Add(ok_sizer, 0, wxCENTER | wxALL, 20);

	live_host_dlg->SetSizerAndFit(top_sizer);

	while(true) {
		int ret = live_host_dlg->ShowModal();
		if(ret == wxID_OK) {
			LiveServer* liveServer = editor->StartLiveServer();
			liveServer->setName(hostname->GetValue());
			liveServer->setPassword(password->GetValue());
			liveServer->setPort(port->GetValue());

			const wxString& error = liveServer->getLastError();
			if(!error.empty()) {
				gui.PopupDialog(live_host_dlg, wxT("Error"), error, wxOK);
				editor->CloseLiveServer();
				continue;
			}

			if(!liveServer->bind()) {
				gui.PopupDialog(wxT("Socket Error"), wxT("Could not bind socket! Try another port?"), wxOK);
				editor->CloseLiveServer();
			} else {
				liveServer->createLogWindow(gui.tabbook);
			}
			break;
		} else
			break;
	}
	live_host_dlg->Destroy();
	Update();
}

void MainMenuBar::OnJoinLive(wxCommandEvent& event)
{
	wxDialog* live_join_dlg = newd wxDialog(frame, wxID_ANY, wxT("Join Live Server"), wxDefaultPosition, wxDefaultSize);

	wxSizer* top_sizer = newd wxBoxSizer(wxVERTICAL);
	wxFlexGridSizer* gsizer = newd wxFlexGridSizer(2, 10, 10);
	gsizer->AddGrowableCol(0, 2);
	gsizer->AddGrowableCol(1, 3);

	// Data fields
	wxTextCtrl* name;
	wxTextCtrl* ip;
	wxSpinCtrl* port;
	wxTextCtrl* password;

	gsizer->Add(newd wxStaticText(live_join_dlg, wxID_ANY, wxT("Name:")));
	gsizer->Add(name = newd wxTextCtrl(live_join_dlg, wxID_ANY, wxT("")), 0, wxEXPAND);

	gsizer->Add(newd wxStaticText(live_join_dlg, wxID_ANY, wxT("IP:")));
	gsizer->Add(ip = newd wxTextCtrl(live_join_dlg, wxID_ANY, wxT("localhost")), 0, wxEXPAND);

	gsizer->Add(newd wxStaticText(live_join_dlg, wxID_ANY, wxT("Port:")));
	gsizer->Add(port = newd wxSpinCtrl(live_join_dlg, wxID_ANY, wxT("31313"), wxDefaultPosition, wxDefaultSize, wxSP_ARROW_KEYS, 1, 65535, 31313), 0, wxEXPAND);

	gsizer->Add(newd wxStaticText(live_join_dlg, wxID_ANY, wxT("Password:")));
	gsizer->Add(password = newd wxTextCtrl(live_join_dlg, wxID_ANY), 0, wxEXPAND);

	top_sizer->Add(gsizer, 0, wxALL, 20);

	wxSizer* ok_sizer = newd wxBoxSizer(wxHORIZONTAL);
	ok_sizer->Add(newd wxButton(live_join_dlg, wxID_OK, wxT("OK")), 1, wxRIGHT);
	ok_sizer->Add(newd wxButton(live_join_dlg, wxID_CANCEL, wxT("Cancel")), 1, wxRIGHT);
	top_sizer->Add(ok_sizer, 0, wxCENTER | wxALL, 20);

	live_join_dlg->SetSizerAndFit(top_sizer);

	while(true) {
		int ret = live_join_dlg->ShowModal();
		if(ret == wxID_OK) {
			LiveClient* liveClient = newd LiveClient();
			liveClient->setPassword(password->GetValue());

			wxString tmp = name->GetValue();
			if(tmp.empty()) {
				tmp = wxT("User");
			}
			liveClient->setName(tmp);

			const wxString& error = liveClient->getLastError();
			if(!error.empty()) {
				gui.PopupDialog(live_join_dlg, wxT("Error"), error, wxOK);
				delete liveClient;
				continue;
			}

			const wxString& address = ip->GetValue();
			int32_t portNumber = port->GetValue();

			liveClient->createLogWindow(gui.tabbook);
			if(!liveClient->connect(nstr(address), portNumber)) {
				gui.PopupDialog(wxT("Connection Error"), liveClient->getLastError(), wxOK);
				delete liveClient;
			}

			break;
		} else
			break;
	}
	live_join_dlg->Destroy();
	Update();
}

void MainMenuBar::OnCloseLive(wxCommandEvent& event)
{
	Editor* editor = gui.GetCurrentEditor();
	if(editor && editor->IsLive())
		gui.CloseLiveEditors(&editor->GetLive());

	Update();
}
