#
# CMake project definition for exercise cv02_CalderonFilter
#

set(target cv02_CalderonFilter)

set(include_path "${CMAKE_CURRENT_SOURCE_DIR}")
set(source_path "${CMAKE_CURRENT_SOURCE_DIR}")

file(GLOB sources
        ${CMAKE_CURRENT_SOURCE_DIR}/cv02_CalderonFilter.cpp
        )

add_executable(${target}
        ${sources}
        )

set_target_properties(${target}
        PROPERTIES
        ${DEFAULT_PROJECT_OPTIONS}
        FOLDER "apps/exercices"
        )

target_include_directories(${target}
        PRIVATE
        ${OpenCV_INCLUDE_DIR}
        ${glfw_INCLUDE_DIR}
        PUBLIC
        INTERFACE
        )

include(${SL_PROJECT_ROOT}/cmake/PlatformLinkLibs.cmake)

target_link_libraries(${target}
        PRIVATE
        ${PlatformLinkLibs}
        ${OpenCV_LIBS}
        ${glfw_LIBS}
        PUBLIC
        INTERFACE
        )

target_compile_definitions(${target}
        PRIVATE
        ${compile_definitions}
        PUBLIC
        ${DEFAULT_COMPILE_DEFINITIONS}
        INTERFACE
        )

target_compile_options(${target}
        PRIVATE
        PUBLIC
        ${DEFAULT_COMPILE_OPTIONS}
        INTERFACE
        )

