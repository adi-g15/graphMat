cmake_minimum_required(VERSION 3.14)

project(graphMat)

include(cmake/CPM.cmake)

CPMAddPackage("gh:adi-g15/utilpp@1.0.4")

# specify this library not meant to generate any build output
add_library(graphMat INTERFACE)
target_include_directories(graphMat INTERFACE include/)
target_link_libraries(graphMat INTERFACE utilpp)

install(DIRECTORY include/graphMat
        TYPE INCLUDE)
