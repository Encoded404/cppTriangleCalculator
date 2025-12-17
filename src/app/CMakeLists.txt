# Executable that links the static library
add_executable(modern_cmake_template_app)
target_sources(modern_cmake_template_app PRIVATE
    main.cpp
)
target_link_libraries(modern_cmake_template_app PRIVATE modern_cmake_template)

# Optionally set executable output name; keep simple default
set_target_properties(modern_cmake_template_app PROPERTIES OUTPUT_NAME modern_cmake_template)

# Set the C++ standard and enable compiler features
target_compile_features(modern_cmake_template_app PRIVATE cxx_std_20)

# Compiler-specific warning options for the library target
target_compile_options(modern_cmake_template_app PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /permissive->
)

# Multi-configuration generator options
if(CMAKE_CONFIGURATION_TYPES)
	target_compile_options(modern_cmake_template_app PRIVATE
        "$<$<CONFIG:Debug>:-O0>"
        "$<$<CONFIG:Release>:-O3>"
    )
endif()

# Include directories
target_include_directories(modern_cmake_template_app PRIVATE
    # include any app-specific includes here
)

# Add compile definitions for logging toggle
# Define LOGIFACE_ENABLE_LOGGING based on ENABLE_LOGGING
target_compile_definitions(modern_cmake_template_app
    PUBLIC
        $<$<BOOL:${ENABLE_LOGGING}>:LOGIFACE_ENABLE_LOGGING=1>
        $<$<NOT:$<BOOL:${ENABLE_LOGGING}>>:LOGIFACE_ENABLE_LOGGING=0>
)