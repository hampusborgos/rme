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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/minimap_window.hpp $
// $Id: minimap_window.hpp 310 2010-02-26 18:03:48Z admin $

#include "main.h"

#include "graphics.h"
#include "editor.h"
#include "map.h"

#include "gui.h"
#include "map_display.h"
#include "minimap_window.h"

BEGIN_EVENT_TABLE(MinimapWindow, wxPanel)
	EVT_LEFT_DOWN(MinimapWindow::OnMouseClick)
	EVT_SIZE(MinimapWindow::OnSize)
	EVT_PAINT(MinimapWindow::OnPaint)
	EVT_ERASE_BACKGROUND(MinimapWindow::OnEraseBackground)
	EVT_CLOSE(MinimapWindow::OnClose)
	EVT_TIMER(wxID_ANY, MinimapWindow::OnDelayedUpdate)
	EVT_KEY_DOWN(MinimapWindow::OnKey)
END_EVENT_TABLE()

MinimapWindow::MinimapWindow(wxWindow* parent) : 
	wxPanel(parent, wxID_ANY, wxDefaultPosition, wxSize(205, 130)),
	update_timer(this)
{
	for(int i = 0; i < 256; ++i) {
		pens[i] = newd wxPen(wxColor(minimap_color[i].red, minimap_color[i].green, minimap_color[i].blue));
	}
}

MinimapWindow::~MinimapWindow() {
	for(int i = 0; i < 256; ++i) {
		delete pens[i];
	}
}

void MinimapWindow::OnSize(wxSizeEvent& event) {
	Refresh();
}

void MinimapWindow::OnClose(wxCloseEvent&) {
	gui.DestroyMinimap();
}

void MinimapWindow::DelayedUpdate() {
	// We only updated the window AFTER actions have taken place, that
	// way we don't waste too much performance on updating this window
	update_timer.Start(settings.getInteger(Config::MINIMAP_UPDATE_DELAY), true);
}

void MinimapWindow::OnDelayedUpdate(wxTimerEvent& event) {
	Refresh();
}

void MinimapWindow::OnPaint(wxPaintEvent& event) {
	wxBufferedPaintDC pdc(this);
	
	pdc.SetBackground(*wxBLACK_BRUSH);
	pdc.Clear();

	if(gui.IsEditorOpen() == false) return;
	Editor& editor = *gui.GetCurrentEditor();
	
	int window_width = GetSize().GetWidth();
	int window_height = GetSize().GetHeight();
	//printf("W:%d\tH:%d\n", window_width, window_height);
	int center_x, center_y;
	
	MapCanvas* canvas = gui.GetCurrentMapTab()->GetCanvas();
	canvas->GetScreenCenter(&center_x, &center_y);

	int start_x, start_y;
	int end_x, end_y;
	start_x = center_x - window_width/2;
	start_y = center_y - window_height/2;
	
	end_x = center_x + window_width/2;
	end_y = center_y + window_height/2;
	
	if(start_x < 0) {
		start_x = 0;
		end_x = window_width;
	} else if(end_x > editor.map.getWidth()) {
		start_x = editor.map.getWidth() - window_width;
		end_x = editor.map.getWidth();
	}
	if(start_y < 0) {
		start_y = 0;
		end_y = window_height;
	} else if(end_y > editor.map.getHeight()) {
		start_y = editor.map.getHeight() - window_height;
		end_y = editor.map.getHeight();
	}

	start_x = max(start_x, 0);
	start_y = max(start_y, 0);
	end_x = min(end_x, editor.map.getWidth());
	end_y = min(end_y, editor.map.getHeight());

	last_start_x = start_x;
	last_start_y = start_y;

	int floor = gui.GetCurrentFloor();

	//printf("Draw from %d:%d to %d:%d\n", start_x, start_y, end_x, end_y);
	uint8_t last = 0;
	if(gui.IsRenderingEnabled()) {
		for(int y = start_y, window_y = 0; y <= end_y; ++y, ++window_y) {
			for(int x = start_x, window_x = 0; x <= end_x; ++x, ++window_x) {
				Tile* tile = editor.map.getTile(x, y, floor);
				if(tile) {
					uint8_t color = tile->getMiniMapColor();
					if(color) {
						if(last != color) {
							pdc.SetPen(*pens[color]);
							last = color;
						}
						pdc.DrawPoint(window_x, window_y);
					}
				}
			}
		}

		if(settings.getInteger(Config::MINIMAP_VIEW_BOX))
		{
			pdc.SetPen(*wxWHITE_PEN);
			// Draw the rectangle on the minimap
			
			// Some view info
			int screensize_x, screensize_y;
			int view_scroll_x, view_scroll_y;

			canvas->GetViewBox(&view_scroll_x, &view_scroll_y, &screensize_x, &screensize_y);

			// bounds of the view
			int view_start_x, view_start_y;
			int view_end_x, view_end_y;

			int tile_size = int(32/canvas->GetZoom()); // after zoom

			int floor_offset = (floor > 7? 0 : (7 - floor));

			view_start_x = view_scroll_x / 32 + floor_offset;
			view_start_y = view_scroll_y / 32 + floor_offset;

			view_end_x = view_start_x + screensize_x / tile_size + 1;
			view_end_y = view_start_y + screensize_y / tile_size + 1;

			for(int x = view_start_x; x <= view_end_x; ++x)
			{
				pdc.DrawPoint(x - start_x, view_start_y - start_y);
				pdc.DrawPoint(x - start_x, view_end_y - start_y);
			}
			for(int y = view_start_y; y < view_end_y; ++y)
			{
				pdc.DrawPoint(view_start_x - start_x, y - start_y);
				pdc.DrawPoint(view_end_x - start_x, y - start_y);
			}
		}
	}
}

void MinimapWindow::OnMouseClick(wxMouseEvent& event) {
	if(gui.IsEditorOpen() == false) return;
	int new_map_x = last_start_x + event.GetX();
	int new_map_y = last_start_y + event.GetY();
	gui.CenterOnPosition(Position(new_map_x, new_map_y, gui.GetCurrentFloor()));
	Refresh();
	gui.RefreshView();
}

void MinimapWindow::OnKey(wxKeyEvent& event) {
	if(gui.GetCurrentTab() != nullptr) {
		gui.GetCurrentMapTab()->GetEventHandler()->AddPendingEvent(event);
	}
}
