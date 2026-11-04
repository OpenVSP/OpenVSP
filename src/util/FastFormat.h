//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Number formatting for large text exports.
//
//   Each function appends exactly what snprintf writes for the printf conversion
//   it is named after.  snprintf takes a process-wide lock on some platforms, so
//   threads formatting a mesh file side by side wait on one another; these do not.
//
//******************************************************************************

#ifndef FASTFORMAT_H
#define FASTFORMAT_H

#include <string>

namespace FastFormat
{
    // %d, %lld
    void AppendInt( std::string &out, long long v );

    // %<width>.<prec>g
    void AppendG( std::string &out, double x, int width, int prec );

    // %<width>.<prec>e
    void AppendE( std::string &out, double x, int width, int prec );

    // %<width>.<prec>f
    void AppendF( std::string &out, double x, int width, int prec );
}

#endif
