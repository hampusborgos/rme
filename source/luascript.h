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

#ifndef RME_LUA_SCRIPT_H_
#define RME_LUA_SCRIPT_H_

#include <luajit/lua.hpp>
#include "gui.h"
#include "map.h"

class GUI;
class Editor;
class Brush;
class FlagBrush;

enum LuaDataType {
	LuaData_Unknown,

	LuaData_Editor,
	LuaData_Tile,
	LuaData_Selection,
	LuaData_House
};

class LuaInterface
{
public:
	LuaInterface();
	~LuaInterface();

	bool init();

	int runScript(const wxString& script);
	void print(const wxString& text);

	// Userdata
	static int luaUserdataCompare(lua_State* L);

	// print
	static int luaPrint(lua_State* L);

	// g_gui
	static int luaGuiNewMap(lua_State* L);
	static int luaGuiLoadMap(lua_State* L);
	static int luaGuiShouldSaveMap(lua_State* L);
	static int luaGuiSaveCurrentMap(lua_State* L);
	static int luaGuiCanUndo(lua_State* L);
	static int luaGuiCanRedo(lua_State* L);
	static int luaGuiDoUndo(lua_State* L);
	static int luaGuiDoRedo(lua_State* L);
	static int luaGuiGetEditorAt(lua_State* L);
	static int luaGuiGetCurrentEditor(lua_State* L);
	static int luaGuiSetCurrentEditor(lua_State* L);
	static int luaGuiGetCurrentFloor(lua_State* L);
	static int luaGuiSetCurrentFloor(lua_State* L);
	static int luaGuiGetCenterPosition(lua_State* L);
	static int luaGuiSetCenterPosition(lua_State* L);
	static int luaGuiSetSelectionMode(lua_State* L);
	static int luaGuiSetDrawingMode(lua_State* L);
	static int luaGuiSetStatusText(lua_State* L);
	static int luaGuiShowTextBox(lua_State* L);
	static int luaGuiGetExecDirectory(lua_State* L);
	static int luaGuiGetDataDirectory(lua_State* L);
	static int luaGuiGetLocalDataDirectory(lua_State* L);
	static int luaGuiGetLocalDirectory(lua_State* L);
	static int luaGuiGetExtensionsDirectory(lua_State* L);

	// Editor
	static int luaEditorCreate(lua_State* L);
	static int luaEditorAddItem(lua_State* L);
	static int luaEditorGetMapDescription(lua_State* L);
	static int luaEditorSetMapDescription(lua_State* L);
	static int luaEditorGetMapWidth(lua_State* L);
	static int luaEditorSetMapWidth(lua_State* L);
	static int luaEditorGetMapHeight(lua_State* L);
	static int luaEditorSetMapHeight(lua_State* L);
	static int luaEditorGetTile(lua_State* L);
	static int luaEditorCreateSelection(lua_State* L);
	static int luaEditorSelectTiles(lua_State* L);
	static int luaEditorGetSelection(lua_State* L);
	static int luaEditorGetItemCount(lua_State* L);
	static int luaEditorRemoveItem(lua_State* L);
	static int luaEditorReplaceItems(lua_State* L);
	static int luaEditorGetHouses(lua_State* L);

	// Tile
	static int luaTileCreate(lua_State* L);
	static int luaTileGetPosition(lua_State* L);
	static int luaTileIsHookEast(lua_State* L);
	static int luaTileIsHookSouth(lua_State* L);
	static int luaTileIsHouse(lua_State* L);
	static int luaTileIsHouseExit(lua_State* L);
	static int luaTileIsPvP(lua_State* L);
	static int luaTileIsNoPvP(lua_State* L);
	static int luaTileIsNoLogout(lua_State* L);
	static int luaTileIsPZ(lua_State* L);
	static int luaTileIsBlocking(lua_State* L);
	static int luaTileIsSelected(lua_State* L);
	static int luaTileIsModified(lua_State* L);
	static int luaTileSetPvP(lua_State* L);
	static int luaTileSetNoPvP(lua_State* L);
	static int luaTileSetNoLogout(lua_State* L);
	static int luaTileSetPZ(lua_State* L);
	static int luaTileHasWall(lua_State* L);

	// Selection
	static int luaSelectionCreate(lua_State* L);
	static int luaSelectionGetTiles(lua_State* L);
	static int luaSelectionGetTileCount(lua_State* L);
	static int luaSelectionGetMinPosition(lua_State* L);
	static int luaSelectionGetMaxPosition(lua_State* L);
	static int luaSelectionMove(lua_State* L);
	static int luaSelectionOffset(lua_State* L);
	static int luaSelectionBorderize(lua_State* L);
	static int luaSelectionRandomize(lua_State* L);
	static int luaSelectionSaveAsMinimap(lua_State* L);
	static int luaSelectionReplaceItems(lua_State* L);
	static int luaSelectionDestroy(lua_State* L);

	// House
	static int luaHouseCreate(lua_State* L);
	static int luaHouseGetId(lua_State* L);
	static int luaHouseGetName(lua_State* L);
	static int luaHouseGetTownId(lua_State* L);
	static int luaHouseGetSize(lua_State* L);
	static int luaHouseGetTiles(lua_State* L);

	int getTop();
	bool hasIndex(int index) { return (getTop() >= (index < 0 ? -index : index) && index != 0); }

	static wxString popString(lua_State* L);

