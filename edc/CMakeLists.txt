####################################################
#
#
#
#	EDC files
#
#
#
####################################################
CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(browser CXX)

MESSAGE("EDC_DEFINITIONS: " ${EDC_DEFINITIONS})

#####################################################
# Screen resolution specific edc files in sub folders
ADD_SUBDIRECTORY(WVGA_480x800)
SET(EDC_DEFINITIONS "${EDC_DEFINITIONS} -DBROWSER_WVGA_EDC_DEFINE")

#####################################################
# Common edc files for resolutions

# browser-common.edj
ADD_CUSTOM_TARGET(browser-common.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_IMAGES_DIR} -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/browser-common.edc
        ${CMAKE_BINARY_DIR}/edc/browser-common.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-common.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-common.edj)

# br-genlist.edj
ADD_CUSTOM_TARGET(br-genlist.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_IMAGES_DIR} -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/br-genlist.edc
        ${CMAKE_BINARY_DIR}/edc/br-genlist.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/br-genlist.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} br-genlist.edj)

# browser-popup-lite.edj
ADD_CUSTOM_TARGET(browser-popup-lite.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_IMAGES_DIR} -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/browser-popup-lite.edc
        ${CMAKE_BINARY_DIR}/edc/browser-popup-lite.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/browser-popup-lite.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-popup-lite.edj)

# more-menu-manager.edj
ADD_CUSTOM_TARGET(more-menu-manager.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_IMAGES_DIR} -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/more-menu-manager.edc
        ${CMAKE_BINARY_DIR}/edc/more-menu-manager.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/more-menu-manager.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} more-menu-manager.edj)

# tray-toolbar.edj
ADD_CUSTOM_TARGET(tray-toolbar.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_IMAGES_DIR} -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/tray-toolbar.edc
        ${CMAKE_BINARY_DIR}/edc/tray-toolbar.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/tray-toolbar.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} tray-toolbar.edj)

# history-view.edj
ADD_CUSTOM_TARGET(history-view.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_IMAGES_DIR} -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/history-view.edc
        ${CMAKE_BINARY_DIR}/edc/history-view.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/history-view.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} history-view.edj)

# predictive-history.edj
ADD_CUSTOM_TARGET(predictive-history.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_IMAGES_DIR} -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/predictive-history.edc
        ${CMAKE_BINARY_DIR}/edc/predictive-history.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/predictive-history.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} predictive-history.edj)

# predictive-history-genlist-lite.edj
ADD_CUSTOM_TARGET(predictive-history-genlist-lite.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_IMAGES_DIR} -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/predictive-history-genlist-lite.edc
        ${CMAKE_BINARY_DIR}/edc/predictive-history-genlist-lite.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/predictive-history-genlist-lite.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} predictive-history-genlist-lite.edj)

# bookmark-edit-view.edj
ADD_CUSTOM_TARGET(bookmark-edit-view.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_IMAGES_DIR} -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/bookmark-edit-view.edc
        ${CMAKE_BINARY_DIR}/edc/bookmark-edit-view.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/bookmark-edit-view.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} bookmark-edit-view.edj)

# settings.edj
ADD_CUSTOM_TARGET(settings.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_IMAGES_DIR} -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/settings.edc
        ${CMAKE_BINARY_DIR}/edc/settings.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/settings.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} settings.edj)

# tab-manager-view-lite.edj
ADD_CUSTOM_TARGET(tab-manager-view-lite.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_IMAGES_DIR} -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/tab-manager-view-lite.edc
        ${CMAKE_BINARY_DIR}/edc/tab-manager-view-lite.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/tab-manager-view-lite.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} tab-manager-view-lite.edj)

# progress-bar.edj
ADD_CUSTOM_TARGET(progress-bar.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_IMAGES_DIR} -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/progress-bar.edc
        ${CMAKE_BINARY_DIR}/edc/progress-bar.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/progress-bar.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} progress-bar.edj)

# install edj

INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-common.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/more-menu-manager.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/tray-toolbar.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/predictive-history.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/predictive-history-genlist-lite.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/history-view.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/bookmark-edit-view.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/settings.edj DESTINATION ${EDJE_DIR})

INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/br-genlist.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-popup-lite.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/tab-manager-view-lite.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/progress-bar.edj DESTINATION ${EDJE_DIR})

