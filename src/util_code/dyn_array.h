//******************************************************************************
//    
//   Single Dimension Array Class
//  
// 
//   J.R. Gloudemans - 10/18/93
//   Sterling Software
//
//******************************************************************************


#ifndef DYN_ARRAY_H
#define DYN_ARRAY_H

#include <iostream>
using namespace std;

#include <assert.h>
#include<string.h>
#include<float.h> // for DBL_EPSILON

template<class Item_type>

class dyn_array
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

		dyn_array();
        dyn_array( int d1 );
		dyn_array( const dyn_array<Item_type>&);
		~dyn_array();
        void set_chunk_size( int size_in )	{ chunk_size = size_in; }
        void init( int d1 );
        void set_all_to(const Item_type& value);
        int dimension()			{ return(dim); }
        int num_items()			{ return(dim); }
        Item_type& operator [] (int ind1);
        Item_type& operator () (int ind1);
        void append(const Item_type& in_item);
        int del(const Item_type& in_item);
        int del_index(int ind);
        int find_index(const Item_type& value);
        void insert_after_index(const Item_type& in_item, int ind);
        int find_interval(const Item_type& value);
        int find_interval(const Item_type& value, int guess);
        float interpolate(const Item_type& value, int interval);
        void quicksort_increasing();
        void quicksort_decreasing();
		dyn_array<Item_type>& operator= ( const dyn_array<Item_type>& );
        
        
};

template<class Item_type>

//==== Clear and Delete all Elements in List ====//
dyn_array<Item_type>::dyn_array( )
{
        chunk_size = 8;
        total_size = 0;
        dim = 0;
        arr = 0;
}

template<class Item_type>

//==== Clear and Delete all Elements in List ====//
dyn_array<Item_type>::dyn_array( int d1 )
{

	if ( d1 >= 0 )
          {
            chunk_size = 8;
	    dim = d1;
	    allocate_space();
	  }
       	else
	  {
	    cout << "ERROR - dyn_array init bounds prob" << endl;
          }
}

template<class Item_type>

//==== Clear and Delete all Elements in List and Assign new Values ====//
dyn_array<Item_type>::dyn_array( const dyn_array<Item_type>& array_in )
{
// TODO
}

template<class Item_type>

//==== Clear and Delete all Elements in List ====//
dyn_array<Item_type>::~dyn_array()
{
	clear_space();          
}
            
template<class Item_type>

//==== Allocate Memory ====//
void dyn_array<Item_type>::allocate_space()
{
        total_size = (dim/chunk_size + 1)*chunk_size;
	arr = new Item_type [total_size];
}
            
template<class Item_type>

//==== Allocate Memory ====//
void dyn_array<Item_type>::clear_space()
{
	if ( dim > 0 )
          {
            delete [] arr;
	  }
}

template<class Item_type>

//==== Initialize Memory ====//
void dyn_array<Item_type>::init( int d1 )
{
        //==== Dont Clear Space If Change Is Within Chunk Size Limits ====//
        if (d1 <= total_size /*&& d1 > (total_size-chunk_size)*/ )
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
	    cout << "ERROR - dyn_array init bounds prob" << endl;
          }
}

template<class Item_type>

//==== Access Operator ====//
Item_type& dyn_array<Item_type>::operator()( int ind1 )
{
        if ( ind1 >= 0 && ind1 < dim )
	  return( arr[ind1] );
        else
          {
            cout << "ERROR - dyn_array access out of bounds" << endl;
          }
        return( arr[0] );
}

template<class Item_type>

//==== Access Operator ====//
Item_type& dyn_array<Item_type>::operator[]( int ind1 )
{
	assert( ind1 >= 0 && ind1 < dim );
        if ( ind1 >= 0 && ind1 < dim )
	  return( arr[ind1] );
        else
          {
            cout << "ERROR - dyn_array access out of bounds" << endl;
          }
        return( arr[0] );
}

template<class Item_type>

//==== Append Item To End Of List ====//
void dyn_array<Item_type>::append(const Item_type& in_item)
{
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
  arr[dim-1] = in_item;

}


template<class Item_type>

//==== Find Item And Delete ====//
int dyn_array<Item_type>::del(const Item_type& in_item)
{
  if ( dim <= 0 ) return(0);

  int cnt = 0;
  int del_ind = -1;
 
  while ( del_ind  == -1 && cnt < dim )
    {
      if ( arr[cnt] == in_item )
        {
          del_ind = cnt;
        }
      cnt++;
    }

  if ( del_ind != -1 )
    {
      for ( int i = del_ind ; i < dim-1 ; i++ )
        {
           arr[i] = arr[i+1];
        }
      dim--;
      return(1);
    }

  return(0);             
}

