cmake_minimum_required(VERSION 3.15)
project(CHALKFORGE LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
add_executable(CHALKFORGE
    src/ChalkApp.cpp
    src/ChalkError.cpp
    src/ChalkGraphics.cpp
    src/ChalkLogger.cpp
    src/ChalkWindow.cpp
)
target_include_directories(CHALKFORGE PRIVATE src)
target_compile_definitions(CHALKFORGE PRIVATE
    NOMINMAX
    UNICODE
    _UNICODE
    WINVER=0x0A00
    _WIN32_WINNT=0x0A00
    $<$<CONFIG:Debug>:CHALK_DEBUG_LAYER>
)
if(MSVC)
    target_compile_options(CHALKFORGE PRIVATE /utf-8)
endif()
if(WIN32)
    target_link_options(CHALKFORGE PRIVATE /subsystem:windows)
    target_link_libraries(CHALKFORGE PRIVATE d3d12 dxgi d3dcompiler)
endif()

add_custom_command(TARGET CHALKFORGE POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/shaders"
        "$<TARGET_FILE_DIR:CHALKFORGE>/shaders"
    COMMENT "Copying shaders to output directory"
)
