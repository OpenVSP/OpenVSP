//********************************
// String Class
//
// Cathy D. Roberts
// Sterling Software
//
//********************************

#include "stringc.h"

//-------------------------- Constructors -----------------------------------
//----------------------------------------------------------------------------

Stringc::Stringc()
{
    chunk_size = 16;
    total_size = 0;
    num_chars = 0;
    make_space();
    char_array[num_chars] = '\0';
}

//----------------------------------------------------------------------------

Stringc::Stringc(const char ch)
{
    chunk_size = 16;
    num_chars = 1;
    make_space();
    (*this)[0] = ch;
    (*this)[num_chars] = '\0';
}

//----------------------------------------------------------------------------

Stringc::Stringc(const char *ch_array)
{
    chunk_size = 16;
    if (ch_array)
      num_chars = strlen(ch_array);
    else
      num_chars = 0;
    make_space();
    for (int i=0;i<num_chars;i++)
         (*this)[i] = ch_array[i];
      (*this)[num_chars] = '\0';
}

//----------------------------------------------------------------------------

Stringc::Stringc(int ichar)
{
   chunk_size = 16;
   num_chars = ichar;
   make_space();
   for (int i=0; i<num_chars; i++)
      char_array[i] = ' ';
   char_array[num_chars] = '\0';
}

//----------------------------------------------------------------------------

Stringc::Stringc(const Stringc& str)
{
   num_chars = str.num_chars;
   chunk_size = str.chunk_size;
   make_space();
   for (int i=0; i< num_chars; i++)
      (*this)[i] = str[i];
   (*this)[num_chars] = '\0';
}

//----------------------------- Destructor -----------------------------------
//----------------------------------------------------------------------------

Stringc::~Stringc()
{
   clear_space();
}

//--------------------------- Private functions ------------------------------
//----------------------------------------------------------------------------

void Stringc::make_space()
{
    int size_mult = (num_chars+1)/chunk_size+1;
    total_size = size_mult*chunk_size;
    char_array = new char[total_size];
}

//----------------------------------------------------------------------------

void Stringc::clear_space()
{
//    if ( num_chars >= 0 ) delete [] char_array;
    if ( total_size >= 0 ) delete [] char_array;
}

//----------------------------------------------------------------------------

void Stringc::append(char ch)
{
   num_chars++;
   if ( (num_chars+1) >= total_size )
      {
      char* old_char_array = char_array;
      make_space();
      if ( old_char_array ) 
         {
         memcpy(char_array, old_char_array, num_chars*sizeof(char));
         delete [] old_char_array;
         }
      }
   char_array[num_chars-1] = ch;
   char_array[num_chars] = '\0';
}

//----------------------------------------------------------------------------

void Stringc::prepend(char ch)
{
   num_chars++;
   if ( (num_chars+1) >= total_size )
      {
      char* old_char_array = char_array;
      make_space();
      if ( old_char_array ) 
         {
         memcpy(char_array+1, old_char_array, num_chars*sizeof(char));
         delete [] old_char_array;
         }
      }
   else
   {
	   // shift chars
	   for (int i = num_chars-1; i > 0; i--)
		   char_array[i] = char_array[i-1];
   }
   char_array[0] = ch;
   char_array[num_chars] = '\0';
}

//----------------------------------------------------------------------------

void Stringc::rdelete(int istart, int iend)
{
  int i=0;
   if ((istart > -1)&&(istart < num_chars)&&(iend > -1)&&(iend < num_chars)
        &&(istart <= iend))
      {
      int new_num = num_chars - (iend-istart+1);
      int new_size = ((new_num+1)/chunk_size)*chunk_size;
      if (new_size == total_size)
         {
         for( i=istart; i<new_num; i++)
            char_array[i] = char_array[i+(iend-istart+1)];
         num_chars = new_num;
         char_array[num_chars] = '\0';
         }
      else
         {   
         int new_length = num_chars - (iend-istart+1);
         char *new_char = new char[new_length+1];
         for (i=0; i<istart; i++)
            new_char[i] = char_array[i];
         int j=istart;
         for (i=iend+1; i<num_chars; i++)
            {
            new_char[j] = char_array[i];
            j++;
            }
         clear_space();
         num_chars = new_length;
         make_space();
         for ( i=0; i<num_chars; i++)
            char_array[i] = new_char[i];
         char_array[num_chars] = '\0';
         delete [] new_char;
         }
      }
   else
      {
      cout << "num_chars = " << num_chars << "\n";
      cout << "Error in Stringc::delete routine.\n";
      cout << "   Bad range " << istart << " to " << iend;
      cout << "... Nothing will be deleted.\n";
      }
}

