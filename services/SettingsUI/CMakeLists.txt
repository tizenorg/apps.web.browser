project(SettingsUI)

if (${PROFILE} MATCHES "mobile")
set(SettingsUI_SRCS
    SettingsUI_mob.cpp
    )
set(SettingsUI_HEADERS
    SettingsUI_mob.h
    )
else (${PROFILE} MATCHES "mobile")
set(SettingsUI_SRCS
    SettingsUI.cpp
    )
set(SettingsUI_HEADERS
    SettingsUI.h
    )
endif (${PROFILE} MATCHES "mobile")

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

if (${PROFILE} MATCHES "mobile")
EDJ_TARGET(SettingsMobileUI.edj
    ${CMAKE_CURRENT_SOURCE_DIR}/edc/SettingsMobile.edc
    ${CMAKE_CURRENT_BINARY_DIR})
else (${PROFILE} MATCHES "mobile")
EDJ_TARGET(SettingsUI.edj
    ${CMAKE_CURRENT_SOURCE_DIR}/edc/Settings.edc
    ${CMAKE_CURRENT_BINARY_DIR})
endif (${PROFILE} MATCHES "mobile")
