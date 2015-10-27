project(WebPageUI)

set(WebPageUI_SRCS
    WebPageUI.cpp
    ButtonBar.cpp
    URIEntry.cpp
    UrlHistoryList/UrlHistoryList.cpp
    UrlHistoryList/GenlistManager.cpp
    UrlHistoryList/GenlistManagerCallbacks.cpp
    UrlHistoryList/UrlMatchesStyler.cpp
    UrlHistoryList/GenlistItemsManager.cpp
    )

set(WebPageUI_HEADERS
    WebPageUI.h
    ButtonBar.h
    URIEntry.h
    UrlHistoryList/UrlHistoryList.h
    UrlHistoryList/GenlistManager.h
    UrlHistoryList/GenlistManagerCallbacks.h
    UrlHistoryList/UrlMatchesStyler.h
    UrlHistoryList/GenlistItemsManager.h
    )

include(Coreheaders)
include(EFLHelpers)
include(EWebKitHelpers)

include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/services)
include_directories(${CMAKE_SOURCE_DIR}/services/WebEngineService)
include_directories(${CMAKE_SOURCE_DIR}/services/WebEngineService/src)
include_directories(${CMAKE_SOURCE_DIR}/services/MoreMenuUI)
include_directories(${CMAKE_SOURCE_DIR}/services/QuickAccess)
include_directories(${CMAKE_SOURCE_DIR}/services/SettingsUI)
include_directories(${CMAKE_SOURCE_DIR}/services/TabUI)


include(FindPkgConfig)

pkg_check_modules(capi_net QUIET capi-network-connection)

add_library(${PROJECT_NAME} SHARED ${WebPageUI_SRCS})

add_dependencies(${PROJECT_NAME} MoreMenuUI)
add_dependencies(${PROJECT_NAME} QuickAccess)
add_dependencies(${PROJECT_NAME} TabUI)
target_link_libraries(${PROJECT_NAME} MoreMenuUI)
target_link_libraries(${PROJECT_NAME} QuickAccess)
target_link_libraries(${PROJECT_NAME} TabUI)
target_link_libraries(${PROJECT_NAME} ${EFL_LDFLAGS})

IF (${capi_net_FOUND})
    target_link_libraries(${PROJECT_NAME} ${capi_net_LDFLAGS})
ENDIF (${capi_net_FOUND})

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)

include(EDCCompile)

#please do not add edc/ directory
set(edcFiles
    WebPageUI.edc
    ErrorMessage.edc
    LeftButtonBar.edc
    RightButtonBar.edc
    URIEntry.edc
    PrivateMode.edc
    UrlHistoryList.edc
    )

foreach(edec ${edcFiles})
    string(REPLACE ".edc" ".edj" target_name ${edec})
    EDJ_TARGET(${target_name}
               ${CMAKE_CURRENT_SOURCE_DIR}/edc/${edec}
               ${CMAKE_CURRENT_BINARY_DIR})
endforeach(edec)
