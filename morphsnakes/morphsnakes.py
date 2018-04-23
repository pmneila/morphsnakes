# -*- coding: utf-8 -*-

"""
morphsnakes
===========

This is a Python implementation of the algorithms introduced in the paper

  Márquez-Neila, P., Baumela, L., Álvarez, L., "A morphological approach
  to curvature-based evolution of curves and surfaces". IEEE Transactions
  on Pattern Analysis and Machine Intelligence (PAMI), 2013.

This implementation is intended to be as brief, understandable and self-contained
as possible. It does not include any enhancement to make it fast or efficient.

Any practical implementation of this algorithm should work only over the
neighbor pixels of the 0.5-levelset, not over all the embedding function,
and perhaps should feature multi-threading or GPU capabilities.

The classes MorphGAC and MorphACWE provide most of the functionality of this
module. They implement the Morphological Geodesic Active Contours and the
Morphological Active Contours without Edges, respectively. See the
aforementioned paper for full details.

See test.py for examples of usage.
"""
__author__ = "P. Márquez Neila <p.mneila@upm.es>"

import logging
from itertools import cycle

import tqdm
import numpy as np
from matplotlib import pyplot as plt
from scipy.ndimage import binary_dilation, binary_erosion
from scipy.ndimage import gaussian_filter, gaussian_gradient_magnitude


class FCycle(object):
    
    def __init__(self, iterable):
        """Call functions from the iterable each time it is called."""
        self.funcs = cycle(iterable)
    
    def __call__(self, *args, **kwargs):
        f = next(self.funcs)
        return f(*args, **kwargs)
    

# operator_si and operator_is operators for 2D and 3D.
_P2 = [np.eye(3), np.array([[0, 1, 0]] * 3),
       np.flipud(np.eye(3)), np.rot90([[0, 1, 0]] * 3)]
_P3 = [np.zeros((3, 3, 3)) for i in range(9)]

_P3[0][:, :, 1] = 1
_P3[1][:, 1, :] = 1
_P3[2][1, :, :] = 1
_P3[3][:, [0, 1, 2], [0, 1, 2]] = 1
_P3[4][:, [0, 1, 2], [2, 1, 0]] = 1
_P3[5][[0, 1, 2], :, [0, 1, 2]] = 1
_P3[6][[0, 1, 2], :, [2, 1, 0]] = 1
_P3[7][[0, 1, 2], [0, 1, 2], :] = 1
_P3[8][[0, 1, 2], [2, 1, 0], :] = 1

_aux = np.zeros((0))


def operator_si(u):
    """operator_si operator."""
    global _aux
    if np.ndim(u) == 2:
        P = _P2
    elif np.ndim(u) == 3:
        P = _P3
    else:
        raise ValueError("u has an invalid number of dimensions "
                         "(should be 2 or 3)")
    
    if u.shape != _aux.shape[1:]:
        _aux = np.zeros((len(P),) + u.shape)
    
    for _aux_i, P_i in zip(_aux, P):
        _aux_i[:] = binary_erosion(u, P_i)
    
    return _aux.max(0)


def operator_is(u):
    """operator_is operator."""
    global _aux
    if np.ndim(u) == 2:
        P = _P2
    elif np.ndim(u) == 3:
        P = _P3
    else:
        raise ValueError("u has an invalid number of dimensions "
                         "(should be 2 or 3)")
    
    if u.shape != _aux.shape[1:]:
        _aux = np.zeros((len(P),) + u.shape)
    
    for _aux_i, P_i in zip(_aux, P):
        _aux_i[:] = binary_dilation(u, P_i)
    
    return _aux.min(0)


# operator_si_o_is operator.
operator_si_o_is = lambda u: operator_si(operator_is(u))
operator_os_o_si = lambda u: operator_is(operator_si(u))
curvop = FCycle([operator_si_o_is, operator_os_o_si])


# Stopping factors (function g(I) in the paper).
def gborders(img, alpha=1.0, sigma=1.0):
    """Stopping criterion for image borders."""
    # The norm of the gradient.
    gradnorm = gaussian_gradient_magnitude(img, sigma, mode='constant')
    return 1.0/np.sqrt(1.0 + alpha*gradnorm)


def glines(img, sigma=1.0):
    """Stopping criterion for image black lines."""
    return gaussian_filter(img, sigma)


