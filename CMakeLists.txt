#
# Copyright 2013  Samsung Electronics Co., Ltd
#
# Licensed under the Flora License, Version 1.1 (the "License");
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
#
# Contact: Hyerim Bae <hyerim.bae@samsung.com>

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(browser CXX)

SET (ENABLE_WEB_NOTIFICATION_API OFF)
SET (EDC_DEFINITIONS "-DTIZEN_LITE")

SET(SRCS
	src/bookmark.cpp
	src/bookmark-common-view.cpp
	src/bookmark-item.cpp
	src/bookmark-add-view.cpp
	src/bookmark-select-folder-view.cpp
	src/bookmark-create-folder-view.cpp
	src/lite/bookmark-view.cpp
	src/lite/bookmark-edit-view.cpp
	src/browser.cpp
	src/browser-object.cpp
	src/browser-view.cpp
	src/certificate-manager.cpp
	src/certificate-view.cpp
	src/clear-private-data-view.cpp
	src/cloud-sync-manager.cpp
	src/common-view.cpp
	src/download-manager.cpp
	src/find-on-page.cpp
	src/history.cpp
	src/history-item.cpp
	src/history-view.cpp
	src/html5-feature-manager.cpp
	src/main.cpp
	src/main-toolbar.cpp
	src/main-view.cpp
	src/more-menu-manager.cpp
	src/network-manager.cpp
	src/platform-service.cpp
	src/preference.cpp
	src/progress-bar.cpp
	src/url-bar.cpp
	src/url-input-bar.cpp
	src/user-agent-manager.cpp
	src/webview.cpp
	src/webview_info.cpp
	src/webview-list.cpp
	src/search-engine-manager.cpp
	src/settings.cpp
	src/settings-basic.cpp
	src/settings-advanced.cpp
	src/tab-manager-view-lite.cpp
	src/tab-view-item-lite.cpp
	src/custom-user-agent-view.cpp
)
IF (ENABLE_WEB_NOTIFICATION_API)
    LIST(APPEND SRCS src/web-notification.cpp)
ENDIF (ENABLE_WEB_NOTIFICATION_API)

IF("${CMAKE_BUILD_TYPE}" STREQUAL "")
	SET(CMAKE_BUILD_TYPE "Release")
ENDIF("${CMAKE_BUILD_TYPE}" STREQUAL "")
MESSAGE("Build type: ${CMAKE_BUILD_TYPE}")

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/strings/lite)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/lite)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src/include)
INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/src)

# set pkg-config modules
SET (PKG_MODULES
		eina ecore evas ecore-evas edbus elementary
		dlog db-util utilX ui-gadget-1 tapi
		libsoup-2.4 libssl
		deviced
		ecore-input ecore-imf edje cairo ecore-x appsvc capi-appfw-application
		capi-appfw-app-manager capi-appfw-preference capi-network-connection
		vconf-internal-keys
		capi-system-info
		capi-network-wifi
		capi-media-image-util
		capi-content-media-content
		notification
		capi-web-tab
		capi-web-bookmark
		capi-web-history
		capi-web-scrap
		capi-media-image-util
		efl-extension
		capi-system-device
		storage
		xext
		json-glib-1.0
		glib-2.0
		capi-system-system-settings
		vconf
)

SET(PKG_MODULES ${PKG_MODULES} ewebkit2)

INCLUDE(FindPkgConfig)
pkg_check_modules(pkgs REQUIRED ${PKG_MODULES})

