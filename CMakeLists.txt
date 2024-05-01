cmake_minimum_required(VERSION 3.11)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

project(PrettyPath)

include(FetchContent)

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

add_executable(${PROJECT_NAME} src/prettypath.cpp)
target_include_directories(${PROJECT_NAME} PRIVATE ${GDAL_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${GDAL_LIBRARIES})
add_executable(OSMParser src/osmparser.cpp)
target_include_directories(OSMParser PRIVATE ${libosmium_SOURCE_DIR}/include ${protozero_SOURCE_DIR}/include ${ZLIB_INCLUDE_DIRS} ${GDAL_INCLUDE_DIRS})
target_link_libraries(OSMParser ${ZLIB_LIBRARIES} ${GDAL_LIBRARIES})
