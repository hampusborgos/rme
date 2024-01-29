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

#include "main.h"

#include "gui.h"
#include "editor.h"
#include "brush.h"
#include "sprites.h"
#include "map.h"
#include "tile.h"
#include "old_properties_window.h"
#include "properties_window.h"
#include "palette_window.h"
#include "map_display.h"
#include "map_drawer.h"
#include "application.h"
#include "live_server.h"
#include "browse_tile_window.h"

#include "doodad_brush.h"
#include "house_exit_brush.h"
#include "house_brush.h"
#include "wall_brush.h"
#include "spawn_monster_brush.h"
#include "monster_brush.h"
#include "ground_brush.h"
#include "waypoint_brush.h"
#include "raw_brush.h"
#include "carpet_brush.h"
#include "table_brush.h"
#include "spawn_npc_brush.h"
#include "npc_brush.h"

BEGIN_EVENT_TABLE(MapCanvas, wxGLCanvas)
	EVT_KEY_DOWN(MapCanvas::OnKeyDown)
	EVT_KEY_DOWN(MapCanvas::OnKeyUp)

	// Mouse events
	EVT_MOTION(MapCanvas::OnMouseMove)
	EVT_LEFT_UP(MapCanvas::OnMouseLeftRelease)
	EVT_LEFT_DOWN(MapCanvas::OnMouseLeftClick)
	EVT_LEFT_DCLICK(MapCanvas::OnMouseLeftDoubleClick)
	EVT_MIDDLE_DOWN(MapCanvas::OnMouseCenterClick)
	EVT_MIDDLE_UP(MapCanvas::OnMouseCenterRelease)
	EVT_RIGHT_DOWN(MapCanvas::OnMouseRightClick)
	EVT_RIGHT_UP(MapCanvas::OnMouseRightRelease)
	EVT_MOUSEWHEEL(MapCanvas::OnWheel)
	EVT_ENTER_WINDOW(MapCanvas::OnGainMouse)
	EVT_LEAVE_WINDOW(MapCanvas::OnLoseMouse)

	//Drawing events
	EVT_PAINT(MapCanvas::OnPaint)
	EVT_ERASE_BACKGROUND(MapCanvas::OnEraseBackground)

	// Menu events
	EVT_MENU(MAP_POPUP_MENU_CUT, MapCanvas::OnCut)
	EVT_MENU(MAP_POPUP_MENU_COPY, MapCanvas::OnCopy)
	EVT_MENU(MAP_POPUP_MENU_COPY_POSITION, MapCanvas::OnCopyPosition)
	EVT_MENU(MAP_POPUP_MENU_PASTE, MapCanvas::OnPaste)
	EVT_MENU(MAP_POPUP_MENU_DELETE, MapCanvas::OnDelete)
	//----
	EVT_MENU(MAP_POPUP_MENU_COPY_ITEM_ID, MapCanvas::OnCopyItemId)
	EVT_MENU(MAP_POPUP_MENU_COPY_NAME, MapCanvas::OnCopyName)
	// ----
	EVT_MENU(MAP_POPUP_MENU_ROTATE, MapCanvas::OnRotateItem)
	EVT_MENU(MAP_POPUP_MENU_GOTO, MapCanvas::OnGotoDestination)
	EVT_MENU(MAP_POPUP_MENU_SWITCH_DOOR, MapCanvas::OnSwitchDoor)
	// ----
	EVT_MENU(MAP_POPUP_MENU_SELECT_RAW_BRUSH, MapCanvas::OnSelectRAWBrush)
	EVT_MENU(MAP_POPUP_MENU_SELECT_GROUND_BRUSH, MapCanvas::OnSelectGroundBrush)
	EVT_MENU(MAP_POPUP_MENU_SELECT_DOODAD_BRUSH, MapCanvas::OnSelectDoodadBrush)
	EVT_MENU(MAP_POPUP_MENU_SELECT_DOOR_BRUSH, MapCanvas::OnSelectDoorBrush)
	EVT_MENU(MAP_POPUP_MENU_SELECT_WALL_BRUSH, MapCanvas::OnSelectWallBrush)
	EVT_MENU(MAP_POPUP_MENU_SELECT_CARPET_BRUSH, MapCanvas::OnSelectCarpetBrush)
	EVT_MENU(MAP_POPUP_MENU_SELECT_TABLE_BRUSH, MapCanvas::OnSelectTableBrush)
	EVT_MENU(MAP_POPUP_MENU_SELECT_MONSTER_BRUSH, MapCanvas::OnSelectMonsterBrush)
	EVT_MENU(MAP_POPUP_MENU_SELECT_SPAWN_BRUSH, MapCanvas::OnSelectSpawnBrush)
	EVT_MENU(MAP_POPUP_MENU_SELECT_NPC_BRUSH, MapCanvas::OnSelectNpcBrush)
	EVT_MENU(MAP_POPUP_MENU_SELECT_SPAWN_NPC_BRUSH, MapCanvas::OnSelectSpawnNpcBrush)
	EVT_MENU(MAP_POPUP_MENU_SELECT_HOUSE_BRUSH, MapCanvas::OnSelectHouseBrush)
	// ----
	EVT_MENU(MAP_POPUP_MENU_PROPERTIES, MapCanvas::OnProperties)
	// ----
	EVT_MENU(MAP_POPUP_MENU_BROWSE_TILE, MapCanvas::OnBrowseTile)
END_EVENT_TABLE()

bool MapCanvas::processed[] = {0};

MapCanvas::MapCanvas(MapWindow* parent, Editor& editor, int* attriblist) :
	wxGLCanvas(parent, wxID_ANY, nullptr, wxDefaultPosition, wxDefaultSize, wxWANTS_CHARS),
	editor(editor),
	floor(GROUND_LAYER),
	zoom(1.0),
	cursor_x(-1),
	cursor_y(-1),
	dragging(false),
	boundbox_selection(false),
	screendragging(false),
	drawing(false),
	dragging_draw(false),
	replace_dragging(false),

	screenshot_buffer(nullptr),

	drag_start_x(-1),
	drag_start_y(-1),
	drag_start_z(-1),

	last_cursor_map_x(-1),
	last_cursor_map_y(-1),
	last_cursor_map_z(-1),

	last_click_map_x(-1),
	last_click_map_y(-1),
	last_click_map_z(-1),
	last_click_abs_x(-1),
	last_click_abs_y(-1),
	last_click_x(-1),
	last_click_y(-1),

	last_mmb_click_x(-1),
	last_mmb_click_y(-1)
{
	popup_menu = newd MapPopupMenu(editor);
	animation_timer = newd AnimationTimer(this);
	drawer = new MapDrawer(this);
	keyCode = WXK_NONE;
}

MapCanvas::~MapCanvas()
{
	delete popup_menu;
	popup_menu = nullptr;
	delete animation_timer;
	animation_timer = nullptr;
	delete drawer;
	drawer = nullptr;
	free(screenshot_buffer);
}

void MapCanvas::Refresh()
{
	if(refresh_watch.Time() > g_settings.getInteger(Config::HARD_REFRESH_RATE)) {
		refresh_watch.Start();
		wxGLCanvas::Update();
	}
	wxGLCanvas::Refresh();
}

void MapCanvas::SetZoom(double value)
{
	if(value < 0.125)
		value = 0.125;

	if(value > 25.00)
		value = 25.0;

	if(zoom != value) {
		int center_x, center_y;
		GetScreenCenter(&center_x, &center_y);

		zoom = value;
		static_cast<MapWindow*>(GetParent())->SetScreenCenterPosition(Position(center_x, center_y, floor));

		UpdatePositionStatus();
		UpdateZoomStatus();
		Refresh();
	}
}

void MapCanvas::GetViewBox(int* view_scroll_x, int* view_scroll_y, int* screensize_x, int* screensize_y) const
{
	static_cast<MapWindow*>(GetParent())->GetViewSize(screensize_x, screensize_y);
	static_cast<MapWindow*>(GetParent())->GetViewStart(view_scroll_x, view_scroll_y);
}

void MapCanvas::OnPaint(wxPaintEvent& event)
{
	SetCurrent(*g_gui.GetGLContext(this));

	if(g_gui.IsRenderingEnabled()) {
		DrawingOptions& options = drawer->getOptions();
		if(screenshot_buffer) {
			options.SetIngame();
		} else {
			options.transparent_floors = g_settings.getBoolean(Config::TRANSPARENT_FLOORS);
			options.transparent_items = g_settings.getBoolean(Config::TRANSPARENT_ITEMS);
			options.show_ingame_box = g_settings.getBoolean(Config::SHOW_INGAME_BOX);
			options.show_grid = g_settings.getInteger(Config::SHOW_GRID);
			options.ingame = !g_settings.getBoolean(Config::SHOW_EXTRA);
			options.show_all_floors = g_settings.getBoolean(Config::SHOW_ALL_FLOORS);
			options.show_monsters = g_settings.getBoolean(Config::SHOW_MONSTERS);
			options.show_spawns_monster = g_settings.getBoolean(Config::SHOW_SPAWNS_MONSTER);
			options.show_npcs = g_settings.getBoolean(Config::SHOW_NPCS);
			options.show_spawns_npc = g_settings.getBoolean(Config::SHOW_SPAWNS_NPC);
			options.show_houses = g_settings.getBoolean(Config::SHOW_HOUSES);
			options.show_shade = g_settings.getBoolean(Config::SHOW_SHADE);
			options.show_special_tiles = g_settings.getBoolean(Config::SHOW_SPECIAL_TILES);
			options.show_items = g_settings.getBoolean(Config::SHOW_ITEMS);
			options.highlight_items = g_settings.getBoolean(Config::HIGHLIGHT_ITEMS);
			options.show_blocking = g_settings.getBoolean(Config::SHOW_BLOCKING);
			options.show_tooltips = g_settings.getBoolean(Config::SHOW_TOOLTIPS);
			options.show_as_minimap = g_settings.getBoolean(Config::SHOW_AS_MINIMAP);
			options.show_only_colors = g_settings.getBoolean(Config::SHOW_ONLY_TILEFLAGS);
			options.show_only_modified = g_settings.getBoolean(Config::SHOW_ONLY_MODIFIED_TILES);
			options.show_preview = g_settings.getBoolean(Config::SHOW_PREVIEW);
			options.show_hooks = g_settings.getBoolean(Config::SHOW_WALL_HOOKS);
			options.hide_items_when_zoomed = g_settings.getBoolean(Config::HIDE_ITEMS_WHEN_ZOOMED);
		}

		options.dragging = boundbox_selection;

		if(options.show_preview)
			animation_timer->Start();
		else
			animation_timer->Stop();

		drawer->SetupVars();
		drawer->SetupGL();
		drawer->Draw();

		if(screenshot_buffer)
			drawer->TakeScreenshot(screenshot_buffer);

		drawer->Release();
	}

	// Clean unused textures
	g_gui.gfx.garbageCollection();

	// Swap buffer
	SwapBuffers();

	// Send newd node requests
	editor.SendNodeRequests();
}

void MapCanvas::TakeScreenshot(wxFileName path, wxString format)
{
	int screensize_x, screensize_y;
	GetViewBox(&view_scroll_x, &view_scroll_y, &screensize_x, &screensize_y);

	delete[] screenshot_buffer;
	screenshot_buffer = newd uint8_t[3 * screensize_x * screensize_y];

	// Draw the window
	Refresh();
	wxGLCanvas::Update(); // Forces immediate redraws the window.

	// screenshot_buffer should now contain the screenbuffer
	if(screenshot_buffer == nullptr) {
		g_gui.PopupDialog("Capture failed", "Image capture failed. Old Video Driver?", wxOK);
	} else {
		// We got the shit
		int screensize_x, screensize_y;
		static_cast<MapWindow*>(GetParent())->GetViewSize(&screensize_x, &screensize_y);
		wxImage screenshot(screensize_x, screensize_y, screenshot_buffer);

		time_t t = time(nullptr);
		struct tm current_time;
		#if defined(__WINDOWS__)
		localtime_s(&current_time, &t);
		#else
		localtime_r(current_time, &t);
		#endif

		wxString date;
		date << "screenshot_" << (1900 + current_time.tm_year);
		if(current_time.tm_mon < 9) {
			date << "-" << "0" << current_time.tm_mon+1;
		} else {
			date << "-" << current_time.tm_mon+1;
		}
		date << "-" << current_time.tm_mday;
		date << "-" << current_time.tm_hour;
		date << "-" << current_time.tm_min;
		date << "-" << current_time.tm_sec;

		int type = 0;
		path.SetName(date);
		if(format == "bmp") {
			path.SetExt(format);
			type = wxBITMAP_TYPE_BMP;
		} else if(format == "png") {
			path.SetExt(format);
			type = wxBITMAP_TYPE_PNG;
		} else if(format == "jpg" || format == "jpeg") {
			path.SetExt(format);
			type = wxBITMAP_TYPE_JPEG;
		} else if(format == "tga") {
			path.SetExt(format);
			type = wxBITMAP_TYPE_TGA;
		} else {
			g_gui.SetStatusText("Unknown screenshot format \'" + format + "\", switching to default (png)");
			path.SetExt("png");;
			type = wxBITMAP_TYPE_PNG;
		}

		path.Mkdir(0755, wxPATH_MKDIR_FULL);
		wxFileOutputStream of(path.GetFullPath());
		if(of.IsOk()) {
			if(screenshot.SaveFile(of, static_cast<wxBitmapType>(type)))
				g_gui.SetStatusText("Took screenshot and saved as " + path.GetFullName());
			else
				g_gui.PopupDialog("File error", "Couldn't save image file correctly.", wxOK);
		} else {
			g_gui.PopupDialog("File error", "Couldn't open file " + path.GetFullPath() + " for writing.", wxOK);
		}

	}

	Refresh();

	screenshot_buffer = nullptr;
}

