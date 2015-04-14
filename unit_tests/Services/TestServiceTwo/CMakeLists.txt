project(TestServiceTwo)

set(TestServiceTwo_SRCS
    TestServiceTwo.cpp
    )

set(TestServiceTwo_HEDS
    TestServiceTwo.h
    )

include(Coreheaders)


add_library(${PROJECT_NAME} SHARED ${TestServiceTwo_SRCS})


install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)
