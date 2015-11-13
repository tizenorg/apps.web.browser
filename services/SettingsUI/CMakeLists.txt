project(SettingsUI)

set(SettingsUI_SRCS
    SettingsUI.cpp
    )

set(SettingsUI_HEADERS
    SettingsUI.h
    )

include(Coreheaders)
include(EFLHelpers)

add_library(${PROJECT_NAME} SHARED ${SettingsUI_SRCS})

if(TIZEN_BUILD)
    target_link_libraries(${PROJECT_NAME} ${pkgs_LDFLAGS})
endif(TIZEN_BUILD)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)

include(EDCCompile)

EDJ_TARGET(SettingsMobileUI.edj
    ${CMAKE_CURRENT_SOURCE_DIR}/edc/SettingsMobile.edc
    ${CMAKE_CURRENT_BINARY_DIR})

#EDJ_TARGET(ResetBrowserMobileUI.edj
#    ${CMAKE_CURRENT_SOURCE_DIR}/edc/ResetBrowserMobile.edc
#    ${CMAKE_CURRENT_BINARY_DIR})

#EDJ_TARGET(ResetMostVisitedMobileUI.edj
#    ${CMAKE_CURRENT_SOURCE_DIR}/edc/ResetMostVisitedMobile.edc
#    ${CMAKE_CURRENT_BINARY_DIR})

#EDJ_TARGET(DeleteBrowsingDataMobileUI.edj
#    ${CMAKE_CURRENT_SOURCE_DIR}/edc/DeleteBrowsingDataMobile.edc
#    ${CMAKE_CURRENT_BINARY_DIR})
