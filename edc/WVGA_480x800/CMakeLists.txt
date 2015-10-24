####################################################
#
#
#
#	480X800 WVGA specific EDC files
#
#
#
####################################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(browser CXX)

SET(CMAKE_SOURCE_IMAGES_WVGA_DIR "${CMAKE_SOURCE_IMAGES_DIR}/WVGA_480x800")

# browser-view.edj
ADD_CUSTOM_TARGET(browser-view.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_IMAGES_WVGA_DIR} -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
	-sd ${CMAKE_SOURCE_DIR}/sounds
        ${CMAKE_SOURCE_DIR}/edc/WVGA_480x800/browser-view.edc
        ${CMAKE_BINARY_DIR}/edc/browser-view.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/WVGA_480x800/browser-view.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-view.edj)
# main-view.edj
ADD_CUSTOM_TARGET(main-view.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_IMAGES_WVGA_DIR} -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/WVGA_480x800/main-view.edc
        ${CMAKE_BINARY_DIR}/edc/main-view.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/WVGA_480x800/main-view.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} main-view.edj)
# url-bar.edj
ADD_CUSTOM_TARGET(url-bar.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_IMAGES_WVGA_DIR} -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/WVGA_480x800/url-bar.edc
        ${CMAKE_BINARY_DIR}/edc/url-bar.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/WVGA_480x800/url-bar.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} url-bar.edj)
# url-input-bar.edj
ADD_CUSTOM_TARGET(url-input-bar.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_IMAGES_WVGA_DIR} -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/WVGA_480x800/url-input-bar.edc
        ${CMAKE_BINARY_DIR}/edc/url-input-bar.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/WVGA_480x800/url-input-bar.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} url-input-bar.edj)
# main-toolbar.edj
ADD_CUSTOM_TARGET(main-toolbar.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_IMAGES_WVGA_DIR} -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/WVGA_480x800/main-toolbar.edc
        ${CMAKE_BINARY_DIR}/edc/main-toolbar.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/WVGA_480x800/main-toolbar.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} main-toolbar.edj)
# bookmark-view.edj
ADD_CUSTOM_TARGET(bookmark-view.edj
        COMMAND edje_cc -no-save
         ${EDC_DEFINITIONS}
        -id ${CMAKE_SOURCE_IMAGES_WVGA_DIR}
        -id ${CMAKE_SOURCE_IMAGES_WVGA_DIR}/bookmark-view
        -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${CMAKE_SOURCE_DIR}/edc/WVGA_480x800/bookmark-view.edc
        ${CMAKE_BINARY_DIR}/edc/bookmark-view.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/WVGA_480x800/bookmark-view.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} bookmark-view.edj)

# tab-manager-view-gengrid.edj
	ADD_CUSTOM_TARGET(tab-manager-view-gengrid.edj
	        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_IMAGES_DIR} -sd ${CMAKE_SOURCE_DIR}/images
	        ${EDC_DEFINITIONS}
				-id ${CMAKE_SOURCE_IMAGES_DIR}/tizen-hd/Focus
				-id ${CMAKE_SOURCE_IMAGES_DIR}/tizen-hd/Content_views/Gengrid
				-sd ${CMAKE_SOURCE_SOUNDS_DIR}
	        ${CMAKE_SOURCE_DIR}/edc/WVGA_480x800/tab-manager-view-gengrid.edc
	        ${CMAKE_BINARY_DIR}/edc/tab-manager-view-gengrid.edj
	        DEPENDS ${CMAKE_SOURCE_DIR}/edc/WVGA_480x800/tab-manager-view-gengrid.edc
	        )
	ADD_DEPENDENCIES(${PROJECT_NAME} tab-manager-view-gengrid.edj)
####################################################

## find-on-page.edj
ADD_CUSTOM_TARGET(find-on-page.edj
        COMMAND edje_cc -no-save
        ${EDC_DEFINITIONS}
		-id ${CMAKE_SOURCE_IMAGES_WVGA_DIR}
        -id ${CMAKE_SOURCE_IMAGES_DIR}/tizen-hd/Content_views
        -id ${CMAKE_SOURCE_IMAGES_DIR}/tizen-hd/Copy_Paste
        -id ${CMAKE_SOURCE_IMAGES_DIR}/tizen-hd/GUI_Style_Guide
		-sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${CMAKE_SOURCE_DIR}/edc/WVGA_480x800/find-on-page.edc
        ${CMAKE_BINARY_DIR}/edc/find-on-page.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/WVGA_480x800/find-on-page.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} find-on-page.edj)

###################################################
# install edj
####################################################
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-view.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/bookmark-view.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/url-bar.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/url-input-bar.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/main-toolbar.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/main-view.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/find-on-page.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/tab-manager-view-gengrid.edj DESTINATION ${EDJE_DIR})