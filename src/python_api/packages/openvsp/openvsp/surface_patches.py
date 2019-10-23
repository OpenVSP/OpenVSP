# Copyright (c) 2018 Uber Technologies, Inc.

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

import openvsp as vsp
from typing import List


class SurfaceComponent:
    def __init__(self, name, id, surfaces):
        """
        Constructor
        :param name: name of this vsp component
        :param id: id of the vsp component
        :param surfaces: list of surfaces belonging to this vsp component
        :param surfaces: list of surfaces belonging to this vsp component
        """
        self.name = name
        self.id = id
        self.surfaces = surfaces


class Surface:
    def __init__(self, comp_id, surf_index, patches):
        """
        Constructor
        :param comp_id: id of the vsp component this surface belongs to
        :param surf_index: index of this surface
        :param patches: list of patches belonging to this surface
        """
        self.comp_id = comp_id
        self.surf_index = surf_index
        self.patches = patches


class SurfacePatch:
    def __init__(self, comp_id, surf_index, patch_index, x, y, z, nx, ny, nz):
        """
        Constructor
        :param comp_id: id of the vsp component this patch belongs to
        :param surf_index: surf index this patch belongs to
        :param patch_index: index of this patch in parent surface
        :param x: x coordinates
        :param y: y coordinates
        :param z: z coordinates
        :param nx: normal x components
        :param ny: normal y components
        :param nz: normal z components
        """
        self.comp_id = comp_id
        self.surf_index = surf_index
        self.patch_index = patch_index
        self.x = x
        self.y = y
        self.z = z
        self.nx = nx
        self.ny = ny
        self.nz = nz


def export_surface_patches(export_set: int, remove_degenerate=True):
    """
    Function that export surfaces patches of current set
    :param export_set: vsp set to export
    :param remove_degenerate: if true, degenerate surface patches are removed
    :return: list components with surface patches
    """
    import numpy as np

    components = []

    vsp.SetIntAnalysisInput("SurfacePatches", "Set", [export_set])
    surf_patch_res_id = vsp.ExecAnalysis("SurfacePatches")

    patch_results = vsp.parse_results_object(surf_patch_res_id)
    comp_ids = patch_results.components
    for comp_id in comp_ids:
        comp_res = vsp.parse_results_object(comp_id)
        surface_ids = comp_res.surfaces

        surfaces = []
        for surf_id in surface_ids:
            surf_res = vsp.parse_results_object(surf_id)
            patch_ids = surf_res.patches

            patches = []
            for patch_id in patch_ids:
                patch_res = vsp.parse_results_object(patch_id)

                add_patch = True
                if remove_degenerate and (len(patch_res.x) <= 1 or len(patch_res.x[0]) <= 1):
                    add_patch = False

                if add_patch:
                    patches.append(SurfacePatch(patch_res.comp_id[0], patch_res.surf_index[0], patch_res.patch_index[0],
                                                np.array(patch_res.x), np.array(patch_res.y), np.array(patch_res.z),
                                                np.array(patch_res.nx), np.array(patch_res.ny), np.array(patch_res.nz)))

            surfaces.append(Surface(surf_res.comp_id[0], surf_res.surf_index[0], patches))
        if len(surfaces) > 0:
            components.append(SurfaceComponent(comp_res.name[0], comp_res.id[0], surfaces))

    return components


def plot_surface_components(components: List[SurfaceComponent], plot_normals=True, quiver_len=0.2):
    """
    Plots a list of surface components
    :param components: list of surface components to plot
    :param plot_normals: set to true to plot normal vectors, set to false to turn off normal vector plotting
    :return: handle to plotted figure
    """

    import  matplotlib.pyplot as plt
    from mpl_toolkits.mplot3d import Axes3D

    fig = plt.figure()
    ax = fig.gca(projection='3d')

    for comp in components:
        for surf in comp.surfaces:
            for patch in surf.patches:
                ax.plot_surface(patch.x, patch.y, patch.z)
                if plot_normals:
                    ax.quiver(patch.x, patch.y, patch.z, patch.nx, patch.ny, patch.nz, length=quiver_len)

    vsp.set_3d_axis_equal(ax)
    plt.show()

    return fig
