cmake_minimum_required(VERSION 3.5)

project( glcar3o VERSION 0.1 LANGUAGES C CXX )
set(PROJECT_DESCRIPTION "Chasm: The Rift 3D .car/.3o animation model OpenGL viewer")

include(GNUInstallDirs)

include(CheckIncludeFileCXX)
if( NOT CMAKE_BUILD_TYPE )
        set( CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE )
endif()

set( CMAKE_C_STANDARD 23 )
set( CMAKE_C_STANDARD_REQUIRED ON )
set( CMAKE_C_EXTENSIONS ON )

if( NOT CMAKE_BUILD_TYPE STREQUAL Debug )
add_compile_options(-O3)
else()
add_compile_options(-O0 -g)
endif()

add_compile_options(-march=native)
add_compile_options(-D_FILE_OFFSET_BITS=64)

find_package(OpenGL)
find_package(GLEW)
find_package(freeglut)

add_executable( glcar3o src/glcar3o.c )
target_include_directories( glcar3o PUBLIC
        PUBLIC_HEADER $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries( glcar3o PUBLIC m OpenGL::GL OpenGL::GLU glut)

add_executable( 3oviewer external/3oviewer.c )
target_include_directories( 3oviewer PUBLIC
        PUBLIC_HEADER $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries( 3oviewer PUBLIC m OpenGL::GL OpenGL::GLU glut)

add_executable( carviewer external/carviewer.c )
target_include_directories( 3oviewer PUBLIC
        PUBLIC_HEADER $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_link_libraries( carviewer PUBLIC m OpenGL::GL OpenGL::GLU glut)

install(TARGETS glcar3o 3oviewer carviewer DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT EXECUTABLES)
