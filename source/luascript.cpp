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

// Push
void LuaInterface::pushBoolean(lua_State* L, bool value)
{
	lua_pushboolean(L, value ? 1 : 0);
}

void LuaInterface::pushPosition(lua_State* L, const Position& position, int32_t stackpos/* = 0*/)
{
	lua_createtable(L, 0, 4);

	setField(L, "x", position.x);
	setField(L, "y", position.y);
	setField(L, "z", position.z);
	setField(L, "stackpos", stackpos);

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

int LuaInterface::luaGuiSetCenterPosition(lua_State* L)
{
	// g_gui.setCenterPosition(position)
	const Position& position = getPosition(L, 1);
	pushBoolean(L, g_gui.CenterOnPosition(position));
	return 1;
}

int LuaInterface::luaGuiShowTextBox(lua_State* L)
{
	// g_gui.showTextBox(title, contents)
	const wxString& contents = getString(L, 1);
	const wxString& title = getString(L, 2);
	g_gui.ShowTextBox(title, contents);
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

void LuaInterface::registerFunctions()
{
	// g_gui
	registerTable("g_gui");
	registerMethod("g_gui", "newMap", LuaInterface::luaGuiNewMap);
	registerMethod("g_gui", "loadMap", LuaInterface::luaGuiLoadMap);
	registerMethod("g_gui", "shouldSaveCurrentMap", LuaInterface::luaGuiShouldSaveMap);
	registerMethod("g_gui", "saveCurrentMap", LuaInterface::luaGuiSaveCurrentMap);
	registerMethod("g_gui", "doUndo", LuaInterface::luaGuiDoUndo);
	registerMethod("g_gui", "doRedo", LuaInterface::luaGuiDoRedo);
	registerMethod("g_gui", "getCurrentFloor", LuaInterface::luaGuiGetCurrentFloor);
	registerMethod("g_gui", "setCurrentFloor", LuaInterface::luaGuiSetCurrentFloor);
	registerMethod("g_gui", "setCenterPosition", LuaInterface::luaGuiSetCenterPosition);
	registerMethod("g_gui", "showTextBox", LuaInterface::luaGuiShowTextBox);
}

int LuaInterface::luaErrorHandler(lua_State* L)
{
	const wxString& errorMessage = popString(L);
	g_gui.ShowTextBox(wxT("Error"), errorMessage);
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

bool LuaInterface::loadFile(const wxFileName& file)
{
	//loads file as a chunk at stack top
	int ret = luaL_loadfile(luaState, (file.GetFullPath()).c_str());
	if(ret != 0) {
		return false;
	}

	//check that it is loaded as a function
	if(!isFunction(luaState, -1)) {
		return false;
	}

	//execute it
	ret = protectedCall(luaState, 0, 0);
	if(ret != 0) {
		return false;
	}

	return true;
}
