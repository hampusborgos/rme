
#include "main.h"

#ifdef __LINUX__
	#include <GL/glut.h>
#else
	#include <glut.h>
#endif

#include "editor.h"
#include "gui.h"
#include "sprites.h"
#include "map_drawer.h"
#include "map_display.h"
#include "copybuffer.h"
#include "live_socket.h"

#include "doodad_brush.h"
#include "creature_brush.h"
#include "house_exit_brush.h"
#include "house_brush.h"
#include "spawn_brush.h"
#include "wall_brush.h"
#include "carpet_brush.h"
#include "raw_brush.h"
#include "table_brush.h"
#include "waypoint_brush.h"

DrawingOptions::DrawingOptions()
{
	SetDefault();
}

void DrawingOptions::SetDefault()
{
	transparent_floors = false;
	transparent_items = false;
	show_ingame_box = false;
	ingame = false;
	dragging = false;

	show_grid = 0;
	show_all_floors = true;
	show_creatures = true;
	show_spawns = true;
	show_houses = true;
	show_shade = true;
	show_special_tiles = true;
	show_items = true;

	highlight_items = false;
	show_blocking = false;
	show_tooltips = false;
	show_only_colors = false;
	show_only_modified = false;
	show_preview = false;
	hide_items_when_zoomed = true;
}

void DrawingOptions::SetIngame()
{
	transparent_floors = false;
	transparent_items = false;
	show_ingame_box = false;
	ingame = true;
	dragging = false;

	show_grid = 0;
	show_all_floors = true;
	show_creatures = true;
	show_spawns = false;
	show_houses = false;
	show_shade = false;
	show_special_tiles = false;
	show_items = true;

	highlight_items = false;
	show_blocking = false;
	show_tooltips = false;
	show_only_colors = false;
	show_only_modified = false;
	show_preview = false;
	hide_items_when_zoomed = false;
}

MapDrawer::MapDrawer(MapCanvas* canvas) : canvas(canvas), editor(canvas->editor)
{
	////
}

MapDrawer::~MapDrawer()
{
	Release();
}

void MapDrawer::SetupVars()
{
	canvas->MouseToMap(&mouse_map_x, &mouse_map_y);
	canvas->GetViewBox(&view_scroll_x, &view_scroll_y, &screensize_x, &screensize_y);

	dragging = canvas->dragging;
	dragging_draw = canvas->dragging_draw;

	zoom = (float)canvas->GetZoom();
	tile_size = int(TILE_SIZE / zoom); // after zoom
	floor = canvas->GetFloor();

	if(options.show_all_floors) {
		if(floor < 8)
			start_z = GROUND_LAYER;
		else
			start_z = std::min(MAP_MAX_LAYER, floor + 2);
	}
	else
		start_z = floor;

	end_z = floor;
	superend_z = (floor > GROUND_LAYER ? 8 : 0);

	start_x = view_scroll_x / TILE_SIZE;
	start_y = view_scroll_y / TILE_SIZE;

	if(floor > GROUND_LAYER) {
		start_x -= 2;
		start_y -= 2;
	}

	end_x = start_x + screensize_x / tile_size + 2;
	end_y = start_y + screensize_y / tile_size + 2;
}

void MapDrawer::SetupGL()
{
	glViewport(0, 0, screensize_x, screensize_y);

	// Enable 2D mode
	int vPort[4];

	glGetIntegerv(GL_VIEWPORT, vPort);

	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadIdentity();
	glOrtho(0, vPort[2]*zoom, vPort[3]*zoom, 0, -1, 1);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glTranslatef(0.375f, 0.375f, 0.0f);
}

void MapDrawer::Release()
{
	for(std::vector<MapTooltip*>::const_iterator it = tooltips.begin(); it != tooltips.end(); ++it) {
		delete *it;
	}
	tooltips.clear();

	// Disable 2D mode
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	glPopMatrix();
}

void MapDrawer::Draw()
{
	DrawBackground();
	DrawMap();
	DrawDraggingShadow();
	DrawHigherFloors();
	if(options.dragging)
		DrawSelectionBox();
	DrawLiveCursors();
	DrawBrush();
	if(options.show_grid)
		DrawGrid();
	if(options.show_ingame_box)
		DrawIngameBox();
	if(options.show_tooltips)
		DrawTooltips();
}

void MapDrawer::DrawBackground()
{
	// Black Background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);

	//glAlphaFunc(GL_GEQUAL, 0.9f);
	//glEnable(GL_ALPHA_TEST);
}

inline int getFloorAdjustment(int floor)
{
	if(floor > GROUND_LAYER) // Underground
		return 0; // No adjustment
	else
		return TILE_SIZE * (GROUND_LAYER - floor);
}

void MapDrawer::DrawMap()
{
	bool live_client = editor.IsLiveClient();

	Brush* brush = g_gui.GetCurrentBrush();

	// The current house we're drawing
	current_house_id = 0;
	if(brush) {
		if(brush->isHouse())
			current_house_id = brush->asHouse()->getHouseID();
		else if(brush->isHouseExit())
			current_house_id = brush->asHouseExit()->getHouseID();
	}

	// Enable texture mode
	if(!options.show_only_colors)
		glEnable(GL_TEXTURE_2D);

	for(int map_z = start_z; map_z >= superend_z; map_z--) {
		if(map_z == end_z && start_z != end_z && options.show_shade) {
			// Draw shade
			if(!options.show_only_colors)
				glDisable(GL_TEXTURE_2D);

			glColor4ub(0, 0, 0, 128);
			glBegin(GL_QUADS);
				glVertex2f(0,int(screensize_y*zoom));
				glVertex2f(int(screensize_x*zoom),int(screensize_y*zoom));
				glVertex2f(int(screensize_x*zoom),0);
				glVertex2f(0,0);
			glEnd();

			if(!options.show_only_colors)
				glEnable(GL_TEXTURE_2D);
		}

		if(map_z >= end_z) {
			int nd_start_x = start_x & ~3;
			int nd_start_y = start_y & ~3;
			int nd_end_x = (end_x & ~3) + 4;
			int nd_end_y = (end_y & ~3) + 4;

			for(int nd_map_x = nd_start_x; nd_map_x <= nd_end_x; nd_map_x += 4) {
				for(int nd_map_y = nd_start_y; nd_map_y <= nd_end_y; nd_map_y += 4) {
					QTreeNode* nd = editor.map.getLeaf(nd_map_x, nd_map_y);
					if(!nd) {
						if(live_client) {
							nd = editor.map.createLeaf(nd_map_x, nd_map_y);
							nd->setVisible(false, false);
						}
						else
							continue;
					}

					if(!live_client || nd->isVisible(map_z > GROUND_LAYER)) {
						for(int map_x = 0; map_x < 4; ++map_x) {
							for(int map_y = 0; map_y < 4; ++map_y) {
								DrawTile(nd->getTile(map_x, map_y, map_z));
							}
						}
					} else {
						if(!nd->isRequested(map_z > GROUND_LAYER)) {
							// Request the node
							editor.QueryNode(nd_map_x, nd_map_y, map_z > GROUND_LAYER);
							nd->setRequested(map_z > GROUND_LAYER, true);
						}
						int cy = (nd_map_y) * TILE_SIZE - view_scroll_y - getFloorAdjustment(floor);
						int cx = (nd_map_x) * TILE_SIZE - view_scroll_x - getFloorAdjustment(floor);

						glColor4ub(255, 0, 255, 128);
						glBegin(GL_QUADS);
							glVertex2f(cx, cy + TILE_SIZE * 4);
							glVertex2f(cx + TILE_SIZE * 4, cy + TILE_SIZE * 4);
							glVertex2f(cx + TILE_SIZE * 4, cy);
							glVertex2f(cx,     cy);
						glEnd();
					}
				}
			}
		}

		if(options.show_only_colors)
			glEnable(GL_TEXTURE_2D);

		// Draws the doodad preview or the paste preview (or import preview)
		if(g_gui.secondary_map != nullptr && !options.ingame) {
			Position normalPos;
			Position to(mouse_map_x, mouse_map_y, floor);

			if(canvas->isPasting()) {
				normalPos = editor.copybuffer.getPosition();
			} else if(brush && brush->isDoodad()) {
				normalPos = Position(0x8000, 0x8000, 0x8);
			}

			for(int map_x = start_x; map_x <= end_x; map_x++) {
				for(int map_y = start_y; map_y <= end_y; map_y++) {
					Position final(map_x, map_y, map_z);
					Position pos = normalPos + final - to;
					//Position pos = topos + copypos - Position(map_x, map_y, map_z);
					if(pos.z >= MAP_LAYERS || pos.z < 0) {
						continue;
					}

					Tile* tile = g_gui.secondary_map->getTile(pos);
					if(tile) {
						// Compensate for underground/overground
						int offset;
						if(map_z <= GROUND_LAYER)
							offset = (GROUND_LAYER - map_z) * TILE_SIZE;
						else
							offset = TILE_SIZE * (floor - map_z);

						int draw_x = ((map_x * TILE_SIZE) - view_scroll_x) - offset;
						int draw_y = ((map_y * TILE_SIZE) - view_scroll_y) - offset;

						// Draw ground
						uint8_t r = 160, g = 160, b = 160;
						if(tile->ground) {
							if(tile->isBlocking() && options.show_blocking) {
								g = g/3*2;
								b = b/3*2;
							}
							if(tile->isHouseTile() && options.show_houses) {
								if((int)tile->getHouseID() == current_house_id) {
									r /= 2;
								} else {
									r /= 2;
									g /= 2;
								}
							} else if(options.show_special_tiles && tile->isPZ()) {
								r /= 2;
								b /= 2;
							}
							if(options.show_special_tiles && tile->getMapFlags() & TILESTATE_PVPZONE) {
								r = r/3*2;
								b = r/3*2;
							}
							if(options.show_special_tiles && tile->getMapFlags() & TILESTATE_NOLOGOUT) {
								b /= 2;
							}
							if(options.show_special_tiles && tile->getMapFlags() & TILESTATE_NOPVP) {
								g /= 2;
							}
							BlitItem(draw_x, draw_y, tile, tile->ground, true, r, g, b, 160);
						}

						// Draw items on the tile
						if(zoom <= 10.0 || !options.hide_items_when_zoomed) {
							ItemVector::iterator it;
							for(it = tile->items.begin(); it != tile->items.end(); it++) {
								if((*it)->isBorder()) {
									BlitItem(draw_x, draw_y, tile, *it, true, 160, r, g, b);
								} else {
									BlitItem(draw_x, draw_y, tile, *it, true, 160, 160, 160, 160);
								}
							}
							if(tile->creature && options.show_creatures)
								BlitCreature(draw_x, draw_y, tile->creature);
						}
					}
				}
			}
		}

		--start_x;
		--start_y;
		++end_x;
		++end_y;
	}

	if(!options.show_only_colors)
		glEnable(GL_TEXTURE_2D);
}

