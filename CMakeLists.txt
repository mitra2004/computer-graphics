cmake_minimum_required(VERSION 3.15)
project(Galaxy LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)   # Release matters: 150k stars are generated on the CPU
endif()

# Expected layout:
#   main.cpp
#   include/glad/glad.h
#   include/KHR/khrplatform.h
#   src/glad.c
add_executable(Galaxy src/main.cpp src/glad.c)
target_include_directories(Galaxy PRIVATE ${CMAKE_SOURCE_DIR}/include)

# --- GLFW ---------------------------------------------------------------
# Preferred: an installed GLFW (vcpkg, brew, apt, pacman...).
find_package(glfw3 3.3 QUIET)
if(glfw3_FOUND)
    target_link_libraries(Galaxy PRIVATE glfw)
else()
    # Fallback: GLFW source dropped into external/glfw
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
    add_subdirectory(external/glfw)
    target_link_libraries(Galaxy PRIVATE glfw)
endif()

# --- Lines demo ---------------------------------------------------------
add_executable(Lines src/lines.cpp src/glad.c)
target_include_directories(Lines PRIVATE ${CMAKE_SOURCE_DIR}/include)
if(glfw3_FOUND)
    target_link_libraries(Lines PRIVATE glfw)
else()
    target_link_libraries(Lines PRIVATE glfw)
endif()

# --- Platform GL / dl ---------------------------------------------------
find_package(OpenGL REQUIRED)
target_link_libraries(Galaxy PRIVATE OpenGL::GL)
target_link_libraries(Lines PRIVATE OpenGL::GL)

if(UNIX AND NOT APPLE)
    target_link_libraries(Galaxy PRIVATE dl pthread)
    target_link_libraries(Lines PRIVATE dl pthread)
endif()