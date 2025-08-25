%module vspaero_opt
%{
    // Include the C header file.
    #include "vspaero.H"
    #include "OptimizationParameterData.H"
    #include "OptimizationGradientData.H"
%}

// Expose all functions in header
%include "vspaero.H"
%include "OptimizationParameterData.H"
%include "OptimizationGradientData.H"

// To expose value at pointer
%include <std_vector.i>
namespace std {
    %template(DoubleVector) vector<double>;
}

// Create buffer of 1024 characters for FileName

%include "cstring.i"
%cstring_bounded_output(char *FileName, 1024);