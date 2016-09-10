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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/gui.h $
// $Id: gui.h 320 2010-03-08 16:38:07Z admin $

#ifndef RME_GUI_H_
#define RME_GUI_H_

#include "graphics.h"
#include "position.h"

#include "copybuffer.h"
#include "dcbutton.h"
#include "brush_enums.h"
#include "gui_ids.h"
#include "editor_tabs.h"
#include "map_tab.h"
#include "palette_window.h"
#include "client_version.h"
#include "scripting_window.h"

class BaseMap;
class Map;

class Editor;
class Brush;
class HouseBrush;
class HouseExitBrush;
class WaypointBrush;
class OptionalBorderBrush;
class EraserBrush;
class SpawnBrush;
class DoorBrush;
class FlagBrush;

class MainFrame;
class MapWindow;
class MapCanvas;

class SearchResultWindow;
class MinimapWindow;
class PaletteWindow;
class OldPropertiesWindow;
class EditTownsDialog;
class ItemButton;

class LiveSocket;
class LuaInterface;

extern const wxEventType EVT_UPDATE_MENUS;

#define EVT_ON_UPDATE_MENUS(id, fn) \
    DECLARE_EVENT_TABLE_ENTRY( \
        EVT_UPDATE_MENUS, id, wxID_ANY, \
        (wxObjectEventFunction)(wxEventFunction) wxStaticCastEvent( wxCommandEventFunction, &fn ), \
        (wxObject *) nullptr \
    ),

class Hotkey
{
public:
	Hotkey();
	Hotkey(Position pos);
	Hotkey(Brush* brush);
	Hotkey(std::string _brushname);
	~Hotkey();

	bool IsPosition() const {return type == POSITION;}
	bool IsBrush() const {return type == BRUSH;}
	Position GetPosition() const {ASSERT(IsPosition()); return pos;}
	std::string GetBrushname() const {ASSERT(IsBrush()); return brushname;}

private:
	enum
	{
		NONE,
		POSITION,
		BRUSH,
	} type;

	Position pos;
	std::string brushname;

	friend std::ostream& operator<<(std::ostream& os, const Hotkey& hotkey);
	friend std::istream& operator>>(std::istream& os, Hotkey& hotkey);
};

std::ostream& operator<<(std::ostream& os, const Hotkey& hotkey);
std::istream& operator>>(std::istream& os, Hotkey& hotkey);



class GUI
{
public: // dtor and ctor
	GUI();
	~GUI();

private:
	GUI(const GUI& g_gui); // Don't copy me
	GUI& operator=(const GUI& g_gui); // Don't assign me
	bool operator==(const GUI& g_gui); // Don't compare me

public:
	/**
	 * Saves the perspective to the configuration file
	 * This is the position of all windows etc. in the editor
	 */
	void SavePerspective();

	/**
	 * Loads the stored perspective from the configuration file
	 */
	void LoadPerspective();

	/**
	 * Creates a loading bar with the specified message, title is always "Loading"
	 * The default scale is 0 - 100
	 */
	void CreateLoadBar(wxString message, bool canCancel = false);

	/**
	 * Sets how much of the load has completed, the scale can be set with
	 * SetLoadScale.
	 * If this returns false, the user has hit the quit button and you should
	 * abort the loading.
	 */
	bool SetLoadDone(int32_t done, const wxString& newMessage = wxT(""));

	/**
	 * Sets the scale of the loading bar.
	 * Calling this with (50, 80) means that setting 50 as 'done',
	 * it will display as 0% loaded, 80 will display as 100% loaded.
	 */
	void SetLoadScale(int32_t from, int32_t to);

	/**
	 * Destroys (hides) the current loading bar.
	 */
	void DestroyLoadBar();

	bool IsRenderingEnabled() const {return disabled_counter == 0;}

	void EnableHotkeys();
	void DisableHotkeys();
	bool AreHotkeysEnabled() const;

	// This sends the event to the main window (redirecting from other controls)
	void AddPendingCanvasEvent(wxEvent& event);

protected:
	void DisableRendering() {++disabled_counter;}
	void EnableRendering() {--disabled_counter;}

public:
	void SetTitle(wxString newtitle);
	void UpdateTitle();
	void UpdateMenus();
	void SetStatusText(wxString text);

	long PopupDialog(wxWindow* parent, wxString title, wxString text, long style, wxString configsavename = wxEmptyString, uint32_t configsavevalue = 0);
	long PopupDialog(wxString title, wxString text, long style, wxString configsavename = wxEmptyString, uint32_t configsavevalue = 0);

	void ListDialog(wxWindow* parent, wxString title, const wxArrayString& vec);
	void ListDialog(const wxString& title, const wxArrayString& vec) { ListDialog(nullptr, title, vec); }

	void ShowTextBox(wxWindow* parent, const wxString& title, const wxString& text);
	void ShowTextBox(const wxString& title, const wxString& text) { ShowTextBox(nullptr, title, text); }

