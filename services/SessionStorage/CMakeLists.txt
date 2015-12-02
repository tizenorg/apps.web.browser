project(SessionStorage)

set(SessionStorage_SRCS
    SessionStorage.cpp
    SqlStorage.cpp
    Session.cpp
    )

include(Coreheaders)
include(EFLHelpers)

include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/services)
include_directories(${CMAKE_SOURCE_DIR}/services/StorageService)
include_directories(${CMAKE_SOURCE_DIR}/services/StorageService/Storage/include)

add_library(${PROJECT_NAME} SHARED ${SessionStorage_SRCS})
target_link_libraries(${PROJECT_NAME} StorageService)
target_link_libraries(${PROJECT_NAME} boost_date_time)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)
