add_executable(tests FooTest.cpp)
target_link_libraries(tests ${COMMON_LIBRARIES})
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin")
