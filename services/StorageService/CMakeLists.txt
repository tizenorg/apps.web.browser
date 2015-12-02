PROJECT(StorageService)

set(StorageServiceSOURCES
	Storage/StorageException.cpp
	Storage/StorageExceptionInitialization.cpp
	Storage/DriverManager.cpp
	Storage/Field.cpp
	Storage/SQLDatabase.cpp
    src/BookmarksSorage.cpp
    src/BookmarksSQLite.cpp
	src/StorageService.cpp
)

include(Coreheaders)
include(EFLHelpers)

include_directories(${CMAKE_SOURCE_DIR}/core/Tools)
include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Storage/include)

add_library(${PROJECT_NAME} SHARED ${StorageServiceSOURCES})
target_link_libraries(${PROJECT_NAME} -lboost_date_time)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)