//---------------------------- Public functions ------------------------------
//----------------------------------------------------------------------------

void Stringc::print()
{
   cout << "\n<" << char_array << ">\n";
   cout << "Num_chars: " << num_chars << "\n";
   cout << "Chunk: " << chunk_size << "\n";
   cout << "Total_Size: " << total_size << "\n\n";
}

//----------------------------------------------------------------------------

double Stringc::convert_to_double()
{
   double data=0.0;
   data = atof(char_array);
   return data;
}

//----------------------------------------------------------------------------

int Stringc::count_words()
{
   int i = 0;
   int iword = 0;
   while ( i < num_chars )
      {

//    Step through spaces

      while ( i< num_chars &&
            (char_array[i] == ' ' || char_array[i] == '\t')) i++;

//    Step through the word and increment the word counter, iword.

      if ( i < num_chars )
         {
         while ( i < num_chars && char_array[i] != ' ' 
                               && char_array[i] != '\t') i++;
         iword++;
         }
      }
   return iword;
}

//----------------------------------------------------------------------------

Stringc Stringc::get_word(int iword)
{
   int i = 0;
   int iw = 0;
   int istart = 0;
   int iend = 0;

// Find the start and end location of the desired iword'th word.

   while ( i < num_chars && iw <= iword)
      {

//    Step through spaces

      while ( i< num_chars &&
            (char_array[i] == ' ' || char_array[i] == '\t')) i++;

//    Step through a word and save its start and end location if it
//    is the desired iword.

      if ( i < num_chars )
         {
         if ( iw == iword) istart = i;
         while ( i < num_chars && char_array[i] != ' ' 
                               && char_array[i] != '\t') 
            {
            if ( iw == iword) iend = i;
            i++;
            }
         iw++;
         }
      }

// Extract the word string from the character array if iword exists.

   if ( iword < iw && iword >= 0)
      {
      int word_size = iend-istart+1;
      Stringc sword(word_size);
      for ( i=0; i<word_size; i++)
         {
         sword[i] = char_array[istart+i];
         }
      sword[word_size] = '\0';
      return sword;
      }
   else
      {
      cout << "\nError in Stringc::get_word routine.\n";
      if ( iword < 0 ) 
         cout << "   Negative word index does not make sense.\n";
      else
         cout << "   Word index is greater than the number of words.\n";
      cout << "   Returning an empty string as word\n";
      Stringc sword(1);
      sword[0] = '\0';
      return sword;
      }
}

//----------------------------------------------------------------------------

void Stringc::delete_word(int iword)
{
   int i = 0;
   int iw = 0;
   int istart = 0;
   int iend = 0;

// Find the start and end location of the desired iword word.

   while ( i < num_chars && iw <= iword)
      {

//    Step through spaces

      while ( i< num_chars &&
            (char_array[i] == ' ' || char_array[i] == '\t')) i++;

//    Step through a word and save its start and end location if it
//    is the desired iword.

      if ( i < num_chars )
         {
         if ( iw == iword) istart = i;
         while ( i < num_chars && char_array[i] != ' ' 
                               && char_array[i] != '\t') 
            {
            if ( iw == iword) iend = i;
            i++;
            }
         iw++;
         }
      }

// If iword exists delete it from the character array.

   if ( iword < iw && iword >= 0)
      rdelete(istart,iend);
   else
      {
      cout << "Error in Stringc::delete_word routine.\n";
      if ( iword < 0 ) 
         cout << "   Negative word index does not make sense.\n";
      else
         cout << "   Word index is greater than the number of words.\n";
      }
}