void MapDrawer::DrawIngameBox()
{
	int center_x = start_x + int(screensize_x * zoom / 64);
	int center_y = start_y + int(screensize_y * zoom / 64);

	int offset_y = 2;
	int box_start_map_x = center_x;
	int box_start_map_y = center_y + offset_y;
	int box_end_map_x = center_x + CLIENT_MAP_WIDTH;
	int box_end_map_y = center_y + CLIENT_MAP_HEIGHT + offset_y;

	int box_start_x = box_start_map_x * TILE_SIZE - view_scroll_x;
	int box_start_y = box_start_map_y * TILE_SIZE - view_scroll_y;
	int box_end_x = box_end_map_x * TILE_SIZE - view_scroll_x;
	int box_end_y = box_end_map_y * TILE_SIZE - view_scroll_y;

	if(box_start_map_x >= start_x) {
		glColor4ub(0, 0, 0, 128);
		glBegin(GL_QUADS);
			glVertex2f(0, screensize_y * zoom);
			glVertex2f(box_start_x, screensize_y * zoom);
			glVertex2f(box_start_x, 0);
			glVertex2f(0, 0);
		glEnd();
	}

	if(box_end_map_x < end_x) {
		glColor4ub(0, 0, 0, 128);
		glBegin(GL_QUADS);
			glVertex2f(box_end_x, screensize_y * zoom);
			glVertex2f(screensize_x * zoom, screensize_y * zoom);
			glVertex2f(screensize_x * zoom, 0);
			glVertex2f(box_end_x, 0);
		glEnd();
	}

	if(box_start_map_y >= start_y) {
		glColor4ub(0, 0, 0, 128);
		glBegin(GL_QUADS);
			glVertex2f(box_start_x, box_start_y);
			glVertex2f(box_end_x, box_start_y);
			glVertex2f(box_end_x, 0);
			glVertex2f(box_start_x, 0);
		glEnd();
	}

	if(box_end_map_y < end_y) {
		glColor4ub(0, 0, 0, 128);
		glBegin(GL_QUADS);
			glVertex2f(box_start_x, screensize_y * zoom);
			glVertex2f(box_end_x, screensize_y * zoom);
			glVertex2f(box_end_x, box_end_y);
			glVertex2f(box_start_x, box_end_y);
		glEnd();
	}

	// client hidden tiles
	glColor4ub(255, 0, 0, 128);
	glBegin(GL_LINE_STRIP);
		glVertex2f(box_start_x, box_start_y);
		glVertex2f(box_end_x, box_start_y);
		glVertex2f(box_end_x, box_end_y);
		glVertex2f(box_start_x, box_end_y);
		glVertex2f(box_start_x, box_start_y);
	glEnd();

	box_start_x += TILE_SIZE;
	box_start_y += TILE_SIZE;
	box_end_x -= 2 * TILE_SIZE;
	box_end_y -= 2 * TILE_SIZE;

	// client visible tiles
	glColor4ub(0, 255, 0, 128);
	glBegin(GL_LINE_STRIP);
		glVertex2f(box_start_x, box_start_y);
		glVertex2f(box_end_x, box_start_y);
		glVertex2f(box_end_x, box_end_y);
		glVertex2f(box_start_x, box_end_y);
		glVertex2f(box_start_x, box_start_y);
	glEnd();

	box_start_x += 7 * TILE_SIZE;
	box_start_y += 5 * TILE_SIZE;
	box_end_x = box_start_x + TILE_SIZE;
	box_end_y = box_start_y + TILE_SIZE;

	// client player position
	glColor4ub(0, 255, 0, 128);
	glBegin(GL_LINE_STRIP);
		glVertex2f(box_start_x, box_start_y);
		glVertex2f(box_end_x, box_start_y);
		glVertex2f(box_end_x, box_end_y);
		glVertex2f(box_start_x, box_end_y);
		glVertex2f(box_start_x, box_start_y);
	glEnd();
}

void MapDrawer::DrawGrid()
{
	for(int y = start_y; y < end_y; ++y) {
		glColor4ub(255, 255, 255, 128);
		glBegin(GL_LINES);
			glVertex2f(start_x * TILE_SIZE - view_scroll_x, y * TILE_SIZE - view_scroll_y);
			glVertex2f(end_x * TILE_SIZE - view_scroll_x, y * TILE_SIZE - view_scroll_y);
		glEnd();
	}

	for(int x = start_x; x < end_x; ++x) {
		glColor4ub(255, 255, 255, 128);
		glBegin(GL_LINES);
			glVertex2f(x * TILE_SIZE - view_scroll_x, start_y * TILE_SIZE - view_scroll_y);
			glVertex2f(x * TILE_SIZE - view_scroll_x, end_y * TILE_SIZE - view_scroll_y);
		glEnd();
	}
}

