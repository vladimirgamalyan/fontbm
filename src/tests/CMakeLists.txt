add_executable(tests
        catch.hpp
        TestRunner.cpp
        TestHelpers.cpp
        ../ProgramOptions.cpp
        )

target_link_libraries(tests ${COMMON_LIBRARIES})
