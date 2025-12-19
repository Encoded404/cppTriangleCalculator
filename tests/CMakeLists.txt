# Add the test executable
add_executable(TriangleCalculatorTests TriangleCalculatorTests.cpp)

# Find GTest installed via vcpkg and link it along with the main library
find_package(GTest CONFIG REQUIRED)
find_package(nlohmann_json CONFIG REQUIRED)
include(GoogleTest)
target_link_libraries(TriangleCalculatorTests PRIVATE
    TriangleCalculatorLib
    GTest::gtest
    GTest::gtest_main
    nlohmann_json::nlohmann_json
)

# Set compiler options for the test executable
target_compile_options(TriangleCalculatorTests PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Werror>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Werror>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

# Multi-configuration generator options
if(CMAKE_CONFIGURATION_TYPES)
	target_compile_options(TriangleCalculatorTests PRIVATE
        "$<$<CONFIG:Debug>:-g>"
        "$<$<CONFIG:Debug>:-O0>"
        "$<$<CONFIG:Release>:-O3>"
    )
endif()

# Set the environment variable for the seed
set(TRIANGLE_GEN_SEED 42 CACHE STRING "Seed for triangle generation")

# Pass the seed as an environment variable
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/triangles_fp.json
    COMMAND ${CMAKE_COMMAND} -E env TRIANGLE_GEN_SEED=${TRIANGLE_GEN_SEED} ${Python3_EXECUTABLE} ./TriangleGenerator.py
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/TriangleGenerator.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating triangles for tests"
)

# Ensure the test executable depends on the generated file
add_custom_target(GenerateTriangles ALL
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/triangles_fp.json
)
add_dependencies(TriangleCalculatorTests GenerateTriangles)

# Let CTest discover individual GoogleTest tests in the binary
gtest_discover_tests(TriangleCalculatorTests
    PROPERTIES
        ENVIRONMENT "TEST_LOG_DIR=${CMAKE_BINARY_DIR}/Testing/logs"
)