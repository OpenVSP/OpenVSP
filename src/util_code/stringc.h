//
// This file is released under the terms of the NASA Open Source Agreement (NOSA)
// version 1.3 as detailed in the LICENSE file which accompanies this software.
//

//********************************
// String Class
//
// Cathy D. Roberts
// Sterling Software
//
//********************************

#ifndef STRINGC_H
#define STRINGC_H

#include <iostream>
using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defines.h"

#define STRINGC_CHUNK_SIZE (64)

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

class Stringc
{
private:

   int num_chars;
   int total_size;
   int chunk_size;
   char* char_array;

   void make_space();
   void clear_space();
   void append(char ch);
   void prepend(char ch);
   void rdelete(int istart, int iend);

public:

// Constructors

   VSPDLL Stringc();
   Stringc(int ichar);
   Stringc(const char ch);
   VSPDLL Stringc(const char *ch_array);
   VSPDLL Stringc(const Stringc& str);

// Destructor

   VSPDLL ~Stringc();

// Member functions

   void print();                            // Print out string info
   int get_length()                            {return(num_chars);}
   void set_chunk_size(int size)               {chunk_size = size;}

   int count_words();                       // Count the words in a string
   Stringc get_word(int iword);             // Get a word from a string
   void delete_word(int iword);             // Delete a word from a string

   int count_lines();                       // Count the lines in a string
   Stringc get_line(int iline);             // Get a line from a string
   void delete_line(int iline);             // Delete a line from a string

   void get_rid_of_comments();              // Deletes all characters after comment
   void underscore_to_space();              // Change '_' to ' ' in string
   void space_to_underscore();              // Change ' ' to '_' in string
   void change_from_to( const char f, const char t);

   void concatenate(const char  ch);        // Add a single character to string
   VSPDLL void concatenate(const char* cstring);   // Add a char* to end of string
   void concatenate(const Stringc cstring); // Add a string to end of string

   VSPDLL int search_for_substring(const char* substring);  // find location of substring
   int search_for_substring(char character);         // find location of character
   int matchwild(Stringc wildcardstring);			     // wildcard matching (using *?)

   void insert_string_at (int position, const char *cstring);   // insert a *char
   void insert_string_at (int position, const char character);  // insert a character
   void insert_string_at (int position, const Stringc cstring); // insert a string

   VSPDLL void delete_range (int istart, int iend);    
   VSPDLL Stringc get_range (int istart, int iend);    
   void overwrite_at_position(int position, const char *cstring);  

   void remove_leading_blanks();             
   void remove_trailing_blanks();            
   void trim_blanks()				{ remove_leading_blanks(); remove_trailing_blanks(); }
   void remove_leading(char c);
   void remove_trailing(char c);
   void trim(char c)				{ remove_leading(c); remove_trailing(c); }
   void quote(char c)				{ prepend(c); append(c); }

   int count_substrings(const char *substring);  
   void remove_substring(const char *substring, int istring);  
   void remove_substring(char character, int ichar);  
   void remove_all_substrings(const char *substring);          
   void remove_all_substrings(char character);          

   double convert_to_double();
   float convert_to_float()                    {return((float)atof(char_array));}
   int convert_to_integer()                    {return(atoi(char_array));}

   int is_double( double* ret );

// Operators

// These functions convert the string to a character pointer.  They
// are usefull when passing the object to a function that takes a 
// character pointer as an argument

   operator char* ()                           {return(char_array);}
   char* operator() ()                         {return(char_array);}
   //operator const char* ()                     {return(char_array);}
   operator const char* () const               {return(char_array);}

// String Array access functions

   char* get_char_star()                       {return(char_array);}
   char& operator[] (int index)                {return(char_array[index]);}
   const char& operator[] ( int index ) const  {return(char_array[index]);}

// Equality functions

   Stringc& operator=( const char ch );
   VSPDLL Stringc& operator=( const Stringc& cstring );
   VSPDLL Stringc& operator=( const char *cstring );

// String comparison functions 

   int operator==( const Stringc& cstring ) const;
   int operator==( const char *cstring ) const;
   int operator!=( const Stringc& cstring ) const;
   int operator!=( const char *cstring ) const;

// Output function

//   friend ostream & operator<<( ostream &os, const Stringc &stringc );
   
};

#endif
