// IMGUI
#include "imgui_impl.h"
#include "dostring_cache.h"
#include "imgui/imgui.h"

// STD
#include <algorithm>
#include <cmath>
#include <exception>
#include <map>
#include <string>
#include <vector>
//#include <iostream>

// Data
static bool g_MousePressed[3] = {false, false, false};
static float g_MouseWheel     = 0.0f;
static std::string g_iniPath;
static std::map<std::string, int> g_keyMap;
static lua_State *g_L;

// dostring cache hack
#ifdef luaL_dostring
#undef luaL_dostring
#define luaL_dostring DoStringCache::doString
#endif

// This is the main rendering function that you have to implement and provide to ImGui (via setting up
// 'RenderDrawListsFn' in the ImGuiIO structure) If text or lines are blurry when integrating ImGui in your engine:
// - in your Render function, try translating your projection matrix by (0.5f,0.5f) or (0.375f,0.375f)
void ImGui_Impl_RenderDrawLists(ImDrawData *draw_data) {
	// Avoid rendering when minimized, scale coordinates for retina displays (screen coordinates != framebuffer
	// coordinates)
	ImGuiIO &io   = ImGui::GetIO();
	int fb_width  = ( int )(io.DisplaySize.x * io.DisplayFramebufferScale.x);
	int fb_height = ( int )(io.DisplaySize.y * io.DisplayFramebufferScale.y);
	if (fb_width == 0 || fb_height == 0) return;
	draw_data->ScaleClipRects(io.DisplayFramebufferScale);

	DoStringCache::getImgui(g_L);
	// Render command lists
	for (int n = 0; n < draw_data->CmdListsCount; n++) {
		const ImDrawList *cmd_list = draw_data->CmdLists[n];

		lua_newtable(g_L);
		for (int i = 1; i <= cmd_list->IdxBuffer.size(); i++) {
			lua_pushnumber(g_L, i);
			lua_pushnumber(g_L, cmd_list->IdxBuffer[i - 1] + 1);
			lua_rawset(g_L, -3);
		}
		lua_setfield(g_L, -2, "idx");

		lua_pushlstring(g_L, ( char * )&cmd_list->VtxBuffer.front(), cmd_list->VtxBuffer.size() * sizeof(ImDrawVert));
		lua_setfield(g_L, -2, "verticesData");
		lua_pushnumber(g_L, cmd_list->VtxBuffer.size() * sizeof(ImDrawVert));
		lua_setfield(g_L, -2, "verticesSize");

		luaL_dostring(
				g_L,
				"imgui.renderMesh = love.graphics.newMesh(imgui.vertexformat, love.image.newImageData(imgui.verticesSize / 4, 1, 'rgba8', imgui.verticesData), \"triangles\")\
						    imgui.renderMesh:setTexture(imgui.textureObject)\
							imgui.renderMesh:setVertexMap(imgui.idx)");

		int position = 1;
		for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.size(); cmd_i++) {
			auto pcmd = &cmd_list->CmdBuffer[cmd_i];
			lua_pushnumber(g_L, pcmd->ElemCount);
			lua_setfield(g_L, -2, "vertexCount");
			lua_pushnumber(g_L, position);
			lua_setfield(g_L, -2, "vertexPosition");
			position += pcmd->ElemCount;

			lua_pushnumber(g_L, ( int )pcmd->ClipRect.x);
			lua_setfield(g_L, -2, "clipX");
			lua_pushnumber(g_L, ( int )(pcmd->ClipRect.y));
			lua_setfield(g_L, -2, "clipY");
			lua_pushnumber(g_L, ( int )(pcmd->ClipRect.z - pcmd->ClipRect.x));
			lua_setfield(g_L, -2, "clipWidth");
			lua_pushnumber(g_L, ( int )(pcmd->ClipRect.w - pcmd->ClipRect.y));
			lua_setfield(g_L, -2, "clipHeight");

			luaL_dostring(g_L, "love.graphics.setBlendMode(\"alpha\")");
			if (pcmd->TextureId == NULL)
				luaL_dostring(g_L, "imgui.renderMesh:setTexture(imgui.textureObject)");
			else {
				lua_pushnumber(g_L, (( int * )pcmd->TextureId)[0]);
				lua_setfield(g_L, -2, "currentTexture");
				luaL_dostring(g_L, "\
					local texture = imgui.textures[imgui.currentTexture]\
					if texture:typeOf(\"Canvas\") then\
						love.graphics.setBlendMode(\"alpha\", \"premultiplied\")\
					end\
					imgui.renderMesh:setTexture(texture)\
				");
			}

			luaL_dostring(g_L, "\
				love.graphics.setScissor(imgui.clipX, imgui.clipY, imgui.clipWidth, imgui.clipHeight) \
				imgui.renderMesh:setDrawRange(imgui.vertexPosition, imgui.vertexCount) \
				love.graphics.draw(imgui.renderMesh) \
			");
		}
	}
	luaL_dostring(g_L, "love.graphics.setScissor()");
	lua_pop(g_L, 1);
}

