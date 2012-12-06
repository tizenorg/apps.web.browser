#
# Copyright 2012  Samsung Electronics Co., Ltd
#
# Licensed under the Flora License, Version 1.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.tizenopensource.org/license
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(browser CXX)

SET(SRCS
	src/browser-class.cpp
	src/browser-window.cpp
	src/browser-utility.cpp
	src/browser-main.cpp
	src/browser-common-view.cpp
	src/browser-data-manager.cpp
	src/browser-policy-client/browser-policy-decision-maker.cpp
	src/browser-extension/browser-geolocation.cpp
	src/browser-extension/browser-find-word.cpp
	src/browser-view/browser-view.cpp
	src/browser-view/browser-context-menu.cpp
	src/browser-multi-window/browser-multi-window-view.cpp
	src/data-network/browser-network-manager.cpp
	src/browser-bookmark/browser-bookmark-view.cpp
	src/browser-bookmark/browser-add-to-bookmark-view.cpp
	src/browser-bookmark/browser-new-folder-view.cpp
	src/browser-bookmark/browser-select-folder-view.cpp
	src/browser-history/browser-history-layout.cpp
	src/database/browser-bookmark-db.cpp
	src/database/browser-user-agent-db.cpp
	src/database/browser-history-db.cpp
	src/database/browser-geolocation-db.cpp
	src/browser-settings/browser-settings-class.cpp
	src/browser-settings/browser-settings-main-view.cpp
	src/browser-settings/browser-settings-edit-homepage-view.cpp
	src/browser-settings/browser-settings-clear-data-view.cpp
	src/browser-settings/browser-settings-website-setting.cpp
	src/html5-feature-manager/custom-protocol-handler.cpp
	src/html5-feature-manager/custom-content-handler.cpp
)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/database)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/data-network)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/browser-bookmark)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/browser-view)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/most-visited-sites)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/browser-multi-window)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/browser-history)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/browser-settings)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/browser-policy-client)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/browser-extension)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/html5-feature-manager)

# set pkg-config modules
INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED 
		eina ecore evas ecore-evas edbus elementary
		dlog db-util utilX ui-gadget-1 tapi
		secure-storage libsoup-2.4 libxml-2.0 libssl capi-location-manager haptic devman
		ecore-input ecore-imf edje cairo ecore-x ewebkit2 appsvc capi-appfw-application
		capi-appfw-app-manager capi-web-url-download shortcut syspopup-caller capi-network-connection pkgmgr
		vconf-internal-keys
		capi-system-device
		capi-system-sensor
)

FIND_LIBRARY(LIB_GNUTLS gnutls)
FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

# set extra defines
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
SET(DATADIR "/opt/usr/apps/org.tizen.browser/data")
# res (read-only)
SET(EDJE_DIR "${RESDIR}/edje/")
SET(LOCALE_DIR "${RESDIR}/locale/")
SET(IMAGES_DIR "${RESDIR}/images/")
SET(HTML_DIR "${RESDIR}/html/")
SET(ICON_DIR "/usr/share/icons/default/small/")
# data
SET(XML_DIR "${DATADIR}/xml/")
SET(OPML_DIR "${DATADIR}/opml/")
SET(DATABASE_DIR "${DATADIR}/db/")
SET(SCREENSHOTS_DIR "${DATADIR}/screenshots/")
SET(CERTIFICATES_TMP_FILE "${DATADIR}/certificates.crt")
SET(WEBKIT_SOUP_CACHE_DIR "${DATADIR}/cache/")
# other defines
SET(CERTIFICATES_DIR "/opt/etc/ssl/certs/")
SET(DATABASENAME ".browser.db")
SET(HISTORY_DATABASENAME ".browser-history.db")
SET(CREDENTIAL_DATABASENAME ".browser-credential.db")
SET(MOST_VISITED_DATABASENAME ".browser-mostvisited.db")
SET(DESKTOP_DIR "/usr/share/applications")
ADD_DEFINITIONS(
		-DPROJECT_NAME=\"${PROJECT_NAME}\"
		-DDATABASENAME=\"${DATABASENAME}\"
		-DHISTORY_DATABASENAME=\"${HISTORY_DATABASENAME}\"
		-DCREDENTIAL_DATABASENAME=\"${CREDENTIAL_DATABASENAME}\"
		-DSPEEDDIAL_DATABASENAME=\"${SPEEDDIAL_DATABASENAME}\"
		-DEDJE_DIR=\"${EDJE_DIR}\"
		-DLOCALEDIR=\"${LOCALE_DIR}\"
		-DIMAGES_DIR=\"${IMAGES_DIR}\"
		-DSCREENSHOTS_DIR=\"${SCREENSHOTS_DIR}\"
		-DHTML_DIR=\"${HTML_DIR}\"
		-DXML_DIR=\"${XML_DIR}\"
		-DOPML_DIR=\"${OPML_DIR}\"
		-DCERTIFICATES_DIR=\"${CERTIFICATES_DIR}\"
		-DCERTIFICATES_TMP_FILE=\"${CERTIFICATES_TMP_FILE}\"
		-DDATABASE_DIR=\"${DATABASE_DIR}\"
		-DWEBKIT_SOUP_CACHE_DIR=\"${WEBKIT_SOUP_CACHE_DIR}\"
		-DLOG_TAG=\"org.tizen.browser\"
		-D__BUILD__
)

