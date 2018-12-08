cmake_minimum_required(VERSION 3.9)
project(frdupl)

set(CMAKE_CXX_STANDARD 11)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}  -fsanitize=address,undefined")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
set(CMAKE_PREFIX_PATH /usr/local/Cellar/qt/5.10.1)

find_package(Qt5Widgets REQUIRED)

add_executable(frdupl
        d_scanner.cpp
        d_scanner.h
        main.cpp
        mainwindow.cpp
        mainwindow.h
        mainwindow.ui)

qt5_use_modules(frdupl Widgets)