import os, sys
import logging

import matplotlib
if os.environ.get('DISPLAY','') == '':
    logging.warning('No display found. Using non-interactive Agg backend')
    matplotlib.use('Agg')

import numpy as np
from scipy.ndimage import imread
from matplotlib import pyplot as plt

sys.path += [os.path.abspath('.'), os.path.abspath('..')]  # Add path to root
from morphsnakes.cmorphsnakes import MorphGAC, MorphACWE
from morphsnakes.morphsnakes import gborders, evolve_visual, evolve_visual3d
import morphsnakes.multi_snakes as multi_ms
from tests.test_python import rgb2gray, circle_levelset, find_path


PATH_IMAGES = find_path('images')
assert os.path.exists(PATH_IMAGES)
PATH_OUTPUT = find_path('output')
PATH_IMG_NODULE = os.path.join(PATH_IMAGES, 'mama07ORI.bmp')
PATH_IMG_STARFISH = os.path.join(PATH_IMAGES, 'seastar2.png')
PATH_IMG_LAKES = os.path.join(PATH_IMAGES, 'lakes3.jpg')
PATH_ARRAY_CONFOCAL = os.path.join(PATH_IMAGES, 'confocal.npy')


def test_nodule():
    logging.info('running: test_nodule...')
    # Load the image.
    img = imread(PATH_IMG_NODULE)[..., 0] / 255.0
    
    # g(I)
    gI = gborders(img, alpha=1000, sigma=5.48)
    
    # Morphological GAC. Initialization of the level-set.
    levelset = circle_levelset(img.shape, (100, 126), 20)
    mgac = MorphGAC(levelset, gI, smoothing=1, threshold=0.31, balloon=1)
    
    # Visual evolution.
    fig = plt.figure()
    evolve_visual(mgac, fig, num_iters=45, background=img)
    assert os.path.exists(PATH_OUTPUT)
    fig.savefig(os.path.join(PATH_OUTPUT, 'cmorphsnakes_nodule.png'))
    plt.close(fig)


def test_starfish():
    logging.info('running: test_starfish...')
    # Load the image.
    imgcolor = imread(PATH_IMG_STARFISH) / 255.0
    img = rgb2gray(imgcolor)
    
    # g(I)
    gI = gborders(img, alpha=1000, sigma=2)
    
    # Morphological GAC. Initialization of the level-set.
    levelset = circle_levelset(img.shape, (163, 137), 135)
    mgac = MorphGAC(levelset, gI, smoothing=2, threshold=0.3, balloon=-1)
    
    # Visual evolution.
    fig = plt.figure()
    evolve_visual(mgac, fig, num_iters=100, background=imgcolor)
    assert os.path.exists(PATH_OUTPUT)
    fig.savefig(os.path.join(PATH_OUTPUT, 'cmorphsnakes_starfish.png'))
    plt.close(fig)


def test_lakes():
    logging.info('running: test_lakes...')
    # Load the image.
    imgcolor = imread(PATH_IMG_LAKES)/255.0
    img = rgb2gray(imgcolor)
    
    # MorphACWE does not need g(I)
    
    # Morphological ACWE. Initialization of the level-set.
    levelset = circle_levelset(img.shape, (80, 170), 25)
    macwe = MorphACWE(levelset, img, smoothing=3, lambda1=1, lambda2=1)
    
    # Visual evolution.
    fig = plt.figure()
    evolve_visual(macwe, fig, num_iters=200, background=imgcolor)
    assert os.path.exists(PATH_OUTPUT)
    fig.savefig(os.path.join(PATH_OUTPUT, 'cmorphsnakes_lakes.png'))
    plt.close(fig)


def test_multi_lakes():
    logging.info('running: test_multi_lakes...')
    # Load the image.
    imgcolor = imread(PATH_IMG_LAKES)/255.0
    img = rgb2gray(imgcolor)

    # MorphACWE does not need g(I)

    # Morphological ACWE. Initialization of the level-set.
    mask = np.zeros(img.shape, dtype=int)
    mask[circle_levelset(img.shape, (50, 100), 25) == 1] = 1
    mask[circle_levelset(img.shape, (150, 150), 25) == 1] = 2
    mask[circle_levelset(img.shape, (120, 220), 15) == 1] = 3
    mask[circle_levelset(img.shape, (80, 250), 15) == 1] = 4

    ms = multi_ms.MultiMorphSnakes(img, mask, MorphACWE,
                                   dict(smoothing=3, lambda1=1, lambda2=1))

    # Visual evolution.
    fig = plt.figure()
    evolve_visual(ms, fig, num_iters=200, background=imgcolor)
    assert os.path.exists(PATH_OUTPUT)
    fig.savefig(os.path.join(PATH_OUTPUT, 'cmorphsnakes_multi_lakes.png'))
    plt.close(fig)


def sample_confocal3d():
    logging.info('running: sample_confocal3d...')
    # Load the image.
    img = np.load(PATH_ARRAY_CONFOCAL)
    
    # Morphological ACWE. Initialization of the level-set.
    levelset = circle_levelset(img.shape, (30, 50, 80), 25)
    macwe = MorphACWE(levelset, img, smoothing=1, lambda1=1, lambda2=2)
    
    # Visual evolution.
    evolve_visual3d(macwe, num_iters=150, animate_ui=False, animate_delay=10)


if __name__ == '__main__':
    logging.basicConfig(level=logging.DEBUG)
    test_nodule()
    test_starfish()
    test_lakes()
    sample_confocal3d()
    plt.show()