void MapDrawer::DrawDraggingShadow()
{
	glEnable(GL_TEXTURE_2D);

	// Draw dragging shadow
	if(!editor.selection.isBusy() && dragging && !options.ingame) {
		for(TileVector::iterator tit = editor.selection.begin(); tit != editor.selection.end(); tit++) {
			Tile* tile = *tit;
			Position pos = tile->getPosition();

			int move_x, move_y, move_z;
			move_x = canvas->drag_start_x - mouse_map_x;
			move_y = canvas->drag_start_y - mouse_map_y;
			move_z = canvas->drag_start_z - floor;

			pos.x -= move_x;
			pos.y -= move_y;
			pos.z -= move_z;

			if(pos.z < 0 || pos.z >= MAP_LAYERS) {
				continue;
			}

			// On screen and dragging?
			if(pos.x+2 > start_x && pos.x < end_x && pos.y+2 > start_y && pos.y < end_y && (move_x != 0 || move_y != 0 || move_z != 0)) {
				int offset;
				if(pos.z <= GROUND_LAYER)
					offset = (GROUND_LAYER - pos.z) * TILE_SIZE;
				else
					offset = TILE_SIZE * (floor - pos.z);

				int draw_x = ((pos.x * TILE_SIZE) - view_scroll_x) - offset;
				int draw_y = ((pos.y * TILE_SIZE) - view_scroll_y) - offset;

				ItemVector toRender = tile->getSelectedItems();
				Tile* desttile = editor.map.getTile(pos);
				for(ItemVector::const_iterator iit = toRender.begin(); iit != toRender.end(); iit++) {
					if(desttile)
						BlitItem(draw_x, draw_y, desttile, *iit, true, 160,160,160,160);
					else
						BlitItem(draw_x, draw_y, pos, *iit, true, 160,160,160,160);
				}

				if(tile->creature && tile->creature->isSelected() && options.show_creatures)
					BlitCreature(draw_x, draw_y, tile->creature);
				if(tile->spawn && tile->spawn->isSelected())
					BlitSpriteType(draw_x, draw_y, SPRITE_SPAWN, 160, 160, 160, 160);
			}
		}
	}

	glDisable(GL_TEXTURE_2D);
}

void MapDrawer::DrawHigherFloors()
{
	glEnable(GL_TEXTURE_2D);

	// Draw "transparent higher floor"
	if(floor != 8 && floor != 0 && options.transparent_floors) {
		int map_z = floor - 1;
		for(int map_x = start_x; map_x <= end_x; map_x++) {
			for(int map_y = start_y; map_y <= end_y; map_y++) {
				Tile* tile = editor.map.getTile(map_x, map_y, map_z);
				if(tile) {
					int offset;
					if (map_z <= GROUND_LAYER)
						offset = (GROUND_LAYER - map_z) * TILE_SIZE;
					else
						offset = TILE_SIZE*(floor - map_z);

					int draw_x = ((map_x * TILE_SIZE) - view_scroll_x) - offset;
					int draw_y = ((map_y * TILE_SIZE) - view_scroll_y) - offset;

					//Position pos = tile->getPosition();

					if(tile->ground) {
						if(tile->isPZ()) {
							BlitItem(draw_x, draw_y, tile, tile->ground, false, 128,255,128, 96);
						} else {
							BlitItem(draw_x, draw_y, tile, tile->ground, false, 255,255,255, 96);
						}
					}
					if(zoom <= 10.0 || !options.hide_items_when_zoomed) {
						ItemVector::iterator it;
						for(it = tile->items.begin(); it != tile->items.end(); it++)
							BlitItem(draw_x, draw_y, tile, *it, false, 255,255,255, 96);
					}
				}
			}
		}
	}

	glDisable(GL_TEXTURE_2D);
}

void MapDrawer::DrawSelectionBox()
{
	if(options.ingame)
		return;

	// Draw bounding box

	int last_click_rx = canvas->last_click_abs_x - view_scroll_x;
	int last_click_ry = canvas->last_click_abs_y - view_scroll_y;
	double cursor_rx = canvas->cursor_x * zoom;
	double cursor_ry = canvas->cursor_y * zoom;

	double lines[4][4];

	lines[0][0] = last_click_rx;
	lines[0][1] = last_click_ry;
	lines[0][2] = cursor_rx;
	lines[0][3] = last_click_ry;

	lines[1][0] = cursor_rx;
	lines[1][1] = last_click_ry;
	lines[1][2] = cursor_rx;
	lines[1][3] = cursor_ry;

	lines[2][0] = cursor_rx;
	lines[2][1] = cursor_ry;
	lines[2][2] = last_click_rx;
	lines[2][3] = cursor_ry;

	lines[3][0] = last_click_rx;
	lines[3][1] = cursor_ry;
	lines[3][2] = last_click_rx;
	lines[3][3] = last_click_ry;

	glEnable(GL_LINE_STIPPLE);
	glLineStipple(1, 0xf0);
	glLineWidth(1.0);
	glColor4f(1.0,1.0,1.0,1.0);
	glBegin(GL_LINES);
	for(int i = 0; i < 4; i++) {
		glVertex2f(lines[i][0], lines[i][1]);
		glVertex2f(lines[i][2], lines[i][3]);
	}
	glEnd();
	glDisable(GL_LINE_STIPPLE);
}

void MapDrawer::DrawLiveCursors()
{
	if(options.ingame || !editor.IsLive())
		return;

	LiveSocket& live = editor.GetLive();
	for(LiveCursor& cursor : live.getCursorList()) {
		if(cursor.pos.z <= GROUND_LAYER && floor > GROUND_LAYER) {
			continue;
		}

		if(cursor.pos.z > GROUND_LAYER && floor <= 8) {
			continue;
		}

		if(cursor.pos.z < floor) {
			cursor.color = wxColor(
				cursor.color.Red(),
				cursor.color.Green(),
				cursor.color.Blue(),
				std::max<uint8_t>(cursor.color.Alpha() / 2, 64)
			);
		}

		int offset;
		if (cursor.pos.z <= GROUND_LAYER)
			offset = (GROUND_LAYER - cursor.pos.z) * TILE_SIZE;
		else
			offset = TILE_SIZE * (floor - cursor.pos.z);

		float draw_x = ((cursor.pos.x * TILE_SIZE) - view_scroll_x) - offset;
		float draw_y = ((cursor.pos.y * TILE_SIZE) - view_scroll_y) - offset;

		glColor(cursor.color);
		glBegin(GL_QUADS);
			glVertex2f(draw_x, draw_y);
			glVertex2f(draw_x + TILE_SIZE, draw_y);
			glVertex2f(draw_x + TILE_SIZE, draw_y + TILE_SIZE);
			glVertex2f(draw_x, draw_y + TILE_SIZE);
		glEnd();
	}
}