static const char *ImGui_Impl_GetClipboardText(void *user_data) {
	luaL_dostring(g_L, "return love.system.getClipboardText()");
	return luaL_checkstring(g_L, 0);
}

static void ImGui_Impl_SetClipboardText(void *user_data, const char *text) {
	DoStringCache::getImgui(g_L);
	lua_pushstring(g_L, text);
	lua_setfield(g_L, -2, "clipboardText");
	luaL_dostring(g_L, "love.system.setClipboardText(imgui.clipboardText)");
	lua_pop(g_L, 1);
}

//
// Public part
//

bool Init(lua_State *L) {
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();

	// LUA state
	g_L = L;

	// Create the texture object
	unsigned char *pixels;
	int width, height;
	io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

	DoStringCache::getImgui(L);
	lua_pushnumber(L, width);
	lua_setfield(L, -2, "textureWidth");
	lua_pushnumber(L, height);
	lua_setfield(L, -2, "textureHeight");
	lua_pushlstring(L, ( char * )pixels, width * height * 4);
	lua_setfield(L, -2, "texturePixels");
	luaL_dostring(
			L,
			"imgui.textureObject = love.graphics.newImage(love.image.newImageData(imgui.textureWidth, imgui.textureHeight, 'rgba8', imgui.texturePixels))\
					  imgui.vertexformat = { {\"VertexPosition\", \"float\", 2}, {\"VertexTexCoord\", \"float\", 2}, {\"VertexColor\", \"byte\", 4} }");
	lua_pop(L, 1);

	// Key map
	g_keyMap["tab"]       = 1;
	g_keyMap["left"]      = 2;
	g_keyMap["right"]     = 3;
	g_keyMap["up"]        = 4;
	g_keyMap["down"]      = 5;
	g_keyMap["pageup"]    = 6;
	g_keyMap["pagedown"]  = 7;
	g_keyMap["home"]      = 8;
	g_keyMap["end"]       = 9;
	g_keyMap["delete"]    = 10;
	g_keyMap["backspace"] = 11;
	g_keyMap["return"]    = 12;
	g_keyMap["escape"]    = 13;
	g_keyMap["a"]         = 14;
	g_keyMap["c"]         = 15;
	g_keyMap["v"]         = 16;
	g_keyMap["x"]         = 17;
	g_keyMap["y"]         = 18;
	g_keyMap["z"]         = 19;

	io.KeyMap[ImGuiKey_Tab] =
			g_keyMap["tab"]; // Keyboard mapping. ImGui will use those indices to peek into the io.KeyDown[] array.
	io.KeyMap[ImGuiKey_LeftArrow]  = g_keyMap["left"];
	io.KeyMap[ImGuiKey_RightArrow] = g_keyMap["right"];
	io.KeyMap[ImGuiKey_UpArrow]    = g_keyMap["up"];
	io.KeyMap[ImGuiKey_DownArrow]  = g_keyMap["down"];
	io.KeyMap[ImGuiKey_PageUp]     = g_keyMap["pageup"];
	io.KeyMap[ImGuiKey_PageDown]   = g_keyMap["pagedown"];
	io.KeyMap[ImGuiKey_Home]       = g_keyMap["home"];
	io.KeyMap[ImGuiKey_End]        = g_keyMap["end"];
	io.KeyMap[ImGuiKey_Delete]     = g_keyMap["delete"];
	io.KeyMap[ImGuiKey_Backspace]  = g_keyMap["backspace"];
	io.KeyMap[ImGuiKey_Enter]      = g_keyMap["return"];
	io.KeyMap[ImGuiKey_Escape]     = g_keyMap["escape"];
	io.KeyMap[ImGuiKey_Space]			 = g_keyMap["space"];
	io.KeyMap[ImGuiKey_A]          = g_keyMap["a"];
	io.KeyMap[ImGuiKey_C]          = g_keyMap["c"];
	io.KeyMap[ImGuiKey_V]          = g_keyMap["v"];
	io.KeyMap[ImGuiKey_X]          = g_keyMap["x"];
	io.KeyMap[ImGuiKey_Y]          = g_keyMap["y"];
	io.KeyMap[ImGuiKey_Z]          = g_keyMap["z"];

	io.RenderDrawListsFn =
			ImGui_Impl_RenderDrawLists; // Alternatively you can set this to NULL and call ImGui::GetDrawData() after
																	// ImGui::Render() to get the same ImDrawData pointer.
	io.SetClipboardTextFn = ImGui_Impl_SetClipboardText;
	io.GetClipboardTextFn = ImGui_Impl_GetClipboardText;

	io.Fonts->TexID = NULL;

	luaL_dostring(L, "love.filesystem.createDirectory('/') return love.filesystem.getSaveDirectory()");
	auto path      = luaL_checkstring(L, 1);
	g_iniPath      = std::string(path) + std::string("/imgui.ini");
	io.IniFilename = g_iniPath.c_str();

	return true;
}

