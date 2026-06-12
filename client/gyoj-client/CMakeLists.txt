cmake_minimum_required(VERSION 3.16)
project(gyoj_client VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Network QuickWidgets WebView)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Network QuickWidgets WebView)

option(GYOJ_USE_QSCINTILLA "Use QScintilla editor if available" OFF)

set(SOURCES
    src/main.cpp
    src/AppConfig.cpp
    src/ApiClient.cpp
    src/CodeEditor.cpp
    src/LocalRunner.cpp
    src/MainWindow.cpp
    src/ProctorController.cpp
)

set(HEADERS
    src/AppConfig.h
    src/ApiClient.h
    src/CodeEditor.h
    src/LocalRunner.h
    src/MainWindow.h
    src/ProctorController.h
)

add_executable(gyoj-client ${SOURCES} ${HEADERS})
set_target_properties(gyoj-client PROPERTIES QT_NO_LINK_QTMAIN ON)
target_link_libraries(gyoj-client PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Network
    Qt${QT_VERSION_MAJOR}::QuickWidgets
    Qt${QT_VERSION_MAJOR}::WebView
)

add_custom_command(TARGET gyoj-client POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/qml
        $<TARGET_FILE_DIR:gyoj-client>/qml
)

if(WIN32)
    target_link_libraries(gyoj-client PRIVATE user32 shell32 advapi32)
endif()

if(GYOJ_USE_QSCINTILLA)
    target_compile_definitions(gyoj-client PRIVATE GYOJ_USE_QSCINTILLA=1)
    target_link_libraries(gyoj-client PRIVATE qscintilla2_qt${QT_VERSION_MAJOR})
endif()