//----------------------------------------------------------------------------

int Stringc::count_lines()
{
   int i = 0;
   int iline = 0;
   while ( i < num_chars )
      {

//    Step through empty lines

      while ( i< num_chars &&
            (char_array[i] == '\n')) i++;

//    Step through the lines and increment the line counter, iline.

      if ( i < num_chars )
         {
         while ( i < num_chars && char_array[i] != '\n') i++;
         iline++;
         }
      }
   return iline;
}

//----------------------------------------------------------------------------

Stringc Stringc::get_line(int iline)
{
   int i = 0;
   int il = 0;
   int istart = 0;
   int iend = 0;

// Find the start and end location of the desired iline'th line.

   while ( i < num_chars && il <= iline)
      {

//    Step through spaces

      while ( i< num_chars && (char_array[i] == '\n')) i++;

//    Step through a line and save its start and end location if it
//    is the desired iline.

      if ( i < num_chars )
         {
         if ( il == iline) istart = i;
         while ( i < num_chars && char_array[i] != '\n' ) 
            {
            if ( il == iline) iend = i;
            i++;
            }
         il++;
         }
      }

// Extract the line string from the character array if iline exists.

   if ( iline < il && iline >= 0)
      {
      int line_size = iend-istart+1;
      Stringc sline(line_size);
      for ( i=0; i<line_size; i++)
         {
         sline[i] = char_array[istart+i];
         }
      sline[line_size] = '\0';
      return sline;
      }
   else
      {
      cout << "\nError in Stringc::get_line routine.\n";
      if ( iline < 0 ) 
         cout << "   Negative line index does not make sense.\n";
      else
         cout << "   Line index is greater than the number of lines.\n";
      cout << "   Returning an empty string as line\n";
      Stringc sline(1);
      sline[0] = '\0';
      return sline;
      }
}

//----------------------------------------------------------------------------

void Stringc::delete_line(int iline)
{
   int i = 0;
   int il = 0;
   int istart = 0;
   int iend = 0;

// Find the start and end location of the desired iline line.

   while ( i < num_chars && il <= iline)
      {

//    Step through spaces

      while ( i< num_chars && (char_array[i] == '\n')) i++;

//    Step through a line and save its start and end location if it
//    is the desired iline.

      if ( i < num_chars )
         {
         if ( il == iline) istart = i;
         while ( i < num_chars && char_array[i] != '\n') 
            {
            if ( il == iline) iend = i;
            i++;
            }
         il++;
         }
      }

// If iline exists delete it from the character array.

   if ( iline < il && iline >= 0)
      rdelete(istart,iend);
   else
      {
      cout << "Error in Stringc::delete_line routine.\n";
      if ( iline< 0 ) 
         cout << "   Negative line index does not make sense.\n";
      else
         cout << "   Line index is greater than the number of lines.\n";
      }
}

//----------------------------------------------------------------------------

void Stringc::get_rid_of_comments()
{
   int iend=-1;
   int i = 0;

// Find location of comment

   while ( iend < 0 && i < num_chars-1 ) 
      {
      if ((char_array[i] == '/' ) && (char_array[i+1] == '/'))
         iend = i;
      i++;
      }

// Delete comment.

   if ( iend >= 0 )
      rdelete(iend,num_chars-1);
}

//----------------------------------------------------------------------------

void Stringc::concatenate(const char ch)
{
    append(ch);
}

//----------------------------------------------------------------------------

void Stringc::concatenate(const char *cstring)
{
    for(int i=0; i < (int)strlen(cstring); i++)
       append(cstring[i]);
}

//----------------------------------------------------------------------------

void Stringc::concatenate(const Stringc cstring)
{
    for(int i=0; i<cstring.num_chars; i++)
       append(cstring[i]);
}

//----------------------------------------------------------------------------

void Stringc::underscore_to_space()
{
    for(int i=0; i<num_chars; i++)
       if (char_array[i] == '_') char_array[i] = ' ';
}

//----------------------------------------------------------------------------

void Stringc::space_to_underscore()
{
    for(int i=0; i<num_chars; i++)
       if (char_array[i] == ' ') char_array[i] = '_';
}