	// Get the current GL context
	// Param is required if the context is to be created.
	wxGLContext* GetGLContext(wxGLCanvas* win);

	// Search Results
	SearchResultWindow* ShowSearchWindow();
	void HideSearchWindow();

	ScriptingWindow* ShowScriptingWindow();
	void HideScriptingWindow();

	// Minimap
	void CreateMinimap();
	void HideMinimap();
	void DestroyMinimap();
	void UpdateMinimap(bool immediate = false);
	bool IsMinimapVisible() const;

	int GetCurrentFloor();
	void ChangeFloor(int newfloor);

	double GetCurrentZoom();
	void SetCurrentZoom(double zoom);

	void SwitchMode();
	void SetSelectionMode();
	void SetDrawingMode();
	bool IsSelectionMode() const {return mode == SELECTION_MODE;}
	bool IsDrawingMode() const {return mode == DRAWING_MODE;}

	void SetHotkey(int index, Hotkey& hotkey);
	const Hotkey& GetHotkey(int index) const;
	void SaveHotkeys() const;
	void LoadHotkeys();

	// Brushes
	void FillDoodadPreviewBuffer();
	// Selects the currently seleceted brush in the active palette
	void SelectBrush();
	// Updates the palette AND selects the brush, second parameter is first palette to look in
	// Returns true if the brush was found and selected
	bool SelectBrush(const Brush* brush, PaletteType pt = TILESET_UNKNOWN);
	// Selects the brush selected before the current brush
	void SelectPreviousBrush();
	// Only selects the brush, doesn't update the palette
	void SelectBrushInternal(Brush* brush);
	// Get different brush parameters
	Brush* GetCurrentBrush() const;
	BrushShape GetBrushShape() const;
	int GetBrushSize() const;
	int GetBrushVariation() const;
	int GetSpawnTime() const;

	// Additional brush parameters
	void SetSpawnTime(int time) {creature_spawntime = time;}
	void SetBrushSize(int nz);
	void SetBrushSizeInternal(int nz);
	void SetBrushShape(BrushShape bs);
	void SetBrushVariation(int nz);
	void SetBrushThickness(int low, int ceil);
	void SetBrushThickness(bool on, int low = -1, int ceil = -1);
	// Helper functions for size
	void DecreaseBrushSize(bool wrap = false);
	void IncreaseBrushSize(bool wrap = false);

	// Fetch different useful directories
	static wxString GetExecDirectory();
	static wxString GetDataDirectory();
	static wxString GetLocalDataDirectory();
	static wxString GetLocalDirectory();
	static wxString GetExtensionsDirectory();

	void discoverDataDirectory(const wxString& existentFile);
	wxString getFoundDataDirectory() { return m_dataDirectory; }

	// Load/unload a client version (takes care of dialogs aswell)
	void UnloadVersion();
	bool LoadVersion(ClientVersionID ver, wxString& error, wxArrayString& warnings, bool force = false);
	// The current version loaded (returns CLIENT_VERSION_NONE if no version is loaded)
	const ClientVersion& GetCurrentVersion() const;
	ClientVersionID GetCurrentVersionID() const;
	// If any version is loaded at all
	bool IsVersionLoaded() const {return loaded_version != CLIENT_VERSION_NONE;}

	// Centers current view on position
	bool CenterOnPosition(Position pos);
	// Refresh the view canvas
	void RefreshView();
	// Fit all/specified current map view to map dimensions
	void FitViewToMap();
	void FitViewToMap(MapTab* mt);
	// Start a pasting session
	bool isPasting() {return pasting;}
	void StartPasting();
	void EndPasting();
	void DoPaste();
	bool CanUndo();
	bool CanRedo();
	bool DoUndo();
	bool DoRedo();

	// Editor interface
	EditorTab* GetCurrentTab();
	EditorTab* GetTab(int idx);
	int GetTabCount() const;
	bool IsEditorOpen() const;
	void CloseCurrentEditor();
	Editor* GetCurrentEditor();
	int GetEditorIndex(Editor* editor) const;
	void SetCurrentEditor(Editor* editor);
	void SetCurrentEditor(int index);
	Editor* GetEditorAt(int index);
	MapTab* GetCurrentMapTab() const;
	void CycleTab(bool forward = true);
	bool CloseLiveEditors(LiveSocket* sock);
	bool CloseAllEditors();
	void NewMapView();

	// Map
	Map& GetCurrentMap();
	int GetOpenMapCount();
	bool ShouldSave();
	void SaveCurrentMap(FileName filename, bool showdialog); // "" means default filename
	void SaveCurrentMap(bool showdialog = true) { SaveCurrentMap(wxString(wxT("")), showdialog); }
	bool NewMap();
	bool LoadMap(FileName fn);

protected:
	bool LoadDataFiles(wxString& error, wxArrayString& warnings);
	ClientVersion* getLoadedVersion() const {
		return loaded_version == CLIENT_VERSION_NONE ? nullptr : ClientVersion::get(loaded_version);
	}

