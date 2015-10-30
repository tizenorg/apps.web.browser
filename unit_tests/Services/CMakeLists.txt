project(ServicesTest)

add_subdirectory(TestServiceOne)
add_subdirectory(TestServiceTwo)

#include(Coreheaders)













####### INDIRECT ACCESS ###########################
#add_executable(indirectAccess ServiceInterface.cpp IndirectAccess.cpp)

#add_dependencies(indirectAccess ServiceManager)
#add_dependencies(indirectAccess logger)
#add_dependencies(indirectAccess Config)

#target_link_libraries(indirectAccess browserCore)

#install(TARGETS indirectAccess RUNTIME DESTINATION bin)
######## END INDIRECT ACCESS #######################















####### DIRECT ACCESS ##############################
#add_executable(directAccess ServiceInterface.cpp DirectAccess.cpp)
#add_dependencies(directAccess ServiceManager)
#add_dependencies(directAccess logger)
#add_dependencies(directAccess Config)
#add_dependencies(directAccess TestServiceTwo)

#target_link_libraries(directAccess browserCore)

# TestServiceTwo is accessed directly so it requires linking
#target_link_libraries(directAccess TestServiceTwo)

#install(TARGETS directAccess RUNTIME DESTINATION bin)
####### END DIRECT ACCESS ###########################






