void MapDrawer::DrawBrush()
{
	if(!g_gui.IsDrawingMode())
		return;
	if(!g_gui.GetCurrentBrush())
		return;
	if(options.ingame)
		return;

	Brush* brush = g_gui.GetCurrentBrush();

	BrushColor brushColor = COLOR_BLANK;
	if(brush->isTerrain() || brush->isTable() || brush->isCarpet())
		brushColor = COLOR_BRUSH;
	else if(brush->isHouse())
		brushColor = COLOR_HOUSE_BRUSH;
	else if(brush->isFlag())
		brushColor = COLOR_FLAG_BRUSH;
	else if(brush->isSpawn())
		brushColor = COLOR_SPAWN_BRUSH;
	else if(brush->isEraser())
		brushColor = COLOR_ERASER;

	if(dragging_draw) {
		ASSERT(brush->canDrag());

		if(brush->isWall()) {
			int last_click_start_map_x = std::min(canvas->last_click_map_x, mouse_map_x);
			int last_click_start_map_y = std::min(canvas->last_click_map_y, mouse_map_y);
			int last_click_end_map_x   = std::max(canvas->last_click_map_x, mouse_map_x)+1;
			int last_click_end_map_y   = std::max(canvas->last_click_map_y, mouse_map_y)+1;

			int last_click_start_sx = last_click_start_map_x * TILE_SIZE - view_scroll_x - getFloorAdjustment(floor);
			int last_click_start_sy = last_click_start_map_y * TILE_SIZE - view_scroll_y - getFloorAdjustment(floor);
			int last_click_end_sx = last_click_end_map_x * TILE_SIZE - view_scroll_x - getFloorAdjustment(floor);
			int last_click_end_sy = last_click_end_map_y * TILE_SIZE - view_scroll_y - getFloorAdjustment(floor);

			int delta_x = last_click_end_sx - last_click_start_sx;
			int delta_y = last_click_end_sy - last_click_start_sy;

			glColor(brushColor);
			glBegin(GL_QUADS);
				{
					glVertex2f(last_click_start_sx, last_click_start_sy + TILE_SIZE);
					glVertex2f(last_click_end_sx, last_click_start_sy + TILE_SIZE);
					glVertex2f(last_click_end_sx, last_click_start_sy);
					glVertex2f(last_click_start_sx, last_click_start_sy);
				}

				if(delta_y > TILE_SIZE) {
					glVertex2f(last_click_start_sx, last_click_end_sy - TILE_SIZE);
					glVertex2f(last_click_start_sx + TILE_SIZE, last_click_end_sy - TILE_SIZE);
					glVertex2f(last_click_start_sx + TILE_SIZE, last_click_start_sy + TILE_SIZE);
					glVertex2f(last_click_start_sx, last_click_start_sy + TILE_SIZE);
				}

				if(delta_x > TILE_SIZE && delta_y > TILE_SIZE) {
					glVertex2f(last_click_end_sx - TILE_SIZE, last_click_start_sy + TILE_SIZE);
					glVertex2f(last_click_end_sx, last_click_start_sy + TILE_SIZE);
					glVertex2f(last_click_end_sx, last_click_end_sy - TILE_SIZE);
					glVertex2f(last_click_end_sx - TILE_SIZE, last_click_end_sy - TILE_SIZE);
				}

				if (delta_y > TILE_SIZE) {
					glVertex2f(last_click_start_sx, last_click_end_sy - TILE_SIZE);
					glVertex2f(last_click_end_sx, last_click_end_sy - TILE_SIZE);
					glVertex2f(last_click_end_sx, last_click_end_sy);
					glVertex2f(last_click_start_sx, last_click_end_sy);
				}
			glEnd();
		} else {
			if(brush->isRaw())
				glEnable(GL_TEXTURE_2D);

			if(g_gui.GetBrushShape() == BRUSHSHAPE_SQUARE || brush->isSpawn() /* Spawn brush is always square */) {
				if(brush->isRaw() || brush->isOptionalBorder()) {
					int start_x, end_x;
					int start_y, end_y;

					if(mouse_map_x < canvas->last_click_map_x) {
						start_x = mouse_map_x;
						end_x = canvas->last_click_map_x;
					} else {
						start_x = canvas->last_click_map_x;
						end_x = mouse_map_x;
					}
					if(mouse_map_y < canvas->last_click_map_y) {
						start_y = mouse_map_y;
						end_y = canvas->last_click_map_y;
					} else {
						start_y = canvas->last_click_map_y;
						end_y = mouse_map_y;
					}

					RAWBrush* raw_brush = nullptr;
					if(brush->isRaw())
						raw_brush = brush->asRaw();

					for(int y = start_y; y <= end_y; y++) {
						int cy = y * TILE_SIZE - view_scroll_y - getFloorAdjustment(floor);
						for(int x = start_x; x <= end_x; x++) {
							int cx = x * TILE_SIZE - view_scroll_x - getFloorAdjustment(floor);
							if(brush->isOptionalBorder())
								glColorCheck(brush, Position(x, y, floor));
							else
								BlitSpriteType(cx, cy, raw_brush->getItemType()->sprite, 160, 160, 160, 160);
						}
					}
				} else {
					int last_click_start_map_x = std::min(canvas->last_click_map_x, mouse_map_x);
					int last_click_start_map_y = std::min(canvas->last_click_map_y, mouse_map_y);
					int last_click_end_map_x   = std::max(canvas->last_click_map_x, mouse_map_x)+1;
					int last_click_end_map_y   = std::max(canvas->last_click_map_y, mouse_map_y)+1;

					int last_click_start_sx = last_click_start_map_x * TILE_SIZE - view_scroll_x - getFloorAdjustment(floor);
					int last_click_start_sy = last_click_start_map_y * TILE_SIZE - view_scroll_y - getFloorAdjustment(floor);
					int last_click_end_sx = last_click_end_map_x * TILE_SIZE - view_scroll_x - getFloorAdjustment(floor);
					int last_click_end_sy = last_click_end_map_y * TILE_SIZE - view_scroll_y - getFloorAdjustment(floor);

					glColor(brushColor);
					glBegin(GL_QUADS);
						glVertex2f(last_click_start_sx, last_click_start_sy);
						glVertex2f(last_click_end_sx, last_click_start_sy);
						glVertex2f(last_click_end_sx, last_click_end_sy);
						glVertex2f(last_click_start_sx, last_click_end_sy);
					glEnd();
				}
			} else if(g_gui.GetBrushShape() == BRUSHSHAPE_CIRCLE) {
				// Calculate drawing offsets
				int start_x, end_x;
				int start_y, end_y;
				int width = std::max(
					std::abs(std::max(mouse_map_y, canvas->last_click_map_y) - std::min(mouse_map_y, canvas->last_click_map_y)),
					std::abs(std::max(mouse_map_x, canvas->last_click_map_x) - std::min(mouse_map_x, canvas->last_click_map_x))
					);

				if(mouse_map_x < canvas->last_click_map_x) {
					start_x = canvas->last_click_map_x - width;
					end_x = canvas->last_click_map_x;
				} else {
					start_x = canvas->last_click_map_x;
					end_x = canvas->last_click_map_x + width;
				}

				if(mouse_map_y < canvas->last_click_map_y) {
					start_y = canvas->last_click_map_y - width;
					end_y = canvas->last_click_map_y;
				} else {
					start_y = canvas->last_click_map_y;
					end_y = canvas->last_click_map_y + width;
				}

				int center_x = start_x + (end_x - start_x) / 2;
				int center_y = start_y + (end_y - start_y) / 2;
				float radii = width / 2.0f + 0.005f;

				RAWBrush* raw_brush = nullptr;
				if(brush->isRaw())
					raw_brush = brush->asRaw();

				for(int y = start_y-1; y <= end_y+1; y++) {
					int cy = y * TILE_SIZE - view_scroll_y - getFloorAdjustment(floor);
					float dy = center_y - y;
					for(int x = start_x-1; x <= end_x+1; x++) {
						int cx = x * TILE_SIZE - view_scroll_x - getFloorAdjustment(floor);

						float dx = center_x - x;
						//printf("%f;%f\n", dx, dy);
						float distance = sqrt(dx*dx + dy*dy);
						if(distance < radii) {
							if(brush->isRaw()) {
								BlitSpriteType(cx, cy, raw_brush->getItemType()->sprite, 160, 160, 160, 160);
							} else {
								glColor(brushColor);
								glBegin(GL_QUADS);
									glVertex2f(cx, cy + TILE_SIZE);
									glVertex2f(cx + TILE_SIZE, cy + TILE_SIZE);
									glVertex2f(cx + TILE_SIZE, cy);
									glVertex2f(cx,   cy);
								glEnd();
							}
						}
					}
				}
			}

			if(brush->isRaw())
				glDisable(GL_TEXTURE_2D);
		}
	} else {
		if(brush->isWall()) {
			int start_map_x = mouse_map_x - g_gui.GetBrushSize();
			int start_map_y = mouse_map_y - g_gui.GetBrushSize();
			int end_map_x   = mouse_map_x + g_gui.GetBrushSize() + 1;
			int end_map_y   = mouse_map_y + g_gui.GetBrushSize() + 1;

			int start_sx = start_map_x * TILE_SIZE - view_scroll_x - getFloorAdjustment(floor);
			int start_sy = start_map_y * TILE_SIZE - view_scroll_y - getFloorAdjustment(floor);
			int end_sx = end_map_x * TILE_SIZE - view_scroll_x - getFloorAdjustment(floor);
			int end_sy = end_map_y * TILE_SIZE - view_scroll_y - getFloorAdjustment(floor);

			int delta_x = end_sx - start_sx;
			int delta_y = end_sy - start_sy;

			glColor(brushColor);
			glBegin(GL_QUADS);
				{
					glVertex2f(start_sx, start_sy + TILE_SIZE);
					glVertex2f(end_sx, start_sy + TILE_SIZE);
					glVertex2f(end_sx, start_sy);
					glVertex2f(start_sx, start_sy);
				}

				if(delta_y > TILE_SIZE) {
					glVertex2f(start_sx, end_sy - TILE_SIZE);
					glVertex2f(start_sx + TILE_SIZE, end_sy - TILE_SIZE);
					glVertex2f(start_sx + TILE_SIZE, start_sy + TILE_SIZE);
					glVertex2f(start_sx, start_sy + TILE_SIZE);
				}

				if(delta_x > TILE_SIZE && delta_y > TILE_SIZE) {
					glVertex2f(end_sx - TILE_SIZE, start_sy + TILE_SIZE);
					glVertex2f(end_sx, start_sy + TILE_SIZE);
					glVertex2f(end_sx, end_sy - TILE_SIZE);
					glVertex2f(end_sx - TILE_SIZE, end_sy - TILE_SIZE);
				}

				if(delta_y > TILE_SIZE) {
					glVertex2f(start_sx, end_sy - TILE_SIZE);
					glVertex2f(end_sx, end_sy - TILE_SIZE);
					glVertex2f(end_sx, end_sy);
					glVertex2f(start_sx, end_sy);
				}
			glEnd();
		} else if(brush->isDoor()) {
			int cx = (mouse_map_x) * TILE_SIZE - view_scroll_x - getFloorAdjustment(floor);
			int cy = (mouse_map_y) * TILE_SIZE - view_scroll_y - getFloorAdjustment(floor);

			glColorCheck(brush, Position(mouse_map_x, mouse_map_y, floor));
			glBegin(GL_QUADS);
				glVertex2f(cx, cy + TILE_SIZE);
				glVertex2f(cx + TILE_SIZE, cy + TILE_SIZE);
				glVertex2f(cx + TILE_SIZE, cy);
				glVertex2f(cx, cy);
			glEnd();
		} else if(brush->isCreature()) {
			glEnable(GL_TEXTURE_2D);
			int cy = (mouse_map_y) * TILE_SIZE - view_scroll_y - getFloorAdjustment(floor);
			int cx = (mouse_map_x) * TILE_SIZE - view_scroll_x - getFloorAdjustment(floor);
			CreatureBrush* creature_brush = brush->asCreature();
			if(creature_brush->canDraw(&editor.map, Position(mouse_map_x, mouse_map_y, floor)))
				BlitCreature(cx, cy, creature_brush->getType()->outfit, SOUTH, 255, 255, 255, 160);
			else
				BlitCreature(cx, cy, creature_brush->getType()->outfit, SOUTH, 255, 64, 64, 160);
			glDisable(GL_TEXTURE_2D);
		} else if(!brush->isDoodad()) {
			RAWBrush* raw_brush = nullptr;
			if(brush->isRaw()) { // Textured brush
				glEnable(GL_TEXTURE_2D);
				raw_brush = brush->asRaw();
			}

			for(int y = -g_gui.GetBrushSize()-1; y <= g_gui.GetBrushSize()+1; y++) {
				int cy = (mouse_map_y + y) * TILE_SIZE - view_scroll_y - getFloorAdjustment(floor);
				for(int x = -g_gui.GetBrushSize()-1; x <= g_gui.GetBrushSize()+1; x++) {
					int cx = (mouse_map_x + x) * TILE_SIZE - view_scroll_x - getFloorAdjustment(floor);
					if(g_gui.GetBrushShape() == BRUSHSHAPE_SQUARE) {
						if(x >= -g_gui.GetBrushSize() && x <= g_gui.GetBrushSize() && y >= -g_gui.GetBrushSize() && y <= g_gui.GetBrushSize()) {
							if(brush->isRaw()) {
								BlitSpriteType(cx, cy, raw_brush->getItemType()->sprite, 160, 160, 160, 160);
							} else {
								if(brush->isWaypoint()) {
									uint8_t r, g, b;
									getColor(brush, Position(mouse_map_x + x, mouse_map_y + y, floor), r, g, b);
									DrawBrushIndicator(cx, cy, brush, r, g, b);
								} else {
									if(brush->isHouseExit() || brush->isOptionalBorder())
										glColorCheck(brush, Position(mouse_map_x + x, mouse_map_y + y, floor));
									else
										glColor(brushColor);

									glBegin(GL_QUADS);
										glVertex2f(cx, cy + TILE_SIZE);
										glVertex2f(cx + TILE_SIZE, cy + TILE_SIZE);
										glVertex2f(cx + TILE_SIZE, cy);
										glVertex2f(cx, cy);
									glEnd();
								}
							}
						}
					} else if(g_gui.GetBrushShape() == BRUSHSHAPE_CIRCLE) {
						double distance = sqrt(double(x*x) + double(y*y));
						if(distance < g_gui.GetBrushSize()+0.005) {
							if(brush->isRaw()) {
								BlitSpriteType(cx, cy, raw_brush->getItemType()->sprite, 160, 160, 160, 160);
							} else {
								if(brush->isWaypoint()) {
									uint8_t r, g, b;
									getColor(brush, Position(mouse_map_x + x, mouse_map_y + y, floor), r, g, b);
									DrawBrushIndicator(cx, cy, brush, r, g, b);
								} else {
									if(brush->isHouseExit() || brush->isOptionalBorder())
										glColorCheck(brush, Position(mouse_map_x + x, mouse_map_y + y, floor));
									else
										glColor(brushColor);

									glBegin(GL_QUADS);
										glVertex2f(cx, cy + TILE_SIZE);
										glVertex2f(cx + TILE_SIZE, cy + TILE_SIZE);
										glVertex2f(cx + TILE_SIZE, cy);
										glVertex2f(cx, cy);
									glEnd();
								}
							}
						}
					}
				}
			}

			if(brush->isRaw()) { // Textured brush
				glDisable(GL_TEXTURE_2D);
			}
		}
	}
}

