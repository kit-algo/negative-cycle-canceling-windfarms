set(OGDF_HOME "${OGDF_HOME}" CACHE PATH "Path to the OGDF directory.")

message(STATUS "")
message(STATUS "OGDF:")

mark_as_advanced(OGDF_LIBRARY OGDF_LIBRARY_DIR OGDF_INCLUDE_DIR)

find_path(  OGDF_INCLUDE_DIR
    NAMES   ogdf/basic/Graph.h
    HINTS   ${OGDF_HOME}/include/
            /opt/ogdf/include/
            /usr/local/include/
         )
message(STATUS "\tOGDF_INCLUDE_DIR:\t\t${OGDF_INCLUDE_DIR}")

find_library(OGDF_LIBRARY
    libOGDF.a libOGDF.so
    HINTS   ${OGDF_HOME}/
            ${OGDF_HOME}/lib/
            /usr/local/lib/
            /opt/ogdf/lib/
            /usr/local/lib/ogdf/
            )
message(STATUS "\tOGDF_LIBRARY:\t\t\t${OGDF_LIBRARY}")

# Set OGDF_FOUND variable and handles the REQUIRED argument
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(OGDF DEFAULT_MSG OGDF_LIBRARY OGDF_INCLUDE_DIR) #OGDF_AUTOGEN_INCLUDE_DIR
