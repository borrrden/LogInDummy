cmake_minimum_required (VERSION 3.1)
cmake_policy(VERSION 3.1)
project (LogInDummy)
INCLUDE(CheckIncludeFile)

if(WIN32)
    set(CMAKE_REQUIRED_DEFINITIONS -DIBM)
    add_definitions(-DIBM)
    set(variables
        CMAKE_C_FLAGS_DEBUG
        CMAKE_C_FLAGS_MINSIZEREL
        CMAKE_C_FLAGS_RELEASE
        CMAKE_C_FLAGS_RELWITHDEBINFO
        CMAKE_CXX_FLAGS_DEBUG
        CMAKE_CXX_FLAGS_MINSIZEREL
        CMAKE_CXX_FLAGS_RELEASE
        CMAKE_CXX_FLAGS_RELWITHDEBINFO)

        foreach(variable ${variables})
            string(REPLACE "/MD" "/MT" ${variable} "${${variable}}")
        endforeach()
elseif(APPLE)
    set(CMAKE_REQUIRED_DEFINITIONS -DAPL)
    add_definitions(-DAPL)
else()
    set(CMAKE_REQUIRED_DEFINITIONS -DLIN)
    add_definitions(-DLIN)
endif()

add_definitions(-DXPLM200 -DXPLM210 -DXPLM300 -DXPLM301)

include_directories("XPlaneSDK/CHeaders/XPLM" "XPlaneSDK/CHeaders/Widgets")
set(CMAKE_REQUIRED_INCLUDES "${CMAKE_SOURCE_DIR}/XPlaneSDK/CHeaders/XPLM/" "${CMAKE_SOURCE_DIR}/XPlaneSDK/CHeaders/Widgets/")
set(HEADER_FILES XPLMDataAccess.h XPLMPlugin.h XPLMProcessing.h XPLMUtilities.h XPLMMenus.h
    XPStandardWidgets.h XPWidgets.h)
foreach(header_file ${HEADER_FILES})
    unset(HAVE_XPLANE_HEADER CACHE)
    check_include_file(${header_file} HAVE_XPLANE_HEADER)
    if(NOT HAVE_XPLANE_HEADER)
        message(FATAL_ERROR "Missing or corrupted required X-Plane SDK header!  See XPlaneSDK/INSTRUCTIONS.txt")
    endif()
endforeach()

if(WIN32)
    add_library(LogInDummy SHARED "src/DllMain.cpp" "src/LogInDummy.c" "src/Preferences.cc")
    find_library(XPLM_LIB NAMES XPLM_64 HINTS "XPlaneSDK/Libraries/Win")
     if(NOT XPLM_LIB)
         message(FATAL_ERROR "XPLM_64.lib not found.  X-Plane SDK is corrupted")
     endif()

     find_library(XPWIDGETS_LIB NAMES XPWidgets_64 HINTS "XPlaneSDK/Libraries/Win")
     if(NOT XPWIDGETS_LIB)
         message(FATAL_ERROR "XPWidgets_64.lib not found.  X-Plane SDK is corrupted.")
     endif()

     target_link_libraries(LogInDummy ${XPLM_LIB} ${XPWIDGETS_LIB})
elseif(APPLE)
    add_library(LogInDummy SHARED "src/LogInDummy.c" "src/Preferences.cc")
    find_library(XPLM_LIB NAMES XPLM HINTS "XPlaneSDK/Libraries/Mac")
    if(NOT XPLM_LIB)
        message(FATAL_ERROR "XPLM.framework not found.  X-Plane SDK is corrupted")
    endif()

    find_library(XPWIDGETS_LIB NAMES XPWidgets HINTS "XPlaneSDK/Libraries/Mac")
    if(NOT XPWIDGETS_LIB)
        message(FATAL_ERROR "XPWidgets.framework not found.  X-Plane SDK is corrupted.")
    endif()

    target_link_libraries(LogInDummy ${XPLM_LIB} ${XPWIDGETS_LIB})
else()
    add_library(LogInDummy SHARED "src/LogInDummy.c" "src/Preferences.cc")
    set_target_properties(LogInDummy PROPERTIES LINK_FLAGS "-nodefaultlibs -undefined_warning")
endif()

set_target_properties(LogInDummy PROPERTIES SUFFIX ".xpl")