void ShutDown() { ImGui::DestroyContext(); }

void NewFrame() {
	ImGuiIO &io = ImGui::GetIO();

	// Setup display size (every frame to accommodate for window resizing)
	luaL_dostring(g_L, "return love.graphics.getPixelDimensions()");
	io.DisplaySize = ImVec2(luaL_checknumber(g_L, -2), luaL_checknumber(g_L, -1));
	lua_pop(g_L, 2);

	// int display_w, display_h;
	// SDL_GL_GetDrawableSize(window, &display_w, &display_h);
	// io.DisplayFramebufferScale = ImVec2(w > 0 ? ((float)display_w / w) : 0, h > 0 ? ((float)display_h / h) : 0);
	io.DisplayFramebufferScale = ImVec2(1.0f, 1.0f);

	// Setup time step
	luaL_dostring(g_L, "return love.timer.getDelta()");
	auto time = luaL_checknumber(g_L, -1);
	lua_pop(g_L, 1);
	io.DeltaTime = ( float )time;

	// Setup input
	io.MouseDown[0] = g_MousePressed[0];
	io.MouseDown[1] = g_MousePressed[1];
	io.MouseDown[2] = g_MousePressed[2];
	io.MouseWheel   = g_MouseWheel;
	g_MouseWheel    = 0.0f;

	// Hide OS mouse cursor if ImGui is drawing it
	DoStringCache::getImgui(g_L);
	lua_pushboolean(g_L, ( int )io.MouseDrawCursor);
	lua_setfield(g_L, -2, "mouseDrawCursor");
	luaL_dostring(g_L, "love.mouse.setVisible(not imgui.mouseDrawCursor)");

	// Init lua data
	luaL_dostring(g_L, "imgui.textures = nil");
	lua_pop(g_L, 1);

	// Start the frame
	ImGui::NewFrame();
}

//
// Inputs
//
void MouseMoved(int x, int y) {
	if (g_L) {
		ImGuiIO &io = ImGui::GetIO();
		luaL_dostring(g_L, "return love.window.hasMouseFocus()");
		auto focus = lua_toboolean(g_L, 3);
		if (focus > 0)
			io.MousePos = ImVec2(
					( float )x, ( float )y); // Mouse position, in pixels (set to -1,-1 if no mouse / on another screen, etc.)
		else
			io.MousePos = ImVec2(-1, -1);
	}
}

