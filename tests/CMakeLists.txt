qt_add_executable(abcompare_tests
    unit/tst_audioengine.cpp
)

target_link_libraries(abcompare_tests PRIVATE
    abcompare_core
    Qt6::Test
)

add_test(NAME abcompare_tests COMMAND abcompare_tests)

