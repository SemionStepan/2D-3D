
cmake_minimum_required(VERSION 3.20)

project(Lab5_OpenGL_Lighting LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(lab5 WIN32 5.cpp)

target_link_libraries(lab5 PRIVATE opengl32 glu32 gdi32 user32)