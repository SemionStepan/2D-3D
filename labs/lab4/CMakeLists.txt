
cmake_minimum_required(VERSION 3.20)

project(Lab4_OpenGL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(lab4 WIN32 4.cpp)

target_link_libraries(lab4 PRIVATE opengl32 glu32 gdi32 user32)