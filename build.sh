#!/usr/bin/env sh


Compiler="clang"

GO_FAST=0
DO_NOT_Compile_With_Custom_Keybinds=0

if [ $GO_FAST -eq 1 ]; then
    Debug=0
    Optimized=1
    No_Assert=1
else
    Debug=1
    Optimized=0
    No_Assert=0
fi


if [ $# -eq 0 ]; then
    Source_File_Name="proc"
else
    Source_File_Name=$1
fi


mkdir -p build
cd build


if [ $Debug -eq 0 ]; then
    Debug=""
elif [ $Debug -eq 1 ]; then
    Debug="-g3"
fi

if [ $Optimized -eq 0 ]; then
    Target="-O0"
elif [ $Optimized -eq 1 ]; then
    Target="-O2"
fi

if [ $No_Assert -eq 1 ]; then
    Settings="$Settings -DNo_Assert"
fi

if [ $GO_FAST -eq 1 ]; then
    Settings="$Settings -DGO_FAST"
fi

Source_File="../source/$Source_File_Name.c"

Base_File_Name="base"
Base_File="../source/$Base_File_Name.c"

Should_Compile_With_Custom_Keybinds=0
Custom_File="../config/custom_keybinds.c"
if [ -f $Custom_File ]; then
    Should_Compile_With_Custom_Keybinds=1
fi

if [ $DO_NOT_Compile_With_Custom_Keybinds == 1 ]; then
    Should_Compile_With_Custom_Keybinds=0
fi

Base_Object_File="$Base_File_Name.o"
Custom_Object_File="custom.o"
Executable_File="$Source_File_Name.out"

Graphics_Frameworks="-framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL"
Graphics_Lib="../libraries/raylib-5.5_macos/lib/libraylib.a"

Settings="-std=c99 -Wall -Wextra -Wstrict-prototypes -Wold-style-definition -Wno-comment"
# Toggle settings
Settings="$Settings -Wno-unused-function"
Settings="$Settings -Wno-unused-parameter"
# Settings="$Settings -Wmissing-prototypes -Wmissing-declarations"
Settings="$Settings -Wno-switch"
Settings="$Settings -Wno-unused-variable"
Settings="$Settings -Wno-char-subscripts"
Settings="$Settings -Wno-sign-compare"
Settings="$Settings -fno-inline-functions"

if [ -e $Custom_File ]; then
    Settings="$Settings -DCustom_Keybinds"
fi

Settings="$Settings -fno-pie"  # No-pie is used for symbol-sets at the moment
# Settings="$Settings -E"




Base_Settings="-c $Settings"
App_Settings="$Settings $Graphics_Frameworks $Graphics_Lib"



Base_Args="$Base_File -o $Base_Object_File $Target $Debug $Base_Settings"
Custom_Args=""

App_Objects_To_Link=""

echo
echo "Compiling $Base_File_Name"
echo "    $Base_Args"
$Compiler $Base_Args

if [ $Should_Compile_With_Custom_Keybinds == 1 ]; then
    App_Objects_To_Link="$Base_Object_File $Custom_Object_File"
    Custom_Args="$Custom_File -o $Custom_Object_File $Target $Debug $Base_Settings"
    echo
    echo "Compiling $Custom_File"
    echo "    $Custom_Args"
    $Compiler $Custom_Args
else
    App_Objects_To_Link="$Base_Object_File"
fi

App_Args="$Source_File -o $Executable_File $App_Objects_To_Link $Target $Debug $App_Settings"

echo
echo "Compiling $Source_File_Name"
echo "    $App_Args"
$Compiler $App_Args