class MorphACWE(object):
    """Morphological ACWE based on the Chan-Vese energy functional."""
    
    def __init__(self, levelset, data, smoothing=1, lambda1=1, lambda2=1):
        """Create a Morphological ACWE solver.
        
        Parameters
        ----------
        levelset : ndarray
            The levelset function.
        data : ndarray
            The image data.
        smoothing : scalar
            The number of repetitions of the smoothing step (the
            curv operator) in each iteration. In other terms,
            this is the strength of the smoothing. This is the
            parameter µ.
        lambda1, lambda2 : scalars
            Relative importance of the inside pixels (lambda1)
            against the outside pixels (lambda2).
        """
        self.levelset = levelset
        self.smoothing = smoothing
        self.lambda1 = lambda1
        self.lambda2 = lambda2
        
        self.data = data
    
    def set_levelset(self, u):
        self._u = np.double(u)
        self._u[u>0] = 1
        self._u[u<=0] = 0
    
    levelset = property(lambda self: self._u,
                        set_levelset,
                        doc="The level set embedding function (u).")
    
    def step(self):
        """ Perform a single step of the morphological Chan-Vese evolution."""
        # Assign attributes to local variables for convenience.
        u = self._u
        
        if u is None:
            raise ValueError("the levelset function is not set "
                             "(use set_levelset)")
        
        data = self.data
        
        # Determine c0 and c1.
        inside = u > 0
        outside = u <= 0
        c0 = data[outside].sum() / float(outside.sum())
        c1 = data[inside].sum() / float(inside.sum())
        
        # Image attachment.
        dres = np.array(np.gradient(u))
        abs_dres = np.abs(dres).sum(0)
        #aux = abs_dres * (c0 - c1) * (c0 + c1 - 2*data)
        aux = abs_dres * (self.lambda1*(data - c1) ** 2 -
                          self.lambda2*(data - c0) ** 2)
        
        res = np.copy(u)
        res[aux < 0] = 1
        res[aux > 0] = 0
        
        # Smoothing.
        for i in range(self.smoothing):
            res = curvop(res)
        
        self._u = res
    
    def run(self, nb_iters):
        """Run several nb_iters of the morphological Chan-Vese method."""
        last_segm = np.array(self.levelset)
        for i in range(nb_iters):
            self.step()
            if np.array_equal(last_segm, self.levelset):
                logging.info('no change after %i iterations', i)
                break
            last_segm = np.array(self.levelset)
    

class MorphGAC(object):
    """Morphological GAC based on the Geodesic Active Contours."""
    
    def __init__(self, levelset, data, smoothing=1, threshold=0, balloon=0):
        """Create a Morphological GAC solver.
        
        Parameters
        ----------
        levelset : ndarray
            The levelset function.
        data : array-like
            The stopping criterion g(I). See functions gborders and glines.
        smoothing : scalar
            The number of repetitions of the smoothing step in each
            iteration. This is the parameter µ.
        threshold : scalar
            The threshold that determines which areas are affected
            by the morphological balloon. This is the parameter θ.
        balloon : scalar
            The strength of the morphological balloon. This is the parameter ν.
        """
        self.levelset = levelset
        self._v = balloon
        self._theta = threshold
        self.smoothing = smoothing
        
        self.set_data(data)
    
    def set_levelset(self, u):
        self._u = np.double(u)
        self._u[u>0] = 1
        self._u[u<=0] = 0
    
    def set_balloon(self, v):
        self._v = v
        self._update_mask()
    
    def set_threshold(self, theta):
        self._theta = theta
        self._update_mask()
    
    def set_data(self, data):
        self._data = data
        self._ddata = np.gradient(data)
        self._update_mask()
        # The structure element for binary dilation and erosion.
        self.structure = np.ones((3,)*np.ndim(data))
    
    def _update_mask(self):
        """Pre-compute masks for speed."""
        self._threshold_mask = self._data > self._theta
        self._threshold_mask_v = self._data > self._theta/np.abs(self._v)
    
    levelset = property(lambda self: self._u,
                        set_levelset,
                        doc="The level set embedding function (u).")
    data = property(lambda self: self._data,
                        set_data,
                        doc="The data that controls the snake evolution "
                            "(the image or g(I)).")
    balloon = property(lambda self: self._v,
                        set_balloon,
                        doc="The morphological balloon parameter "
                            "(ν (nu, not v)).")
    threshold = property(lambda self: self._theta,
                        set_threshold,
                        doc="The threshold value (θ).")
    
    def step(self):
        """Perform a single step of the morphological snake evolution."""
        # Assign attributes to local variables for convenience.
        u = self._u
        gI = self._data
        dgI = self._ddata
        theta = self._theta
        v = self._v
        
        if u is None:
            raise ValueError("the levelset is not set (use set_levelset)")
        
        res = np.copy(u)
        
        # Balloon.
        if v > 0:
            aux = binary_dilation(u, self.structure)
        elif v < 0:
            aux = binary_erosion(u, self.structure)
        if v!= 0:
            res[self._threshold_mask_v] = aux[self._threshold_mask_v]
        
        # Image attachment.
        aux = np.zeros_like(res)
        dres = np.gradient(res)
        for el1, el2 in zip(dgI, dres):
            aux += el1*el2
        res[aux > 0] = 1
        res[aux < 0] = 0
        
        # Smoothing.
        for i in range(self.smoothing):
            res = curvop(res)
        
        self._u = res
    
    def run(self, nb_iters):
        """Run several nb_iters of the morphological snakes method."""
        last_segm = np.array(self.levelset)
        for i in range(nb_iters):
            self.step()
            if np.array_equal(last_segm, self.levelset):
                logging.info('no change after %i iterations', i)
                break
            last_segm = np.array(self.levelset)
    

