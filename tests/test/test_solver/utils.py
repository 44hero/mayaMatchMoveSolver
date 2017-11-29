"""
Testing Utilities - base class for the test cases.
"""

import os
import math
import time
import unittest

try:
    import maya.standalone
    maya.standalone.initialize()
except RuntimeError:
    pass
import maya.cmds


class SolverTestBase(unittest.TestCase):

    def setUp(self):
        # Start the Profiler
        self._profilerName = self.id().replace('.', '_')
        self._profilerDataName = self._profilerName + '.txt'
        self._profilerPath = None
        if '__file__' in dir():
            self._profilerPath = os.path.join(os.path.dirname(__file__), self._profilerName)
        maya.cmds.profiler(addCategory='mmSolver')
        maya.cmds.profiler(bufferSize=250)
        maya.cmds.profiler(sampling=True)

    def tearDown(self):
        # Stop the Profiler
        maya.cmds.profiler(sampling=False)
        if self._profilerPath is not None:
            maya.cmds.profiler(output=self._profilerPath)

    def quitMaya(self):
        if maya.cmds.about(batch=True):
            maya.cmds.quit(force=True)

    def approxEqual(self, x, y, eps=0.0001):
        return x == y or (x < (y + eps) and x > (y - eps))
