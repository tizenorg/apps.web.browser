PROJECT(StorageService)

set(StorageServiceSOURCES
	Storage/StorageException.cpp
	Storage/StorageExceptionInitialization.cpp
	Storage/DriverManager.cpp
	Storage/Field.cpp
	Storage/SQLDatabase.cpp
    src/BookmarksSorage.cpp
    src/BookmarksSQLite.cpp
	src/HistoryException.cpp
	src/HistorySQLite.cpp
	src/HistoryStorage.cpp
	src/StorageService.cpp
)

set(StorageService_HEADERS
	Storage/include/StorageException.h
	Storage/include/StorageExceptionInitialization.h
	StorageService.h
	BookmarksStorage.h
	BookmarksSQLite.h
	HistoryStorage.h
	HistorySQLite.h
	HistoryException.h
	Storage/include/Field.h
	Storage/include/DriverManager.h
	Storage/include/SQLDatabase.h
    Storage/include/SQLDatabaseImpl.h
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



