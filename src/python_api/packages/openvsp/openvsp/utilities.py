# Copyright (c) 2018-2020 Uber Technologies, Inc.

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

from utilities import RunManager
import openvsp as vsp
from collections import namedtuple
import numpy as np

class PropInfo():
    def __init__(self, geom_id, thrust_vector, rotation_direction, hub_center, transmat, diameter):
        self.geom_id = geom_id
        self.thrust_vector = thrust_vector
        self.rotation_direction = rotation_direction
        self.hub_center = hub_center
        self.transmat = transmat
        self.diameter = diameter

    @property
    def area(self):
        return np.pi * np.power(self.diameter/2, 2)

def genX3d(file=None,set=vsp.SET_ALL, dims=[1000,400], **kwargs):

    if file is not None:
        vsp.ClearVSPModel()
        vsp.ReadVSPFile(file)
        vsp.Update()

    with RunManager(**kwargs):
        vsp.ExportFile("prop.x3d", set, vsp.EXPORT_X3D)
        with open("prop.x3d", "r") as f:
            x3d_str = f.read()


        x3d_str = x3d_str[0:26] + " width=\"{}px\" height=\"{}px\"".format(dims[0],dims[1]) + x3d_str[26:-1]

    return x3d_str


def convert_vec3d_array_to_list_matrix(vec3d_array):
    """
    Converts an array of vec3d objects into a list of x,y,z points
    :param vec3d_array: array of vec3d objects
    :return: list of x,y,z pairs
    """
    return [[v3d.x(), v3d.y(), v3d.z()] for v3d in vec3d_array]


def convert_double_tuple_to_list_matrix(tuple_matrix):
    """
    Converts a double matrix of tuples into a maxtrix of lists
    :param tuple_matrix: matrix of tuples
    :return: matrix of lists
    """
    return [list(row) for row in tuple_matrix]


def parse_results_object(res_id):
    """
    Function to parse a generic results object from openvsp results manager
    :param res_id: id of the results object to parse
    :return: named tuple of results values
    """
    names = vsp.GetAllDataNames(res_id)
    name = vsp.GetResultsName(res_id)

    res_tuple = namedtuple(name, names)
    data = []
    for name in names:
        type = vsp.GetResultsType(res_id, name)
        d = []
        if type == vsp.INT_DATA:
            d = list(vsp.GetIntResults(res_id, name))
        elif type == vsp.STRING_DATA:
            d = list(vsp.GetStringResults(res_id, name))
        elif type == vsp.DOUBLE_DATA:
            d = list(vsp.GetDoubleResults(res_id, name))
        elif type == vsp.DOUBLE_MATRIX_DATA:
            d = vsp.convert_double_tuple_to_list_matrix(vsp.GetDoubleMatResults(res_id, name))
        elif type == vsp.VEC3D_DATA:
            d = vsp.convert_vec3d_array_to_list_matrix(vsp.GetVec3dResults(res_id, name))

        data.append(d)

    return res_tuple(*data)


def get_wing_reference_quantities(wing_name=None, wing_id=None):
    """
    Gets wing reference area, reference span, and reference chord (sref, bref, cref) from the TotalArea, TotalSpan, and
    TotalChord properties of Wing component

    :param wing_name: Name of the wing (will find the first wing with this name)
    :param wing_id: ID of the wing object, if None wing name will be used to find the wing id. Wing ID takes precedence
    over wing_name
    :return: sref, bref, and cref
    """

    # Check that the wing_name and wing_id are not both none
    if wing_name is None and wing_id is None:
        raise ValueError("wing_name and wing_id cannot both be None")

    # If wing_id is None, then use wing name to find the wing id
    if wing_id is None:
        found_wing_id = vsp.FindGeom(wing_name, 0)
        if not found_wing_id:
            raise ValueError("could not find wing with name \"{}\"".format(wing_name))
        wing_id = found_wing_id

    # Get reference parameters
    # TODO: Error check that the wing_id is actually the id to a wing component
    bref = vsp.GetParmVal(wing_id, "TotalSpan", "WingGeom")
    sref = vsp.GetParmVal(wing_id, "TotalArea", "WingGeom")
    cref = vsp.GetParmVal(wing_id, "TotalChord", "WingGeom")

    return sref, bref, cref


