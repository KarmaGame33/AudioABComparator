cmake_minimum_required(VERSION 3.24)

project(
    ab_compare
    VERSION 1.0.0
    DESCRIPTION "Comparateur audio A/B multiplateforme"
    LANGUAGES C CXX
)

set(AB_COMPARE_RELEASE_VERSION "1.0.0")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 6.9 REQUIRED COMPONENTS Concurrent Core Gui LinguistTools Qml Quick QuickControls2 Multimedia Test)
qt_standard_project_setup(REQUIRES 6.9)

include(FetchContent)
set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build third-party libraries statically" FORCE)
set(ENABLE_TESTS OFF CACHE BOOL "Disable libebur128 tests" FORCE)
set(ENABLE_INTERNAL_QUEUE_H ON CACHE BOOL "Use libebur128 bundled queue implementation" FORCE)
FetchContent_Declare(
    libebur128
    URL https://github.com/jiixyj/libebur128/archive/refs/tags/v1.2.6.tar.gz
    URL_HASH SHA256=baa7fc293a3d4651e244d8022ad03ab797ca3c2ad8442c43199afe8059faa613
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    EXCLUDE_FROM_ALL
)
FetchContent_MakeAvailable(libebur128)

include(CTest)

add_subdirectory(app)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()
