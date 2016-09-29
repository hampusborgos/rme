//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////

#ifndef RME_PALETTE_H_
#define RME_PALETTE_H_

#include "palette_common.h"

class BrushPalettePanel;
class CreaturePalettePanel;
class HousePalettePanel;
class WaypointPalettePanel;

class PaletteWindow : public wxPanel {
public:
	PaletteWindow(wxWindow* parent, const TilesetContainer& tilesets);
	~PaletteWindow();

	// Interface
	// Reloads layout settings from settings (and using map)
	void ReloadSettings(Map* from);
	// Flushes all pages and forces them to be reloaded from the palette data again
	void InvalidateContents();
	// (Re)Loads all currently displayed data, called from InvalidateContents implicitly
	void LoadCurrentContents();
	// Goes to the selected page and selects any brush there
	void SelectPage(PaletteType palette);
	// The currently selected brush in this palette
	Brush* GetSelectedBrush() const;
	// The currently selected brush size in this palette
	int GetSelectedBrushSize() const;
	// The currently selected page (terrain, doodad...)
	PaletteType GetSelectedPage() const;

	// Custom Event handlers (something has changed?)
	// Finds the brush pointed to by whatbrush and selects it as the current brush (also changes page)
	// Returns if the brush was found in this palette
	virtual bool OnSelectBrush(const Brush* whatbrush, PaletteType primary = TILESET_UNKNOWN);
	// Updates the palette window to use the current brush size
	virtual void OnUpdateBrushSize(BrushShape shape, int size);
	// Updates the content of the palette (eg. houses, creatures)
	virtual void OnUpdate(Map* map);

	// wxWidgets Event Handlers
	void OnSwitchingPage(wxChoicebookEvent& event);
	void OnPageChanged(wxChoicebookEvent& event);
	// Forward key events to the parent window (The Map Window)
	void OnKey(wxKeyEvent& event);
	void OnClose(wxCloseEvent&);
protected:
	static PalettePanel* CreateTerrainPalette(wxWindow* parent, const TilesetContainer& tilesets);
	static PalettePanel* CreateDoodadPalette(wxWindow* parent, const TilesetContainer& tilesets);
	static PalettePanel* CreateItemPalette(wxWindow* parent, const TilesetContainer& tilesets);
	static PalettePanel* CreateCreaturePalette(wxWindow* parent, const TilesetContainer& tilesets);
	static PalettePanel* CreateHousePalette(wxWindow* parent, const TilesetContainer& tilesets);
	static PalettePanel* CreateWaypointPalette(wxWindow* parent, const TilesetContainer& tilesets);
	static PalettePanel* CreateRAWPalette(wxWindow* parent, const TilesetContainer& tilesets);

	wxChoicebook* choicebook;

	BrushPalettePanel* terrain_palette;
	BrushPalettePanel* doodad_palette;
	BrushPalettePanel* item_palette;
	CreaturePalettePanel* creature_palette;
	HousePalettePanel* house_palette;
	WaypointPalettePanel* waypoint_palette;
	BrushPalettePanel* raw_palette;

	DECLARE_EVENT_TABLE()
};

#endif
