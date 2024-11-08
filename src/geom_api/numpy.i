/* File : numpy.i */
%{
#include <numpy/arrayobject.h>
%}
%init %{
import_array();
%}
%nothread as_numpy;

%extend vec3d{
    PyObject* as_numpy() {
        npy_intp dims[1] = {3};
        PyArrayObject* numpy_array = (PyArrayObject*) PyArray_SimpleNew(1, dims, NPY_DOUBLE);

        // Fill the NumPy array with the data from the vector
        double *data_ptr = (double*) PyArray_DATA(numpy_array);
        data_ptr[0] = self->v[0];
        data_ptr[1] = self->v[1];
        data_ptr[2] = self->v[2];

        return (PyObject*)numpy_array;
    }
}
%extend std::vector<vec3d>{
    PyObject* as_numpy() {
        int size = self->size();
        npy_intp dims[2] = {size, 3};
        PyObject *numpy_array = PyArray_SimpleNew(2, dims, NPY_DOUBLE);

        // Fill the NumPy array with the data from the vector
        double *data_ptr = (double*)PyArray_DATA((PyArrayObject*)numpy_array);
        for (int i = 0; i < size; ++i) {
            data_ptr[i * 3 + 0] = (*self)[i].v[0];
            data_ptr[i * 3 + 1] = (*self)[i].v[1];
            data_ptr[i * 3 + 2] = (*self)[i].v[2];
        }

        return numpy_array;
    }
}
