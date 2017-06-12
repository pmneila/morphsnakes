
from . import _morphsnakes as _ms

import numpy as np

class MorphACWE(object):
    
    def __init__(self, levelset, data, smoothing=1, lambda1=1, lambda2=1):
        self._levelset = np.uint8(levelset)
        self.data = np.float_(data)
        self.smoothing = smoothing
        self.lambda1 = lambda1
        self.lambda2 = lambda2
        
        self._build_c_instance()
    
    def _build_c_instance(self):
        if self._levelset.ndim == 2:
            klass = _ms.MorphACWE2d
        elif self._levelset.ndim == 3:
            klass = _ms.MorphACWE3d
        else:
            raise NotImplementedError("Only two and three dimensional images are valid")
        
        self.cmorphacwe = klass(self._levelset, self.data, self.smoothing, self.lambda1, self.lambda2)
    
    def set_levelset(self, levelset):
        self._levelset = np.uint8(levelset)
        self._build_c_instance()
    
    levelset = property(lambda self: self._levelset,
                        set_levelset,
                        doc="The level set embedding function.")
    
    def step(self):
        """Perform a single step of the morphological Chan-Vese evolution."""
        self.cmorphacwe.step()
    
    def run(self, iterations):
        """Run several iterations of the morphological Chan-Vese method."""
        for i in xrange(iterations):
            self.step()
    

class MorphGAC(object):
    
    def __init__(self, levelset, data, smoothing=1, threshold=1, balloon=1):
        self._levelset = np.uint8(levelset)
        self.data = np.float_(data)
        self.grads = np.gradient(self.data)
        self.smoothing = smoothing
        self.threshold = threshold
        self.balloon = balloon
        
        self._build_c_instance()
    
    def _build_c_instance(self):
        if self._levelset.ndim == 2:
            klass = _ms.MorphGAC2d
        elif self._levelset.ndim == 3:
            klass = _ms.MorphGAC3d
        else:
            raise NotImplementedError("Only two and three dimensional images are valid")
        
        self.cmorphgac = klass(self._levelset, self.data, self.grads, self.smoothing, self.threshold, self.balloon)
    
    def set_levelset(self, levelset):
        self._levelset = np.uint8(levelset)
        self._build_c_instance()
    
    levelset = property(lambda self: self._levelset,
                        set_levelset,
                        doc="The level set embedding function.")
    
    def step(self):
        """Perform a single step of the morphological Chan-Vese evolution."""
        self.cmorphgac.step()
    
    def run(self, iterations):
        """Run several iterations of the morphological Chan-Vese method."""
        for i in xrange(iterations):
            self.step()
