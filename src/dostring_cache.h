#pragma once

#include <string>

#include "lua.hpp"

namespace DoStringCache {

	// imgui at -1 and love at -2
	void init(lua_State *L, const std::string &tabname);
	int doString(lua_State *L, const std::string &str);
	void getImgui(lua_State *L);
	void getLOVE(lua_State *L);

} // namespace DoStringCache
