cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)

project(PrettyPath)

include(FetchContent)
include(CheckCXXCompilerFlag)

FetchContent_Declare(
	libosmium
	GIT_REPOSITORY https://github.com/osmcode/libosmium.git
	GIT_TAG        v2.20.0
)

FetchContent_Declare(
    protozero
    GIT_REPOSITORY https://github.com/mapbox/protozero.git
    GIT_TAG        v1.7.1
)

FetchContent_GetProperties(libosmium)
if(NOT libosmium_POPULATED)
  FetchContent_Populate(libosmium)
endif()

FetchContent_GetProperties(protozero)
if(NOT protozero_POPULATED)
  FetchContent_Populate(protozero)
endif()

find_package(ZLIB REQUIRED)
if(NOT ZLIB_FOUND)
	message(FATAL_ERROR "ZLIB not found")
endif()
find_package(GDAL REQUIRED)
if(NOT GDAL_FOUND)
	message(FATAL_ERROR "GDAL not found")
endif()

set(FAST_MATH_FLAGS -ffast-math -fno-unsafe-math-optimizations)
CHECK_CXX_COMPILER_FLAG("${FAST_MATH_FLAGS}" COMPILER_SUPPORTS_FAST_MATH)
CHECK_CXX_COMPILER_FLAG(-march=native COMPILER_SUPPORTS_MARCH_NATIVE)

if (COMPILER_SUPPORTS_FAST_MATH)
  add_compile_options(${FAST_MATH_FLAGS})
endif()

if (COMPILER_SUPPORTS_MARCH_NATIVE)
  add_compile_options(-march=native)
endif()

set (
  OSMParser_sources
  src/OSMParser/handler.cpp
  src/OSMParser/main.cpp
)

set (
  PrettyPath_sources
  src/PrettyPath/main.cpp
  src/PrettyPath/parser.cpp
  src/PrettyPath/graph.cpp
  src/PrettyPath/pathfinder.cpp
)

add_executable(${PROJECT_NAME} ${PrettyPath_sources})
target_include_directories(${PROJECT_NAME} PRIVATE ${GDAL_INCLUDE_DIRS} include/PrettyPath)
target_link_libraries(${PROJECT_NAME} PRIVATE ${GDAL_LIBRARIES})

add_executable(OSMParser ${OSMParser_sources})
target_include_directories(OSMParser PRIVATE include/OSMParser ${libosmium_SOURCE_DIR}/include ${protozero_SOURCE_DIR}/include ${ZLIB_INCLUDE_DIRS} ${GDAL_INCLUDE_DIRS})
target_link_libraries(OSMParser ${ZLIB_LIBRARIES} ${GDAL_LIBRARIES})