void MapCanvas::ScreenToMap(int screen_x, int screen_y, int* map_x, int* map_y)
{
	int start_x, start_y;
	static_cast<MapWindow*>(GetParent())->GetViewStart(&start_x, &start_y);

	screen_x *= GetContentScaleFactor();
	screen_y *= GetContentScaleFactor();

	if(screen_x < 0) {
		*map_x = (start_x + screen_x) / TILE_SIZE;
	} else {
		*map_x = int(start_x + (screen_x * zoom)) / TILE_SIZE;
	}

	if(screen_y < 0) {
		*map_y = (start_y + screen_y) / TILE_SIZE;
	} else {
		*map_y = int(start_y + (screen_y * zoom)) / TILE_SIZE;
	}

	if(floor <= GROUND_LAYER) {
		*map_x += GROUND_LAYER - floor;
		*map_y += GROUND_LAYER - floor;
	}/* else {
		*map_x += MAP_MAX_LAYER - floor;
		*map_y += MAP_MAX_LAYER - floor;
	}*/
}

void MapCanvas::GetScreenCenter(int* map_x, int* map_y)
{
	int width, height;
	static_cast<MapWindow*>(GetParent())->GetViewSize(&width, &height);
	return ScreenToMap(width/2, height/2, map_x, map_y);
}

Position MapCanvas::GetCursorPosition() const
{
	return Position(last_cursor_map_x, last_cursor_map_y, floor);
}

void MapCanvas::UpdatePositionStatus(int x, int y)
{
	if(x == -1) x = cursor_x;
	if(y == -1) y = cursor_y;

	int map_x, map_y;
	ScreenToMap(x, y, &map_x, &map_y);

	wxString ss;
	ss << "x: " << map_x << " y:" << map_y << " z:" << floor;
	g_gui.root->SetStatusText(ss,2);

	ss = "";
	Tile* tile = editor.map.getTile(map_x, map_y, floor);
	if(tile) {
		if(tile->spawnMonster && g_settings.getInteger(Config::SHOW_SPAWNS_MONSTER)) {
			ss << "Monster spawn radius: " << tile->spawnMonster->getSize();
		} else if(tile->monster && g_settings.getInteger(Config::SHOW_MONSTERS)) {
			ss << ("Monster");
			ss << " \"" << wxstr(tile->monster->getName()) << "\" spawntime: " << tile->monster->getSpawnMonsterTime();
		} else if(tile->spawnNpc && g_settings.getInteger(Config::SHOW_SPAWNS_NPC)) {
			ss << "Npc spawn radius: " << tile->spawnNpc->getSize();
		} else if(tile->npc && g_settings.getInteger(Config::SHOW_NPCS)) {
			ss << ("NPC");
			ss << " \"" << wxstr(tile->npc->getName()) << "\" spawntime: " << tile->npc->getSpawnNpcTime();
		} else if(Item* item = tile->getTopItem()) {
			ss << "Item \"" << wxstr(item->getName()) << "\"";
			ss << " id:" << item->getID();
			if(item->getUniqueID()) ss << " uid:" << item->getUniqueID();
			if(item->getActionID()) ss << " aid:" << item->getActionID();
			if(item->hasWeight()) {
				wxString s;
				s.Printf("%.2f", item->getWeight());
				ss << " weight: " << s;
			}
		} else {
			ss << "Nothing";
		}
	} else {
		ss << "Nothing";
	}

	if(editor.IsLive()) {
		editor.GetLive().updateCursor(Position(map_x, map_y, floor));
	}

	g_gui.root->SetStatusText(ss, 1);
}

void MapCanvas::UpdateZoomStatus()
{
	int percentage = (int)((1.0 / zoom) * 100);
	wxString ss;
	ss << "zoom: " << percentage << "%";
	g_gui.root->SetStatusText(ss, 3);
}

void MapCanvas::OnMouseMove(wxMouseEvent& event)
{
	if(screendragging) {
		static_cast<MapWindow*>(GetParent())->ScrollRelative(int(g_settings.getFloat(Config::SCROLL_SPEED) * zoom*(event.GetX() - cursor_x)), int(g_settings.getFloat(Config::SCROLL_SPEED) * zoom*(event.GetY() - cursor_y)));
		Refresh();
	}

	cursor_x = event.GetX();
	cursor_y = event.GetY();

	int mouse_map_x, mouse_map_y;
	MouseToMap(&mouse_map_x,&mouse_map_y);

	bool map_update = false;
	if(last_cursor_map_x != mouse_map_x || last_cursor_map_y != mouse_map_y || last_cursor_map_z != floor) {
		map_update = true;
	}
	last_cursor_map_x = mouse_map_x;
	last_cursor_map_y = mouse_map_y;
	last_cursor_map_z = floor;

	if(map_update) {
		UpdatePositionStatus(cursor_x, cursor_y);
		UpdateZoomStatus();
	}

	if(g_gui.IsSelectionMode()) {
		if(map_update && isPasting()) {
			Refresh();
		} else if(map_update && dragging) {
			wxString ss;

			int move_x = drag_start_x - mouse_map_x;
			int move_y = drag_start_y - mouse_map_y;
			int move_z = drag_start_z - floor;
			ss << "Dragging " << -move_x << "," << -move_y << "," << -move_z;
			g_gui.SetStatusText(ss);

			Refresh();
		} else if(boundbox_selection) {
			if(map_update) {
				wxString ss;

				int move_x = std::abs(last_click_map_x - mouse_map_x);
				int move_y = std::abs(last_click_map_y - mouse_map_y);
				ss << "Selection " << move_x+1 << ":" << move_y+1;
				g_gui.SetStatusText(ss);
			}

			Refresh();
		}
	} else { // Drawing mode
		Brush* brush = g_gui.GetCurrentBrush();
		if(map_update && drawing && brush) {
			if(brush->isDoodad()) {
				if(event.ControlDown()) {
					PositionVector tilestodraw;
					getTilesToDraw(mouse_map_x, mouse_map_y, floor, &tilestodraw, nullptr);
					editor.undraw(tilestodraw, event.ShiftDown() || event.AltDown());
				} else {
					editor.draw(Position(mouse_map_x, mouse_map_y, floor), event.ShiftDown() || event.AltDown());
				}
			} else if(brush->isDoor()) {
				if(!brush->canDraw(&editor.map, Position(mouse_map_x, mouse_map_y, floor))) {
					// We don't have to waste an action in this case...
				} else {
					PositionVector tilestodraw;
					PositionVector tilestoborder;

					tilestodraw.push_back(Position(mouse_map_x, mouse_map_y, floor));

					tilestoborder.push_back(Position(mouse_map_x    , mouse_map_y - 1, floor));
					tilestoborder.push_back(Position(mouse_map_x - 1, mouse_map_y    , floor));
					tilestoborder.push_back(Position(mouse_map_x    , mouse_map_y + 1, floor));
					tilestoborder.push_back(Position(mouse_map_x + 1, mouse_map_y    , floor));

					if(event.ControlDown()) {
						editor.undraw(tilestodraw, tilestoborder, event.AltDown());
					} else {
						editor.draw(tilestodraw, tilestoborder, event.AltDown());
					}
				}
			} else if(brush->needBorders()) {
				PositionVector tilestodraw, tilestoborder;

				getTilesToDraw(mouse_map_x, mouse_map_y, floor, &tilestodraw, &tilestoborder);

				if(event.ControlDown()) {
					editor.undraw(tilestodraw, tilestoborder, event.AltDown());
				} else {
					editor.draw(tilestodraw, tilestoborder, event.AltDown());
				}
			} else if(brush->oneSizeFitsAll()) {
				drawing = true;
				PositionVector tilestodraw;
				tilestodraw.push_back(Position(mouse_map_x,mouse_map_y, floor));

				if(event.ControlDown()) {
					editor.undraw(tilestodraw, event.AltDown());
				} else {
					editor.draw(tilestodraw, event.AltDown());
				}
			} else { // No borders
				PositionVector tilestodraw;

				for(int y = -g_gui.GetBrushSize(); y <= g_gui.GetBrushSize(); y++) {
					for(int x = -g_gui.GetBrushSize(); x <= g_gui.GetBrushSize(); x++) {
						if(g_gui.GetBrushShape() == BRUSHSHAPE_SQUARE) {
							tilestodraw.push_back(Position(mouse_map_x+x,mouse_map_y+y, floor));
						} else if(g_gui.GetBrushShape() == BRUSHSHAPE_CIRCLE) {
							double distance = sqrt(double(x*x) + double(y*y));
							if(distance < g_gui.GetBrushSize()+0.005) {
								tilestodraw.push_back(Position(mouse_map_x+x,mouse_map_y+y, floor));
							}
						}
					}
				}
				if(event.ControlDown()) {
					editor.undraw(tilestodraw, event.AltDown());
				} else {
					editor.draw(tilestodraw, event.AltDown());
				}
			}

			// Create newd doodad layout (does nothing if a non-doodad brush is selected)
			g_gui.FillDoodadPreviewBuffer();

			g_gui.RefreshView();
		} else if(dragging_draw) {
			g_gui.RefreshView();
		} else if (map_update && brush) {
			Refresh();
		}
	}
}

void MapCanvas::OnMouseLeftRelease(wxMouseEvent& event)
{
	OnMouseActionRelease(event);
}

void MapCanvas::OnMouseLeftClick(wxMouseEvent& event)
{
	OnMouseActionClick(event);
}

void MapCanvas::OnMouseLeftDoubleClick(wxMouseEvent& event)
{
	if(g_settings.getInteger(Config::DOUBLECLICK_PROPERTIES)) {
		int mouse_map_x, mouse_map_y;
		ScreenToMap(event.GetX(), event.GetY(), &mouse_map_x, &mouse_map_y);
		Tile* tile = editor.map.getTile(mouse_map_x, mouse_map_y, floor);

		if(tile && tile->size() > 0) {
			Tile* new_tile = tile->deepCopy(editor.map);
			wxDialog* w = nullptr;
			// Show monster spawn
			if(new_tile->spawnMonster && g_settings.getInteger(Config::SHOW_SPAWNS_MONSTER))
				w = newd OldPropertiesWindow(g_gui.root, &editor.map, new_tile, new_tile->spawnMonster);
			// Show monster
			else if(new_tile->monster && g_settings.getInteger(Config::SHOW_MONSTERS))
				w = newd OldPropertiesWindow(g_gui.root, &editor.map, new_tile, new_tile->monster);
			// Show npc
			else if(new_tile->npc && g_settings.getInteger(Config::SHOW_NPCS))
				w = newd OldPropertiesWindow(g_gui.root, &editor.map, new_tile, new_tile->npc);
			// Show npc spawn
			else if(new_tile->spawnNpc && g_settings.getInteger(Config::SHOW_SPAWNS_NPC))
				w = newd OldPropertiesWindow(g_gui.root, &editor.map, new_tile, new_tile->spawnNpc);
			else if(Item* item = new_tile->getTopItem()) {
				if(editor.map.getVersion().otbm >= MAP_OTBM_4)
					w = newd PropertiesWindow(g_gui.root, &editor.map, new_tile, item);
				else
					w = newd OldPropertiesWindow(g_gui.root, &editor.map, new_tile, item);
			} else
				return;

			int ret = w->ShowModal();
			if(ret != 0) {
				Action* action = editor.actionQueue->createAction(ACTION_CHANGE_PROPERTIES);
				action->addChange(newd Change(new_tile));
				editor.addAction(action);
			} else {
				// Cancel!
				delete new_tile;
			}
			w->Destroy();
		}
	}
}

