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
// $URL: http://svn.rebarp.se/svn/RME/trunk/source/minimap_window.h $
// $Id: minimap_window.h 298 2010-02-23 17:09:13Z admin $

#ifndef RME_MINIMAP_WINDOW_H_
#define RME_MINIMAP_WINDOW_H_

class MinimapWindow : public wxPanel {
public:
	MinimapWindow(wxWindow* parent);
	virtual ~MinimapWindow();

	void OnPaint(wxPaintEvent&);
	void OnEraseBackground(wxEraseEvent&) {}
	void OnMouseClick(wxMouseEvent&);
	void OnSize(wxSizeEvent&);
	void OnClose(wxCloseEvent&);

	void DelayedUpdate();
	void OnDelayedUpdate(wxTimerEvent& event);
	void OnKey(wxKeyEvent& event);
protected:
	wxPen* pens[256];
	wxTimer	update_timer;
	int last_start_x;
	int last_start_y;

	DECLARE_EVENT_TABLE()
};

#endif
