cmake_minimum_required(VERSION 3.16)
project(XORFileProcessor LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

add_executable(XORFileProcessor
    main.cpp
    MainWindow.cpp
    MainWindow.h
    MainWindow.ui
    FileProcessor.h FileProcessor.cpp
    resource.qrc
)

target_link_libraries(XORFileProcessor PRIVATE Qt6::Widgets)
