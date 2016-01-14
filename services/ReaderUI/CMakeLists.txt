project(ReaderUI)

set(ReaderUI_SRCS
    ReaderUI.cpp
    )

set(ReaderUI_HEADERS
    ReaderUI.h
    )

include(Coreheaders)
include(EFLHelpers)
include(EWebKitHelpers)

add_library(${PROJECT_NAME} SHARED ${ReaderUI_SRCS})

if(TIZEN_BUILD)
    target_link_libraries(${PROJECT_NAME} ${pkgs_LDFLAGS})
endif(TIZEN_BUILD)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)
