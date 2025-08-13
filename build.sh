#!/usr/bin/env sh

Compiler="clang"
GO_FAST=0

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

Source_File="../source/$Source_File_Name.c"

mkdir -p build
cd build

Settings="-std=c99 -Wall -Wextra -Wstrict-prototypes -Wold-style-definition -Wno-comment"
Settings="$Settings -Wno-unused-function"
Settings="$Settings -Wno-unused-parameter"
# Settings="$Settings -Wmissing-prototypes -Wmissing-declarations"

Graphics_Frameworks="-framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL"
Graphics_Lib="../libraries/libraylib.a"

Executable_File="-o $Source_File_Name.out"

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



# Toggle settings
Settings="$Settings -Wno-switch"
Settings="$Settings -Wno-unused-variable"
Settings="$Settings -Wno-char-subscripts"
Settings="$Settings -Wno-sign-compare"
Settings="$Settings -fno-inline-functions"
# Settings="$Settings -fno-pie"
# Settings="$Settings -E"


Settings="$Settings $Graphics_Frameworks $Graphics_Lib"



Compiler_Args="$Source_File $Executable_File $Target $Debug $Settings"
echo
echo "Compiling $Source_File_Name"
echo "    $Compiler_Args"
$Compiler $Compiler_Args
