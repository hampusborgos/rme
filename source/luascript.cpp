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

#include "main.h"
#include "luascript.h"
#include "gui.h"
#include "editor.h"
#include "selection.h"
#include "brush.h"
#include "house.h"
#include "items.h"
#include "raw_brush.h"

LuaInterface g_lua;

LuaInterface::LuaInterface()
{
	luaState = nullptr;
}

LuaInterface::~LuaInterface()
{
	luaState = nullptr;
}

bool LuaInterface::init()
{
	luaState = luaL_newstate();
	if (!luaState) {
		return false;
	}

	luaL_openlibs(luaState);
	registerFunctions();
	return true;
}

int LuaInterface::runScript(const wxString& script)
{
	if(script.empty()) {
		lua_pushnil(luaState);
		return 0;
	}

	// loads file the script at stack top
	int ret = luaL_loadbuffer(luaState, script.c_str(), script.length(), "line");
	if(ret != 0) {
		luaErrorHandler(luaState);
		return 0;
	}

	// check that it is loaded as a function
	if(!isFunction(luaState, -1)) {
		return 0;
	}

	// execute it
	ret = protectedCall(luaState, 0, 0);
	if(ret != 0) {
		luaErrorHandler(luaState);
		return 0;
	}

	return 1;
}

void LuaInterface::print(const wxString& text)
{
	if(!text.empty()) {
		ScriptingWindow* window = g_gui.ShowScriptingWindow();
		window->AppendLog(text);
	}
}

void LuaInterface::pushString(lua_State* L, const wxString& value)
{
	lua_pushlstring(L, value.c_str(), value.length());
}

// Push
void LuaInterface::pushBoolean(lua_State* L, bool value)
{
	lua_pushboolean(L, value ? 1 : 0);
}

void LuaInterface::pushPosition(lua_State* L, const Position& position)
{
	lua_createtable(L, 0, 3);

	setField(L, "x", position.x);
	setField(L, "y", position.y);
	setField(L, "z", position.z);
	setMetatable(L, -1, "Position");
}

// Metatables
void LuaInterface::setMetatable(lua_State* L, int32_t index, const std::string& name)
{
	luaL_getmetatable(L, name.c_str());
	lua_setmetatable(L, index - 1);
}

wxString LuaInterface::popString(lua_State* L)
{
	if(lua_gettop(L) == 0) {
		return wxEmptyString;
	}

	wxString str(getString(L, -1));
	lua_pop(L, 1);
	return str;
}

wxString LuaInterface::getString(lua_State* L, int32_t arg)
{
	size_t len;
	const char* c_str = lua_tolstring(L, arg, &len);
	if(!c_str || len == 0) {
		return wxEmptyString;
	}
	return wxString(c_str, len);
}

int LuaInterface::getTop()
{
	return lua_gettop(luaState);
}

Position LuaInterface::getPosition(lua_State* L, int32_t arg)
{
	Position position;
	position.x = getField<uint16_t>(L, arg, "x");
	position.y = getField<uint16_t>(L, arg, "y");
	position.z = getField<uint8_t>(L, arg, "z");

	lua_pop(L, 3);
	return position;
}

// Userdata
int LuaInterface::luaUserdataCompare(lua_State* L)
{
	// userdataA == userdataB
	pushBoolean(L, getUserdata<void>(L, 1) == getUserdata<void>(L, 2));
	return 1;
}

// print
int LuaInterface::luaPrint(lua_State* L)
{
	wxString text = wxEmptyString;
	if (isString(L, 1)) {
		text = getString(L, 1);
	} else if(isNumber(L, 1)) {
		int value = getNumber<int>(L, 1);
		text = i2ws(value);
	} else if(isBoolean(L, 1)) {
		bool value = getBoolean(L, 1);
		text = value == 0 ? "false": "true";
	}
	ScriptingWindow* window = g_gui.ShowScriptingWindow();
	if(window) {
		window->AppendLog(text);
	}
	return 1;
}

// Gui
int LuaInterface::luaGuiNewMap(lua_State* L)
{
	// g_gui.newMap()
	pushBoolean(L, g_gui.NewMap());
	return 1;
}

int LuaInterface::luaGuiLoadMap(lua_State* L)
{
	// g_gui.loadMap(path)
	FileName path = getString(L, 1);
	pushBoolean(L, g_gui.LoadMap(path));
	return 1;
}

int LuaInterface::luaGuiShouldSaveMap(lua_State* L)
{
	// g_gui.shouldSaveCurrentMap()
	pushBoolean(L, g_gui.ShouldSave());
	return 1;
}

int LuaInterface::luaGuiSaveCurrentMap(lua_State* L)
{
	// g_gui.saveCurrentMap()
	g_gui.SaveCurrentMap();
	pushBoolean(L, true);
	return 1;
}

int LuaInterface::luaGuiCanUndo(lua_State* L)
{
	// g_gui.canUndo()
	pushBoolean(L, g_gui.CanUndo());
	return 1;
}

int LuaInterface::luaGuiCanRedo(lua_State* L)
{
	// g_gui.canRedo()
	pushBoolean(L, g_gui.CanRedo());
	return 1;
}

int LuaInterface::luaGuiDoUndo(lua_State* L)
{
	// g_gui.doUndo()
	pushBoolean(L, g_gui.DoUndo());
	return 1;
}

int LuaInterface::luaGuiDoRedo(lua_State* L)
{
	// g_gui.doRedo()
	pushBoolean(L, g_gui.DoRedo());
	return 1;
}

int LuaInterface::luaGuiGetEditorAt(lua_State* L)
{
	if (isNumber(L, 1)) {
		int index = getNumber<int>(L, 1);
		Editor* editor = g_gui.GetEditorAt(index);
		if(editor) {
			pushUserdata<Editor>(L, editor);
			setMetatable(L, -1, "Editor");
			return 1;
		}
	}

	lua_pushnil(L);
	return 1;
}

