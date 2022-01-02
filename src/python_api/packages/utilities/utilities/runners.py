# Copyright (c) 2020 Uber Technologies, Inc.

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.


import numpy as np
import logging
from multiprocessing_logging import MultiProcessingHandler

class DOE():
    """
    DOE
    """
    def __init__(self, cases):
        """

        :param cases: list of cases to execute
        """
        mtlog = MultiProcessingHandler('foo.log')
        self.logger = logging.getLogger(__name__)
        self.logger.addHandler(mtlog)

        #self.logger = logging.getLogger(__name__)
        self.logger.log(logging.DEBUG,"Initializing DOE object.")
        self.cases = cases
        self.results = None
        self.runs = None
    @property
    def shape(self):
        return (len(self.cases),)

    def run(self, f, nCores=1, **kwargs):
        """

        :param f: function handle that returns a result
        :param nCores: number of cores to run on. For single threaded, set nCores==1. To multi-thread, set >1.
        :return:
        """
        self.logger.info("Running {} cases with {} cores.".format(len(self.cases), nCores))
        if nCores == 1:
            r = list( map(f, self.cases) )
        elif nCores > 1:
            # we have to use pathos.multiprocessing instead of multiprocessing because pathos uses
            # dill instead of pickle, which works much better with pickling functions
            # https://stackoverflow.com/a/19985580
            import pathos.multiprocessing as mp
            with mp.Pool(nCores) as pool:
                r = pool.map(f, self.cases)
        else:
            raise ValueError("nCores must be >= 1.")

        self.results = r
        self.runs = []
        for i in range(0,len(self.results)):
            self.runs.append( [self.cases[i], self.results[i]])

    def plot(self,x,y):
        # choose x,y indices and plot them. maybe support 3d?
        pass


    def unravel_index(self,indices):
        self.logger.debug("Unraveling {}".format(indices))
        return np.unravel_index(indices, self.shape)


    def ravel_index(self,indices):
        self.logger.debug("Raveling {}".format(indices))
        return np.ravel_multi_index(indices, self.shape)

    def getByIndex(self, array, indx):
        """
        Returns an element at index=indx from array. The index can either be linear or subscripts

        Example:
        getByIndex( array, 1) returns the 1-th index of array
        getByIndex( array, (0,0,1)) returns the (0,0,1) index of array.

        :param array: array to parse
        :param indx: index to return
        :return: array(indx)
        """

        try:
            linearIndex = self.ravel_index(indx)
            self.logger.debug("Got array by subscript index")
        except ValueError:
            linearIndex = indx
            self.logger.debug("Got array by linear index")
        return array[linearIndex]

    def getResult(self, indx):
        """
        Returns results of indx
        :param indx: index to return
        :return: results[indx]
        """
        self.logger.info("Getting result.")
        try:
            return self.getByIndex(self.results,indx)
        except AttributeError as a:
            newException = AttributeError(a.__str__() + "\nCannot access results until analysis has been run with the run method.")
        raise newException

    def getCase(self, indx):
        """
        Returns cases of indx
        :param indx: index to return
        :return: cases[indx]
        """
        self.logger.info("Getting case.")
        return self.getByIndex(self.cases,indx)

    def getRun(self, indx):
        """
        Returns runs of indx
        :param indx: index to return
        :return: runs[indx]
        """
        self.logger.info("Getting run.")
        try:
            return self.getByIndex(self.runs,indx)
        except AttributeError as a:
            newException = AttributeError(a.__str__() + "\nCannot access runs until analysis has been run with the run method.")
        raise newException
class LatinHypercube(DOE):
    """
    Latin Hypercube DOE
    """
    def __init__(self, bnds, npts,seed=None):
        """

        :param bnds: array of bnds for each variable. example: ( (0,1), (-5,20), (5, 10) )
        :param npts: desired number of runs
        :param seed: numpy.random seed. default is None, which uses the current numpy.random seed
        """
        self.logger = logging.getLogger(__name__)
        self.logger.info("Initializing LatinHypercube object.")
        self.bnds = bnds
        self.nCases = npts
        # build LHC runs based on bnds and npts
        if seed is not None:
            np.random.seed(seed)
        nVars = len(bnds)
        cases = np.empty( (self.nCases, len(bnds)))

        # generate normalized cases
        segments = np.linspace(0,1,npts+1)
        delta = segments[1] - segments[0]

        for iVar in range(0, nVars):
            self.logger.debug("Generating normalized variable.")
            for iCase in range(0, npts):
                cases[iCase, iVar] = np.random.rand(1) * delta + segments[iCase]

            self.logger.debug("Randomizing normalized variable.")
            cases[:,iVar] = np.random.permutation( cases[:,iVar])

        # now multiply by the bnds for each variable
        for iVar in range(0,nVars):
            self.logger.debug("Scaling normalized variable to specified bounds.")
            cases[:,iVar] = cases[:,iVar]*abs(bnds[iVar][0] - bnds[iVar][1]) + min(bnds[iVar])

        self.logger.debug("Done initializing LatinHypercube object.")

        super().__init__(cases)
    @property
    def shape(self):
        return (self.nCases,)
class Random(DOE):
    """
    Random DOE
    """
    def __init__(self, bnds, npts, seed=None):
        """

        :param bnds: array of bnds for each variable. example: ( (0,1), (-5,20), (5, 10) )
        :param npts: desired number of runs
        :param seed: numpy.random seed. default is None, which uses the current numpy.random seed
        """
        self.logger = logging.getLogger(__name__)
        self.logger.info("Initializing Random DOE object.")
        self.bnds = bnds
        if seed is not None:
            np.random.seed(seed)

        nVars = len(bnds)
        self.logger.debug("Generating normalized variable.")
        normalizedCases = np.random.random_sample( (npts, nVars) )
        self.logger.debug("Done generating normalized variable.")
        self.nCases = normalizedCases.shape[0]
        cases = np.empty(normalizedCases.shape)
        for j in range(0, nVars):
            self.logger.debug("Scaling normalized variable.")
            for i in range(0,npts):
                cases[i,j] = normalizedCases[i,j] * abs(bnds[j][1] - bnds[j][0]) + min(bnds[j])

        self.logger.debug("Done initializing LatinHypercube object.")
        super().__init__(cases)

    @property
    def shape(self):
        return (self.nCases,)

class FullFactorial(DOE):
    """
    Full Factorial DOE
    """
    def __init__(self, vars):
        """

        :param vars: array of arrays of variables. example: ( (0,1,2), (4,6,7,3,2,6),("hello","goodbye") )
        """
        self.logger = logging.getLogger(__name__)
        self.logger.info("Initializing FullFactorial variables.")

        self.vars = vars
        shape = []
        #TODO this might break for a 1-d list. do we support?
        for i in range(0,len(vars)):
            shape.append(len(vars[i]))
        self._shape = shape
        import itertools

        cases = list( itertools.product( *vars) )
        self.logger.debug("Done building FullFactorial variables.")

        super().__init__(cases)
    @property
    def shape(self):
        return self._shape