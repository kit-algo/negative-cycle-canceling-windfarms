#!/bin/sh
# This script can be used for easier compiling by avoiding a manual configuration
# using cmake.
#
# Usage: ./useThisToBuild.sh BUILD_MODE CLEAN_BUILD_DIR
#
# BUILD_MODE: Use either 'Debug' or 'Release' to specify the build mode.
# CLEAN_BUILD_DIR: Use either 'TRUE' or 'FALSE' to specify whether cmake files
#       should be removed from the build directory (see below) prior to building.
#
# Further information must be provided prior to invoking this script,
# directly after this comment block.
# CXX_COMPILER: Full path to the C++-compiler.
# BUILD_DIR_RELEASE/BUILD_DIR_DEBUG: Full path to the directories in which the
#       program should be built in the respective build mode. These directories
#       must exist already.
# VERBOSE_MAKEFILE: Boolean-type variable to specify whether the building process
#       during the use of make-files should be verbose.
# OGDF_HOME_RELEASE/OGDF_HOME_DEBUG: Full path to the already built (!) OGDF
#       depending on the build mode. Make sure to use a snapshot not later than 2017-07-23.
#

### -- Beginning of place for further information -- ###

# Specify paths to compiler.
CXX_COMPILER="/usr/bin/g++"

# Specify build directories for both release and debug builds. Make sure the directories exist already.
BUILD_DIR_RELEASE="/build-release"
BUILD_DIR_DEBUG="/build-debug"

# Should the make file be verbose or not?
VERBOSE_MAKEFILE="FALSE"

# OGDF - tested with snapshot 2017-07-23 - does not work with snapshot 2018-03-28 (or later).
OGDF_HOME_RELEASE="/ogdf-release"
OGDF_HOME_DEBUG="/ogdf-debug"

###    -- End of place for further information --    ###


# Which build mode, i.e., Debug or Release, do we want to use?
if [ -n "$1" ]; then
    BUILD_MODE=$1
else
    BUILD_MODE="Release"
fi

# Do we want to remove cmake files from the build directories before building?
if [ -n "$2" ]; then
    CLEAN_BUILD_DIR=$2
else
    CLEAN_BUILD_DIR="FALSE"
fi

# Infer build directory from build mode.
if [ "$BUILD_MODE" = "Release" ]; then
    BUILD_DIR="$BUILD_DIR_RELEASE"
else
    BUILD_DIR="$BUILD_DIR_DEBUG"
fi


cd "$BUILD_DIR"

# Clean build directory
if [ $CLEAN_BUILD_DIR = "TRUE" ]; then
    echo "\n-- Remove cmake files from $(pwd)\n";
    /bin/rm -rf "./CMakeFiles"
    /bin/rm -f "./CMakeCache.txt"
fi

# Configure and show configuration of the project
if [ $BUILD_MODE = "Release" ]; then
    cmake .. \
        -DCMAKE_BUILD_TYPE="Release" \
        -DCMAKE_CXX_COMPILER="$CXX_COMPILER" \
        -DCMAKE_VERBOSE_MAKEFILE="$VERBOSE_MAKEFILE" \
        -DOGDF_HOME="$OGDF_HOME_RELEASE"
else
    cmake .. \
        -DCMAKE_BUILD_TYPE="Debug" \
        -DCMAKE_CXX_COMPILER="$CXX_COMPILER" \
        -DCMAKE_VERBOSE_MAKEFILE="$VERBOSE_MAKEFILE" \
        -DOGDF_HOME="$OGDF_HOME_DEBUG"
fi

echo

# Build project
cmake --build .

# Show general information
echo "\n-- The build mode is set to:\t\t$BUILD_MODE"
echo "-- The build directory is set to:\t$BUILD_DIR"
echo "-- Current directory is:\t\t$(pwd)\n"