void MapDrawer::BlitItem(int& draw_x, int& draw_y, const Tile* tile, const Item* item, bool ephemeral, int red, int green, int blue, int alpha) {
	ItemType& it = g_items[item->getID()];

	if(!options.ingame && !ephemeral && item->isSelected()) {
		red /= 2;
		blue /= 2;
		green /= 2;
	}

	// Ugly hacks. :)
	if(it.id == 0) {
		glDisable(GL_TEXTURE_2D);
		glBlitSquare(draw_x, draw_y, 255, 0, 0, alpha);
		glEnable(GL_TEXTURE_2D);
		return;
	} else if(it.id == 459 && !options.ingame) {
		glDisable(GL_TEXTURE_2D);
		glBlitSquare(draw_x, draw_y, red, green, 0, alpha/3*2);
		glEnable(GL_TEXTURE_2D);
		return;
	} else if(it.id == 460 && !options.ingame) {
		glDisable(GL_TEXTURE_2D);
		glBlitSquare(draw_x, draw_y, red, 0, 0, alpha/3*2);
		glEnable(GL_TEXTURE_2D);
		return;
	}


	GameSprite* spr = it.sprite;

	if(it.isMetaItem())
		return;
	if(spr == nullptr)
		return;
	if(!ephemeral && it.pickupable && !options.show_items)
		return;

	int screenx = draw_x - spr->getDrawOffset().first;
	int screeny = draw_y - spr->getDrawOffset().second;

	const Position& pos = tile->getPosition();

	// Set the newd drawing height accordingly
	draw_x -= spr->getDrawHeight();
	draw_y -= spr->getDrawHeight();

	int subtype = -1;

	int pattern_x = pos.x % spr->pattern_x;
	int pattern_y = pos.y % spr->pattern_y;
	int pattern_z = pos.z % spr->pattern_z;

	if(it.isSplash() || it.isFluidContainer()) {
		subtype = item->getSubtype();
	} else if(it.isHangable) {
		if(tile->hasProperty(ISVERTICAL)) {
			pattern_x = 2;
		} else if(tile->hasProperty(ISHORIZONTAL)) {
			pattern_x = 1;
		} else {
			pattern_x = -0;
		}
	} else if(it.stackable) {
		if(item->getSubtype() <= 1)
			subtype = 0;
		else if(item->getSubtype() <= 2)
			subtype = 1;
		else if(item->getSubtype() <= 3)
			subtype = 2;
		else if(item->getSubtype() <= 4)
			subtype = 3;
		else if(item->getSubtype() < 10)
			subtype = 4;
		else if(item->getSubtype() < 25)
			subtype = 5;
		else if(item->getSubtype() < 50)
			subtype = 6;
		else
			subtype = 7;
	}

	if(!ephemeral && options.transparent_items &&
			(!it.isGroundTile() || spr->width > 1 || spr->height > 1) &&
			!it.isSplash() &&
			(!it.isBorder || spr->width > 1 || spr->height > 1)
	  )
	{
		alpha /= 2;
	}

	int frame = item->getFrame();
	for(int cx = 0; cx != spr->width; cx++) {
		for(int cy = 0; cy != spr->height; cy++) {
			for(int cf = 0; cf != spr->layers; cf++) {
				int texnum = spr->getHardwareID(cx,cy,cf,
					subtype,
					pattern_x,
					pattern_y,
					pattern_z,
					frame
				);
				glBlitTexture(screenx - cx * TILE_SIZE, screeny - cy * TILE_SIZE, texnum, red, green, blue, alpha);
			}
		}
	}
}

