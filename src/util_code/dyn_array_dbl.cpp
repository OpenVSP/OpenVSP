//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//******************************************************************************
//    
//   Single Dimension Array Class of Doubles
//  
// 
//   J.R. Gloudemans, Paul C. Davis - 10/17/94
//   Sterling Software
//
//******************************************************************************

#include <memory.h>
#include "dyn_array_dbl.h"

#include <math.h>
#include <float.h> //For DBL_EPSILON

//==== Default Constructor ====//
Dyn_array_dbl::Dyn_array_dbl( )
{
        chunk_size = 8;
        total_size = 0;
        dim = 0;
        arr = 0;
}

//==== Copy Constructor ====//
Dyn_array_dbl::Dyn_array_dbl( const Dyn_array_dbl& d )
{
// TODO
}

//==== Dimension Constructor ====//
Dyn_array_dbl::Dyn_array_dbl( int d1 )
{

	if ( d1 >= 0 )
          {
            chunk_size = 8;
	    dim = d1;
	    allocate_space();
	  }
       	else
	  {
	    print_error_message(ERR_INIT);
          }
}

//==== Destructor ====//
Dyn_array_dbl::~Dyn_array_dbl()
{
	clear_space();          
}
            
//==== Assignment Operator ====//
Dyn_array_dbl& Dyn_array_dbl::operator= (const Dyn_array_dbl& array_in)
{
// TODO
}

//==== Allocate Memory ====//
void Dyn_array_dbl::allocate_space()
{
        total_size = (dim/chunk_size + 1)*chunk_size;
	arr = new double [total_size];
}

//==== Initialize Memory ====//
void Dyn_array_dbl::init( int d1 )
{
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
	    print_error_message(ERR_INIT);
          }
}

//==== Append Item To End Of List ====//
void Dyn_array_dbl::append(double in_item)
{
  dim++;
  if ( dim >= total_size )
    {
      double* old_arr = arr;
      allocate_space();
      if (old_arr) 
        {
          memcpy(arr, old_arr, (dim-1)*sizeof(double));
          delete [] old_arr;
        }
    }
  arr[dim-1] = in_item;

}

