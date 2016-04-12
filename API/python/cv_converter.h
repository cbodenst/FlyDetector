#define NPY_NO_DEPRECATED_API NPY_1_7_API_VERSION

#include <opencv2/opencv.hpp> 
#include <Python.h>
#include <numpy/arrayobject.h>
#include <numpy/ndarraytypes.h>

cv::Mat* toCV(PyArrayObject* np_array, bool copy=true)
{
    int channels = 1;  
    int ndims = PyArray_NDIM(np_array);
    if(ndims > 2)
    {
	channels = (int)PyArray_DIMS(np_array)[2];
	ndims = 2;
    } 
    int dims[ndims];
    for (int i = 0; i< ndims;i++)
	dims[i] = (int)PyArray_DIMS(np_array)[i];
    int type;
    switch(PyArray_TYPE(np_array))
    {   
        case NPY_INT8: type = CV_8SC(channels); break;
        case NPY_INT16: type = CV_16SC(channels); break;
        case NPY_INT32: type = CV_32SC(channels); break;
        case NPY_UINT8: type = CV_8UC(channels); break;
        case NPY_UINT16: type = CV_16UC(channels); break;
        case NPY_FLOAT32: type = CV_32FC(channels); break;
        case NPY_FLOAT64: type = CV_64FC(channels); break;
        default : 
        {
            PyErr_SetString(PyExc_ValueError,"The provided type is not supported");
            return NULL;
        }
    }   
    
    char* data = (char*)PyArray_DATA(np_array);
    cv::Mat* ret;
    if ((copy))
    {
	int nbytes = PyArray_NBYTES(np_array);
        ret = new cv::Mat(ndims, dims, type);
        std::copy(data, data + nbytes, ret->data);
    }
    else
    {
        ret = new cv::Mat(ndims, dims, type, data);
    };
    return ret;
}

PyObject* toNumpy(cv::Mat* cv_mat, bool copy = true)
{  
    int ndims = cv_mat->dims;
    int channels = cv_mat->channels();
    if(channels > 1)
	ndims++;
    npy_intp dims[ndims];
    for (int i = 0; i < ndims; i++) 
        dims[i] = cv_mat->size[i];
    if(channels > 1)
    	dims[ndims]=channels;
    
    int type;
    switch(cv_mat->type())
    {   
        case CV_8S : type = NPY_INT8; break;
        case CV_16S : type = NPY_INT16; break;
        case CV_32S: type = NPY_INT32; break;
        case CV_8U : type = NPY_UINT8; break;
        case CV_16U : type = NPY_UINT16; break;
        case CV_32F : type = NPY_FLOAT32; break;
        case CV_64F : type = NPY_FLOAT64; break;
        default : 
        {
            PyErr_SetString(PyExc_ValueError,"The provided type is not supported");
            return NULL;
        }
    }
    void* data;
    bool owner = true;
    if(!copy)
    { 
        data = cv_mat->data;
        owner = false;
    }
    else
    {
	int nbytes = cv_mat->total()*cv_mat->elemSize();
	uchar* d = cv_mat->data;
        data = new char[nbytes];
        std::copy(d, d + nbytes, (uchar*)data);
    }
    PyObject* array = PyArray_SimpleNewFromData(ndims, dims, type, data);
    if(owner)
        PyArray_ENABLEFLAGS((PyArrayObject*) array, NPY_ARRAY_OWNDATA);
    return array;
}
