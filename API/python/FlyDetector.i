%module FlyDetector

%{
    #define SWIG_FILE_WITH_INIT
    #include "flycounter.h"
    #include "vials.h"
%}

/* Convert C++ exceptions to Python exception */ 
%exception { 
   try { 
      $action 
   } catch (std::exception &e) { 
      PyErr_SetString(PyExc_Exception, const_cast<char*>(e.what())); 
      return NULL; 
   } 
} 

/* Wrap std::string */
%include std_string.i

/* Wrap std::vector */
%include "std_vector.i"

/* Wrap OpenCV */
%include opencv.i

/*Wrap FlyCounter*/
%include "flycounter.h"
%include "vials.h"
%template(Vials) std::vector<Vial>;

/* Remove unwanted *_swigregister globals */
%pythoncode %{
def __cleanup_namespace():
    for i in globals().copy():
        if i.endswith("_swigregister"):
            del globals()[i]
__cleanup_namespace()
del __cleanup_namespace
%}
