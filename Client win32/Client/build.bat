@echo off

mkdir "../build"
pushd "../build"
cl -MDd -Zi -Od -Oi -fp:fast -nologo -I "../../sdl/include" /FeClient /D win32 /D DEBUG "../Client/Main.cpp" ../../sdl/lib/x86/SDL2.lib ../../sdl/lib/x86/SDL2main.lib ../../sdl/lib/x86/SDL2_ttf.lib ../../sdl/lib/x86/SDL2_Net.lib ../../sdl/lib/x86/SDL2_image.lib -link -SUBSYSTEM:WINDOWS kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /NODEFAULTLIB:msvcrt.lib
REM cl /GS /W3 /Zc:wchar_t /I "../../sdl/include" /Zi /Od /sdl /fp:fast /D "win32" /D "_MBCS" /errorReport:prompt /WX- /Zc:forScope /RTC1 /Gd /Oy- /MDd /nologo /FeClient /Tp "../Client/Main.cpp" ../../sdl/lib/x86/SDL2.lib ../../sdl/lib/x86/SDL2main.lib ../../sdl/lib/x86/SDL2_ttf.lib ../../sdl/lib/x86/SDL2_Net.lib ../../sdl/lib/x86/SDL2_image.lib -link -SUBSYSTEM:WINDOWS
popd
