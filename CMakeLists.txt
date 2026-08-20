cmake_minimum_required(VERSION 3.24)

project(
    ab_compare
    VERSION 0.2.1
    DESCRIPTION "Comparateur audio A/B multiplateforme"
    LANGUAGES CXX
)

set(AB_COMPARE_RELEASE_VERSION "0.2.1-beta.1")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.9 REQUIRED COMPONENTS Core Gui Qml Quick QuickControls2 Multimedia Test)
qt_standard_project_setup(REQUIRES 6.9)

include(CTest)

add_subdirectory(app)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