//----------------------------------------------------------------------------

void Stringc::change_from_to( const char f, const char t)
{
    for(int i=0; i<num_chars; i++)
       if (char_array[i] == f) char_array[i] = t;
}


//----------------------------------------------------------------------------

int Stringc::search_for_substring( const char *substring )
   {

// Use strstr to find out if substring is in char_array.

   char *character = strstr( char_array, substring );

// if a match was found, return the element position of the match

   if ( character != NULL )
      {
      int i = 0;
      while ( &char_array[i] != character ) i++;
         return i;
      }

// if no match was found, return minus 1

   return -1;
   }

//----------------------------------------------------------------------------

void Stringc::insert_string_at (int position, const Stringc cstring)
{

// check to see if position to insert string at makes sense

   if (position > num_chars || position < 0)
      {
      cout << "Error in Stringc::insert_string_at routine.\n";
      cout << "  Bad position = " << position << "\n";
      cout << "  Postion should be >= 0 and < " << num_chars+1 <<"\n";
      return;
      }
   int len_char = cstring.num_chars;
   int new_size = num_chars+len_char;
   int num_save = num_chars;
   int i = 0;

// If more room is needed in the char_array for the new string, 
// append spaces.

   for ( i=0; i<len_char; i++) 
      append(' ');

// Shift characters after position to make room for inserted string

   for (i=0; i<num_save-position; i++) {
      char_array[new_size-i-1] = char_array[num_save-i-1];
      }

// Insert string

   for (i=position; i<position+len_char; i++)
      char_array[i] = cstring[i-position];
   num_chars = new_size;
   char_array[num_chars] = '\0';
}    

//----------------------------------------------------------------------------

void Stringc::insert_string_at (int position, const char *cstring)
{

// check to see if position to insert string at makes sense

   if (position > num_chars || position < 0)
      {
      cout << "Error in Stringc::insert_string_at routine.\n";
      cout << "  Bad position = " << position << "\n";
      cout << "  Postion should be >= 0 and < " << num_chars+1 <<"\n";
      return;
      }
   int len_char = strlen(cstring);
   int new_size = num_chars+len_char;
   int num_save = num_chars;
   int i = 0;
// Make room for character string.

   for ( i=0; i<len_char; i++) 
      append(' ');

// Shift characters after position to make room for inserted string

   for (i=0; i<num_save-position; i++) {
        char_array[new_size-i-1] = char_array[num_save-i-1];
      }

// Insert string

   for (i=position; i<position+len_char; i++)
        char_array[i] = cstring[i-position];
//   num_chars = new_size;
//   char_array[num_chars] = '\0';

}

//----------------------------------------------------------------------------

void Stringc::insert_string_at (int position, const char character)
{
   int num_save = num_chars;
   int new_size = num_chars+1;

// Add room for a character.

   append(' ');

// Shift characters after position to make room for inserted string

   for (int i=0; i<num_save-position; i++)
      char_array[new_size-i-1] = char_array[num_save-i-1];

// Insert character

   char_array[position] = character;
   num_chars = new_size;
   char_array[num_chars] = '\0';
}

//----------------------------------------------------------------------------

int Stringc::search_for_substring( char character )
{

// search for the character

   for ( int i = 0; i < num_chars; i++ )
      {
      if ( char_array[i] == character )
         {
         return i;
         }
      }

// if no match was found, return minus 1

   return -1;
}

//----------------------------------------------------------------------------

int Stringc::matchwild(Stringc wildcardstring)
{

	char *wild = wildcardstring();
	char *string = get_char_star();

	char *cp, *mp;
	
	while ((*string) && (*wild != '*')) {
		if ((*wild != *string) && (*wild != '?')) {
			return 0;
		}
		wild++;
		string++;
	}
		
	while (*string) {
		if (*wild == '*') {
			if (!*++wild) {
				return 1;
			}
			mp = wild;
			cp = string+1;
		} else if ((*wild == *string) || (*wild == '?')) {
			wild++;
			string++;
		} else {
			wild = mp;
			string = cp++;
		}
	}
		
	while (*wild == '*') {
		wild++;
	}
	return !*wild;

}

