cmake_minimum_required(VERSION 3.1)

project(SelPh CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Release)

add_subdirectory(external/mathtoolbox)
set(MATHTOOLBOX_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/mathtoolbox/)

# Set include directory paths for header-only libraries
set(TINYCOLORMAP_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/external/tinycolormap/include)

add_subdirectory(SelPh)
