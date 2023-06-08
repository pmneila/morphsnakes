# -*- encoding: utf-8 -*-

try:
    from setuptools import setup
except ImportError:
    from distutils.core import setup


def get_version():
    """ Avoid importing package before dependencies are installed. """
    values = {}
    with open("morphsnakes.py", "r") as f:
        for line in f.readlines():
            if "__version__" in line:
                exec(line, {}, values)
                break

    version = values.get("__version__", (0, 1, 0))
    return ".".join(map(str, version))


setup(
    name="morphsnakes",
    version=get_version(),
    description="Morphological Snakes",
    author="Pablo Márquez Neila",
    author_email="pablo.marquez@artorg.unibe.ch",
    url="https://github.com/pmneila/morphsnakes",
    license="BSD 3-clause",
    # packages=["morphsnakes"],
    py_modules=["morphsnakes"],
    install_requires=['numpy', 'scipy'],
    long_description="""
    The Morphological Snakes are a family of methods for image-guided 
    evolution of curves and surfaces represented as a level-set of an embedding 
    function. They have application in several Computer Vision areas, 
    such as image segmentation and tracking.
    """,
    classifiers=[
        "Development Status :: 4 - Beta",
        "Environment :: Console",
        "Intended Audience :: Developers",
        "Intended Audience :: Science/Research",
        "License :: OSI Approved :: BSD License",
        "Natural Language :: English",
        "Operating System :: OS Independent",
        "Programming Language :: Python",
        "Topic :: Multimedia :: Graphics :: 3D Modeling",
        "Topic :: Scientific/Engineering :: Image Recognition",
    ],
)
