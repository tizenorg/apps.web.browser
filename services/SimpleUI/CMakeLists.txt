project(SimpleUI)

set(SimpleUI_SRCS
    SimpleUI.cpp
    ButtonBar.cpp
    SimplePopup.cpp
    BookmarksManager.cpp
    ViewManager.cpp
    )

set(SimpleUI_HEADERS
    SimpleUI.h
    ButtonBar.h
    SimplePopup.h
    BookmarksManager.h
    ViewManager.h
    )

include(Coreheaders)
include(EFLHelpers)
include(EWebKitHelpers)

include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/services)
include_directories(${CMAKE_SOURCE_DIR}/services/SimpleURI)
include_directories(${CMAKE_SOURCE_DIR}/services/WebEngineService)
include_directories(${CMAKE_SOURCE_DIR}/services/WebEngineService/src)
include_directories(${CMAKE_SOURCE_DIR}/services/StorageService)
include_directories(${CMAKE_SOURCE_DIR}/services/BookmarkManagerUI)
include_directories(${CMAKE_SOURCE_DIR}/services/StorageService/Storage/include)
include_directories(${CMAKE_SOURCE_DIR}/services/FavoriteService)
include_directories(${CMAKE_SOURCE_DIR}/services/HistoryService)
include_directories(${CMAKE_SOURCE_DIR}/services/MoreMenuUI)
include_directories(${CMAKE_SOURCE_DIR}/services/HistoryUI)
include_directories(${CMAKE_SOURCE_DIR}/services/MainUI)
include_directories(${CMAKE_SOURCE_DIR}/services/SettingsUI)
include_directories(${CMAKE_SOURCE_DIR}/services/TabUI)
include_directories(${CMAKE_SOURCE_DIR}/services/PlatformInputManager)
include_directories(${CMAKE_SOURCE_DIR}/services/SessionStorage)

include(FindPkgConfig)

pkg_check_modules(capi_net QUIET capi-network-connection)

add_library(${PROJECT_NAME} SHARED ${SimpleUI_SRCS})

add_dependencies(${PROJECT_NAME} SimpleURI)
add_dependencies(${PROJECT_NAME} StorageService)
add_dependencies(${PROJECT_NAME} HistoryService)
add_dependencies(${PROJECT_NAME} MoreMenuUI)
add_dependencies(${PROJECT_NAME} BookmarkManagerUI)
add_dependencies(${PROJECT_NAME} MainUI)
add_dependencies(${PROJECT_NAME} HistoryUI)
add_dependencies(${PROJECT_NAME} TabUI)
add_dependencies(${PROJECT_NAME} SettingsUI)
add_dependencies(${PROJECT_NAME} PlatformInputManager)
add_dependencies(${PROJECT_NAME} SessionStorage)
target_link_libraries(${PROJECT_NAME} SimpleURI)
target_link_libraries(${PROJECT_NAME} StorageService)
target_link_libraries(${PROJECT_NAME} HistoryService)
target_link_libraries(${PROJECT_NAME} MoreMenuUI)
target_link_libraries(${PROJECT_NAME} MainUI)
target_link_libraries(${PROJECT_NAME} HistoryUI)
target_link_libraries(${PROJECT_NAME} TabUI)
target_link_libraries(${PROJECT_NAME} SettingsUI)
target_link_libraries(${PROJECT_NAME} BookmarkManagerUI)
target_link_libraries(${PROJECT_NAME} PlatformInputManager)
target_link_libraries(${PROJECT_NAME} SessionStorage)
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
    MainLayout.edc
    LeftButtonBar.edc
    RightButtonBar.edc
    AuthenticationPopup.edc
    BookmarksManager.edc
    ScrollerDefault.edc
    ErrorMessage.edc
    Tooltip.edc
    )

foreach(edec ${edcFiles})
    string(REPLACE ".edc" ".edj" target_name ${edec})
    EDJ_TARGET(${target_name}
               ${CMAKE_CURRENT_SOURCE_DIR}/edc/${edec}
               ${CMAKE_CURRENT_BINARY_DIR})
endforeach(edec)
