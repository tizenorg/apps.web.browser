project(MoreMenuUI)

set(MoreMenuUI_SRCS
    MoreMenuUI.cpp
    )

set(MoreMenuUI_HEADERS
    MoreMenuUI.h
    )

include(Coreheaders)
include(EFLHelpers)

include_directories(${CMAKE_SOURCE_DIR}/services/FavoriteService)

add_library(${PROJECT_NAME} SHARED ${MoreMenuUI_SRCS})

if(TIZEN_BUILD)
    target_link_libraries(${PROJECT_NAME} ${pkgs_LDFLAGS})
endif(TIZEN_BUILD)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)

#please do not add edc/ directory
if (${PROFILE} MATCHES "mobile")    # mobile profile
    include(EDCCompile_mob)
    set(edcFiles
        MoreMenu_mob.edc
        )
else (${PROFILE} MATCHES "mobile")  # tv profile
    include(EDCCompile)
    set(edcFiles
        MoreMenu.edc
        )
endif (${PROFILE} MATCHES "mobile")

foreach(edec ${edcFiles})
    string(REGEX REPLACE "(_mob)?.edc" ".edj" target_name ${edec})
    EDJ_TARGET(${target_name}
        ${CMAKE_CURRENT_SOURCE_DIR}/edc/${edec}
        ${CMAKE_CURRENT_BINARY_DIR})
endforeach(edec)
