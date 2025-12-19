# Set the C++ standard and enable compiler features
add_library(TriangleCalculatorLib STATIC)
target_compile_features(TriangleCalculatorLib PUBLIC cxx_std_20)
target_link_libraries(TriangleCalculatorLib PUBLIC logiface)

# Compiler-specific warning options for the library target
target_compile_options(TriangleCalculatorLib PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Werror>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Werror>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

# Multi-configuration generator options
if(CMAKE_CONFIGURATION_TYPES)
	target_compile_options(TriangleCalculatorLib PRIVATE
        "$<$<CONFIG:Debug>:-O0>"
        "$<$<CONFIG:Release>:-O3>"
    )
endif()

# Include directories
target_include_directories(TriangleCalculatorLib
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/include>
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
)

# Source files
target_sources(TriangleCalculatorLib PRIVATE
    TriangleCalculator.cpp
    TriangleCalculatorBackend.cpp
)