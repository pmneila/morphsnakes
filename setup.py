# -*- encoding: utf-8 -*-

try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup

from Cython.Build import cythonize

import numpy
from distutils.extension import Extension

# Get the version number.
numpy_include_dir = numpy.get_include()

morphsnakes_module = Extension(
    "morphsnakes._morphsnakes",
    [
        "morphsnakes/src/_morphsnakes.pyx",
    ],
    language="c++",
    extra_compile_args=['-std=c++11'],
    include_dirs=[numpy_include_dir]
)

setup(name="MorphSnakes",
    version="0.0.4",
    description="Morphological Snakes",
    author="Pablo Márquez Neila",
    author_email="pablo.marquezneila@epfl.ch",
    url="https://github.com/pmneila/morphsnakes",
    license="BSD 3-clause",
    long_description="""
    Morphological Snakes
    """,
    classifiers=[
        "Development Status :: 4 - Beta",
        "Environment :: Console",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: BSD License",
        "Natural Language :: English",
        "Operating System :: OS Independent",
        "Programming Language :: C++",
        "Programming Language :: Python",
        "Topic :: Multimedia :: Graphics :: 3D Modeling",
        "Topic :: Scientific/Engineering :: Image Recognition",
    ],
    packages=["morphsnakes"],
    ext_modules=cythonize(morphsnakes_module),
    requires=['numpy', 'Cython'],
    setup_requires=['numpy', 'Cython']
)
