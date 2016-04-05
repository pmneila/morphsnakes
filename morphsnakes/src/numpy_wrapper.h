
#ifndef _NUMPY_WRAPPER_H
#define _NUMPY_WRAPPER_H

#include "morphsnakes/morphsnakes.h"
#include "pyarraymodule.h"

#include <stdexcept>

namespace ms = morphsnakes;

template<class T, size_t D>
ms::NDImage<T, D> numpy2ndimage(PyObject* arr)
{
    if(!PyArray_Check(arr))
        throw std::domain_error("numpy2ndimage requires a NumPy array as input");

    return numpy2ndimage<T, D>(reinterpret_cast<PyArrayObject*>(arr));
}

template<class T, size_t D>
ms::NDImage<T, D> numpy2ndimage(PyArrayObject* arr)
{
    if(numpy_typemap<T>::type != PyArray_TYPE(arr))
        throw std::domain_error("invalid array datatype in numpy2ndimage");

    if(PyArray_NDIM(arr) != D)
        throw std::domain_error("invalid number of dimensions of input array in numpy2ndimage");

    npy_intp* arr_shape = PyArray_SHAPE(arr);
    npy_intp* arr_stride = PyArray_STRIDES(arr);

    ms::Shape<D> shape;
    ms::Stride<D> stride;
    for(size_t i = 0; i < D; ++i)
    {
        shape[i] = arr_shape[i];
        stride[i] = arr_stride[i];
    }

    T* data = reinterpret_cast<T*>(PyArray_DATA(arr));

    return ms::NDImage<T, D>(data, shape, stride);
}

// Python list to std::array
template<class T, size_t D>
typename std::enable_if<D == 2, std::array<ms::NDImage<T, D>, D> >::type
pylist2stdrray(PyObject* lst)
{
    if(!PyList_Check(lst))
        throw std::domain_error("pylist2stdarray requires a list as input");

    if(PyList_Size(lst) != D)
        throw std::domain_error("pylist2stdarray requires a list with D elements");

    return std::array<ms::NDImage<T, D>, D>{
        numpy2ndimage<T, D>(PyList_GET_ITEM(lst, 0)),
        numpy2ndimage<T, D>(PyList_GET_ITEM(lst, 1))
    };
}

template<class T, size_t D>
typename std::enable_if<D == 3, std::array<ms::NDImage<T, D>, D> >::type
pylist2stdrray(PyObject* lst)
{
    if(!PyList_Check(lst))
        throw std::domain_error("pylist2stdarray requires a list as input");

    if(PyList_Size(lst) != D)
        throw std::domain_error("pylist2stdarray requires a list with D elements");

    return std::array<ms::NDImage<T, D>, D>{
        numpy2ndimage<T, D>(PyList_GET_ITEM(lst, 0)),
        numpy2ndimage<T, D>(PyList_GET_ITEM(lst, 1)),
        numpy2ndimage<T, D>(PyList_GET_ITEM(lst, 2))
    };
}

template<size_t D>
ms::NarrowBand<D>& dereferenceNB(ms::NarrowBand<D>* nb) {return *nb;}

#endif // _NUMPY_WRAPPER_H
