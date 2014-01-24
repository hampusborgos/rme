//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include <wx/display.h>

#include "gui.h"

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

const wxEventType EVT_UPDATE_MENUS = wxNewEventType();


// Global GUI instance
GUI gui;

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
	delete gui.aui_manager;
	delete OGLContext;
}

wxGLContext* GUI::GetGLContext(wxGLCanvas* win)
{
	if(OGLContext == nullptr)
    {
#ifdef __WXOSX__
        /*
        wxGLContext(AGLPixelFormat fmt, wxGLCanvas *win,
                    const wxPalette& WXUNUSED(palette),
                    const wxGLContext *other
                    );
        */
        wxPalette unused;
        OGLContext = new wxGLContext((AGLPixelFormat)AGL_RGBA, (wxGLCanvas*)nullptr, unused, nullptr);
#else
		OGLContext = newd wxGLContext(win);
#endif
    }
    
	return OGLContext;
}

wxString GUI::GetDataDirectory()
{
	std::string cfg_str = settings.getString(Config::DATA_DIRECTORY);
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

	exec_directory.AppendDir(wxT("data"));
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
		wxLogError(wxT("Could not fetch executable directory."));
	}
	return exec_directory.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
}

wxString GUI::GetLocalDataDirectory()
{
	if(settings.getInteger(Config::INDIRECTORY_INSTALLATION))
	{
		FileName dir = GetDataDirectory();
		dir.AppendDir(wxT("user"));
		dir.AppendDir(wxT("data"));
		dir.Mkdir(0755, wxPATH_MKDIR_FULL);
		return dir.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);;
	}
	else
	{
		FileName dir = dynamic_cast<wxStandardPaths&>(wxStandardPaths::Get()).GetUserDataDir();
#ifdef __WINDOWS__
		dir.AppendDir(wxT("Remere's Map Editor"));
#else
		dir.AppendDir(wxT(".rme"));
#endif
		dir.AppendDir(wxT("data"));
		dir.Mkdir(0755, wxPATH_MKDIR_FULL);
		return dir.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
	}
}

wxString GUI::GetLocalDirectory()
{
	if(settings.getInteger(Config::INDIRECTORY_INSTALLATION))
	{
		FileName dir = GetDataDirectory();
		dir.AppendDir(wxT("user"));
		dir.Mkdir(0755, wxPATH_MKDIR_FULL);
		return dir.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);;
	}
	else
	{
		FileName dir = dynamic_cast<wxStandardPaths&>(wxStandardPaths::Get()).GetUserDataDir();
#ifdef __WINDOWS__
		dir.AppendDir(wxT("Remere's Map Editor"));
#else
		dir.AppendDir(wxT(".rme"));
#endif
		dir.Mkdir(0755, wxPATH_MKDIR_FULL);
		return dir.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
	}
}

wxString GUI::GetExtensionsDirectory()
{
	std::string cfg_str = settings.getString(Config::EXTENSIONS_DIRECTORY);
	if(cfg_str.size())
	{
		FileName dir;
		dir.Assign(wxstr(cfg_str));
		wxString path;
		if(dir.DirExists())
		{
			path = dir.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
			return path;
		}
	}

	// Silently reset directory
	FileName local_directory = GetLocalDirectory();
	local_directory.AppendDir(wxT("extensions"));
	local_directory.Mkdir(0755, wxPATH_MKDIR_FULL);
	return local_directory.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR);
}

