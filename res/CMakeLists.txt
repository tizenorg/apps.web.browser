####################################################
#
#
#
#	Resource files
#
#
#
####################################################

CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(browser CXX)

#########################################################
# i18n text resources
#########################################################
ADD_SUBDIRECTORY(po_lite)


#########################################################
# Installing resources
#########################################################
# Application Icon
INSTALL(FILES ${CMAKE_SOURCE_IMAGES_DIR}/org.tizen.${PROJECT_NAME}.png DESTINATION ${ICON_DIR})

# Resources for Browser wide depends on screen resolution
INSTALL(DIRECTORY ${CMAKE_SOURCE_IMAGES_INSTALL_DIR}/WVGA_480x800/ DESTINATION "${IMAGES_DIR}")

# java script files
INSTALL(DIRECTORY ${CMAKE_SOURCE_RES_DIR}/js/ DESTINATION "${RESDIR}/js")

# database files
SET(CMAKE_SOURCE_RES_DATABASE_DIR "${CMAKE_SOURCE_RES_DIR}/database")
INSTALL(FILES ${CMAKE_SOURCE_RES_DATABASE_DIR}/.pref.db DESTINATION "${DATADIR}/")
INSTALL(FILES ${CMAKE_SOURCE_RES_DATABASE_DIR}/.pref.db-journal DESTINATION "${DATADIR}/")

# html files
SET(CMAKE_SOURCE_RES_HTML_DIR "${CMAKE_SOURCE_RES_DIR}/html/")
INSTALL(DIRECTORY ${CMAKE_SOURCE_RES_HTML_DIR} DESTINATION ${HTML_DIR}
		FILES_MATCHING PATTERN "*.html"
		PATTERN "*.png"
)

