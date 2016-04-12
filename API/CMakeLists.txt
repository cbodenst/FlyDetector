cmake_minimum_required(VERSION 2.8.11)

project(FlyDetector)


SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/CMakeModules")
message(STATUS ${CMAKE_MODULE_PATH})

# Find the QtWidgets library
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Core REQUIRED)

include_directories(Qt5Widget_INCLUDE_DIRS)
include_directories(Qt5Core_INCLUDE_DIRS)

#Find OpenMP
find_package(OpenMP REQUIRED)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

# OpenCV
find_package(OpenCV REQUIRED)

#Find libg gphoto
find_package(Gphoto2 REQUIRED)

include_directories("../")
include_directories("../dbscan")

file(GLOB_RECURSE sources 
	"../flycounter.cpp"
	"../vials.cpp"
	"../logger.cpp"
	"../filecam.cpp"
	"../reflexcam.cpp"
	"../webcamera.cpp"
	"../usbshaker.cpp"
	"../noshaker.cpp"
	"../filecam.cpp"
	"../dbscan/rules.cpp"
	"../dbscan/space.cpp"
	"../dbscan/hpdbscan.cpp"
	"../dbscan/points.cpp"
)

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)

add_library(flyDetector SHARED ${sources})

# Use the Widgets module from Qt 5.
target_link_libraries(flyDetector Qt5::Widgets Qt5::Core ${OpenCV_LIBS} ${GPHOTO2_LIBS})


target_compile_features(flyDetector PRIVATE cxx_range_for)


ADD_SUBDIRECTORY(python)
