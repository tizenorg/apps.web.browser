project(SimpleURI)

set(SimpleURI_SRCS
    SimpleURI.cpp
    )

set(SimpleURI_HEADERS
    SimpleURI.h
    )

include(Coreheaders)
include(EFLHelpers)

add_library(${PROJECT_NAME} SHARED ${SimpleURI_SRCS})

if(TIZEN_BUILD)
    target_link_libraries(${PROJECT_NAME} ${pkgs_LDFLAGS})
    target_link_libraries(${PROJECT_NAME} boost_regex)
endif(TIZEN_BUILD)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)

EDJ_TARGET(URIEntry.edj
    ${CMAKE_CURRENT_SOURCE_DIR}/edc/URIEntry.edc
    ${CMAKE_CURRENT_BINARY_DIR})