FIND_LIBRARY(LIB_GNUTLS gnutls)
FOREACH(flag ${pkgs_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

#########################################################
# Definitions for compiling
#########################################################
SET(CMAKE_SOURCE_RES_DIR "${CMAKE_SOURCE_DIR}/res")
SET(CMAKE_SOURCE_IMAGES_DIR "${CMAKE_SOURCE_RES_DIR}/images")
SET(CMAKE_SOURCE_IMAGES_INSTALL_DIR "${CMAKE_SOURCE_RES_DIR}/images_install")
SET(CMAKE_SOURCE_SOUNDS_DIR "${CMAKE_SOURCE_RES_DIR}/sounds")

#########################################################
# Definitions for Installing
#########################################################
# set extra defines
SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(PREV_DIR "${PREFIX}/shared/res/")
SET(BINDIR "${PREFIX}/bin")
SET(RESDIR "${PREFIX}/res")
SET(DATADIR "/opt/usr/apps/org.tizen.browser/data")
# res (read-only)
SET(EDJE_DIR "${RESDIR}/edje/")
SET(LOCALE_DIR "${RESDIR}/locale/")
SET(IMAGES_DIR "${RESDIR}/images/")
SET(HTML_DIR "${RESDIR}/html/")
SET(ICON_DIR "/usr/share/icons/default/small/")
SET(FAKE_IMAGE_DIR "/usr/share/splash_images/")
SET(TEMPLATE_DIR "${RESDIR}/template/")
# data
SET(MHT_DIR "${DATADIR}/mht/")
SET(XML_DIR "${DATADIR}/xml/")
SET(DATABASE_DIR "${DATADIR}/db/")
SET(SNAPSHOTS_IMAGE_DIR "${DATADIR}/snapshots/")

# other defines
SET(DESKTOP_DIR "/usr/share/applications")
SET(UGDIR "${PREFIX}/lib/ug")
ADD_DEFINITIONS(
		-DPROJECT_NAME=\"${PROJECT_NAME}\"
		-DEDJE_DIR=\"${EDJE_DIR}\"
		-DLOCALEDIR=\"${LOCALE_DIR}\"
		-DIMAGES_DIR=\"${IMAGES_DIR}\"
		-DHTML_DIR=\"${HTML_DIR}\"
		-DXML_DIR=\"${XML_DIR}\"
		-DDATABASE_DIR=\"${DATABASE_DIR}\"
		-DLOG_TAG=\"Internet\"
		-D__BUILD__
)

# Feature definitions
ADD_DEFINITIONS("-DWEBKIT_EFL")
ADD_DEFINITIONS("-DHW_MORE_BACK_KEY")
ADD_DEFINITIONS(-DBROWSER_APP_NAME=\"org.tizen.browser\")

# Browser function defines
SET (ENABLE_YAHOO_SEARCH ON)
SET (ENABLE_BING_SEARCH ON)
SET (ENABLE_INCOGNITO_WINDOW OFF)

IF (ENABLE_YAHOO_SEARCH)
	ADD_DEFINITIONS("-DENABLE_YAHOO_SEARCH")
ENDIF ()
IF (ENABLE_BING_SEARCH)
	ADD_DEFINITIONS("-DENABLE_BING_SEARCH")
ENDIF ()
IF (ENABLE_INCOGNITO_WINDOW)
	ADD_DEFINITIONS("-DENABLE_INCOGNITO_WINDOW")
ENDIF ()
IF (ENABLE_WEB_NOTIFICATION_API)
	ADD_DEFINITIONS("-DENABLE_WEB_NOTIFICATION_API")
ENDIF ()

# set flags
SET(GC_SECTIONS_FLAGS "-fdata-sections -ffunction-sections -Wl,--gc-sections")
SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${GC_SECTIONS_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GC_SECTIONS_FLAGS}")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${EXTRA_CFLAGS} -fPIE -Wall")
SET(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -fPIE")
SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O2 -fPIE")

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" STREQUAL "arm")
	ADD_DEFINITIONS("-D_TARGET")
	MESSAGE("add -D_TARGET")
    #SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mabi=aapcs-linux -march=armv7-a -msoft-float")
    SET(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mabi=aapcs-linux -mfpu=vfp -mfloat-abi=softfp")
ENDIF("${ARCH}" STREQUAL "arm")

SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,--as-needed -pie")
MESSAGE(${pkgs_LDFLAGS})

# real make??
ADD_EXECUTABLE(${PROJECT_NAME} ${SRCS})
TARGET_LINK_LIBRARIES(${PROJECT_NAME} ${pkgs_LDFLAGS} ${LIB_GNUTLS})

#########################################################
# EDC Compile
#########################################################
ADD_SUBDIRECTORY(edc)

#########################################################
# File Install
#########################################################
INSTALL(PROGRAMS ${PROJECT_NAME} DESTINATION bin)

# install empty directories
INSTALL(DIRECTORY DESTINATION ${DATABASE_DIR})
INSTALL(DIRECTORY DESTINATION ${XML_DIR})
INSTALL(DIRECTORY DESTINATION ${MHT_DIR})
INSTALL(DIRECTORY DESTINATION ${TEMPLATE_DIR})
INSTALL(DIRECTORY DESTINATION ${SNAPSHOTS_IMAGE_DIR})

# install manifest file & icon
INSTALL(FILES ${CMAKE_SOURCE_DIR}/org.tizen.${PROJECT_NAME}.xml DESTINATION /usr/share/packages)

#install SMACK rule file
INSTALL(FILES ${CMAKE_SOURCE_DIR}/org.tizen.${PROJECT_NAME}.efl DESTINATION /etc/smack/accesses.d)

# install desktop file & icon
#CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/resource/org.tizen.${PROJECT_NAME}.desktop.in org.tizen.${PROJECT_NAME}.desktop)
#INSTALL(FILES ${CMAKE_BINARY_DIR}/org.tizen.${PROJECT_NAME}.desktop DESTINATION ${DESKTOP_DIR})

# resources
ADD_SUBDIRECTORY(res)

# user agent
ADD_SUBDIRECTORY(user-agent-select)
