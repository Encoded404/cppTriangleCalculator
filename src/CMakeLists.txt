# Header-only logging interface
add_library(logiface INTERFACE)
target_compile_features(logiface INTERFACE cxx_std_20)
target_include_directories(logiface INTERFACE
	$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src/include>
)

# app library
add_subdirectory(lib1)

# Executable that links the static library
add_subdirectory(app)