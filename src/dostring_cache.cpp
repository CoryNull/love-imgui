#include "dostring_cache.h"
#include <map>

namespace DoStringCache {

	std::map<std::string, int> refList;
	std::string tableName;

	void init(lua_State *L, const std::string &tabname) {
		int top   = lua_gettop(L);
		tableName = tabname;
		lua_pushlstring(L, tabname.c_str(), tabname.length());
		lua_newtable(L);
		lua_pushstring(L, "imgui");
		lua_pushvalue(L, top - 1);
		lua_rawset(L, -3);
		lua_pushstring(L, "love");
		lua_pushvalue(L, top);
		lua_rawset(L, -3);
		lua_rawset(L, LUA_REGISTRYINDEX);
	}

	int doString(lua_State *L, const std::string &str) {
		// get table in registry
		lua_pushlstring(L, tableName.c_str(), tableName.length());
		lua_rawget(L, LUA_REGISTRYINDEX);
		int top = lua_gettop(L);

		lua_pushlstring(L, str.c_str(), str.length());
		lua_rawget(L, top);

		if (lua_isnil(L, -1)) {
			// Load new string
			static std::string prepend = "local imgui, love = ... ";
			int r;
			std::string luastr = prepend + str;
			lua_pop(L, 1); // remove nil

			if ((r = luaL_loadbuffer(L, luastr.c_str(), luastr.length(), luastr.c_str()))) {
				// leave error message in stack
				lua_remove(L, top);
				return r;
			}

			lua_pushlstring(L, str.c_str(), str.length());
			lua_pushvalue(L, -2);
			lua_rawset(L, top);
		}

		// function is in -1
		lua_pushstring(L, "imgui");
		lua_rawget(L, top);
		lua_pushstring(L, "love");
		lua_rawget(L, top);
		lua_remove(L, top);
		lua_call(L, 2, LUA_MULTRET);
		return 0;
	}

	void getImgui(lua_State *L) {
		lua_pushlstring(L, tableName.c_str(), tableName.length());
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_pushstring(L, "imgui");
		lua_rawget(L, -2);
		lua_remove(L, -2);
	}

	void getLOVE(lua_State *L) {
		lua_pushlstring(L, tableName.c_str(), tableName.length());
		lua_rawget(L, LUA_REGISTRYINDEX);
		lua_pushstring(L, "love");
		lua_rawget(L, -2);
		lua_remove(L, -2);
	}

} // namespace DoStringCache