void MousePressed(int button) {
	if (button == 1) g_MousePressed[0] = true;
	if (button == 2) g_MousePressed[1] = true;
	if (button == 3) g_MousePressed[2] = true;
}

void MouseReleased(int button) {
	if (button == 1) g_MousePressed[0] = false;
	if (button == 2) g_MousePressed[1] = false;
	if (button == 3) g_MousePressed[2] = false;
}

void WheelMoved(int y) {
	if (y > 0) g_MouseWheel = 1;
	if (y < 0) g_MouseWheel = -1;
}

void KeyPressed(const char *key) {
	if (g_L) {
		std::string k(key);
		if (k == "kpenter") k = "return";
		ImGuiIO &io              = ImGui::GetIO();
		io.KeysDown[g_keyMap[k]] = true;
		luaL_dostring(g_L, "return (love.keyboard.isDown('rshift') or love.keyboard.isDown('lshift'))");
		auto down   = lua_toboolean(g_L, 2) > 0;
		io.KeyShift = down;
		luaL_dostring(g_L, "return (love.keyboard.isDown('rctrl') or love.keyboard.isDown('lctrl'))");
		down       = lua_toboolean(g_L, 3) > 0;
		io.KeyCtrl = down;
		luaL_dostring(g_L, "return (love.keyboard.isDown('ralt') or love.keyboard.isDown('lalt'))");
		down      = lua_toboolean(g_L, 4) > 0;
		io.KeyAlt = down;
		luaL_dostring(g_L, "return (love.keyboard.isDown('rgui') or love.keyboard.isDown('lgui'))");
		down        = lua_toboolean(g_L, 5) > 0;
		io.KeySuper = down;
	}
}

void KeyReleased(const char *key) {
	if (g_L) {
		std::string k(key);
		if (k == "kpenter") k = "return";
		ImGuiIO &io                      = ImGui::GetIO();
		io.KeysDown[g_keyMap[k.c_str()]] = false;
		luaL_dostring(g_L, "return (love.keyboard.isDown('rshift') or love.keyboard.isDown('lshift'))");
		bool down   = lua_toboolean(g_L, 2) > 0;
		io.KeyShift = down;
		luaL_dostring(g_L, "return (love.keyboard.isDown('rctrl') or love.keyboard.isDown('lctrl'))");
		down       = lua_toboolean(g_L, 3) > 0;
		io.KeyCtrl = down;
		luaL_dostring(g_L, "return (love.keyboard.isDown('ralt') or love.keyboard.isDown('lalt'))");
		down      = lua_toboolean(g_L, 4) > 0;
		io.KeyAlt = down;
		luaL_dostring(g_L, "return (love.keyboard.isDown('rgui') or love.keyboard.isDown('lgui'))");
		down        = lua_toboolean(g_L, 5) > 0;
		io.KeySuper = down;
	}
}

void TextInput(const char *text) {
	ImGuiIO &io = ImGui::GetIO();
	io.AddInputCharactersUTF8(text);
}

// Inputs state
bool GetWantCaptureMouse() {
	ImGuiIO &io = ImGui::GetIO();
	return io.WantCaptureMouse;
}

bool GetWantCaptureKeyboard() {
	ImGuiIO &io = ImGui::GetIO();
	return io.WantCaptureKeyboard;
}

bool GetWantTextInput() {
	ImGuiIO &io = ImGui::GetIO();
	return io.WantTextInput;
}

