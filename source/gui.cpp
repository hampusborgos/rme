//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include <wx/display.h>

#include "gui.h"
#include "main_menubar.h"

#include "editor.h"
#include "brush.h"
#include "map.h"
#include "sprites.h"
#include "materials.h"
#include "doodad_brush.h"
#include "spawn_brush.h"

#include "common_windows.h"
#include "result_window.h"
#include "minimap_window.h"
#include "palette_window.h"
#include "map_display.h"
#include "application.h"

#include "live_client.h"
#include "live_tab.h"
#include "live_server.h"

#ifdef __WXOSX__
#include <AGL/agl.h>
#endif

const wxEventType EVT_UPDATE_MENUS = wxNewEventType();

// Global GUI instance
GUI g_gui;

// GUI class implementation
GUI::GUI() :
	aui_manager(nullptr),
	root(nullptr),
	minimap(nullptr),
	gem(nullptr),
	search_result_window(nullptr),
	secondary_map(nullptr),
	doodad_buffer_map(nullptr),

	house_brush(nullptr),
	house_exit_brush(nullptr),
	waypoint_brush(nullptr),
	optional_brush(nullptr),
	eraser(nullptr),
	normal_door_brush(nullptr),
	locked_door_brush(nullptr),
	magic_door_brush(nullptr),
	quest_door_brush(nullptr),
	hatch_door_brush(nullptr),
	window_door_brush(nullptr),

	OGLContext(nullptr),
	loaded_version(CLIENT_VERSION_NONE),
	mode(SELECTION_MODE),
	pasting(false),
	hotkeys_enabled(true),

	current_brush(nullptr),
	previous_brush(nullptr),
	brush_shape(BRUSHSHAPE_SQUARE),
	brush_size(0),
	brush_variation(0),

	creature_spawntime(0),
	use_custom_thickness(false),
	custom_thickness_mod(0.0),
	progressBar(nullptr),
	disabled_counter(0)
{
	doodad_buffer_map = newd BaseMap();
}

GUI::~GUI()
{
	delete doodad_buffer_map;
	delete g_gui.aui_manager;
	delete OGLContext;
}

wxGLContext* GUI::GetGLContext(wxGLCanvas* win)
{
	if(OGLContext == nullptr) {
#ifdef __WXOSX__
        /*
        wxGLContext(AGLPixelFormat fmt, wxGLCanvas *win,
                    const wxPalette& WXUNUSED(palette),
                    const wxGLContext *other
                    );
        */
		OGLContext = new wxGLContext(win, nullptr);
#else
		OGLContext = newd wxGLContext(win);
#endif
    }

	return OGLContext;
}

wxString GUI::GetDataDirectory()
{
	std::string cfg_str = g_settings.getString(Config::DATA_DIRECTORY);
	if(cfg_str.size()) {
		FileName dir;
		dir.Assign(wxstr(cfg_str));
		wxString path;
		if(dir.DirExists()) {
			path = dir.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
			return path;
		}
	}

	// Silently reset directory
	FileName exec_directory;
	try
	{
		exec_directory = dynamic_cast<wxStandardPaths&>(wxStandardPaths::Get()).GetExecutablePath();
	}
	catch(std::bad_cast)
	{
		throw; // Crash application (this should never happend anyways...)
	}

	exec_directory.AppendDir("data");
	return exec_directory.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
}

wxString GUI::GetExecDirectory()
{
	// Silently reset directory
	FileName exec_directory;
	try
	{
		exec_directory = dynamic_cast<wxStandardPaths&>(wxStandardPaths::Get()).GetExecutablePath();
	}
	catch(std::bad_cast)
	{
		wxLogError("Could not fetch executable directory.");
	}
	return exec_directory.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
}

wxString GUI::GetLocalDataDirectory()
{
	if(g_settings.getInteger(Config::INDIRECTORY_INSTALLATION)) {
		FileName dir = GetDataDirectory();
		dir.AppendDir("user");
		dir.AppendDir("data");
		dir.Mkdir(0755, wxPATH_MKDIR_FULL);
		return dir.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);;
	} else {
		FileName dir = dynamic_cast<wxStandardPaths&>(wxStandardPaths::Get()).GetUserDataDir();
#ifdef __WINDOWS__
		dir.AppendDir("Remere's Map Editor");
#else
		dir.AppendDir(".rme");
#endif
		dir.AppendDir("data");
		dir.Mkdir(0755, wxPATH_MKDIR_FULL);
		return dir.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
	}
}

wxString GUI::GetLocalDirectory()
{
	if(g_settings.getInteger(Config::INDIRECTORY_INSTALLATION)) {
		FileName dir = GetDataDirectory();
		dir.AppendDir("user");
		dir.Mkdir(0755, wxPATH_MKDIR_FULL);
		return dir.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);;
	} else {
		FileName dir = dynamic_cast<wxStandardPaths&>(wxStandardPaths::Get()).GetUserDataDir();
#ifdef __WINDOWS__
		dir.AppendDir("Remere's Map Editor");
#else
		dir.AppendDir(".rme");
#endif
		dir.Mkdir(0755, wxPATH_MKDIR_FULL);
		return dir.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
	}
}

wxString GUI::GetExtensionsDirectory()
{
	std::string cfg_str = g_settings.getString(Config::EXTENSIONS_DIRECTORY);
	if(cfg_str.size()) {
		FileName dir;
		dir.Assign(wxstr(cfg_str));
		wxString path;
		if(dir.DirExists()) {
			path = dir.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
			return path;
		}
	}

	// Silently reset directory
	FileName local_directory = GetLocalDirectory();
	local_directory.AppendDir("extensions");
	local_directory.Mkdir(0755, wxPATH_MKDIR_FULL);
	return local_directory.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
}

void GUI::discoverDataDirectory(const wxString& existentFile)
{
	wxString currentDir = wxGetCwd();
	wxString execDir = GetExecDirectory();

	wxString possiblePaths[] = {
		execDir,
		currentDir + "/",

		// these are used usually when running from build directories
		execDir + "/../",
		execDir + "/../../",
		currentDir + "/../",
	};

	bool found = false;
	for(const wxString& path : possiblePaths) {
		if(wxFileName(path + "data/" + existentFile).FileExists()) {
			m_dataDirectory = path + "data/";
			found = true;
			break;
		}
	}

	if(!found)
		wxLogError(wxString() + "Could not find data directory.\n");
}

bool GUI::LoadVersion(ClientVersionID version, wxString& error, wxArrayString& warnings, bool force)
{
	if(ClientVersion::get(version) == nullptr) {
		error = "Unsupported client version! (8)";
		return false;
	}

	if(version != loaded_version || force) {
		if(getLoadedVersion() != nullptr)
			// There is another version loaded right now, save window layout
			g_gui.SavePerspective();

		// Disable all rendering so the data is not accessed while reloading
		UnnamedRenderingLock();
		DestroyPalettes();
		DestroyMinimap();

		// Destroy the previous version
		UnloadVersion();

		loaded_version = version;
		if(!getLoadedVersion()->hasValidPaths()) {
			if(!getLoadedVersion()->loadValidPaths()) {
				error = "Couldn't load relevant data files";
				loaded_version = CLIENT_VERSION_NONE;
				return false;
			}
		}

		bool ret = LoadDataFiles(error, warnings);
		if(ret)
			g_gui.LoadPerspective();
		else
			loaded_version = CLIENT_VERSION_NONE;

		return ret;
	}
	return true;
}

