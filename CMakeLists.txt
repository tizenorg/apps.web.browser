#
# Copyright 2012  Samsung Electronics Co., Ltd
#
# Licensed under the Flora License, Version 1.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://floralicense.org/license/
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(browser CXX)

SET(SRCS
	src/authentication-manager.cpp
	src/bookmark.cpp
	src/bookmark-item.cpp
	src/bookmark-view.cpp
	src/bookmark-edit-view.cpp
	src/bookmark-add-view.cpp
	src/bookmark-select-folder-view.cpp
	src/bookmark-create-folder-view.cpp
	src/browser.cpp
	src/browser-object.cpp
	src/browser-view.cpp
	src/certificate-manager.cpp
	src/common-view.cpp
	src/custom-content-handler.cpp
	src/custom-protocol-handler.cpp
	src/download-manager.cpp
	src/find-on-page.cpp
	src/gcf-manager.cpp
	src/history.cpp
	src/history-item.cpp
	src/history-view.cpp
	src/html5-feature-manager.cpp
	src/main.cpp
	src/most-visited.cpp
	src/most-visited-item.cpp
	src/multiwindow-item.cpp
	src/multiwindow-view.cpp
	src/network-manager.cpp
	src/platform-service.cpp
	src/preference.cpp
	src/reader.cpp
	src/scrap.cpp
	src/scrap-view.cpp
	src/setting-view.cpp
	src/geolocation-manager.cpp
	src/geolocation-item.cpp
	src/clear-private-data-view.cpp
	src/homepage-edit-view.cpp
	src/user-agent-view.cpp
	src/custom-user-agent-view.cpp
	src/website-setting-view.cpp
	src/website-setting-manager.cpp
	src/website-setting-item.cpp
	src/website-setting-item-view.cpp
	src/text-encoding-type-view.cpp
	src/uri-bar.cpp
	src/uri-input-bar.cpp
	src/user-agent-manager.cpp
	src/webview.cpp
	src/webview-list.cpp
)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)

# set pkg-config modules
INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED 
		eina ecore evas ecore-evas edbus elementary
		dlog db-util utilX ui-gadget-1 tapi
		secure-storage libsoup-2.4 libxml-2.0 libssl capi-location-manager
		haptic
		devman
		accounts-svc ecore-input ecore-imf edje cairo ecore-x ewebkit2 appsvc capi-appfw-application
		capi-appfw-app-manager capi-web-url-download shortcut syspopup-caller capi-network-connection pkgmgr
		vconf-internal-keys
		capi-system-device
		capi-system-sensor
		capi-network-nfc
		capi-network-wifi
		capi-media-image-util
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
SET(TEMPLATE_DIR "${RESDIR}/template/")
# data
SET(MHT_DIR "${DATADIR}/mht/")
SET(XML_DIR "${DATADIR}/xml/")
SET(OPML_DIR "${DATADIR}/opml/")
SET(DATABASE_DIR "${DATADIR}/db/")
SET(CERTIFICATES_TMP_FILE "${DATADIR}/certificates.crt")
SET(WEBKIT_SOUP_CACHE_DIR "${DATADIR}/cache/")
# other defines
SET(CERTIFICATES_DIR "/opt/etc/ssl/certs/")
SET(DATABASENAME ".browser.db")
SET(HISTORY_DATABASENAME ".browser-history.db")
SET(GEOLOCATION_DATABASENAME ".browser-geolocation.db")
SET(DESKTOP_DIR "/usr/share/applications")
ADD_DEFINITIONS(
		-DPROJECT_NAME=\"${PROJECT_NAME}\"
		-DDATABASENAME=\"${DATABASENAME}\"
		-DHISTORY_DATABASENAME=\"${HISTORY_DATABASENAME}\"
		-DGEOLOCATION_DATABASENAME=\"${GEOLOCATION_DATABASENAME}\"
		-DCREDENTIAL_DATABASENAME=\"${CREDENTIAL_DATABASENAME}\"
		-DSPEEDDIAL_DATABASENAME=\"${SPEEDDIAL_DATABASENAME}\"
		-DEDJE_DIR=\"${EDJE_DIR}\"
		-DLOCALEDIR=\"${LOCALE_DIR}\"
		-DIMAGES_DIR=\"${IMAGES_DIR}\"
		-DHTML_DIR=\"${HTML_DIR}\"
		-DXML_DIR=\"${XML_DIR}\"
		-DTEMPLATE_DIR=\"${TEMPLATE_DIR}\"
		-DOPML_DIR=\"${OPML_DIR}\"
		-DCERTIFICATES_DIR=\"${CERTIFICATES_DIR}\"
		-DCERTIFICATES_TMP_FILE=\"${CERTIFICATES_TMP_FILE}\"
		-DDATABASE_DIR=\"${DATABASE_DIR}\"
		-DWEBKIT_SOUP_CACHE_DIR=\"${WEBKIT_SOUP_CACHE_DIR}\"
		-DLOG_TAG=\"org.tizen.browser\"
		-D__BUILD__
)