def evolve_visual(msnake, fig=None, levelset=None, num_iters=20, background=None):
    """
    Visual evolution of a morphological snake.
    
    Parameters
    ----------
    msnake : MorphGAC or MorphACWE instance
        The morphological snake solver.
    fig: object, optional
        Handles to actual figure.
    levelset : array-like, optional
        If given, the levelset of the solver is initialized to this. If not
        given, the evolution will use the levelset already set in msnake.
    num_iters : int, optional
        The number of iterations.
    background : array-like, optional
        If given, background will be shown behind the contours instead of
        msnake.data.
    """
    if levelset is not None:
        msnake.levelset = levelset
    
    # Prepare the visual environment.
    if fig is None:
        fig = plt.figure()
    fig.clf()
    ax1 = fig.add_subplot(1, 2, 1)
    if background is None:
        ax1.imshow(msnake.data, cmap=plt.cm.gray)
    else:
        ax1.imshow(background, cmap=plt.cm.gray)
    ax1.contour(msnake.levelset, [0.5], colors='r')
    
    ax2 = fig.add_subplot(1, 2, 2)
    ax_u = ax2.imshow(msnake.levelset)
    plt.pause(0.001)
    
    # Iterate.
    for _ in tqdm.tqdm(range(num_iters)):
        # Evolve.
        msnake.step()
        
        # Update figure.
        del ax1.collections[:]
        ax1.contour(msnake.levelset, colors='r')
        ax_u.set_data(msnake.levelset)
        fig.canvas.draw()
        #plt.pause(0.001)
    
    # Return the last levelset.
    return msnake.levelset


def evolve_visual3d(msnake, fig=None, levelset=None, num_iters=20,
                    animate_ui=True, animate_delay=250):
    """
    Visual evolution of a three-dimensional morphological snake.
    
    Parameters
    ----------
    msnake : MorphGAC or MorphACWE instance
        The morphological snake solver.
    fig: object, optional
        Handles to actual figure.
    levelset : array-like, optional
        If given, the levelset of the solver is initialized to this. If not
        given, the evolution will use the levelset already set in msnake.
    num_iters : int, optional
        The number of iterations.
    animate_ui : bool, optional
        Show the animation interface
    animate_delay : int, optional
        The number of delay between frames.
    """
    from mayavi import mlab

    if levelset is not None:
        msnake.levelset = levelset

    if fig is None:
        fig = mlab.gcf()
    mlab.clf()
    src = mlab.pipeline.scalar_field(msnake.data)
    mlab.pipeline.image_plane_widget(src, plane_orientation='x_axes',
                                     colormap='gray')
    cnt = mlab.contour3d(msnake.levelset, contours=[0.5])
    
    @mlab.animate(ui=animate_ui, delay=animate_delay)
    def anim():
        tqdm_bar = tqdm.tqdm(total=num_iters)
        for i in range(num_iters):
            msnake.step()
            cnt.mlab_source.scalars = msnake.levelset
            # logging.debug("Iteration %i/%i..." % (i + 1, num_iters))
            tqdm_bar.update()
            yield
    
    anim()
    mlab.show()
    
    # Return the last levelset.
    return msnake.levelset
