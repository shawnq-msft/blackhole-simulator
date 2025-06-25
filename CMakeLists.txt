cmake_minimum_required(VERSION 3.10)
project(BlackHoleSimulator)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find DirectX packages
find_package(DirectX REQUIRED)

# Include directories
include_directories(
    ${DirectX_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
)

# Source files
file(GLOB_RECURSE SOURCES 
    "src/*.cpp"
    "src/*.h"
)

# Shader files
file(GLOB SHADER_FILES "shaders/*.hlsl")

# Add executable
add_executable(BlackHoleSimulator WIN32 ${SOURCES})

# Link libraries
target_link_libraries(BlackHoleSimulator
    d3d11.lib
    dxgi.lib
    d3dcompiler.lib
    dxguid.lib
)

# Copy shader files to build directory
foreach(SHADER ${SHADER_FILES})
    add_custom_command(
        TARGET BlackHoleSimulator POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${SHADER}
        $<TARGET_FILE_DIR:BlackHoleSimulator>/shaders/
    )
endforeach()

# Create shaders directory in build directory
add_custom_command(
    TARGET BlackHoleSimulator PRE_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory
    $<TARGET_FILE_DIR:BlackHoleSimulator>/shaders/
)