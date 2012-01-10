//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//
//   Multiple Dimension Array Class
//
//
//   J.R. Gloudemans - 10/18/93
//   Sterling Software
//
//******************************************************************************


#ifndef ARRAY_2D_H
#define ARRAY_2D_H

#include <iostream>
using namespace std;

template<class Item_type>

class array_2d
{
private:

        int dim1;
        int dim2;
        Item_type** array;

        void allocate_space();
        void clear_space();

public:

		array_2d();
	    array_2d( int d1, int d2 );
		array_2d( const array_2d<Item_type>& );
		~array_2d();
        void init( int d1, int d2 );
        int dimen_1()			{ return(dim1); }
        int dimen_2()			{ return(dim2); }
        Item_type& operator () (int ind1, int ind2) const;
		array_2d<Item_type>& operator= ( const array_2d<Item_type>& );


};



template<class Item_type>

//==== Clear and Delete all Elements in List ====//
array_2d<Item_type>::array_2d( )
{
	dim1 = dim2 = 1;
	allocate_space();
}

template<class Item_type>

//==== Clear and Delete all Elements in List ====//
array_2d<Item_type>::array_2d( int d1, int d2 )
{

	if ( d1 >= 0 && d2 >= 0)
          {
	    dim1 = d1;
	    dim2 = d2;
	    allocate_space();
	  }
       	else
	  {
	    cout << "ERROR - array_2d init bounds prob" << endl;
          }
}

template<class Item_type>

//==== Clear and Delete all Elements in List and Assign New Values ====//
array_2d<Item_type>::array_2d(const array_2d<Item_type>& array_in)
  : dim1(array_in.dim1), dim2(array_in.dim2)
{
  // allocate space
  allocate_space();

  // set values
  for (int j=0; j<dim2; ++j)
    for (int i=0; i<dim1; ++i)
      array[i][j]=array_in.array[i][j];
}

template<class Item_type>

//==== Clear and Delete all Elements in List ====//
array_2d<Item_type>::~array_2d()
{
	clear_space();
}

template<class Item_type>

//==== Allocate Memory ====//
void array_2d<Item_type>::allocate_space()
{
	array = new Item_type* [dim1];
	for (int i = 0 ; i < dim1 ; i++)
	  {
            array[i] = new Item_type [dim2];
          }
}

template<class Item_type>

//==== Allocate Memory ====//
void array_2d<Item_type>::clear_space()
{
	if ( dim1 >= 0 && dim2 >= 0)
          {
	    for (int i = 0 ; i < dim1 ; i++)
	      {
            	delete [] array[i];
              }
            delete [] array;
	    dim1 = dim2 = 0;
	  }
}

template<class Item_type>

//==== Allocate Memory ====//
void array_2d<Item_type>::init( int d1, int d2 )
{
	if ( d1 == dim1 && d2 == dim2 )
		return;

    clear_space();
	if ( d1 >= 0 && d2 >= 0)
	{
	    dim1 = d1;
	    dim2 = d2;
	    allocate_space();
	}
    else
	{
	    cout << "ERROR - array_2d init bounds prob" << endl;
    }
}

template<class Item_type>

//==== Allocate Memory ====//
Item_type& array_2d<Item_type>::operator()( int ind1, int ind2 ) const
{
        if ( ind1 >= 0 && ind1 < dim1 && ind2 >= 0 && ind2 < dim2)
	  return( array[ind1][ind2] );
        else
          {
            cout << "ERROR - array_2d access out of bounds" << endl;
            return( array[0][0] );
          }
}

template<class Item_type>

//==== Assignment Operator ====//
array_2d<Item_type>&
array_2d<Item_type>::operator=(const array_2d<Item_type>& array_in)
{
  // only do this if not same instance
  if (&array_in!=this)
  {
    // resize if needed
    int d1(array_in.dim1), d2(array_in.dim2);
    init(d1, d2);

    // set values
    for (int j=0; j<dim2; ++j)
      for (int i=0; i<dim1; ++i)
        array[i][j]=array_in.array[i][j];
  }

  return (*this);
}
#endif