template<class Item_type>

//====  Delete Item at Index ====//
int dyn_array<Item_type>::del_index(int ind)
{
  if ( ind < 0 || ind >= dim ) return(0);
       
  for ( int i = ind ; i < dim-1 ; i++ )
    {
      arr[i] = arr[i+1];
    }

  dim--;
  return(1);
}

template<class Item_type>

//==== Find Index of Array ===//
int dyn_array<Item_type>::find_index(const Item_type& value)
{
  if (dim <= 0) return(0);

  for ( int i = 0 ; i < dim ; i++)
    {
       if ( value == arr[i] ) return(i);
    }

  cout << "ERROR - dyn_array index found for value " << value << endl;
  return(0);
}

template<class Item_type>

//====  Insert Item at After Index ====//
void dyn_array<Item_type>::insert_after_index(const Item_type& in_item, int ind)
{
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

//==== Find Interval for Single Valued Increasing or Decreasing Array ===//
int dyn_array<Item_type>::find_interval(const Item_type& value)
{
  if (dim <= 0) return(0);

  //==== Check If Array Increasing Or Decreasing ====//
  if ( arr[0] < arr[dim-1] )
    {
      for ( int i = 1 ; i < dim ; i++)
        {
          if ( value < arr[i] ) return(i-1);
        }
    }
  else
    {
      for ( int i = 1 ; i < dim ; i++)
        {
          if ( value > arr[i] ) return(i-1);
        }
    }
  return(dim-2);
}


template<class Item_type>

//==== Find Interval for Single Valued Increasing or Decreasing Array ===//
float dyn_array<Item_type>::interpolate(const Item_type& value, int interval)
{
  if (dim <= 0) return(0);

  if ( interval < 0 ) return(0.0); 
  if ( interval > dim - 2 ) return(1.0);

  Item_type denom = arr[interval+1] - arr[interval];

  //if (denom == 0.0) return(0.0);
  if (denom <= DBL_EPSILON && denom >= 0.0) return(0.0);

  return( (float)(value - arr[interval])/denom);

}

template<class Item_type>

//==== Find Interval for Single Valued Increasing or Decreasing Array ===//
int dyn_array<Item_type>::find_interval(const Item_type& value, int guess)
{
  return(0);


}

template<class Item_type>


//==== Quicksort - Increasing Order ===//
void dyn_array<Item_type>::quicksort_increasing()
{
  qs_inc(0,dim-1);

}

template<class Item_type>


//==== Quicksort - Increasing Order ===//
void dyn_array<Item_type>::quicksort_decreasing()
{
  qs_dec(0,dim-1);
}

template<class Item_type>

//==== Quicksort - Increasing Order ===//
void dyn_array<Item_type>::qs_inc(int left, int right)
{
  if ( left < right )
    {
      Item_type temp;
      int j = left;
      int k = right + 1;
      while ( j <= k )
        {
          j++; while ( arr[j] < arr[left] ) j++;
          k--; while ( arr[k] > arr[left] ) k--;
    
          if ( j < k )
            {
              temp = arr[j];
              arr[j] = arr[k];
              arr[k] = temp;
            }
        }

      temp = arr[left];
      arr[left] = arr[k];
      arr[k] = temp;

      qs_inc(left, k-1);
      qs_inc(k+1, right);
    }
}

template<class Item_type>

//==== Quicksort - Decreasing Order ===//
void dyn_array<Item_type>::qs_dec(int left, int right)
{
  if ( left < right )
    {
      Item_type temp;
      int j = left;
      int k = right + 1;
      while ( j <= k )
        {
          j++; while ( arr[j] > arr[left] ) j++;
          k--; while ( arr[k] < arr[left] ) k--;
    
          if ( j < k )
            {
              temp = arr[j];
              arr[j] = arr[k];
              arr[k] = temp;
            }
        }

      temp = arr[left];
      arr[left] = arr[k];
      arr[k] = temp;

      qs_dec(left, k-1);
      qs_dec(k+1, right);
    }
}

template<class Item_type>

//==== Set All Elements To Value ===//
void dyn_array<Item_type>::set_all_to(const Item_type& value)
{
  for (int i = 0; i < dim; i++)
    arr[i] = value;
}

template<class Item_type>

//==== Assignment operator ====//
dyn_array<Item_type>&
dyn_array<Item_type>::operator= (const dyn_array<Item_type>& array_in)
{
// TODO
}

#endif
