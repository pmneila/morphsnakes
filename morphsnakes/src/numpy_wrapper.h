
#ifndef _NUMPY_WRAPPER_H
#define _NUMPY_WRAPPER_H

#include "morphsnakes/morphsnakes.h"
#include "pyarraymodule.h"

namespace ms = morphsnakes;

template<class T, size_t D>
ms::NDImage<T, D> numpy2ndimage(PyArrayObject* arr)
{
    std::cout << "TODO TODO TODO TODO TODO" << std::endl;
}

// Python list to std::array
template<class T, size_t D>
std::array<ms::NDImage<T, D>, D>& pylist2stdrray(PyObject* lst)
{
    std::cout << "TODO TODO TODO TODO TODO" << std::endl;
}

template<size_t D>
ms::NarrowBand<D>& dereferenceNB(ms::NarrowBand<D>* nb) {return *nb;}

#endif // _NUMPY_WRAPPER_H
