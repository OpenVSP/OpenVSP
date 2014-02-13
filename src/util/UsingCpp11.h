//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//
#ifndef USINGCPP11_H
#define USINGCPP11_H

#include <memory>
#include <unordered_map>

#if _MSC_VER == 1500
namespace std
{
using std::tr1::unordered_map;
using std::tr1::unordered_multimap;
using std::tr1::shared_ptr;
}
#endif



#endif



