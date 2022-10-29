#!/usr/bin/python
"""
pyvspaero - The Python wrapper for the VSPAero/VSPAero-adjoint

This python interface is designed to provide a easier interface to the
C++ layer of VSPAero. User-supplied hooks allow for nearly complete
customization of any or all parts of the problem setup.
"""
# =============================================================================
# Imports
# =============================================================================
import numpy as np
from .utilities import BaseUI
import vspaero


class pyVSPAero(BaseUI):
    """
    The class for working with a VSPAero model
    """

    # Default class options
    default_options = {
        'solver_type': [str, 'VLM', "'VLM' or 'Panel'."],
        'print_debug': [bool, True, 'Flag for whether to print debug information while loading file.'],
        'print_timing': [bool, False, 'Flag for printing out timing information for class procedures.'],
    }

    def __init__(self, vsp_file, comps=None, options=None, **kwargs):

        # openvsp python interface
        try:
            import openvsp
        except ImportError:
            raise ImportError("OpenVSP Python API failed to import. "
                              "Python API must be installed to use pyvspaero modules.")

        self.openvsp = openvsp

        # Clear the vsp model
        self.openvsp.ClearVSPModel()

        self.openvsp.ReadVSPFile(vsp_file)

        # List of all components returned from VSP. Note that this
        # order is important. It is the order that we use to map the
        # actual geom_id by using the geom_names
        all_comps = self.openvsp.FindGeoms()
        all_names = []
        for c in all_comps:
            all_names.append(self.openvsp.GetContainerName(c))

        if not comps:
            # no components specified, we use all
            self.comps = all_comps[:]
        else:
            # we get the vsp comp IDs from the comps list
            self.comps = []
            for c in comps:
                self.comps.append(all_comps[all_names.index(c)])

        # Create a VSP set that we'll use to identify surfaces we want to output
        for geom_id in all_comps:
            geom_name = self.openvsp.GetContainerName(geom_id)
            if geom_id in self.comps:
                set_flag = True
            else:
                set_flag = False
            self.openvsp.SetSetFlag(geom_id, self.VSPAERO_VSP_SET, set_flag)

        # Set default options
        self.set_options_to_default()
        # Update w/ user-defined options
        if options:
            for key in options:
                self.set_option(key, options[key])

        # Pick out default reference quantities based on first WingGeom found
        self.Sref, self.bref, self.cref = self._get_ref_quantities()

        # Overwrite ref quantities with user-defined ones
        for ref_quant in kwargs:
            if ref_quant.lower() == "sref":
                self.Sref = kwargs[ref_quant]
            elif ref_quant.lower() == "bref":
                self.bref = kwargs[ref_quant]
            elif ref_quant.lower() == "cref":
                self.cref = kwargs[ref_quant]

        self.func_list = {}

    def __del__(self):
        """
        Clear OpenVSP model on deconstruct of class.
        """
        self.openvsp.ClearVSPModel()

    def create_steady_problem(self, name, options=None, **kwargs):
        """
        Create a new staticProblem for modeling a static load cases.
        This object can be used to set loads, evalFunctions as well as perform
        solutions and sensitivities related to static problems

        Parameters
        ----------
        name : str
            Name to assign problem.
        options : dict
            Problem-specific options to pass to StaticProblem instance (case-insensitive).

        Returns
        ----------
        problem : SteadyProblem
            SteadyProblem object used for modeling and solving steady cases.
        """

        flight_vars = kwargs
        sol_type = self.get_option("solver_type")

        problem = vspaero.problems.SteadyProblem(name, self.openvsp, self.func_list.copy(), sol_type, options)
        # Set with original flight vars and coordinates, in case they have changed
        problem.set_flight_vars(Sref=self.Sref, bref=self.bref, cref=self.cref)
        problem.set_flight_vars(**flight_vars)
        return problem

    def add_function(self, func_name, func_id, comp_ids=None):
        """
        Generic method to add a function for pyVSPAero. It is intended to
        be reasonably generic since the user supplies the actual
        function handle to use. See the :py:mod:`~vspaero.functions` module
        for supported VSPAero eval functions.

        Parameters
        ----------
        func_name : str
            The user-supplied name for the function. This will
            typically be a string that is meaningful to the user

        func_id : vspaero.Function
            The function ID to use for creation. This must come
            from the functions module in vspaero.

        comp_ids: list
            List of compIDs to select.
        """
        self.func_list[func_name] = func_id

    def _get_ref_quantities(self):
        for comp in self.comps:
            geom_type = self.openvsp.GetGeomTypeName(comp)
            if geom_type == "Wing":
                sref_id = self.openvsp.FindParm(comp, "TotalArea", "WingGeom")
                Sref = self.openvsp.GetParmVal(sref_id)
                bref_id = self.openvsp.FindParm(comp, "TotalProjectedSpan", "WingGeom")
                bref = self.openvsp.GetParmVal(bref_id)
                cref_id = self.openvsp.FindParm(comp, "TotalChord", "WingGeom")
                cref = self.openvsp.GetParmVal(cref_id)
                return Sref, bref, cref
        return 100.0, 1.0, 1.0
