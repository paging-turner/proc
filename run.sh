#!/usr/bin/env sh

# The app needs to be invoked while inside the "build" directory so that the paths to things like config files will work.
# This script is a convenience so that you can run the app from the root of the project (for people who want to build/run from the command-line).
cd build
./proc.out
