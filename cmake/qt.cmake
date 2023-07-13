# Find QT5 
# Once done this will define
# 
# Qt5_FOUND         - system has Qt5
# QT5_LIBRARIES     - the libraries needed to use Qt5
# QTCORE_LOCATION   - location of the module Qt5::Core
# QTQUICK_LOCATION  - location of the module Qt5::Quick

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

# AUTOUIC is a boolean specifying whether CMake will handle the Qt uic code
# generator automatically, i.e. without having to use the QT4_WRAP_UI() or
# QT5_WRAP_UI() macro. Currently Qt4 and Qt5 are supported.
# 
# https://cmake.org/cmake/help/v3.0/prop_tgt/AUTOUIC.html
# 
# Note that without enabling it headers such as "ui_mainwindow.h" and "ui_*.h"
# will not be found. 
set(CMAKE_AUTOUIC ON)

message(STATUS "Looking for Qt5")
FIND_PACKAGE(Qt5 COMPONENTS Core REQUIRED) # 
get_target_property(QTCORE_LOCATION Qt5::Core LOCATION)
add_definitions( ${QT_DEFINITIONS} )

if(Qt5_FOUND)
  message( STATUS "\tFound Qt5 Core module in \"${QTCORE_LOCATION}\" " )
endif()

set ( QT5_LIBRARIES Qt5::Core )
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(QT5 DEFAULT_MSG QT5_LIBRARIES)