# Feature definitions
ADD_DEFINITIONS("-DTEST_CODE")
ADD_DEFINITIONS("-DMULTIWINDOW_PINCH_EFFECT_DISABLE")
#ADD_DEFINITIONS("-DBROWSER_THUMBNAIL_VIEW")
#ADD_DEFINITIONS("-DBROWSER_BOOKMARK_SYNC")
ADD_DEFINITIONS("-DROOT_IS_ZERO")

SET (BOOKMARK_SERVICE ON) 
IF (BOOKMARK_SERVICE) 
	INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/bookmark-service) 
        LIST(APPEND SRCS src/bookmark-service/bmsvc-bookmarks.cpp
                src/bookmark-service/bmsvc-private.cpp
                src/bookmark-service/database.cpp ) 
ENDIF ()

# set flags
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} -fpie -Wall")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -fpie")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -fpie")

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" STREQUAL "arm")
	ADD_DEFINITIONS("-D_TARGET")
	MESSAGE("add -D_TARGET")
    #SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mabi=aapcs-linux -march=armv7-a -msoft-float")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mabi=aapcs-linux -mfpu=vfp -mfloat-abi=softfp")
ENDIF("${ARCH}" STREQUAL "arm")

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed -pie")

# real make??
ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} ${LIB_GNUTLS})
#TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} ewebkit2)

# browser-view.edj
ADD_CUSTOM_TARGET(browser-view.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images -sd ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/browser-view.edc
        ${CMAKE_BINARY_DIR}/edc/browser-view.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-view.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-view.edj)

# browser-popup.edj
ADD_CUSTOM_TARGET(browser-popup.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images -sd ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/browser-popup.edc
        ${CMAKE_BINARY_DIR}/edc/browser-popup.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-popup.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-popup.edj)

# uri-bar.edj
ADD_CUSTOM_TARGET(uri-bar.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images -sd ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/uri-bar.edc
        ${CMAKE_BINARY_DIR}/edc/uri-bar.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/uri-bar.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} uri-bar.edj)

# uri-input-bar.edj
ADD_CUSTOM_TARGET(uri-input-bar.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images -sd ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/uri-input-bar.edc
        ${CMAKE_BINARY_DIR}/edc/uri-input-bar.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/uri-input-bar.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} uri-input-bar.edj)

# find-on-page.edj
ADD_CUSTOM_TARGET(find-on-page.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images -sd ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/find-on-page.edc
        ${CMAKE_BINARY_DIR}/edc/find-on-page.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/find-on-page.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} find-on-page.edj)

# reader.edj
ADD_CUSTOM_TARGET(reader.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images -sd ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/reader.edc
        ${CMAKE_BINARY_DIR}/edc/reader.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/reader.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} reader.edj)

# multiwindow-view.edj
ADD_CUSTOM_TARGET(multiwindow-view.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images -sd ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/multiwindow-view.edc
        ${CMAKE_BINARY_DIR}/edc/multiwindow-view.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/multiwindow-view.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} multiwindow-view.edj)

