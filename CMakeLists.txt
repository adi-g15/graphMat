cmake_minimum_required(VERSION 3.14)

project(graphMat)

# specify this library not meant to generate any build output
add_library(graphMat INTERFACE)
target_include_directories(graphMat INTERFACE include/)

install(DIRECTORY include/graphMat
        TYPE INCLUDE)