void UseGamepad(unsigned int index) {
	ImGuiIO &io = ImGui::GetIO();
	memset(io.NavInputs, 0, sizeof(io.NavInputs));
	auto command = std::string()
		+ "local joysticks = love.joystick.getJoysticks()\n"
		+ "local navs = {}\n"
		+ "if #joysticks >= "+std::to_string(index)+" then\n"
			+ "local joystick = joysticks["+std::to_string(index)+"]\n"
			/* Button */
			+ "navs[" + std::to_string(ImGuiNavInput_Activate) + "] = joystick:isGamepadDown('a') and 1 or 0\n"
			+ "navs[" + std::to_string(ImGuiNavInput_Cancel) + "] = joystick:isGamepadDown('b') and 1 or 0\n"
			+ "navs[" + std::to_string(ImGuiNavInput_Menu) + "] = joystick:isGamepadDown('x') and 1 or 0\n"
			+ "navs[" + std::to_string(ImGuiNavInput_Input) + "] = joystick:isGamepadDown('y') and 1 or 0\n"
			+ "navs[" + std::to_string(ImGuiNavInput_DpadLeft) + "] = joystick:isGamepadDown('dpleft') and 1 or 0\n"
			+ "navs[" + std::to_string(ImGuiNavInput_DpadRight) + "] = joystick:isGamepadDown('dpright') and 1 or 0\n"
			+ "navs[" + std::to_string(ImGuiNavInput_DpadUp) + "] = joystick:isGamepadDown('dpup') and 1 or 0\n"
			+ "navs[" + std::to_string(ImGuiNavInput_DpadDown) + "] = joystick:isGamepadDown('dpdown') and 1 or 0\n"
			+ "navs[" + std::to_string(ImGuiNavInput_FocusPrev) + "] = joystick:isGamepadDown('leftshoulder') and 1 or 0\n"
			+ "navs[" + std::to_string(ImGuiNavInput_FocusNext) + "] = joystick:isGamepadDown('rightshoulder') and 1 or 0\n"
			+ "navs[" + std::to_string(ImGuiNavInput_TweakSlow) + "] = joystick:isGamepadDown('leftshoulder') and 1 or 0\n"
			+ "navs[" + std::to_string(ImGuiNavInput_TweakFast) + "] = joystick:isGamepadDown('rightshoulder') and 1 or 0\n"
			/* Axis */
			+ "navs[" + std::to_string(ImGuiNavInput_LStickLeft) + "] = math.max(joystick:getGamepadAxis('leftx'), 0)\n"
			+ "navs[" + std::to_string(ImGuiNavInput_LStickRight) + "] = math.min(joystick:getGamepadAxis('leftx'), 0)\n"
			+ "navs[" + std::to_string(ImGuiNavInput_LStickUp) + "] = math.min(joystick:getGamepadAxis('lefty'), 0)\n"
			+ "navs[" + std::to_string(ImGuiNavInput_LStickDown) + "] = math.max(joystick:getGamepadAxis('lefty'), 0)\n"
		+ "else\n"
			+ "error('Gamepad index out of bounds')\n"
		+ "end\n"
		+ "return navs\n";
	luaL_dostring(g_L, command.c_str());
	io.BackendFlags |= ImGuiBackendFlags_HasGamepad;
	lua_pushnil(g_L); // first key
	while (lua_next(g_L, -2)) {
		lua_pushvalue(g_L, -2);
		auto i = (unsigned int)luaL_checknumber(g_L, -1);
		float v = luaL_checknumber(g_L, -2);
		io.NavInputs[i] = v;
		lua_pop(g_L, 2);
	}
	lua_pop(g_L, 1);
}

// Fonts
void SetGlobalFontFromFileTTF(const char *path, float size_pixels, float spacing_x, float spacing_y, float oversample_x,
															float oversample_y) {
	ImGuiIO &io = ImGui::GetIO();
	ImFontConfig conf;
	conf.OversampleH         = oversample_x;
	conf.OversampleV         = oversample_y;
	conf.GlyphExtraSpacing.x = spacing_x;
	conf.GlyphExtraSpacing.y = spacing_y;
	io.Fonts->AddFontFromFileTTF(path, size_pixels, &conf);
}

void SetStyleColorU32(unsigned int idx, unsigned int col) {
	ImGuiStyle &style = ImGui::GetStyle();
	style.Colors[idx] = ImGui::ColorConvertU32ToFloat4(col);
}

void SetStyleColorV4(unsigned int idx, float col[4]) {
	ImGuiStyle &style = ImGui::GetStyle();
	style.Colors[idx] = ImVec4(col[0], col[1], col[2], col[3]);
}

bool nearEqaul(float a, float b) {
	if (fabs(a - b) <= FLT_EPSILON) return true;
	return false;
}

