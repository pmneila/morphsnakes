# -*- encoding: utf-8 -*-

try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup


setup(
    name="MorphSnakes",
    version="0.1",
    description="Morphological Snakes",
    author="Pablo Márquez Neila",
    author_email="pablo.marquezneila@epfl.ch",
    url="https://github.com/pmneila/morphsnakes",
    license="BSD 3-clause",
    # packages=["morphsnakes"],
    py_modules=["morphsnakes"],
    requires=['numpy', 'scipy'],
    long_description="""
    The Morphological Snakes are a family of related methods for image-guided 
    evolution of curves and surfaces represented as a level-set of an embedding 
    function. They have application in several computer vision areas, 
    such as tracking and image segmentation.
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
)