void MapCanvas::OnMouseCenterClick(wxMouseEvent& event)
{
	if(g_settings.getInteger(Config::SWITCH_MOUSEBUTTONS)) {
		OnMousePropertiesClick(event);
	} else {
		OnMouseCameraClick(event);
	}
}

void MapCanvas::OnMouseCenterRelease(wxMouseEvent& event)
{
	if(g_settings.getInteger(Config::SWITCH_MOUSEBUTTONS)) {
		OnMousePropertiesRelease(event);
	} else {
		OnMouseCameraRelease(event);
	}
}

void MapCanvas::OnMouseRightClick(wxMouseEvent& event)
{
	if(g_settings.getInteger(Config::SWITCH_MOUSEBUTTONS)) {
		OnMouseCameraClick(event);
	} else {
		OnMousePropertiesClick(event);
	}
}

void MapCanvas::OnMouseRightRelease(wxMouseEvent& event)
{
	if(g_settings.getInteger(Config::SWITCH_MOUSEBUTTONS)) {
		OnMouseCameraRelease(event);
	} else {
		OnMousePropertiesRelease(event);
	}
}

void MapCanvas::OnMouseActionClick(wxMouseEvent& event)
{
	SetFocus();

	int mouse_map_x, mouse_map_y;
	ScreenToMap(event.GetX(), event.GetY(), &mouse_map_x, &mouse_map_y);

	if(event.ControlDown() && event.AltDown()) {
		Tile* tile = editor.map.getTile(mouse_map_x, mouse_map_y, floor);
		if(tile && tile->size() > 0) {
			Item* item = tile->getTopItem();
			if(item && item->getRAWBrush())
				g_gui.SelectBrush(item->getRAWBrush(), TILESET_RAW);
		}
	} else if(g_gui.IsSelectionMode()) {
		if(isPasting()) {
			// Set paste to false (no rendering etc.)
			EndPasting();

			// Paste to the map
			editor.copybuffer.paste(editor, Position(mouse_map_x, mouse_map_y, floor));

			// Start dragging
			dragging = true;
			drag_start_x = mouse_map_x;
			drag_start_y = mouse_map_y;
			drag_start_z = floor;
		} else do {
			boundbox_selection = false;
			if(event.ShiftDown()) {
				boundbox_selection = true;

				if(!event.ControlDown()) {
					editor.selection.start(); // Start selection session
					editor.selection.clear(); // Clear out selection
					editor.selection.finish(); // End selection session
					editor.selection.updateSelectionCount();
				}
			} else if(event.ControlDown()) {
				Tile* tile = editor.map.getTile(mouse_map_x, mouse_map_y, floor);
				if(tile) {
					// Show monster spawn
					if(tile->spawnMonster && g_settings.getInteger(Config::SHOW_SPAWNS_MONSTER)) {
						editor.selection.start(); // Start selection session
						if(tile->spawnMonster->isSelected()) {
							editor.selection.remove(tile, tile->spawnMonster);
						} else {
							editor.selection.add(tile, tile->spawnMonster);
						}
						editor.selection.finish(); // Finish selection session
						editor.selection.updateSelectionCount();
					// Show monsters
					} else if(tile->monster && g_settings.getInteger(Config::SHOW_MONSTERS)) {
						editor.selection.start(); // Start selection session
						if(tile->monster->isSelected()) {
							editor.selection.remove(tile, tile->monster);
						} else {
							editor.selection.add(tile, tile->monster);
						}
						editor.selection.finish(); // Finish selection session
						editor.selection.updateSelectionCount();
					} else {
						Item* item = tile->getTopItem();
						if(item) {
							editor.selection.start(); // Start selection session
							if(item->isSelected()) {
								editor.selection.remove(tile, item);
							} else {
								editor.selection.add(tile, item);
							}
							editor.selection.finish(); // Finish selection session
							editor.selection.updateSelectionCount();
						}
					}
					// Show npcs
					if(tile->spawnNpc && g_settings.getInteger(Config::SHOW_SPAWNS_NPC)) {
						editor.selection.start(); // Start selection session
						if(tile->spawnNpc->isSelected()) {
							editor.selection.remove(tile, tile->spawnNpc);
						} else {
							editor.selection.add(tile, tile->spawnNpc);
						}
						editor.selection.finish(); // Finish selection session
						editor.selection.updateSelectionCount();
					} else if(tile->npc && g_settings.getInteger(Config::SHOW_NPCS)) {
						editor.selection.start(); // Start selection session
						if(tile->npc->isSelected()) {
							editor.selection.remove(tile, tile->npc);
						} else {
							editor.selection.add(tile, tile->npc);
						}
						editor.selection.finish(); // Finish selection session
						editor.selection.updateSelectionCount();
					// Show npc spawn
					} else {
						Item* item = tile->getTopItem();
						if(item) {
							editor.selection.start(); // Start selection session
							if(item->isSelected()) {
								editor.selection.remove(tile, item);
							} else {
								editor.selection.add(tile, item);
							}
							editor.selection.finish(); // Finish selection session
							editor.selection.updateSelectionCount();
						}
					}
				}
			} else {
				Tile* tile = editor.map.getTile(mouse_map_x, mouse_map_y, floor);
				if(!tile) {
					editor.selection.start(); // Start selection session
					editor.selection.clear(); // Clear out selection
					editor.selection.finish(); // End selection session
					editor.selection.updateSelectionCount();
				} else if(tile->isSelected()) {
					dragging = true;
					drag_start_x = mouse_map_x;
					drag_start_y = mouse_map_y;
					drag_start_z = floor;
				} else {
					editor.selection.start(); // Start a selection session
					editor.selection.clear();
					editor.selection.commit();
					// Show monster spawn
					if(tile->spawnMonster && g_settings.getInteger(Config::SHOW_SPAWNS_MONSTER)) {
						editor.selection.add(tile, tile->spawnMonster);
						dragging = true;
						drag_start_x = mouse_map_x;
						drag_start_y = mouse_map_y;
						drag_start_z = floor;
					// Show monsters
					} else if(tile->monster && g_settings.getInteger(Config::SHOW_MONSTERS)) {
						editor.selection.add(tile, tile->monster);
						dragging = true;
						drag_start_x = mouse_map_x;
						drag_start_y = mouse_map_y;
						drag_start_z = floor;
					// Show npc spawns
					} else if(tile->spawnNpc && g_settings.getInteger(Config::SHOW_SPAWNS_NPC)) {
						editor.selection.add(tile, tile->spawnNpc);
						dragging = true;
						drag_start_x = mouse_map_x;
						drag_start_y = mouse_map_y;
						drag_start_z = floor;
					// Show npcs
					} else if(tile->npc && g_settings.getInteger(Config::SHOW_NPCS)) {
						editor.selection.add(tile, tile->npc);
						dragging = true;
						drag_start_x = mouse_map_x;
						drag_start_y = mouse_map_y;
						drag_start_z = floor;
					} else {
						Item* item = tile->getTopItem();
						if(item) {
							editor.selection.add(tile, item);
							dragging = true;
							drag_start_x = mouse_map_x;
							drag_start_y = mouse_map_y;
							drag_start_z = floor;
						}
					}
					editor.selection.finish(); // Finish the selection session
					editor.selection.updateSelectionCount();
				}
			}
		} while(false);
	} else if(g_gui.GetCurrentBrush()) { // Drawing mode
		Brush* brush = g_gui.GetCurrentBrush();
		if(event.ShiftDown() && brush->canDrag()) {
			dragging_draw = true;
		} else {
			if(g_gui.GetBrushSize() == 0 && !brush->oneSizeFitsAll()) {
				drawing = true;
			} else {
				drawing = g_gui.GetCurrentBrush()->canSmear();
			}
			if(brush->isWall()) {
				if(event.AltDown() && g_gui.GetBrushSize() == 0) {
					// z0mg, just clicked a tile, shift variaton.
					if(event.ControlDown()) {
						editor.undraw(Position(mouse_map_x, mouse_map_y, floor), event.AltDown());
					} else {
						editor.draw(Position(mouse_map_x, mouse_map_y, floor), event.AltDown());
					}
				} else {
					PositionVector tilestodraw;
					PositionVector tilestoborder;

					int start_map_x = mouse_map_x - g_gui.GetBrushSize();
					int start_map_y = mouse_map_y - g_gui.GetBrushSize();
					int end_map_x   = mouse_map_x + g_gui.GetBrushSize();
					int end_map_y   = mouse_map_y + g_gui.GetBrushSize();

					for(int y = start_map_y -1; y <= end_map_y + 1; ++y) {
						for(int x = start_map_x - 1; x <= end_map_x + 1; ++x) {
							if((x <= start_map_x+1 || x >= end_map_x-1) || (y <= start_map_y+1 || y >= end_map_y-1)) {
								tilestoborder.push_back(Position(x,y,floor));
							}
							if(((x == start_map_x || x == end_map_x) || (y == start_map_y || y == end_map_y)) &&
								((x >= start_map_x && x <= end_map_x) && (y >= start_map_y && y <= end_map_y))) {
								tilestodraw.push_back(Position(x,y,floor));
							}
						}
					}
					if(event.ControlDown()) {
						editor.undraw(tilestodraw, tilestoborder, event.AltDown());
					} else {
						editor.draw(tilestodraw, tilestoborder, event.AltDown());
					}
				}
			} else if(brush->isDoor()) {
				PositionVector tilestodraw;
				PositionVector tilestoborder;

				tilestodraw.push_back(Position(mouse_map_x, mouse_map_y, floor));

				tilestoborder.push_back(Position(mouse_map_x    , mouse_map_y - 1, floor));
				tilestoborder.push_back(Position(mouse_map_x - 1, mouse_map_y    , floor));
				tilestoborder.push_back(Position(mouse_map_x    , mouse_map_y + 1, floor));
				tilestoborder.push_back(Position(mouse_map_x + 1, mouse_map_y    , floor));

				if(event.ControlDown()) {
					editor.undraw(tilestodraw, tilestoborder, event.AltDown());
				} else {
					editor.draw(tilestodraw, tilestoborder, event.AltDown());
				}
			} else if(brush->isDoodad() || brush->isSpawnMonster() || brush->isMonster()) {
				if(event.ControlDown()) {
					if(brush->isDoodad()) {
						PositionVector tilestodraw;
						getTilesToDraw(mouse_map_x, mouse_map_y, floor, &tilestodraw, nullptr);
						editor.undraw(tilestodraw, event.AltDown());
					} else {
						editor.undraw(Position(mouse_map_x, mouse_map_y, floor), event.ShiftDown() || event.AltDown());
					}
				} else {
					bool will_show_spawn = false;
					if(brush->isSpawnMonster() || brush->isMonster()) {
						if(!g_settings.getBoolean(Config::SHOW_SPAWNS_MONSTER)) {
							Tile* tile = editor.map.getTile(mouse_map_x, mouse_map_y, floor);
							if(!tile || !tile->spawnMonster) {
								will_show_spawn = true;
							}
						}
					}

					editor.draw(Position(mouse_map_x, mouse_map_y, floor), event.ShiftDown() || event.AltDown());

					if(will_show_spawn) {
						Tile* tile = editor.map.getTile(mouse_map_x, mouse_map_y, floor);
						if(tile && tile->spawnMonster) {
							g_settings.setInteger(Config::SHOW_SPAWNS_MONSTER, true);
							g_gui.UpdateMenubar();
						}
					}
				}
			} else if(brush->isDoodad() || brush->isSpawnNpc() || brush->isNpc()) {
				if(event.ControlDown()) {
					if(brush->isDoodad()) {
						PositionVector tilestodraw;
						getTilesToDraw(mouse_map_x, mouse_map_y, floor, &tilestodraw, nullptr);
						editor.undraw(tilestodraw, event.AltDown());
					} else {
						editor.undraw(Position(mouse_map_x, mouse_map_y, floor), event.ShiftDown() || event.AltDown());
					}
				} else {
					bool will_show_spawn_npc = false;
					if(brush->isSpawnNpc() || brush->isNpc()) {
						if(!g_settings.getBoolean(Config::SHOW_SPAWNS_NPC)) {
							Tile* tile = editor.map.getTile(mouse_map_x, mouse_map_y, floor);
							if(!tile || !tile->spawnNpc) {
								will_show_spawn_npc = true;
							}
						}
					}

					editor.draw(Position(mouse_map_x, mouse_map_y, floor), event.ShiftDown() || event.AltDown());

					if(will_show_spawn_npc) {
						Tile* tile = editor.map.getTile(mouse_map_x, mouse_map_y, floor);
						if(tile && tile->spawnNpc) {
							g_settings.setInteger(Config::SHOW_SPAWNS_NPC, true);
							g_gui.UpdateMenubar();
						}
					}
				}
			} else {
				if(brush->isGround() && event.AltDown()) {
					replace_dragging = true;
					Tile* draw_tile = editor.map.getTile(mouse_map_x, mouse_map_y, floor);
					if(draw_tile) {
						editor.replace_brush = draw_tile->getGroundBrush();
					} else {
						editor.replace_brush = nullptr;
					}
				}

				if(brush->needBorders()) {
					PositionVector tilestodraw;
					PositionVector tilestoborder;

					bool fill = keyCode == WXK_CONTROL_D && event.ControlDown() && brush->isGround();
					getTilesToDraw(mouse_map_x, mouse_map_y, floor, &tilestodraw, &tilestoborder, fill);

					if(!fill && event.ControlDown()) {
						editor.undraw(tilestodraw, tilestoborder, event.AltDown());
					} else {
						editor.draw(tilestodraw, tilestoborder, event.AltDown());
					}
				} else if(brush->oneSizeFitsAll()) {
					if(brush->isHouseExit() || brush->isWaypoint()) {
						editor.draw(Position(mouse_map_x, mouse_map_y, floor), event.AltDown());
					} else {
						PositionVector tilestodraw;
						tilestodraw.push_back(Position(mouse_map_x,mouse_map_y, floor));
						if(event.ControlDown()) {
							editor.undraw(tilestodraw, event.AltDown());
						} else {
							editor.draw(tilestodraw, event.AltDown());
						}
					}
				} else {
					PositionVector tilestodraw;

					getTilesToDraw(mouse_map_x, mouse_map_y, floor, &tilestodraw, nullptr);

					if(event.ControlDown()) {
						editor.undraw(tilestodraw, event.AltDown());
					} else {
						editor.draw(tilestodraw, event.AltDown());
					}
				}
			}
			// Change the doodad layout brush
			g_gui.FillDoodadPreviewBuffer();
		}
	}
	last_click_x = int(event.GetX()*zoom);
	last_click_y = int(event.GetY()*zoom);

	int start_x, start_y;
	static_cast<MapWindow*>(GetParent())->GetViewStart(&start_x, &start_y);
	last_click_abs_x = last_click_x + start_x;
	last_click_abs_y = last_click_y + start_y;

	last_click_map_x = mouse_map_x;
	last_click_map_y = mouse_map_y;
	last_click_map_z = floor;
	g_gui.RefreshView();
	g_gui.UpdateMinimap();
}

