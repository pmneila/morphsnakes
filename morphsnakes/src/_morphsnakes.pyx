
# distutils: language = c++
# cython: embedsignature = True

# from libcpp.vector cimport vector
import numpy as np

# Define PY_ARRAY_UNIQUE_SYMBOL
cdef extern from "pyarray_symbol.h":
    pass

cimport numpy as np

np.import_array()

cdef extern from "morphsnakes.h" namespace "morphsnakes":
    cdef cppclass NDImage_double_2 "morphsnakes::NDImage<double, 2>":
        pass
    
    cdef cppclass NDImage_double_3 "morphsnakes::NDImage<double, 3>":
        pass
    
    cdef cppclass Embedding_2 "morphsnakes::Embedding<2>":
        pass
    
    cdef cppclass Embedding_3 "morphsnakes::Embedding<3>":
        pass
    
    cdef cppclass ArrayOfNDImage_double_2 "std::array<morphsnakes::NDImage<double, 2>, 2>":
        pass
    
    cdef cppclass ArrayOfNDImage_double_3 "std::array<morphsnakes::NDImage<double, 3>, 3>":
        pass
    
    cdef cppclass NarrowBand_2 "morphsnakes::NarrowBand<2>":
        NarrowBand_2(Embedding_2)
    
    cdef cppclass NarrowBand_3 "morphsnakes::NarrowBand<3>":
        NarrowBand_3(Embedding_3)
    
    cdef cppclass c_MorphACWE_double_2 "morphsnakes::MorphACWE<double, 2>":
        c_MorphACWE_double_2(Embedding_2, NDImage_double_2, int, double, double)
        void step()
    
    cdef cppclass c_MorphACWE_double_3 "morphsnakes::MorphACWE<double, 3>":
        c_MorphACWE_double_3(Embedding_3, NDImage_double_3, int, double, double)
        void step()
    
    cdef cppclass c_MorphGAC_double_2 "morphsnakes::MorphGAC<double, 2>":
        c_MorphGAC_double_2(Embedding_2, NDImage_double_2, ArrayOfNDImage_double_2, int, double, double)
        void step()

    cdef cppclass c_MorphGAC_double_3 "morphsnakes::MorphGAC<double, 3>":
        c_MorphGAC_double_3(Embedding_3, NDImage_double_3, ArrayOfNDImage_double_3, int, double, double)
        void step()
    
    void curv_2 "morphsnakes::curv<2>"(int, NarrowBand_2)
    void curv_3 "morphsnakes::curv<3>"(int, NarrowBand_3)

cdef extern from "numpy_wrapper.h":
    Embedding_2 c_numpy2embedding_2 "numpy2ndimage<unsigned char, 2>"(np.ndarray)
    Embedding_3 c_numpy2embedding_3 "numpy2ndimage<unsigned char, 3>"(np.ndarray)
    NDImage_double_2 c_numpy2ndimage_double_2 "numpy2ndimage<double, 2>"(np.ndarray) # except +
    NDImage_double_3 c_numpy2ndimage_double_3 "numpy2ndimage<double, 3>"(np.ndarray) # except +
    ArrayOfNDImage_double_2 c_pylist2stdrray_double_2 "pylist2stdrray<double, 2>"(list) # except +
    ArrayOfNDImage_double_3 c_pylist2stdrray_double_3 "pylist2stdrray<double, 3>"(list) # except +
    NarrowBand_2 dereferenceNB_2 "dereferenceNB<2>"(NarrowBand_2*)
    NarrowBand_3 dereferenceNB_3 "dereferenceNB<3>"(NarrowBand_3*)

cdef class MorphACWE2d:
    cdef c_MorphACWE_double_2* thisptr
    def __cinit__(self, np.ndarray[np.uint8_t, ndim=2] embedding,
                np.ndarray[np.double_t, ndim=2] image,
                int smoothing, double lambda1, double lambda2):
        self.thisptr = new c_MorphACWE_double_2(
                                c_numpy2embedding_2(embedding),
                                c_numpy2ndimage_double_2(image),
                                smoothing, lambda1, lambda2)
    def __dealloc__(self):
        del self.thisptr
    def step(self):
        self.thisptr.step()

cdef class MorphACWE3d:
    cdef c_MorphACWE_double_3* thisptr
    def __cinit__(self, np.ndarray[np.uint8_t, ndim=3] embedding,
                np.ndarray[np.double_t, ndim=3] image,
                int smoothing, double lambda1, double lambda2):
        self.thisptr = new c_MorphACWE_double_3(
                                c_numpy2embedding_3(embedding),
                                c_numpy2ndimage_double_3(image),
                                smoothing, lambda1, lambda2)
    def __dealloc__(self):
        del self.thisptr
    def step(self):
        self.thisptr.step()

cdef class MorphGAC2d:
    cdef c_MorphGAC_double_2* thisptr
    def __cinit__(self, np.ndarray[np.uint8_t, ndim=2] embedding,
                np.ndarray[np.double_t, ndim=2] gimg,
                list gradients,
                int smoothing, double threshold, double balloon):
        self.thisptr = new c_MorphGAC_double_2(
                                c_numpy2embedding_2(embedding),
                                c_numpy2ndimage_double_2(gimg),
                                c_pylist2stdrray_double_2(gradients),
                                smoothing, threshold, balloon)
    def __dealloc__(self):
        del self.thisptr
    def step(self):
        self.thisptr.step()

cdef class MorphGAC3d:
    cdef c_MorphGAC_double_3* thisptr
    def __cinit__(self, np.ndarray[np.uint8_t, ndim=3] embedding,
                np.ndarray[np.double_t, ndim=3] gimg,
                list gradients,
                int smoothing, double threshold, double balloon):
        self.thisptr = new c_MorphGAC_double_3(
                                c_numpy2embedding_3(embedding),
                                c_numpy2ndimage_double_3(gimg),
                                c_pylist2stdrray_double_3(gradients),
                                smoothing, threshold, balloon)
    def __dealloc__(self):
        del self.thisptr
    def step(self):
        self.thisptr.step()

cdef class NarrowBand2d:
    cdef NarrowBand_2* thisptr
    def __cinit__(self, np.ndarray[np.uint8_t, ndim=2] embedding):
        self.thisptr = new NarrowBand_2(c_numpy2embedding_2(embedding))
    def __dealloc__(self):
        del self.thisptr

cdef class NarrowBand3d:
    cdef NarrowBand_3* thisptr
    def __cinit__(self, np.ndarray[np.uint8_t, ndim=3] embedding):
        self.thisptr = new NarrowBand_3(c_numpy2embedding_3(embedding))
    def __dealloc__(self):
        del self.thisptr

cdef curv_op2d(int inf_sup, NarrowBand2d narrowband):
    # TODO: This is HORRIBLE. Cython does not allow dereference narrowband.thisptr
    # ("cannot convert to Python object"????)
    curv_2(inf_sup, dereferenceNB_2(narrowband.thisptr))

cdef curv_op3d(int inf_sup, NarrowBand3d narrowband):
    # TODO: This is HORRIBLE. Cython does not allow dereference narrowband.thisptr
    # ("cannot convert to Python object"????)
    curv_3(inf_sup, dereferenceNB_3(narrowband.thisptr))
