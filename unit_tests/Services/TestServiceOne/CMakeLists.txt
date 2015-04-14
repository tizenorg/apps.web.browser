project(TestServiceOne)

set(TestServiceOne_SRCS
    TestServiceOne.cpp
    )

set(TestServiceOne_HEDS
    TestServiceOne.h
    )

include(Coreheaders)


add_library(${PROJECT_NAME} SHARED ${TestServiceOne_SRCS})


install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)
