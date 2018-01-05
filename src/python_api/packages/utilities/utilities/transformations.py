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


import numpy as np
import math
import utilities.units as u

class TransMatrix:
    """
    Defines a class 4x4 affine transformation matrices
    """

    def __init__(self, mat=None):
        """
        Initializes to a transformation matrix with no transformations

        :param mat: optional 4x4 matrix for initialization
        """

        if mat is None:
            self.mat = np.identity(4)
        else:
            if mat.shape != (4, 4):
                raise ValueError("mat must be (4, 4) not {}".format(mat.shape))
            self.mat = mat

    @staticmethod
    def create_from_axis_angle(axis, angle):
        """
        Creates a transformation matrix from a rotation axis and angle about that axis
        :param axis: axis of rotation (x,y,z) vector
        :param angle: counter-clockwise angle of rotation (radians)
        :return: transformation matrix that will resulting from rotation about input axis
        """
        import numpy.linalg
        # Create unit vector of input axis
        axis_len = np.linalg.norm(axis)

        # Handle zero length rotation axis
        if np.isclose(axis_len, 0.0):
            return TransMatrix()

        unit_axis = axis/axis_len

        # Create new transformation matrix
        return_matrix = TransMatrix()

        ux = unit_axis[0]
        uy = unit_axis[1]
        uz = unit_axis[2]

        # pre-compute trig functions
        ca = np.cos(angle)
        sa = np.sin(angle)

        # fill in matrix
        return_matrix.mat[0, 0] = ca+(ux**2.0)*(1.0-ca)
        return_matrix.mat[0, 1] = ux*uy*(1.0-ca) - uz*sa
        return_matrix.mat[0, 2] = ux*uz*(1.0-ca) + uy*sa
        return_matrix.mat[1, 0] = uy*ux*(1.0-ca) + uz*sa
        return_matrix.mat[1, 1] = ca + (uy**2.0)*(1.0-ca)
        return_matrix.mat[1, 2] = uy*uz*(1.0-ca) - ux*sa
        return_matrix.mat[2, 0] = uz*ux*(1.0-ca) - uy*sa
        return_matrix.mat[2, 1] = uz*uy*(1.0-ca) + ux*sa
        return_matrix.mat[2, 2] = ca + (uz**2.0)*(1.0-ca)

        return return_matrix

    def apply_rotations(self, vector):
        """
        Applys rotations to input vector
        :param vector: column vectors
        :return: vectors rotated by transmat
        """

        return np.matmul(self.mat[0:3, 0:3], _convert_to_column_vector(vector))

    def apply_translations(self, vector):
        """
        Applys translations to input vector
        :param vector: column vector
        :return: translations apply to column vector
        """

        return vector + self.mat[0:3, 3].reshape((3, 1))

    def apply_transformation(self, vector):
        """
        Applys transformations to input vector
        :param vector: column vector
        :return: transformed column vector
        """

        return self.apply_translations(self.apply_rotations(vector))

    def get_inverse_transform(self):
        """
        Returns a transformation matrix that will result in the inverse
        of this tranformation matrix
        :return: inverse transformation matrix
        """

        ret_trans = TransMatrix()
        ret_trans.mat[0:3, 0:3] = self.mat[0:3, 0:3].transpose()

        orig_translations = self.mat[0:3, 3]
        ret_trans.mat[0:3, 3] = -1.0*np.matmul(ret_trans.mat[0:3, 0:3],
                                               _convert_to_column_vector(orig_translations)).reshape(3)

        return ret_trans

    def get_angles(self):
        """
        Returns (x,y,z) rotation angles which are applied in z then y then x order
        :return: (x,y,z) rotation angles
        """

        angles = np.zeros(3)

        if np.abs(self.mat[0, 2]) != 1:
            angles[1] = np.arcsin(self.mat[0, 2])
            cos_y = np.cos(angles[1])
            angles[0] = np.arctan2(self.mat[1, 2]*-1.0, self.mat[2, 2])
            angles[2] = np.arctan2(self.mat[0, 1]*-1.0, self.mat[0, 0])
        elif self.mat[0, 2] == 1:
            angles[1] = math.pi/2.0
            angles[0] = np.arctan2(self.mat[1, 0], self.mat[1, 1])
            angles[2] = 0.0
        elif self.mat[0, 2] == -1:
            angles[1] = -math.pi/2.0
            angles[0] = np.arctan2(self.mat[2, 1], self.mat[2, 0])
            angles[2] = 0.0

        return angles*u.rad2deg

    def get_angles_xyz(self):
        """
        Returns (x,y,z) rotation angles which are applied in x then y then z order
        :return: (x,y,z) rotation angles
        """

        angles = np.zeros(3)

        if self.mat[2, 0] == -1.0:
            angles[1] = math.pi/2.0
            angles[2] = 0.0
            angles[0] = math.atan2(self.mat[0, 1], self.mat[1, 1])
        elif self.mat[2, 0] == 1.0:
            angles[1] = -math.pi/2.0
            angles[2] = 0.0
            angles[0] = math.atan2(self.mat[0, 1], self.mat[1, 1])
        else:
            angles[1] = np.arcsin(self.mat[2, 0])
            angles[0] = np.arctan2(self.mat[2, 1], self.mat[2, 2])
            angles[2] = np.arctan2(self.mat[1, 0], self.mat[0, 0])

        return angles*u.rad2deg

    def get_angles_zxy(self):
        """
        Returns (x,y,z) rotation angles which are applied in z then x then y order
        :return: (x,y,z) rotation angles
        """

        angles = np.zeros(3)

        if self.mat[1, 2] == -1.0:
            angles[0] = math.pi/2.0
            angles[2] = 0.0
            angles[1] = math.acos(self.mat[0, 0])
        elif self.mat[1, 2] == 1.0:
            angles[0] = -math.pi/2.0
            angles[2] = 0.0
            angles[1] = math.acos(self.mat[0, 0])
        else:
            angles[0] = math.asin(-self.mat[1, 2])
            angles[1] = math.atan2(self.mat[0, 2], self.mat[2, 2])
            angles[2] = math.atan2(self.mat[1, 0], self.mat[1, 1])

        return angles*u.rad2deg

    def get_translations(self):
        """
        Returns the translations from matrix
        :return: (x,y,z) translations
        """
        return np.array(self.mat[0:3, 3]).reshape(3, 1)

    def set_translations(self, translation_vector):
        """
        Sets the (x,y,z) translation of the matrix
        :return: Nothing
        """
        self.mat[0:3, 3] = np.array(translation_vector).reshape(3)


def _convert_to_column_vector(vector):
    """
    Converts a 1d vector to a column vector
    :param vector: 1d vector or matrix of column vectors
    :return: column vector or matrix of column vectors
    """
    column_vector = np.array(vector)
    if len(column_vector.shape) == 1:
        num_rows = column_vector.shape[0]
        column_vector = np.reshape(vector, (num_rows, 1))
    return column_vector
