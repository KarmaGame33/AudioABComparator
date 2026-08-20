add_library(abcompare_core STATIC
    src/audio/AudioEngine.cpp
    src/audio/AudioEngine.h
    src/audio/PcmIODevice.cpp
    src/audio/PcmIODevice.h
)

target_include_directories(abcompare_core PUBLIC src)
target_link_libraries(abcompare_core PUBLIC Qt6::Core Qt6::Multimedia)

qt_add_executable(ab-compare
    src/main.cpp
)

target_compile_definitions(ab-compare PRIVATE
    AB_COMPARE_VERSION="${AB_COMPARE_RELEASE_VERSION}"
)

qt_add_qml_module(ab-compare
    URI AudioAB
    VERSION 1.0
    QML_FILES
        ui/Main.qml
        ui/components/WaveformPanel.qml
        ui/components/InteractiveTimeline.qml
)

target_link_libraries(ab-compare PRIVATE
    abcompare_core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
)

set_target_properties(ab-compare PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "AB Compare"
)

install(TARGETS ab-compare
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)