void MapDrawer::BlitItem(int& draw_x, int& draw_y, const Position& pos, const Item* item, bool ephemeral, int red, int green, int blue, int alpha) {
	ItemType& it = g_items[item->getID()];

	if(!options.ingame && !ephemeral && item->isSelected()) {
		red /= 2;
		blue /= 2;
		green /= 2;
	}

	if(it.id == 459 && !options.ingame) { // Ugly hack yes?
		glDisable(GL_TEXTURE_2D);
		glBlitSquare(draw_x, draw_y, red, green, 0, alpha/3*2);
		glEnable(GL_TEXTURE_2D);
		return;
	} else if(it.id == 460 && !options.ingame) { // Ugly hack yes?
		glDisable(GL_TEXTURE_2D);
		glBlitSquare(draw_x, draw_y, red, 0, 0, alpha/3*2);
		glEnable(GL_TEXTURE_2D);
		return;
	}

	GameSprite* spr = it.sprite;

	if(it.isMetaItem())
		return;
	if(spr == nullptr)
		return;
	if(!ephemeral && it.pickupable && options.show_items)
		return;

	int screenx = draw_x - spr->getDrawOffset().first;
	int screeny = draw_y - spr->getDrawOffset().second;

	// Set the newd drawing height accordingly
	draw_x -= spr->getDrawHeight();
	draw_y -= spr->getDrawHeight();

	int subtype = -1;

	int pattern_x = pos.x % spr->pattern_x;
	int pattern_y = pos.y % spr->pattern_y;
	int pattern_z = pos.z % spr->pattern_z;

	if(it.isSplash() || it.isFluidContainer()) {
		subtype = item->getSubtype();
	} else if(it.isHangable) {
		pattern_x = 0;
		/*
		if(tile->hasProperty(ISVERTICAL)) {
			pattern_x = 2;
		} else if(tile->hasProperty(ISHORIZONTAL)) {
			pattern_x = 1;
		} else {
			pattern_x = -0;
		}
		*/
	} else if(it.stackable) {
		if(item->getSubtype() <= 1)
			subtype = 0;
		else if(item->getSubtype() <= 2)
			subtype = 1;
		else if(item->getSubtype() <= 3)
			subtype = 2;
		else if(item->getSubtype() <= 4)
			subtype = 3;
		else if(item->getSubtype() < 10)
			subtype = 4;
		else if(item->getSubtype() < 25)
			subtype = 5;
		else if(item->getSubtype() < 50)
			subtype = 6;
		else
			subtype = 7;
	}

	if(!ephemeral && options.transparent_items &&
			(!it.isGroundTile() || spr->width > 1 || spr->height > 1) &&
			!it.isSplash() &&
			(!it.isBorder || spr->width > 1 || spr->height > 1)
	  )
	{
		alpha /= 2;
	}

	int frame = item->getFrame();
	for(int cx = 0; cx != spr->width; ++cx) {
		for(int cy = 0; cy != spr->height; ++cy) {
			for(int cf = 0; cf != spr->layers; ++cf) {
				int texnum = spr->getHardwareID(cx,cy,cf,
					subtype,
					pattern_x,
					pattern_y,
					pattern_z,
					frame
				);
				glBlitTexture(screenx - cx * TILE_SIZE, screeny - cy * TILE_SIZE, texnum, red, green, blue, alpha);
			}
		}
	}
}

void MapDrawer::BlitSpriteType(int screenx, int screeny, uint32_t spriteid, int red, int green, int blue, int alpha)
{
	GameSprite* spr = g_items[spriteid].sprite;
	if(spr == nullptr) return;
	screenx -= spr->getDrawOffset().first;
	screeny -= spr->getDrawOffset().second;

	int tme = 0; //GetTime() % itype->FPA;
	for(int cx = 0; cx != spr->width; ++cx) {
		for(int cy = 0; cy != spr->height; ++cy) {
			for(int cf = 0; cf != spr->layers; ++cf) {
				int texnum = spr->getHardwareID(cx,cy,cf,-1,0,0,0,tme);
				//printf("CF: %d\tTexturenum: %d\n", cf, texnum);
				glBlitTexture(screenx - cx * TILE_SIZE, screeny - cy * TILE_SIZE, texnum, red, green, blue, alpha);
			}
		}
	}
}

void MapDrawer::BlitSpriteType(int screenx, int screeny, GameSprite* spr, int red, int green, int blue, int alpha)
{
	if(spr == nullptr) return;
	screenx -= spr->getDrawOffset().first;
	screeny -= spr->getDrawOffset().second;

	int tme = 0; //GetTime() % itype->FPA;
	for(int cx = 0; cx != spr->width; ++cx) {
		for(int cy = 0; cy != spr->height; ++cy) {
			for(int cf = 0; cf != spr->layers; ++cf) {
				int texnum = spr->getHardwareID(cx,cy,cf,-1,0,0,0,tme);
				//printf("CF: %d\tTexturenum: %d\n", cf, texnum);
				glBlitTexture(screenx - cx * TILE_SIZE, screeny - cy * TILE_SIZE, texnum, red, green, blue, alpha);
			}
		}
	}
}

void MapDrawer::BlitCreature(int screenx, int screeny, const Outfit& outfit, Direction dir, int red, int green, int blue, int alpha)
{
	if(outfit.lookItem != 0) {
		ItemType& it = g_items[outfit.lookItem];
		BlitSpriteType(screenx, screeny, it.sprite, red, green, blue, alpha);
	} else {
		GameSprite* spr = g_gui.gfx.getCreatureSprite(outfit.lookType);
		if(!spr || outfit.lookType == 0) {
			return;
			/*
			spr = g_gui.gfx.getCreatureSprite(138);
			if(!spr)
				return;
			 */
		}

		int tme = 0; //GetTime() % itype->FPA;
		for(int cx = 0; cx != spr->width; ++cx) {
			for(int cy = 0; cy != spr->height; ++cy) {
				int texnum = spr->getHardwareID(cx,cy,(int)dir,outfit,tme);
				glBlitTexture(screenx - cx * TILE_SIZE, screeny - cy * TILE_SIZE, texnum, red, green, blue, alpha);
			}
		}
	}
}