	//=========================================================================
	// Palette Interface
public:
	// Spawn a newd palette
	PaletteWindow* NewPalette();
	// Bring this palette to the front (as the 'active' palette)
	void ActivatePalette(PaletteWindow* p);
	// Rebuild forces palette to reload the entire contents
	void RebuildPalettes();
	// Refresh only updates the content (such as house/waypoint list)
	void RefreshPalettes(Map* m = nullptr, bool usedfault = true);
	// Won't refresh the palette in the parameter
	void RefreshOtherPalettes(PaletteWindow* p);
	// If no palette is shown, this displays the primary palette
	// else does nothing.
	void ShowPalette();
	// Select a particular page on the primary palette
	void SelectPalettePage(PaletteType pt);

	// Returns primary palette
	PaletteWindow* GetPalette();
	// Returns list of all palette, first in the list is primary
	const std::list<PaletteWindow*>& GetPalettes();

	// Hidden from public view
protected:
	void DestroyPalettes();
	PaletteWindow* CreatePalette();

	//=========================================================================
	// Public members
	//=========================================================================
public:
	wxString m_dataDirectory;
	wxAuiManager* aui_manager;
	MapTabbook* tabbook;
	MainFrame* root; // The main frame
	CopyBuffer copybuffer;

	MinimapWindow* minimap;
	DCButton* gem; // The small gem in the lower-right corner
	SearchResultWindow* search_result_window;
	ScriptingWindow* scripting_window;
	GraphicManager gfx;

	BaseMap* secondary_map; // A buffer map
	BaseMap* doodad_buffer_map; // The map in which doodads are temporarily stored

	//=========================================================================
	// Brush references
	//=========================================================================

	HouseBrush* house_brush;
	HouseExitBrush* house_exit_brush;
	WaypointBrush* waypoint_brush;
	OptionalBorderBrush* optional_brush;
	EraserBrush* eraser;
	SpawnBrush* spawn_brush;
	DoorBrush* normal_door_brush;
	DoorBrush* locked_door_brush;
	DoorBrush* magic_door_brush;
	DoorBrush* quest_door_brush;
	DoorBrush* hatch_door_brush;
	DoorBrush* window_door_brush;
	FlagBrush* pz_brush;
	FlagBrush* rook_brush;
	FlagBrush* nolog_brush;
	FlagBrush* pvp_brush;

protected:

	//=========================================================================
	// Global GUI state
	//=========================================================================
	typedef std::list<PaletteWindow*> PaletteList;
	PaletteList palettes;

	wxGLContext* OGLContext;

	ClientVersionID loaded_version;
	EditorMode mode;
	bool pasting;

	Hotkey hotkeys[10];
	bool hotkeys_enabled;

	//=========================================================================
	// Internal brush data
	//=========================================================================
	Brush* current_brush;
	Brush* previous_brush;
	BrushShape brush_shape;
	int brush_size;
	int brush_variation;
	int creature_spawntime;

	bool use_custom_thickness;
	float custom_thickness_mod;

	//=========================================================================
	// Progress bar tracking
	//=========================================================================
	wxString progressText;
	wxGenericProgressDialog* progressBar;

	int32_t progressFrom;
	int32_t progressTo;
	int32_t currentProgress;

	wxWindowDisabler* winDisabler;
	int disabled_counter;

	friend class RenderingLock;
	friend MapTab::MapTab(MapTabbook*, Editor*);
	friend MapTab::MapTab(const MapTab*);
};

extern GUI g_gui;

class RenderingLock
{
	bool acquired;
public:
	RenderingLock() : acquired(true)
	{
		g_gui.DisableRendering();
	}
	~RenderingLock()
	{
		release();
	}
	void release()
	{
		g_gui.EnableRendering();
		acquired = false;
	}
};

/**
 * Will push a loading bar when it is constructed
 * which will the be popped when it destructs.
 * Look in the GUI class for documentation of what the methods mean.
 */
class ScopedLoadingBar
{
public:
	ScopedLoadingBar(wxString message, bool canCancel = false)
	{
		g_gui.CreateLoadBar(message, canCancel);
	}
	~ScopedLoadingBar()
	{
		g_gui.DestroyLoadBar();
	}

	void SetLoadDone(int32_t done, const wxString& newmessage = wxEmptyString)
	{
		g_gui.SetLoadDone(done, newmessage);
	}

	void SetLoadScale(int32_t from, int32_t to)
	{
		g_gui.SetLoadScale(from, to);
	}
};

#define UnnamedRenderingLock() RenderingLock __unnamed_rendering_lock_##__LINE__

void SetWindowToolTip(wxWindow* a, const wxString& tip);
void SetWindowToolTip(wxWindow* a, wxWindow* b, const wxString& tip);

#endif