void GUI::EnableHotkeys()
{
	hotkeys_enabled = true;
}

void GUI::DisableHotkeys()
{
	hotkeys_enabled = false;
}

bool GUI::AreHotkeysEnabled() const
{
	return hotkeys_enabled;
}

ClientVersionID GUI::GetCurrentVersionID() const
{
	if(loaded_version != CLIENT_VERSION_NONE) {
		return getLoadedVersion()->getID();
	}
	return CLIENT_VERSION_NONE;
}

const ClientVersion& GUI::GetCurrentVersion() const
{
	assert(loaded_version);
	return *getLoadedVersion();
}

void GUI::CycleTab(bool forward)
{
	tabbook->CycleTab(forward);
}

bool GUI::LoadDataFiles(wxString& error, wxArrayString& warnings)
{
	FileName data_path = getLoadedVersion()->getDataPath();
	FileName client_path = getLoadedVersion()->getClientPath();
	FileName extension_path = GetExtensionsDirectory();

	FileName exec_directory;
	try
	{
		exec_directory = dynamic_cast<wxStandardPaths&>(wxStandardPaths::Get()).GetExecutablePath();
	}
	catch(std::bad_cast)
	{
		error = "Couldn't establish working directory...";
		return false;
	}

	g_gui.gfx.client_version = getLoadedVersion();

	FileName otfi_path = wxString(client_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + wxString(ASSETS_NAME) + ".otfi");
	if(!g_gui.gfx.loadOTFI(otfi_path, error, warnings)) {
		error = "Couldn't load otfi file: " + error;
		g_gui.DestroyLoadBar();
		UnloadVersion();
		return false;
	}

	g_gui.CreateLoadBar("Loading asset files");
	g_gui.SetLoadDone(0, "Loading metadata file...");
	FileName dat_path = wxString(client_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + wxString(ASSETS_NAME) + ".dat");

	if(!g_gui.gfx.loadSpriteMetadata(dat_path, error, warnings)) {
		error = "Couldn't load metadata: " + error;
		g_gui.DestroyLoadBar();
		UnloadVersion();
		return false;
	}

	FileName spr_path = wxString(client_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + wxString(ASSETS_NAME) + ".spr");

	g_gui.SetLoadDone(10, "Loading sprites file...");
	if(!g_gui.gfx.loadSpriteData(spr_path.GetFullPath(), error, warnings)) {
		error = "Couldn't load sprites: " + error;
		g_gui.DestroyLoadBar();
		UnloadVersion();
		return false;
	}

	g_gui.SetLoadDone(20, "Loading items.otb file...");
	if(!g_items.loadFromOtb(wxString(data_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + "items.otb"), error, warnings)) {
		error = "Couldn't load items.otb: " + error;
		g_gui.DestroyLoadBar();
		UnloadVersion();
		return false;
	}

	g_gui.SetLoadDone(30, "Loading items.xml ...");
	if(!g_items.loadFromGameXml(wxString(data_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + "items.xml"), error, warnings)) {
		warnings.push_back("Couldn't load items.xml: " + error);
	}

	g_gui.SetLoadDone(45, "Loading creatures.xml ...");
	if(!g_creatures.loadFromXML(wxString(data_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + "creatures.xml"), true, error, warnings)) {
		warnings.push_back("Couldn't load creatures.xml: " + error);
	}

	g_gui.SetLoadDone(45, "Loading user creatures.xml ...");
	{
		FileName cdb = getLoadedVersion()->getLocalDataPath();
		cdb.SetFullName("creatures.xml");
		wxString nerr;
		wxArrayString nwarn;
		g_creatures.loadFromXML(cdb, false, nerr, nwarn);
	}

	g_gui.SetLoadDone(50, "Loading materials.xml ...");
	if(!g_materials.loadMaterials(wxString(data_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + "materials.xml"), error, warnings)) {
		warnings.push_back("Couldn't load materials.xml: " + error);
	}

	g_gui.SetLoadDone(70, "Loading extensions...");
	if(!g_materials.loadExtensions(extension_path, error, warnings)) {
		//warnings.push_back("Couldn't load extensions: " + error);
	}

	g_gui.SetLoadDone(70, "Finishing...");
	g_brushes.init();
	g_materials.createOtherTileset();

	g_gui.DestroyLoadBar();
	return true;
}

void GUI::UnloadVersion()
{
	UnnamedRenderingLock();
	gfx.clear();
	current_brush = nullptr;
	previous_brush = nullptr;

	house_brush = nullptr;
	house_exit_brush = nullptr;
	waypoint_brush = nullptr;
	optional_brush = nullptr;
	eraser = nullptr;
	normal_door_brush = nullptr;
	locked_door_brush = nullptr;
	magic_door_brush = nullptr;
	quest_door_brush = nullptr;
	hatch_door_brush = nullptr;
	window_door_brush = nullptr;

	if(loaded_version != CLIENT_VERSION_NONE) {
		//g_gui.UnloadVersion();
		g_materials.clear();
		g_brushes.clear();
		g_items.clear();
		gfx.clear();

		FileName cdb = getLoadedVersion()->getLocalDataPath();
		cdb.SetFullName("creatures.xml");
		g_creatures.saveToXML(cdb);
		g_creatures.clear();

		loaded_version = CLIENT_VERSION_NONE;
	}
}

void GUI::SaveCurrentMap(FileName filename, bool showdialog)
{
	MapTab* mapTab = GetCurrentMapTab();
	if(mapTab) {
		Editor* editor = mapTab->GetEditor();
		if(editor) {
			editor->saveMap(filename, showdialog);

			const std::string& filename = editor->map.getFilename();
			const Position& position = mapTab->GetScreenCenterPosition();
			std::ostringstream stream;
			stream << position;
			g_settings.setString(Config::RECENT_EDITED_MAP_PATH, filename);
			g_settings.setString(Config::RECENT_EDITED_MAP_POSITION, stream.str());
		}
	}

	UpdateTitle();
	root->UpdateMenubar();
	root->Refresh();
}

bool GUI::IsEditorOpen() const
{
	return tabbook != nullptr && GetCurrentMapTab();
}

double GUI::GetCurrentZoom()
{
	MapTab* tab = GetCurrentMapTab();
	if(tab)
		return tab->GetCanvas()->GetZoom();
	return 1.0;
}

void GUI::SetCurrentZoom(double zoom)
{
	MapTab* tab = GetCurrentMapTab();
	if(tab)
		tab->GetCanvas()->SetZoom(zoom);
}

void GUI::FitViewToMap()
{
	for(int index = 0; index < tabbook->GetTabCount(); ++index) {
		if(MapTab* tab = dynamic_cast<MapTab*>(tabbook->GetTab(index))) {
			tab->GetView()->FitToMap();
		}
	}
}

