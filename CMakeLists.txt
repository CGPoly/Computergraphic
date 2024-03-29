cmake_minimum_required(VERSION 3.1)
project(AnimationProject)

option(GLFW_BUILD_DOCS OFF)
option(GLFW_BUILD_EXAMPLES OFF)
option(GLFW_BUILD_TESTS OFF)
add_subdirectory(vendor/glfw)

set (CMAKE_CXX_STANDARD 20)
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    if(NOT WIN32)
        set(GLAD_LIBRARIES dl)
    endif()
endif()

configure_file (
        "${PROJECT_SOURCE_DIR}/config.hpp.in"
        "${PROJECT_SOURCE_DIR}/generated/config.hpp"
)

include_directories(
        generated/
        vendor/glad/include/
        vendor/glfw/include/
        vendor/glm/
        vendor/stb/
)

file(GLOB VENDORS_SOURCES
        vendor/glad/src/glad.c
        vendor/imgui/*.cpp)
file(GLOB VENDORS_HEADERS
        vendor/imgui/*.h)
file(GLOB LIBRARY_SOURCES src/library/*.cpp)
file(GLOB LIBRARY_HEADERS src/library/*.hpp)
file(GLOB PROJECT_SOURCES src/*.cpp)
file(GLOB PROJECT_HEADERS src/*.hpp)
file(GLOB PROJECT_SHADERS
        shaders/*.comp
        shaders/*.frag
        shaders/*.geom
        shaders/*.vert)
file(GLOB PROJECT_CONFIGS
        CMakeLists.txt
        #Readme.md
        .gitignore
        .gitmodules)

source_group("Sources" FILES ${PROJECT_SOURCES})
source_group("Headers" FILES ${PROJECT_HEADERS})
source_group("Shaders" FILES ${PROJECT_SHADERS})
source_group("Library" FILES ${LIBRARY_SOURCES})
source_group("LibraryHeaders" FILES ${LIBRARY_HEADERS})
source_group("Vendors" FILES ${VENDORS_SOURCES})
source_group("VendorHeaders" FILES ${VENDORS_HEADERS})

add_definitions(-DGLFW_INCLUDE_NONE -DPROJECT_SOURCE_DIR=\"${PROJECT_SOURCE_DIR}\")

#add_executable(
#        ${PROJECT_NAME}
#        ${PROJECT_SOURCES}
#        ${PROJECT_HEADERS}
#        ${PROJECT_SHADERS}
#        ${PROJECT_CONFIGS}
#        ${LIBRARY_SOURCES}
#        ${LIBRARY_HEADERS}
#        ${VENDORS_SOURCES}
#        ${VENDORS_HEADERS}
#)
#target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} ${GLAD_LIBRARIES})
foreach(PROJECT_SOURCE_FILE ${PROJECT_SOURCES})
    get_filename_component(SRC_NAME ${PROJECT_SOURCE_FILE} NAME_WE)
    add_executable(
            ${SRC_NAME}
            ${PROJECT_SOURCE_FILE}
            ${PROJECT_HEADERS}
            ${PROJECT_SHADERS}
            ${PROJECT_CONFIGS}
            ${LIBRARY_SOURCES}
            ${LIBRARY_HEADERS}
            ${VENDORS_SOURCES}
            ${VENDORS_HEADERS}
    )
    target_link_libraries(${SRC_NAME} glfw ${GLFW_LIBRARIES} ${GLAD_LIBRARIES})
    #set_target_properties(${SRC_NAME} PROPERTIES
    #RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/${PROJECT_NAME})
endforeach(PROJECT_SOURCE_FILE)

