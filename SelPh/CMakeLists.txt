cmake_minimum_required(VERSION 3.1)

find_package(Eigen3 REQUIRED)
find_package(NLopt REQUIRED)
find_package(Qt5 COMPONENTS Widgets OpenGL Concurrent REQUIRED)
find_package(OpenGL REQUIRED)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

include_directories(${EIGEN3_INCLUDE_DIR})
include_directories(${NLopt_INCLUDE_DIRS})
include_directories(${OPENGL_INCLUDE_DIRS})

file(GLOB files *.cpp *.h *.ui)
file(GLOB data_files_small ../resources/data/test_set_small/*.JPG)
file(GLOB data_files ../resources/data/test_set/*.JPG)
file(GLOB shader_files ${ENHANCER_VERT_SHADER_PATH} ${ENHANCER_FRAG_SHADER_PATH})

add_executable(
	SelPh
	MACOSX_BUNDLE
	${files}
	${data_files}
	${data_files_small}
	${shader_files}
)

set_source_files_properties(
	${data_files_small}
	PROPERTIES
	MACOSX_PACKAGE_LOCATION Resources/data/test_set_small
)
set_source_files_properties(
	${data_files}
	PROPERTIES
	MACOSX_PACKAGE_LOCATION Resources/data/test_set
)
set_source_files_properties(
	${shader_files}
	PROPERTIES
	MACOSX_PACKAGE_LOCATION Resources/shaders
)
set_target_properties(SelPh PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/resources/Info.plist.in)
target_link_libraries(SelPh Qt5::Widgets Qt5::Concurrent Qt5::OpenGL ${NLopt_LIBRARIES} ${OPENGL_LIBRARIES} mathtoolbox enhancer parallel-util)
