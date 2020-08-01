#
# Copyright 2008,2009 Free Software Foundation, Inc.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

# The presence of this file turns this directory into a Python package

'''
This is the GNU Radio SDRPLAY3 module.
This module supports the following SDRplay RSP devices:
  - RSP1
  - RSP1A
  - RSP2
  - RSPduo
  - RSPdx
This module requires SDRplay API version 3.x to work.
'''
from __future__ import unicode_literals
import os

# import pybind11 generated symbols into the sdrplay3 namespace
try:
    from .sdrplay3_python import *
except ImportError:
    dirname, filename = os.path.split(os.path.abspath(__file__))
    __path__.append(os.path.join(dirname, "bindings"))
    from .sdrplay3_python import *

# import any pure python here
#
