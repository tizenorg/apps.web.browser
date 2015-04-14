project(FavoriteService)

set(FavoriteService_SRCS
    FavoriteService.cpp
    )

set(FavoriteService_HEADERS
    FavoriteService.h
    )

include(Coreheaders)
include(EFLHelpers)

include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR})
include_directories(${CMAKE_SOURCE_DIR}/services/StorageService)
include_directories(${CMAKE_SOURCE_DIR}/services/StorageService/Storage/include)
include_directories(${CMAKE_SOURCE_DIR}/core/AbstractInterfaces)

add_library(${PROJECT_NAME} SHARED ${FavoriteService_SRCS})

if(TIZEN_BUILD)
    target_link_libraries(${PROJECT_NAME} ${pkgs_LDFLAGS})
endif(TIZEN_BUILD)

add_dependencies(${PROJECT_NAME} StorageService)
target_link_libraries(${PROJECT_NAME} StorageService)

set (IMAGES
      images/samsung.ico images/samsung.png
      images/tizen.ico images/tizen.png
      images/google.ico images/google.png
     )

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)

install(FILES ${IMAGES} DESTINATION res)
