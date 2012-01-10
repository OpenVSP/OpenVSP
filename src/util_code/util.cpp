
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