void GUI::discoverDataDirectory(const wxString& existentFile)
{
	wxString possiblePaths[] = {
		GetExecDirectory(),
		wxGetCwd() + "/",

		// these are used usually when running from build directories
		GetExecDirectory() + "/../",
		wxGetCwd() + "/../",
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
		wxLogError(wxString() + wxT("Could not find data directory.\n"));
}

bool GUI::LoadVersion(ClientVersionID ver, wxString& error, wxArrayString& warnings, bool force)
{
	if(ClientVersion::get(ver) == nullptr)
	{
		error = wxT("Unsupported client version! (8)");
		return false;
	}
	
	if(ver != loaded_version || force)
	{
		if(getLoadedVersion() != nullptr)
			// There is another version loaded right now, save window layout
			gui.SavePerspective();

		// Disable all rendering so the data is not accessed while reloading
		UnnamedRenderingLock();
		DestroyPalettes();
		DestroyMinimap();
		
		// Destroy the previous version
		UnloadVersion();

		loaded_version = ver;
		if(getLoadedVersion()->hasValidPaths() == false)
		{
			if(getLoadedVersion()->loadValidPaths() == false)
			{
				error = wxT("Couldn't load relevant data files");
				loaded_version = CLIENT_VERSION_NONE;
				return false;
			}
		}
		bool ret = LoadDataFiles(error, warnings);

		if(ret)
			gui.LoadPerspective();
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

ClientVersionID GUI::GetCurrentVersionID() const {
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
		error = wxT("Couldn't establish working directory...");
		return false;
	}

	gui.gfx.client_version = getLoadedVersion();
	
	gui.CreateLoadBar(wxT("Loading data files"));
	gui.SetLoadDone(0, wxT("Loading Tibia.dat ..."));
	FileName dat_path = wxString(client_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + wxT("Tibia.dat"));
	
	if(!gui.gfx.loadSpriteMetadata(dat_path, error, warnings))
	{
		error = wxT("Couldn't load tibia.dat: ") + error;
		gui.DestroyLoadBar();
		UnloadVersion();
		return false;
	}
	
	FileName spr_path = wxString(client_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + wxT("Tibia.spr"));

	gui.SetLoadDone(10, wxT("Loading Tibia.spr ..."));
	if(!gui.gfx.loadSpriteData(spr_path.GetFullPath(), error, warnings))
	{
		error = wxT("Couldn't load tibia.spr: ") + error;
		gui.DestroyLoadBar();
		UnloadVersion();
		return false;
	}

	gui.SetLoadDone(20, wxT("Loading items.otb ..."));
	if(!item_db.loadFromOtb(wxString(data_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + wxT("items.otb")), error, warnings))
	{
		error = wxT("Couldn't load items.otb: ") + error;
		gui.DestroyLoadBar();
		UnloadVersion();
		return false;
	}

	gui.SetLoadDone(30, wxT("Loading items.xml ..."));
	if(!item_db.loadFromGameXml(wxString(data_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + wxT("items.xml")), error, warnings))
	{
		warnings.push_back(wxT("Couldn't load items.xml: ") + error);
	}

	gui.SetLoadDone(45, wxT("Loading creatures.xml ..."));
	if(!creature_db.loadFromXML(wxString(data_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + wxT("creatures.xml")), true, error, warnings))
	{
		warnings.push_back(wxT("Couldn't load creatures.xml: ") + error);
	}

	gui.SetLoadDone(45, wxT("Loading user creatures.xml ..."));
	{
		FileName cdb = getLoadedVersion()->getLocalDataPath();
		cdb.SetFullName(wxT("creatures.xml"));
		wxString nerr;
		wxArrayString nwarn;
		creature_db.loadFromXML(cdb, false, nerr, nwarn);
	}

	gui.SetLoadDone(50, wxT("Loading materials.xml ..."));
	if(!materials.loadMaterials(wxString(data_path.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + wxT("materials.xml")), error, warnings))
	{
		warnings.push_back(wxT("Couldn't load materials.xml: ") + error);
	}

	gui.SetLoadDone(70, wxT("Loading extensions..."));
	if(!materials.loadExtensions(extension_path, error, warnings))
	{
		//warnings.push_back(wxT("Couldn't load extensions: ") + error);
	}

	gui.SetLoadDone(70, wxT("Finishing..."));
	brushes.init();
	materials.createOtherTileset();

	gui.DestroyLoadBar();
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

	if(loaded_version != CLIENT_VERSION_NONE)
	{
		//gui.UnloadVersion();
		materials.clear();
		brushes.clear();
		item_db.clear();
		gfx.clear();

		FileName cdb = getLoadedVersion()->getLocalDataPath();
		cdb.SetFullName(wxT("creatures.xml"));
		creature_db.saveToXML(cdb);
		creature_db.clear();

		loaded_version = CLIENT_VERSION_NONE;
	}
}

void GUI::SaveCurrentMap(FileName filename, bool showdialog)
{
	Editor* edit = GetCurrentEditor();
	if(edit)
		edit->saveMap(filename, showdialog);

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

void GUI::FitViewToMap()
{
	for(int index = 0; index < tabbook->GetTabCount(); ++index)
	{
		if(MapTab* tab = dynamic_cast<MapTab*>(tabbook->GetTab(index)))
		{
			tab->GetView()->FitToMap();
		}
	}
}

void GUI::FitViewToMap(MapTab* mt)
{
	for(int index = 0; index < tabbook->GetTabCount(); ++index)
	{
		if(MapTab* tab = dynamic_cast<MapTab*>(tabbook->GetTab(index)))
		{
			if(tab->HasSameReference(mt)) {
				tab->GetView()->FitToMap();
			}
		}
	}
}

bool GUI::NewMap()
{
	Editor* ed;
	try
	{
		ed = newd Editor(copybuffer);
	}
	catch(std::runtime_error& e)
	{
		PopupDialog(root, wxT("Error!"), wxString(e.what(), wxConvUTF8), wxOK);
		return false;
	}

	MapTab* edit = newd MapTab(tabbook, ed);
	edit->OnSwitchEditorMode(mode);

	SetStatusText(wxT("Created new map"));
	UpdateTitle();
	RefreshPalettes();
	root->UpdateMenubar();
	root->Refresh();

	return true;
}

bool GUI::LoadMap(FileName name)
{
	if(GetCurrentEditor() && GetCurrentMap().hasChanged() == false && GetCurrentMap().hasFile() == false)
		gui.CloseCurrentEditor();

	Editor* ed;
	try
	{
		ed = newd Editor(copybuffer, name);
	}
	catch(std::runtime_error& e)
	{
		PopupDialog(root, wxT("Error!"), wxString(e.what(), wxConvUTF8), wxOK);
		return false;
	}

	MapTab* edit = newd MapTab(tabbook, ed);
	edit->OnSwitchEditorMode(mode);

	root->AddRecentFile(name);
		
	edit->GetView()->FitToMap();
	UpdateTitle();
	ListDialog(wxT("Map loader errors"), edit->GetMap()->getWarnings());
	root->DoQueryImportCreatures();

	FitViewToMap(edit);
	root->UpdateMenubar();
	return true;
}

Editor* GUI::GetCurrentEditor()
{
	MapTab* etab = GetCurrentMapTab();
	if(etab)
		return etab->GetEditor();
	return nullptr;
}

EditorTab* GUI::GetTab(int idx) {
	return tabbook->GetTab(idx);
}

int GUI::GetTabCount() const {
	return tabbook->GetTabCount();
}


EditorTab* GUI::GetCurrentTab()
{
	return tabbook->GetCurrentTab();
}

MapTab* GUI::GetCurrentMapTab() const
{
	if(tabbook && tabbook->GetTabCount() > 0)
	{
		EditorTab* tab = tabbook->GetCurrentTab();
		MapTab* etab = dynamic_cast<MapTab*>(tab);
		return etab;
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

	for(int i = 0; i < tabbook->GetTabCount(); ++i)
	{
		MapTab* tab = dynamic_cast<MapTab*>(tabbook->GetTab(i));
		if(tab)
			open_maps.insert(open_maps.begin(), tab->GetMap());

	}

	return open_maps.size();
}

bool GUI::ShouldSave()
{
	const Map& map = GetCurrentMap();
	if(map.hasChanged())
	{
		if(map.getTileCount() == 0)
		{
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
	MapTab* etab = GetCurrentMapTab();
	if(etab)
		etab->GetCanvas()->GetEventHandler()->AddPendingEvent(event);
}

void GUI::CloseCurrentEditor()
{
	RefreshPalettes();
	tabbook->DeleteTab(tabbook->GetSelection());
	root->UpdateMenubar();
}

bool GUI::CloseLiveEditors(LiveSocket* sock)
{
	for(int i = 0; i < tabbook->GetTabCount(); ++i)
	{
		MapTab* mt = dynamic_cast<MapTab*>(tabbook->GetTab(i));
		if(mt)
		{
			Editor* editor = mt->GetEditor();
			if(editor->GetLiveClient() == sock)
				tabbook->DeleteTab(i--);
		}
		LiveLogTab* lt = dynamic_cast<LiveLogTab*>(tabbook->GetTab(i));
		if (lt)
		{
			if (lt->GetSocket() == sock)
			{
				lt->Disconnect();
				tabbook->DeleteTab(i--);
			}
		}
	}
	root->UpdateMenubar();
	return true;
}


bool GUI::CloseAllEditors()
{
	for(int i = 0; i < tabbook->GetTabCount(); ++i)
	{
		MapTab* mt = dynamic_cast<MapTab*>(tabbook->GetTab(i));
		if(mt)
		{
			if(mt->IsUniqueReference() && mt->GetMap() && mt->GetMap()->hasChanged())
			{
				tabbook->SetFocusedTab(i);
				if(!root->DoQuerySave(false))
				{
					return false;
				}
				else
				{
					RefreshPalettes();
					tabbook->DeleteTab(i--);
				}
			}
			else
			{
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
	MapTab* mt = GetCurrentMapTab();
	if(mt)
	{
		MapTab* edit = newd MapTab(mt);
		edit->OnSwitchEditorMode(mode);

		SetStatusText(wxT("Created newd view"));
		UpdateTitle();
		RefreshPalettes();
		root->UpdateMenubar();
		root->Refresh();
	}
}

void GUI::LoadPerspective()
{

	if(IsVersionLoaded() == false)
	{
		if(settings.getInteger(Config::WINDOW_MAXIMIZED))
		{
			root->Maximize();
		}
		else
		{
			root->SetSize(wxSize(settings.getInteger(Config::WINDOW_WIDTH), settings.getInteger(Config::WINDOW_HEIGHT)));
		}
	}
	else
	{
		std::string layout = settings.getString(Config::PALETTE_LAYOUT);
		std::vector<std::string> palette_list;

		std::string::const_iterator last_iterator = layout.begin();
		std::string tmp;
		size_t i = 0;
		while(i  < layout.size())
		{
			if(layout[i] == '|')
			{
				palette_list.push_back(tmp);
				tmp.clear();
			}
			else
				tmp.append(1, layout[i]);
			++i;
		}
		if(tmp.size())
			palette_list.push_back(tmp);

		for(std::vector<std::string>::const_iterator iter = palette_list.begin();
			iter != palette_list.end();
			++iter)
		{
			PaletteWindow* palette = CreatePalette();
			wxAuiPaneInfo& info = aui_manager->GetPane(palette);
			wxString data = wxstr(*iter);
			aui_manager->LoadPaneInfo(data, info);

			if(info.IsFloatable())
			{
				bool offscreen = true;
				for(uint index = 0; index < wxDisplay::GetCount(); ++index)
				{
					wxDisplay display(index);
					wxRect rect = display.GetClientArea();
					if(rect.Contains(info.floating_pos))
					{
						offscreen = false;
						break;
					}
				}
				if(offscreen)
				{
					info.Dock();
				}
			}
		}

		if(settings.getInteger(Config::MINIMAP_VISIBLE))
		{
			if(!minimap)
			{
				wxAuiPaneInfo info;
				wxString data = wxstr(settings.getString(Config::MINIMAP_LAYOUT));
				aui_manager->LoadPaneInfo(data, info);
				minimap = newd MinimapWindow(root);
				aui_manager->AddPane(minimap, info);
			}
			else
			{
				wxAuiPaneInfo& info = aui_manager->GetPane(minimap);
				wxString data = wxstr(settings.getString(Config::MINIMAP_LAYOUT));
				aui_manager->LoadPaneInfo(data, info);
			}
			wxAuiPaneInfo& info = aui_manager->GetPane(minimap);

			if(info.IsFloatable())
			{
				bool offscreen = true;
				for(uint index = 0; index < wxDisplay::GetCount(); ++index)
				{
					wxDisplay display(index);
					wxRect rect = display.GetClientArea();
					if(rect.Contains(info.floating_pos))
					{
						offscreen = false;
						break;
					}
				}
				if(offscreen)
				{
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
	settings.setInteger(Config::WINDOW_MAXIMIZED, root->IsMaximized());
	settings.setInteger(Config::WINDOW_WIDTH, root->GetSize().GetWidth());
	settings.setInteger(Config::WINDOW_HEIGHT, root->GetSize().GetHeight());
	
	settings.setInteger(Config::MINIMAP_VISIBLE, minimap? 1: 0);
	
	wxString pinfo;
	for(PaletteList::iterator piter = palettes.begin();
		piter != palettes.end();
		++piter)
	{
		if(aui_manager->GetPane(*piter).IsShown())
			pinfo << aui_manager->SavePaneInfo(aui_manager->GetPane(*piter)) << wxT("|");
	}
	settings.setString(Config::PALETTE_LAYOUT, nstr(pinfo));

	if(minimap)
	{
		wxString s = aui_manager->SavePaneInfo(aui_manager->GetPane(minimap));
		settings.setString(Config::MINIMAP_LAYOUT, nstr(s));
	}
}

void GUI::HideSearchWindow()
{
	if(search_result_window)
	{
		aui_manager->GetPane(search_result_window).Show(false);
		aui_manager->Update();
	}
}

SearchResultWindow* GUI::ShowSearchWindow()
{
	if(search_result_window == nullptr)
	{
		search_result_window = newd SearchResultWindow(root);
		aui_manager->AddPane(search_result_window, wxAuiPaneInfo().Caption(wxT("Search Results")));
	}
	else
	{
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
	for(PaletteList::iterator piter = palettes.begin();
		piter != palettes.end();
		++piter)
	{
		(*piter)->OnUpdate(m? m : (usedefault? (IsEditorOpen()? &GetCurrentMap() : nullptr): nullptr));
	}
	SelectBrush();
}

void GUI::RefreshOtherPalettes(PaletteWindow* p)
{
	for(PaletteList::iterator piter = palettes.begin();
		piter != palettes.end();
		++piter)
	{
		if(*piter != p)
			(*piter)->OnUpdate(IsEditorOpen()? &GetCurrentMap() : nullptr);
	}
	SelectBrush();
}

PaletteWindow* GUI::CreatePalette()
{
	if(IsVersionLoaded() == false)
		return nullptr;
	
	PaletteWindow* palette = newd PaletteWindow(root, materials.tilesets);
	aui_manager->AddPane(palette, wxAuiPaneInfo().Caption(wxT("Palette")).TopDockable(false).BottomDockable(false));
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
	for(PaletteList::iterator piter = palettes.begin();
		piter != palettes.end();
		++piter)
	{
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
	for(PaletteList::iterator piter = tmp.begin();
		piter != tmp.end();
		++piter)
	{
		(*piter)->ReloadSettings(IsEditorOpen()? &GetCurrentMap() : nullptr);
	}
	aui_manager->Update();
}

void GUI::ShowPalette()
{
	if(palettes.empty())
		return;

	for(PaletteList::iterator piter = palettes.begin();
		piter != palettes.end();
		++piter)
	{
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
	if(IsVersionLoaded() == false) 
		return;

	if(minimap)
	{
		aui_manager->GetPane(minimap).Show(true);
	}
	else
	{
		minimap = newd MinimapWindow(root);
		minimap->Show(true);
		aui_manager->AddPane(minimap, wxAuiPaneInfo().Caption(wxT("Minimap")));
	}
	aui_manager->Update();
}

void GUI::HideMinimap()
{
	if(minimap)
	{
		aui_manager->GetPane(minimap).Show(false);
		aui_manager->Update();
	}
}

void GUI::DestroyMinimap()
{
	if(minimap)
	{
		aui_manager->DetachPane(minimap);
		aui_manager->Update();
		minimap->Destroy();
		minimap = nullptr;
	}
}

void GUI::UpdateMinimap(bool immediate)
{
	if(IsMinimapVisible())
	{
		if(immediate)
		{
			minimap->Refresh();
		}
		else
		{
			minimap->DelayedUpdate();
		}
	}
}

bool GUI::IsMinimapVisible() const
{
	if(minimap)
	{
		const wxAuiPaneInfo& pi = aui_manager->GetPane(minimap);
		if(pi.IsShown())
		{
			return true;
		}
	}
	return false;
}

//=============================================================================

void GUI::RefreshView()
{
	std::vector<EditorTab*> ets;
	MapTab* mt = dynamic_cast<MapTab*>(GetCurrentTab());
	if(mt)
	{
		for(int n = 0; n < tabbook->GetTabCount(); ++n)
		{
			MapTab* mtn = dynamic_cast<MapTab*>(tabbook->GetTab(n));
			if(mtn)
				ets.push_back(mtn);
		}
	}
	else if(GetCurrentTab())
	{
		ets.push_back(GetCurrentTab());
	}

	for(std::vector<EditorTab*>::iterator eti = ets.begin(); eti != ets.end(); ++eti)
	{
		(*eti)->GetWindow()->Refresh();
	}
}

void GUI::CreateLoadBar(wxString message, bool canCancel /* = false */ )
{
	progressText = message;
	progressFrom = 0;
	progressTo = 100;
	progressBar = newd wxGenericProgressDialog(wxT("Loading"), progressText + wxT(" (0%)"), 100, root,
		wxPD_APP_MODAL | wxPD_SMOOTH | (canCancel ? wxPD_CAN_ABORT : 0)
	);
	progressBar->SetSize(280, -1);
	progressBar->Show(true);
	
	for(int idx = 0; idx < tabbook->GetTabCount(); ++idx)
	{
		MapTab* mt = dynamic_cast<MapTab*>(tabbook->GetTab(idx));
		if (mt && mt->GetEditor()->IsLiveServer())
			mt->GetEditor()->GetLiveServer()->StartOperation(progressText);
	}
	progressBar->Update(0);
}

void GUI::SetLoadScale(int from, int to)
{
	progressFrom = from;
	progressTo = to;
}

bool GUI::SetLoadDone(int done, wxString newmessage)
{
	if (done == 100) {
		DestroyLoadBar();
		return true;
	}

	if (newmessage != wxT("")) {
		progressText = newmessage;
	}

	int32_t new_done = progressFrom + static_cast<int32_t>((done / 100.f) * (progressTo - progressFrom));
	new_done = std::max<int32_t>(0, std::min<int32_t>(100, new_done));
	
	bool skip = false;
	wxString text = progressText + wxT(" (") + std::to_string(new_done) + wxT("%)");
	if (progressBar) {
		progressBar->Update(new_done, text, &skip);
	}

	for (int idx = 0; idx < tabbook->GetTabCount(); ++idx) {
		MapTab* mt = dynamic_cast<MapTab*>(tabbook->GetTab(idx));
		if (mt && mt->GetEditor()->IsLiveServer())
			mt->GetEditor()->GetLiveServer()->UpdateOperation(new_done);
	}

	return skip;
}

void GUI::DestroyLoadBar()
{
	if (progressBar) {
		progressBar->Show(false);
		progressBar->Destroy();
		progressBar = nullptr;

		if (root->IsActive()) {
			root->Raise();
		} else {
			root->RequestUserAttention();
		}
	}
}

void GUI::CenterOnPosition(Position p)
{
	MapTab* tab = GetCurrentMapTab();
	if(tab)
	{
		tab->CenterOnPosition(p);
	}
}

void GUI::DoPaste()
{
	MapTab* mt = GetCurrentMapTab();
	if(mt)
	{
		copybuffer.paste(*mt->GetEditor(), mt->GetCanvas()->GetCursorPosition());
	}
}

void GUI::StartPasting()
{
	if(GetCurrentEditor())
	{
		pasting = true;
		secondary_map = &copybuffer.getBufferMap();
	}
}

void GUI::EndPasting()
{
	if(pasting)
	{
		pasting = false;
		secondary_map = nullptr;
	}
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
	if(tab)
	{
		int old_floor = GetCurrentFloor();
		if(new_floor < 0) return;
		if(new_floor > 15) return;
		if(old_floor != new_floor)
			tab->GetCanvas()->ChangeFloor(new_floor);

	}
}

void GUI::SetStatusText(wxString text)
{
	gui.root->SetStatusText(text, 0);
}

void GUI::SetTitle(wxString title)
{
	if(gui.root == nullptr)
		return;

#ifdef NIGHTLY_BUILD
#  ifdef SVN_BUILD
#     define TITLE_APPEND (wxString(wxT(" (Nightly Build #")) << i2ws(SVN_BUILD) << wxT(")"))
#  else
#     define TITLE_APPEND (wxString(wxT(" (Nightly Build)")))
#  endif
#else
#  ifdef SVN_BUILD
#     define TITLE_APPEND (wxString(wxT(" (Build #")) << i2ws(SVN_BUILD) << wxT(")"))
#  else
#     define TITLE_APPEND (wxString(wxT("")))
#  endif
#endif
#ifdef __EXPERIMENTAL__
	if(title != wxT(""))
	{
		gui.root->SetTitle(title << wxT(" - Remere's Map Editor BETA") << TITLE_APPEND);
	}
	else
	{
		gui.root->SetTitle(wxString(wxT("Remere's Map Editor BETA")) << TITLE_APPEND);
	}
#else
	if(title != wxT(""))
	{
		gui.root->SetTitle(title << wxT(" - Remere's Map Editor") << TITLE_APPEND);
	}
	else
	{
		gui.root->SetTitle(wxString(wxT("Remere's Map Editor")) << TITLE_APPEND);
	}
#endif
}

void GUI::UpdateTitle()
{
	if(tabbook->GetTabCount() > 0)
	{
		SetTitle(tabbook->GetCurrentTab()->GetTitle());
		for(int idx = 0; idx < tabbook->GetTabCount(); ++idx)
		{
			if(tabbook->GetTab(idx))
				tabbook->SetTabLabel(idx, tabbook->GetTab(idx)->GetTitle());
		}
	}
	else
	{
		SetTitle(wxT(""));
	}
}

void GUI::UpdateMenus()
{
	wxCommandEvent evt(EVT_UPDATE_MENUS);
	gui.root->AddPendingEvent(evt);
}

void GUI::SwitchMode()
{
	if(mode == DRAWING_MODE)
	{
		SetSelectionMode();
	}
	else
	{
		SetDrawingMode();
	}
}

void GUI::SetSelectionMode()
{
	if(mode == SELECTION_MODE)
		return;
	
	DoodadBrush* dbrush = dynamic_cast<DoodadBrush*>(current_brush);
	if(dbrush)
	{
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
	for(int idx = 0; idx < tabbook->GetTabCount(); ++idx)
	{
		EditorTab* et = tabbook->GetTab(idx);
		if(MapTab* mt = dynamic_cast<MapTab*>(et))
		{
			if(al.find(mt) != al.end())
				continue;

			Editor* editor = mt->GetEditor();
			editor->selection.start();
			editor->selection.clear();
			editor->selection.finish();
			al.insert(mt);
		}
	}

	DoodadBrush* dbrush = dynamic_cast<DoodadBrush*>(current_brush);
	if(dbrush)
	{
		secondary_map = doodad_buffer_map;
	}
	else
	{
		secondary_map = nullptr;
	}
	
	tabbook->OnSwitchEditorMode(DRAWING_MODE);
	mode = DRAWING_MODE;
}

void GUI::SetBrushSizeInternal(int nz)
{
	DoodadBrush* dbrush = dynamic_cast<DoodadBrush*>(current_brush);
	if(nz != brush_size && dbrush && dbrush->oneSizeFitsAll() == false)
	{
		brush_size = nz;
		FillDoodadPreviewBuffer();
		secondary_map = doodad_buffer_map;
	}
	else
	{
		brush_size = nz;
	}
}

void GUI::SetBrushSize(int nz)
{
	SetBrushSizeInternal(nz);

	for(PaletteList::iterator piter = palettes.begin();
		piter != palettes.end();
		++piter)
	{
		(*piter)->OnUpdateBrushSize(brush_shape, brush_size);
	}
}

void GUI::SetBrushVariation(int nz)
{
	DoodadBrush* dbrush = dynamic_cast<DoodadBrush*>(current_brush);
	if(nz != brush_variation && dbrush)
	{
		// Monkey!
		brush_variation = nz;
		FillDoodadPreviewBuffer();
		secondary_map = doodad_buffer_map;
	}
}

void GUI::SetBrushShape(BrushShape bs)
{
	DoodadBrush* dbrush = dynamic_cast<DoodadBrush*>(current_brush);
	if(bs != brush_shape && dbrush && dbrush->oneSizeFitsAll() == false)
	{
		// Donkey!
		brush_shape = bs;
		FillDoodadPreviewBuffer();
		secondary_map = doodad_buffer_map;
	}
	brush_shape = bs;
	
	for(PaletteList::iterator piter = palettes.begin();
		piter != palettes.end();
		++piter)
	{
		(*piter)->OnUpdateBrushSize(brush_shape, brush_size);
	}
}

void GUI::SetBrushThickness(bool on, int x, int y)
{
	use_custom_thickness = on;

	if(x != -1 || y != -1)
	{
		custom_thickness_mod = float(max(x, 1)) / float(max(y, 1));
	}

	DoodadBrush* dbrush = dynamic_cast<DoodadBrush*>(current_brush);
	if(dbrush)
	{
		FillDoodadPreviewBuffer();
	}

	RefreshView();
}

void GUI::SetBrushThickness(int low, int ceil)
{
	custom_thickness_mod = float(max(low, 1)) / float(max(ceil, 1));
	
	if(use_custom_thickness)
	{
		DoodadBrush* dbrush = dynamic_cast<DoodadBrush*>(current_brush);
		if(dbrush)
		{
			FillDoodadPreviewBuffer();
		}
	}
	
	RefreshView();
}

void GUI::DecreaseBrushSize(bool wrap)
{
	switch(brush_size)
	{
		case 0: {
			if(wrap) {
				SetBrushSize(11);
			}
		} break;
		case 1: {
			SetBrushSize(0);
		} break;
		case 2: case 3: {
			SetBrushSize(1);
		} break;
		case 4: case 5: {
			SetBrushSize(2);
		} break;
		case 6: case 7: {
			SetBrushSize(4);
		} break;
		case 8: case 9: case 10: {
			SetBrushSize(6);
		} break;
		case 11: default: {
			SetBrushSize(8);
		} break;
	}
}

void GUI::IncreaseBrushSize(bool wrap)
{
	switch(brush_size) {
		case 0: {
			SetBrushSize(1);
		} break;
		case 1: {
			SetBrushSize(2);
		} break;
		case 2: case 3: {
			SetBrushSize(4);
		} break;
		case 4: case 5: {
			SetBrushSize(6);
		} break;
		case 6: case 7: {
			SetBrushSize(8);
		} break;
		case 8: case 9: case 10: {
			SetBrushSize(11);
		} break;
		case 11: default: {
			if(wrap) {
				SetBrushSize(0);
			}
		} break;
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
	if(current_brush != brush && brush != nullptr)
		previous_brush = current_brush;

	current_brush = brush;
	if(current_brush == nullptr)
		return;

	brush_variation = min(brush_variation, (brush? brush->getMaxVariation() : 0));
	FillDoodadPreviewBuffer();
	if(dynamic_cast<DoodadBrush*>(brush))
		secondary_map = doodad_buffer_map;

	SetDrawingMode();
	RefreshView();
}

void GUI::SelectPreviousBrush()
{
	if(previous_brush)
	{
		SelectBrush(previous_brush);
	}
}

void GUI::FillDoodadPreviewBuffer()
{
	DoodadBrush* brush = dynamic_cast<DoodadBrush*>(current_brush);
	if(!brush)
		return;

	doodad_buffer_map->clear();

	if(brush->isEmpty(GetBrushVariation()))
		return;

	int object_count = 0;
	int area;
	if(GetBrushShape() == BRUSHSHAPE_SQUARE)
	{
		area = 2*GetBrushSize();
		area = area*area + 1;
	}
	else
	{
		if(GetBrushSize() == 1)
		{
			// There is a huge deviation here with the other formula.
			area = 5;
		}
		else
		{
			area = int(0.5 + GetBrushSize() * GetBrushSize() * PI);
		}
	}
	const int object_range = (use_custom_thickness? int(area*custom_thickness_mod) : brush->getThickness() * area / max(1, brush->getThicknessCeiling()));
	const int final_object_count = max(1, object_range + random(object_range));

	Position center_pos(0x8000, 0x8000, 0x8);

	if(brush_size > 0 && brush->oneSizeFitsAll() == false)
	{
		while(object_count < final_object_count)
		{
			int retries = 0;
			bool exit = false;

			// Try to place objects 5 times
			while(retries < 5 && exit == false) 
			{
				
				int pos_retries = 0;
				int xpos = 0, ypos = 0;
				bool found_pos = false;
				if(GetBrushShape() == BRUSHSHAPE_CIRCLE)
				{
					while(pos_retries < 5 && found_pos == false)
					{
						xpos = random(-brush_size, brush_size);
						ypos = random(-brush_size, brush_size);
						float distance = sqrt(float(xpos*xpos) + float(ypos*ypos));
						if (distance < gui.GetBrushSize() + 0.005) {
							found_pos = true;
						} else {
							++pos_retries;
						}
					}
				}
				else
				{
					found_pos = true;
					xpos = random(-brush_size, brush_size);
					ypos = random(-brush_size, brush_size);
				}

				if(found_pos == false)
				{
					++retries;
					continue;
				}

				// Decide whether the zone should have a composite or several single objects.
				bool fail = false;
				if(random(brush->getTotalChance(GetBrushVariation())) <= brush->getCompositeChance(GetBrushVariation()))
				{
					// Composite
					const CompositeTileList& composites = brush->getComposite(GetBrushVariation());

					// Figure out if the placement is valid
					for(CompositeTileList::const_iterator composite_iter = composites.begin();
							composite_iter != composites.end();
							++composite_iter)
					{
						Position pos = center_pos + composite_iter->first + Position(xpos, ypos, 0);
						if(Tile* tile = doodad_buffer_map->getTile(pos))
						{
							if(tile->size() > 0)
							{
								fail = true;
								break;
							}
						}
					}
					if(fail)
					{
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
				}
				else if(brush->hasSingleObjects(GetBrushVariation()))
				{
					Position pos = center_pos + Position(xpos, ypos, 0);
					Tile* tile = doodad_buffer_map->getTile(pos);
					if(tile)
					{
						if(tile->size() > 0)
						{
							fail = true;
							break;
						}
					}
					else
					{
						tile = doodad_buffer_map->allocator(doodad_buffer_map->createTileL(pos));
					}
					int variation = GetBrushVariation();
					brush->draw(doodad_buffer_map, tile, &variation);
					//std::cout << "\tpos: " << tile->getPosition() << std::endl;
					doodad_buffer_map->setTile(tile->getPosition(), tile);
					exit = true;
				}
				if(fail)
				{
					++retries;
					break;
				}
			}
			++object_count;
		}
	}
	else
	{
		if(brush->hasCompositeObjects(GetBrushVariation()) &&
				random(brush->getTotalChance(GetBrushVariation())) <= brush->getCompositeChance(GetBrushVariation()))
		{
			// Composite
			const CompositeTileList& composites = brush->getComposite(GetBrushVariation());

			// All placement is valid...

			// Transfer items to the buffer
			for(CompositeTileList::const_iterator composite_iter = composites.begin();
					composite_iter != composites.end();
					++composite_iter)
			{
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
		}
		else if(brush->hasSingleObjects(GetBrushVariation()))
		{
			Tile* tile = doodad_buffer_map->allocator(doodad_buffer_map->createTileL(center_pos));
			int variation = GetBrushVariation();
			brush->draw(doodad_buffer_map, tile, &variation);
			doodad_buffer_map->setTile(center_pos, tile);
		}
	}
}

long GUI::PopupDialog(wxWindow* parent, wxString title, wxString text, long style, wxString confisavename, uint configsavevalue)
{
	if(text.empty())
		return wxID_ANY;

	wxMessageDialog dlg(parent, text, title, style);
	return dlg.ShowModal();
}

long GUI::PopupDialog(wxString title, wxString text, long style, wxString configsavename, uint configsavevalue)
{
	return gui.PopupDialog(gui.root, title, text, style, configsavename, configsavevalue);
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

	for(size_t i = 0; i != list_items.GetCount();)
	{
		wxString str = list_items[i];
		size_t pos = str.find(wxT("\n"));
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
	stdsizer->Add(newd wxButton(dlg, wxID_OK, wxT("OK")), wxSizerFlags(1).Center());
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
	choicesizer->Add(newd wxButton(dlg, wxID_CANCEL, wxT("OK")), wxSizerFlags(1).Center());
	topsizer->Add(choicesizer, wxSizerFlags(0).Center());
	dlg->SetSizerAndFit(topsizer);

	dlg->ShowModal();
}

void GUI::SetHotkey(int index, Hotkey& hotkey)
{
	ASSERT(index >= 0 && index <= 9);
	hotkeys[index] = hotkey;
	SetStatusText(wxT("Set hotkey ") + i2ws(index) + wxT("."));
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
	settings.setString(Config::NUMERICAL_HOTKEYS, os.str());
}

void GUI::LoadHotkeys()
{
	std::istringstream is;
	is.str(settings.getString(Config::NUMERICAL_HOTKEYS));
	
	std::string line;
	int index = 0;
	while(getline(is, line))
	{
		std::istringstream line_is;
		line_is.str(line);
		line_is >> hotkeys[index];
		
		++index;
	}
}

Hotkey::Hotkey() :
	type(NONE)
{
	//..
}

Hotkey::Hotkey(Position _pos) : type(POSITION), pos(_pos)
{
	//...
}

Hotkey::Hotkey(Brush* brush) : type(BRUSH), brushname(brush->getName())
{
	//...
}

Hotkey::Hotkey(std::string _name) : type(BRUSH), brushname(_name)
{
	//...
}

Hotkey::~Hotkey()
{
	//...
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

