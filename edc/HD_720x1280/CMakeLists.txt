####################################################
#
#
#
#	720X1280 HD specific EDC files
#
#
#
####################################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(browser CXX)

# browser-view.edj
ADD_CUSTOM_TARGET(browser-view.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
	-sd ${CMAKE_SOURCE_DIR}/sounds
        ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/browser-view.edc
        ${CMAKE_BINARY_DIR}/edc/browser-view.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/browser-view.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-view.edj)
# main-view.edj
ADD_CUSTOM_TARGET(main-view.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/main-view.edc
        ${CMAKE_BINARY_DIR}/edc/main-view.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/main-view.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} main-view.edj)
# url-bar.edj
ADD_CUSTOM_TARGET(url-bar.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/url-bar.edc
        ${CMAKE_BINARY_DIR}/edc/url-bar.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/url-bar.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} url-bar.edj)
# url-input-bar.edj
ADD_CUSTOM_TARGET(url-input-bar.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/url-input-bar.edc
        ${CMAKE_BINARY_DIR}/edc/url-input-bar.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/url-input-bar.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} url-input-bar.edj)
# main-toolbar.edj
ADD_CUSTOM_TARGET(main-toolbar.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/main-toolbar.edc
        ${CMAKE_BINARY_DIR}/edc/main-toolbar.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/main-toolbar.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} main-toolbar.edj)
# bookmark-view.edj
	ADD_CUSTOM_TARGET(bookmark-view.edj
	        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images -sd ${CMAKE_SOURCE_SOUNDS_DIR}
	        ${EDC_DEFINITIONS}
	        ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/bookmark-view.edc
	        ${CMAKE_BINARY_DIR}/edc/bookmark-view.edj
	        DEPENDS ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/bookmark-view.edc
	        )
	ADD_DEPENDENCIES(${PROJECT_NAME} bookmark-view.edj)

# tab-manager-view-gengrid.edj
ADD_CUSTOM_TARGET(tab-manager-view-gengrid.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
			-id ${CMAKE_SOURCE_DIR}/res/images/tizen-hd/Focus
			-id ${CMAKE_SOURCE_DIR}/res/images/tizen-hd/Content_views/Gengrid
			-sd ${CMAKE_SOURCE_DIR}/sounds
        ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/tab-manager-view-gengrid.edc
        ${CMAKE_BINARY_DIR}/edc/tab-manager-view-gengrid.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/tab-manager-view-gengrid.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} tab-manager-view-gengrid.edj)

# br-navigationbar.edj
ADD_CUSTOM_TARGET(br-navigationbar.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images/br-navigationbar -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        -id ${CMAKE_SOURCE_DIR}/res/images/tizen-hd/Focus
        ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/br-navigationbar.edc
        ${CMAKE_BINARY_DIR}/edc/br-navigationbar.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/br-navigationbar.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} br-navigationbar.edj)

# browser-genlist.edj
ADD_CUSTOM_TARGET(browser-genlist.edj
        COMMAND edje_cc -no-save
        ${EDC_DEFINITIONS}
        -id ${CMAKE_SOURCE_DIR}/res/images/tizen-hd
        -id ${CMAKE_SOURCE_DIR}/res/images/tizen-hd/Content_views
        -id ${CMAKE_SOURCE_DIR}/res/images/tizen-hd/Content_views/Gengrid
        -id ${CMAKE_SOURCE_DIR}/res/images/tizen-hd/Focus
        -id ${CMAKE_SOURCE_DIR}/res/images/tizen-hd/Controls
        -id ${CMAKE_SOURCE_DIR}/res/images/tizen-hd/Controls/Button
        -id ${CMAKE_SOURCE_DIR}/res/images/tizen-hd/GUI_Style_Guide
        -id ${CMAKE_SOURCE_DIR}/res/images/tizen-hd/Naviframe
        -id ${CMAKE_SOURCE_DIR}/res/images/tizen-hd/Popup
        -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/browser-genlist.edc
        ${CMAKE_BINARY_DIR}/edc/browser-genlist.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/browser-genlist.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} browser-genlist.edj)

# scrap-view.edj
ADD_CUSTOM_TARGET(scrap-view.edj
        COMMAND edje_cc -no-save -id ${CMAKE_SOURCE_DIR}/res/images -sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${EDC_DEFINITIONS}
        ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/scrap-view.edc
        ${CMAKE_BINARY_DIR}/edc/scrap-view.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/scrap-view.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} scrap-view.edj)

# find-on-page.edj
ADD_CUSTOM_TARGET(find-on-page.edj
        COMMAND edje_cc -no-save
        ${EDC_DEFINITIONS}
	-id ${CMAKE_SOURCE_DIR}/res/images
	-id ${CMAKE_SOURCE_DIR}/res/images/HD_720x1280
        -id ${CMAKE_SOURCE_DIR}/res/images/tizen-hd/Content_views
        -id ${CMAKE_SOURCE_DIR}/res/images/tizen-hd/Copy_Paste
        -id ${CMAKE_SOURCE_DIR}/res/images/tizen-hd/GUI_Style_Guide
	-sd ${CMAKE_SOURCE_SOUNDS_DIR}
        ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/find-on-page.edc
        ${CMAKE_BINARY_DIR}/edc/find-on-page.edj
        DEPENDS ${CMAKE_SOURCE_DIR}/edc/HD_720x1280/find-on-page.edc
        )
ADD_DEPENDENCIES(${PROJECT_NAME} find-on-page.edj)

####################################################
# install edj
####################################################
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/main-view.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-view.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/bookmark-view.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/tab-manager-view-gengrid.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/url-bar.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/url-input-bar.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/main-toolbar.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/br-navigationbar.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/browser-genlist.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/scrap-view.edj DESTINATION ${EDJE_DIR})
INSTALL(FILES ${CMAKE_BINARY_DIR}/edc/find-on-page.edj DESTINATION ${EDJE_DIR})
