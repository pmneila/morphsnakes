"""

Copyright (C) 2014-2017 Jiri Borovec <jiri.borovec@fel.cvut.cz>
"""

import os
import logging

import numpy as np
import tqdm

import morphsnakes

DEFAULT_SNAKE = morphsnakes.MorphACWE
DEFAULT_PARAMS = dict(smoothing=1, lambda1=1, lambda2=1)


class MultiMorphSnakes(object):
    """

    >>> np.random.seed(0)
    >>> img = np.random.random((15, 15))
    >>> mask = np.zeros(img.shape, dtype=int)
    >>> mask[2:9, 3:7] = 1
    >>> mask[10:14, 9:12] = 2
    >>> ms = MultiMorphSnakes(img, mask)
    >>> ms.levelset
    array([[0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 2, 2, 2, 0, 0, 0],
           [0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0]])
    >>> ms.run(1)
    >>> np.array_equal(mask, ms.levelset)
    False
    """

    def __init__(self, image, init_mask, list_snakes=None, list_params=None):
        self._image = np.array(image)
        self._u = init_mask

        inits = np.unique(init_mask)[1:]

        if list_snakes is None:
            list_snakes = [DEFAULT_SNAKE for lb in inits]
        elif not isinstance(list_snakes, list):
            list_snakes = [list_snakes for lb in inits]
        elif len(list_snakes) == 1 and len(inits) > 1:
            list_snakes = [list_snakes[0] for lb in inits]

        if list_params is None:
            list_params = [DEFAULT_PARAMS for lb in inits]
        elif not isinstance(list_params, list):
            list_params = [list_params for lb in inits]
        elif len(list_params) == 1 and len(inits) > 1:
            list_params = [list_params[0] for lb in inits]

        self._snakes = []
        for lb, snake, params in zip(inits, list_snakes, list_params):
            levelset = (init_mask == lb)
            snake_inst = snake(levelset, self._image, **params)
            self._snakes.append(snake_inst)

    def step(self):
        """ Perform a single step of the morphological Chan-Vese evolution."""
        lvs = []
        for snake in self._snakes:
            snake.step()
            lvs.append(snake.levelset)

        # collision
        mask = np.sum(np.array(lvs), axis=0) > 1
        for snake in self._snakes:
            snake.levelset[mask] = 0

    def get_levelsets(self):
        self._u = np.zeros(self._image.shape, dtype=int)
        for i, snake in enumerate(self._snakes):
            self._u[snake.levelset == 1] = i + 1
        return self._u

    levelset = property(get_levelsets, lambda self: self._u,
                        doc="The level set embedding function (u).")

    def run(self, nb_iters):
        """ Run several nb_iters of the morphological Chan-Vese method."""
        tqdm_bar = tqdm.tqdm(total=nb_iters)
        for i in range(nb_iters):
            self.step()
            tqdm_bar.update()



# levelset = circle_levelset(img.shape, (80, 170), 25)
# macwe = morphsnakes.MorphACWE(levelset, img, smoothing=3,
#                               lambda1=1, lambda2=1)
#
# for i in range(num_iters):
#     # Evolve.
#     msnake.step()
#
#     # Update figure.
#     del ax1.collections[0]
#     ax1.contour(msnake.levelset, [0.5], colors='r')
#     ax_u.set_data(msnake.levelset)
#     fig.canvas.draw()