//----------------------------------------------------------------------------

void Stringc::delete_range(int istart, int iend)
{
   rdelete(istart,iend);
}

//----------------------------------------------------------------------------

Stringc Stringc::get_range(int istart, int iend)
{
   int str_len = iend-istart+2;
   char *new_char = new char[str_len];
   for (int i=istart; i<=iend; i++) 
      new_char[i-istart] = char_array[i];
   new_char[iend+1-istart] = '\0';
   Stringc new_string(new_char);
   delete [] new_char;
   return new_string;
}

//----------------------------------------------------------------------------

void Stringc::overwrite_at_position(int position, const char *cstring)
{
   int len_char = strlen(cstring);
   if (position < 0)
      {
      cout << "Error in Stringc::overwrite_at_position routine.\n";
      cout << "   String: " << char_array;
      cout << " Overwrite string: " << cstring << "\n";
      cout << "   Position cannot be negative it can run from 0 to ";
      cout << num_chars-len_char << "\n";
      }
   else if (position+len_char > num_chars )
      {
      cout << "Error in Stringc::overwrite_at_position routine.\n";
      cout << "   String: " << char_array;
      cout << "  Overwrite string: " << cstring << "\n";
      cout << "   Position, " << position;
      cout << ", is out of range it can run from 0 to ";
      cout << num_chars-len_char << "\n"; 
      }     
   else
      for (int i=0; i < len_char; i++)
         char_array[i+position] = cstring[i];
}

//----------------------------------------------------------------------------

void Stringc::remove_leading_blanks()
{
   if ( num_chars > 0 )
      {
      int i = 0;
      while ( (char_array[i] == ' ' || char_array[i] == '\t') && i<num_chars)
         i++;
      int iend = i; 
      if (iend != 0 ) rdelete(0,iend-1);
      }
}

//----------------------------------------------------------------------------

void Stringc::remove_trailing_blanks()
{
   if ( num_chars > 0 )
      {
      int i=num_chars-1;
      while ( (char_array[i] == ' ' || char_array[i] == '\t') && i>=0 ) i--;
      int istart = i+1;
      if ( istart != num_chars ) rdelete(istart,num_chars-1);
      }
}

//----------------------------------------------------------------------------

void Stringc::remove_leading(char c)
{
   if ( num_chars > 0 )
      {
      int i = 0;
      while ( (char_array[i] == c) && i<num_chars)
         i++;
      int iend = i; 
      if (iend != 0 ) rdelete(0,iend-1);
      }
}

//----------------------------------------------------------------------------

void Stringc::remove_trailing(char c)
{
   if ( num_chars > 0 )
      {
      int i=num_chars-1;
      while ( (char_array[i] == c) && i>=0 ) i--;
      int istart = i+1;
      if ( istart != num_chars ) rdelete(istart,num_chars-1);
      }
}
//----------------------------------------------------------------------------

int Stringc::count_substrings(const char *substring)
{
   int j = 0;
   int len_sub = strlen(substring);
   int icount = 0;
   for (int i=0; i <= num_chars-len_sub; i++)
      {
      j = 0;
      int matches = 1;
      while ( j < len_sub && matches )
         {
         if ( char_array[i+j] != substring[j] ) matches = 0;
         j++;
         } 
      if ( matches ) icount++; 
      }
   return icount; 
}

//----------------------------------------------------------------------------

void Stringc::remove_substring(const char *substring, int istring)
{
   int j = 0;
   int len_sub = strlen(substring);
   int icount = 0;
   int i=0;
   int notfound = 1;
   while (notfound && i<num_chars)
      {
      j = 0;
      int matches = 1;
      while ( j < len_sub && matches )
         {
         if ( char_array[i+j] != substring[j] ) matches = 0;
         j++;
         } 
      if ( matches ) icount++;
      if ( matches && icount == istring) 
         {
         rdelete(i,i+len_sub-1);
         notfound = 0;
         }
      i++;
      }
}

//----------------------------------------------------------------------------

