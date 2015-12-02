project(BrowserApp)
cmake_minimum_required(VERSION 2.8)
#
########## Defines ###########
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res" CACHE STRING "The path of resources")
SET(WEBKITDIR "/opt/usr/data/webkit" CACHE STRING "The path of web engine")
SET(EDJE_DIR "${RESDIR}/edje/")
SET(CERTS_DIR "${RESDIR}/certs/")
SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")
SET(GCC_COVERAGE_COMPILE_FLAGS "-fprofile-arcs -ftest-coverage")

if(NOT DEFINED PACKAGE_NAME)
    SET(PACKAGE_NAME "org.tizen.browser")
endif(NOT DEFINED PACKAGE_NAME)

if(NOT DEFINED MANIFESTDIR)
    SET(MANIFESTDIR "/usr/share/packages")
endif(NOT DEFINED MANIFESTDIR)

if(NOT DEFINED ICONDIR)
    SET(ICONDIR "/usr/share/icons/default/small/")
endif(NOT DEFINED ICONDIR)

if(NOT DEFINED DEMODIR)
    SET(DEMODIR "/usr/apps/org.tizen.browser/res/demo/")
endif(NOT DEFINED DEMODIR)

option(BUILD_UT "Build Unit tests" OFF)
option(TIZEN_BUILD "Build for Tizen. Default ON" ON)
option(DYN_INT_LIBS "Buils internal libs as dynamic" ON)
option(COVERAGE_STATS "Code coverage" OFF)

#Enable C++11 support
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)


if (COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -O0 -g -ggdb3")
    elseif(COMPILER_SUPPORTS_CXX0X)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -O0 -g -ggdb3")
else()
    message(FATAL_ERROR "The Compiler ${CMAKE_CXX_COMPILER} doesn't support C++11")
endif()

set(BOOST_LIBS regex filesystem system date_time)
if(BUILD_UT)
    set(BOOST_LIBS ${BOOST_LIBS} unit_test_framework)
ENDIF(BUILD_UT)
find_package(Boost 1.49.9 REQUIRED COMPONENTS ${BOOST_LIBS})

#RPATH settings
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib:${CMAKE_INSTALL_PREFIX}/services")

#disable for merge - WebView is not ready for this
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wno-variadic-macros -Wno-long-long -Werror")

if(COVERAGE_STATS)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
endif(COVERAGE_STATS)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

# TIZEN SPECIFIC
# set pkg-config modules
if(TIZEN_BUILD)
    include(FindPkgConfig)

    set(PLATFORM_TIZEN ON)
    SET(tizen_pkgs_list
        dlog
        capi-appfw-application
        libpng
    )
    if (${PROFILE} MATCHES "mobile")
    SET(tizen_pkgs_list
        ${tizen_pkgs_list}
      	 capi-content-media-content
      	 capi-appfw-app-manager
      	 notification
      	 appsvc
    )
	endif (${PROFILE} MATCHES "mobile")
    pkg_check_modules(tizen_pkgs REQUIRED "${tizen_pkgs_list}")

    FOREACH(flag ${tizen_pkgs_CFLAGS})
        SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${flag}")
    ENDFOREACH(flag)

    message("${CMAKE_CXX_FLAGS}")
    MACRO (TODAY RESULT)
        EXECUTE_PROCESS(COMMAND "date" "+%d-%m-%Y" OUTPUT_VARIABLE ${RESULT})
        STRING(REGEX REPLACE "(..)-(..)-(....).*" "\\1-\\2-\\3" ${RESULT} ${${RESULT}})
    ENDMACRO (TODAY)

    TODAY(BUILD_DATE)

    ADD_DEFINITIONS(
        -DBUILD_DATE=\"${BUILD_DATE}\"
        -DVERSION=\"${VERSION}\"
        -D__BUILD__
        -DM_VCS_ID=\"${VCS_ID}\"
        -DTIZEN_BUILD=\"${TIZEN_BUILD}\"
        -DCERTS_DIR=\"${CERTS_DIR}\"
        )

    if (${PROFILE} MATCHES "mobile")
        ADD_DEFINITIONS(-DPROFILE_MOBILE=1)
    endif (${PROFILE} MATCHES "mobile")

endif(TIZEN_BUILD)

ADD_DEFINITIONS(-DEDJE_DIR=\"${EDJE_DIR}\")

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR})

#services
add_subdirectory(services)

#browser core
add_subdirectory(core)

#compiling EDC files
#
#Temporary location of EDC files
#
# probably should be moved to Tizen section.
#to build unit tests add --define "build_unit_test ON" parameter to gbs build command
if(BUILD_UT)
  add_subdirectory(unit_tests)
ENDIF(BUILD_UT)

configure_file(${CMAKE_SOURCE_DIR}/cmakeconfig.h.cmake ${CMAKE_BINARY_DIR}/cmakeconfig.h)

if(TIZEN_BUILD)
if (${PROFILE} MATCHES "mobile")
configure_file(${CMAKE_SOURCE_DIR}/manifest.xml.in.mb ${CMAKE_BINARY_DIR}/${PACKAGE_NAME}.xml)
else (${PROFILE} MATCHES "mobile")
configure_file(${CMAKE_SOURCE_DIR}/manifest.xml.in ${CMAKE_BINARY_DIR}/${PACKAGE_NAME}.xml)
endif (${PROFILE} MATCHES "mobile")

install(FILES ${CMAKE_BINARY_DIR}/${PACKAGE_NAME}.xml DESTINATION ${MANIFESTDIR})
# install desktop file & icon
install(FILES ${CMAKE_SOURCE_DIR}/org.tizen.browser.png DESTINATION ${ICONDIR})
install(FILES ${CMAKE_SOURCE_DIR}/apps_img_web_default_4x2.png DESTINATION ${ICONDIR})
# install demo files
install(FILES ${CMAKE_SOURCE_DIR}/demo/b.htm DESTINATION ${DEMODIR})
install(FILES ${CMAKE_SOURCE_DIR}/demo/Widget.htm DESTINATION ${DEMODIR})
install(FILES ${CMAKE_SOURCE_DIR}/demo/header.jpg DESTINATION ${DEMODIR})
install(FILES ${CMAKE_SOURCE_DIR}/demo/main_bg.jpg DESTINATION ${DEMODIR})
# install certs
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/certs/ DESTINATION ${CERTS_DIR})
endif(TIZEN_BUILD)

message(STATUS "------------------------------------------")
message(STATUS "Build for Tizen          (TIZEN_BUILD)     :" ${TIZEN_BUILD})
message(STATUS "Interal libs are dynamic (DYN_INT_LIBS)    :" ${DYN_INT_LIBS})
message(STATUS "Building Unit tests      (BUILD_UT)        :" ${BUILD_UT})
message(STATUS "Code Coverage statistics (COVERAGE_STATS)  :" ${COVERAGE_STATS})
message(STATUS "Device profile           (PROFILE)         :" ${PROFILE})
message(STATUS "------------------------------------------")