void MapCanvas::OnMouseActionRelease(wxMouseEvent& event)
{
	int mouse_map_x, mouse_map_y;
	ScreenToMap(event.GetX(), event.GetY(), &mouse_map_x, &mouse_map_y);

	int move_x = last_click_map_x - mouse_map_x;
	int move_y = last_click_map_y - mouse_map_y;
	int move_z = last_click_map_z - floor;

	if(g_gui.IsSelectionMode()) {
		if(dragging && (move_x != 0 || move_y != 0 || move_z != 0)) {
			editor.moveSelection(Position(move_x, move_y, move_z));
		} else {
			if(boundbox_selection) {
				if(mouse_map_x == last_click_map_x && mouse_map_y == last_click_map_y && event.ControlDown()) {
					// Mouse hasn't moved, do control+shift thingy!
					Tile* tile = editor.map.getTile(mouse_map_x, mouse_map_y, floor);
					if(tile) {
						editor.selection.start(); // Start a selection session
						if(tile->isSelected()) {
							editor.selection.remove(tile);
						} else {
							editor.selection.add(tile);
						}
						editor.selection.finish(); // Finish the selection session
						editor.selection.updateSelectionCount();
					}
				} else {
					// The cursor has moved, do some boundboxing!
					if(last_click_map_x > mouse_map_x) {
						int tmp = mouse_map_x;
						mouse_map_x = last_click_map_x;
						last_click_map_x = tmp;
					}
					if(last_click_map_y > mouse_map_y) {
						int tmp = mouse_map_y;
						mouse_map_y = last_click_map_y;
						last_click_map_y = tmp;
					}

					int numtiles = 0;
					int threadcount = std::max(g_settings.getInteger(Config::WORKER_THREADS), 1);

					int start_x = 0, start_y = 0, start_z = 0;
					int end_x = 0, end_y = 0, end_z = 0;

					switch(g_settings.getInteger(Config::SELECTION_TYPE)) {
						case SELECT_CURRENT_FLOOR: {
							start_z = end_z = floor;
							start_x = last_click_map_x;
							start_y = last_click_map_y;
							end_x = mouse_map_x;
							end_y = mouse_map_y;
							break;
						}
						case SELECT_ALL_FLOORS: {
							start_x = last_click_map_x;
							start_y = last_click_map_y;
							start_z = MAP_MAX_LAYER;
							end_x = mouse_map_x;
							end_y = mouse_map_y;
							end_z = floor;

							if(g_settings.getInteger(Config::COMPENSATED_SELECT)) {
								start_x -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);
								start_y -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);

								end_x -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);
								end_y -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);
							}

							numtiles = (start_z - end_z) * (end_x - start_x) * (end_y - start_y);
							break;
						}
						case SELECT_VISIBLE_FLOORS: {
							start_x = last_click_map_x;
							start_y = last_click_map_y;
							if(floor < 8) {
								start_z = GROUND_LAYER;
							} else {
								start_z = std::min(MAP_MAX_LAYER, floor + 2);
							}
							end_x = mouse_map_x;
							end_y = mouse_map_y;
							end_z = floor;

							if(g_settings.getInteger(Config::COMPENSATED_SELECT)) {
								start_x -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);
								start_y -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);

								end_x -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);
								end_y -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);
							}
							break;
						}
					}

					if(numtiles < 500) {
						// No point in threading for such a small set.
						threadcount = 1;
					}
					// Subdivide the selection area
					// We know it's a square, just split it into several areas
					int width = end_x - start_x;
					if(width < threadcount) {
						threadcount = std::min(1, width);
					}
					// Let's divide!
					int remainder = width;
					int cleared = 0;
					std::vector<SelectionThread*> threads;
					if(width == 0) {
						threads.push_back(newd SelectionThread(editor, Position(start_x, start_y, start_z), Position(start_x, end_y, end_z)));
					} else {
						for(int i = 0; i < threadcount; ++i) {
							int chunksize = width / threadcount;
							// The last threads takes all the remainder
							if(i == threadcount - 1) {
								chunksize = remainder;
							}
							threads.push_back(newd SelectionThread(editor, Position(start_x + cleared, start_y, start_z), Position(start_x + cleared + chunksize, end_y, end_z)));
							cleared += chunksize;
							remainder -= chunksize;
						}
					}
					ASSERT(cleared == width);
					ASSERT(remainder == 0);

					editor.selection.start(); // Start a selection session
					for(std::vector<SelectionThread*>::iterator iter = threads.begin(); iter != threads.end(); ++iter) {
						(*iter)->Execute();
					}
					for(std::vector<SelectionThread*>::iterator iter = threads.begin(); iter != threads.end(); ++iter) {
						editor.selection.join(*iter);
					}
					editor.selection.finish(); // Finish the selection session
					editor.selection.updateSelectionCount();
				}
			} else if(event.ControlDown()) {
				////
			} else {
				// User hasn't moved anything, meaning selection/deselection
				Tile* tile = editor.map.getTile(mouse_map_x, mouse_map_y, floor);
				if(tile) {
					if(tile->spawnMonster && g_settings.getInteger(Config::SHOW_SPAWNS_MONSTER)) {
						if(!tile->spawnMonster->isSelected()) {
							editor.selection.start(); // Start a selection session
							editor.selection.add(tile, tile->spawnMonster);
							editor.selection.finish(); // Finish the selection session
							editor.selection.updateSelectionCount();
						}
					} else if(tile->monster && g_settings.getInteger(Config::SHOW_MONSTERS)) {
						if(!tile->monster->isSelected()) {
							editor.selection.start(); // Start a selection session
							editor.selection.add(tile, tile->monster);
							editor.selection.finish(); // Finish the selection session
							editor.selection.updateSelectionCount();
						}
					} else if(tile->spawnNpc && g_settings.getInteger(Config::SHOW_SPAWNS_NPC)) {
						if(!tile->spawnNpc->isSelected()) {
							editor.selection.start(); // Start a selection session
							editor.selection.add(tile, tile->spawnNpc);
							editor.selection.finish(); // Finish the selection session
							editor.selection.updateSelectionCount();
						}
					} else if(tile->npc && g_settings.getInteger(Config::SHOW_NPCS)) {
						if(!tile->npc->isSelected()) {
							editor.selection.start(); // Start a selection session
							editor.selection.add(tile, tile->monster);
							editor.selection.finish(); // Finish the selection session
							editor.selection.updateSelectionCount();
						}
					} else {
						Item* item = tile->getTopItem();
						if(item && !item->isSelected()) {
							editor.selection.start(); // Start a selection session
							editor.selection.add(tile, item);
							editor.selection.finish(); // Finish the selection session
							editor.selection.updateSelectionCount();
						}
					}
				}
			}
		}
		editor.actionQueue->resetTimer();
		dragging = false;
		boundbox_selection = false;
	} else if(g_gui.GetCurrentBrush()){ // Drawing mode
		Brush* brush = g_gui.GetCurrentBrush();
		if(dragging_draw) {
			if(brush->isSpawnMonster()) {
				int start_map_x = std::min(last_click_map_x, mouse_map_x);
				int start_map_y = std::min(last_click_map_y, mouse_map_y);
				int end_map_x   = std::max(last_click_map_x, mouse_map_x);
				int end_map_y   = std::max(last_click_map_y, mouse_map_y);

				int map_x = start_map_x + (end_map_x - start_map_x)/2;
				int map_y = start_map_y + (end_map_y - start_map_y)/2;

				int width = std::min(g_settings.getInteger(Config::MAX_SPAWN_MONSTER_RADIUS), ((end_map_x - start_map_x)/2 + (end_map_y - start_map_y)/2)/2);
				int old = g_gui.GetBrushSize();
				g_gui.SetBrushSize(width);
				editor.draw(Position(map_x, map_y, floor), event.AltDown());
				g_gui.SetBrushSize(old);
			} else if(brush->isSpawnNpc()) {
				int start_map_x = std::min(last_click_map_x, mouse_map_x);
				int start_map_y = std::min(last_click_map_y, mouse_map_y);
				int end_map_x   = std::max(last_click_map_x, mouse_map_x);
				int end_map_y   = std::max(last_click_map_y, mouse_map_y);

				int map_x = start_map_x + (end_map_x - start_map_x)/2;
				int map_y = start_map_y + (end_map_y - start_map_y)/2;

				int width = std::min(g_settings.getInteger(Config::MAX_SPAWN_NPC_RADIUS), ((end_map_x - start_map_x)/2 + (end_map_y - start_map_y)/2)/2);
				int old = g_gui.GetBrushSize();
				g_gui.SetBrushSize(width);
				editor.draw(Position(map_x, map_y, floor), event.AltDown());
				g_gui.SetBrushSize(old);
			} else {
				PositionVector tilestodraw;
				PositionVector tilestoborder;
				if(brush->isWall()) {
					int start_map_x = std::min(last_click_map_x, mouse_map_x);
					int start_map_y = std::min(last_click_map_y, mouse_map_y);
					int end_map_x   = std::max(last_click_map_x, mouse_map_x);
					int end_map_y   = std::max(last_click_map_y, mouse_map_y);

					for(int y = start_map_y-1; y <= end_map_y+1; y ++) {
						for(int x = start_map_x-1; x <= end_map_x+1; x++) {
							if((x <= start_map_x+1 || x >= end_map_x-1) || (y <= start_map_y+1 || y >= end_map_y-1)) {
								tilestoborder.push_back(Position(x,y,floor));
							}
							if(((x == start_map_x || x == end_map_x) || (y == start_map_y || y == end_map_y)) &&
								((x >= start_map_x && x <= end_map_x) && (y >= start_map_y && y <= end_map_y))) {
								tilestodraw.push_back(Position(x,y,floor));
							}
						}
					}
				} else {
					if(g_gui.GetBrushShape() == BRUSHSHAPE_SQUARE) {
						if(last_click_map_x > mouse_map_x) {
							int tmp = mouse_map_x; mouse_map_x = last_click_map_x; last_click_map_x = tmp;
						}
						if(last_click_map_y > mouse_map_y) {
							int tmp = mouse_map_y; mouse_map_y = last_click_map_y; last_click_map_y = tmp;
						}

						for(int x = last_click_map_x-1; x <= mouse_map_x+1; x++) {
							for(int y = last_click_map_y-1; y <= mouse_map_y+1; y ++) {
								if((x <= last_click_map_x || x >= mouse_map_x) || (y <= last_click_map_y || y >= mouse_map_y)) {
									tilestoborder.push_back(Position(x,y,floor));
								}
								if((x >= last_click_map_x && x <= mouse_map_x) && (y >= last_click_map_y && y <= mouse_map_y)) {
									tilestodraw.push_back(Position(x,y,floor));
								}
							}
						}
					} else {
						int start_x, end_x;
						int start_y, end_y;
						int width = std::max(
							std::abs(
							std::max(mouse_map_y, last_click_map_y) -
							std::min(mouse_map_y, last_click_map_y)
							),
							std::abs(
							std::max(mouse_map_x, last_click_map_x) -
							std::min(mouse_map_x, last_click_map_x)
							)
							);
						if(mouse_map_x < last_click_map_x) {
							start_x = last_click_map_x - width;
							end_x = last_click_map_x;
						} else {
							start_x = last_click_map_x;
							end_x = last_click_map_x + width;
						}
						if(mouse_map_y < last_click_map_y) {
							start_y = last_click_map_y - width;
							end_y = last_click_map_y;
						} else {
							start_y = last_click_map_y;
							end_y = last_click_map_y + width;
						}

						int center_x = start_x + (end_x - start_x) / 2;
						int center_y = start_y + (end_y - start_y) / 2;
						float radii = width / 2.0f + 0.005f;

						for(int y = start_y-1; y <= end_y+1; y++) {
							float dy = center_y - y;
							for(int x = start_x-1; x <= end_x+1; x++) {
								float dx = center_x - x;
								//printf("%f;%f\n", dx, dy);
								float distance = sqrt(dx*dx + dy*dy);
								if(distance < radii) {
									tilestodraw.push_back(Position(x,y,floor));
								}
								if(std::abs(distance - radii) < 1.5) {
									tilestoborder.push_back(Position(x,y,floor));
								}
							}
						}
					}
				}
				if(event.ControlDown()) {
					editor.undraw(tilestodraw, tilestoborder, event.AltDown());
				} else {
					editor.draw(tilestodraw, tilestoborder, event.AltDown());
				}
			}
		}
		editor.actionQueue->resetTimer();
		drawing = false;
		dragging_draw = false;
		replace_dragging = false;
		editor.replace_brush = nullptr;
	}
	g_gui.RefreshView();
	g_gui.UpdateMinimap();
}

