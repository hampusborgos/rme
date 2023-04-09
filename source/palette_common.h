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

#ifndef RME_PALETTE_COMMONS_H_
#define RME_PALETTE_COMMONS_H_

#include "main.h"

#include "dcbutton.h"
#include "tileset.h"
#include "gui_ids.h"
#include "common_windows.h"

class GUI;
class BrushButton;
class TilesetCategory;
class BrushSizePanel;
class BrushPalettePanel;
class PaletteWindow;

typedef TilesetCategoryType PaletteType;

class BrushButton : public ItemToggleButton {
public:
	BrushButton(wxWindow* parent, Brush* brush, RenderSize, uint32_t id = wxID_ANY);
	BrushButton(wxWindow* parent, Brush* brush, RenderSize, EditorSprite* espr, uint32_t id = wxID_ANY);
	virtual ~BrushButton();

	Brush* brush;

	void OnKey(wxKeyEvent& event);

	DECLARE_EVENT_TABLE()
};

class PalettePanel : public wxPanel {
public:
	PalettePanel(wxWindow* parent, wxWindowID id = wxID_ANY, long style = wxTAB_TRAVERSAL);
	~PalettePanel();

	// Interface
	// Flushes this panel and consequent views will feature reloaded data
	virtual void InvalidateContents();
	// Loads the currently displayed page
	virtual void LoadCurrentContents();
	// Loads all content in this panel
	virtual void LoadAllContents();

	PaletteWindow* GetParentPalette() const;
	virtual wxString GetName() const;
	virtual PaletteType GetType() const;

	// Add a tool panel!
	virtual void AddToolPanel(PalettePanel* panel);
	// Sets the style for this toolbar and child toolabrs
	virtual void SetToolbarIconSize(bool large_icons);

	// Select the first brush
	virtual void SelectFirstBrush();
	// Returns the currently selected brush (First brush if panel is not loaded)
	virtual Brush* GetSelectedBrush() const;
	// Returns the currently selected brush size
	virtual int GetSelectedBrushSize() const;
	// Select the brush in the parameter, this only changes the look of the panel
	virtual bool SelectBrush(const Brush* whatbrush);

	// Updates the palette window to use the current brush size
	virtual void OnUpdateBrushSize(BrushShape shape, int size);
	// Called when this page is about to be displayed
	virtual void OnSwitchIn();
	// Called when this page is hidden
	virtual void OnSwitchOut();
	// Called sometimes
	virtual void OnUpdate();
	// When the palette should do a delayed refresh (necessary for multiple palettes)
	void OnRefreshTimer(wxTimerEvent&);

	void RefreshOtherPalettes();
protected:
	typedef std::vector<PalettePanel*> ToolBarList;
	ToolBarList tool_bars;
	wxTimer refresh_timer;
	int last_brush_size;

	DECLARE_EVENT_TABLE();
};

class BrushSizePanel : public PalettePanel {
public:
	BrushSizePanel(wxWindow* parent);
	~BrushSizePanel() {}


	// Interface
	// Flushes this panel and consequent views will feature reloaded data
	void InvalidateContents();
	// Loads the currently displayed page
	void LoadCurrentContents();
	// Loads all content in this panel
	void LoadAllContents();

	wxString GetName() const;
	void SetToolbarIconSize(bool large);

	// Updates the palette window to use the current brush size
	void OnUpdateBrushSize(BrushShape shape, int size);
	// Called when this page is displayed
	void OnSwitchIn();

	// wxWidgets event handling
	void OnClickSquareBrush(wxCommandEvent& event);
	void OnClickCircleBrush(wxCommandEvent& event);

	void OnClickBrushSize(int which);
	void OnClickBrushSize0(wxCommandEvent& event) {OnClickBrushSize(0);}
	void OnClickBrushSize1(wxCommandEvent& event) {OnClickBrushSize(1);}
	void OnClickBrushSize2(wxCommandEvent& event) {OnClickBrushSize(2);}
	void OnClickBrushSize4(wxCommandEvent& event) {OnClickBrushSize(4);}
	void OnClickBrushSize6(wxCommandEvent& event) {OnClickBrushSize(6);}
	void OnClickBrushSize8(wxCommandEvent& event) {OnClickBrushSize(8);}
	void OnClickBrushSize11(wxCommandEvent& event){OnClickBrushSize(11);}

protected:
	bool loaded;
	bool large_icons;

	DCButton* brushshapeSquareButton;
	DCButton* brushshapeCircleButton;

	DCButton* brushsize0Button;
	DCButton* brushsize1Button;
	DCButton* brushsize2Button;
	DCButton* brushsize4Button;
	DCButton* brushsize6Button;
	DCButton* brushsize8Button;
	DCButton* brushsize11Button;

	DECLARE_EVENT_TABLE()
};

class BrushToolPanel : public PalettePanel {
public:
	BrushToolPanel(wxWindow* parent);
	~BrushToolPanel();

	// Interface
	// Flushes this panel and consequent views will feature reloaded data
	void InvalidateContents();
	// Loads the currently displayed page
	void LoadCurrentContents();
	// Loads all content in this panel
	void LoadAllContents();

	wxString GetName() const;
	void SetToolbarIconSize(bool large);

	// Returns the currently selected brush (First brush if panel is not loaded)
	Brush* GetSelectedBrush() const;
	// Select the brush in the parameter, this only changes the look of the panel
	bool SelectBrush(const Brush* whatbrush);

	// Called when this page is displayed
	void OnSwitchIn();

	// wxWidgets event handling
	void OnClickGravelButton(wxCommandEvent& event);
	void OnClickEraserButton(wxCommandEvent& event);
	// ----
	void OnClickNormalDoorButton(wxCommandEvent& event);
	void OnClickLockedDoorButton(wxCommandEvent& event);
	void OnClickMagicDoorButton(wxCommandEvent& event);
	void OnClickQuestDoorButton(wxCommandEvent& event);
	void OnClickHatchDoorButton(wxCommandEvent& event);
	void OnClickWindowDoorButton(wxCommandEvent& event);
	// ----
	void OnClickPZBrushButton(wxCommandEvent& event);
	void OnClickNOPVPBrushButton(wxCommandEvent& event);
	void OnClickNoLogoutBrushButton(wxCommandEvent& event);
	void OnClickPVPZoneBrushButton(wxCommandEvent& event);
public:
	void DeselectAll();

	bool loaded;
	bool large_icons;

	BrushButton* optionalBorderButton;
	BrushButton* eraserButton;
	// ----
	BrushButton* normalDoorButton;
	BrushButton* lockedDoorButton;
	BrushButton* magicDoorButton;
	BrushButton* questDoorButton;
	BrushButton* hatchDoorButton;
	BrushButton* windowDoorButton;
	// ----
	BrushButton* pzBrushButton;
	BrushButton* nopvpBrushButton;
	BrushButton* nologBrushButton;
	BrushButton* pvpzoneBrushButton;

	DECLARE_EVENT_TABLE()
};

class BrushThicknessPanel : public PalettePanel {
public:
	BrushThicknessPanel(wxWindow* parent);
	~BrushThicknessPanel();

	// Interface
	wxString GetName() const;

	// Called when this page is displayed
	void OnSwitchIn();

	// wxWidgets event handling
	void OnScroll(wxScrollEvent& event);
	void OnClickCustomThickness(wxCommandEvent& event);
public:
	wxSlider* slider;
	wxCheckBox* use_button;

	DECLARE_EVENT_TABLE()
};

#endif
