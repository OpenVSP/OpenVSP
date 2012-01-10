//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//


#include "util.h"
#include <math.h>

template <class T> bool ContainsVal( vector< T > & vec, T const & val )
{
	for ( int i = 0 ; i < (int)vec.size() ; i++ )
	{
		if ( val == vec[i] )
			return true;
	}
	return false;
}

