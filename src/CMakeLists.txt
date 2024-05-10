cmake_minimum_required(VERSION 3.28)
project(paint)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(CMAKE_PREFIX_PATH "C:/Qt/6.7.0/msvc2019_64")

find_package(Qt6 COMPONENTS
        Core
        Gui
        Widgets
        OpenGL
        OpenGLWidgets
        REQUIRED)

find_package(OpenGL REQUIRED)

add_executable(paint main.cpp)
target_link_libraries(paint
        Qt::Core
        Qt::Gui
        Qt::Widgets
        Qt::OpenGL
        Qt::OpenGLWidgets
        OpenGL::GL
)

if (WIN32 AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
endif ()

set(PAINT_BIN_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../bin)

set_target_properties(paint
        PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${PAINT_BIN_DIR}
        RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL ${PAINT_BIN_DIR}
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${PAINT_BIN_DIR}
        RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PAINT_BIN_DIR}
        LIBRARY_OUTPUT_DIRECTORY_RELEASE ${PAINT_BIN_DIR}
        LIBRARY_OUTPUT_DIRECTORY_MINSIZEREL ${PAINT_BIN_DIR}
        LIBRARY_OUTPUT_DIRECTORY_DEBUG ${PAINT_BIN_DIR}
        LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${PAINT_BIN_DIR}
)