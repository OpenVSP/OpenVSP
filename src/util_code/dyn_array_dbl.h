//******************************************************************************
//    
//   Single Dimension Array Class of Doubles
//  
// 
//   J.R. Gloudemans, Paul C. Davis - 10/17/94
//   Sterling Software
//
//******************************************************************************

#ifndef DYN_ARRAY_DBL_H
#define DYN_ARRAY_DBL_H

#include <iostream>
using namespace std;

#include <assert.h>

//=== Define Error Flags ===//
#define ERR_INIT		0
#define ERR_NO_VALUE		1
#define ERR_INTERP_ARRAY_DIM	2
#define ERR_INTERP_ARRAY_INDEX	3

class Dyn_array_dbl
{
private:

        int	chunk_size;
	    int	total_size;
        int	dim;
        double*	arr;

        void	allocate_space();
        void	clear_space()			{ if (dim > 0)  delete [] arr; }
        void	print_error_message(int error_flag);

public:

	    Dyn_array_dbl();			           // Default Constructor
        Dyn_array_dbl( const Dyn_array_dbl& ); // Copy Constructor
        Dyn_array_dbl( int d1 );	           // Dimension Constructor
	    ~Dyn_array_dbl();			           // Destructor

        void	set_chunk_size( int size_in )      { chunk_size = size_in; }
        void	init( int d1 );
        int	dimension() const	                   { return(dim); }
        int	num_items() const			           { return(dim); }
        double&	operator [] (int ind1) const	   { assert( ind1 > -1 && ind1 < dim );
                                                     return( arr[ind1] ); }
        double&	operator () (int ind1) const	   { assert( ind1 > -1 && ind1 < dim );
                                                     return( arr[ind1] ); }
		double	get(int ind1) const                { assert( ind1 > -1 && ind1 < dim );
                                                     return( arr[ind1] ); }
		Dyn_array_dbl& operator= ( const Dyn_array_dbl&);

        void	append(double in_item);
        int	del(double in_item);
        int	del_index(int ind);
        int	find_index(double value);
        int	find_interval(double value);
        int	find_interval(double value, int guess);
        double	interpolate(double value, int interval);
        double	interpolate_array(Dyn_array_dbl* array_ptr, double value);
        double	find_min_value();
        double	find_max_value();

       
        
};

#endif