void GUI::FitViewToMap(MapTab* mt)
{
	for(int index = 0; index < tabbook->GetTabCount(); ++index) {
		if(MapTab* tab = dynamic_cast<MapTab*>(tabbook->GetTab(index))) {
			if(tab->HasSameReference(mt)) {
				tab->GetView()->FitToMap();
			}
		}
	}
}

bool GUI::NewMap()
{
	Editor* editor;
	try
	{
		editor = newd Editor(copybuffer);
	}
	catch(std::runtime_error& e)
	{
		PopupDialog(root, "Error!", wxString(e.what(), wxConvUTF8), wxOK);
		return false;
	}

	MapTab* mapTab = newd MapTab(tabbook, editor);
	mapTab->OnSwitchEditorMode(mode);

	SetStatusText("Created new map");
	UpdateTitle();
	RefreshPalettes();
	root->UpdateMenubar();
	root->Refresh();

	return true;
}

void GUI::OpenMap()
{
	wxString wildcard = g_settings.getInteger(Config::USE_OTGZ) != 0 ? MAP_LOAD_FILE_WILDCARD_OTGZ : MAP_LOAD_FILE_WILDCARD;
	wxFileDialog dialog(root, "Open map file", wxEmptyString, wxEmptyString, wildcard, wxFD_OPEN | wxFD_FILE_MUST_EXIST);

	if (dialog.ShowModal() == wxID_OK)
		LoadMap(dialog.GetPath());
}