	// Userdata
	template<class T>
	static void pushUserdata(lua_State* L, T* value)
	{
		T** userdata = static_cast<T**>(lua_newuserdata(L, sizeof(T*)));
		*userdata = value;
	}

	// Metatables
	static void setMetatable(lua_State* L, int32_t index, const std::string& name);

	//push/pop common structures
	static void pushString(lua_State* L, const wxString& value);
	static void pushBoolean(lua_State* L, bool value);
	static void pushPosition(lua_State* L, const Position& position);

	// Get
	static wxString getString(lua_State* L, int32_t arg);
	template<typename T>
	inline static typename std::enable_if<std::is_enum<T>::value, T>::type
		getNumber(lua_State* L, int32_t arg)
	{
		return static_cast<T>(static_cast<int64_t>(lua_tonumber(L, arg)));
	}
	template<typename T>
	inline static typename std::enable_if<std::is_integral<T>::value || std::is_floating_point<T>::value, T>::type
		getNumber(lua_State* L, int32_t arg)
	{
		return static_cast<T>(lua_tonumber(L, arg));
	}
	template<typename T>
	static T getNumber(lua_State *L, int32_t arg, T defaultValue)
	{
		const auto parameters = lua_gettop(L);
		if (parameters == 0 || arg > parameters) {
			return defaultValue;
		}
		return getNumber<T>(L, arg);
	}
	template<class T>
	inline static T** getRawUserdata(lua_State* L, int32_t arg)
	{
		return static_cast<T**>(lua_touserdata(L, arg));
	}
	template<class T>
	static T* getUserdata(lua_State* L, int32_t arg)
	{
		T** userdata = getRawUserdata<T>(L, arg);
		if (!userdata) {
			return nullptr;
		}
		return *userdata;
	}

	template<typename T>
	static T getField(lua_State* L, int32_t arg, const std::string& key)
	{
		lua_getfield(L, arg, key.c_str());
		return getNumber<T>(L, -1);
	}

	static Position getPosition(lua_State* L, int32_t arg);

	// Is
	inline static bool isNumber(lua_State* L, int32_t arg)
	{
		return lua_type(L, arg) == LUA_TNUMBER;
	}
	inline static bool isString(lua_State* L, int32_t arg)
	{
		return lua_isstring(L, arg) != 0;
	}
	inline static bool isBoolean(lua_State* L, int32_t arg)
	{
		return lua_isboolean(L, arg);
	}
	inline static bool isTable(lua_State* L, int32_t arg)
	{
		return lua_istable(L, arg);
	}
	inline static bool isFunction(lua_State* L, int32_t arg)
	{
		return lua_isfunction(L, arg);
	}
	inline static bool isUserdata(lua_State* L, int32_t arg)
	{
		return lua_isuserdata(L, arg) != 0;
	}

	inline static void setField(lua_State* L, const char* index, lua_Number value)
	{
		lua_pushnumber(L, value);
		lua_setfield(L, -2, index);
	}
	inline static void setField(lua_State* L, const char* index, const wxString& value)
	{
		pushString(L, value);
		lua_setfield(L, -2, index);
	}
	inline static bool getBoolean(lua_State* L, int32_t arg)
	{
		return lua_toboolean(L, arg) != 0;
	}
	inline static bool getBoolean(lua_State* L, int32_t arg, bool defaultValue)
	{
		const auto parameters = lua_gettop(L);
		if(parameters == 0 || arg > parameters) {
			return defaultValue;
		}
		return lua_toboolean(L, arg) != 0;
	}

	static int luaErrorHandler(lua_State* L);
	static int protectedCall(lua_State* L, int nargs, int nresults);

	lua_State* luaState;

private:
	void registerClass(const std::string& className, const std::string& baseClass, lua_CFunction newFunction = nullptr);
	void registerTable(const std::string& tableName);
	void registerMethod(const std::string& className, const std::string& methodName, lua_CFunction func);
	void registerMetaMethod(const std::string& className, const std::string& methodName, lua_CFunction func);
	void registerGlobalMethod(const std::string& functionName, lua_CFunction func);
	void registerFunctions();

	static bool useRawBrush(Editor *editor, TileLocation* location, uint16_t itemId, bool adding);
	static bool replaceItems(Editor *editor, std::map<uint32_t, uint32_t>& items, bool selectedTiles = false);
	static bool setTileFlag(Tile* tile, uint16_t flag, bool enable);
};

extern LuaInterface g_lua;



struct ItemFinder
{
	ItemFinder(uint16_t itemid, int32_t limit = -1) : itemid(itemid), limit(limit), exceeded(false) {}

	void operator()(Map& map, Tile* tile, Item* item, long long done) {
		if(exceeded)
			return;

		if(item->getID() == itemid) {
			result.push_back(std::make_pair(tile, item));
			if(limit > 0 && result.size() >= size_t(limit))
				exceeded = true;
		}
	}

	std::vector<std::pair<Tile*, Item*>> result;

private:
	uint16_t itemid;
	int32_t limit;
	bool exceeded;
};



struct ItemCounter
{
	ItemCounter(uint16_t itemid) : itemid(itemid), result(0) {}

	void operator()(Map& map, Tile* tile, Item* item, long long done) {
		if(item->getID() == itemid)
			result++;
	}

	uint16_t itemid;
	uint32_t result;
};

#endif