def get_propeller_thrust_vectors(prop_set, alternate_rotation_direction=False):
    """
    Returns propeller thrust vector directions and rotation direction for all propellers in
    the input set
    :param prop_set: set containing all propellers (no other geometries should be included)
    :return: Named Tuple with (geom_id, thrust_vector, rotation_direction, hub_center)
    """
    import degen_geom as dg

    # Run degen geom on the input set
    degen_mgr = vsp.run_degen_geom(set_index=prop_set)

    # loop over all geoms in degen_mgr
    results = []
    for geom_id, degens in degen_mgr.degen_objs.items():
        for propeller in degens.copies:
            # Only look at first blade
            blade_0 = degens.copies[propeller][0]
            results.append(get_single_propeller_info(blade_0))

    if alternate_rotation_direction:
        for i in range(len(results)):
            if i % 2 == 1:
                results[i].rotation_direction = -1

    return results


def get_single_propeller_info(prop_dg):
    """
    Gets propeller info for a single propeller degen geom object
    :param prop_dg: propeller degen geom object
    :return: propeller info named tuple
    """

    # Define named tuple
    #PropInfo = namedtuple("PropInfo", "geom_id thrust_vector rotation_direction hub_center transmat diameter")

    reverse_flag = vsp.GetParmVal(prop_dg.geom_id, "ReverseFlag", "Design")
    rotation_dir_geom = 1 if reverse_flag < 0.5 else -1

    diameter = vsp.GetParmVal(prop_dg.geom_id, "Diameter", "Design")

    # Use transformation matrix to get hub center and rotate default thrust vector into position
    transmat = prop_dg.transmat
    hub_center = transmat.get_translations()
    thrust_vector = transmat.apply_rotations(np.array([-1.0, 0.0, 0.0]).reshape((3, 1)))

    # Check the flip normal flag to determine if rotation direction should be reversed
    rotation_dir_blade = rotation_dir_geom
    if (prop_dg.flip_normal == 0 and not reverse_flag) or (prop_dg.flip_normal == 1 and reverse_flag):
        rotation_dir_blade *= -1

    return PropInfo(prop_dg.geom_id, thrust_vector, rotation_dir_blade, hub_center, transmat, diameter)


def plot_propeller_info(prop_infos, vector_scale=10.0, markersize=2.0, mutation_scale=20.0):
    """
    Plots an array of propeller info tuples to visually inspect thrust and rotation directions
    :param prop_infos: array of prop info tuples
    :param vector_scale: length of each thrust vector
    :param markersize: size of marker for hub location
    :param mutation_scale: mutation scale applied to arrow objects
    :return: figure handle
    """
    from matplotlib import pyplot as plt
    from matplotlib.patches import FancyArrowPatch
    from mpl_toolkits.mplot3d import proj3d

    try:
        if len(vector_scale) != len(prop_infos):
            raise ValueError("vector_scale must be either a scalar or have the same length as prop_infos")
        #else:
        #    maxScale = max(vector_scale)
        #    vector_scale = [20.*v/maxScale for v in vector_scale]
    except TypeError:
        vector_scale = [vector_scale for p in prop_infos]

    class Arrow3D(FancyArrowPatch):
        def __init__(self, xs, ys, zs, *args, **kwargs):
            self._verts3d = xs, ys, zs
            FancyArrowPatch.__init__(self, (0, 0), (0, 0), *args, **kwargs)

        def draw(self, renderer):
            xs3d, ys3d, zs3d = self._verts3d
            xs, ys, zs = proj3d.proj_transform(xs3d, ys3d, zs3d, renderer.M)
            self.set_positions((xs[0], ys[0]), (xs[1], ys[1]))
            FancyArrowPatch.draw(self, renderer)

    fig = plt.figure()
    ax = fig.add_subplot(111, projection='3d')

    for iprop, prop_info in enumerate(prop_infos):
        color = 'blue'
        if prop_info.rotation_direction == -1:
            color = 'red'

        ax.plot(prop_info.hub_center[0], prop_info.hub_center[1], prop_info.hub_center[2], 'o', markersize=markersize,
                color='black')
        a = Arrow3D([prop_info.hub_center[0][0], prop_info.hub_center[0][0]+prop_info.thrust_vector[0][0]*vector_scale[iprop]],
                    [prop_info.hub_center[1][0], prop_info.hub_center[1][0]+prop_info.thrust_vector[1][0]*vector_scale[iprop]],
                    [prop_info.hub_center[2][0], prop_info.hub_center[2][0]+prop_info.thrust_vector[2][0]*vector_scale[iprop]],
                    mutation_scale=mutation_scale, lw=2, arrowstyle="-|>", color=color)
        ax.add_artist(a)

        # add circle for rotor diameter
        radius = prop_info.diameter/2.0
        thetas = np.linspace(0.0, 2.0*np.pi, 50)
        yprop = np.cos(thetas)*radius
        zprop = np.sin(thetas)*radius
        xprop = np.zeros(len(thetas))
        prop_points = np.array([xprop, yprop, zprop])
        prop_points_transformed = prop_info.transmat.apply_transformation(prop_points)
        ax.plot(prop_points_transformed[0, :], prop_points_transformed[1, :], prop_points_transformed[2, :],
                color='black')

    ax.set_xlabel('x')
    ax.set_ylabel('y')
    ax.set_zlabel('z')

    set_3d_axis_equal(ax)

    return fig


