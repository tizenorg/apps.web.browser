project(SettingsUI)

if (${PROFILE} MATCHES "mobile")
set(SettingsUI_SRCS
    SettingsManager.cpp
    SettingsUI_mob.cpp
    SettingsMain_mob.cpp
    SettingsHomePage_mob.cpp
    SettingsSearchEngine_mob.cpp
    SettingsPrivacy_mob.cpp
    SettingsAdvanced_mob.cpp
    SettingsDelPersData_mob.cpp
    SettingsAFProfile_mob.cpp
    SettingsAFCreator_mob.cpp
    AutoFillForm/AutoFillFormItem.cpp
    )
set(SettingsUI_HEADERS
    SettingsManager.h
    SettingsUI_mob.h
    SettingsMain_mob.h
    SettingsHomePage_mob.h
    SettingsSearchEngine_mob.h
    SettingsPrivacy_mob.h
    SettingsAdvanced_mob.h
    SettingsDelPersData_mob.h
    SettingsAFProfile_mob.h
    SettingsAFCreator_mob.cpp
    AutoFillForm/AutoFillFormItem.h
    )
else (${PROFILE} MATCHES "mobile")
set(SettingsUI_SRCS
    SettingsUI.cpp
    )
endif (${PROFILE} MATCHES "mobile")

include(Coreheaders)
include(EFLHelpers)
include(EWebKitHelpers)

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
    ${CMAKE_CURRENT_SOURCE_DIR}/edc/Settings_mob.edc
    ${CMAKE_CURRENT_BINARY_DIR})
EDJ_TARGET(AutoFillMobileUI.edj
    ${CMAKE_CURRENT_SOURCE_DIR}/edc/AutoFill_mob.edc
    ${CMAKE_CURRENT_BINARY_DIR})
else (${PROFILE} MATCHES "mobile")
EDJ_TARGET(SettingsUI.edj
    ${CMAKE_CURRENT_SOURCE_DIR}/edc/Settings.edc
    ${CMAKE_CURRENT_BINARY_DIR})
endif (${PROFILE} MATCHES "mobile")
