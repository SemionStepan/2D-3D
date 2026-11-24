
cmake_minimum_required(VERSION 3.20)

project(Lab7_ParticleSystem LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(lab7 WIN32 7.cpp)

target_link_libraries(lab7 PRIVATE opengl32 glu32 gdi32 user32)
