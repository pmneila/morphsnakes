# -*- encoding: utf-8 -*-

import sys
import os

try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup

from Cython.Build import cythonize

import numpy
from distutils.extension import Extension

extra_compile_args = ['-c', '--std=c++11']

# Fix a problem with OSX
if sys.platform == 'darwin':
    extra_compile_args.append("--stdlib=libc++")
    cur_target = os.environ.get('MACOSX_DEPLOYMENT_TARGET')
    if cur_target is None:
        os.environ['MACOSX_DEPLOYMENT_TARGET'] = "10.7"

numpy_include_dir = numpy.get_include()

morphsnakes_module = Extension(
    "morphsnakes._morphsnakes",
    [
        "morphsnakes/src/_morphsnakes.pyx",
    ],
    language="c++",
    extra_compile_args=extra_compile_args,
    include_dirs=[numpy_include_dir, os.path.join('morphsnakes', 'include')]
)

setup(name="MorphSnakes",
    version="1.0.0",
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
