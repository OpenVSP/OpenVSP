//******************************************************************************
//    
//   Single Dimension Array Class
//  
// 
//   J.R. Gloudemans - 10/18/93
//   Sterling Software
//
//******************************************************************************


#ifndef ARRAY_H
#define ARRAY_H

#include <iostream>
using namespace std;

// Define Error Flags //
#define BELOW_BOUNDS	0
#define ABOVE_BOUNDS	1


template<class Item_type>

class array
{
private:

    int dim;
    Item_type* arr;

    void allocate_space();
    void clear_space();
    void print_error_message(int error_flag);

public:

	array();
    array( int d1 );
    array( const array<Item_type>& ); 
	~array();
    
    void init( int d1 );
    int dimension()	const		{ return(dim); }
    
    Item_type& operator [] (int ind1);
    Item_type& operator () (int ind1);
	array<Item_type>& operator= ( const array<Item_type>& );
            
    int find_interval(const Item_type& value) const;
    int find_interval(const Item_type& value, int guess) const;
    double interpolate(const Item_type& value, int interval) const;
        
};

template<class Item_type>

//==== Default constructor ====//
array<Item_type>::array( ) : dim(0)
{
}

template<class Item_type>

//==== Dimension contructor ====//
array<Item_type>::array( int d1 )
{

	if ( d1 >= 0 )
    {
	    dim = d1;
	    allocate_space();
    }
    else
    {
	    cout << "ERROR - array init bounds prob" << endl;
    }
}

template<class Item_type>

//===== Copy constructor =====//
array<Item_type>::array(const array<Item_type>& array_in)
{
// TODO

}

template<class Item_type>

//==== Clear and Delete all Elements in List ====//
array<Item_type>::~array()
{
	clear_space();          
}
            
template<class Item_type>

//==== Allocate Memory ====//
void array<Item_type>::allocate_space()
{
	arr = new Item_type [dim];
}
            
template<class Item_type>

//==== Clear Space ====//
void array<Item_type>::clear_space()
{
	if ( dim > 0 )
    {
        delete [] arr;
    }
}

template<class Item_type>

//==== Initialize Memory ====//
void array<Item_type>::init( int d1 )
{
        if (d1 == dim) return;

        clear_space();
	if ( d1 >= 0 )
          {
	    dim = d1;
	    allocate_space();
	  }
       	else
	  {
	    cout << "ERROR - array init bounds prob" << endl;
          }
}

template<class Item_type>

//==== Access Operator ====//
Item_type& array<Item_type>::operator()( int ind1 )
{
        if ( ind1 >= 0 && ind1 < dim )
	  return( arr[ind1] );
        else
          {
            cout << "ERROR - array access out of bounds" << endl;
          }
        return( arr[0] );
}
/*
template<class Item_type>

//==== Access Operator ====//
Item_type& array<Item_type>::operator[]( int ind1 )
{
        if ( ind1 >= 0 && ind1 < dim )
	  return( arr[ind1] );
        else
          {
            cout << "ERROR - array access out of bounds" << endl;
          }
}
*/ 

template<class Item_type>

//==== Access Operator ====//
Item_type& array<Item_type>::operator[]( int ind1 )
{
        if ( ind1 < 0 )
          {
            ind1 = 0;
            print_error_message(BELOW_BOUNDS);
          }
        else if ( ind1 > dim-1 )
          {
            ind1 = dim-1;
            print_error_message(ABOVE_BOUNDS);
          }

	return( arr[ind1] );
}

template<class Item_type>

//==== Assignment Operator ====//
array<Item_type>& 
array<Item_type>::operator=(const array<Item_type>& array_in)
{
// TODO
}

template<class Item_type>

//==== Find Interval for Single Valued Increasing or Decreasing Array ===//
int array<Item_type>::find_interval(const Item_type& value) const
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

//==== Find Interval for Single Valued Increasing or Decreasing Array With Guess ===//
int array<Item_type>::find_interval(const Item_type& value, int guess) const
{
  if (dim <= 0) return(0);

  if (guess <= 0) guess = 1;
  if (guess > dim-1) guess = dim-1;

  //==== Increasing Array ====//
  if ( arr[0] < arr[dim-1] )
    {

      //==== Guess Overshot Value ====//
      if ( value < arr[guess-1] )
        {
          for ( int i = guess-2 ; i >= 0 ; i--)
            {
              if ( value > arr[i] ) return(i);
            }
        }

      //==== Guess Undershot Value ====//
      else
        {
          for ( int i = guess ; i < dim ; i++)
            {
              if ( value < arr[i] ) return(i-1);
            }
        }
    }

  //==== Decreasing Array ====//
  else
    {

      //==== Guess Overshot Value ====//
      if ( value > arr[guess-1] )
        {
          for ( int i = guess-2 ; i >= 0 ; i--)
            {
              if ( value < arr[i] ) return(i);
            }
        }

      //==== Guess Undershot Value ====//
      else
        {
          for ( int i = guess ; i < dim ; i++)
           {
              if ( value > arr[i] ) return(i-1);
            }
        }
    }
  return(dim-2);
}

template<class Item_type>

//==== Interpolate ===//
double array<Item_type>::interpolate(const Item_type& value, int interval) const
{
  if (dim <= 0) return(0);

  if ( interval < 0 ) return(0.0); 
  if ( interval > dim - 2 ) return(1.0);

  Item_type denom = arr[interval+1] - arr[interval];

  if (denom == 0.0) return(0.0);

  return( (double)(value - arr[interval])/denom);

}
            
template<class Item_type>

//==== Print Error Message ====//
void array<Item_type>::print_error_message(int error_flag)
{
  switch(error_flag)
  {
    case BELOW_BOUNDS:
      cout << "ERROR - array access below bounds" << endl;
      break;

    case ABOVE_BOUNDS:
      cout << "ERROR - array access above bounds" << endl;
      break;
  }

}


#endif