void MapCanvas::OnMouseCameraClick(wxMouseEvent& event)
{
	SetFocus();

	last_mmb_click_x = event.GetX();
	last_mmb_click_y = event.GetY();
	if(event.ControlDown()) {
		int screensize_x, screensize_y;
		static_cast<MapWindow*>(GetParent())->GetViewSize(&screensize_x, &screensize_y);

		static_cast<MapWindow*>(GetParent())->ScrollRelative(
			int(-screensize_x * (1.0 - zoom) * (std::max(cursor_x, 1) / double(screensize_x))),
			int(-screensize_y * (1.0 - zoom) * (std::max(cursor_y, 1) / double(screensize_y)))
			);
		zoom = 1.0;
		Refresh();
	} else {
		screendragging = true;
	}
}

void MapCanvas::OnMouseCameraRelease(wxMouseEvent& event)
{
	SetFocus();
	screendragging = false;
	if(event.ControlDown()) {
		// ...
		// Haven't moved much, it's a click!
	} else if(last_mmb_click_x > event.GetX() - 3 && last_mmb_click_x < event.GetX() + 3 &&
				last_mmb_click_y > event.GetY() - 3 && last_mmb_click_y < event.GetY() + 3) {
		int screensize_x, screensize_y;
		static_cast<MapWindow*>(GetParent())->GetViewSize(&screensize_x, &screensize_y);
		static_cast<MapWindow*>(GetParent())->ScrollRelative(
			int(zoom * (2*cursor_x - screensize_x)),
			int(zoom * (2*cursor_y - screensize_y))
			);
		Refresh();
	}
}

void MapCanvas::OnMousePropertiesClick(wxMouseEvent& event)
{
	SetFocus();

	int mouse_map_x, mouse_map_y;
	ScreenToMap(event.GetX(), event.GetY(), &mouse_map_x, &mouse_map_y);
	Tile* tile = editor.map.getTile(mouse_map_x, mouse_map_y, floor);

	if(g_gui.IsDrawingMode()) {
		g_gui.SetSelectionMode();
	}

	EndPasting();

	boundbox_selection = false;
	if(event.ShiftDown()) {
		boundbox_selection = true;

		if(!event.ControlDown()) {
			editor.selection.start(); // Start selection session
			editor.selection.clear(); // Clear out selection
			editor.selection.finish(); // End selection session
			editor.selection.updateSelectionCount();
		}
	} else if(!tile) {
		editor.selection.start(); // Start selection session
		editor.selection.clear(); // Clear out selection
		editor.selection.finish(); // End selection session
		editor.selection.updateSelectionCount();
	} else if(tile->isSelected()) {
		// Do nothing!
	} else {
		editor.selection.start(); // Start a selection session
		editor.selection.clear();
		editor.selection.commit();
		if(tile->spawnMonster && g_settings.getInteger(Config::SHOW_SPAWNS_MONSTER)) {
			editor.selection.add(tile, tile->spawnMonster);
		} else if(tile->monster && g_settings.getInteger(Config::SHOW_MONSTERS)) {
			editor.selection.add(tile, tile->monster);
		} else if(tile->npc && g_settings.getInteger(Config::SHOW_NPCS)) {
			editor.selection.add(tile, tile->npc);
		} else if(tile->spawnNpc && g_settings.getInteger(Config::SHOW_SPAWNS_NPC)) {
			editor.selection.add(tile, tile->spawnNpc);
		} else {
			Item* item = tile->getTopItem();
			if(item) {
				editor.selection.add(tile, item);
			}
		}
		editor.selection.finish(); // Finish the selection session
		editor.selection.updateSelectionCount();
	}

	last_click_x = int(event.GetX()*zoom);
	last_click_y = int(event.GetY()*zoom);

	int start_x, start_y;
	static_cast<MapWindow*>(GetParent())->GetViewStart(&start_x, &start_y);
	last_click_abs_x = last_click_x + start_x;
	last_click_abs_y = last_click_y + start_y;

	last_click_map_x = mouse_map_x;
	last_click_map_y = mouse_map_y;
	g_gui.RefreshView();
}

void MapCanvas::OnMousePropertiesRelease(wxMouseEvent& event)
{
	int mouse_map_x, mouse_map_y;
	ScreenToMap(event.GetX(), event.GetY(), &mouse_map_x, &mouse_map_y);

	if(g_gui.IsDrawingMode()) {
		g_gui.SetSelectionMode();
	}

	if(boundbox_selection) {
		if(mouse_map_x == last_click_map_x && mouse_map_y == last_click_map_y && event.ControlDown()) {
			// Mouse hasn't move, do control+shift thingy!
			Tile* tile = editor.map.getTile(mouse_map_x, mouse_map_y, floor);
			if(tile) {
				editor.selection.start(); // Start a selection session
				if(tile->isSelected()) {
					editor.selection.remove(tile);
				} else {
					editor.selection.add(tile);
				}
				editor.selection.finish(); // Finish the selection session
				editor.selection.updateSelectionCount();
			}
		} else {
			// The cursor has moved, do some boundboxing!
			if(last_click_map_x > mouse_map_x) {
				int tmp = mouse_map_x; mouse_map_x = last_click_map_x; last_click_map_x = tmp;
			}
			if(last_click_map_y > mouse_map_y) {
				int tmp = mouse_map_y; mouse_map_y = last_click_map_y; last_click_map_y = tmp;
			}

			editor.selection.start(); // Start a selection session
			switch(g_settings.getInteger(Config::SELECTION_TYPE)) {
				case SELECT_CURRENT_FLOOR: {
					for(int x = last_click_map_x; x <= mouse_map_x; x++) {
						for(int y = last_click_map_y; y <= mouse_map_y; y ++) {
							Tile* tile = editor.map.getTile(x, y, floor);
							if(!tile) continue;
							editor.selection.add(tile);
						}
					}
					break;
				}
				case SELECT_ALL_FLOORS: {
					int start_x, start_y, start_z;
					int end_x, end_y, end_z;

					start_x = last_click_map_x;
					start_y = last_click_map_y;
					start_z = MAP_MAX_LAYER;
					end_x = mouse_map_x;
					end_y = mouse_map_y;
					end_z = floor;

					if(g_settings.getInteger(Config::COMPENSATED_SELECT)) {
						start_x -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);
						start_y -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);

						end_x -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);
						end_y -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);
					}

					for(int z = start_z; z >= end_z; z--) {
						for(int x = start_x; x <= end_x; x++) {
							for(int y = start_y; y <= end_y; y++) {
								Tile* tile = editor.map.getTile(x, y, z);
								if(!tile) continue;
								editor.selection.add(tile);
								}
						}
						if(z <= GROUND_LAYER && g_settings.getInteger(Config::COMPENSATED_SELECT)) {
							start_x++; start_y++;
							end_x++; end_y++;
						}
					}
					break;
				}
				case SELECT_VISIBLE_FLOORS: {
					int start_x, start_y, start_z;
					int end_x, end_y, end_z;

					start_x = last_click_map_x;
					start_y = last_click_map_y;
					if(floor < 8) {
						start_z = GROUND_LAYER;
					} else {
						start_z = std::min(MAP_MAX_LAYER, floor + 2);
					}
					end_x = mouse_map_x;
					end_y = mouse_map_y;
					end_z = floor;

					if(g_settings.getInteger(Config::COMPENSATED_SELECT)) {
						start_x -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);
						start_y -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);

						end_x -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);
						end_y -= (floor < GROUND_LAYER ? GROUND_LAYER - floor : 0);
					}

					for(int z = start_z; z >= end_z; z--) {
						for(int x = start_x; x <= end_x; x++) {
							for(int y = start_y; y <= end_y; y++) {
								Tile* tile = editor.map.getTile(x, y, z);
								if(!tile) continue;
								editor.selection.add(tile);
							}
						}
						if(z <= GROUND_LAYER && g_settings.getInteger(Config::COMPENSATED_SELECT)) {
							start_x++; start_y++;
							end_x++; end_y++;
						}
					}
					break;
				}
			}
			editor.selection.finish(); // Finish the selection session
			editor.selection.updateSelectionCount();
		}
	} else if(event.ControlDown()) {
		// Nothing
	}

	popup_menu->Update();
	PopupMenu(popup_menu);

	editor.actionQueue->resetTimer();
	dragging = false;
	boundbox_selection = false;

	last_cursor_map_x = mouse_map_x;
	last_cursor_map_y = mouse_map_y;
	last_cursor_map_z = floor;

	g_gui.RefreshView();
}

void MapCanvas::OnWheel(wxMouseEvent& event)
{
	if(event.ControlDown()) {
		static double diff = 0.0;
		diff += event.GetWheelRotation();
		if(diff <= 1.0 || diff >= 1.0) {
			if(diff < 0.0) {
				g_gui.ChangeFloor(floor - 1);
			} else {
				g_gui.ChangeFloor(floor + 1);
			}
			diff = 0.0;
		}
		UpdatePositionStatus();
	} else if(event.AltDown()) {
		static double diff = 0.0;
		diff += event.GetWheelRotation();
		if(diff <= 1.0 || diff >= 1.0) {
			if(diff < 0.0) {
				g_gui.IncreaseBrushSize();
			} else {
				g_gui.DecreaseBrushSize();
			}
			diff = 0.0;
		}
	} else {
		double diff = -event.GetWheelRotation() * g_settings.getFloat(Config::ZOOM_SPEED) / 640.0;
		double oldzoom = zoom;
		zoom += diff;

		if(zoom < 0.125) {
			diff = 0.125 - oldzoom;
			zoom = 0.125;
		}
		if(zoom > 25.00) {
			diff = 25.00 - oldzoom;
			zoom = 25.0;
		}

		UpdateZoomStatus();

		int screensize_x, screensize_y;
		static_cast<MapWindow*>(GetParent())->GetViewSize(&screensize_x, &screensize_y);

		// This took a day to figure out!
		int scroll_x = int(screensize_x * diff * (std::max(cursor_x, 1) / double(screensize_x))) * GetContentScaleFactor();
		int scroll_y = int(screensize_y * diff * (std::max(cursor_y, 1) / double(screensize_y))) * GetContentScaleFactor();

		static_cast<MapWindow*>(GetParent())->ScrollRelative(-scroll_x, -scroll_y);
	}

	Refresh();
}

