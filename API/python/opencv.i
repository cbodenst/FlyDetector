%{
#include <cv_converter.h>
%} 
%init %{
import_array();
%}

%typemap(typecheck, precedence=SWIG_TYPECHECK_FLOAT_ARRAY)
( const cv::Mat & ),
(       cv::Mat & )
{
   $1 = PyArray_Check($input) ? 1 : 0;
}

%typemap(in)
( const cv::Mat & ),
(       cv::Mat & ) 
{
    PyArrayObject *temp=NULL;
    if (PyArray_Check($input))
        temp = (PyArrayObject*)$input; 
    else    
    {
        PyErr_SetString(PyExc_ValueError,"Input object is not an array");
        return NULL;
    }
    $1=toCV(temp, false);
}

%typemap(freearg)
( const cv::Mat & ),
(       cv::Mat & ) 
{
   delete $1;
}

%typemap(out)
( const cv::Mat & ),
(       cv::Mat & )
{
    $result = toNumpy($1, true);
}

%typemap(out)
(cv::Mat)
{
    $result = toNumpy(&$1, true);
}


