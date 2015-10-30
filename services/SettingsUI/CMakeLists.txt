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
EDJ_TARGET(SettingsUI.edj
    ${CMAKE_CURRENT_SOURCE_DIR}/edc/Settings.edc
    ${CMAKE_CURRENT_BINARY_DIR})
