project(BookmarksUI)

set(BookmarksUI_SRCS
    BookmarksUI.cpp
    )

set(BookmarksUI_HEADERS
    BookmarksUI.h
    )

include(Coreheaders)
include(EFLHelpers)

include_directories(${CMAKE_SOURCE_DIR}/services/FavoriteService)

add_library(${PROJECT_NAME} SHARED ${BookmarksUI_SRCS})

if(TIZEN_BUILD)
    target_link_libraries(${PROJECT_NAME} ${pkgs_LDFLAGS})
endif(TIZEN_BUILD)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)

include(EDCCompile)
EDJ_TARGET(Bookmarks.edj
    ${CMAKE_CURRENT_SOURCE_DIR}/edc/Bookmarks.edc
    ${CMAKE_CURRENT_BINARY_DIR})
