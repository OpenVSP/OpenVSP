//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//----------------------------------------------------------------------------
//
//   Single Dimension Array Class
//
//
//   J.R. Gloudemans - 10/18/93
//   Sterling Software
//
//----------------------------------------------------------------------------
#ifndef SDYN_ARRAY_H
#define SDYN_ARRAY_H

#include <iostream>
using namespace std;

template<class Item_type>

class sdyn_array
{
private:

  int chunk_size;
  int total_size;
  int dim;
  Item_type* arr;

  void allocate_space();
  void clear_space();
  void qs_inc(int left, int right);
  void qs_dec(int left, int right);

public:
  // constructors and destructor
  sdyn_array( void );
  sdyn_array( int d1 );
  sdyn_array( const sdyn_array& ); // copy constructor
  ~sdyn_array();

  // initialization methods
  void set_chunk_size( int size_in )          { chunk_size = size_in; }
  void init( int d1 );
  void set_all_to(const Item_type& value);

  // size accessor methods
  int dimension() const                       { return(dim); }
  int num_items() const                       { return(dim); }

  // element access operators
  Item_type& operator [] (int ind1) const;
  Item_type& operator () (int ind1) const;
  sdyn_array<Item_type>& operator=(const sdyn_array<Item_type>&);

  // element addition methods
  void append(const Item_type& in_item);
  int del_index(int ind);
  void insert_after_index(const Item_type& in_item, int ind);

  // utility methods
  int find_index(const Item_type& value);
};


template<class Item_type>
//==== Default Constructor ====//
sdyn_array<Item_type>::sdyn_array() :
  chunk_size( 8 ),
  total_size( 0 ),
  dim( 0 ),
  arr( 0 )
{
}

template<class Item_type>
//==== Copy Constructor ====//
sdyn_array<Item_type>::sdyn_array( const sdyn_array<Item_type>& sd ) :
  chunk_size( sd.chunk_size ),
  total_size( sd.total_size ),
  dim( sd.dim )
{
  // allocate space
  allocate_space();

  // set values
  for (int i=0; i<dim; ++i)
    arr[i]=sd[i];
}

template<class Item_type>
//==== Dimension Constructor ====//
sdyn_array<Item_type>::sdyn_array( int d1 ) :
    chunk_size( 8 ),
    total_size( 0 ),
    dim( 0 )
{

  if ( d1 >= 0 )
    {
      chunk_size = 8;
      dim = d1;
      allocate_space();
    }
  else
    {
      cout << "ERROR - sdyn_array init bounds prob" << endl;
    }
}

template<class Item_type>
//==== Destructor ====//
sdyn_array<Item_type>::~sdyn_array()
{
  clear_space();
}

template<class Item_type>

//==== Allocate Memory ====//
void sdyn_array<Item_type>::allocate_space()
{
  int size_mult = dim/chunk_size + 1;
  total_size = (dim/chunk_size + 1)*chunk_size;
  arr = new Item_type [total_size];
}

template<class Item_type>

//==== Allocate Memory ====//
void sdyn_array<Item_type>::clear_space()
{
  if ( dim > 0 )
    {
      delete [] arr;
    }
}

template<class Item_type>

//==== Initialize Memory ====//
void
sdyn_array<Item_type>::init( int d1 ) {
  //==== Dont Clear Space If Change Is Within Chunk Size Limits ====//
  if (d1 <= total_size && d1 > total_size-chunk_size)
    {
      dim = d1;
      return;
    }
  clear_space();
  if ( d1 >= 0 )
    {
      dim = d1;
      allocate_space();
    }
  else
    {
      cout << "ERROR - sdyn_array init bounds prob" << endl;
    }
}

template<class Item_type>

//==== Access Operator ====//
Item_type& sdyn_array<Item_type>::operator()( int ind1 ) const
{
        if ( ind1 >= 0 && ind1 < dim )
          return( arr[ind1] );
        else
          {
            cout << "ERROR - sdyn_array access out of bounds" << endl;
          }
		return arr[0];
}

template<class Item_type>

//==== Access Operator ====//
Item_type&
sdyn_array<Item_type>::operator[]( int ind1 ) const {
  if ( ind1 >= 0 && ind1 < dim )
    return( arr[ind1] );

  cout << "ERROR - sdyn_array access out of bounds" << endl;
  return arr[0];
}

template<class Item_type>
//==== Assignment Operator ====//
sdyn_array<Item_type>& sdyn_array<Item_type>::operator=( const sdyn_array<Item_type>& sd )
{
  // only do this if not same instance
  if (&sd!=this)
  {
    // resize if needed
    chunk_size=sd.chunk_size;
    init(sd.dimension());

    // set values
    for (int i=0; i<dim; ++i)
      arr[i]=sd[i];
  }

  return (*this);
}

template<class Item_type>
//==== Append Item To End Of List ====//
void sdyn_array<Item_type>::append(const Item_type& in_item)
{
  dim++;
  if ( dim >= total_size ) {
    Item_type* old_arr = arr;
    allocate_space();
    if (old_arr)  {
      memcpy(arr, old_arr, (dim-1)*sizeof(Item_type));
      delete [] old_arr;
    }
  }
  arr[dim-1] = in_item;
}

template<class Item_type>
//====  Delete Item at Index ====//
int
sdyn_array<Item_type>::del_index(int ind) {
  if ( ind < 0 || ind >= dim )
    return(0);

  for ( int i = ind ; i < dim-1 ; i++ ) {
    arr[i] = arr[i+1];
  }

  dim--;
  return(1);
}

template<class Item_type>

//====  Insert Item at After Index ====//
void
sdyn_array<Item_type>::insert_after_index(const Item_type& in_item,
                                         int ind) {
  dim++;
  if ( dim >= total_size )
    {
      Item_type* old_arr = arr;
      allocate_space();
      if (old_arr)
        {
          memcpy(arr, old_arr, (dim-1)*sizeof(Item_type));
          delete [] old_arr;
        }
    }

  int start_ind;
  if ( ind < 0 )
    start_ind = 0;
  else if ( ind >= dim )
    start_ind = dim-1;
  else
    start_ind = ind+1;

  for ( int i = dim-1 ; i > start_ind ; i--)
    {
      arr[i] = arr[i-1];
    }
  arr[start_ind] = in_item;

}
template<class Item_type>
//==== Set All Elements To Value ===//
void
sdyn_array<Item_type>::set_all_to(const Item_type& value) {
  for (int i = 0; i < dim; i++)
    arr[i] = value;
}

#endif