void GUI::SaveMap()
{
	if (!IsEditorOpen())
		return;

	if (GetCurrentMap().hasFile()) {
		SaveCurrentMap(true);
	} else {
		wxString wildcard = g_settings.getInteger(Config::USE_OTGZ) != 0 ? MAP_SAVE_FILE_WILDCARD_OTGZ : MAP_SAVE_FILE_WILDCARD;
		wxFileDialog dialog(root, "Save...", wxEmptyString, wxEmptyString, wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

		if (dialog.ShowModal() == wxID_OK)
			SaveCurrentMap(dialog.GetPath(), true);
	}
}

void GUI::SaveMapAs()
{
	if (!IsEditorOpen())
		return;

	wxString wildcard = g_settings.getInteger(Config::USE_OTGZ) != 0 ? MAP_SAVE_FILE_WILDCARD_OTGZ : MAP_SAVE_FILE_WILDCARD;
	wxFileDialog dialog(root, "Save As...", "", "", wildcard, wxFD_SAVE | wxFD_OVERWRITE_PROMPT);

	if (dialog.ShowModal() == wxID_OK) {
		SaveCurrentMap(dialog.GetPath(), true);
		UpdateTitle();
		root->menu_bar->AddRecentFile(dialog.GetPath());
		root->UpdateMenubar();
	}
}

bool GUI::LoadMap(const FileName& fileName)
{
	if(GetCurrentEditor() && !GetCurrentMap().hasChanged() && !GetCurrentMap().hasFile())
		g_gui.CloseCurrentEditor();

	Editor* editor;
	try
	{
		editor = newd Editor(copybuffer, fileName);
	}
	catch(std::runtime_error& e)
	{
		PopupDialog(root, "Error!", wxString(e.what(), wxConvUTF8), wxOK);
		return false;
	}

	MapTab* mapTab = newd MapTab(tabbook, editor);
	mapTab->OnSwitchEditorMode(mode);

	root->AddRecentFile(fileName);

	mapTab->GetView()->FitToMap();
	UpdateTitle();
	ListDialog("Map loader errors", mapTab->GetMap()->getWarnings());
	root->DoQueryImportCreatures();

	FitViewToMap(mapTab);
	root->UpdateMenubar();

	std::string path = g_settings.getString(Config::RECENT_EDITED_MAP_PATH);
	if(!path.empty()) {
		FileName file(path);
		if(file == fileName) {
			std::istringstream stream(g_settings.getString(Config::RECENT_EDITED_MAP_POSITION));
			Position position;
			stream >> position;
			mapTab->SetScreenCenterPosition(position);
		}
	}
	return true;
}

Editor* GUI::GetCurrentEditor()
{
	MapTab* mapTab = GetCurrentMapTab();
	if(mapTab)
		return mapTab->GetEditor();
	return nullptr;
}

EditorTab* GUI::GetTab(int idx)
{
	return tabbook->GetTab(idx);
}

int GUI::GetTabCount() const
{
	return tabbook->GetTabCount();
}

EditorTab* GUI::GetCurrentTab()
{
	return tabbook->GetCurrentTab();
}

MapTab* GUI::GetCurrentMapTab() const
{
	if(tabbook && tabbook->GetTabCount() > 0) {
		EditorTab* editorTab = tabbook->GetCurrentTab();
		MapTab* mapTab = dynamic_cast<MapTab*>(editorTab);
		return mapTab;
	}
	return nullptr;
}

Map& GUI::GetCurrentMap()
{
	Editor* editor = GetCurrentEditor();
	ASSERT(editor);
	return editor->map;
}

int GUI::GetOpenMapCount()
{
	std::set<Map*> open_maps;

	for(int i = 0; i < tabbook->GetTabCount(); ++i) {
		MapTab* tab = dynamic_cast<MapTab*>(tabbook->GetTab(i));
		if(tab)
			open_maps.insert(open_maps.begin(), tab->GetMap());

	}

	return open_maps.size();
}

bool GUI::ShouldSave()
{
	const Map& map = GetCurrentMap();
	if(map.hasChanged()) {
		if(map.getTileCount() == 0) {
			Editor* editor = GetCurrentEditor();
			ASSERT(editor);
			return editor->actionQueue->canUndo();
		}
		return true;
	}
	return false;

}

void GUI::AddPendingCanvasEvent(wxEvent& event)
{
	MapTab* mapTab = GetCurrentMapTab();
	if(mapTab)
		mapTab->GetCanvas()->GetEventHandler()->AddPendingEvent(event);
}

void GUI::CloseCurrentEditor()
{
	RefreshPalettes();
	tabbook->DeleteTab(tabbook->GetSelection());
	root->UpdateMenubar();
}

bool GUI::CloseLiveEditors(LiveSocket* sock)
{
	for(int i = 0; i < tabbook->GetTabCount(); ++i) {
		MapTab* mapTab = dynamic_cast<MapTab*>(tabbook->GetTab(i));
		if(mapTab) {
			Editor* editor = mapTab->GetEditor();
			if(editor->GetLiveClient() == sock)
				tabbook->DeleteTab(i--);
		}
		LiveLogTab* liveLogTab = dynamic_cast<LiveLogTab*>(tabbook->GetTab(i));
		if(liveLogTab) {
			if(liveLogTab->GetSocket() == sock) {
				liveLogTab->Disconnect();
				tabbook->DeleteTab(i--);
			}
		}
	}
	root->UpdateMenubar();
	return true;
}


bool GUI::CloseAllEditors()
{
	for(int i = 0; i < tabbook->GetTabCount(); ++i) {
		MapTab* mapTab = dynamic_cast<MapTab*>(tabbook->GetTab(i));
		if(mapTab) {
			if(mapTab->IsUniqueReference() && mapTab->GetMap() && mapTab->GetMap()->hasChanged()) {
				tabbook->SetFocusedTab(i);
				if(!root->DoQuerySave(false)) {
					return false;
				} else {
					RefreshPalettes();
					tabbook->DeleteTab(i--);
				}
			} else {
				tabbook->DeleteTab(i--);
			}
		}
	}
	if(root)
		root->UpdateMenubar();
	return true;
}

void GUI::NewMapView()
{
	MapTab* mapTab = GetCurrentMapTab();
	if(mapTab) {
		MapTab* newMapTab = newd MapTab(mapTab);
		newMapTab->OnSwitchEditorMode(mode);

		SetStatusText("Created newd view");
		UpdateTitle();
		RefreshPalettes();
		root->UpdateMenubar();
		root->Refresh();
	}
}

void GUI::LoadPerspective()
{
	if(!IsVersionLoaded()) {
		if(g_settings.getInteger(Config::WINDOW_MAXIMIZED)) {
			root->Maximize();
		} else {
			root->SetSize(wxSize(
				g_settings.getInteger(Config::WINDOW_WIDTH),
				g_settings.getInteger(Config::WINDOW_HEIGHT)
			));
		}
	} else {
		std::string tmp;
		std::string layout = g_settings.getString(Config::PALETTE_LAYOUT);

		std::vector<std::string> palette_list;
		for(char c : layout) {
			if(c == '|') {
				palette_list.push_back(tmp);
				tmp.clear();
			} else {
				tmp.push_back(c);
			}
		}

		if(!tmp.empty()) {
			palette_list.push_back(tmp);
		}

		for(const std::string& name : palette_list) {
			PaletteWindow* palette = CreatePalette();

			wxAuiPaneInfo& info = aui_manager->GetPane(palette);
			aui_manager->LoadPaneInfo(wxstr(name), info);

			if(info.IsFloatable()) {
				bool offscreen = true;
				for(uint32_t index = 0; index < wxDisplay::GetCount(); ++index) {
					wxDisplay display(index);
					wxRect rect = display.GetClientArea();
					if(rect.Contains(info.floating_pos)) {
						offscreen = false;
						break;
					}
				}

				if(offscreen) {
					info.Dock();
				}
			}
		}

		if(g_settings.getInteger(Config::MINIMAP_VISIBLE)) {
			if(!minimap) {
				wxAuiPaneInfo info;

				const wxString& data = wxstr(g_settings.getString(Config::MINIMAP_LAYOUT));
				aui_manager->LoadPaneInfo(data, info);

				minimap = newd MinimapWindow(root);
				aui_manager->AddPane(minimap, info);
			} else {
				wxAuiPaneInfo& info = aui_manager->GetPane(minimap);

				const wxString& data = wxstr(g_settings.getString(Config::MINIMAP_LAYOUT));
				aui_manager->LoadPaneInfo(data, info);
			}

			wxAuiPaneInfo& info = aui_manager->GetPane(minimap);
			if(info.IsFloatable()) {
				bool offscreen = true;
				for(uint32_t index = 0; index < wxDisplay::GetCount(); ++index) {
					wxDisplay display(index);
					wxRect rect = display.GetClientArea();
					if(rect.Contains(info.floating_pos)) {
						offscreen = false;
						break;
					}
				}

				if(offscreen) {
					info.Dock();
				}
			}
		}

		aui_manager->Update();
		root->UpdateMenubar();
	}
}

void GUI::SavePerspective()
{
	g_settings.setInteger(Config::WINDOW_MAXIMIZED, root->IsMaximized());
	g_settings.setInteger(Config::WINDOW_WIDTH, root->GetSize().GetWidth());
	g_settings.setInteger(Config::WINDOW_HEIGHT, root->GetSize().GetHeight());

	g_settings.setInteger(Config::MINIMAP_VISIBLE, minimap? 1: 0);

	wxString pinfo;
	for(PaletteList::iterator piter = palettes.begin(); piter != palettes.end(); ++piter) {
		if(aui_manager->GetPane(*piter).IsShown())
			pinfo << aui_manager->SavePaneInfo(aui_manager->GetPane(*piter)) << "|";
	}
	g_settings.setString(Config::PALETTE_LAYOUT, nstr(pinfo));

	if(minimap) {
		wxString s = aui_manager->SavePaneInfo(aui_manager->GetPane(minimap));
		g_settings.setString(Config::MINIMAP_LAYOUT, nstr(s));
	}
}

void GUI::HideSearchWindow()
{
	if(search_result_window) {
		aui_manager->GetPane(search_result_window).Show(false);
		aui_manager->Update();
	}
}

SearchResultWindow* GUI::ShowSearchWindow()
{
	if(search_result_window == nullptr) {
		search_result_window = newd SearchResultWindow(root);
		aui_manager->AddPane(search_result_window, wxAuiPaneInfo().Caption("Search Results"));
	} else {
		aui_manager->GetPane(search_result_window).Show();
	}
	aui_manager->Update();
	return search_result_window;
}

//=============================================================================
// Palette Window Interface implementation

PaletteWindow* GUI::GetPalette()
{
	if(palettes.empty())
		return nullptr;
	return palettes.front();
}

PaletteWindow* GUI::NewPalette()
{
	return CreatePalette();
}

void GUI::RefreshPalettes(Map* m, bool usedefault)
{
	for(PaletteList::iterator piter = palettes.begin(); piter != palettes.end(); ++piter) {
		(*piter)->OnUpdate(m? m : (usedefault? (IsEditorOpen()? &GetCurrentMap() : nullptr): nullptr));
	}
	SelectBrush();
}

void GUI::RefreshOtherPalettes(PaletteWindow* p)
{
	for(PaletteList::iterator piter = palettes.begin(); piter != palettes.end(); ++piter) {
		if(*piter != p)
			(*piter)->OnUpdate(IsEditorOpen()? &GetCurrentMap() : nullptr);
	}
	SelectBrush();
}

PaletteWindow* GUI::CreatePalette()
{
	if(!IsVersionLoaded())
		return nullptr;

	PaletteWindow* palette = newd PaletteWindow(root, g_materials.tilesets);
	aui_manager->AddPane(palette, wxAuiPaneInfo().Caption("Palette").TopDockable(false).BottomDockable(false));
	aui_manager->Update();

	// Make us the active palette
	palettes.push_front(palette);
	// Select brush from this palette
	SelectBrushInternal(palette->GetSelectedBrush());

	return palette;
}

void GUI::ActivatePalette(PaletteWindow* p)
{
	palettes.erase(std::find(palettes.begin(), palettes.end(), p));
	palettes.push_front(p);
}

void GUI::DestroyPalettes()
{
	for(PaletteList::iterator piter = palettes.begin(); piter != palettes.end(); ++piter) {
		PaletteWindow* palette = *piter;
		aui_manager->DetachPane(palette);
		palette->Destroy();
		palette = nullptr;
	}
	palettes.clear();
	aui_manager->Update();
}

void GUI::RebuildPalettes()
{
	// Palette lits might be modified due to active palette changes
	// Use a temporary list for iterating
	PaletteList tmp = palettes;
	for(PaletteList::iterator piter = tmp.begin(); piter != tmp.end(); ++piter) {
		(*piter)->ReloadSettings(IsEditorOpen()? &GetCurrentMap() : nullptr);
	}
	aui_manager->Update();
}

void GUI::ShowPalette()
{
	if(palettes.empty())
		return;

	for(PaletteList::iterator piter = palettes.begin(); piter != palettes.end(); ++piter) {
		if(aui_manager->GetPane(*piter).IsShown())
			return;
	}

	aui_manager->GetPane(palettes.front()).Show(true);
	aui_manager->Update();
}

void GUI::SelectPalettePage(PaletteType pt)
{
	if(palettes.empty())
		CreatePalette();
	PaletteWindow* p = GetPalette();
	if(!p)
		return;

	ShowPalette();
	p->SelectPage(pt);
	aui_manager->Update();
	SelectBrushInternal(p->GetSelectedBrush());
}

//=============================================================================
// Minimap Window Interface Implementation

void GUI::CreateMinimap()
{
	if(!IsVersionLoaded())
		return;

	if(minimap) {
		aui_manager->GetPane(minimap).Show(true);
	} else {
		minimap = newd MinimapWindow(root);
		minimap->Show(true);
		aui_manager->AddPane(minimap, wxAuiPaneInfo().Caption("Minimap"));
	}
	aui_manager->Update();
}

void GUI::HideMinimap()
{
	if(minimap) {
		aui_manager->GetPane(minimap).Show(false);
		aui_manager->Update();
	}
}

void GUI::DestroyMinimap()
{
	if(minimap) {
		aui_manager->DetachPane(minimap);
		aui_manager->Update();
		minimap->Destroy();
		minimap = nullptr;
	}
}

void GUI::UpdateMinimap(bool immediate)
{
	if(IsMinimapVisible()) {
		if(immediate) {
			minimap->Refresh();
		} else {
			minimap->DelayedUpdate();
		}
	}
}

bool GUI::IsMinimapVisible() const
{
	if(minimap) {
		const wxAuiPaneInfo& pi = aui_manager->GetPane(minimap);
		if(pi.IsShown()) {
			return true;
		}
	}
	return false;
}

//=============================================================================

void GUI::RefreshView()
{
	EditorTab* editorTab = GetCurrentTab();
	if(!editorTab) {
		return;
	}

	if(!dynamic_cast<MapTab*>(editorTab)) {
		editorTab->GetWindow()->Refresh();
		return;
	}

	std::vector<EditorTab*> editorTabs;
	for(int32_t index = 0; index < tabbook->GetTabCount(); ++index) {
		MapTab* mapTab = dynamic_cast<MapTab*>(tabbook->GetTab(index));
		if(mapTab) {
			editorTabs.push_back(mapTab);
		}
	}

	for(EditorTab* editorTab : editorTabs) {
		editorTab->GetWindow()->Refresh();
	}
}

void GUI::CreateLoadBar(wxString message, bool canCancel /* = false */ )
{
	progressText = message;

	progressFrom = 0;
	progressTo = 100;
	currentProgress = -1;

	progressBar = newd wxGenericProgressDialog("Loading", progressText + " (0%)", 100, root,
		wxPD_APP_MODAL | wxPD_SMOOTH | (canCancel ? wxPD_CAN_ABORT : 0)
	);
	progressBar->SetSize(280, -1);
	progressBar->Show(true);

	for(int idx = 0; idx < tabbook->GetTabCount(); ++idx) {
		MapTab* mt = dynamic_cast<MapTab*>(tabbook->GetTab(idx));
		if(mt && mt->GetEditor()->IsLiveServer())
			mt->GetEditor()->GetLiveServer()->startOperation(progressText);
	}
	progressBar->Update(0);
}

void GUI::SetLoadScale(int32_t from, int32_t to)
{
	progressFrom = from;
	progressTo = to;
}

bool GUI::SetLoadDone(int32_t done, const wxString& newMessage)
{
	if(done == 100) {
		DestroyLoadBar();
		return true;
	} else if(done == currentProgress) {
		return true;
	}

	if(!newMessage.empty()) {
		progressText = newMessage;
	}

	int32_t newProgress = progressFrom + static_cast<int32_t>((done / 100.f) * (progressTo - progressFrom));
	newProgress = std::max<int32_t>(0, std::min<int32_t>(100, newProgress));

	bool skip = false;
	if(progressBar) {
		progressBar->Update(
			newProgress,
			wxString::Format("%s (%d%%)", progressText, newProgress),
			&skip
		);
		currentProgress = newProgress;
	}

	for(int32_t index = 0; index < tabbook->GetTabCount(); ++index) {
		MapTab* mapTab = dynamic_cast<MapTab*>(tabbook->GetTab(index));
		if(mapTab && mapTab->GetEditor()) {
			LiveServer* server = mapTab->GetEditor()->GetLiveServer();
			if(server) {
				server->updateOperation(newProgress);
			}
		}
	}

	return skip;
}

void GUI::DestroyLoadBar()
{
	if(progressBar) {
		progressBar->Show(false);
		currentProgress = -1;

		progressBar->Destroy();
		progressBar = nullptr;

		if(root->IsActive()) {
			root->Raise();
		} else {
			root->RequestUserAttention();
		}
	}
}

void GUI::UpdateMenubar()
{
	root->UpdateMenubar();
}

void GUI::SetScreenCenterPosition(Position position)
{
	MapTab* mapTab = GetCurrentMapTab();
	if(mapTab)
		mapTab->SetScreenCenterPosition(position);
}

void GUI::DoCut()
{
	if (!IsSelectionMode())
		return;

	Editor* editor = GetCurrentEditor();
	if (!editor)
		return;

	editor->copybuffer.cut(*editor, GetCurrentFloor());
	RefreshView();
	root->UpdateMenubar();
}

void GUI::DoCopy()
{
	if (!IsSelectionMode())
		return;

	Editor* editor = GetCurrentEditor();
	if (!editor)
		return;

	editor->copybuffer.copy(*editor, GetCurrentFloor());
	RefreshView();
	root->UpdateMenubar();
}

void GUI::DoPaste()
{
	MapTab* mapTab = GetCurrentMapTab();
	if(mapTab)
		copybuffer.paste(*mapTab->GetEditor(), mapTab->GetCanvas()->GetCursorPosition());
}

void GUI::PreparePaste()
{
	Editor* editor = GetCurrentEditor();
	if (editor) {
		SetSelectionMode();
		editor->selection.start();
		editor->selection.clear();
		editor->selection.finish();
		StartPasting();
		RefreshView();
	}
}

void GUI::StartPasting()
{
	if(GetCurrentEditor()) {
		pasting = true;
		secondary_map = &copybuffer.getBufferMap();
	}
}

void GUI::EndPasting()
{
	if(pasting) {
		pasting = false;
		secondary_map = nullptr;
	}
}

bool GUI::CanUndo()
{
	Editor* editor = GetCurrentEditor();
	return (editor && editor->actionQueue->canUndo());
}

bool GUI::CanRedo()
{
	Editor* editor = GetCurrentEditor();
	return (editor && editor->actionQueue->canRedo());
}

bool GUI::DoUndo()
{
	Editor* editor = GetCurrentEditor();
	if(editor && editor->actionQueue->canUndo()) {
		editor->actionQueue->undo();
		if(editor->selection.size() > 0)
			SetSelectionMode();
		SetStatusText("Undo action");
		UpdateMinimap();
		root->UpdateMenubar();
		root->Refresh();
		return true;
	}
	return false;
}

bool GUI::DoRedo()
{
	Editor* editor = GetCurrentEditor();
	if(editor && editor->actionQueue->canRedo()) {
		editor->actionQueue->redo();
		if(editor->selection.size() > 0)
			SetSelectionMode();
		SetStatusText("Redo action");
		UpdateMinimap();
		root->UpdateMenubar();
		root->Refresh();
		return true;
	}
	return false;
}

int GUI::GetCurrentFloor()
{
	MapTab* tab = GetCurrentMapTab();
	ASSERT(tab);
	return tab->GetCanvas()->GetFloor();
}

void GUI::ChangeFloor(int new_floor)
{
	MapTab* tab = GetCurrentMapTab();
	if(tab) {
		int old_floor = GetCurrentFloor();
		if(new_floor < 0 || new_floor > MAP_MAX_LAYER)
			return;

		if(old_floor != new_floor)
			tab->GetCanvas()->ChangeFloor(new_floor);
	}
}

void GUI::SetStatusText(wxString text)
{
	g_gui.root->SetStatusText(text, 0);
}

void GUI::SetTitle(wxString title)
{
	if(g_gui.root == nullptr)
		return;

#ifdef NIGHTLY_BUILD
#  ifdef SVN_BUILD
#     define TITLE_APPEND (wxString(" (Nightly Build #") << i2ws(SVN_BUILD) << ")")
#  else
#     define TITLE_APPEND (wxString(" (Nightly Build)"))
#  endif
#else
#  ifdef SVN_BUILD
#     define TITLE_APPEND (wxString(" (Build #") << i2ws(SVN_BUILD) << ")")
#  else
#     define TITLE_APPEND (wxString(""))
#  endif
#endif
#ifdef __EXPERIMENTAL__
	if(title != "") {
		g_gui.root->SetTitle(title << " - Remere's Map Editor BETA" << TITLE_APPEND);
	} else {
		g_gui.root->SetTitle(wxString("Remere's Map Editor BETA") << TITLE_APPEND);
	}
#else
	if(title != "") {
		g_gui.root->SetTitle(title << " - Remere's Map Editor" << TITLE_APPEND);
	} else {
		g_gui.root->SetTitle(wxString("Remere's Map Editor") << TITLE_APPEND);
	}
#endif
}

void GUI::UpdateTitle()
{
	if(tabbook->GetTabCount() > 0) {
		SetTitle(tabbook->GetCurrentTab()->GetTitle());
		for(int idx = 0; idx < tabbook->GetTabCount(); ++idx) {
			if(tabbook->GetTab(idx))
				tabbook->SetTabLabel(idx, tabbook->GetTab(idx)->GetTitle());
		}
	} else {
		SetTitle("");
	}
}

void GUI::UpdateMenus()
{
	wxCommandEvent evt(EVT_UPDATE_MENUS);
	g_gui.root->AddPendingEvent(evt);
}

void GUI::SwitchMode()
{
	if(mode == DRAWING_MODE) {
		SetSelectionMode();
	} else {
		SetDrawingMode();
	}
}

void GUI::SetSelectionMode()
{
	if(mode == SELECTION_MODE)
		return;

	if(current_brush && current_brush->isDoodad()) {
		secondary_map = nullptr;
	}

	tabbook->OnSwitchEditorMode(SELECTION_MODE);
	mode = SELECTION_MODE;
}

void GUI::SetDrawingMode()
{
	if(mode == DRAWING_MODE)
		return;

	std::set<MapTab*> al;
	for(int idx = 0; idx < tabbook->GetTabCount(); ++idx) {
		EditorTab* editorTab = tabbook->GetTab(idx);
		if(MapTab* mapTab = dynamic_cast<MapTab*>(editorTab)) {
			if(al.find(mapTab) != al.end())
				continue;

			Editor* editor = mapTab->GetEditor();
			editor->selection.start();
			editor->selection.clear();
			editor->selection.finish();
			al.insert(mapTab);
		}
	}

	if(current_brush && current_brush->isDoodad()) {
		secondary_map = doodad_buffer_map;
	} else {
		secondary_map = nullptr;
	}

	tabbook->OnSwitchEditorMode(DRAWING_MODE);
	mode = DRAWING_MODE;
}

void GUI::SetBrushSizeInternal(int nz)
{
	if(nz != brush_size && current_brush && current_brush->isDoodad() && !current_brush->oneSizeFitsAll()) {
		brush_size = nz;
		FillDoodadPreviewBuffer();
		secondary_map = doodad_buffer_map;
	} else {
		brush_size = nz;
	}
}

void GUI::SetBrushSize(int nz)
{
	SetBrushSizeInternal(nz);

	for(PaletteList::iterator piter = palettes.begin(); piter != palettes.end(); ++piter) {
		(*piter)->OnUpdateBrushSize(brush_shape, brush_size);
	}
}

void GUI::SetBrushVariation(int nz)
{
	if(nz != brush_variation && current_brush && current_brush->isDoodad()) {
		// Monkey!
		brush_variation = nz;
		FillDoodadPreviewBuffer();
		secondary_map = doodad_buffer_map;
	}
}

void GUI::SetBrushShape(BrushShape bs)
{
	if(bs != brush_shape && current_brush && current_brush->isDoodad() && !current_brush->oneSizeFitsAll()) {
		// Donkey!
		brush_shape = bs;
		FillDoodadPreviewBuffer();
		secondary_map = doodad_buffer_map;
	}
	brush_shape = bs;

	for(PaletteList::iterator piter = palettes.begin(); piter != palettes.end(); ++piter) {
		(*piter)->OnUpdateBrushSize(brush_shape, brush_size);
	}
}

void GUI::SetBrushThickness(bool on, int x, int y)
{
	use_custom_thickness = on;

	if(x != -1 || y != -1) {
		custom_thickness_mod = float(max(x, 1)) / float(max(y, 1));
	}

	if(current_brush && current_brush->isDoodad()) {
		FillDoodadPreviewBuffer();
	}

	RefreshView();
}

void GUI::SetBrushThickness(int low, int ceil)
{
	custom_thickness_mod = float(max(low, 1)) / float(max(ceil, 1));

	if(use_custom_thickness && current_brush && current_brush->isDoodad()) {
		FillDoodadPreviewBuffer();
	}

	RefreshView();
}

void GUI::DecreaseBrushSize(bool wrap)
{
	switch(brush_size) {
		case 0: {
			if(wrap) {
				SetBrushSize(11);
			}
			break;
		}
		case 1: {
			SetBrushSize(0);
			break;
		}
		case 2:
		case 3: {
			SetBrushSize(1);
			break;
		}
		case 4:
		case 5: {
			SetBrushSize(2);
			break;
		}
		case 6:
		case 7: {
			SetBrushSize(4);
			break;
		}
		case 8:
		case 9:
		case 10: {
			SetBrushSize(6);
			break;
		}
		case 11:
		default: {
			SetBrushSize(8);
			break;
		}
	}
}

void GUI::IncreaseBrushSize(bool wrap)
{
	switch(brush_size) {
		case 0: {
			SetBrushSize(1);
			break;
		}
		case 1: {
			SetBrushSize(2);
			break;
		}
		case 2:
		case 3: {
			SetBrushSize(4);
			break;
		}
		case 4:
		case 5: {
			SetBrushSize(6);
			break;
		}
		case 6:
		case 7: {
			SetBrushSize(8);
			break;
		}
		case 8:
		case 9:
		case 10: {
			SetBrushSize(11);
			break;
		}
		case 11:
		default: {
			if(wrap) {
				SetBrushSize(0);
			}
			break;
		}
	}
}

Brush* GUI::GetCurrentBrush() const
{
	return current_brush;
}

BrushShape GUI::GetBrushShape() const
{
	if(current_brush == spawn_brush)
		return BRUSHSHAPE_SQUARE;

	return brush_shape;
}

int GUI::GetBrushSize() const
{
	return brush_size;
}

int GUI::GetBrushVariation() const
{
	return brush_variation;
}

int GUI::GetSpawnTime() const
{
	return creature_spawntime;
}

void GUI::SelectBrush()
{
	if(palettes.empty())
		return;

	SelectBrushInternal(palettes.front()->GetSelectedBrush());

	RefreshView();
}

bool GUI::SelectBrush(const Brush* whatbrush, PaletteType primary)
{
	if(palettes.empty())
		if(!CreatePalette())
			return false;

	if(!palettes.front()->OnSelectBrush(whatbrush, primary))
		return false;

	SelectBrushInternal(const_cast<Brush*>(whatbrush));
	return true;
}

void GUI::SelectBrushInternal(Brush* brush)
{
	// Fear no evil don't you say no evil
	if(current_brush != brush && brush)
		previous_brush = current_brush;

	current_brush = brush;
	if(!current_brush)
		return;

	brush_variation = min(brush_variation, brush->getMaxVariation());
	FillDoodadPreviewBuffer();
	if(brush->isDoodad())
		secondary_map = doodad_buffer_map;

	SetDrawingMode();
	RefreshView();
}

void GUI::SelectPreviousBrush()
{
	if(previous_brush)
		SelectBrush(previous_brush);
}

void GUI::FillDoodadPreviewBuffer()
{
	if(!current_brush || !current_brush->isDoodad())
		return;

	doodad_buffer_map->clear();

	DoodadBrush* brush = current_brush->asDoodad();
	if(brush->isEmpty(GetBrushVariation()))
		return;

	int object_count = 0;
	int area;
	if(GetBrushShape() == BRUSHSHAPE_SQUARE) {
		area = 2*GetBrushSize();
		area = area*area + 1;
	} else {
		if(GetBrushSize() == 1) {
			// There is a huge deviation here with the other formula.
			area = 5;
		} else {
			area = int(0.5 + GetBrushSize() * GetBrushSize() * PI);
		}
	}
	const int object_range = (use_custom_thickness ? int(area*custom_thickness_mod) : brush->getThickness() * area / max(1, brush->getThicknessCeiling()));
	const int final_object_count = max(1, object_range + random(object_range));

	Position center_pos(0x8000, 0x8000, 0x8);

	if(brush_size > 0 && !brush->oneSizeFitsAll()) {
		while(object_count < final_object_count) {
			int retries = 0;
			bool exit = false;

			// Try to place objects 5 times
			while(retries < 5 && !exit) {

				int pos_retries = 0;
				int xpos = 0, ypos = 0;
				bool found_pos = false;
				if(GetBrushShape() == BRUSHSHAPE_CIRCLE) {
					while(pos_retries < 5 && !found_pos) {
						xpos = random(-brush_size, brush_size);
						ypos = random(-brush_size, brush_size);
						float distance = sqrt(float(xpos*xpos) + float(ypos*ypos));
						if(distance < g_gui.GetBrushSize() + 0.005) {
							found_pos = true;
						} else {
							++pos_retries;
						}
					}
				} else {
					found_pos = true;
					xpos = random(-brush_size, brush_size);
					ypos = random(-brush_size, brush_size);
				}

				if(!found_pos) {
					++retries;
					continue;
				}

				// Decide whether the zone should have a composite or several single objects.
				bool fail = false;
				if(random(brush->getTotalChance(GetBrushVariation())) <= brush->getCompositeChance(GetBrushVariation())) {
					// Composite
					const CompositeTileList& composites = brush->getComposite(GetBrushVariation());

					// Figure out if the placement is valid
					for(CompositeTileList::const_iterator composite_iter = composites.begin();
							composite_iter != composites.end();
							++composite_iter)
					{
						Position pos = center_pos + composite_iter->first + Position(xpos, ypos, 0);
						if(Tile* tile = doodad_buffer_map->getTile(pos)) {
							if(tile->size() > 0) {
								fail = true;
								break;
							}
						}
					}
					if(fail) {
						++retries;
						break;
					}

					// Transfer items to the stack
					for(CompositeTileList::const_iterator composite_iter = composites.begin();
							composite_iter != composites.end();
							++composite_iter)
					{
						Position pos = center_pos + composite_iter->first + Position(xpos, ypos, 0);
						const ItemVector& items = composite_iter->second;
						Tile* tile = doodad_buffer_map->getTile(pos);

						if(!tile)
							tile = doodad_buffer_map->allocator(doodad_buffer_map->createTileL(pos));

						for(ItemVector::const_iterator item_iter = items.begin();
								item_iter != items.end();
								++item_iter)
						{
							tile->addItem((*item_iter)->deepCopy());
						}
						doodad_buffer_map->setTile(tile->getPosition(), tile);
					}
					exit = true;
				} else if(brush->hasSingleObjects(GetBrushVariation())) {
					Position pos = center_pos + Position(xpos, ypos, 0);
					Tile* tile = doodad_buffer_map->getTile(pos);
					if(tile) {
						if(tile->size() > 0) {
							fail = true;
							break;
						}
					} else {
						tile = doodad_buffer_map->allocator(doodad_buffer_map->createTileL(pos));
					}
					int variation = GetBrushVariation();
					brush->draw(doodad_buffer_map, tile, &variation);
					//std::cout << "\tpos: " << tile->getPosition() << std::endl;
					doodad_buffer_map->setTile(tile->getPosition(), tile);
					exit = true;
				}
				if(fail) {
					++retries;
					break;
				}
			}
			++object_count;
		}
	} else {
		if(brush->hasCompositeObjects(GetBrushVariation()) &&
				random(brush->getTotalChance(GetBrushVariation())) <= brush->getCompositeChance(GetBrushVariation())) {
			// Composite
			const CompositeTileList& composites = brush->getComposite(GetBrushVariation());

			// All placement is valid...

			// Transfer items to the buffer
			for(CompositeTileList::const_iterator composite_iter = composites.begin();
					composite_iter != composites.end();
					++composite_iter) {
				Position pos = center_pos + composite_iter->first;
				const ItemVector& items = composite_iter->second;
				Tile* tile = doodad_buffer_map->allocator(doodad_buffer_map->createTileL(pos));
				//std::cout << pos << " = " << center_pos << " + " << buffer_tile->getPosition() << std::endl;

				for(ItemVector::const_iterator item_iter = items.begin();
						item_iter != items.end();
						++item_iter)
				{
					tile->addItem((*item_iter)->deepCopy());
				}
				doodad_buffer_map->setTile(tile->getPosition(), tile);
			}
		} else if(brush->hasSingleObjects(GetBrushVariation())) {
			Tile* tile = doodad_buffer_map->allocator(doodad_buffer_map->createTileL(center_pos));
			int variation = GetBrushVariation();
			brush->draw(doodad_buffer_map, tile, &variation);
			doodad_buffer_map->setTile(center_pos, tile);
		}
	}
}

long GUI::PopupDialog(wxWindow* parent, wxString title, wxString text, long style, wxString confisavename, uint32_t configsavevalue)
{
	if(text.empty())
		return wxID_ANY;

	wxMessageDialog dlg(parent, text, title, style);
	return dlg.ShowModal();
}

long GUI::PopupDialog(wxString title, wxString text, long style, wxString configsavename, uint32_t configsavevalue)
{
	return g_gui.PopupDialog(g_gui.root, title, text, style, configsavename, configsavevalue);
}

void GUI::ListDialog(wxWindow* parent, wxString title, const wxArrayString& param_items)
{
	if(param_items.empty())
		return;

	wxArrayString list_items(param_items);

	// Create the window
	wxDialog* dlg = newd wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX);

	wxSizer* sizer = newd wxBoxSizer(wxVERTICAL);
	wxListBox* item_list = newd wxListBox(dlg, wxID_ANY, wxDefaultPosition, wxDefaultSize, 0, nullptr, wxLB_SINGLE);
	item_list->SetMinSize(wxSize(500, 300));

	for(size_t i = 0; i != list_items.GetCount();) {
		wxString str = list_items[i];
		size_t pos = str.find("\n");
		if(pos != wxString::npos) {
			// Split string!
			item_list->Append(str.substr(0, pos));
			list_items[i] = str.substr(pos+1);
			continue;
		}
		item_list->Append(list_items[i]);
		++i;
	}
	sizer->Add(item_list, 1, wxEXPAND);

	wxSizer* stdsizer = newd wxBoxSizer(wxHORIZONTAL);
	stdsizer->Add(newd wxButton(dlg, wxID_OK, "OK"), wxSizerFlags(1).Center());
	sizer->Add(stdsizer, wxSizerFlags(0).Center());

	dlg->SetSizerAndFit(sizer);

	// Show the window
	dlg->ShowModal();
	delete dlg;
}

