cmake_minimum_required(VERSION 2.8.11)

project(FlyDetector)


SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")

#Find OpenMP
find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# OpenCV
find_package(OpenCV REQUIRED)

include_directories("../")
include_directories("../dbscan")

file(GLOB_RECURSE sources 
	"../flycounter.cpp"
	"../vials.cpp"
	"../dbscan/rules.cpp"
	"../dbscan/space.cpp"
	"../dbscan/hpdbscan.cpp"
	"../dbscan/points.cpp"
)

add_library(flyDetector SHARED ${sources})

target_link_libraries(flyDetector ${OpenCV_LIBS})


target_compile_features(flyDetector PRIVATE cxx_range_for)

install(TARGETS flyDetector DESTINATION /usr/lib)

ADD_SUBDIRECTORY(python)
