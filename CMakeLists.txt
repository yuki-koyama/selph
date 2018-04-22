cmake_minimum_required(VERSION 3.1)

project(SelPh CXX)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_BUILD_TYPE Release)

set(EXTERNAL_INSTALL_DIR ${CMAKE_BINARY_DIR}/external)

include_directories(${EXTERNAL_INSTALL_DIR}/include)
link_directories(${EXTERNAL_INSTALL_DIR}/lib)

add_subdirectory(SelPh)

include(ExternalProject)
ExternalProject_Add(
	mathtoolbox
	GIT_REPOSITORY ${CMAKE_SOURCE_DIR}/external/mathtoolbox
	GIT_TAG c4c693507c8e49a58a21202d605e13e4c9d9bc65
	INSTALL_DIR ${EXTERNAL_INSTALL_DIR}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
)
ExternalProject_Add(
	tinycolormap
	GIT_REPOSITORY ${CMAKE_SOURCE_DIR}/external/tinycolormap
	GIT_TAG v0.1.0
	INSTALL_DIR ${EXTERNAL_INSTALL_DIR}
	CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=<INSTALL_DIR>
)

add_dependencies(SelPh tinycolormap mathtoolbox)