void MapDrawer::BlitCreature(int screenx, int screeny, const Creature* c, int red, int green, int blue, int alpha)
{
	if(!options.ingame && c->isSelected()) {
		red /= 2;
		green /= 2;
		blue /= 2;
	}
	BlitCreature(screenx, screeny, c->getLookType(), c->getDirection(), red, green, blue, alpha);
}

void MapDrawer::WriteTooltip(Item* item, std::ostringstream& stream)
{
	const uint16_t id = item->getID();
	if(id < 100)
		return;

	const uint16_t unique = item->getUniqueID();
	const uint16_t action = item->getActionID();
	const std::string& text = item->getText();
	if(unique == 0 && action == 0 && text.empty())
		return;

	if(stream.tellp() > 0)
		stream << "\n";

	stream << "id: " << id << "\n";

	if(unique > 0)
		stream << "uid: " << unique << "\n";
	if(action > 0)
		stream << "aid: " << action << "\n";
	if(!text.empty())
		stream << "text: " << text << "\n";
}

void MapDrawer::WriteTooltip(Waypoint* waypoint, std::ostringstream& stream)
{
	if (stream.tellp() > 0)
		stream << "\n";
	stream << "wp: " << waypoint->name << "\n";
}

void MapDrawer::DrawTile(TileLocation* location)
{
	if(!location)
		return;
	Tile* tile = location->get();

	if(!tile)
		return;

	if(options.show_only_modified && !tile->isModified())
		return;

	int map_x = location->getX();
	int map_y = location->getY();
	int map_z = location->getZ();

	std::ostringstream tooltip;

	if(options.show_tooltips && location->getWaypointCount() > 0) {
		Waypoint* waypoint = canvas->editor.map.waypoints.getWaypoint(location);
		if(waypoint)
			WriteTooltip(waypoint, tooltip);
	}

	bool only_colors = options.show_only_colors;

	int offset;
	if (map_z <= GROUND_LAYER)
		offset = (GROUND_LAYER - map_z) * TILE_SIZE;
	else
		offset = TILE_SIZE * (floor - map_z);

	int draw_x = ((map_x * TILE_SIZE) - view_scroll_x) - offset;
	int draw_y = ((map_y * TILE_SIZE) - view_scroll_y) - offset;

	uint8_t r = 255,g = 255,b = 255;
	if(tile->ground || only_colors) {
		bool showspecial = options.show_only_colors || options.show_special_tiles;

		if(tile->isBlocking() && tile->size() > 0 && options.show_blocking) {
			g = g/3*2;
			b = b/3*2;
		}

		int item_count = tile->items.size();
		if(options.highlight_items && item_count > 0 && !tile->items.back()->isBorder()) {
			static const float factor[5] = {0.75f, 0.6f, 0.48f, 0.40f, 0.33f};
			int idx = (item_count < 5 ? item_count : 5) - 1;
			g = int(g * factor[idx]);
			r = int(r * factor[idx]);
		}

		if(location->getSpawnCount() > 0 && options.show_spawns) {
			float f = 1.0f;
			for(uint32_t i = 0; i < location->getSpawnCount(); ++i) {
				f *= 0.7f;
			}
			g = uint8_t(g * f);
			b = uint8_t(b * f);
		}

		if(tile->isHouseTile() && options.show_houses) {
			if((int)tile->getHouseID() == current_house_id) {
				r /= 2;
			} else {
				r /= 2;
				g /= 2;
			}
		} else if(showspecial && tile->isPZ()) {
			r /= 2;
			b /= 2;
		}

		if(showspecial && tile->getMapFlags() & TILESTATE_PVPZONE) {
			g = r/4;
			b = b/3*2;
		}

		if(showspecial && tile->getMapFlags() & TILESTATE_NOLOGOUT) {
			b /= 2;
		}

		if(showspecial && tile->getMapFlags() & TILESTATE_NOPVP) {
			g /= 2;
		}

		if(only_colors) {
			if(r != 255 || g != 255 || b != 255) {
				glBlitSquare(draw_x, draw_y, r, g, b, 128);
			}
		} else {
			if(options.show_preview && zoom <= 2.0)
				tile->ground->animate();

			BlitItem(draw_x, draw_y, tile, tile->ground, false, r, g, b);
		}

		if(options.show_tooltips && map_z == floor)
			WriteTooltip(tile->ground, tooltip);
	}

	if(!only_colors) {
		if(zoom < 10.0 || !options.hide_items_when_zoomed) {
			for(ItemVector::iterator it = tile->items.begin(); it != tile->items.end(); it++) {
				if(options.show_tooltips && map_z == floor)
					WriteTooltip(*it, tooltip);

				if(options.show_preview && zoom <= 2.0)
					(*it)->animate();

				if((*it)->isBorder()) {
					BlitItem(draw_x, draw_y, tile, *it, false, r, g, b);
				} else {
					BlitItem(draw_x, draw_y, tile, *it);
				}
			}
			if(tile->creature && options.show_creatures) {
				BlitCreature(draw_x, draw_y, tile->creature);
			}
		}
		//if(location->getWaypointCount() > 0 && options.show_houses) {
			//BlitSpriteType(draw_x, draw_y, SPRITE_FLAME_BLUE, 64, 64, 255);
		//}

		if(tile->isHouseExit() && options.show_houses) {
			if(tile->hasHouseExit(current_house_id)) {
				BlitSpriteType(draw_x, draw_y, SPRITE_FLAG_GREY, 64, 255, 255);
			} else {
				BlitSpriteType(draw_x, draw_y, SPRITE_FLAG_GREY, 64, 64, 255);
			}
		}
		//if(tile->isTownExit()) {
		//	BlitSpriteType(draw_x, draw_y, SPRITE_FLAG_GREY, 255, 255, 64);
		//}
		if(tile->spawn && options.show_spawns) {
			if(tile->spawn->isSelected()) {
				BlitSpriteType(draw_x, draw_y, SPRITE_SPAWN, 128, 128, 128);
			} else {
				BlitSpriteType(draw_x, draw_y, SPRITE_SPAWN, 255, 255, 255);
			}
		}
	}

	if(options.show_tooltips) {
		if(location->getWaypointCount() > 0)
			MakeTooltip(draw_x, draw_y, tooltip.str(), 0, 255, 0);
		else
			MakeTooltip(draw_x, draw_y, tooltip.str());
	}
}

void MapDrawer::DrawBrushIndicator(int x, int y, Brush* brush, uint8_t r, uint8_t g, uint8_t b)
{
	x += (TILE_SIZE / 2);
	y += (TILE_SIZE / 2);

	// 7----0----1
	// |         |
	// 6--5  3--2
	//     \/
	//     4
	static int vertexes[9][2] = {
		{-15, -20},  // 0
		{ 15, -20},  // 1
		{ 15, -5},   // 2
		{ 5,  -5},   // 3
		{ 0,   0},   // 4
		{-5,  -5},   // 5
		{-15, -5},   // 6
		{-15, -20},  // 7
		{-15, -20},  // 0
	};

	// circle
	glBegin(GL_TRIANGLE_FAN);
	glColor4ub(0x00, 0x00, 0x00, 0x50);
	glVertex2i(x, y);
	for(int i = 0; i <= 30; i++) {
		float angle = i * 2.0f * PI / 30;
		glVertex2f(cos(angle) * (TILE_SIZE / 2) + x, sin(angle) * (TILE_SIZE / 2) + y);
	}
	glEnd();

	// background
	glColor4ub(r, g, b, 0xB4);
	glBegin(GL_POLYGON);
	for(int i = 0; i < 8; ++i)
		glVertex2i(vertexes[i][0] + x, vertexes[i][1] + y);
	glEnd();

	// borders
	glColor4ub(0x00, 0x00, 0x00, 0xB4);
	glLineWidth(1.0);
	glBegin(GL_LINES);
	for(int i = 0; i < 8; ++i) {
		glVertex2i(vertexes[i][0] + x, vertexes[i][1] + y);
		glVertex2i(vertexes[i + 1][0] + x, vertexes[i + 1][1] + y);
	}
	glEnd();
}