# Feature definitions
ADD_DEFINITIONS("-DBROWSER_SCROLLER_BOUNCING")
ADD_DEFINITIONS("-DSUPPORT_HLS")

# Feature definitions - Must be activated on the Tizen public
ADD_DEFINITIONS("-DTIZEN_PUBLIC")
ADD_DEFINITIONS("-DZOOM_BUTTON")
ADD_DEFINITIONS("-DHORIZONTAL_UI")
ADD_DEFINITIONS("-DGENLIST_SWEEP")
ADD_DEFINITIONS("-DEDIT_FOLDER_VIEW")
ADD_DEFINITIONS("-DSTORE_FAVICON")

SET (USER_AGENT_VIEW ON)
IF (USER_AGENT_VIEW)
        ADD_DEFINITIONS("-DUSER_AGENT_VIEW")
        LIST(APPEND SRCS
                src/browser-settings/browser-settings-user-agent-view.cpp
        )
ENDIF ()
SET (FEATURE_MOST_VISITED_LIST ON)
IF (FEATURE_MOST_VISITED_LIST)
        ADD_DEFINITIONS("-DFEATURE_MOST_VISITED_SITES")
        LIST(APPEND SRCS
		src/most-visited-sites/most-visited-sites.cpp
                src/most-visited-sites/add-to-most-visited-sites-view.cpp
		src/database/most-visited-sites-db.cpp
        )
ENDIF ()
# set flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} -fpie")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -fpie")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -fpie")

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" STREQUAL "arm")
	ADD_DEFINITIONS("-D_TARGET")
	MESSAGE("add -D_TARGET")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpie")
    #SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mabi=aapcs-linux -march=armv7-a -msoft-float")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mabi=aapcs-linux -mfpu=vfp -mfloat-abi=softfp")
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pie")
ENDIF("${ARCH}" STREQUAL "arm")

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed -pie,--hash-style=both")

# real make??
ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} ${LIB_GNUTLS})
#TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} ewebkit2)

# browser-define.edj
ADD_CUSTOM_TARGET(browser-define.edj
	COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images
	${CMAKE_SOURCE_DIR}/edc/browser-define.edc
	${CMAKE_BINARY_DIR}/edc/browser-define.edj
	DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-define.edc
	)
ADD_DEPENDENCIES(${PROJECT_NAME} browser-define.edj)

# browser-view-progressbar.edj
ADD_CUSTOM_TARGET(browser-view-progressbar.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/browser-view-progressbar.edc
        ${CMAKE_BINARY_DIR}/edc/browser-view-progressbar.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-view-progressbar.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-view-progressbar.edj)

# browser-naviframe.edj
ADD_CUSTOM_TARGET(browser-naviframe.edj
	COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images
	${CMAKE_SOURCE_DIR}/edc/browser-naviframe.edc
	${CMAKE_BINARY_DIR}/edc/browser-naviframe.edj
	DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-naviframe.edc
	)
ADD_DEPENDENCIES(${PROJECT_NAME} browser-naviframe.edj)

# browser-reader.edj
ADD_CUSTOM_TARGET(browser-reader.edj
	COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images
	${CMAKE_SOURCE_DIR}/edc/browser-reader.edc
	${CMAKE_BINARY_DIR}/edc/browser-reader.edj
	DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-reader.edc
	)
ADD_DEPENDENCIES(${PROJECT_NAME} browser-reader.edj)

# browser-view-main.edj
ADD_CUSTOM_TARGET(browser-view-main.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/browser-view-main.edc
        ${CMAKE_BINARY_DIR}/edc/browser-view-main.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-view-main.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-view-main.edj)

