## goal
a simple triangle calculator lib with a thin CLI wrapper

## requirements
vcpkg, a guide can be found [here](https://learn.microsoft.com/en-us/vcpkg/get_started/get-started?pivots=shell-powershell#1---set-up-vcpkg)
CMake
a cpp compiler (clang, g++ or mvsc)

## clone the repo
git clone https://github.com/Encoded404/cppTriangleCalculator
cd cppTriangleCalculator

## configure
cmake -S . -B build --preset default

## build
cmake --build build

you can also only build one config if that is wanted
cmake --build build --preset debug (debug|release|relwithdebinfo)

## run tests
ctest --test-dir build/ -C Debug (Debug|Release|RelWithDebInfo)