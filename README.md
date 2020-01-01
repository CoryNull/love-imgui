# LOVE-IMGUI

[imgui](https://github.com/ocornut/imgui) module for the [LÖVE](https://love2d.org/) game engine including lua bindings based on this [project](https://github.com/patrickriordan/imgui_lua_bindings).
**The main difference is that now by default in this version the return values ordering is reverted.** For instance to retrieve the value from a slider, you need to do:

```lua
floatValue, status = imgui.SliderFloat("SliderFloat", floatValue, 0.0, 1.0);
```

Or if you're not interested to know if the field was modified, just:

```lua
floatValue = imgui.SliderFloat("SliderFloat", floatValue, 0.0, 1.0);
```

To reverse this behavior and receive back the return values from a function first before the modified fields, just call at the beginning of your application:

```lua
imgui.SetReturnValueLast(false)
```

Another notable difference is that enum values are handled using strings (and array of strings) instead of numerical values, for instance to create a window:

```lua
imgui.Begin("Test Window", true, { "ImGuiWindowFlags_AlwaysAutoResize", "ImGuiWindowFlags_NoTitleBar" });
```

Or for a single flag:

```lua
imgui.Begin("Test Window", true, "ImGuiWindowFlags_AlwaysAutoResize");
```

It uses imgui 1.73 and is based on LÖVE 11.1.

## Getting Started

Just build the project, and copy the generated dynamic module next to your love executable or into the LÖVE application data folder (for instance "C:/Users/<user>/AppData/Roaming/LOVE" on Windows or ~/.local/shared/love on Linux).

Pre-built binaries for Windows and Mas OSX are provided in the [releases](https://github.com/slages/love-imgui/releases) page.

## Examples

Simple integration:

```lua
local imgui = require "imgui"

local showTestWindow = false
local showAnotherWindow = false
local floatValue = 0;
local sliderFloat = { 0.1, 0.5 }
local clearColor = { 0.2, 0.2, 0.2 }
local comboSelection = 1
local textValue = "text"

--
-- LOVE callbacks
--
function love.load(arg)
    imgui.Init()

    imgui.SetStyleColorV4(imgui.ImGuiCol_TabActive, 0.00, 0.44, 1.00, 1.00)
    imgui.SetStyleColorV4(imgui.ImGuiCol_TabHovered, 0.26, 0.42, 0.98, 0.80)
    imgui.SetStyleColorV4(imgui.ImGuiCol_Tab, 0.00, 0.07, 0.79, 0.86)
    imgui.SetStyleColorV4(imgui.ImGuiCol_ResizeGripActive, 0.44, 0.26, 0.98, 0.95)
    imgui.SetStyleColorV4(imgui.ImGuiCol_ResizeGripHovered, 0.26, 0.98, 0.87, 0.67)
    imgui.SetStyleColorV4(imgui.ImGuiCol_ResizeGrip, 0.29, 0.26, 0.98, 0.25)
    imgui.SetStyleColorV4(imgui.ImGuiCol_SeparatorActive, 0.01, 0.00, 1.00, 1.00)
    imgui.SetStyleColorV4(imgui.ImGuiCol_SeparatorHovered, 0.01, 0.00, 1.00, 0.78)
    imgui.SetStyleColorV4(imgui.ImGuiCol_Separator, 1.00, 1.00, 1.00, 0.50)
    imgui.SetStyleColorV4(imgui.ImGuiCol_HeaderActive, 0.98, 0.31, 0.26, 1.00)
    imgui.SetStyleColorV4(imgui.ImGuiCol_HeaderHovered, 0.98, 0.10, 0.10, 0.80)
    imgui.SetStyleColorV4(imgui.ImGuiCol_Header, 1.00, 0.07, 0.00, 0.45)
    imgui.SetStyleColorV4(imgui.ImGuiCol_ButtonActive, 0.98, 0.06, 0.30, 1.00)
    imgui.SetStyleColorV4(imgui.ImGuiCol_ButtonHovered, 0.98, 0.26, 0.50, 1.00)
    imgui.SetStyleColorV4(imgui.ImGuiCol_Button, 1.00, 0.00, 0.46, 0.40)
    imgui.SetStyleColorV4(imgui.ImGuiCol_SliderGrabActive, 0.98, 0.86, 0.26, 1.00)
    imgui.SetStyleColorV4(imgui.ImGuiCol_SliderGrab, 0.88, 0.79, 0.24, 1.00)
    imgui.SetStyleColorV4(imgui.ImGuiCol_CheckMark, 0.98, 0.90, 0.26, 1.00)
    imgui.SetStyleColorV4(imgui.ImGuiCol_ScrollbarGrabActive, 0.00, 0.91, 0.90, 1.00)
    imgui.SetStyleColorV4(imgui.ImGuiCol_ScrollbarGrabHovered, 1.00, 0.43, 0.00, 1.00)
    imgui.SetStyleColorV4(imgui.ImGuiCol_ScrollbarGrab, 0.81, 1.00, 0.00, 1.00)
    imgui.SetStyleColorV4(imgui.ImGuiCol_ScrollbarBg, 0.97, 0.48, 0.00, 0.09)
    imgui.SetStyleColorV4(imgui.ImGuiCol_MenuBarBg, 0.45, 0.00, 0.18, 1.00)
    imgui.SetStyleColorV4(imgui.ImGuiCol_TitleBgCollapsed, 1.00, 0.00, 0.00, 0.51)
    imgui.SetStyleColorV4(imgui.ImGuiCol_TitleBgActive, 1.00, 0.00, 0.27, 1.00)
    imgui.SetStyleColorV4(imgui.ImGuiCol_TitleBg, 0.64, 0.00, 0.19, 1.00)
    imgui.SetStyleColorV4(imgui.ImGuiCol_FrameBgActive, 0.98, 0.26, 0.57, 0.67)
    imgui.SetStyleColorV4(imgui.ImGuiCol_FrameBgHovered, 0.98, 0.26, 0.74, 0.40)
    imgui.SetStyleColorV4(imgui.ImGuiCol_FrameBg, 0.00, 0.48, 0.39, 0.54)
    imgui.SetStyleColorV4(imgui.ImGuiCol_BorderShadow, 0.00, 0.02, 1.00, 0.00)
    imgui.SetStyleColorV4(imgui.ImGuiCol_Border, 0.00, 1.00, 0.98, 0.50)
    imgui.SetStyleColorV4(imgui.ImGuiCol_PopupBg, 0.27, 0.28, 0.15, 0.23)
    imgui.SetStyleColorV4(imgui.ImGuiCol_ChildBg, 0.28, 0.15, 0.26, 0.23)
    imgui.SetStyleColorV4(imgui.ImGuiCol_WindowBg, 0.23, 0.12, 0.28, 0.23)
    imgui.SetStyleColorV4(imgui.ImGuiCol_TextDisabled, 0.39, 0.39, 0.39, 1.00)
    imgui.SetStyleColorV4(imgui.ImGuiCol_Text, 0.00, 0.95, 1.00, 1.00)
    imgui.SetStyleColorV4(imgui.ImGuiCol_TextSelectedBg, 0.14, 0.16, 0.00, 1.00)

    imgui.SetStyleValue("WindowRounding", 0);
    imgui.SetStyleValue("WindowBorderSize", 1.5);
    imgui.SetStyleValue("ChildRounding", 0);
    imgui.SetStyleValue("ChildBorderSize", 1.5);
    imgui.SetStyleValue("PopupRounding", 0);
    imgui.SetStyleValue("PopupBorderSize", 1.5);
    imgui.SetStyleValue("FrameRounding", 0);
    imgui.SetStyleValue("FrameBorderSize", 0.5);
    imgui.SetStyleValue("ScrollbarRounding", 0);
    imgui.SetStyleValue("GrabRounding", 0);
    imgui.SetStyleValue("TabRounding", 0);
    imgui.SetStyleValue("AntiAliasedLines", 1);
    imgui.SetStyleValue("AntiAliasedFill", 1);
end

function love.update(dt)
    imgui.UseGamepad(1)
    imgui.NewFrame()
end

function love.draw()

    -- Menu
    if imgui.BeginMainMenuBar() then
        if imgui.BeginMenu("File") then
            imgui.MenuItem("Test")
            imgui.EndMenu()
        end
        imgui.EndMainMenuBar()
    end

    -- Debug window
    imgui.Text("Hello, world!");
    clearColor[1], clearColor[2], clearColor[3] = imgui.ColorEdit3("Clear color", clearColor[1], clearColor[2], clearColor[3]);

    -- Sliders
    floatValue = imgui.SliderFloat("SliderFloat", floatValue, 0.0, 1.0);
    sliderFloat[1], sliderFloat[2] = imgui.SliderFloat2("SliderFloat2", sliderFloat[1], sliderFloat[2], 0.0, 1.0);

    -- Combo
    comboSelection = imgui.Combo("Combo", comboSelection, { "combo1", "combo2", "combo3", "combo4" }, 4);

    -- Windows
    if imgui.Button("Test Window") then
        showTestWindow = not showTestWindow;
    end

    if imgui.Button("Another Window") then
        showAnotherWindow = not showAnotherWindow;
    end

    if showAnotherWindow then
        imgui.SetNextWindowPos(50, 50, "ImGuiCond_FirstUseEver")
        showAnotherWindow = imgui.Begin("Another Window", true, { "ImGuiWindowFlags_AlwaysAutoResize", "ImGuiWindowFlags_NoTitleBar" });
        imgui.Text("Hello");
        -- Input text
        textValue = imgui.InputTextMultiline("InputText", textValue, 200, 300, 200);
        imgui.End();
    end

    if showTestWindow then
        showTestWindow = imgui.ShowDemoWindow(true)
    end

    love.graphics.clear(clearColor[1], clearColor[2], clearColor[3])
    imgui.Render();
end

function love.quit()
    imgui.ShutDown();
end

--
-- User inputs
--
function love.textinput(t)
    imgui.TextInput(t)
    if not imgui.GetWantCaptureKeyboard() then
        -- Pass event to the game
    end
end

function love.keypressed(key)
    imgui.KeyPressed(key)
    if not imgui.GetWantCaptureKeyboard() then
        -- Pass event to the game
    end
end

function love.keyreleased(key)
    imgui.KeyReleased(key)
    if not imgui.GetWantCaptureKeyboard() then
        -- Pass event to the game
    end
end

function love.mousemoved(x, y)
    imgui.MouseMoved(x, y)
    if not imgui.GetWantCaptureMouse() then
        -- Pass event to the game
    end
end

function love.mousepressed(x, y, button)
    imgui.MousePressed(button)
    if not imgui.GetWantCaptureMouse() then
        -- Pass event to the game
    end
end

function love.mousereleased(x, y, button)
    imgui.MouseReleased(button)
    if not imgui.GetWantCaptureMouse() then
        -- Pass event to the game
    end
end

function love.wheelmoved(x, y)
    imgui.WheelMoved(y)
    if not imgui.GetWantCaptureMouse() then
        -- Pass event to the game
    end
end
```

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details
