# Generate version header from template
configure_file(version.hpp.in ${CMAKE_CURRENT_BINARY_DIR}/version.hpp @ONLY)

# Executable that links the static library
add_executable(TriangleCalculator_app)
target_sources(TriangleCalculator_app PRIVATE
    main.cpp
)
target_link_libraries(TriangleCalculator_app PRIVATE TriangleCalculatorLib)

# Optionally set executable output name; keep simple default
set_target_properties(TriangleCalculator_app PROPERTIES OUTPUT_NAME TriangleCalculator)

# Set the C++ standard and enable compiler features
target_compile_features(TriangleCalculator_app PRIVATE cxx_std_20)

# Compiler-specific warning options for the library target
target_compile_options(TriangleCalculator_app PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
)

# Multi-configuration generator options
if(CMAKE_CONFIGURATION_TYPES)
	target_compile_options(TriangleCalculator_app PRIVATE
        "$<$<CONFIG:Debug>:-g>"
        "$<$<CONFIG:Debug>:-O0>"
        "$<$<CONFIG:Release>:-O3>"
    )
endif()

# Include directories
target_include_directories(TriangleCalculator_app PRIVATE
    # include any app-specific includes here
    ${CMAKE_CURRENT_BINARY_DIR}  # for generated version.hpp
)

# Add compile definitions for logging toggle
# Define LOGIFACE_ENABLE_LOGGING based on ENABLE_LOGGING
target_compile_definitions(TriangleCalculator_app
    PUBLIC
        $<$<BOOL:${ENABLE_LOGGING}>:LOGIFACE_ENABLE_LOGGING=1>
        $<$<NOT:$<BOOL:${ENABLE_LOGGING}>>:LOGIFACE_ENABLE_LOGGING=0>
)