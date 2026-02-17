cmake_minimum_required(VERSION 3.14)
project(cask_foundation)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(
    cask_core
    GIT_REPOSITORY git@github.com:s-ajensen/cask_core.git
    GIT_TAG master
)
FetchContent_MakeAvailable(cask_core)

if(NOT TARGET cask_redux)
    add_library(cask_redux ALIAS cask_engine)
endif()

FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.4.0
)
FetchContent_MakeAvailable(Catch2)

function(add_cask_plugin plugin_name)
    set(plugin_src plugins/${plugin_name}/${plugin_name}_plugin.cpp)

    add_library(${plugin_name}_plugin SHARED ${plugin_src})
    target_link_libraries(${plugin_name}_plugin PRIVATE cask_engine cask_core)
    set_target_properties(${plugin_name}_plugin PROPERTIES PREFIX "")

    add_executable(${plugin_name}_tests
        spec/${plugin_name}/${plugin_name}_plugin_spec.cpp
        ${plugin_src}
    )
    target_link_libraries(${plugin_name}_tests PRIVATE cask_engine cask_core Catch2::Catch2WithMain)
    catch_discover_tests(${plugin_name}_tests)
endfunction()

include(CTest)
list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)

add_cask_plugin(event)
add_cask_plugin(interpolation)
add_cask_plugin(resource)
add_cask_plugin(entity)
