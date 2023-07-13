cmake_minimum_required(VERSION 3.5.2)
project(WinfarmCabling CXX)

set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS Debug Release)

set(CMAKE_CXX_STANDARD 14)

# Build options
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/bin)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)

if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -march=native")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wshadow")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Ofast")
endif()

####################################################################
# Find external libraries ##########################################
#################################################################### 
include(qt)
find_package(OGDF REQUIRED)

####################################################################
# Compilation ######################################################
####################################################################
file(GLOB_RECURSE SOURCES src/*.cpp ${PROJECT_SOURCE_DIR}/*.h)
list(REMOVE_ITEM SOURCES ${PROJECT_SOURCE_DIR}/src/Runnables/main.cpp)

################################### libraries ########################
add_library(WCP STATIC)
target_include_directories(WCP PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_include_directories(WCP SYSTEM PUBLIC ${OGDF_INCLUDE_DIR} ${GUROBI_INCLUDE_DIR})
target_link_libraries(WCP PUBLIC ${OGDF_LIBRARY} ${GUROBI_LIBRARIES} ${QT5_LIBRARIES})
target_sources(WCP PRIVATE ${SOURCES})
## end library, continue compilation

add_executable(windfarmCabling ${PROJECT_SOURCE_DIR}/src/Runnables/main.cpp)
target_link_libraries(windfarmCabling PUBLIC WCP)

####################################################################
# Documentation ####################################################
####################################################################
message( STATUS "")
message( STATUS "Compiler:")
# the compiler used for C++ files
message( STATUS "\tCMAKE_CXX_COMPILER:\t\t" ${CMAKE_CXX_COMPILER} )
# Verbose mode - make VERBOSE = 1?
message( STATUS "\tmake VERBOSE = 1? :\t\t" ${CMAKE_VERBOSE_MAKEFILE} )

message( STATUS "")
message( STATUS "Other important targets:")
# QT5 found
if(Qt5_FOUND)
  message(STATUS "\tqt5:\t\t\t\tadd libraries of QT5 (in-source)")
endif(Qt5_FOUND) 

# OGDF found
if(OGDF_FOUND)
  message(STATUS "\tOGDF:\t\t\t\tadd libraries of OGDF (in-source)")
endif(OGDF_FOUND)

message( STATUS "")
message( STATUS "Build type is set to:\t\t" ${CMAKE_BUILD_TYPE})
message( STATUS "Build files have been written to:\t" ${CMAKE_CURRENT_BINARY_DIR} )
message( STATUS "Runtime output directory:\t\t" ${CMAKE_RUNTIME_OUTPUT_DIRECTORY} )
message( STATUS "Library output directory:\t\t" ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )
message( STATUS "")
