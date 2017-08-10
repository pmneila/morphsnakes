import os
import logging
import matplotlib

if os.environ.get('DISPLAY','') == '':
    logging.warning('No display found. Using non-interactive Agg backend')
    matplotlib.use('Agg')

import warnings

try:
    from .cmorphsnakes import MorphACWE, MorphGAC
except ImportError:
    warnings.warn("Could not load cmorphsnakes; using the pure Python implementation.")
    from .morphsnakes import MorphACWE, MorphGAC

from .morphsnakes import gborders, glines, evolve_visual, evolve_visual3d
