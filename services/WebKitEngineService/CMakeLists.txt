project(WebKitEngineService)

set(WebKitEngineService_SRCS
    WebKitEngineService.cpp
    WebView.cpp
    )

include(Coreheaders)
include(EFLHelpers)
include(EWebKitHelpers)

include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})

add_library(${PROJECT_NAME} SHARED ${WebKitEngineService_SRCS})
target_link_libraries(${PROJECT_NAME} ${EFL_LDFLAGS})
target_link_libraries(${PROJECT_NAME} ${EWEBKIT2_LDFLAGS})
target_link_libraries(${PROJECT_NAME} boost_regex)
target_link_libraries(${PROJECT_NAME} boost_thread)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)
