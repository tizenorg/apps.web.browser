project(MainUI)

set(MainUI_SRCS
    MainUI.cpp
    )

set(MainUI_HEADERS
    MainUI.h
    )

include(Coreheaders)
include(EFLHelpers)

include_directories(${CMAKE_SOURCE_DIR}/services/FavoriteService)

add_library(${PROJECT_NAME} SHARED ${MainUI_SRCS})

if(TIZEN_BUILD)
    target_link_libraries(${PROJECT_NAME} ${pkgs_LDFLAGS})
endif(TIZEN_BUILD)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)

include(EDCCompile)
EDJ_TARGET(MainUI.edj
    ${CMAKE_CURRENT_SOURCE_DIR}/edc/MainUI.edc
    ${CMAKE_CURRENT_BINARY_DIR})