//==== Find Item And Delete ====//
int Dyn_array_dbl::del(double in_item)
{
  if ( dim <= 0 ) return(0);

  int cnt = 0;
  int del_ind = -1;
 
  while ( del_ind  == -1 && cnt < dim )
    {
      //if ( arr[cnt] == in_item )
	  if( fabs(arr[cnt] - in_item) <= DBL_EPSILON)
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

//====  Delete Item at Index ====//
int Dyn_array_dbl::del_index(int ind)
{
  if ( ind < 0 || ind >= dim ) return(0);
       
  for ( int i = ind ; i < dim-1 ; i++ )
    {
      arr[i] = arr[i+1];
    }

  dim--;
  return(1);
}

//==== Find Index of Array ===//
int Dyn_array_dbl::find_index(double value)
{
  if (dim <= 0) return(0);

  for ( int i = 0 ; i < dim ; i++)
    {
       //if ( value == arr[i] ) return(i);
	  if( fabs(value - arr[i]) <= DBL_EPSILON ) return(i);
    }

  print_error_message(ERR_NO_VALUE);
  return(0);
}

//==== Find Interval for Single Valued Increasing or Decreasing Array ===//
//===== Returns -1 or dim if out of range =====//
int Dyn_array_dbl::find_interval(double value)
{
  if (dim <= 1) return(-1);

  //==== Increasing Array ====//
  if ( arr[0] < arr[dim-1] )
    {
      if ( value < arr[0] ) return(-1);

      for ( int i = 1 ; i < dim ; i++)
        {
          if ( value < arr[i] ) return(i-1);
        }
      return(dim);
    }

  //==== Decreasing Array ====//
  else
    {
      if ( value > arr[0] ) return(-1);

      for ( int i = 1 ; i < dim ; i++)
        {
          if ( value > arr[i] ) return(i-1);
        }
     return(dim);
    }
}

//==== Find Interval for Single Valued Increasing or Decreasing Array With Guess ===//
//===== Returns -1 or dim if out of range =====//
int Dyn_array_dbl::find_interval(double value, int guess)
{
  if (dim <= 1) return(-1);

  if (guess < 0) guess = 0;
  if (guess >= dim-1) guess = dim-2;

  //==== Increasing Array ====//
  if ( arr[0] < arr[dim-1] )
    {
      //===== Guess Is at Lower Limit =====
      if ( guess <= 1 )
        {
          //===== Value is Below Range =====
          if ( guess == 0 && value < arr[guess]) return(-1);
          if ( guess == 1 && value < arr[guess-1]) return(-1);

          for ( int i = guess ; i < dim ; i++)
            {
              if ( value < arr[i] ) return(i-1);
            }
        }

      else
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

      //===== Value is Above Range =====
      return(dim);
    }

  //==== Decreasing Array ====//
  else
    {
      //===== Guess Is at Lower Limit =====
      if ( guess <= 1 )
        {
           //===== Value is Below Range =====
           if ( guess == 0 && value > arr[guess]) return(-1);
           if ( guess == 1 && value > arr[guess-1]) return(-1);

            for ( int i = guess ; i < dim ; i++)
              {
                if ( value > arr[i] ) return(i-1);
              }

        }

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

      //===== Value Is Above Range =====
      return(dim);
    }
}

//==== Interpolate ===//
double Dyn_array_dbl::interpolate(double value, int interval)
{
  if (dim <= 1) return(0.0);

  if ( interval < 0 ) return(0.0); 
  if ( interval > dim - 2 ) return(1.0);

  double denom = arr[interval+1] - arr[interval];

  //if (denom == 0.0) return(0.0);
  if( denom <= DBL_EPSILON && denom >= 0.0 ) return(0.0);

  return( (double)(value - arr[interval])/denom);

}

//==== Find The Minimum Value in the Array ===//
double Dyn_array_dbl::find_min_value()
{
  if ( dim <= 0 ) return(0.0);

  double min = arr[0];

  for (int i = 1; i < dim; i++)
  {
    if ( arr[i] < min )
      min = arr[i];
  }

  return(min);
}

//==== Find The Maximum Value in the Array ===//
double Dyn_array_dbl::find_max_value()
{
  if ( dim <= 0 ) return(0.0);

  double max = arr[0];

  for (int i = 1; i < dim; i++)
  {
    if ( arr[i] > max )
      max = arr[i];
  }

  return(max);
}

//==== Print Error Message ====//
void Dyn_array_dbl::print_error_message(int error_flag)
{
  switch(error_flag)
  {
    case ERR_INIT:
      cout << "ERROR - Dyn_array_dbl, init bounds prob" << endl;
      break;

    case ERR_NO_VALUE:
      cout << "ERROR - Dyn_array_dbl, value not found" << endl;
      break;

    case ERR_INTERP_ARRAY_DIM:
      cout << "ERROR - Dyn_array_dbl, interpolation of different sized arrays" << endl;
      break;

    case ERR_INTERP_ARRAY_INDEX:
      cout << "ERROR - Dyn_array_dbl, array interpolation index out of bounds" << endl;
      break;

	default:
	   cout << "UNKOWN ERROR" << endl;
	   break;
  }

}

//===== Interpolate Array =====//
//===== Interpolates value from another equally sized array based on an index =====//
//===== corresponding to the value given for the current array =====//
//===== Assumes monotonic increasing or decreasing arrays =====//
double Dyn_array_dbl::interpolate_array(Dyn_array_dbl* array_ptr, double value)
{
  //===== Check if Dimensions Are The Same =====//
  if (dim != array_ptr->dimension()) 
  {
    print_error_message(ERR_INTERP_ARRAY_DIM);
    return(0.0);
  }

  int index = find_interval(value);

  if ( index < 0 )
  {
    print_error_message(ERR_INTERP_ARRAY_INDEX);
    return(array_ptr->get(0));
  }
  else if ( index > dim - 2 )
  {
    print_error_message(ERR_INTERP_ARRAY_INDEX);
    return(array_ptr->get(dim-1));
  }
  else
  {
    double fract = interpolate(value,index);

    double array_value = array_ptr->get(index) + fract*(array_ptr->get(index+1) - array_ptr->get(index));

    return(array_value);
  }
}

