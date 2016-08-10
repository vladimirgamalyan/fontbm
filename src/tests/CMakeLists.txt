add_executable(tests TestRunner.cpp TestHelpers.cpp ../ProgramOptions.cpp)
target_link_libraries(tests ${COMMON_LIBRARIES} ${Boost_LIBRARIES})
