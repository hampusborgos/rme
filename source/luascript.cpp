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

// Push
void LuaInterface::pushBoolean(lua_State* L, bool value)
{
	lua_pushboolean(L, value ? 1 : 0);
}

void LuaInterface::pushPosition(lua_State* L, const Position& position)
{
	lua_createtable(L, 0, 4);

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

void LuaInterface::pushString(lua_State* L, const std::string& value)
{
	lua_pushlstring(L, value.c_str(), value.length());
}

wxString LuaInterface::popString(lua_State* L)
{
	if(lua_gettop(L) == 0) {
		return wxString();
	}

	wxString str(getString(L, -1));
	lua_pop(L, 1);
	return str;
}

wxString LuaInterface::getString(lua_State* L, int32_t arg)
{
	size_t len;
	const char* c_str = lua_tolstring(L, arg, &len);
	if(!c_str || len == 0)
	{
		return wxString();
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
	if (mapTab) {
		MapCanvas* canvas = mapTab->GetCanvas();
		int x, y, z = canvas->GetFloor();
		canvas->GetScreenCenter(&x, &y);
		const Position position(x, y, z);
		pushPosition(L, position);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int LuaInterface::luaGuiSetCenterPosition(lua_State* L)
{
	// g_gui.setCenterPosition(position)
	const Position& position = getPosition(L, 1);
	pushBoolean(L, g_gui.CenterOnPosition(position));
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

int LuaInterface::luaGuiShowTextBox(lua_State* L)
{
	// g_gui.showTextBox(title, text)
	const wxString& text = getString(L, 2);
	const wxString& title = getString(L, 1);
	g_gui.ShowTextBox(g_gui.root, title, text);
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

int LuaInterface::luaEditorGetTile(lua_State* L)
{
	// Editor:getTile(x, y, z)
	// Editor:getTile(position)
	Editor* editor = getUserdata<Editor>(L, 1);
	if (!editor) {
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
			pushUserdata<Editor>(L, editor);
			setMetatable(L, -1, "Selection");
			return 1;
		}
	}

	lua_pushnil(L);
	return 1;
}

int LuaInterface::luaEditorMoveSelection(lua_State* L)
{
	// editor:moveSelection(position)
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor) {
		const Position& position = getPosition(L, 2);
		pushBoolean(L, editor->moveSelection(position));
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaEditorDestroySelection(lua_State* L)
{
	// editor:destroySelection()
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor) {
		pushBoolean(L, editor->destroySelection());
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaEditorBorderizeSelection(lua_State* L)
{
	// editor:borderizeSelection()
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor) {
		pushBoolean(L, editor->borderizeSelection());
	} else {
		pushBoolean(L, false);
	}
	return 1;
}

int LuaInterface::luaEditorRandomizeSelection(lua_State* L)
{
	// editor:randomizeSelection()
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor) {
		pushBoolean(L, editor->randomizeSelection());
	} else {
		pushBoolean(L, false);
	}
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
	// Tile:getPosition()
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		const Position position = tile->getPosition();
		pushPosition(L, position);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int LuaInterface::luaTileIsHouse(lua_State* L)
{
	// Tile:isHouse()
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
	// Tile:isHouseExit()
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
	// Tile:isPvP()
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
	// Tile:isNoPvP()
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
	// Tile:isNoLogout()
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
	// Tile:isPZ()
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
	// Tile:isBlocking()
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
	// Tile:isSelected()
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
	// Tile:isModified()
	Tile* tile = getUserdata<Tile>(L, 1);
	if(tile) {
		pushBoolean(L, tile->isModified());
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
			pushUserdata<Editor>(L, editor);
			setMetatable(L, -1, "Selection");
			return 1;
		}
	}

	lua_pushnil(L);
	return 1;
}

int LuaInterface::luaSelectionGetTileCount(lua_State* L)
{
	// Selection:getTileCount()
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
	// Selection:getMinPosition()
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor && editor->selection.size() > 0) {
		const Position& position = editor->selection.minPosition();
		pushPosition(L, position);
	} else {
		lua_pushnil(L);
	}
	return 1;
}

int LuaInterface::luaSelectionGetMaxPosition(lua_State* L)
{
	// Selection:getMaxPosition()
	Editor* editor = getUserdata<Editor>(L, 1);
	if(editor && editor->selection.size() > 0) {
		const Position& position = editor->selection.maxPosition();
		pushPosition(L, position);
	} else {
		lua_pushnil(L);
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
	registerMethod("g_gui", "showTextBox", LuaInterface::luaGuiShowTextBox);

	// Editor
	registerClass("Editor", "", LuaInterface::luaEditorCreate);
	registerMetaMethod("Editor", "__eq", LuaInterface::luaUserdataCompare);
	registerMethod("Editor", "getTile", LuaInterface::luaEditorGetTile);
	registerMethod("Editor", "createSelection", LuaInterface::luaEditorCreateSelection);
	registerMethod("Editor", "moveSelection", LuaInterface::luaEditorMoveSelection);
	registerMethod("Editor", "destroySelection", LuaInterface::luaEditorDestroySelection);
	registerMethod("Editor", "borderizeSelection", LuaInterface::luaEditorBorderizeSelection);
	registerMethod("Editor", "randomizeSelection", LuaInterface::luaEditorRandomizeSelection);
	registerMethod("Editor", "getSelection", LuaInterface::luaEditorGetSelection);

	// Tile
	registerClass("Tile", "", LuaInterface::luaTileCreate);
	registerMetaMethod("Tile", "__eq", LuaInterface::luaUserdataCompare);
	registerMethod("Tile", "getPosition", LuaInterface::luaTileGetPosition);
	registerMethod("Tile", "isHouse", LuaInterface::luaTileIsHouse);
	registerMethod("Tile", "isHouseExit", LuaInterface::luaTileIsHouseExit);
	registerMethod("Tile", "isPvP", LuaInterface::luaTileIsPvP);
	registerMethod("Tile", "isNoPvP", LuaInterface::luaTileIsNoPvP);
	registerMethod("Tile", "isNoLogout", LuaInterface::luaTileIsNoLogout);
	registerMethod("Tile", "isPZ", LuaInterface::luaTileIsPZ);
	registerMethod("Tile", "isBlocking", LuaInterface::luaTileIsBlocking);
	registerMethod("Tile", "isSelected", LuaInterface::luaTileIsSelected);
	registerMethod("Tile", "isModified", LuaInterface::luaTileIsModified);

	// Selection
	registerClass("Selection", "", LuaInterface::luaSelectionCreate);
	registerMetaMethod("Selection", "__eq", LuaInterface::luaUserdataCompare);
	registerMethod("Selection", "getTileCount", LuaInterface::luaSelectionGetTileCount);
	registerMethod("Selection", "getMinPosition", LuaInterface::luaSelectionGetMinPosition);
	registerMethod("Selection", "getMaxPosition", LuaInterface::luaSelectionGetMaxPosition);
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

