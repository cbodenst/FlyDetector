FIND_PACKAGE(SWIG)
IF(SWIG_FOUND)
    INCLUDE(${SWIG_USE_FILE})

    FIND_PACKAGE(PythonLibs)
    FIND_PACKAGE(PythonInterp)
    INCLUDE_DIRECTORIES(${PYTHON_INCLUDE_PATH})

    MESSAGE(STATUS "Python libs: " ${PYTHON_LIBRARIES})    

    EXECUTE_PROCESS(COMMAND
        ${PYTHON_EXECUTABLE} -c "import numpy"
        OUTPUT_VARIABLE Numpy_FOUND
        ERROR_VARIABLE  Numpy_ERROR
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    IF (NOT ${Numpy_ERROR} EQUAL 0)
        MESSAGE(STATUS "numpy not found: " ${Numpy_ERROR})
        RETURN()
    ELSE ()
        EXECUTE_PROCESS(COMMAND
            ${PYTHON_EXECUTABLE} -c "import numpy; print(numpy.get_include())"
            OUTPUT_VARIABLE Numpy_INCLUDE_PATH
            ERROR_VARIABLE  Numpy_INCLUDE_ERROR
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )
        
        EXECUTE_PROCESS(COMMAND
            ${PYTHON_EXECUTABLE} -c "import numpy; print(numpy.__version__)"
            OUTPUT_VARIABLE Numpy_VERSION
            ERROR_VARIABLE  Numpy_VERSION_ERROR
            OUTPUT_STRIP_TRAILING_WHITESPACE
        )

        MESSAGE(STATUS "Found numpy: " ${Numpy_INCLUDE_PATH} " (found version " ${Numpy_VERSION} ")") 
        SET(Numpy_REQUIRED_VERSION "1.9")
        IF(${Numpy_VERSION} VERSION_LESS ${Numpy_REQUIRED_VERSION})
            MESSAGE(STATUS "numpy wrong version: " ${Numpy_VERSION} " < " ${Numpy_REQUIRED_VERSION})
            RETURN()
        ENDIF()
    ENDIF ()

    INCLUDE_DIRECTORIES(${Numpy_INCLUDE_PATH})
    INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})

    SET_SOURCE_FILES_PROPERTIES(FlyDetector.i PROPERTIES CPLUSPLUS ON)
    SET_SOURCE_FILES_PROPERTIES(FlyDetector.i PROPERTIES SWIG_FLAGS  "-wall;-c++")
    add_compile_options(-Wall -std=c++11)
        
    SWIG_ADD_MODULE(FlyDetector python FlyDetector.i "cv_converter.h")
    SWIG_LINK_LIBRARIES(FlyDetector ${PYTHON_LIBRARIES} flyDetector ${OpenCV_LIBS} Qt5::Core)
ENDIF(SWIG_FOUND)
