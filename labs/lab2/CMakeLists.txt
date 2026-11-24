cmake_minimum_required(VERSION 3.20)

project(Lab2_OpenCV)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenCV REQUIRED)

add_executable(lab2 2.cpp)

target_link_libraries(lab2 PRIVATE ${OpenCV_LIBS})