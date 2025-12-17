# Set the C++ standard and enable compiler features
add_library(modern_cmake_template STATIC)
target_compile_features(modern_cmake_template PUBLIC cxx_std_20)
target_link_libraries(modern_cmake_template PUBLIC logiface)

# Compiler-specific warning options for the library target
target_compile_options(modern_cmake_template PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Werror>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Werror>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
)

# Multi-configuration generator options
if(CMAKE_CONFIGURATION_TYPES)
	target_compile_options(modern_cmake_template PRIVATE
        "$<$<CONFIG:Debug>:-O0>"
        "$<$<CONFIG:Release>:-O3>"
    )
endif()

# Include directories
target_include_directories(modern_cmake_template
PUBLIC
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/include>
)

# Source files
target_sources(modern_cmake_template PRIVATE
    lib_example.cpp
    lib_privateCode.cpp
)