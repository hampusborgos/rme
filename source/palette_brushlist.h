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

#ifndef RME_PALETTE_BRUSHLIST_
#define RME_PALETTE_BRUSHLIST_

#include "main.h"
#include "palette_common.h"

enum BrushListType {
	BRUSHLIST_LARGE_ICONS,
	BRUSHLIST_SMALL_ICONS,
	BRUSHLIST_LISTBOX,
	BRUSHLIST_TEXT_LISTBOX,
};

class BrushBoxInterface {
public:
	BrushBoxInterface(const TilesetCategory* _tileset) : tileset(_tileset), loaded(false) {ASSERT(tileset);}
	virtual ~BrushBoxInterface() {}

	virtual wxWindow* GetSelfWindow() = 0;

	// Select the first brush
	virtual void SelectFirstBrush() = 0;
	// Returns the currently selected brush (First brush if panel is not loaded)
	virtual Brush* GetSelectedBrush() const = 0;
	// Select the brush in the parameter, this only changes the look of the panel
	virtual bool SelectBrush(const Brush* brush) = 0;
protected:
	const TilesetCategory* const tileset;
	bool loaded;
};

class BrushListBox : public wxVListBox, public BrushBoxInterface {
public:
	BrushListBox(wxWindow* parent, const TilesetCategory* _tileset);
	~BrushListBox();

	wxWindow* GetSelfWindow() {return this;}

	// Select the first brush
	void SelectFirstBrush();
	// Returns the currently selected brush (First brush if panel is not loaded)
	Brush* GetSelectedBrush() const;
	// Select the brush in the parameter, this only changes the look of the panel
	bool SelectBrush(const Brush* brush);

	// Event handlers
    virtual void OnDrawItem(wxDC& dc, const wxRect& rect, size_t n) const;
	virtual wxCoord OnMeasureItem(size_t n) const;

	void OnKey(wxKeyEvent& event);

	DECLARE_EVENT_TABLE();
};

class BrushIconBox : public wxScrolledWindow, public BrushBoxInterface {
public:
	BrushIconBox(wxWindow* parent, const TilesetCategory* _tileset, RenderSize rsz);
	~BrushIconBox();

	wxWindow* GetSelfWindow() {return this;}

	// Scrolls the window to the position of the named brush button
	void EnsureVisible(BrushButton* btn);
	void EnsureVisible(size_t n);

	// Select the first brush
	void SelectFirstBrush();
	// Returns the currently selected brush (First brush if panel is not loaded)
	Brush* GetSelectedBrush() const;
	// Select the brush in the parameter, this only changes the look of the panel
	bool SelectBrush(const Brush* brush);

	// Event handling...
	void OnClickBrushButton(wxCommandEvent& event);
protected:
	// Used internally to deselect all buttons before selecting a newd one.
	void DeselectAll();
protected:
	std::vector<BrushButton*> brush_buttons;
	RenderSize icon_size;

	DECLARE_EVENT_TABLE();
};

// A panel capapable of displaying a collection of brushes
// Brushes can be arranged in either list or icon fashion
// Contents are *not* created when the panel is created,
// but on the first call to LoadContents(), this is to
// allow procedural loading (faster)

class BrushPanel : public wxPanel {
public:
	BrushPanel(wxWindow* parent);
	~BrushPanel();

	// Interface
	// Flushes this panel and consequent views will feature reloaded data
	void InvalidateContents();
	// Loads the content (This must be called before the panel is displayed, else it will appear empty
	void LoadContents();

	// Sets the display type (list or icons)
	void SetListType(BrushListType ltype);
	void SetListType(wxString ltype);
	// Assigns a tileset to this list
	void AssignTileset(const TilesetCategory* tileset);

	// Select the first brush
	void SelectFirstBrush();
	// Returns the currently selected brush (First brush if panel is not loaded)
	Brush* GetSelectedBrush() const;
	// Select the brush in the parameter, this only changes the look of the panel
	bool SelectBrush(const Brush* whatbrush);

	// Called when the window is about to be displayed
	void OnSwitchIn();
	// Called when this page is hidden
	void OnSwitchOut();

	// wxWidgets event handlers
	void OnClickListBoxRow(wxCommandEvent& event);

protected:
	const TilesetCategory* tileset;
	wxSizer* sizer;
	BrushBoxInterface* brushbox;
	bool loaded;
	BrushListType list_type;

	DECLARE_EVENT_TABLE();
};

class BrushPalettePanel : public PalettePanel {
public:
	BrushPalettePanel(wxWindow* parent, const TilesetContainer& tilesets, TilesetCategoryType category, wxWindowID id = wxID_ANY);
	~BrushPalettePanel();

	// Interface
	// Flushes this panel and consequent views will feature reloaded data
	void InvalidateContents();
	// Loads the currently displayed page
	void LoadCurrentContents();
	// Loads all content in this panel
	void LoadAllContents();

	PaletteType GetType() const;

	// Sets the display type (list or icons)
	void SetListType(BrushListType ltype);
	void SetListType(wxString ltype);

	// Select the first brush
	void SelectFirstBrush();
	// Returns the currently selected brush (first brush if panel is not loaded)
	Brush* GetSelectedBrush() const;
	// Select the brush in the parameter, this only changes the look of the panel
	bool SelectBrush(const Brush* whatbrush);

	// Called when this page is displayed
	void OnSwitchIn();

	// Event handler for child window
	void OnSwitchingPage(wxChoicebookEvent& event);
	void OnPageChanged(wxChoicebookEvent& event);
protected:
	PaletteType palette_type;
	wxChoicebook* choicebook;
	BrushSizePanel* size_panel;
	std::map<wxWindow*, Brush*> remembered_brushes;

	DECLARE_EVENT_TABLE();
};

#endif
