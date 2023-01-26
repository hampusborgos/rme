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

#ifndef RME_DISPLAY_WINDOW_H_
#define RME_DISPLAY_WINDOW_H_

#include "action.h"
#include "tile.h"
#include "creature.h"

class Item;
class Creature;
class MapWindow;
class MapPopupMenu;
class AnimationTimer;
class MapDrawer;

class MapCanvas : public wxGLCanvas
{
public:
	MapCanvas(MapWindow* parent, Editor& editor, int* attriblist);
	virtual ~MapCanvas();
	void Reset();

	// All events
	void OnPaint(wxPaintEvent& event);
	void OnEraseBackground(wxEraseEvent& event) {}

	void OnMouseMove(wxMouseEvent& event);
	void OnMouseLeftRelease(wxMouseEvent& event);
	void OnMouseLeftClick(wxMouseEvent& event);
	void OnMouseLeftDoubleClick(wxMouseEvent& event);
	void OnMouseCenterClick(wxMouseEvent& event);
	void OnMouseCenterRelease(wxMouseEvent& event);
	void OnMouseRightClick(wxMouseEvent& event);
	void OnMouseRightRelease(wxMouseEvent& event);

	void OnKeyDown(wxKeyEvent& event);
	void OnKeyUp(wxKeyEvent& event);
	void OnWheel(wxMouseEvent& event);
	void OnGainMouse(wxMouseEvent& event);
	void OnLoseMouse(wxMouseEvent& event);

	// Mouse events handlers (called by the above)
	void OnMouseActionRelease(wxMouseEvent& event);
	void OnMouseActionClick(wxMouseEvent& event);
	void OnMouseCameraClick(wxMouseEvent& event);
	void OnMouseCameraRelease(wxMouseEvent& event);
	void OnMousePropertiesClick(wxMouseEvent& event);
	void OnMousePropertiesRelease(wxMouseEvent& event);

	//
	void OnCut(wxCommandEvent& event);
	void OnCopy(wxCommandEvent& event);
	void OnCopyPosition(wxCommandEvent& event);
	void OnCopyServerId(wxCommandEvent& event);
	void OnCopyClientId(wxCommandEvent& event);
	void OnCopyName(wxCommandEvent& event);
	void OnBrowseTile(wxCommandEvent& event);
	void OnPaste(wxCommandEvent& event);
	void OnDelete(wxCommandEvent& event);
	// ----
	void OnGotoDestination(wxCommandEvent& event);
	void OnCopyDestination(wxCommandEvent& event);
	void OnRotateItem(wxCommandEvent& event);
	void OnSwitchDoor(wxCommandEvent& event);
	// ----
	void OnSelectRAWBrush(wxCommandEvent& event);
	void OnSelectGroundBrush(wxCommandEvent& event);
	void OnSelectDoodadBrush(wxCommandEvent& event);
	void OnSelectDoorBrush(wxCommandEvent& event);
	void OnSelectWallBrush(wxCommandEvent& event);
	void OnSelectCarpetBrush(wxCommandEvent& event);
	void OnSelectTableBrush(wxCommandEvent& event);
	void OnSelectCreatureBrush(wxCommandEvent& event);
	void OnSelectSpawnBrush(wxCommandEvent& event);
	void OnSelectHouseBrush(wxCommandEvent& event);
	// ---
	void OnProperties(wxCommandEvent& event);

	void Refresh();

	void ScreenToMap(int screen_x, int screen_y, int* map_x, int* map_y);
	void MouseToMap(int* map_x, int* map_y) { ScreenToMap(cursor_x, cursor_y, map_x, map_y); }
	void GetScreenCenter(int* map_x, int* map_y);

	void StartPasting();
	void EndPasting();
	void EnterSelectionMode();
	void EnterDrawingMode();

	void UpdatePositionStatus(int x = -1, int y = -1);
	void UpdateZoomStatus();

	void ChangeFloor(int new_floor);
	int GetFloor() const { return floor; }
	double GetZoom() const { return zoom; }
	void SetZoom(double value);
	void GetViewBox(int* view_scroll_x, int* view_scroll_y, int* screensize_x, int* screensize_y) const;

	MapWindow* GetMapWindow() const;
	Position GetCursorPosition() const;

	void ShowPositionIndicator(const Position& position);
	void TakeScreenshot(wxFileName path, wxString format);

protected:
	void getTilesToDraw(int mouse_map_x, int mouse_map_y, int floor, PositionVector* tilestodraw, PositionVector* tilestoborder, bool fill = false);
	bool floodFill(Map *map, const Position& center, int x, int y, GroundBrush* brush, PositionVector* positions);

private:
	enum {
		BLOCK_SIZE = 64
	};

	inline int getFillIndex(int x, int y) const noexcept { return ((y % BLOCK_SIZE) * BLOCK_SIZE) + (x % BLOCK_SIZE); }

	static bool processed[BLOCK_SIZE*BLOCK_SIZE];

	Editor& editor;
	MapDrawer *drawer;
	int keyCode;

// View related
	int floor;
	double zoom;
	int cursor_x;
	int cursor_y;

	bool dragging;
	bool boundbox_selection;
	bool screendragging;
	bool isPasting() const;
	bool drawing;
	bool dragging_draw;
	bool replace_dragging;

	uint8_t* screenshot_buffer;

	int drag_start_x;
	int drag_start_y;
	int drag_start_z;

	int last_cursor_map_x;
	int last_cursor_map_y;
	int last_cursor_map_z;

	int last_click_map_x;
	int last_click_map_y;
	int last_click_map_z;
	int last_click_abs_x;
	int last_click_abs_y;
	int last_click_x;
	int last_click_y;

	int last_mmb_click_x;
	int last_mmb_click_y;

	int view_scroll_x;
	int view_scroll_y;

	uint32_t current_house_id;

	wxStopWatch refresh_watch;
	MapPopupMenu* popup_menu;
	AnimationTimer* animation_timer;

	friend class MapDrawer;

	DECLARE_EVENT_TABLE()
};

// Right-click popup menu
class MapPopupMenu : public wxMenu {
public:
	MapPopupMenu(Editor& editor);
	virtual ~MapPopupMenu();

	void Update();
protected:
	Editor& editor;
};

class AnimationTimer : public wxTimer
{
public:
	AnimationTimer(MapCanvas *canvas);
	~AnimationTimer();

	void Notify();
	void Start();
	void Stop();

private:
	MapCanvas *map_canvas;
	bool started;
};

#endif