void MapCanvas::OnLoseMouse(wxMouseEvent& event)
{
	Refresh();
}

void MapCanvas::OnGainMouse(wxMouseEvent& event)
{
	if(!event.LeftIsDown()) {
		dragging = false;
		boundbox_selection = false;
		drawing = false;
	}
	if(!event.MiddleIsDown()) {
		screendragging = false;
	}

	Refresh();
}

void MapCanvas::OnKeyDown(wxKeyEvent& event)
{
	//char keycode = event.GetKeyCode();
	// std::cout << "Keycode " << keycode << std::endl;
	switch(event.GetKeyCode()) {
		case WXK_NUMPAD_ADD:
		case WXK_PAGEUP: {
			g_gui.ChangeFloor(floor - 1);
			break;
		}
		case WXK_NUMPAD_SUBTRACT:
		case WXK_PAGEDOWN: {
			g_gui.ChangeFloor(floor + 1);
			break;
		}
		case WXK_NUMPAD_MULTIPLY: {
			double diff = -0.3;

			double oldzoom = zoom;
			zoom += diff;

			if(zoom < 0.125) {
				diff = 0.125 - oldzoom; zoom = 0.125;
			}

			int screensize_x, screensize_y;
			static_cast<MapWindow*>(GetParent())->GetViewSize(&screensize_x, &screensize_y);

			// This took a day to figure out!
			int scroll_x = int(screensize_x * diff * (std::max(cursor_x, 1) / double(screensize_x)));
			int scroll_y = int(screensize_y * diff * (std::max(cursor_y, 1) / double(screensize_y)));

			static_cast<MapWindow*>(GetParent())->ScrollRelative(-scroll_x, -scroll_y);

			UpdatePositionStatus();
			UpdateZoomStatus();
			Refresh();
			break;
		}
		case WXK_NUMPAD_DIVIDE: {
			double diff = 0.3;
			double oldzoom = zoom;
			zoom += diff;

			if(zoom > 25.00) {
				diff = 25.00 - oldzoom; zoom = 25.0;
			}

			int screensize_x, screensize_y;
			static_cast<MapWindow*>(GetParent())->GetViewSize(&screensize_x, &screensize_y);

			// This took a day to figure out!
			int scroll_x = int(screensize_x * diff * (std::max(cursor_x, 1) / double(screensize_x)));
			int scroll_y = int(screensize_y * diff * (std::max(cursor_y, 1) / double(screensize_y)));

			static_cast<MapWindow*>(GetParent())->ScrollRelative(-scroll_x, -scroll_y);

			UpdatePositionStatus();
			UpdateZoomStatus();
			Refresh();
			break;
		}
		// This will work like crap with non-us layouts, well, sucks for them until there is another solution.
		case '[':
		case '+': {
			g_gui.IncreaseBrushSize();
			Refresh();
			break;
		}
		case ']':
		case '-': {
			g_gui.DecreaseBrushSize();
			Refresh();
			break;
		}
		case WXK_NUMPAD_UP:
		case WXK_UP: {
			int start_x, start_y;
			static_cast<MapWindow*>(GetParent())->GetViewStart(&start_x, &start_y);

			int tiles = 3;
			if(event.ControlDown())
				tiles = 10;
			else if(zoom == 1.0)
				tiles = 1;

			static_cast<MapWindow*>(GetParent())->Scroll(start_x, int(start_y - TILE_SIZE * tiles * zoom));
			UpdatePositionStatus();
			Refresh();
			break;
		}
		case WXK_NUMPAD_DOWN:
		case WXK_DOWN: {
			int start_x, start_y;
			static_cast<MapWindow*>(GetParent())->GetViewStart(&start_x, &start_y);

			int tiles = 3;
			if(event.ControlDown())
				tiles = 10;
			else if(zoom == 1.0)
				tiles = 1;

			static_cast<MapWindow*>(GetParent())->Scroll(start_x, int(start_y + TILE_SIZE * tiles * zoom));
			UpdatePositionStatus();
			Refresh();
			break;
		}
		case WXK_NUMPAD_LEFT:
		case WXK_LEFT: {
			int start_x, start_y;
			static_cast<MapWindow*>(GetParent())->GetViewStart(&start_x, &start_y);

			int tiles = 3;
			if(event.ControlDown())
				tiles = 10;
			else if(zoom == 1.0)
				tiles = 1;

			static_cast<MapWindow*>(GetParent())->Scroll(int(start_x - TILE_SIZE * tiles * zoom), start_y);
			UpdatePositionStatus();
			Refresh();
			break;
		}
		case WXK_NUMPAD_RIGHT:
		case WXK_RIGHT: {
			int start_x, start_y;
			static_cast<MapWindow*>(GetParent())->GetViewStart(&start_x, &start_y);

			int tiles = 3;
			if(event.ControlDown())
				tiles = 10;
			else if(zoom == 1.0)
				tiles = 1;

			static_cast<MapWindow*>(GetParent())->Scroll(int(start_x + TILE_SIZE * tiles * zoom), start_y);
			UpdatePositionStatus();
			Refresh();
			break;
		}
		case WXK_SPACE: { // Utility keys
			if(event.ControlDown()) {
				g_gui.FillDoodadPreviewBuffer();
				g_gui.RefreshView();
			} else {
				g_gui.SwitchMode();
			}
			break;
		}
		case WXK_TAB: { // Tab switch
			if(event.ShiftDown()) {
				g_gui.CycleTab(false);
			} else {
				g_gui.CycleTab(true);
			}
			break;
		}
		case WXK_DELETE: { // Delete
			editor.destroySelection();
			g_gui.RefreshView();
			break;
		}
		case 'z':
		case 'Z': { // Rotate counterclockwise (actually shift variaton, but whatever... :P)
			int nv = g_gui.GetBrushVariation();
			--nv;
			if(nv < 0) {
				nv = std::max(0, (g_gui.GetCurrentBrush()? g_gui.GetCurrentBrush()->getMaxVariation() - 1 : 0));
			}
			g_gui.SetBrushVariation(nv);
			g_gui.RefreshView();
			break;
		}
		case 'x':
		case 'X': { // Rotate clockwise (actually shift variaton, but whatever... :P)
			int nv = g_gui.GetBrushVariation();
			++nv;
			if(nv >= (g_gui.GetCurrentBrush()? g_gui.GetCurrentBrush()->getMaxVariation() : 0)) {
				nv = 0;
			}
			g_gui.SetBrushVariation(nv);
			g_gui.RefreshView();
			break;
		}
		case 'q':
		case 'Q': { // Select previous brush
			g_gui.SelectPreviousBrush();
			break;
		}
		// Hotkeys
		case '0': case '1': case '2': case '3': case '4':
		case '5': case '6': case '7': case '8': case '9': {
			int index = event.GetKeyCode() - '0';
			if(event.ControlDown()) {
				Hotkey hk;
				if(g_gui.IsSelectionMode()) {
					int view_start_x, view_start_y;
					static_cast<MapWindow*>(GetParent())->GetViewStart(&view_start_x, &view_start_y);
					int view_start_map_x = view_start_x / TILE_SIZE, view_start_map_y = view_start_y / TILE_SIZE;

					int view_screensize_x, view_screensize_y;
					static_cast<MapWindow*>(GetParent())->GetViewSize(&view_screensize_x, &view_screensize_y);

					int map_x = int(view_start_map_x + (view_screensize_x * zoom) / TILE_SIZE / 2);
					int map_y = int(view_start_map_y + (view_screensize_y * zoom) / TILE_SIZE / 2);

					hk = Hotkey(Position(map_x, map_y, floor));
				} else if(g_gui.GetCurrentBrush()) {
					// Drawing mode
					hk = Hotkey(g_gui.GetCurrentBrush());
				} else {
					break;
				}
				g_gui.SetHotkey(index, hk);
			} else {
				// Click hotkey
				Hotkey hk = g_gui.GetHotkey(index);
				if(hk.IsPosition()) {
					g_gui.SetSelectionMode();

					int map_x = hk.GetPosition().x;
					int map_y = hk.GetPosition().y;
					int map_z = hk.GetPosition().z;

					static_cast<MapWindow*>(GetParent())->Scroll(TILE_SIZE * map_x, TILE_SIZE * map_y, true);
					floor = map_z;

					g_gui.SetStatusText("Used hotkey " + i2ws(index));
					g_gui.RefreshView();
				} else if(hk.IsBrush()) {
					g_gui.SetDrawingMode();

					std::string name = hk.GetBrushname();
					Brush* brush = g_brushes.getBrush(name);
					if(brush == nullptr) {
						g_gui.SetStatusText("Brush \"" + wxstr(name) + "\" not found");
						return;
					}

					if(!g_gui.SelectBrush(brush)) {
						g_gui.SetStatusText("Brush \"" + wxstr(name) + "\" is not in any palette");
						return;
					}

					g_gui.SetStatusText("Used hotkey " + i2ws(index));
					g_gui.RefreshView();
				} else {
					g_gui.SetStatusText("Unassigned hotkey " + i2ws(index));
				}
			}
			break;
		}
		case 'd':
		case 'D': {
			keyCode = WXK_CONTROL_D;
			break;
		}
		default:{
			event.Skip();
			break;
		}
	}
}

void MapCanvas::OnKeyUp(wxKeyEvent& event)
{
	keyCode = WXK_NONE;
}

void MapCanvas::OnCopy(wxCommandEvent& WXUNUSED(event))
{
	if(g_gui.IsSelectionMode())
	   editor.copybuffer.copy(editor, GetFloor());
}

void MapCanvas::OnCut(wxCommandEvent& WXUNUSED(event))
{
	if(g_gui.IsSelectionMode())
		editor.copybuffer.cut(editor, GetFloor());
	g_gui.RefreshView();
}

void MapCanvas::OnPaste(wxCommandEvent& WXUNUSED(event))
{
	g_gui.DoPaste();
	g_gui.RefreshView();
}

void MapCanvas::OnDelete(wxCommandEvent& WXUNUSED(event))
{
	editor.destroySelection();
	g_gui.RefreshView();
}

void MapCanvas::OnCopyPosition(wxCommandEvent& WXUNUSED(event))
{
	if(editor.selection.size() == 0)
		return;

	Position minPos = editor.selection.minPosition();
	Position maxPos = editor.selection.maxPosition();

	std::ostringstream clip;
	if(minPos != maxPos) {
		clip << "{";
		clip << "fromx = " << minPos.x << ", ";
		clip << "tox = " << maxPos.x << ", ";
		clip << "fromy = " << minPos.y << ", ";
		clip << "toy = " << maxPos.y << ", ";
		if(minPos.z != maxPos.z) {
			clip << "fromz = " << minPos.z << ", ";
			clip << "toz = " << maxPos.z;
		}
		else
			clip << "z = " << minPos.z;
		clip << "}";
	} else {
		switch (g_settings.getInteger(Config::COPY_POSITION_FORMAT)) {
			case 0:
				clip << "{x = " << minPos.x << ", y = " << minPos.y << ", z = " << minPos.z << "}";
				break;
			case 1:
				clip << "{\"x\":" << minPos.x << ",\"y\":" << minPos.y << ",\"z\":" << minPos.z << "}";
				break;
			case 2:
				clip << minPos.x << ", " << minPos.y << ", " << minPos.z;
				break;
			case 3:
				clip << "(" << minPos.x << ", " << minPos.y << ", " << minPos.z << ")";
				break;
			case 4:
				clip << "Position(" << minPos.x << ", " << minPos.y << ", " << minPos.z << ")";
				break;
		}
	}

	if(wxTheClipboard->Open()) {
		wxTextDataObject* obj = new wxTextDataObject();
		obj->SetText(wxstr(clip.str()));
		wxTheClipboard->SetData(obj);

		wxTheClipboard->Close();
	}
}

