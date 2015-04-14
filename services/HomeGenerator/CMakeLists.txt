project(HomeGenerator)


set(HomeGenerator_SRCS
    HomeGenerator.cpp
    )

set(HomeGenerator_HEADERS
    HomeGenerator.h
    )

include(Coreheaders)

add_library(${PROJECT_NAME} SHARED ${HomeGenerator_SRCS})
add_dependencies(${PROJECT_NAME} browserCore)
target_link_libraries(${PROJECT_NAME} browserCore)

# set(PageThubs
#     01.png
#     02.png
#     03.png
#     04.png
#     05.png
#     06.png
#     07.png
# )
file(GLOB PageThubs *.png)
file(GLOB PageFavs *.ico)

# set(PageFavs
#     01.ico
#     02.ico
#     03.ico
#     04.ico
#     05.ico
#     06.ico
#     07.ico
#     )

set(HtmlFiles
    HomePage.css
    jquery-1.11.0.min.js
    mousetrap.min.js
    navi.js)

install(TARGETS ${PROJECT_NAME}
            LIBRARY DESTINATION services
            ARCHIVE DESTINATION services/static)

install(FILES ${PageThubs} DESTINATION res)
install(FILES ${PageFavs} DESTINATION res)
install(FILES ${HtmlFiles} DESTINATION res)