void MapDrawer::DrawTooltips()
{
	const int MAX_LINE_CHARS = 40;

	for(std::vector<MapTooltip*>::const_iterator it = tooltips.begin(); it != tooltips.end(); ++it) {
		MapTooltip* tooltip = (*it);
		const char* text = tooltip->text.c_str();
		float line_width = 0.0f;
		float width = 2.0f;
		float height = 14.0f;
		int char_count = 0;

		for(const char* c = text; *c != '\0'; c++) {
			if(*c == '\n' || (char_count >= MAX_LINE_CHARS && *c == ' ')) {
				height += 14.0f;
				line_width = 0.0f;
				char_count = 0;
			} else {
				line_width += glutBitmapWidth(GLUT_BITMAP_HELVETICA_12, *c);
			}
			width = std::max<float>(width, line_width);
			char_count++;
		}

		float scale = zoom < 1.0f ? zoom : 1.0f;

		width = (width + 8.0f) * scale;
		height = (height + 4.0f) * scale;

		float x = tooltip->x + (TILE_SIZE / 2.0f);
		float y = tooltip->y + ((TILE_SIZE / 2.0f) * scale);
		float center = width / 2.0f;
		float space = (7.0f * scale);
		float startx = x - center;
		float endx = x + center;
		float starty = y - (height + space);
		float endy = y - space;

		// 7----0----1
		// |         |
		// 6--5  3--2
		//     \/
		//     4
		float vertexes[9][2] = {
			{x,         starty}, // 0
			{endx,      starty}, // 1
			{endx,      endy},   // 2
			{x + space, endy},   // 3
			{x,         y},      // 4
			{x - space, endy},   // 5
			{startx,    endy},   // 6
			{startx,    starty}, // 7
			{x,         starty}, // 0
		};

		// background
		glColor4ub(tooltip->r, tooltip->g, tooltip->b, 255);
		glBegin(GL_POLYGON);
		for(int i = 0; i < 8; ++i)
			glVertex2f(vertexes[i][0], vertexes[i][1]);
		glEnd();

		// borders
		glColor4ub(0, 0, 0, 255);
		glLineWidth(1.0);
		glBegin(GL_LINES);
		for(int i = 0; i < 8; ++i) {
			glVertex2f(vertexes[i][0], vertexes[i][1]);
			glVertex2f(vertexes[i + 1][0], vertexes[i + 1][1]);
		}
		glEnd();

		// text
		if(zoom <= 1.0) {
			startx += (3.0f * scale);
			starty += (14.0f * scale);
			glColor4ub(0, 0, 0, 255);
			glRasterPos2f(startx, starty);
			char_count = 0;
			for(const char* c = text; *c != '\0'; c++) {
				if(*c == '\n' || (char_count >= MAX_LINE_CHARS && *c == ' ')) {
					starty += (14.0f * scale);
					glRasterPos2f(startx, starty);
					char_count = 0;
				}
				glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *c);
				char_count++;
			}
		}
	}
}

void MapDrawer::MakeTooltip(int screenx, int screeny, const std::string& text, uint8_t r, uint8_t g, uint8_t b)
{
	if(text.empty())
		return;

	MapTooltip *tooltip = newd MapTooltip(screenx, screeny, text, r, g, b);
	tooltip->checkLineEnding();
	tooltips.push_back(tooltip);
}

void MapDrawer::getColor(Brush* brush, const Position& position, uint8_t &r, uint8_t &g, uint8_t &b)
{
	if(brush->canDraw(&editor.map, position)) {
		if(brush->isWaypoint()) {
			r = 0x00; g = 0xff, b = 0x00;
		} else {
			r = 0x00; g = 0x00, b = 0xff;
		}
	} else {
		r = 0xff; g = 0x00, b = 0x00;
	}
}

void MapDrawer::TakeScreenshot(uint8_t* screenshot_buffer)
{
	glFinish(); // Wait for the operation to finish

	glPixelStorei(GL_PACK_ALIGNMENT, 1); // 1 byte alignment

	for(int i = 0; i < screensize_y; ++i)
		glReadPixels(0, screensize_y - i, screensize_x, 1, GL_RGB, GL_UNSIGNED_BYTE, (GLubyte*)(screenshot_buffer) + 3*screensize_x*i);
}

void MapDrawer::glBlitTexture(int sx, int sy, int texture_number, int red, int green, int blue, int alpha)
{
	if(texture_number != 0) {
		glBindTexture(GL_TEXTURE_2D, texture_number);
		glColor4ub(uint8_t(red), uint8_t(green), uint8_t(blue), uint8_t(alpha));
		glBegin(GL_QUADS);
			glTexCoord2f(0.f, 0.f); glVertex2f(sx, sy);
			glTexCoord2f(1.f, 0.f); glVertex2f(sx + TILE_SIZE, sy);
			glTexCoord2f(1.f, 1.f); glVertex2f(sx + TILE_SIZE, sy + TILE_SIZE);
			glTexCoord2f(0.f, 1.f); glVertex2f(sx, sy + TILE_SIZE);
		glEnd();
	}
}

void MapDrawer::glBlitSquare(int sx, int sy, int red, int green, int blue, int alpha)
{
	glColor4ub(uint8_t(red), uint8_t(green), uint8_t(blue), uint8_t(alpha));
	glBegin(GL_QUADS);
		glVertex2f(sx, sy);
		glVertex2f(sx + TILE_SIZE, sy);
		glVertex2f(sx + TILE_SIZE, sy + TILE_SIZE);
		glVertex2f(sx, sy + TILE_SIZE);
	glEnd();
}

void MapDrawer::glColor(wxColor color)
{
	glColor4ub(color.Red(), color.Green(), color.Blue(), color.Alpha());
}

void MapDrawer::glColor(MapDrawer::BrushColor color)
{
	switch(color) {
		case COLOR_BRUSH:
			glColor4ub(
				g_settings.getInteger(Config::CURSOR_RED),
				g_settings.getInteger(Config::CURSOR_GREEN),
				g_settings.getInteger(Config::CURSOR_BLUE),
				g_settings.getInteger(Config::CURSOR_ALPHA)
			);
			break;

		case COLOR_FLAG_BRUSH:
		case COLOR_HOUSE_BRUSH:
			glColor4ub(
				g_settings.getInteger(Config::CURSOR_ALT_RED),
				g_settings.getInteger(Config::CURSOR_ALT_GREEN),
				g_settings.getInteger(Config::CURSOR_ALT_BLUE),
				g_settings.getInteger(Config::CURSOR_ALT_ALPHA)
			);
			break;

		case COLOR_SPAWN_BRUSH:
			glColor4ub(166, 0, 0, 128);
			break;

		case COLOR_ERASER:
			glColor4ub(166, 0, 0, 128);
			break;

		case COLOR_VALID:
			glColor4ub(0, 166, 0, 128);
			break;

		case COLOR_INVALID:
			glColor4ub(166, 0, 0, 128);
			break;

		default:
			glColor4ub(255, 255, 255, 128);
			break;
	}
}

void MapDrawer::glColorCheck(Brush* brush, const Position& pos)
{
	if(brush->canDraw(&editor.map, pos))
		glColor(COLOR_VALID);
	else
		glColor(COLOR_INVALID);
}