void MapCanvas::OnCopyItemId(wxCommandEvent& WXUNUSED(event))
{
	ASSERT(editor.selection.size() == 1);

	if(wxTheClipboard->Open()) {
		Tile* tile = editor.selection.getSelectedTile();
		ItemVector selected_items = tile->getSelectedItems();
		ASSERT(selected_items.size() == 1);

		const Item* item = selected_items.front();

		wxTextDataObject* obj = new wxTextDataObject();
		obj->SetText(i2ws(item->getID()));
		wxTheClipboard->SetData(obj);

		wxTheClipboard->Close();
	}
}

void MapCanvas::OnCopyName(wxCommandEvent& WXUNUSED(event))
{
	ASSERT(editor.selection.size() == 1);

	if(wxTheClipboard->Open()) {
		Tile* tile = editor.selection.getSelectedTile();
		ItemVector selected_items = tile->getSelectedItems();
		ASSERT(selected_items.size() == 1);

		const Item* item = selected_items.front();

		wxTextDataObject* obj = new wxTextDataObject();
		obj->SetText(wxstr(item->getName()));
		wxTheClipboard->SetData(obj);

		wxTheClipboard->Close();
	}
}

void MapCanvas::OnBrowseTile(wxCommandEvent& WXUNUSED(event))
{
	if(editor.selection.size() != 1)
		return;

	Tile* tile = editor.selection.getSelectedTile();
	if(!tile) return;
	ASSERT(tile->isSelected());
	Tile* new_tile = tile->deepCopy(editor.map);

	wxDialog* w = new BrowseTileWindow(g_gui.root, new_tile, wxPoint(cursor_x, cursor_y));

	int ret = w->ShowModal();
	if(ret != 0) {
		Action* action = editor.actionQueue->createAction(ACTION_DELETE_TILES);
		action->addChange(newd Change(new_tile));
		editor.addAction(action);
	} else {
		// Cancel
		delete new_tile;
	}

	w->Destroy();
}

void MapCanvas::OnRotateItem(wxCommandEvent& WXUNUSED(event))
{
	Tile* tile = editor.selection.getSelectedTile();

	Action* action = editor.actionQueue->createAction(ACTION_ROTATE_ITEM);

	Tile* new_tile = tile->deepCopy(editor.map);

	ItemVector selected_items = new_tile->getSelectedItems();
	ASSERT(selected_items.size() > 0);

	selected_items.front()->doRotate();

	action->addChange(newd Change(new_tile));

 	editor.actionQueue->addAction(action);
	g_gui.RefreshView();
}

void MapCanvas::OnGotoDestination(wxCommandEvent& WXUNUSED(event))
{
	Tile* tile = editor.selection.getSelectedTile();
	ItemVector selected_items = tile->getSelectedItems();
	ASSERT(selected_items.size() > 0);
	Teleport* teleport = dynamic_cast<Teleport*>(selected_items.front());
	if(teleport) {
		Position pos = teleport->getDestination();
		g_gui.SetScreenCenterPosition(pos);
	}
}

void MapCanvas::OnSwitchDoor(wxCommandEvent& WXUNUSED(event))
{
	Tile* tile = editor.selection.getSelectedTile();

	Action* action = editor.actionQueue->createAction(ACTION_SWITCHDOOR);

	Tile* new_tile = tile->deepCopy(editor.map);

	ItemVector selected_items = new_tile->getSelectedItems();
	ASSERT(selected_items.size() > 0);

	DoorBrush::switchDoor(selected_items.front());

	action->addChange(newd Change(new_tile));

	editor.actionQueue->addAction(action);
	g_gui.RefreshView();
}

void MapCanvas::OnSelectRAWBrush(wxCommandEvent& WXUNUSED(event))
{
	if(editor.selection.size() != 1) return;
	Tile* tile = editor.selection.getSelectedTile();
	if(!tile) return;
	Item* item = tile->getTopSelectedItem();

	if(item && item->getRAWBrush())
		g_gui.SelectBrush(item->getRAWBrush(), TILESET_RAW);
}

void MapCanvas::OnSelectGroundBrush(wxCommandEvent& WXUNUSED(event))
{
	if(editor.selection.size() != 1) return;
	Tile* tile = editor.selection.getSelectedTile();
	if(!tile) return;
	GroundBrush* bb = tile->getGroundBrush();

	if(bb)
		g_gui.SelectBrush(bb, TILESET_TERRAIN);
}

void MapCanvas::OnSelectDoodadBrush(wxCommandEvent& WXUNUSED(event))
{
	if(editor.selection.size() != 1) return;
	Tile* tile = editor.selection.getSelectedTile();
	if(!tile) return;
	Item* item = tile->getTopSelectedItem();

	if(item)
		g_gui.SelectBrush(item->getDoodadBrush(), TILESET_DOODAD);
}

void MapCanvas::OnSelectDoorBrush(wxCommandEvent& WXUNUSED(event))
{
	if(editor.selection.size() != 1) return;
	Tile* tile = editor.selection.getSelectedTile();
	if(!tile) return;
	Item* item = tile->getTopSelectedItem();

	if(item)
		g_gui.SelectBrush(item->getDoorBrush(), TILESET_TERRAIN);
}

void MapCanvas::OnSelectWallBrush(wxCommandEvent& WXUNUSED(event))
{
	if(editor.selection.size() != 1) return;
	Tile* tile = editor.selection.getSelectedTile();
	if(!tile) return;
	Item* wall = tile->getWall();
	WallBrush* wb = wall->getWallBrush();

	if(wb)
		g_gui.SelectBrush(wb, TILESET_TERRAIN);
}

void MapCanvas::OnSelectCarpetBrush(wxCommandEvent& WXUNUSED(event))
{
	if(editor.selection.size() != 1) return;
	Tile* tile = editor.selection.getSelectedTile();
	if(!tile) return;
	Item* wall = tile->getCarpet();
	CarpetBrush* cb = wall->getCarpetBrush();

	if(cb)
		g_gui.SelectBrush(cb);
}

void MapCanvas::OnSelectTableBrush(wxCommandEvent& WXUNUSED(event))
{
	if(editor.selection.size() != 1) return;
	Tile* tile = editor.selection.getSelectedTile();
	if(!tile) return;
	Item* wall = tile->getTable();
	TableBrush* tb = wall->getTableBrush();

	if(tb)
		g_gui.SelectBrush(tb);
}

void MapCanvas::OnSelectHouseBrush(wxCommandEvent& WXUNUSED(event))
{
	Tile* tile = editor.selection.getSelectedTile();
	if(!tile)
		return;

	if(tile->isHouseTile()) {
		House* house = editor.map.houses.getHouse(tile->getHouseID());
		if(house) {
			g_gui.house_brush->setHouse(house);
			g_gui.SelectBrush(g_gui.house_brush, TILESET_HOUSE);
		}
	}
}

void MapCanvas::OnSelectMonsterBrush(wxCommandEvent& WXUNUSED(event))
{
	Tile* tile = editor.selection.getSelectedTile();
	if(!tile)
		return;

	if(tile->monster)
		g_gui.SelectBrush(tile->monster->getBrush(), TILESET_MONSTER);
}

void MapCanvas::OnSelectSpawnBrush(wxCommandEvent& WXUNUSED(event))
{
	g_gui.SelectBrush(g_gui.spawn_brush, TILESET_MONSTER);
}

void MapCanvas::OnSelectNpcBrush(wxCommandEvent& WXUNUSED(event))
{
	Tile* tile = editor.selection.getSelectedTile();
	if(!tile)
		return;

	if(tile->npc)
		g_gui.SelectBrush(tile->npc->getBrush(), TILESET_NPC);
}

void MapCanvas::OnSelectSpawnNpcBrush(wxCommandEvent& WXUNUSED(event))
{
	g_gui.SelectBrush(g_gui.spawn_npc_brush, TILESET_NPC);
}

void MapCanvas::OnProperties(wxCommandEvent& WXUNUSED(event))
{
	if(editor.selection.size() != 1)
		return;

	Tile* tile = editor.selection.getSelectedTile();
	if(!tile) return;
	ASSERT(tile->isSelected());
	Tile* new_tile = tile->deepCopy(editor.map);

	wxDialog* w = nullptr;

	if(new_tile->spawnMonster && g_settings.getInteger(Config::SHOW_SPAWNS_MONSTER))
		w = newd OldPropertiesWindow(g_gui.root, &editor.map, new_tile, new_tile->spawnMonster);
	else if(new_tile->monster && g_settings.getInteger(Config::SHOW_MONSTERS))
		w = newd OldPropertiesWindow(g_gui.root, &editor.map, new_tile, new_tile->monster);
	else if(new_tile->npc && g_settings.getInteger(Config::SHOW_NPCS))
		w = newd OldPropertiesWindow(g_gui.root, &editor.map, new_tile, new_tile->npc);
	else if(new_tile->spawnNpc && g_settings.getInteger(Config::SHOW_SPAWNS_NPC))
		w = newd OldPropertiesWindow(g_gui.root, &editor.map, new_tile, new_tile->spawnNpc);
	else {
		ItemVector selected_items = new_tile->getSelectedItems();

		Item* item = nullptr;
		int count = 0;
		for(ItemVector::iterator it = selected_items.begin(); it != selected_items.end(); ++it) {
			++count;
			if((*it)->isSelected()) {
				item = *it;
			}
		}

		if(item) {
			if(editor.map.getVersion().otbm >= MAP_OTBM_4)
				w = newd PropertiesWindow(g_gui.root, &editor.map, new_tile, item);
			else
				w = newd OldPropertiesWindow(g_gui.root, &editor.map, new_tile, item);
		}
		else
			return;
	}

	int ret = w->ShowModal();
	if(ret != 0) {
		Action* action = editor.actionQueue->createAction(ACTION_CHANGE_PROPERTIES);
		action->addChange(newd Change(new_tile));
		editor.addAction(action);
	} else {
		// Cancel!
		delete new_tile;
	}
	w->Destroy();
}

void MapCanvas::ChangeFloor(int new_floor)
{
	ASSERT(new_floor >= 0 || new_floor <= MAP_MAX_LAYER);
	int old_floor = floor;
	floor = new_floor;
	if(old_floor != new_floor) {
		UpdatePositionStatus();
		g_gui.root->UpdateFloorMenu();
		g_gui.UpdateMinimap(true);
	}
	Refresh();
}

void MapCanvas::EnterDrawingMode()
{
	dragging = false;
	boundbox_selection = false;
	EndPasting();
	Refresh();
}

void MapCanvas::EnterSelectionMode()
{
	drawing = false;
	dragging_draw = false;
	replace_dragging = false;
	editor.replace_brush = nullptr;
	Refresh();
}

bool MapCanvas::isPasting() const
{
	return g_gui.IsPasting();
}

void MapCanvas::StartPasting()
{
	g_gui.StartPasting();
}

void MapCanvas::EndPasting()
{
	g_gui.EndPasting();
}

void MapCanvas::Reset()
{
	cursor_x = 0;
	cursor_y = 0;

	zoom = 1.0;
	floor = GROUND_LAYER;

	dragging = false;
	boundbox_selection = false;
	screendragging = false;
	drawing = false;
	dragging_draw = false;

	replace_dragging = false;
	editor.replace_brush = nullptr;

	drag_start_x = -1;
	drag_start_y = -1;
	drag_start_z = -1;

	last_click_map_x = -1;
	last_click_map_y = -1;
	last_click_map_z = -1;

	last_mmb_click_x = -1;
	last_mmb_click_y = -1;

	editor.selection.clear();
	editor.actionQueue->clear();
}

MapPopupMenu::MapPopupMenu(Editor& editor) : wxMenu(""), editor(editor)
{
	////
}

MapPopupMenu::~MapPopupMenu()
{
	////
}

