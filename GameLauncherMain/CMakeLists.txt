cmake_minimum_required(VERSION 3.16)
project(GameLauncher VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui)

set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/mainwindow.h
    src/gamecard.cpp
    src/gamecard.h
    src/detailpanel.cpp
    src/detailpanel.h
    src/setupdialog.cpp
    src/setupdialog.h
    src/gamemanager.cpp
    src/gamemanager.h
    src/animatedwidget.cpp
    src/animatedwidget.h
    src/backgroundwidget.cpp
    src/backgroundwidget.h
    src/executabledialog.cpp
    src/executabledialog.h
    resources/resources.qrc
    appicon.rc
)

add_executable(GameLauncher ${SOURCES}
    src/pathmanagerdialog.h
    src/pathmanagerdialog.cpp
    src/orderdialog.h
    src/orderdialog.cpp)
target_link_libraries(GameLauncher PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui)

if(WIN32)
    set_target_properties(GameLauncher PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Gui Network)
target_link_libraries(GameLauncher PRIVATE Qt6::Core Qt6::Widgets Qt6::Gui Qt6::Network)