# multiwindow-panes.edj
ADD_CUSTOM_TARGET(multiwindow-panes.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images -sd ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/multiwindow-panes.edc
        ${CMAKE_BINARY_DIR}/edc/multiwindow-panes.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/multiwindow-panes.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} multiwindow-panes.edj)

# most-visited.edj
ADD_CUSTOM_TARGET(most-visited.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images -sd ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/most-visited.edc
        ${CMAKE_BINARY_DIR}/edc/most-visited.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/most-visited.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} most-visited.edj)

# bookmark-view.edj
ADD_CUSTOM_TARGET(bookmark-view.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images -sd ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/bookmark-view.edc
        ${CMAKE_BINARY_DIR}/edc/bookmark-view.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/bookmark-view.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} bookmark-view.edj)

# scrap-view.edj
ADD_CUSTOM_TARGET(scrap-view.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images -sd ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/scrap-view.edc
        ${CMAKE_BINARY_DIR}/edc/scrap-view.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/scrap-view.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} scrap-view.edj)

# history-view.edj
ADD_CUSTOM_TARGET(history-view.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images -sd ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/history-view.edc
        ${CMAKE_BINARY_DIR}/edc/history-view.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/history-view.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} history-view.edj)

# bookmark-edit-view.edj
ADD_CUSTOM_TARGET(bookmark-edit-view.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images -sd ${CMAKE_SOURCE_DIR}/images
        ${CMAKE_SOURCE_DIR}/edc/bookmark-edit-view.edc
        ${CMAKE_BINARY_DIR}/edc/bookmark-edit-view.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/bookmark-edit-view.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} bookmark-edit-view.edj)

# browser-genlist.edj
ADD_CUSTOM_TARGET(browser-genlist.edj
        COMMAND edje_cc -id ${CMAKE_SOURCE_DIR}/images/genlist -sd ${CMAKE_SOURCE_DIR}/images/genlist/sounds
        ${CMAKE_SOURCE_DIR}/edc/browser-genlist.edc
        ${CMAKE_BINARY_DIR}/edc/browser-genlist.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-genlist.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-genlist.edj)

# install browser
INSTALL(PROGRAMS ${PROJECT_NAME} DESTINATION bin)
# install edj

INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-view.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-popup.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/uri-bar.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/uri-input-bar.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/multiwindow-view.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/multiwindow-panes.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/most-visited.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/bookmark-view.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/find-on-page.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/reader.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/scrap-view.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/history-view.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/bookmark-edit-view.edj DESTINATION ${EDJE_DIR})

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
INSTALL(DIRECTORY DESTINATION ${MHT_DIR})
INSTALL(DIRECTORY DESTINATION ${TEMPLATE_DIR})
INSTALL(DIRECTORY DESTINATION ${OPML_DIR})
INSTALL(DIRECTORY DESTINATION ${WEBKIT_SOUP_CACHE_DIR})

# install manifest file & icon
INSTALL(FILES ${CMAKE_SOURCE_DIR}/org.tizen.${PROJECT_NAME}.xml DESTINATION /usr/share/packages)

# install desktop file & icon
INSTALL(FILES ${CMAKE_SOURCE_DIR}/resource/org.tizen.${PROJECT_NAME}.png DESTINATION ${ICON_DIR})
INSTALL(FILES ${CMAKE_SOURCE_DIR}/resource/default_application_icon.png DESTINATION "${TEMPLATE_DIR}/")
INSTALL(FILES ${CMAKE_SOURCE_DIR}/resource/config_sample.xml DESTINATION "${TEMPLATE_DIR}/")
INSTALL(FILES ${CMAKE_SOURCE_DIR}/resource/template_bluetooth_content_share.html DESTINATION "${TEMPLATE_DIR}/")
INSTALL(DIRECTORY ${CMAKE_SOURCE_DIR}/js/ DESTINATION "${RESDIR}/js/")
# i18n
ADD_SUBDIRECTORY(po)
