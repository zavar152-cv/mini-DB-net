cmake_minimum_required(VERSION 3.23)
project(ZPathProject C)

set(CMAKE_C_STANDARD 17)

add_subdirectory(zgdbServer)
add_subdirectory(zpathClient)