void MapPopupMenu::Update()
{
	// Clear the menu of all items
	while(GetMenuItemCount() != 0) {
		wxMenuItem* m_item = FindItemByPosition(0);
		// If you add a submenu, this won't delete it.
		Delete(m_item);
	}

	bool anything_selected = editor.selection.size() != 0;

	wxMenuItem* cutItem = Append( MAP_POPUP_MENU_CUT, "&Cut\tCTRL+X", "Cut out all selected items");
	cutItem->Enable(anything_selected);

	wxMenuItem* copyItem = Append( MAP_POPUP_MENU_COPY, "&Copy\tCTRL+C", "Copy all selected items");
	copyItem->Enable(anything_selected);

	wxMenuItem* copyPositionItem = Append( MAP_POPUP_MENU_COPY_POSITION, "&Copy Position", "Copy the position as a lua table");
	copyPositionItem->Enable(anything_selected);

	wxMenuItem* pasteItem = Append( MAP_POPUP_MENU_PASTE, "&Paste\tCTRL+V", "Paste items in the copybuffer here");
	pasteItem->Enable(editor.copybuffer.canPaste());

	wxMenuItem* deleteItem = Append( MAP_POPUP_MENU_DELETE, "&Delete\tDEL", "Removes all seleceted items");
	deleteItem->Enable(anything_selected);

	if(anything_selected) {
		if(editor.selection.size() == 1) {
			Tile* tile = editor.selection.getSelectedTile();
			ItemVector selected_items = tile->getSelectedItems();

			bool hasWall = false;
			bool hasCarpet = false;
			bool hasTable = false;
			Item* topItem = nullptr;
			Item* topSelectedItem = (selected_items.size() == 1 ? selected_items.back() : nullptr);
			Monster* topMonster = tile->monster;
			SpawnMonster* topSpawnMonster = tile->spawnMonster;
			Npc* topNpc = tile->npc;
			SpawnNpc* topSpawnNpc = tile->spawnNpc;

			for (auto *item : tile->items) {
				if(item->isWall()) {
					Brush* wb = item->getWallBrush();
					if(wb && wb->visibleInPalette()) hasWall = true;
				}
				if(item->isTable()) {
					Brush* tb = item->getTableBrush();
					if(tb && tb->visibleInPalette()) hasTable = true;
				}
				if(item->isCarpet()) {
					Brush* cb = item->getCarpetBrush();
					if(cb && cb->visibleInPalette()) hasCarpet = true;
				}
				if(item->isSelected()) {
					topItem = item;
				}
			}
			if(!topItem) {
				topItem = tile->ground;
			}

			AppendSeparator();

			if(topSelectedItem) {
				Append(MAP_POPUP_MENU_COPY_ITEM_ID, "Copy Item Id", "Copy the id of this item");
				Append(MAP_POPUP_MENU_COPY_NAME, "Copy Item Name", "Copy the name of this item");
				AppendSeparator();
			}

			if(topSelectedItem || topMonster || topNpc || topItem) {
				Teleport* teleport = dynamic_cast<Teleport*>(topSelectedItem);
				if(topSelectedItem && (topSelectedItem->isBrushDoor() || topSelectedItem->isRoteable() || teleport)) {

					if(topSelectedItem->isRoteable())
						Append(MAP_POPUP_MENU_ROTATE, "&Rotate item", "Rotate this item");

					if(teleport && teleport->hasDestination())
						Append(MAP_POPUP_MENU_GOTO, "&Go To Destination", "Go to the destination of this teleport");

					if(topSelectedItem->isDoor())
					{
						if (topSelectedItem->isOpen()) {
							Append(MAP_POPUP_MENU_SWITCH_DOOR, "&Close door", "Close this door");
						} else {
							Append(MAP_POPUP_MENU_SWITCH_DOOR, "&Open door", "Open this door");
						}
						AppendSeparator();
					}
				}

				if(topMonster)
					Append( MAP_POPUP_MENU_SELECT_MONSTER_BRUSH, "Select Monster", "Uses the current monster as a monster brush");

				if(topSpawnMonster)
					Append( MAP_POPUP_MENU_SELECT_SPAWN_BRUSH, "Select Npc", "Select the npc brush");

				if(topNpc)
					Append( MAP_POPUP_MENU_SELECT_NPC_BRUSH, "Select Npc", "Uses the current npc as a npc brush");

				if(topSpawnNpc)
					Append( MAP_POPUP_MENU_SELECT_SPAWN_NPC_BRUSH, "Select Npc", "Select the npc brush");

				Append( MAP_POPUP_MENU_SELECT_RAW_BRUSH, "Select RAW", "Uses the top item as a RAW brush");

				if(hasWall)
					Append( MAP_POPUP_MENU_SELECT_WALL_BRUSH, "Select Wallbrush", "Uses the current item as a wallbrush");

				if(hasCarpet)
					Append( MAP_POPUP_MENU_SELECT_CARPET_BRUSH, "Select Carpetbrush", "Uses the current item as a carpetbrush");

				if(hasTable)
					Append( MAP_POPUP_MENU_SELECT_TABLE_BRUSH, "Select Tablebrush", "Uses the current item as a tablebrush");

				if(topSelectedItem && topSelectedItem->getDoodadBrush() && topSelectedItem->getDoodadBrush()->visibleInPalette())
					Append( MAP_POPUP_MENU_SELECT_DOODAD_BRUSH, "Select Doodadbrush", "Use this doodad brush");

				if(topSelectedItem && topSelectedItem->isBrushDoor() && topSelectedItem->getDoorBrush())
					Append( MAP_POPUP_MENU_SELECT_DOOR_BRUSH, "Select Doorbrush", "Use this door brush");

				if(tile->hasGround() && tile->getGroundBrush() && tile->getGroundBrush()->visibleInPalette())
					Append( MAP_POPUP_MENU_SELECT_GROUND_BRUSH, "Select Groundbrush", "Uses the current item as a groundbrush");

				if(tile->isHouseTile())
					Append(MAP_POPUP_MENU_SELECT_HOUSE_BRUSH, "Select House", "Draw with the house on this tile.");

				AppendSeparator();
				Append( MAP_POPUP_MENU_PROPERTIES, "&Properties", "Properties for the current object");
			} else {

				if(topMonster)
					Append( MAP_POPUP_MENU_SELECT_MONSTER_BRUSH, "Select Monster", "Uses the current monster as a monster brush");

				if(topSpawnMonster)
					Append( MAP_POPUP_MENU_SELECT_SPAWN_BRUSH, "Select Monster", "Select the monster brush");

				if(topNpc)
					Append( MAP_POPUP_MENU_SELECT_NPC_BRUSH, "Select Npc", "Uses the current npc as a npc brush");

				if(topSpawnNpc)
					Append( MAP_POPUP_MENU_SELECT_SPAWN_NPC_BRUSH, "Select Npc", "Select the npc brush");

				Append( MAP_POPUP_MENU_SELECT_RAW_BRUSH, "Select RAW", "Uses the top item as a RAW brush");
				if(hasWall) {
					Append( MAP_POPUP_MENU_SELECT_WALL_BRUSH, "Select Wallbrush", "Uses the current item as a wallbrush");
				}
				if(tile->hasGround() && tile->getGroundBrush() && tile->getGroundBrush()->visibleInPalette()) {
					Append( MAP_POPUP_MENU_SELECT_GROUND_BRUSH, "Select Groundbrush", "Uses the current tile as a groundbrush");
				}

				if(tile->isHouseTile()) {
					Append(MAP_POPUP_MENU_SELECT_HOUSE_BRUSH, "Select House", "Draw with the house on this tile.");
				}

				if(tile->hasGround() || topMonster || topSpawnMonster || topNpc|| topSpawnNpc) {
					AppendSeparator();
					Append( MAP_POPUP_MENU_PROPERTIES, "&Properties", "Properties for the current object");
				}
			}

			AppendSeparator();

			wxMenuItem* browseTile = Append(MAP_POPUP_MENU_BROWSE_TILE, "Browse Field", "Navigate from tile items");
			browseTile->Enable(anything_selected);
		}
	}
}

void MapCanvas::getTilesToDraw(int mouse_map_x, int mouse_map_y, int floor, PositionVector* tilestodraw, PositionVector* tilestoborder, bool fill /*= false*/)
{
	if(fill) {
		Brush* brush = g_gui.GetCurrentBrush();
		if(!brush || !brush->isGround()) {
			return;
		}

		GroundBrush* newBrush = brush->asGround();
		Position position(mouse_map_x, mouse_map_y, floor);

		Tile* tile = editor.map.getTile(position);
		GroundBrush* oldBrush = nullptr;
		if(tile) {
			oldBrush = tile->getGroundBrush();
		}

		if(oldBrush && oldBrush->getID() == newBrush->getID()) {
			return;
		}

		if((tile && tile->ground && !oldBrush) || (!tile && oldBrush)) {
			return;
		}

		if(tile && oldBrush) {
			GroundBrush* groundBrush = tile->getGroundBrush();
			if(!groundBrush || groundBrush->getID() != oldBrush->getID()) {
				return;
			}
		}

		std::fill(std::begin(processed), std::end(processed), false);
		floodFill(&editor.map, position, BLOCK_SIZE/2, BLOCK_SIZE/2, oldBrush, tilestodraw);

	} else {
		for(int y = -g_gui.GetBrushSize() - 1; y <= g_gui.GetBrushSize() + 1; y++) {
			for(int x = -g_gui.GetBrushSize() - 1; x <= g_gui.GetBrushSize() + 1; x++) {
				if(g_gui.GetBrushShape() == BRUSHSHAPE_SQUARE) {
					if(x >= -g_gui.GetBrushSize() && x <= g_gui.GetBrushSize() && y >= -g_gui.GetBrushSize() && y <= g_gui.GetBrushSize()) {
						if(tilestodraw)
							tilestodraw->push_back(Position(mouse_map_x + x, mouse_map_y + y, floor));
					}
					if(std::abs(x) - g_gui.GetBrushSize() < 2 && std::abs(y) - g_gui.GetBrushSize() < 2) {
						if(tilestoborder)
							tilestoborder->push_back(Position(mouse_map_x + x, mouse_map_y + y, floor));
					}
				} else if(g_gui.GetBrushShape() == BRUSHSHAPE_CIRCLE) {
					double distance = sqrt(double(x*x) + double(y*y));
					if(distance < g_gui.GetBrushSize() + 0.005) {
						if(tilestodraw)
							tilestodraw->push_back(Position(mouse_map_x + x, mouse_map_y + y, floor));
					}
					if(std::abs(distance - g_gui.GetBrushSize()) < 1.5) {
						if(tilestoborder)
							tilestoborder->push_back(Position(mouse_map_x + x, mouse_map_y + y, floor));
					}
				}
			}
		}
	}
}

bool MapCanvas::floodFill(Map *map, const Position& center, int x, int y, GroundBrush* brush, PositionVector* positions)
{
	countMaxFills++;
	if (countMaxFills > (BLOCK_SIZE * 4 * 4)) {
		countMaxFills = 0;
		return true;
	}

	if(x <= 0 || y <= 0 || x >= BLOCK_SIZE || y >= BLOCK_SIZE) {
		return false;
	}

	processed[getFillIndex(x, y)] = true;

	int px = (center.x + x) - (BLOCK_SIZE/2);
	int py = (center.y + y) - (BLOCK_SIZE/2);
	if(px <= 0 || py <= 0 || px >= map->getWidth() || py >= map->getHeight()) {
		return false;
	}

	Tile* tile = map->getTile(px, py, center.z);
	if((tile && tile->ground && !brush) || (!tile && brush)) {
		return false;
	}

	if(tile && brush) {
		GroundBrush* groundBrush = tile->getGroundBrush();
		if(!groundBrush || groundBrush->getID() != brush->getID()) {
			return false;
		}
	}

	positions->push_back(Position(px, py, center.z));

	bool deny = false;
	if(!processed[getFillIndex(x-1, y)]) {
		deny = floodFill(map, center, x-1, y, brush, positions);
	}

	if(!deny && !processed[getFillIndex(x, y-1)]) {
		deny = floodFill(map, center, x, y-1, brush, positions);
	}

	if(!deny && !processed[getFillIndex(x+1, y)]) {
		deny = floodFill(map, center, x+1, y, brush, positions);
	}

	if(!deny && !processed[getFillIndex(x, y+1)]) {
		deny = floodFill(map, center, x, y+1, brush, positions);
	}

	return deny;
}

// ============================================================================
// AnimationTimer

AnimationTimer::AnimationTimer(MapCanvas *canvas) : wxTimer(),
	map_canvas(canvas),
	started(false)
{
	////
};

AnimationTimer::~AnimationTimer()
{
	////
};

void AnimationTimer::Notify()
{
	if(map_canvas->GetZoom() <= 2.0)
		map_canvas->Refresh();
}

void AnimationTimer::Start()
{
	if(!started) {
		started = true;
		wxTimer::Start(100);
	}
};

void AnimationTimer::Stop()
{
	if(started) {
		started = false;
		wxTimer::Stop();
	}
};