void GUI::ShowTextBox(wxWindow* parent, wxString title, wxString content)
{
	wxDialog* dlg = newd wxDialog(parent, wxID_ANY, title, wxDefaultPosition, wxDefaultSize, wxRESIZE_BORDER | wxCAPTION | wxCLOSE_BOX);
	wxSizer* topsizer = newd wxBoxSizer(wxVERTICAL);
	wxTextCtrl* text_field = newd wxTextCtrl(dlg, wxID_ANY, content, wxDefaultPosition, wxDefaultSize, wxTE_MULTILINE | wxTE_READONLY);
	text_field->SetMinSize(wxSize(400, 550));
	topsizer->Add(text_field, wxSizerFlags(5).Expand());

	wxSizer* choicesizer = newd wxBoxSizer(wxHORIZONTAL);
	choicesizer->Add(newd wxButton(dlg, wxID_CANCEL, "OK"), wxSizerFlags(1).Center());
	topsizer->Add(choicesizer, wxSizerFlags(0).Center());
	dlg->SetSizerAndFit(topsizer);

	dlg->ShowModal();
}

void GUI::SetHotkey(int index, Hotkey& hotkey)
{
	ASSERT(index >= 0 && index <= 9);
	hotkeys[index] = hotkey;
	SetStatusText("Set hotkey " + i2ws(index) + ".");
}

