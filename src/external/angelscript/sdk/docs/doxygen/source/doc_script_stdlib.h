/**


\page doc_script_stdlib Standard library

This pages describes the standard library provided by the AngelScript SDK. The applications that 
use AngelScript may or may not expose the standard library to the scripts. Always consult the application's
manual for information on the API it exposes.

 - \subpage doc_script_stdlib_exception
 - \subpage doc_script_stdlib_string
 - \subpage doc_datatypes_arrays
 - \subpage doc_datatypes_dictionary
 - \subpage doc_datatypes_ref
 - \subpage doc_datatypes_weakref
 - \subpage doc_script_stdlib_datetime
 - \subpage doc_script_stdlib_coroutine
 - \subpage doc_script_stdlib_file
 - \subpage doc_script_stdlib_filesystem
 - \subpage doc_script_stdlib_system
 



\page doc_script_stdlib_exception Exception handling

\note The standard <tt>throw</tt> and <tt>getExceptionInfo</tt> are only 
provided if the application \ref doc_addon_helpers_try "registers them". 

\section try_func Functions

<b>void throw(const string &in exception)</b>

Explicitly throw an exception. The string should identify the type of exception, for logging or treating.

<b>string getExceptionInfo()</b>

Get the exception string for the last exception thrown.




\page doc_datatypes_arrays array

\note Arrays are only available in the scripts if the application \ref doc_addon_array "registers the support for them". 
The syntax for using arrays may differ for the application you're working with so consult the application's manual
for more details.

It is possible to declare array variables with the array identifier followed by the type of the 
elements within angle brackets. 

Example:

<pre>
  array<int> a, b, c;
  array<Foo\@> d;
</pre>

<code>a</code>, <code>b</code>, and <code>c</code> are now arrays of integers, and <code>d</code>
is an array of handles to objects of the Foo type.

When declaring arrays it is possible to define the initial size of the array by passing the length as
a parameter to the constructor. The elements can also be individually initialized by specifying an 
initialization list. Example:

<pre>
  array<int> a;           // A zero-length array of integers
  array<int> b(3);        // An array of integers with 3 elements
  array<int> c(3, 1);     // An array of integers with 3 elements, all set to 1 by default
  array<int> d = {5,6,7}; // An array of integers with 3 elements with specific values
</pre>

Multidimensional arrays are supported as arrays of arrays, for example:

<pre>
  array<array<int>> a;                     // An empty array of arrays of integers
  array<array<int>> b = {{1,2},{3,4}}      // A 2 by 2 array with initialized values
  array<array<int>> c(10, array<int>(10)); // A 10 by 10 array of integers with uninitialized values
</pre>

Each element in the array is accessed with the indexing operator. The indices are zero based, i.e. the
range of valid indices are from 0 to length - 1.

<pre>
  a[0] = some_value;
</pre>

When the array stores \ref doc_script_handle "handles" the elements are assigned using the \ref handle "handle assignment".

<pre>
  // Declare an array with initial length 1
  array<Foo\@> arr(1);
  
  // Set the first element to point to a new instance of Foo
  \@arr[0] = Foo();
</pre>

Arrays can also be created and initialized within expressions as \ref anonobj "anonymous objects". 

<pre>
  // Call a function that expects an array of integers as input
  foo({1,2,3,4});
  
  // If the function has multiple overloads supporting different types with 
  // initialization lists it is necessary to explicitly inform the array type
  foo2(array<int> = {1,2,3,4});
</pre>

\section doc_datatypes_arrays_addon Supporting array object

The array object supports a number of operators and has several class methods to facilitate the manipulation of strings.

The array object is a \ref doc_datatypes_obj "reference type" even if the elements are not, so it's possible
to use handles to the array object when passing it around to avoid costly copies.

\subsection doc_datatypes_array_addon_ops Operators

<b>=       assignment</b>

The assignment operator performs a shallow copy of the content.
 
<b>[]      index operator</b>

The index operator returns the reference of an element allowing it to be 
inspected or modified. If the index is out of range, then an exception will be raised.
 
<b>==, !=  equality</b>
 
Performs a value comparison on each of the elements in the two arrays 
and returns true if all match the used operator.

\subsection doc_datatypes_array_addon_mthd Methods

<b>uint length() const</b>
  
Returns the length of the array.
  
<b>void resize(uint)</b>
 
Sets the new length of the array.
 
<b>void reverse()</b>

Reverses the order of the elements in the array.
 
<b>void insertAt(uint index, const T& in value)</b><br>
<b>void insertAt(uint index, const array<T>& arr)</b><br>

Inserts a new element, or another array of elements, into the array at the specified index. 
 
<b>void insertLast(const T& in)</b>

Appends an element at the end of the array.
 
<b>void removeAt(uint index)</b>

Removes the element at the specified index.
 
<b>void removeLast()</b>

Removes the last element of the array.

<b>void removeRange(uint start, uint count)</b>

Removes \a count elements starting from \a start.

<b>void sortAsc()</b><br>
<b>void sortAsc(uint startAt, uint count)</b><br>

Sorts the elements in the array in ascending order. For object types, this will use the type's opCmp method.

The second variant will sort only the elements starting at index \a startAt and the following \a count elements.
 
<b>void sortDesc()</b><br>
<b>void sortDesc(uint startAt, uint count)</b><br>

These does the same thing as sortAsc except sorts the elements in descending order.

<b>void sort(const less &in compareFunc, uint startAt = 0, uint count = uint(-1))</b><br>

This method takes as input a callback function to use for comparing two elements when sorting the array.

The callback function should take as parameters two references of the same type of the array elements and it 
should return a bool. The return value should be true if the first argument should be placed before the second 
argument.

<pre>
  array<int> arr = {3,2,1};
  arr.sort(function(a,b) { return a < b; });
</pre>

The example shows how to use the sort method with a callback to an \ref doc_script_anonfunc "anonymous function".

Here's another example where the callback function is declared explicitly:

<pre>
  bool lessForInt(const int &in a, const int &in b)
  {
    return a < b;
  }
  bool lessForHandle(const obj \@&in a, const obj \@&in b)
  {
    return a < b;
  }
  void sortArrayOfInts(array<int> \@arr) { arr.sort(lessForInt); }
  void sortArrayOfHandles(array<obj\@> \@arr) { arr.sort(lessForHandle); }
</pre>

<b>int  find(const T& in)</b><br>
<b>int  find(uint startAt, const T& in)</b><br>

These will return the index of the first element that has the same value as the wanted value.

For object types, this will use the type's opEquals or opCmp method to compare the value. 
For arrays of handles any null handle will be skipped.

If no match is found the methods will return a negative value.
 
<b>int  findByRef(const T& in)</b><br>
<b>int  findByRef(uint startAt, const T& in)</b><br>

These will search for a matching address. These are especially useful for arrays of handles where
specific instances of objects are desired, and not just objects that happen to have equal value.

If no match is found the methods will return a negative value.

\subsection doc_datatypes_array_addon_example Script example
  
<pre>
  int main()
  {
    array<int> arr = {1,2,3}; // 1,2,3
    arr.insertLast(0);        // 1,2,3,0
    arr.insertAt(2,4);        // 1,2,4,3,0
    arr.removeAt(1);          // 1,4,3,0

    arr.sortAsc();            // 0,1,3,4
  
    int sum = 0;
    for( uint n = 0; n < arr.length(); n++ )
      sum += arr[n];
      
    return sum;
  }
</pre>





\page doc_datatypes_dictionary dictionary

\note Dictionaries are only available in the scripts if the application \ref doc_addon_dict "registers the support for them". 
The syntax for using dictionaries may differ for the application you're working with so consult the application's manual
for more details.

The dictionary stores key-value pairs, where the key is a string, and the value can be of any type. Key-value
pairs can be added or removed dynamically, making the dictionary a good general purpose container object.

<pre>
  obj object;
  obj \@handle;
  
  // Initialize with a list
  dictionary dict = {{'one', 1}, {'object', object}, {'handle', \@handle}};
  
  // Examine and access the values through get or set methods ...
  if( dict.exists('one') )
  {
    // get returns true if the stored type is compatible with the requested type
    bool isValid = dict.get('handle', \@handle);
    if( isValid )
    {
      dict.delete('object');
      dict.set('value', 1);
    }
  }
</pre>

Dictionary values can also be accessed or added by using the index operator.
  
<pre>
  // Read and modify an integer value
  int val = int(dict['value']);
  dict['value'] = val + 1;
  
  // Read and modify a handle to an object instance
  \@handle = cast<obj>(dict['handle']);
  if( handle is null )
    \@dict['handle'] = object;
</pre>
  
Dictionaries can also be created and initialized within expressions as \ref anonobj "anonymous objects". 

<pre>
  // Call a function that expects a dictionary as input and no other overloads
  // In this case it is possible to inform the initialization list without explicitly giving the type
  foo({{'a', 1},{'b', 2}});
  
  // Call a function where there are multiple overloads expecting different
  // In this case it is necessary to explicitly define the type of the initialization list
  foo2(dictionary = {{'a', 1},{'b', 2}});
</pre>

Dictionaries of dictionaries are created using \ref anonobj "anonymous objects" as well.

<pre>
  dictionary d2 = {{'a', dictionary = {{'aa', 1}, {'ab', 2}}}, 
                   {'b', dictionary = {{'ba', 1}, {'bb', 2}}}};
</pre>


\section doc_datatypes_dictionary_addon Supporting dictionary object

The dictionary object is a \ref doc_datatypes_obj "reference type", so it's possible
to use handles to the dictionary object when passing it around to avoid costly copies.

\subsection doc_datatypes_dictionary_addon_ops Operators

<b>=       assignment</b><br>

The assignment operator performs a shallow copy of the content.

<b>[]      index operator</b><br>
 
The index operator takes a string for the key, and returns a reference to the \ref doc_datatypes_dictionaryValue_addon "value".
If the key/value pair doesn't exist it will be inserted with a null value.

\subsection doc_datatypes_dictionary_addon_mthd Methods

<b>void set(const string &in key, ? &in value)</b><br>
<b>void set(const string &in key, int64 &in value)</b><br>
<b>void set(const string &in key, double &in value)</b><br>

Sets a key/value pair in the dictionary. If the key already exists, the value will be changed.

<b>bool get(const string &in key, ? &out value) const</b><br>
<b>bool get(const string &in key, int64 &out value) const</b><br>
<b>bool get(const string &in key, double &out value) const</b><br>
 
Retrieves the value corresponding to the key. The methods return false if the key is not 
found, and in this case the value will maintain its default value based on the type.
 
<b>array<string> \@getKeys() const</b><br>
 
This method returns an array with all of the existing keys in the dictionary. 
The order of the keys in the array is undefined.
 
<b>bool exists(const string &in key) const</b><br>
 
Returns true if the key exists in the dictionary.
 
<b>bool delete(const string &in key)</b><br>

Removes the key and the corresponding value from the dictionary. Returns false if the key wasn't found.
 
<b>void deleteAll()</b><br>

Removes all entries in the dictionary.
 
<b>bool isEmpty() const</b><br>

Returns true if the dictionary doesn't hold any entries.

<b>uint getSize() const</b><br>

Returns the number of keys in the dictionary.



\section doc_datatypes_dictionaryValue_addon Supporting dictionaryValue object

The dictionaryValue type is how the \ref doc_datatypes_dictionary_addon "dictionary" object stores the 
values. When accessing the values through the dictionary index operator a reference to a dictionaryValue is returned.

The dictionaryValue type itself is a value type, i.e. no handles to it can be held, but it can hold handles
to other objects as well as values of any type.

\subsection doc_datatypes_dictionaryValue_addon_ops Operators

<b>=       assignment</b><br>

The value assignment operator should be used to copy a value into the dictionaryValue.

<b>\@=     handle assignment</b><br>

The handle assignment operator should be used to set the dictionaryValue to refer to an object instance.

<b>cast<type>  cast operator</b><br>

The cast operator is used to dynamically cast the handle held in the dictionaryValue to the desired type. 
If the dictionaryValue doesn't hold a handle, or the handle is not compatible with the desired type, the 
cast operator will return a null handle.

<b>type()      conversion operator</b><br>

The conversion operator is used to return a new value of the desired type. If no value conversion is found,
an uninitialized value of the desired type is returned.






\page doc_script_stdlib_string string

\note Strings are only available in the scripts if the application \ref doc_addon_std_string "registers the support for them". 
The syntax for using strings may differ for the application you're working with so consult the application's manual
for more details.

\see \ref doc_datatypes_strings for information on syntax for string literals

\section doc_datatypes_strings_addon Supporting string object and functions

The string object supports a number of operators, and has several class methods and supporting 
global functions to facilitate the manipulation of strings.

\subsection doc_datatypes_strings_addon_ops Operators

<b>=            assignment</b><br>

The assignment operator copies the content of the right hand string into the left hand string. 

Assignment of primitive types is allowed, which will do a default transformation of the primitive to a string.

<b>+, +=        concatenation</b><br>

The concatenation operator appends the content of the right hand string to the end of the left hand string.

Concatenation of primitives types is allowed, which will do a default transformation of the primitive to a string.
 
<b>==, !=       equality</b><br>
 
Compares the content of the two strings.
 
<b><, >, <=, >= comparison</b><br>

Compares the content of the two strings. The comparison is done on the byte values in the strings, which 
may not correspond to alphabetical comparisons for some languages.

<b>[]           index operator</b><br>

The index operator gives access to a single byte in the string.
 
\subsection doc_datatypes_strings_addon_mthd Methods

<b>uint           length() const</b><br>

Returns the length of the string.
 
<b>void           resize(uint)</b><br>

Sets the length of the string.
 
<b>bool           isEmpty() const</b><br>

Returns true if the string is empty, i.e. the length is zero.
 
<b>string         substr(uint start = 0, int count = -1) const</b><br>

Returns a string with the content starting at \a start and the number of bytes given by count. The default arguments will return the whole string as the new string.

<b>void insert(uint pos, const string &in other)</b><br>

Inserts another string \a other at position \a pos in the original string. 

<b>void erase(uint pos, int count = -1)</b><br>

Erases a range of characters from the string, starting at position \a pos and counting \a count characters.

<b>int            findFirst(const string &in str, uint start = 0) const</b><br>

Find the first occurrence of the value \a str in the string, starting at \a start. If no occurrence is found a negative value will be returned.
 
<b>int            findLast(const string &in str, int start = -1) const</b><br>

Find the last occurrence of the value \a str in the string. If \a start is informed the search will begin at that position, i.e. any potential occurrence after that position will not be searched. If no occurrence is found a negative value will be returned.
 
<b>int            findFirstOf(const string &in chars, int start = 0) const</b><br>
<b>int            findFirstNotOf(const string &in chars, int start = 0) const</b><br>
<b>int            findLastOf(const string &in chars, int start = -1) const</b><br>
<b>int            findLastNotOf(const string &in chars, int start = -1) const</b><br>

The first variant finds the first character in the string that matches on of the characters in \a chars, starting at \a start. If no occurrence is found a negative value will be returned.

The second variant finds the first character that doesn't match any of those in \a chars. The third and last variant are the same except they start the search from the end of the string.

\note These functions work on the individual bytes in the strings. They do not attempt to understand encoded characters, e.g. UTF-8 encoded characters that can take up to 4 bytes.

<b>array<string>@ split(const string &in delimiter) const</b><br>

Splits the string in smaller strings where the delimiter is found.
 
\subsection doc_datatypes_strings_addon_funcs Functions

<b>string join(const array<string> &in arr, const string &in delimiter)</b><br>

Concatenates the strings in the array into a large string, separated by the delimiter.
 
<b>int64  parseInt(const string &in str, uint base = 10, uint &out byteCount = 0)</b><br>
<b>uint64 parseUInt(const string &in str, uint base = 10, uint &out byteCount = 0)</b><br>

Parses the string for an integer value. The \a base can be 10 or 16 to support decimal numbers or 
hexadecimal numbers. If \a byteCount is provided it will be set to the number of bytes that were 
considered as part of the integer value.

<b>double parseFloat(const string &in, uint &out byteCount = 0)</b><br>

Parses the string for a floating point value. If \a byteCount is provided it will be set to the 
number of bytes that were considered as part of the value.

<b>string formatInt(int64 val, const string &in options = '', uint width = 0)</b><br>
<b>string formatUInt(uint64 val, const string &in options = '', uint width = 0)</b><br>
<b>string formatFloat(double val, const string &in options = '', uint width = 0, uint precision = 0)</b><br>

The format functions takes a string that defines how the number should be formatted. The string
is a combination of the following characters:

  - l = left justify
  - 0 = pad with zeroes
  - + = always include the sign, even if positive
  - space = add a space in case of positive number
  - h = hexadecimal integer small letters (not valid for formatFloat)
  - H = hexadecimal integer capital letters (not valid for formatFloat)
  - e = exponent character with small e (only valid for formatFloat)
  - E = exponent character with capital E (only valid for formatFloat)

Examples:

<pre>
  // Left justify number in string with 10 characters
  string justified = formatInt(number, 'l', 10);
  
  // Create hexadecimal representation with capital letters, right justified
  string hex = formatInt(number, 'H', 10);
  
  // Right justified, padded with zeroes and two digits after decimal separator
  string num = formatFloat(number, '0', 8, 2);
</pre>






\page doc_datatypes_ref ref

\note <code>ref</code> is only available in the scripts if the application \ref doc_addon_handle "registers the support for it". 

The <code>ref</code> type works like a generic object handle. Normally a \ref handles "handle" can only refer to 
objects of a specific type or those related to it, however not all object types are related, and this is
where <code>ref</code> comes in. Being completely generic it can refer to any object type (as long as it is a \ref doc_datatypes_obj "reference type").

<pre>
  // Two unrelated types
  class car {}
  class banana {}

  // A function that take the ref type as argument can work on both types
  void func(ref \@handle)
  {
    // Cast the handle to the expected type and check which cast work
    car \@c = cast<car>(handle);
    banana \@b = cast<banana>(handle);
    if( c !is null )
      print('The handle refers to a car\\n');
    else if( b !is null )
      print('The handle refers to a banana\\n');
    else if( handle !is null )
      print('The handle refers to a different object\\n');
    else
      print('The handle is null\\n');
  }

  void main()
  {
    // Assigning a ref handle works the same way as ordinary handles
    ref \@r = car();
    func(r);
    \@r = banana();
    func(r);
  }
</pre>

\section doc_datatypes_ref_addon Supporting ref object

The ref object supports only a few operators as it is just a place holder for handles.

\subsection doc_datatypes_ref_addon_ops Operators

<b>\@=          handle assignment</b><br>
 
The handle assignment operator is used to set the object that the referred to by the ref type.
 
<b>is, !is      identity operator</b><br>
 
The identity operators are used to compare the address of the object referred to by the ref type.
 
<b>cast<type>   cast operator</b><br>

The cast operator is used to perform a dynamic cast to the desired type. 
If the type is not compatible with the object referred to by the ref type this will return null.


 
 
\page doc_datatypes_weakref weakref

\note <code>weakref</code> is only available in the scripts if the application \ref doc_addon_weakref "registers the support for it". 

An object handle will keep the object it refers to alive as long as the handle itself exists. A <code>weakref</code> object
can be used in place of the handle where the reference to the object is needed but the object shouldn't be kept alive.

<pre>
  class MyClass {}
  MyClass \@obj1 = MyClass();
  
  // Keep a weakref to the object
  weakref<MyClass> r1(obj1);
  
  // Keep a weakref to a readonly object
  const_weakref<MyClass> r2(obj1);
  
  // As long as there is a strong reference to the object, 
  // the weakref will be able to return a handle to the object
  MyClass \@obj2 = r1.get();
  assert( obj2 !is null );
  
  // After all strong references are removed the
  // weakref will only return null
  \@obj1 = null;
  \@obj2 = null;
  
  const MyClass \@obj3 = r2.get();
  assert( obj3 is null );
</pre>

\section doc_datatypes_weakref_addon Supporting weakref object



\subsection doc_datatypes_weakref_addon_ops Operators

<b>\@=          handle assignment</b><br>
 
The handle assignment operator is used to set the object that the referred to by the ref type.

<b>=            value assignment</b><br>

The value assignment operator is used when one weakref object is copied to another.

<b>is, !is      identity operator</b><br>
 
The identity operators are used to compare the address of the object referred to by the ref type.

<b>cast<type>   implicit cast operator</b><br>

The implicit cast operator is used to cast the weak ref type to strong reference of the type.
If the object referred to by the weakref is already dead this operator will return null.


 
\subsection doc_datatypes_weakref_addon_mthd Methods

<b>T@ get() const</b><br>

This does the exact same thing as the implicit cast operator. It is just a more explicit way of 
writing it.

 
 
 
\page doc_script_stdlib_datetime datetime

\note datetime is only available in the scripts if the application \ref doc_addon_datetime "registers support for it".

The datetime type represents a calendar date and time. It can be used to do math operations with 
dates, such as comparing two dates, determining the difference between dates, and addition/substraction 
on dates to form new dates.

It can also be used to get the current system time and thus allow measuring time for tasks, albeit with
a rather low precision of seconds only.




\section doc_datatype_datetime_addon Supporting datetime object

\subsection doc_addon_datetime_2_construct Constructors

<b>datetime()</b><br>
<b>datetime(const datetime &in other)</b><br>
<b>datetime(uint y, uint m, uint d, uint h = 0, uint mi = 0, uint s = 0)</b><br>
 
The default constructor initializes the object with the current system time in the universal time zone (UTC). If you need 
to consider a specific timezone with or without daylight savings then remember to adjust the time accordingly by
adding the number of seconds for the difference.

The copy constructor copíes the content of the other object.

The set constructor initializes the object with the given date and time.

\subsection doc_addon_datetime_2_methods Methods

<b>uint get_year() const property</b>

Returns the year of the date stored in the object. 

<b>uint get_month() const property</b>

Returns the month of the date stored in the object. The range is 1 to 12, i.e. 1 is January, 12 is December, and so on.
 
<b>uint get_day() const property</b>

Returns the day of the month of the date stored in the object.
 
<b>uint get_hour() const property</b>

Returns the hour of the time stored in the object. The range is 0 to 23.
 
<b>uint get_minute() const property</b>

Returns the minute of the time stored in the object. The range is 0 to 59.

<b>uint get_second() const property</b>
 
Returns the second of the time stored in the object. The range is 0 to 59.

<b>bool setDate(uint year, uint month, uint day)</b><br>
<b>bool setTime(uint hour, uint minute, uint second)</b>

Sets the date or time. Returns true if the specified date or time is valid. Does not modify the object if not valid.

\subsection doc_addon_datetime_2_ops Operators

<b>= assignment</b>
 
The assignment operator copies the content of the other object.

<b>- difference</b>

When subtracting one datetime object from another the result is the number of seconds between them.

<b>+ add</b><br>
<b>- subtract</b><br>
<b>+= add assign</b><br>
<b>-= subtract assign</b>

The datetime object can be added or subtracted with seconds to form a new datetime object.

<b>==, != equality</b><br>
<b><, <=, >=, > comparison</b>

The datetime object can be compared for equality or relativity.



\page doc_script_stdlib_coroutine Co-routines

\note Support for co-routines is only available in the scripts if the application \ref doc_addon_ctxmgr "registers support for it".


\section doc_script_stdlib_coroutine_1 Functions

<b>funcdef void coroutine(dictionary@)</b><br>
<b>void createCoRoutine(coroutine @, dictionary @)</b>

This function is used to create a co-routine. The co-routine will initiate in a 
yielded state, i.e. it will only begin execution once the control is given to it
by the current thread. 

Multiple co-routines can be created, and they will each take turn to execute in 
round-robin fashion.

<b>void yield()</b>

Yields control of the execution for the next co-routine in the queue. 

When a co-routine receives control it will resume execution from the last call to
yield, or the entry point if this is the first time the co-routine is allowed to execute.




\page doc_script_stdlib_file file

\note file is only available in the scripts if the application \ref doc_addon_file "registers support for it".

Script example:

<pre>
  file f;
  // Open the file in 'read' mode
  if( f.open("file.txt", "r") >= 0 ) 
  {
      // Read the whole file into the string buffer
      string str = f.readString(f.getSize()); 
      f.close();
  }
</pre>

\section doc_script_stdlib_file_1 Supporting file object

\subsection doc_script_stdlib_file_1_1 Methods

<b>int open(const string &in filename, const string &in mode)</b><br>

Opens a file. The mode can be "r" for reading, "w" for writing, or "a" for appending.

If the file couldn't be opened, a negative value is returned.

<b>int close()</b><br>

Closes the file.

If no file is open, a negative value is returned.

<b>int getSize() const</b><br>

Returns the size of the file, or a negative value if no file is open.

<b>bool isEndOfFile() const</b><br>

Returns true if the current position is at the end of the file.

<b>string readString(uint length)</b><br>

Reads \a length bytes into a string and returns it.

<b>string readLine()</b><br>

Reads until a new line character, e.g. '\\n', or end-of-file and returns the string. The new line character is also returned in the string.

<b>int64 readInt(uint bytes)</b><br>

Reads \a bytes as a signed integer number.

<b>uint64 readUInt(uint bytes)</b><br>

Reads \a bytes as an unsigned integer number.

<b>float readFloat()</b><br>

Reads 4 bytes as a float number.

<b>double readDouble()</b><br>

Reads 8 bytes as a double number.

<b>int writeString(const string &in str)</b><br>

Writes the bytes of the string into the file. 

Returns the number of bytes written, or a negative value on error.

<b>int writeInt(int64 value, uint bytes)</b><br>

Writes \a bytes as a signed integer value.

Returns the number of bytes written, or a negative value on error.

<b>int writeUInt(uint64 value, uint bytes)</b><br>

Writes \a bytes as an unsigned integer value.

Returns the number of bytes written, or a negative value on error.

<b>int writeFloat(float value)</b><br>

Writes 4 bytes as a float value.

Returns the number of bytes written, or a negative value on error.

<b>int writeDouble(double value)</b><br>

Writes 8 bytes as a double value.

Returns the number of bytes written, or a negative value on error.

<b>int getPos() const</b><br>

Returns the current position in the file, or a negative value on error.

<b>int setPos(int pos)</b><br>

Sets the current position in the file. Returns the previous position or a negative value on error.

<b>int movePos(int delta)</b><br>

Moves the position \a delta bytes relative to the current position. Returns the previous position or a negative value on error.

\subsection doc_script_stdlib_file_1_2 Properties

<b>bool mostSignificantByteFirst</b><br>

This property should be set to true if the most significant bit should be read or written first in the methods that reads/writes numbers.

It is set to false by default, which is the standard on most platforms.




\page doc_script_stdlib_filesystem filesystem

\note filesystem is only available in the scripts if the application \ref doc_addon_filesystem "registers support for it".


\section doc_script_stdlib_filesystem_1 Supporting filesystem object

\subsection doc_script_stdlib_filesystem_1_1 Methods

<b>bool changeCurrentPath(const string &in path)</b>

This changes the current directory used by the filesystem object. It will return true if the given path is valid.

It doesn't change the application' working directory.

<b>string getCurrentPath() const</b>

Returns the current path used by the filesystem object.

<b>array<string> \@getDirs()</b>

Returns a list with the names of all directories in the current path.

<b>array<string> \@getFiles()</b>

Returns a list with the names of all files in the current path.

<b>bool isDir(const string &in path)</b>

Returns true if the given path is a directory.

<b>bool isLink(const string &in path)</b>

Returns true if the given path is a link.

<b>int64 getSize(const string &in) const</b>

Returns the size of a file.

Returns -1 if the file doesn't exist or cannot be accessed.

<b>int makeDir(const string &in)</b>

Creates a new directory. Returns 0 on success.

<b>int removeDir(const string &in)</b>

Removes a directory. Will only remove the directory if it is empty. Returns 0 on success.

<b>int deleteFile(const string &in)</b>

Deletes a file. Returns 0 on success.

<b>int copyFile(const string &in, const string &in)</b>

Copies a file. Returns 0 on success.

<b>int move(const string &in, const string &in)</b>

Moves or renames a file or directory. Returns 0 on success.

<b>datetime getCreateDateTime(const string &in)</b>

Returns the date and time of the file creation in UTC. 

Raises an exception if the file doesn't exist or cannot be accessed.

<b>datetime getModifyDateTime(const string &in)</b>

Returns the date and time of the last modification of the file in UTC. 

Raises an exception if the file doesn't exist or cannot be accessed.





\page doc_script_stdlib_system System functions

\note The system functions are only available in the scripts if the application \ref doc_samples_asrun "registers support for it".

\section doc_script_stdlib_system_1 Functions

<b>void print(const string &in line)</b>

Prints a line to the standard output.
 
<b>string getInput()</b>

Gets a line from the standard input.
 
<b>array<string> \@getCommandLineArgs()</b>

Gets the command line arguments as an array.
 
<b>int exec(const string &in cmd)</b><br>
<b>int exec(const string &in cmd, string &out output)</b>

Executes a system command. 

Returns -1 on error or raises an exception. On success returns the exit code from the system commmand.

The second alternative allows to capture the stdout into a string, to be further processed.





*/