void Stringc::remove_all_substrings(const char *substring)
{
   int j = 0;
   int len_sub = strlen(substring);
   int i=0;
   while (i < num_chars-len_sub)
      {
      j = 0;
      int matches = 1;
      while ( j < len_sub && matches )
         {
         if ( char_array[i+j] != substring[j] ) matches = 0;
         j++;
         } 
      if ( matches ) rdelete(i,i+len_sub-1);
      i++;
      }
}

//----------------------------------------------------------------------------

void Stringc::remove_substring(char character, int ichar)
{
   int j = 0;
   int icount = 0;
   int i=0;
   int notfound = 1;
   while (notfound && i<num_chars)
      {
      j = 0;
      int matches = 1;
      while ( j < 1 && matches )
         {
         if ( char_array[i+j] != character ) matches = 0;
         j++;
         } 
      if ( matches ) icount++;
      if ( matches && icount == ichar) 
         {
         rdelete(i,i);
         notfound = 0;
         }
      i++;
      }
}

//----------------------------------------------------------------------------

void Stringc::remove_all_substrings(char character)
{
   int j = 0;
   int i=0;
   while (i < num_chars)
      {
      j = 0;
      int matches = 1;
      while ( j < 1 && matches )
         {
         if ( char_array[i+j] != character ) matches = 0;
         j++;
         } 
      if ( matches ) rdelete(i,i);
      i++;
      }
}

//----------------------------------------------------------------------------

int Stringc::is_double( double* ret )
{
	for ( int i = 0 ; i < num_chars ; i++ )
	{
		if ( char_array[i] == '+' || char_array[i] == '-' || char_array[i] == '.' || 
			 (( char_array[i] >= '0') && ( char_array[i] <= '9' )) )
			continue;
		else
			return 0;
	}

	*ret = atof( get_char_star() );
	return 1;
			
}

//----------------------------- OPERATORS ------------------------------------
//----------------------------------------------------------------------------

Stringc& Stringc::operator=( const char ch)
   {
   (*this).clear_space();
   (*this).num_chars = 1;
   (*this).chunk_size = 8;
   (*this).make_space();
   (*this).char_array[0] = ch;
   (*this).char_array[1] = '\0';
   return *this;
   }

//----------------------------------------------------------------------------

Stringc& Stringc::operator=( const Stringc& cstring )
   {
   (*this).clear_space();
   (*this).num_chars = cstring.num_chars;
   (*this).chunk_size = cstring.chunk_size;
   (*this).make_space();
   for (int i=0; i<=cstring.num_chars; i++)
      (*this).char_array[i] = cstring.char_array[i];
(*this).char_array[num_chars] = '\0';

   return *this;
   }

//----------------------------------------------------------------------------

Stringc& Stringc::operator=( const char *cstring )
   {
   (*this).clear_space();
   (*this).num_chars = strlen(cstring);
   (*this).chunk_size = 8;
   (*this).make_space();
   for (int i=0; i < (int)strlen(cstring); i++)
      (*this).char_array[i] = cstring[i];
   (*this)[num_chars] = '\0';
   return *this;
   }

//----------------------------------------------------------------------------

int Stringc::operator==( const Stringc& cstring ) const
   {
   if ( strcmp( char_array, cstring.char_array ) )
      return FALSE;
   else 
      return TRUE;
   }

//----------------------------------------------------------------------------

int Stringc::operator==( const char *cstring ) const
   {
   if ( strcmp( char_array, cstring ) )
      return FALSE;
   else 
      return TRUE;
   }


//----------------------------------------------------------------------------

int Stringc::operator!=( const Stringc& cstring ) const
   {
   if ( strcmp( char_array, cstring.char_array ) )
      return TRUE;
   else 
      return FALSE;
   }

//----------------------------------------------------------------------------

int Stringc::operator!=( const char *cstring ) const
   {
   if ( strcmp( char_array, cstring ) )
      return TRUE;
   else 
      return FALSE;
   }

//----------------------------------------------------------------------------

//ostream& operator<<( ostream &os, const Stringc &stringc )
//   {
//   os << stringc.char_array;
//   return os;
//   }
