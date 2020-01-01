// Lua
extern "C" {
  #define LUA_COMPAT_ALL
  #include <lauxlib.h>
  #include <lua.h>
  #include <lualib.h>
}

bool Init(lua_State *L);
void ShutDown();
void NewFrame();
// Inputs
void MouseMoved(int x, int y);
void MousePressed(int button);
void MouseReleased(int button);
void WheelMoved(int y);
void KeyPressed(const char *key);
void KeyReleased(const char *key);
void TextInput(const char *text);
// Inputs state
bool GetWantCaptureMouse();
bool GetWantCaptureKeyboard();
bool GetWantTextInput();
void UseGamepad(unsigned int index);
// Fonts
void SetGlobalFontFromFileTTF(const char *path, float size_pixels, float spacing_x, float spacing_y, float oversample_x,
															float oversample_y);

void SetStyleColorU32(unsigned int idx, unsigned int col);
void SetStyleColorV4(unsigned int idx, float col[4]);
void SetStyleValue(const char* name, float x, float y = 0.0);
