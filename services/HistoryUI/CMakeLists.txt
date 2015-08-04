project(HistoryUI)

set(HistoryUI_SRCS
    HistoryUI.cpp
    )

set(HistoryUI_HEADERS
    HistoryUI.h
    )

include(Coreheaders)
include(EFLHelpers)

include_directories(${CMAKE_SOURCE_DIR}/services/FavoriteService)

add_library(${PROJECT_NAME} SHARED ${HistoryUI_SRCS})

if(TIZEN_BUILD)
    target_link_libraries(${PROJECT_NAME} ${pkgs_LDFLAGS})
endif(TIZEN_BUILD)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)

include(EDCCompile)
EDJ_TARGET(History.edj
    ${CMAKE_CURRENT_SOURCE_DIR}/edc/History.edc
    ${CMAKE_CURRENT_BINARY_DIR})
