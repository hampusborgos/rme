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

#ifndef RME_MAP_WINDOW_H_
#define RME_MAP_WINDOW_H_

#include "position.h"
#include "replace_items_window.h"

class MapCanvas;
class DCButton;

// Map window, a window displaying a map, complete with scrollbars
// and everything. This is the window that's inside each tab in the
// editor. Does NOT control any map rendering or editing at all.
// MapCanvas does that. (mapdisplay.h)
class MapWindow : public wxPanel
{
public:
	MapWindow(wxWindow* parent, Editor& editor);
	virtual ~MapWindow();

	// Event handlers
	void OnSize(wxSizeEvent& event);
	void OnScroll(wxScrollEvent& event);
	void OnScrollLineDown(wxScrollEvent& event);
	void OnScrollLineUp(wxScrollEvent& event);
	void OnScrollPageDown(wxScrollEvent& event);
	void OnScrollPageUp(wxScrollEvent& event);
	void OnGem(wxCommandEvent& event);

	// Custom interface for MapWindow

	// GetViewSize returns the size of the containing canvas, in pixels
	void GetViewSize(int* x, int* y);
	// Returns the start of the camera on the map, in pixels
	// Actually is scroll thumb positions
	void GetViewStart(int* x, int* y);

	// Set size of this window (in pixels)
	// if center is true, the camera will be moved to the center of the map.
	void SetSize(int x, int y, bool center = false);

	// Scroll to the specified, absolute position (in pixels)
	void Scroll(int x, int y, bool center = false);

	// Scroll this many pixels in X/Y, relative to current position
	void ScrollRelative(int x, int y);

	// Resize scrollbars to fit to the map dimensions
	// This needs to be called after updating map height/width
	void FitToMap();

	// Screen position.
	Position GetScreenCenterPosition();
	void SetScreenCenterPosition(const Position& position);
	void GoToPreviousCenterPosition();

	// Return the containing canvas
	MapCanvas* GetCanvas() const { return canvas; }

	void ShowReplaceItemsDialog(bool selectionOnly);
	void CloseReplaceItemsDialog();
	void OnReplaceItemsDialogClose(wxCloseEvent& event);

protected:
	// For internal use, call to resize the scrollbars with
	// the newd dimensions of *this* window
	void UpdateScrollbars(int nx, int ny);
	void UpdateDialogs(bool show);

protected:
	Editor& editor;
	DCButton* gem;
	MapCanvas* canvas;
	wxScrollBar* hScroll;
	wxScrollBar* vScroll;

private:
	ReplaceItemsDialog* replaceItemsDialog;
	Position previous_position;

	friend class MainFrame;
	friend class MapCanvas;

	DECLARE_EVENT_TABLE()
};

// MapScrollbar, a special scrollbar that relays alot of events
// to the canvas, which allows scrolling when the scrollbar has
// focus (even though it also resents focus as hard as it can.
class MapScrollBar : public wxScrollBar
{
public:
	MapScrollBar(MapWindow* parent, wxWindowID id, long style, wxWindow* canvas) :
	  wxScrollBar(parent, id, wxDefaultPosition, wxDefaultSize, style), canvas(canvas) {}
	virtual ~MapScrollBar() {}

	void OnKey(wxKeyEvent& event) {canvas->GetEventHandler()->AddPendingEvent(event);}
	void OnWheel(wxMouseEvent& event) {canvas->GetEventHandler()->AddPendingEvent(event);}
	void OnFocus(wxFocusEvent& event) {canvas->SetFocus();}

	wxWindow* canvas;
	DECLARE_EVENT_TABLE()
};

#endif
