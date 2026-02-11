cmake_minimum_required(VERSION 3.10)
project(Basic)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)

# Handle generic OpenGL finding
find_package(OpenGL REQUIRED)

include_directories(include)
include_directories(${GTK3_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIR})

link_directories(${GTK3_LIBRARY_DIRS})
add_definitions(${GTK3_CFLAGS_OTHER})

file(GLOB SOURCES "src/*.cpp")

add_executable(Basic ${SOURCES})

# Link libraries - try targets first, fall back to variables
if(TARGET OpenGL::GL)
    target_link_libraries(Basic ${GTK3_LIBRARIES} OpenGL::GL)
else()
    target_link_libraries(Basic ${GTK3_LIBRARIES} ${OPENGL_gl_LIBRARY})
endif()

if(TARGET OpenGL::GL)
    target_link_libraries(Basic ${GTK3_LIBRARIES} OpenGL::GL)
else()
    target_link_libraries(Basic ${GTK3_LIBRARIES} ${OPENGL_gl_LIBRARY})
endif()
