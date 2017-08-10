import os, sys
import logging

import matplotlib
matplotlib.use('Agg') # suppress the visualisations

import numpy as np
from scipy.ndimage import imread
from matplotlib import pyplot as plt

sys.path += [os.path.abspath('.'), os.path.abspath('..')]  # Add path to root
import morphsnakes


def find_path(path):
    for i in range(3):
        if not os.path.exists(path):
            path = os.path.join('..', path)
    return path


PATH_IMAGES = find_path('images')
assert os.path.exists(PATH_IMAGES)
PATH_OUTPUT = find_path('output')
assert os.path.exists(PATH_OUTPUT)
PATH_IMG_NODULE = os.path.join(PATH_IMAGES, 'mama07ORI.bmp')
PATH_IMG_STARFISH = os.path.join(PATH_IMAGES, 'seastar2.png')
PATH_IMG_LAKES = os.path.join(PATH_IMAGES, 'lakes3.jpg')
PATH_ARRAY_CONFOCAL = os.path.join(PATH_IMAGES, 'confocal.npy')


def rgb2gray(img):
    """Convert a RGB image to gray scale."""
    return 0.2989 * img[:, :, 0] + 0.587 * img[:, :, 1] + 0.114 * img[:, :, 2]


def circle_levelset(shape, center, sqradius):
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
    levelset = circle_levelset(img.shape, (100, 126), 20)
    mgac = morphsnakes.MorphGAC(levelset, gI, smoothing=1, threshold=0.31,
                                balloon=1)
    
    # Visual evolution.
    fig = plt.figure()
    morphsnakes.evolve_visual(mgac, fig, num_iters=45, background=img)
    fig.savefig(os.path.join(PATH_OUTPUT, 'figure_nodule.png'))


def test_starfish():
    logging.info('running: test_starfish...')
    # Load the image.
    imgcolor = imread(PATH_IMG_STARFISH) / 255.0
    img = rgb2gray(imgcolor)
    
    # g(I)
    gI = morphsnakes.gborders(img, alpha=1000, sigma=2)
    
    # Morphological GAC. Initialization of the level-set.
    levelset = circle_levelset(img.shape, (163, 137), 135)
    mgac = morphsnakes.MorphGAC(levelset, gI, smoothing=2, threshold=0.3,
                                balloon=-1)
    
    # Visual evolution.
    fig = plt.figure()
    morphsnakes.evolve_visual(mgac, fig, num_iters=100, background=imgcolor)
    fig.savefig(os.path.join(PATH_OUTPUT, 'figure_starfish.png'))


def test_lakes():
    logging.info('running: test_lakes...')
    # Load the image.
    imgcolor = imread(PATH_IMG_LAKES)/255.0
    img = rgb2gray(imgcolor)
    
    # MorphACWE does not need g(I)
    
    # Morphological ACWE. Initialization of the level-set.
    levelset = circle_levelset(img.shape, (80, 170), 25)
    macwe = morphsnakes.MorphACWE(levelset, img, smoothing=3,
                                  lambda1=1, lambda2=1)
    
    # Visual evolution.
    fig = plt.figure()
    morphsnakes.evolve_visual(macwe, fig, num_iters=200, background=imgcolor)
    fig.savefig(os.path.join(PATH_OUTPUT, 'figure_lakes.png'))


def sample_confocal3d():
    logging.info('running: sample_confocal3d...')
    # Load the image.
    img = np.load(PATH_ARRAY_CONFOCAL)
    
    # Morphological ACWE. Initialization of the level-set.
    levelset = circle_levelset(img.shape, (30, 50, 80), 25)
    macwe = morphsnakes.MorphACWE(levelset, img, smoothing=1,
                                  lambda1=1, lambda2=2)
    
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
