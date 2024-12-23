cmake_minimum_required(VERSION 3.29)
project(hecate_c_compiler_c)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find LLVM
find_package(LLVM REQUIRED CONFIG)

# Debug: Print LLVM include directories
message(STATUS "LLVM include directories: ${LLVM_INCLUDE_DIRS}")

# Include LLVM and Clang directories
include_directories(SYSTEM ${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

# Add the executable
add_executable(hecate_c_compiler_c main.cpp)

# Manually add /usr/include if needed
include_directories(SYSTEM /usr/include)
include_directories(/usr/include/clang)
# Link LLVM and Clang libraries
llvm_map_components_to_libnames(llvm_libs support core irreader)
target_link_libraries(hecate_c_compiler_c ${llvm_libs})