def run_degen_geom(set_index=None, set_name=None):
    """
    Runs degen geom on input set
    :param set_index: set index, will take precedence if both set index and set name are specified
    :param set_name: name of set, will be used if set index is not specified
    :return: degen geom manager object
    """
    import degen_geom as dg

    if set_index is None and set_name is None:
        raise ValueError("set_index and set_name cannot both be None")

    # Get set index from name, if set index was not specified
    if set_index is None:
        set_index = vsp.GetSetIndex(set_name)

    # Run degen geom on the input set
    vsp.SetAnalysisInputDefaults("DegenGeom")
    vsp.SetIntAnalysisInput("DegenGeom", "Set", [set_index], 0)
    vsp.SetIntAnalysisInput("DegenGeom", "WriteCSVFlag", [0], 0)
    vsp.SetIntAnalysisInput("DegenGeom", "WriteMFileFlag", [0], 0)
    degen_res_id = vsp.ExecAnalysis("DegenGeom")

    degen_objs = vsp.parse_degen_geom(degen_res_id)

    degen_mgr = dg.DegenGeomMgr(degen_objs)

    return degen_mgr


def set_3d_axis_equal(ax):
    """
    Sets axis aspect ratio to equal
    :param ax: 3d axis object
    :return:
    """
    xlims = list(ax.get_xlim())
    ylims = list(ax.get_ylim())
    zlims = list(ax.get_zlim())
    rx = xlims[1]-xlims[0]
    ry = ylims[1]-ylims[0]
    rz = zlims[1]-zlims[0]

    rmax = max(rx, ry, rz)
    if rx < rmax:
        diff = abs(rx-rmax)
        xlims[0] -= diff/2.0
        xlims[1] += diff/2.0

    if ry < rmax:
        diff = abs(ry-rmax)
        ylims[0] -= diff/2.0
        ylims[1] += diff/2.0

    if rz < rmax:
        diff = abs(rz-rmax)
        zlims[0] -= diff/2.0
        zlims[1] += diff/2.0

    ax.set_xlim(xlims[0], xlims[1])
    ax.set_ylim(ylims[0], ylims[1])
    ax.set_zlim(zlims[0], zlims[1])


def export_airfoils(set=vsp.SET_ALL, **kwargs):
    import airfoils
    af_dict = dict()
    with RunManager(**kwargs) as r:
        output_file = r.cwd + "/airfoils.csv"
        vsp.ExportFile(output_file, set, vsp.EXPORT_SELIG_AIRFOIL)

        #parse airfoils.csv
        with open(output_file,"r") as f:
            lines = f.readlines()

        i = 2
        strs = lines[i].split(",")
        rootDir = strs[1].strip()
        i = i+1
        while i < len(lines):
            if "Airfoil File Name" in lines[i]:
                file = rootDir + lines[i].split(",")[1].strip()
                af = airfoils.VspSeligExport(file)

                i = i+1

                af.geom_name = lines[i].split(",")[1].strip()
                i = i+1

                af.geom_id = lines[i].split(",")[1].strip()
                i = i+1

                af.airfoil_index = int(lines[i].split(",")[1])
                i = i+1

                af.xsec_flag = lines[i].split(",")[1].strip() == "1"

                i = i+1

                if af.xsec_flag:
                    af.xsec_index = int(lines[i].split(",")[1])
                    i = i+1

                    af.xsec_surf_id = lines[i].split(",")[1].strip()
                    i = i+1

                af.foil_surf_u_value = float( lines[i].split(",")[1] )

                i = i+1
                af.global_u_value = float( lines[i].split(",")[1] )

                i = i+1
                af.le_point = np.array( lines[i].split(",")[1:]).astype(float)

                i = i+1
                af.te_point = np.array( lines[i].split(",")[1:] ).astype(float)

                i = i+1
                af.chord = float( lines[i].split(",")[1])

                i = i+1

                #af.findSegments()
                try:
                    af_dict[af.geom_id].append( af )
                except KeyError:
                    af_dict[af.geom_id] = []
                    af_dict[af.geom_id].append( af )


            else:
                i = i + 1

    return af_dict