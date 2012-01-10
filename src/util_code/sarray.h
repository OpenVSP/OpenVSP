//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//-------------------------------------------------------------------
//
//   Single Dimension Array Class
//
//
//   J.R. Gloudemans - 10/18/93
//   Sterling Software
//
//--------------------------------------------------------------------
#ifndef SARRAY_H
#define SARRAY_H

#include <iostream>
using namespace std;

#include <assert.h>
#include <memory.h>

// Define Error Flags //
#define BELOW_BOUNDS    0
#define ABOVE_BOUNDS    1


template<class Item_type>

class struct_array {
private:

  int dim;
  Item_type* arr;

  void allocate_space();
  void clear_space();
  void print_error_message(int error_flag) const;

public:

  struct_array();
  struct_array( const struct_array<Item_type>& );
  struct_array( int d1 );
  ~struct_array();
  void init( int d1 );
  int dimension() const  { return(dim); }
  Item_type& operator [] (int ind1) const;
  Item_type& operator () (int ind1) const;
  struct_array<Item_type>& operator=( const struct_array<Item_type>& );

};

//==== Default Constructor ====//
template<class Item_type>
struct_array<Item_type>::struct_array( ) {
  dim = 0;
}

//==== Copy Constructor ====//
template<class Item_type>
struct_array<Item_type>::struct_array( const struct_array<Item_type>& s ) {
// TODO
}

//==== Dimension Constructor ====//
template<class Item_type>
struct_array<Item_type>::struct_array( int d1 ) {

  if ( d1 >= 0 ) {
    dim = d1;
    allocate_space();
  } else {
    cout << "ERROR - struct_array init bounds prob" << endl;
  }
}

//==== Destructor ====//
template<class Item_type>
struct_array<Item_type>::~struct_array() {
  clear_space();
}

template<class Item_type>

//==== Allocate Memory ====//
void
struct_array<Item_type>::allocate_space() {
  arr = new Item_type [dim];
}

template<class Item_type>

//==== Clear Space ====//
void
struct_array<Item_type>::clear_space() {
  if ( dim > 0 )
  {
    delete [] arr;
    dim = 0;
  }
}

template<class Item_type>
//==== Initialize Memory ====//
void
struct_array<Item_type>::init( int d1 ) {
  if (d1 == dim) return;

  clear_space();
  if ( d1 >= 0 ) {
    dim = d1;
    allocate_space();
  } else {
    cout << "ERROR - struct_array init bounds prob" << endl;
  }
}
template<class Item_type>
//==== Access Operator ====//
Item_type&
struct_array<Item_type>::operator()( int ind1 ) const {
  if ( ind1 >= 0 && ind1 < dim )
    return( arr[ind1] );
  else
    {
      cout << "ERROR - struct_array access out of bounds" << endl;
    }
  return ( arr[0] );
}
template<class Item_type>
//==== Access Operator ====//
Item_type&
struct_array<Item_type>::operator[]( int ind1 ) const {
  if ( ind1 < 0 ) {
    ind1 = 0;
    print_error_message(BELOW_BOUNDS);
  }
  else if ( ind1 > dim-1 ) {
    ind1 = dim-1;
    print_error_message(ABOVE_BOUNDS);
  }
  return( arr[ind1] );
}

template<class Item_type>
//==== Assignment Operator ====//
struct_array<Item_type>&
struct_array<Item_type>::operator=( const struct_array<Item_type>& s )
{
// TODO
}		  

template<class Item_type>
//==== Print Error Message ====//
void
struct_array<Item_type>::print_error_message(int error_flag) const {
  switch(error_flag) {
    case BELOW_BOUNDS:
      cout << "ERROR - struct_array access below bounds" << endl;
      break;

    case ABOVE_BOUNDS:
      cout << "ERROR - struct_array access above bounds" << endl;
      assert(0);
      break;
  }

}

#endif
