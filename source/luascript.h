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

#include <lua.hpp>

class GUI;
class Editor;

enum LuaDataType {
	LuaData_Unknown,

	LuaData_Editor,
	LuaData_Tile
};

class LuaInterface
{
public:
	LuaInterface();
	~LuaInterface();

	bool init();

	// Push
	static void pushBoolean(lua_State* L, bool value);
	static void pushPosition(lua_State* L, const Position& position, int32_t stackpos = 0);

	// Userdata
	static int luaUserdataCompare(lua_State* L);

	// g_gui
	static int luaGuiNewMap(lua_State* L);
	static int luaGuiLoadMap(lua_State* L);
	static int luaGuiShouldSaveMap(lua_State* L);
	static int luaGuiSaveCurrentMap(lua_State* L);
	static int luaGuiGetCurrentFloor(lua_State* L);
	static int luaGuiSetCurrentFloor(lua_State* L);
	static int luaGuiSetCenterPosition(lua_State* L);
	static int luaGuiShowTextBox(lua_State* L);

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
	static void pushString(lua_State* L, const std::string& value);

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
	inline static void setField(lua_State* L, const char* index, const std::string& value)
	{
		pushString(L, value);
		lua_setfield(L, -2, index);
	}

	static int luaErrorHandler(lua_State* L);
	static int protectedCall(lua_State* L, int nargs, int nresults);
	bool loadFile(const wxFileName& file);

	lua_State* luaState;

private:
	void registerClass(const std::string& className, const std::string& baseClass, lua_CFunction newFunction = nullptr);
	void registerTable(const std::string& tableName);
	void registerMethod(const std::string& className, const std::string& methodName, lua_CFunction func);
	void registerMetaMethod(const std::string& className, const std::string& methodName, lua_CFunction func);
	void registerFunctions();
};

extern LuaInterface g_lua;

#endif