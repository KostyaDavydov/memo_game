cmake_minimum_required(VERSION 3.31)

set(CMAKE_AUTOMOC ON)

project(MemoGame LANGUAGES CXX)

find_package(Qt5 REQUIRED Core Widgets)

add_executable(${PROJECT_NAME}
                   "${CMAKE_SOURCE_DIR}/src/main.cpp"
                   "${CMAKE_SOURCE_DIR}/src/mainwidget.cpp"
                   "${CMAKE_SOURCE_DIR}/src/carditem.cpp"
                   "${CMAKE_SOURCE_DIR}/src/gamescene.cpp"
                   "${CMAKE_SOURCE_DIR}/include/mainwidget.h"
                   "${CMAKE_SOURCE_DIR}/include/carditem.h"
                   "${CMAKE_SOURCE_DIR}/include/gamescene.h")

target_include_directories(${PROJECT_NAME}
                               PRIVATE "${CMAKE_SOURCE_DIR}/include")

target_link_libraries(${PROJECT_NAME}
                          Qt5::Core
                          Qt5::Widgets)
