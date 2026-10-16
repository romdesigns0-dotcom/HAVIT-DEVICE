@echo off
call "C:\Program Files (x86)\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" >nul
cl /nologo /MDd /Zi /DLV_LVGL_H_INCLUDE_SIMPLE /DLV_KCONFIG_IGNORE /DLV_CONF_PATH="\"C:/Users/Romar/Documents/HAVIT UUI/lvgl_vscode_project/ui/sim/build-local/lv_conf.h\"" /I"ui" /I"firmware/managed_components/lvgl__lvgl" /I"ui/sim/build-local/_deps/lvgl-build" /I"ui/sim/build-local" "docs/flow-review/capture.c" "ui/ui_ring_overlay.c" /Fo"docs/flow-review/" /Fe"docs/flow-review/capture.exe" /link "ui/sim/build-local/ui/lib-ui.lib" "ui/sim/build-local/_deps/lvgl-build/lib/lvgl.lib" user32.lib gdi32.lib >"docs/flow-review/capture-build.log" 2>&1
if errorlevel 1 exit /b 1
"docs\flow-review\capture.exe" "docs/flow-review/screens"

