find_path(GPHOTO2_INCLUDE_DIRS
    NAMES
	gphoto2.h
    PATHS
      /usr/include
      /usr/local/include
      /opt/local/include
	PATH_SUFFIXES
	  gphoto2
  )

  find_library(GPHOTO2_LIBS
    NAMES
      gphoto2
    PATHS
      /usr/lib
      /usr/local/lib
      /opt/local/lib
  )
  if (GPHOTO2_INCLUDE_DIRS AND GPHOTO2_LIBS)
     set(GPHOTO2_FOUND TRUE)
  endif (GPHOTO2_INCLUDE_DIRS AND GPHOTO2_LIBS)

  if (GPHOTO2_FOUND)
	message(STATUS "Found gphoto2:")
        message(STATUS " - Includes: ${GPHOTO2_INCLUDE_DIRS}")
	message(STATUS " - Libraries: ${GPHOTO2_LIBS}")
  else()
	message(STATUS "gphoto2 not gound")
  endif(GPHOTO2_FOUND)
