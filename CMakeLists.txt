cmake_minimum_required(VERSION 3.13.4)

project(gameboy-emulator)

set(CMAKE_BUILD_TYPE Debug)

add_subdirectory(gbemulator/src)
add_subdirectory(gbemulator/test/src)