void SetStyleValue(const char *name, float x, float y) {
	ImGuiStyle &style = ImGui::GetStyle();
	auto str          = std::string(name);
	std::transform(str.begin(), str.end(), str.begin(), ::tolower);
	// Made lowercase to help with string comparison
	if (str == "alpha")
		style.Alpha = x;
	else if (str == "windowrounding")
		style.WindowRounding = x;
	else if (str == "windowbordersize")
		style.WindowBorderSize = x;
	else if (str == "windowmenubuttonposition") {
		if (nearEqaul(x, -1))
			style.WindowMenuButtonPosition = ImGuiDir_None;
		else if (nearEqaul(x, 0))
			style.WindowMenuButtonPosition = ImGuiDir_Left;
		else if (nearEqaul(x, 1))
			style.WindowMenuButtonPosition = ImGuiDir_Right;
		else if (nearEqaul(x, 2))
			style.WindowMenuButtonPosition = ImGuiDir_Up;
		else if (nearEqaul(x, 3))
			style.WindowMenuButtonPosition = ImGuiDir_Down;
	} else if (str == "childrounding")
		style.ChildRounding = x;
	else if (str == "childbordersize")
		style.ChildBorderSize = x;
	else if (str == "popuprounding")
		style.PopupRounding = x;
	else if (str == "popupbordersize")
		style.PopupBorderSize = x;
	else if (str == "framerounding")
		style.FrameRounding = x;
	else if (str == "framebordersize")
		style.FrameBorderSize = x;
	else if (str == "indentspacing")
		style.IndentSpacing = x;
	else if (str == "columnsminspacing")
		style.ColumnsMinSpacing = x;
	else if (str == "scrollbarsize")
		style.ScrollbarSize = x;
	else if (str == "scrollbarrounding")
		style.ScrollbarRounding = x;
	else if (str == "grabminsize")
		style.GrabMinSize = x;
	else if (str == "grabrounding")
		style.GrabRounding = x;
	else if (str == "tabrounding")
		style.TabRounding = x;
	else if (str == "tabbordersize")
		style.TabBorderSize = x;
	else if (str == "colorbuttonposition") {
		if (nearEqaul(x, -1))
			style.ColorButtonPosition = ImGuiDir_None;
		else if (nearEqaul(x, 0))
			style.ColorButtonPosition = ImGuiDir_Left;
		else if (nearEqaul(x, 1))
			style.ColorButtonPosition = ImGuiDir_Right;
		else if (nearEqaul(x, 2))
			style.ColorButtonPosition = ImGuiDir_Up;
		else if (nearEqaul(x, 3))
			style.ColorButtonPosition = ImGuiDir_Down;
	} else if (str == "mousecursorscale")
		style.MouseCursorScale = x;
	else if (str == "antialiasedlines")
		style.AntiAliasedLines = (nearEqaul(x, 1)) ? true : false;
	else if (str == "antialiasedfill")
		style.AntiAliasedLines = (nearEqaul(x, 1)) ? true : false;
	else if (str == "curvetessellationtol")
		style.CurveTessellationTol = x;
	else if (str == "windowpadding")
		style.WindowPadding = ImVec2(x, y);
	else if (str == "windowminsize")
		style.WindowMinSize = ImVec2(x, y);
	else if (str == "windowtitlealign")
		style.WindowTitleAlign = ImVec2(x, y);
	else if (str == "framepadding")
		style.FramePadding = ImVec2(x, y);
	else if (str == "itemspacing")
		style.ItemSpacing = ImVec2(x, y);
	else if (str == "iteminnerspacing")
		style.ItemInnerSpacing = ImVec2(x, y);
	else if (str == "touchextrapadding")
		style.TouchExtraPadding = ImVec2(x, y);
	else if (str == "buttontextalign")
		style.ButtonTextAlign = ImVec2(x, y);
	else if (str == "selectabletextalign")
		style.SelectableTextAlign = ImVec2(x, y);
	else if (str == "displaywindowpadding")
		style.DisplayWindowPadding = ImVec2(x, y);
	else if (str == "displaywindowpadding")
		style.DisplayWindowPadding = ImVec2(x, y);
	else {
		throw "Unknown style variable: " + std::string(name);
	}
}