const Hotkey& GUI::GetHotkey(int index) const
{
	ASSERT(index >= 0 && index <= 9);
	return hotkeys[index];
}

void GUI::SaveHotkeys() const
{
	std::ostringstream os;
	for(int i = 0; i < 10; ++i) {
		os << hotkeys[i] << '\n';
	}
	g_settings.setString(Config::NUMERICAL_HOTKEYS, os.str());
}

void GUI::LoadHotkeys()
{
	std::istringstream is;
	is.str(g_settings.getString(Config::NUMERICAL_HOTKEYS));

	std::string line;
	int index = 0;
	while(getline(is, line)) {
		std::istringstream line_is;
		line_is.str(line);
		line_is >> hotkeys[index];

		++index;
	}
}

Hotkey::Hotkey() :
	type(NONE)
{
	////
}

Hotkey::Hotkey(Position _pos) : type(POSITION), pos(_pos)
{
	////
}

Hotkey::Hotkey(Brush* brush) : type(BRUSH), brushname(brush->getName())
{
	////
}

Hotkey::Hotkey(std::string _name) : type(BRUSH), brushname(_name)
{
	////
}

Hotkey::~Hotkey()
{
	////
}

std::ostream& operator<<(std::ostream& os, const Hotkey& hotkey)
{
	switch(hotkey.type) {
		case Hotkey::POSITION: {
			os << "pos:{" << hotkey.pos << "}";
		} break;
		case Hotkey::BRUSH: {
			if(hotkey.brushname.find('{') != std::string::npos ||
					hotkey.brushname.find('}') != std::string::npos) {
				break;
			}
			os << "brush:{" << hotkey.brushname << "}";
		} break;
		default: {
			os << "none:{}";
		} break;
	}
	return os;
}

std::istream& operator>>(std::istream& is, Hotkey& hotkey)
{
	std::string type;
	getline(is, type, ':');
	if(type == "none") {
		is.ignore(2); // ignore "{}"
	} else if(type == "pos") {
		is.ignore(1); // ignore "{"
		Position pos;
		is >> pos;
		hotkey = Hotkey(pos);
		is.ignore(1); // ignore "}"
	} else if(type == "brush") {
		is.ignore(1); // ignore "{"
		std::string brushname;
		getline(is, brushname, '}');
		hotkey = Hotkey(brushname);
	} else {
		// Do nothing...
	}

	return is;
}

void SetWindowToolTip(wxWindow* a, const wxString& tip)
{
	a->SetToolTip(tip);
}

void SetWindowToolTip(wxWindow* a, wxWindow* b, const wxString& tip)
{
	a->SetToolTip(tip);
	b->SetToolTip(tip);
}
