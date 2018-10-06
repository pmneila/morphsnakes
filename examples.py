import os
import logging

import matplotlib
# in case you are running on machine without display, e.g. server
if os.environ.get('DISPLAY', '') == '':
    logging.warning('No display found. Using non-interactive Agg backend.')
    matplotlib.use('Agg')

import numpy as np
from scipy.misc import imread
from matplotlib import pyplot as plt

import morphsnakes

PATH_IMG_NODULE = 'images/mama07ORI.bmp'
PATH_IMG_STARFISH = 'images/seastar2.png'
PATH_IMG_LAKES = 'images/lakes3.jpg'
PATH_ARRAY_CONFOCAL = 'images/confocal.npy'


def rgb2gray(img):
    """Convert a RGB image to gray scale."""
    return 0.2989 * img[:, :, 0] + 0.587 * img[:, :, 1] + 0.114 * img[:, :, 2]


def circle_levelset(shape, center, sqradius, scalerow=1.0):
    """Build a binary function with a circle as the 0.5-levelset."""
    grid = np.mgrid[list(map(slice, shape))].T - center
    phi = sqradius - np.sqrt(np.sum((grid.T) ** 2, 0))
    u = np.float_(phi > 0)
    return u


def test_nodule():
    logging.info('running: test_nodule...')
    # Load the image.
    img = imread(PATH_IMG_NODULE)[..., 0] / 255.0
    
    # g(I)
    gI = morphsnakes.gborders(img, alpha=1000, sigma=5.48)
    
    # Morphological GAC. Initialization of the level-set.
    mgac = morphsnakes.MorphGAC(gI, smoothing=1, threshold=0.31, balloon=1)
    mgac.levelset = circle_levelset(img.shape, (100, 126), 20)
    
    # Visual evolution.
    fig = plt.figure()
    morphsnakes.evolve_visual(mgac, fig, num_iters=45, background=img)


def test_starfish():
    logging.info('running: test_starfish...')
    # Load the image.
    imgcolor = imread(PATH_IMG_STARFISH) / 255.0
    img = rgb2gray(imgcolor)
    
    # g(I)
    gI = morphsnakes.gborders(img, alpha=1000, sigma=2)
    
    # Morphological GAC. Initialization of the level-set.
    mgac = morphsnakes.MorphGAC(gI, smoothing=2, threshold=0.3, balloon=-1)
    mgac.levelset = circle_levelset(img.shape, (163, 137), 135, scalerow=0.75)
    
    # Visual evolution.
    fig = plt.figure()
    morphsnakes.evolve_visual(mgac, fig, num_iters=100, background=imgcolor)


def test_lakes():
    logging.info('running: test_lakes...')
    # Load the image.
    imgcolor = imread(PATH_IMG_LAKES)/255.0
    img = rgb2gray(imgcolor)
    
    # MorphACWE does not need g(I)
    
    # Morphological ACWE. Initialization of the level-set.
    macwe = morphsnakes.MorphACWE(img, smoothing=3, lambda1=1, lambda2=1)
    macwe.levelset = circle_levelset(img.shape, (80, 170), 25)
    
    # Visual evolution.
    fig = plt.figure()
    morphsnakes.evolve_visual(macwe, fig, num_iters=200, background=imgcolor)


def sample_confocal3d():
    logging.info('running: sample_confocal3d...')
    # Load the image.
    img = np.load(PATH_ARRAY_CONFOCAL)
    
    # Morphological ACWE. Initialization of the level-set.
    macwe = morphsnakes.MorphACWE(img, smoothing=1, lambda1=1, lambda2=2)
    macwe.levelset = circle_levelset(img.shape, (30, 50, 80), 25)
    
    # Visual evolution.
    morphsnakes.evolve_visual3d(macwe, num_iters=150,
                                animate_ui=False, animate_delay=10)


if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG)
    test_nodule()
    test_starfish()
    test_lakes()
    sample_confocal3d()
    plt.show()