int LuaInterface::luaGuiGetCurrentEditor(lua_State* L)
{
	Editor* editor = g_gui.GetCurrentEditor();
	if(editor) {
		pushUserdata<Editor>(L, editor);
		setMetatable(L, -1, "Editor");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int LuaInterface::luaGuiSetCurrentEditor(lua_State* L)
{
	if(g_gui.IsEditorOpen()) {
		if(isUserdata(L, 1)) {
			Editor* editor = getUserdata<Editor>(L, 1);
			if(editor) {
				g_gui.SetCurrentEditor(editor);
				pushBoolean(L, true);
				return 1;
			}
		} else if(isNumber(L, 1)) {
			int index = getNumber<int>(L, 1);
			g_gui.SetCurrentEditor(index);
			pushBoolean(L, true);
			return 1;
		}
	}

	pushBoolean(L, false);
	return 1;
}

int LuaInterface::luaGuiGetCurrentFloor(lua_State* L)
{
	// g_gui.getCurrentFloor()
	int floor = g_gui.GetCurrentFloor();
	lua_pushnumber(L, floor);
	return 1;
}

int LuaInterface::luaGuiSetCurrentFloor(lua_State* L)
{
	// g_gui.changeFloor(floor)
	int floor = getNumber<int>(L, 1);
	if(floor >= 0 && floor <= MAP_MAX_LAYER) {
		g_gui.ChangeFloor(floor);
		pushBoolean(L, true);
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaGuiGetCenterPosition(lua_State* L)
{
	// g_gui.getCenterPosition()
	MapTab* mapTab = g_gui.GetCurrentMapTab();
	if(mapTab) {
		pushPosition(L, mapTab->GetScreenCenterPosition());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int LuaInterface::luaGuiSetCenterPosition(lua_State* L)
{
	// g_gui.setCenterPosition(position)
	const Position& position = getPosition(L, 1);
	pushBoolean(L, g_gui.SetScreenCenterPosition(position));
	return 1;
}

int LuaInterface::luaGuiSetSelectionMode(lua_State* L)
{
	// g_gui.setSelectionMode()
	if(g_gui.IsEditorOpen()) {
		g_gui.SetSelectionMode();
		pushBoolean(L, true);
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaGuiSetDrawingMode(lua_State* L)
{
	// g_gui.setDrawingMode()
	if(g_gui.IsEditorOpen()) {
		g_gui.SetDrawingMode();
		pushBoolean(L, true);
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaGuiSetStatusText(lua_State* L)
{
	// g_gui.setStausText(text)
	const wxString& text = getString(L, 1);
	g_gui.SetStatusText(text);
	return 1;
}

int LuaInterface::luaGuiShowTextBox(lua_State* L)
{
	// g_gui.showTextBox(title, text)
	const wxString& text = getString(L, 2);
	const wxString& title = getString(L, 1);
	g_gui.ShowTextBox(g_gui.root, title, text);
	return 1;
}

int LuaInterface::luaGuiGetExecDirectory(lua_State* L)
{
	// g_gui.getExecDirectory()
	pushString(L, GUI::GetExecDirectory());
	return 1;
}

int LuaInterface::luaGuiGetDataDirectory(lua_State* L)
{
	// g_gui.getDataDirectory()
	pushString(L, GUI::GetDataDirectory());
	return 1;
}

int LuaInterface::luaGuiGetLocalDataDirectory(lua_State* L)
{
	// g_gui.getLocalDataDirectory()
	pushString(L, GUI::GetLocalDataDirectory());
	return 1;
}

int LuaInterface::luaGuiGetLocalDirectory(lua_State* L)
{
	// g_gui.getLocalDirectory()
	pushString(L, GUI::GetLocalDirectory());
	return 1;
}

int LuaInterface::luaGuiGetExtensionsDirectory(lua_State* L)
{
	// g_gui.getExtensionsDirectory()
	pushString(L, GUI::GetExtensionsDirectory());
	return 1;
}

// Editor
int LuaInterface::luaEditorCreate(lua_State* L)
{
	// Editor(index)
	int index = getNumber<int>(L, 2);
	Editor* editor = g_gui.GetEditorAt(index);
	if(editor) {
		pushUserdata<Editor>(L, editor);
		setMetatable(L, -1, "Editor");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int LuaInterface::luaEditorAddItem(lua_State* L)
{
	// editor:addItem(position, itemId)
	Editor* editor = getUserdata<Editor>(L, 1);
	if(!editor) {
		pushBoolean(L, false);
		return 1;
	}

	if(isTable(L, 2)) {
		uint16_t itemId = getNumber<int32_t>(L, 3);
		const Position& position = getPosition(L, 2);
		TileLocation* location = editor->map.createTileL(position);
		pushBoolean(L, useRawBrush(editor, location, itemId, true));
		return 1;
	}

	pushBoolean(L, false);
	return 1;
}

int LuaInterface::luaEditorGetMapDescription(lua_State* L)
{
	// editor:getMapDescription()
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor) {
		pushString(L, editor->map.getMapDescription());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int LuaInterface::luaEditorSetMapDescription(lua_State* L)
{
	// editor:setMapDescription(newDescription)
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor) {
		const wxString& description = getString(L, 2);
		editor->map.setMapDescription(std::string(description.mb_str()));
		pushBoolean(L, true);
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaEditorGetMapWidth(lua_State* L)
{
	// editor:getMapWidth()
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor) {
		lua_pushnumber(L, editor->map.getWidth());
	} else {
		lua_pushnumber(L, 0);
	}
	return 1;
}

int LuaInterface::luaEditorSetMapWidth(lua_State* L)
{
	// editor:setMapWidth(newWidth)
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor) {
		int32_t width = getNumber<int32_t>(L, 2);
		width = std::max<int32_t>(MAP_MIN_WIDTH, std::min<int32_t>(MAP_MAX_WIDTH, width));
		if(width != editor->map.getWidth()) {
			MapTab* mapTab = g_gui.GetMapTab(editor);
			if(mapTab) {
				editor->map.setWidth(width);
				g_gui.FitViewToMap(mapTab);
				pushBoolean(L, true);
				return 1;
			}
		}
	}

	pushBoolean(L, false);
	return 1;
}

int LuaInterface::luaEditorGetMapHeight(lua_State* L)
{
	// editor:getMapHeight()
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor) {
		lua_pushnumber(L, editor->map.getHeight());
	} else {
		lua_pushnumber(L, 0);
	}
	return 1;
}

int LuaInterface::luaEditorSetMapHeight(lua_State* L)
{
	// editor:setMapHeight(newHeight)
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor) {
		int32_t height = getNumber<int32_t>(L, 2);
		height = std::max<int32_t>(MAP_MIN_HEIGHT, std::min<int32_t>(MAP_MAX_HEIGHT, height));
		if(height != editor->map.getHeight()) {
			MapTab* mapTab = g_gui.GetMapTab(editor);
			if(mapTab) {
				editor->map.setHeight(height);
				g_gui.FitViewToMap(mapTab);
				pushBoolean(L, true);
				return 1;
			}
		}
	}

	pushBoolean(L, false);
	return 1;
}

int LuaInterface::luaEditorGetTile(lua_State* L)
{
	// editor:getTile(x, y, z)
	// editor:getTile(position)
	Editor* editor = getUserdata<Editor>(L, 1);
	if(!editor) {
		lua_pushnil(L);
		return 1;
	}

	Map& map = editor->map;
	Tile* tile;

	if(isTable(L, 2)) {
		const Position& position = getPosition(L, 2);
		tile = map.getTile(position);
	} else {
		uint8_t z = getNumber<uint8_t>(L, 4);
		uint16_t y = getNumber<uint16_t>(L, 3);
		uint16_t x = getNumber<uint16_t>(L, 2);
		tile = map.getTile(x, y, z);
	}

	if(tile) {
		pushUserdata<Tile>(L, tile);
		setMetatable(L, -1, "Tile");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int LuaInterface::luaEditorCreateSelection(lua_State* L)
{
	// editor:createSelection(start, end)
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor) {
		g_gui.SetSelectionMode();
		const Position& start = getPosition(L, 2);
		const Position& end = getPosition(L, 3);
		if(editor->createSelection(start, end)) {
			g_gui.RefreshView();
			pushUserdata<Editor>(L, editor);
			setMetatable(L, -1, "Selection");
			return 1;
		}
	}

	lua_pushnil(L);
	return 1;
}

int LuaInterface::luaEditorSelectTiles(lua_State* L)
{
	// editor:selectTiles(tiles)
	Editor* editor = getUserdata<Editor>(L, 1);
	if(!editor) {
		lua_pushnil(L);
		return 1;
	}

	lua_pushnil(L);
	g_gui.SetSelectionMode();

	// remove any current selection.
	editor->selection.start();
	editor->selection.clear();
	editor->selection.finish();
	editor->selection.updateSelectionCount();

	PositionVector positions;
	while(lua_next(L, -2) != 0) {
		Tile* tile = getUserdata<Tile>(L, -1);
		if(tile) {
			positions.push_back(tile->getPosition());
		}
		lua_pop(L, 1);
	}

	if(editor->createSelection(positions)) {
		g_gui.RefreshView();
		pushUserdata<Editor>(L, editor);
		setMetatable(L, -1, "Selection");
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

int LuaInterface::luaEditorGetSelection(lua_State* L)
{
	// editor:getSelection()
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor) {
		pushUserdata<Editor>(L, editor);
		setMetatable(L, -1, "Selection");
		return 1;
	}

	lua_pushnil(L);
	return 1;
}

int LuaInterface::luaEditorGetItemCount(lua_State* L)
{
	// editor:getItemCount(itemid)
	Editor* editor = getUserdata<Editor>(L, 1);
	if(!editor) {
		lua_pushnumber(L, 0);
		return 1;
	}

	uint16_t itemid = getNumber<uint16_t>(L, 2);
	ItemCounter counter(itemid);
	foreach_ItemOnMap(editor->map, counter, false);
	lua_pushnumber(L, counter.result);
	return 1;
}

int LuaInterface::luaEditorReplaceItems(lua_State* L)
{
	// editor:replaceItems(items)
	Editor* editor = getUserdata<Editor>(L, 1);
	if(!editor) {
		pushBoolean(L, false);
		return 1;
	}

	lua_pushnil(L);

	std::map<uint32_t, uint32_t> items;
	while(lua_next(L, -2) != 0) {
		uint32_t key = (uint32_t)lua_tonumber(L, -2);
		uint32_t value = (uint32_t)lua_tonumber(L, -1);
		if(g_items.hasItemId(key) && g_items.hasItemId(value))
			items.insert(std::pair<uint32_t, uint32_t>(key, value));
		lua_pop(L, 1);
	}

	if(items.empty()) {
		g_lua.print("No items were found on the map.");
		pushBoolean(L, false);
		return 1;
	}

	if(!replaceItems(editor, items)) {
		pushBoolean(L, false);
		return 1;
	}

	g_gui.RefreshView();

	pushBoolean(L, true);
	return 1;
}

int LuaInterface::luaEditorGetHouses(lua_State* L)
{
	// editor:getHouses()
	Editor* editor = getUserdata<Editor>(L, 1);
	if(!editor) {
		lua_pushnil(L);
		return 1;
	}

	Houses& houses = editor->map.houses;
	lua_createtable(L, houses.count(), 0);

	int index = 0;
	for(HouseMap::iterator it = houses.begin(); it != houses.end(); ++it) {
		House* house = it->second;
		if(house) {
			pushUserdata<House>(L, house);
			setMetatable(L, -1, "House");
			lua_rawseti(L, -2, ++index);
		}
	}
	return 1;
}

int LuaInterface::luaEditorGetTowns(lua_State* L)
{
	// editor:getTowns()
	Editor* editor = getUserdata<Editor>(L, 1);
	if(!editor) {
		lua_pushnil(L);
		return 1;
	}

	Towns& towns = editor->map.towns;
	lua_createtable(L, towns.count(), 0);

	int index = 0;
	for(TownMap::iterator it = towns.begin(); it != towns.end(); ++it) {
		Town* town = it->second;
		if(town) {
			pushUserdata<Town>(L, town);
			setMetatable(L, -1, "Town");
			lua_rawseti(L, -2, ++index);
		}
	}
	return 1;
}

// Tile
int LuaInterface::luaTileCreate(lua_State* L)
{
	// Tile(x, y, z)
	// Tile(position)
	Editor* editor = g_gui.GetCurrentEditor();
	if(!editor) {
		lua_pushnil(L);
		return 1;
	}

	Map& map = editor->map;

	Tile* tile;
	if(isTable(L, 2)) {
		const Position& position = getPosition(L, 2);
		tile = map.getTile(position);
	} else {
		uint8_t z = getNumber<uint8_t>(L, 4);
		uint16_t y = getNumber<uint16_t>(L, 3);
		uint16_t x = getNumber<uint16_t>(L, 2);
		tile = map.getTile(x, y, z);
	}

	if(tile) {
		pushUserdata<Tile>(L, tile);
		setMetatable(L, -1, "Tile");
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int LuaInterface::luaTileGetPosition(lua_State* L)
{
	// tile:getPosition()
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		const Position position = tile->getPosition();
		pushPosition(L, position);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int LuaInterface::luaTileIsHookEast(lua_State* L)
{
	// tile:isHookEast()
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		pushBoolean(L, tile->hasProperty(HOOK_EAST));
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaTileIsHookSouth(lua_State* L)
{
	// tile:isHookSouth()
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		pushBoolean(L, tile->hasProperty(HOOK_SOUTH));
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaTileIsHouse(lua_State* L)
{
	// tile:isHouse()
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		pushBoolean(L, tile->isHouseTile());
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaTileIsHouseExit(lua_State* L)
{
	// tile:isHouseExit()
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		pushBoolean(L, tile->isHouseExit());
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaTileIsPvP(lua_State* L)
{
	// tile:isPvP()
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		pushBoolean(L, (tile->getMapFlags() & TILESTATE_PVPZONE) == TILESTATE_PVPZONE);
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaTileIsNoPvP(lua_State* L)
{
	// tile:isNoPvP()
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		pushBoolean(L, (tile->getMapFlags() & TILESTATE_NOPVP) == TILESTATE_NOPVP);
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaTileIsNoLogout(lua_State* L)
{
	// tile:isNoLogout()
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		pushBoolean(L, (tile->getMapFlags() & TILESTATE_NOLOGOUT) == TILESTATE_NOLOGOUT);
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaTileIsPZ(lua_State* L)
{
	// tile:isPZ()
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		pushBoolean(L, tile->isPZ());
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaTileIsBlocking(lua_State* L)
{
	// tile:isBlocking()
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		pushBoolean(L, tile->isBlocking());
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaTileIsSelected(lua_State* L)
{
	// tile:isSelected()
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		pushBoolean(L, tile->isSelected());
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaTileIsModified(lua_State* L)
{
	// tile:isModified()
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		pushBoolean(L, tile->isModified());
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaTileSetPvP(lua_State* L)
{
	// tile:setPvP(enable)
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		bool enable = getBoolean(L, 2);
		if(setTileFlag(tile, TILESTATE_PVPZONE, enable)) {
			pushBoolean(L, true);
			return 1;
		}
	}

	pushBoolean(L, false);
	return 1;
}

int LuaInterface::luaTileSetNoPvP(lua_State* L)
{
	// tile:setNoPvP(enable)
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		bool enable = getBoolean(L, 2);
		if(setTileFlag(tile, TILESTATE_NOPVP, enable)) {
			pushBoolean(L, true);
			return 1;
		}
	}

	pushBoolean(L, false);
	return 1;
}

int LuaInterface::luaTileSetNoLogout(lua_State* L)
{
	// tile:setNoLogout(enable)
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		bool enable = getBoolean(L, 2);
		if(setTileFlag(tile, TILESTATE_NOLOGOUT, enable)) {
			pushBoolean(L, true);
			return 1;
		}
	}

	pushBoolean(L, false);
	return 1;
}

int LuaInterface::luaTileSetPZ(lua_State* L)
{
	// tile:setPZ(enable)
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		bool enable = getBoolean(L, 2);
		if(setTileFlag(tile, TILESTATE_PROTECTIONZONE, enable)) {
			pushBoolean(L, true);
			return 1;
		}
	}

	pushBoolean(L, false);
	return 1;
}

int LuaInterface::luaTileHasItemId(lua_State* L)
{
	// tile:hasItemId(itemid)
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		uint16_t itemId = getNumber<uint16_t>(L, 2);
		pushBoolean(L, tile->hasItemId(itemId));
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaTileHasWall(lua_State* L)
{
	// tile:hasWall()
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		pushBoolean(L, tile->hasWall());
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaSelectionCreate(lua_State* L)
{
	// Selection(editor, start, end)
	Editor* editor = getUserdata<Editor>(L, 2);
	if(editor) {
		g_gui.SetSelectionMode();
		const Position& start = getPosition(L, 3);
		const Position& end = getPosition(L, 4);
		if(editor->createSelection(start, end)) {
			g_gui.RefreshView();
			pushUserdata<Editor>(L, editor);
			setMetatable(L, -1, "Selection");
			return 1;
		}
	}

	lua_pushnil(L);
	return 1;
}

int LuaInterface::luaSelectionGetTiles(lua_State* L)
{
	// selection:getTiles()
	Editor* editor = getUserdata<Editor>(L, 1);
	if(!editor) {
		lua_pushnil(L);
		return 1;
	}

	TileVector& tiles = editor->selection.getTiles();
	lua_createtable(L, tiles.size(), 0);

	int index = 0;
	for(Tile* tile : tiles) {
		pushUserdata<Tile>(L, tile);
		setMetatable(L, -1, "Tile");
		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

int LuaInterface::luaSelectionGetTileCount(lua_State* L)
{
	// selection:getTileCount()
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor) {
		lua_pushnumber(L, editor->selection.size());
	} else {
		lua_pushnumber(L, 0);
	}
	return 1;
}

int LuaInterface::luaSelectionGetMinPosition(lua_State* L)
{
	// selection:getMinPosition()
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor && editor->hasSelection()) {
		const Position& position = editor->selection.minPosition();
		pushPosition(L, position);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int LuaInterface::luaSelectionGetMaxPosition(lua_State* L)
{
	// selection:getMaxPosition()
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor && editor->hasSelection()) {
		const Position& position = editor->selection.maxPosition();
		pushPosition(L, position);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int LuaInterface::luaSelectionMove(lua_State* L)
{
	// selection:move(position)
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor && editor->hasSelection()) {
		const Position& position = getPosition(L, 2);
		const Position& minPosition = editor->selection.minPosition();
		Position offset(position - minPosition);
		offset.x = -offset.x;
		offset.y = -offset.y;
		offset.z = -offset.z;
		if(editor->moveSelection(offset)) {
			g_gui.RefreshView();
			pushBoolean(L, true);
			return 1;
		}
	}

	pushBoolean(L, false);
	return 1;
}

int LuaInterface::luaSelectionOffset(lua_State* L)
{
	// selection:offset(x[, y = 0[, z = 0]])
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor && editor->hasSelection()) {
		Position offset;
		offset.x = getNumber<int>(L, 2);
		offset.y = lua_gettop(L) > 2 ? getNumber<int>(L, 3) : 0;
		offset.z = lua_gettop(L) > 3 ? getNumber<int>(L, 4) : 0;
		if(editor->moveSelection(offset)) {
			g_gui.RefreshView();
			pushBoolean(L, true);
			return 1;
		}
	}

	pushBoolean(L, false);
	return 1;
}

int LuaInterface::luaSelectionBorderize(lua_State* L)
{
	// selection:borderize()
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor && editor->hasSelection()) {
		if(editor->borderizeSelection()) {
			g_gui.RefreshView();
			pushBoolean(L, true);
			return 1;
		}
	}

	pushBoolean(L, false);
	return 1;
}

int LuaInterface::luaSelectionRandomize(lua_State* L)
{
	// selection:randomize()
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor && editor->hasSelection()) {
		if(editor->randomizeSelection()) {
			g_gui.RefreshView();
			pushBoolean(L, true);
			return 1;
		}
	}

	pushBoolean(L, false);
	return 1;
}

int LuaInterface::luaSelectionSaveAsMinimap(lua_State* L)
{
	// selection:saveAsMinimap(directory, fileName)
	Editor* editor = getUserdata<Editor>(L, 1);
	if(!editor || !editor->hasSelection()) {
		pushBoolean(L, false);
		return 1;
	}

	const wxString& path = getString(L, 2);
	const wxFileName& directory = resolvePath(path);
	if(!directory.IsDirWritable()) {
		g_lua.print(wxT("[selection:saveAsMinimap]: The provided directory is not writable."));
		pushBoolean(L, false);
		return 1;
	}

	const wxString& fileName = getString(L, 3);
	if(fileName.empty()) {
		g_lua.print(wxT("[selection:saveAsMinimap]: The parameter 'fileName' must be non-nil and non-empty."));
		pushBoolean(L, false);
		return 1;
	}

	int min_x = MAP_MAX_WIDTH + 1;
	int min_y = MAP_MAX_HEIGHT + 1;
	int min_z = MAP_MAX_LAYER + 1;
	int max_x = 0;
	int max_y = 0;
	int max_z = 0;

	const TileVector& tiles = editor->selection.getTiles();
	for(Tile* tile : tiles) {
		if(tile->empty())
			continue;

		Position pos = tile->getPosition();

		if(pos.x < min_x)
			min_x = pos.x;

		if(pos.y < min_y)
			min_y = pos.y;

		if(pos.z < min_z)
			min_z = pos.z;

		if(pos.x > max_x)
			max_x = pos.x;

		if(pos.y > max_y)
			max_y = pos.y;

		if(pos.z > max_z)
			max_z = pos.z;
	}

	int numtiles = (max_x - min_x) * (max_y - min_y);
	int minimap_width = max_x - min_x + 1;
	int minimap_height = max_y - min_y + 1;

	if(numtiles == 0 || minimap_width > 2048 || minimap_height > 2048) {
		g_lua.print(wxT("[selection:saveAsMinimap]: Tile count == 0 or minimap size greater than 2048px"));
		pushBoolean(L, false);
		return 1;
	}

	int tiles_iterated = 0;

	for(int z = min_z; z <= max_z; z++) {
		uint8_t* pixels = newd uint8_t[minimap_width * minimap_height * 3]; // 3 bytes per pixel
		memset(pixels, 0, minimap_width * minimap_height * 3);

		for(Tile* tile : tiles) {
			if(tile->getZ() != z)
				continue;

			++tiles_iterated;
			if (tiles_iterated % 8192 == 0)
				g_gui.SetLoadDone(int(tiles_iterated / double(tiles.size()) * 90.0));

			if (tile->empty())
				continue;

			uint8_t color = 0;

			for(Item* item : tile->items) {
				if(item->getMiniMapColor() != 0) {
					color = item->getMiniMapColor();
					break;
				}
			}

			if(color == 0 && tile->hasGround())
				color = tile->ground->getMiniMapColor();

			uint32_t index = ((tile->getY() - min_y) * minimap_width + (tile->getX() - min_x)) * 3;

			pixels[index] = (uint8_t)(int(color / 36) % 6 * 51);     // red
			pixels[index + 1] = (uint8_t)(int(color / 6) % 6 * 51);  // green
			pixels[index + 2] = (uint8_t)(color % 6 * 51);           // blue
		}

		wxString filePath = directory.GetPath(wxPATH_GET_VOLUME | wxPATH_GET_SEPARATOR) + fileName + wxT("_") + i2ws(z) + wxT(".png");
		wxImage* image = newd wxImage(minimap_width, minimap_height, pixels, true);
		image->SaveFile(filePath, wxBITMAP_TYPE_PNG);
		image->Destroy();
		delete[] pixels;
	}

	pushBoolean(L, true);
	return 1;
}

int LuaInterface::luaEditorRemoveItem(lua_State* L)
{
	// editor:removeItem(position, itemId)
	Editor* editor = getUserdata<Editor>(L, 1);
	if(!editor) {
		pushBoolean(L, false);
		return 1;
	}

	if(isTable(L, 2)) {
		uint16_t itemId = getNumber<int32_t>(L, 3);
		const Position& position = getPosition(L, 2);
		TileLocation* location = editor->map.createTileL(position);
		pushBoolean(L, useRawBrush(editor, location, itemId, false));
		return 1;
	}

	pushBoolean(L, false);
	return 1;
}

int LuaInterface::luaSelectionReplaceItems(lua_State* L)
{
	// selection:replaceItems(items)
	Editor* editor = getUserdata<Editor>(L, 1);
	if(!editor || !editor->hasSelection()) {
		pushBoolean(L, false);
		return 1;
	}

	lua_pushnil(L);

	std::map<uint32_t, uint32_t> items;
	while(lua_next(L, -2) != 0) {
		uint32_t key = (uint32_t)lua_tonumber(L, -2);
		uint32_t value = (uint32_t)lua_tonumber(L, -1);
		if(g_items.hasItemId(key) && g_items.hasItemId(value))
			items.insert(std::pair<uint32_t, uint32_t>(key, value));
		lua_pop(L, 1);
	}

	if(items.empty()) {
		g_lua.print("No items were found on the map.");
		pushBoolean(L, false);
		return 1;
	}

	if(!replaceItems(editor, items, true)) {
		pushBoolean(L, false);
		return 1;
	}

	g_gui.RefreshView();

	pushBoolean(L, true);
	return 1;
}

int LuaInterface::luaSelectionDestroy(lua_State* L)
{
	// selection:destroy()
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor && editor->hasSelection()) {
		if(editor->destroySelection()) {
			g_gui.RefreshView();
			pushBoolean(L, true);
			return 1;
		}
	}

	pushBoolean(L, false);
	return 1;
}

int LuaInterface::luaHouseCreate(lua_State* L)
{
	// House(editor, houseId)
	Editor* editor = getUserdata<Editor>(L, 2);
	if(editor) {
		uint32_t houseId = (uint32_t)lua_tonumber(L, 3);
		House* house = editor->map.getHouse(houseId);
		if(house) {
			pushUserdata<House>(L, house);
			setMetatable(L, -1, "House");
			return 1;
		}
	}

	lua_pushnil(L);
	return 1;
}

int LuaInterface::luaHouseGetId(lua_State* L)
{
	// house:getId()
	House* house = getUserdata<House>(L, 1);
	if(house) {
		lua_pushnumber(L, house->getId());
	} else {
		lua_pushnumber(L, 0);
	}
	return 1;
}

int LuaInterface::luaHouseGetName(lua_State* L)
{
	// house:getName()
	House* house = getUserdata<House>(L, 1);
	if(house) {
		pushString(L, house->getName());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int LuaInterface::luaHouseGetTownId(lua_State* L)
{
	// house:getTownId()
	House* house = getUserdata<House>(L, 1);
	if(house) {
		lua_pushnumber(L, house->getTownId());
	} else {
		lua_pushnumber(L, 0);
	}
	return 1;
}

int LuaInterface::luaHouseGetSize(lua_State* L)
{
	// house:getSize()
	House* house = getUserdata<House>(L, 1);
	if(house) {
		lua_pushnumber(L, house->size());
	} else {
		lua_pushnumber(L, 0);
	}
	return 1;
}

int LuaInterface::luaHouseGetTiles(lua_State* L)
{
	// house:getTiles()
	House* house = getUserdata<House>(L, 1);
	if(!house) {
		lua_pushnil(L);
		return 1;
	}

	TileVector& tiles = house->getTiles();
	lua_createtable(L, tiles.size(), 0);

	int index = 0;
	for(Tile* tile : tiles) {
		pushUserdata<Tile>(L, tile);
		setMetatable(L, -1, "Tile");
		lua_rawseti(L, -2, ++index);
	}
	return 1;
}

// Town
int LuaInterface::luaTownCreate(lua_State* L)
{
	// Town(editor, townId)
	Editor* editor = getUserdata<Editor>(L, 2);
	if(editor) {
		uint32_t townId = (uint32_t)lua_tonumber(L, 3);
		Town* town = editor->map.getTown(townId);
		if(town) {
			pushUserdata<Town>(L, town);
			setMetatable(L, -1, "Town");
			return 1;
		}
	}

	lua_pushnil(L);
	return 1;
}

int LuaInterface::luaTownGetId(lua_State* L)
{
	// town:getId()
	Town* town = getUserdata<Town>(L, 1);
	if(town) {
		lua_pushnumber(L, town->getID());
	} else {
		lua_pushnumber(L, 0);
	}
	return 1;
}

int LuaInterface::luaTownGetName(lua_State* L)
{
	// town:getName()
	Town* town = getUserdata<Town>(L, 1);
	if(town) {
		pushString(L, town->getName());
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int LuaInterface::luaTownGetTemplePosition(lua_State* L)
{
	// town:getTemplePosition()
	Town* town = getUserdata<Town>(L, 1);
	if(town) {
		pushPosition(L, town->getTemplePosition());
	} else {
		lua_pushnumber(L, 0);
	}
	return 1;
}

void LuaInterface::registerClass(const std::string& className, const std::string& baseClass, lua_CFunction newFunction/* = nullptr*/)
{
	// className = {}
	lua_newtable(luaState);
	lua_pushvalue(luaState, -1);
	lua_setglobal(luaState, className.c_str());
	int methods = lua_gettop(luaState);

	// methodsTable = {}
	lua_newtable(luaState);
	int methodsTable = lua_gettop(luaState);

	if(newFunction) {
		// className.__call = newFunction
		lua_pushcfunction(luaState, newFunction);
		lua_setfield(luaState, methodsTable, "__call");
	}

	uint32_t parents = 0;
	if(!baseClass.empty()) {
		lua_getglobal(luaState, baseClass.c_str());
		lua_rawgeti(luaState, -1, 'p');
		parents = getNumber<uint32_t>(luaState, -1) + 1;
		lua_pop(luaState, 1);
		lua_setfield(luaState, methodsTable, "__index");
	}

	// setmetatable(className, methodsTable)
	lua_setmetatable(luaState, methods);

	// className.metatable = {}
	luaL_newmetatable(luaState, className.c_str());
	int metatable = lua_gettop(luaState);

	// className.metatable.__metatable = className
	lua_pushvalue(luaState, methods);
	lua_setfield(luaState, metatable, "__metatable");

	// className.metatable.__index = className
	lua_pushvalue(luaState, methods);
	lua_setfield(luaState, metatable, "__index");

	// className.metatable['h'] = hash
	lua_pushnumber(luaState, std::hash<std::string>()(className));
	lua_rawseti(luaState, metatable, 'h');

	// className.metatable['p'] = parents
	lua_pushnumber(luaState, parents);
	lua_rawseti(luaState, metatable, 'p');

	// className.metatable['t'] = type
	if(className == "Editor") {
		lua_pushnumber(luaState, LuaData_Editor);
	} else if(className == "Tile") {
		lua_pushnumber(luaState, LuaData_Tile);
	} else if(className == "Selection") {
		lua_pushnumber(luaState, LuaData_Selection);
	} else if(className == "House") {
		lua_pushnumber(luaState, LuaData_House);
	} else if(className == "Town") {
		lua_pushnumber(luaState, LuaData_Town);
	} else {
		lua_pushnumber(luaState, LuaData_Unknown);
	}
	lua_rawseti(luaState, metatable, 't');

	// pop className, className.metatable
	lua_pop(luaState, 2);
}

void LuaInterface::registerTable(const std::string& tableName)
{
	// _G[tableName] = {}
	lua_newtable(luaState);
	lua_setglobal(luaState, tableName.c_str());
}

void LuaInterface::registerMethod(const std::string& globalName, const std::string& methodName, lua_CFunction func)
{
	// globalName.methodName = func
	lua_getglobal(luaState, globalName.c_str());
	lua_pushcfunction(luaState, func);
	lua_setfield(luaState, -2, methodName.c_str());

	// pop globalName
	lua_pop(luaState, 1);
}

void LuaInterface::registerMetaMethod(const std::string& className, const std::string& methodName, lua_CFunction func)
{
	// className.metatable.methodName = func
	luaL_getmetatable(luaState, className.c_str());
	lua_pushcfunction(luaState, func);
	lua_setfield(luaState, -2, methodName.c_str());

	// pop className.metatable
	lua_pop(luaState, 1);
}

void LuaInterface::registerGlobalMethod(const std::string& functionName, lua_CFunction func)
{
	// _G[functionName] = func
	lua_pushcfunction(luaState, func);
	lua_setglobal(luaState, functionName.c_str());
}

void LuaInterface::registerFunctions()
{
	// print
	registerGlobalMethod("print", LuaInterface::luaPrint);

	// g_gui
	registerTable("g_gui");
	registerMethod("g_gui", "newMap", LuaInterface::luaGuiNewMap);
	registerMethod("g_gui", "loadMap", LuaInterface::luaGuiLoadMap);
	registerMethod("g_gui", "shouldSaveCurrentMap", LuaInterface::luaGuiShouldSaveMap);
	registerMethod("g_gui", "saveCurrentMap", LuaInterface::luaGuiSaveCurrentMap);
	registerMethod("g_gui", "canUndo", LuaInterface::luaGuiCanUndo);
	registerMethod("g_gui", "canRedo", LuaInterface::luaGuiCanRedo);
	registerMethod("g_gui", "doUndo", LuaInterface::luaGuiDoUndo);
	registerMethod("g_gui", "doRedo", LuaInterface::luaGuiDoRedo);
	registerMethod("g_gui", "getEditorAt", LuaInterface::luaGuiGetEditorAt);
	registerMethod("g_gui", "getCurrentEditor", LuaInterface::luaGuiGetCurrentEditor);
	registerMethod("g_gui", "setCurrentEditor", LuaInterface::luaGuiSetCurrentEditor);
	registerMethod("g_gui", "getCurrentFloor", LuaInterface::luaGuiGetCurrentFloor);
	registerMethod("g_gui", "setCurrentFloor", LuaInterface::luaGuiSetCurrentFloor);
	registerMethod("g_gui", "getCenterPosition", LuaInterface::luaGuiGetCenterPosition);
	registerMethod("g_gui", "setCenterPosition", LuaInterface::luaGuiSetCenterPosition);
	registerMethod("g_gui", "setSelectionMode", LuaInterface::luaGuiSetSelectionMode);
	registerMethod("g_gui", "setDrawingMode", LuaInterface::luaGuiSetDrawingMode);
	registerMethod("g_gui", "setStatusText", LuaInterface::luaGuiSetStatusText);
	registerMethod("g_gui", "showTextBox", LuaInterface::luaGuiShowTextBox);
	registerMethod("g_gui", "getExecDirectory", LuaInterface::luaGuiGetExecDirectory);
	registerMethod("g_gui", "getDataDirectory", LuaInterface::luaGuiGetDataDirectory);
	registerMethod("g_gui", "getLocalDataDirectory", LuaInterface::luaGuiGetLocalDataDirectory);
	registerMethod("g_gui", "getLocalDirectory", LuaInterface::luaGuiGetLocalDirectory);
	registerMethod("g_gui", "getExtensionsDirectory", LuaInterface::luaGuiGetExtensionsDirectory);

	// Editor
	registerClass("Editor", "", LuaInterface::luaEditorCreate);
	registerMetaMethod("Editor", "__eq", LuaInterface::luaUserdataCompare);
	registerMethod("Editor", "addItem", LuaInterface::luaEditorAddItem);
	registerMethod("Editor", "getMapDescription", LuaInterface::luaEditorGetMapDescription);
	registerMethod("Editor", "setMapDescription", LuaInterface::luaEditorSetMapDescription);
	registerMethod("Editor", "getMapWidth", LuaInterface::luaEditorGetMapWidth);
	registerMethod("Editor", "setMapWidth", LuaInterface::luaEditorSetMapWidth);
	registerMethod("Editor", "getMapHeight", LuaInterface::luaEditorGetMapHeight);
	registerMethod("Editor", "setMapHeight", LuaInterface::luaEditorSetMapHeight);
	registerMethod("Editor", "getTile", LuaInterface::luaEditorGetTile);
	registerMethod("Editor", "createSelection", LuaInterface::luaEditorCreateSelection);
	registerMethod("Editor", "selectTiles", LuaInterface::luaEditorSelectTiles);
	registerMethod("Editor", "getSelection", LuaInterface::luaEditorGetSelection);
	registerMethod("Editor", "getItemCount", LuaInterface::luaEditorGetItemCount);
	registerMethod("Editor", "removeItem", LuaInterface::luaEditorRemoveItem);
	registerMethod("Editor", "replaceItems", LuaInterface::luaEditorReplaceItems);
	registerMethod("Editor", "getHouses", LuaInterface::luaEditorGetHouses);
	registerMethod("Editor", "getTowns", LuaInterface::luaEditorGetTowns);

	// Tile
	registerClass("Tile", "", LuaInterface::luaTileCreate);
	registerMetaMethod("Tile", "__eq", LuaInterface::luaUserdataCompare);
	registerMethod("Tile", "getPosition", LuaInterface::luaTileGetPosition);
	registerMethod("Tile", "isHookEast", LuaInterface::luaTileIsHookEast);
	registerMethod("Tile", "isHookSouth", LuaInterface::luaTileIsHookSouth);
	registerMethod("Tile", "isHouse", LuaInterface::luaTileIsHouse);
	registerMethod("Tile", "isHouseExit", LuaInterface::luaTileIsHouseExit);
	registerMethod("Tile", "isPvP", LuaInterface::luaTileIsPvP);
	registerMethod("Tile", "isNoPvP", LuaInterface::luaTileIsNoPvP);
	registerMethod("Tile", "isNoLogout", LuaInterface::luaTileIsNoLogout);
	registerMethod("Tile", "isPZ", LuaInterface::luaTileIsPZ);
	registerMethod("Tile", "isBlocking", LuaInterface::luaTileIsBlocking);
	registerMethod("Tile", "isSelected", LuaInterface::luaTileIsSelected);
	registerMethod("Tile", "isModified", LuaInterface::luaTileIsModified);
	registerMethod("Tile", "setPvP", LuaInterface::luaTileSetPvP);
	registerMethod("Tile", "setNoPvP", LuaInterface::luaTileSetNoPvP);
	registerMethod("Tile", "setNoLogout", LuaInterface::luaTileSetNoLogout);
	registerMethod("Tile", "setPZ", LuaInterface::luaTileSetPZ);
	registerMethod("Tile", "hasItemId", LuaInterface::luaTileHasItemId);
	registerMethod("Tile", "hasWall", LuaInterface::luaTileHasWall);

	// Selection
	registerClass("Selection", "", LuaInterface::luaSelectionCreate);
	registerMetaMethod("Selection", "__eq", LuaInterface::luaUserdataCompare);
	registerMethod("Selection", "getTiles", LuaInterface::luaSelectionGetTiles);
	registerMethod("Selection", "getTileCount", LuaInterface::luaSelectionGetTileCount);
	registerMethod("Selection", "getMinPosition", LuaInterface::luaSelectionGetMinPosition);
	registerMethod("Selection", "getMaxPosition", LuaInterface::luaSelectionGetMaxPosition);
	registerMethod("Selection", "move", LuaInterface::luaSelectionMove);
	registerMethod("Selection", "offset", LuaInterface::luaSelectionOffset);
	registerMethod("Selection", "borderize", LuaInterface::luaSelectionBorderize);
	registerMethod("Selection", "randomize", LuaInterface::luaSelectionRandomize);
	registerMethod("Selection", "saveAsMinimap", LuaInterface::luaSelectionSaveAsMinimap);
	registerMethod("Selection", "replaceItems", LuaInterface::luaSelectionReplaceItems);
	registerMethod("Selection", "destroy", LuaInterface::luaSelectionDestroy);

	// House
	registerClass("House", "", LuaInterface::luaHouseCreate);
	registerMetaMethod("House", "__eq", LuaInterface::luaUserdataCompare);
	registerMethod("House", "getId", LuaInterface::luaHouseGetId);
	registerMethod("House", "getName", LuaInterface::luaHouseGetName);
	registerMethod("House", "getTownId", LuaInterface::luaHouseGetTownId);
	registerMethod("House", "getSize", LuaInterface::luaHouseGetSize);
	registerMethod("House", "getTiles", LuaInterface::luaHouseGetTiles);

	// Town
	registerClass("Town", "", LuaInterface::luaTownCreate);
	registerMetaMethod("Town", "__eq", LuaInterface::luaUserdataCompare);
	registerMethod("Town", "getId", LuaInterface::luaTownGetId);
	registerMethod("Town", "getName", LuaInterface::luaTownGetName);
	registerMethod("Town", "getTemplePosition", LuaInterface::luaTownGetTemplePosition);
}

int LuaInterface::luaErrorHandler(lua_State* L)
{
	const wxString& text = popString(L);
	ScriptingWindow* window = g_gui.ShowScriptingWindow();
	if(window) {
		window->AppendLog(text);
	}
	return 1;
}

// Same as lua_pcall, but adds stack trace to error strings in called function.
int LuaInterface::protectedCall(lua_State* L, int nargs, int nresults)
{
	int error_index = lua_gettop(L) - nargs;
	lua_pushcfunction(L, luaErrorHandler);
	lua_insert(L, error_index);

	int ret = lua_pcall(L, nargs, nresults, error_index);
	lua_remove(L, error_index);
	return ret;
}

bool LuaInterface::setTileFlag(Tile* tile, uint16_t flag, bool enable)
{
	if (tile && tile->hasGround()) {
		Editor* editor = g_gui.GetCurrentEditor(); // TODO get editor by tile.
		if (editor && editor->CanEdit()) {
			FlagBrush* brush = nullptr;
			switch (flag)
			{
				case TILESTATE_PVPZONE:
					brush = g_gui.pvp_brush;
					break;

				case TILESTATE_NOPVP:
					brush = g_gui.rook_brush;
					break;

				case TILESTATE_NOLOGOUT:
					brush = g_gui.nolog_brush;
					break;

				case TILESTATE_PROTECTIONZONE:
					brush = g_gui.pz_brush;
					break;

				default:
					return false;
			}

			g_gui.SetDrawingMode();
			g_gui.SelectBrushInternal(brush);
			PositionVector positions;
			positions.push_back(tile->getPosition());
			if (enable)
				editor->draw(positions, false);
			else
				editor->undraw(positions, false);
			g_gui.RefreshView();
			return true;
		}
	}
	return false;
}

bool LuaInterface::useRawBrush(Editor *editor, TileLocation* location, uint16_t itemId, bool adding)
{
	if(!editor || !location || itemId == 0)
		return false;

	if(!g_items.hasItemId(itemId))
		return false;

	ItemType& item = g_items.getItemType(itemId);
	RAWBrush* brush = item.raw_brush;

	if(brush) {
		Map *map = &editor->map;
		Tile* tile = location->get();
		Tile* new_tile = nullptr;

		if(tile) {
			new_tile = tile->deepCopy(*map);
			if(adding)
				brush->draw(map, new_tile, false);
			else
				brush->undraw(map, new_tile);
		} else if(adding) {
			new_tile = map->allocator(location);
			brush->draw(map, new_tile, false);
		}

		if(new_tile) {
			Action* action = editor->actionQueue->createAction(ACTION_DRAW);
			action->addChange(newd Change(new_tile));
			editor->addAction(action, 2);
			return true;
		}
	}

	return false;
}

bool LuaInterface::replaceItems(Editor *editor, std::map<uint32_t, uint32_t>& items, bool selectedTiles/*= false*/)
{
	if(!editor || items.empty())
		return false;

	g_lua.print("Starting search & replace on map...");

	for(std::map<uint32_t, uint32_t>::iterator it = items.begin(); it != items.end(); it++) {
		uint16_t find_id = it->first;
		uint16_t with_id = it->second;
		ItemFinder finder(find_id);

		g_gui.GetCurrentEditor()->actionQueue->clear();

		g_lua.print("\nSearching item id " + i2ws(find_id) + "...");

		// Search the map
		foreach_ItemOnMap(editor->map, finder, selectedTiles);

		std::vector<std::pair<Tile*, Item*>>& result = finder.result;
		if(result.empty()) {
			g_lua.print("No item id " + i2ws(find_id) + " was found.");
			continue;
		}

		g_lua.print("Replacing item id " + i2ws(find_id) + " with item id " + i2ws(with_id) + "...");

		// Replace the items in a second step (can't replace while iterating)

		for(std::vector<std::pair<Tile*, Item*>>::const_iterator rit = result.begin(); rit != result.end(); ++rit) {
			transformItem(rit->second, with_id, rit->first);
		}

		wxString msg;
		msg << "Replaced " << result.size() << " items id " << find_id << " with id " << with_id << ".";
		g_lua.print(msg);
	}

	return true;
}

