include(GNUInstallDirs)

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
    src/LanguageManager.cpp
    src/LanguageManager.h
)

set(app_icon_png resources/audio-ab-comparator.png)
set_source_files_properties(${app_icon_png} PROPERTIES
    QT_RESOURCE_ALIAS audio-ab-comparator.png
)
qt_add_resources(ab-compare "application_icon"
    PREFIX "/icons"
    FILES ${app_icon_png}
)

if(WIN32)
    target_sources(ab-compare PRIVATE
        resources/windows/audio-ab-comparator.rc
    )
endif()

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

qt_add_translations(ab-compare
    TS_FILES
        translations/ab_compare_de.ts
        translations/ab_compare_es.ts
        translations/ab_compare_fr.ts
        translations/ab_compare_ja.ts
        translations/ab_compare_pt_BR.ts
        translations/ab_compare_zh_CN.ts
    RESOURCE_PREFIX "/i18n"
)

set_target_properties(ab-compare PROPERTIES
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "AB Compare"
)

install(TARGETS ab-compare
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(UNIX AND NOT APPLE)
    install(FILES resources/audio-ab-comparator.svg
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
        RENAME io.github.KarmaGame33.AudioABComparator.svg
    )

    install(FILES resources/linux/io.github.KarmaGame33.AudioABComparator.desktop
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
    )
endif()