# browser-view-url-layout.edj
ADD_CUSTOM_TARGET(browser-view-url-layout.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/browser-view-url-layout.edc
        ${CMAKE_BINARY_DIR}/edc/browser-view-url-layout.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-view-url-layout.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-view-url-layout.edj)

# browser-view-find-word-layout.edj
ADD_CUSTOM_TARGET(browser-view-find-word-layout.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/browser-view-find-word-layout.edc
        ${CMAKE_BINARY_DIR}/edc/browser-view-find-word-layout.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-view-find-word-layout.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-view-find-word-layout.edj)

# browser-view-control-bar.edj
ADD_CUSTOM_TARGET(browser-view-control-bar.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/browser-view-control-bar.edc
        ${CMAKE_BINARY_DIR}/edc/browser-view-control-bar.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-view-control-bar.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-view-control-bar.edj)

# browser-bookmark-view.edj
ADD_CUSTOM_TARGET(browser-bookmark-view.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/browser-bookmark-view.edc
        ${CMAKE_BINARY_DIR}/edc/browser-bookmark-view.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-bookmark-view.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-bookmark-view.edj)

# browser-most-visited.edj
ADD_CUSTOM_TARGET(browser-most-visited.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/browser-most-visited.edc
        ${CMAKE_BINARY_DIR}/edc/browser-most-visited.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-most-visited.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-most-visited.edj)

# browser-multi-window.edj
ADD_CUSTOM_TARGET(browser-multi-window.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/browser-multi-window.edc
        ${CMAKE_BINARY_DIR}/edc/browser-multi-window.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-multi-window.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-multi-window.edj)

# browser-predictive-history.edj
ADD_CUSTOM_TARGET(browser-predictive-history.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/browser-predictive-history.edc
        ${CMAKE_BINARY_DIR}/edc/browser-predictive-history.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-predictive-history.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-predictive-history.edj)

# most-visited-sites.edj
ADD_CUSTOM_TARGET(most-visited-sites.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/most-visited-sites.edc
        ${CMAKE_BINARY_DIR}/edc/most-visited-sites.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/most-visited-sites.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} most-visited-sites.edj)

# browser-settings.edj
ADD_CUSTOM_TARGET(browser-settings.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/browser-settings.edc
        ${CMAKE_BINARY_DIR}/edc/browser-settings.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-settings.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-settings.edj)

# browser-popup.edj
ADD_CUSTOM_TARGET(browser-popup.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/browser-popup.edc
        ${CMAKE_BINARY_DIR}/edc/browser-popup.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-popup.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-popup.edj)

# install browser
INSTALL(PROGRAMS ${PROJECT_NAME} DESTINATION bin)
# install edj
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-define.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-naviframe.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-view-main.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-view-progressbar.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-view-url-layout.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-view-find-word-layout.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-view-control-bar.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-bookmark-view.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-most-visited.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-multi-window.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-predictive-history.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/most-visited-sites.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-settings.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-reader.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-popup.edj DESTINATION ${EDJE_DIR})

# install images
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/images/install/ DESTINATION ${IMAGES_DIR}
		FILES_MATCHING
		PATTERN "*.png"
)
# install html files
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/html/ DESTINATION ${HTML_DIR}
		FILES_MATCHING PATTERN "*.html"
		PATTERN "*.png"
)
# install empty directories
INSTALL(DIRECTORY DESTINATION ${DATABASE_DIR})
INSTALL(DIRECTORY DESTINATION ${XML_DIR})
INSTALL(DIRECTORY DESTINATION ${OPML_DIR})
INSTALL(DIRECTORY DESTINATION ${SCREENSHOTS_DIR})
INSTALL(DIRECTORY DESTINATION ${WEBKIT_SOUP_CACHE_DIR})

# install manifest file & icon
INSTALL(FILES ${CMAKE_SOURCE_DIR}/org.tizen.${PROJECT_NAME}.xml DESTINATION /usr/share/packages)

# install desktop file & icon
INSTALL(FILES ${CMAKE_SOURCE_DIR}/resource/org.tizen.${PROJECT_NAME}.png DESTINATION ${ICON_DIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/resource/default_0 DESTINATION ${SCREENSHOTS_DIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/resource/default_1 DESTINATION ${SCREENSHOTS_DIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/resource/default_2 DESTINATION ${SCREENSHOTS_DIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/resource/default_3 DESTINATION ${SCREENSHOTS_DIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/resource/default_application_icon.png DESTINATION "${DATADIR}/")
INSTALL(FILES ${CMAKE_SOURCE_DIR}/resource/config_sample.xml DESTINATION "${DATADIR}/")
# i18n
ADD_SUBDIRECTORY(po)
