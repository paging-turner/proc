echo off

set Base_Source_File=source\base.c
set App_Source_File=source\proc.c

set Base_Obj_File=build\base.obj
set App_Exe_File=build\proc.exe

set Raylib_Lib=libraries\raylib-5.5_win64_msvc16\lib\raylib.lib

set App_Windows_Libs=opengl32.lib kernel32.lib user32.lib gdi32.lib winmm.lib Shell32.lib Advapi32.lib Userenv.lib

cl %Base_Source_File% /c /Fo%Base_Obj_File%

REM  /link %App_Windows_Libs% /NODEFAULTLIB:MSVCRT
cl %App_Source_File% %Base_Obj_File% /Fe%App_Exe_File% /Zi /MD %Raylib_Lib% %App_Windows_Libs% /link /NODEFAULTLIB:LIBCMT
