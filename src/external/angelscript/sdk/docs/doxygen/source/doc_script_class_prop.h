/**

\page doc_script_class_prop Property accessors

\note The application can optionally \ref doc_adv_custom_options_lang_mod "turn off support for property accessors", 
so you need to verify your application's manual to determine if this is supported for your application or not.

Many times when working with properties it is necessary to make sure specific logic is followed when
accessing them. An example would be to always send a notification when a property is modified, or computing
the value of the property from other properties. By implementing property accessor methods for the properties
this can be implemented by the class itself, making it easier for the one who accesses the properties.

In AngelScript property accessors are declared with the following syntax:

<pre>
  class MyObj
  {
    // A virtual property with accessors
    int prop 
    { 
      get const 
      { 
        // The actual value of the property could be stored
        // somewhere else, or even computed at access time.
        return realProp; 
      } 
      set 
      { 
        // The new value is stored in a hidden parameter appropriately called 'value'.
        realProp = value; 
      }
    }
  
    // The actual value can be stored in a member or elsewhere.
    // It is actually possible to use the same name for the real property, if so is desired.
    private int realProp;
  }
</pre>

Behind the scene the compiler transforms this into two methods with the name of the property and the prefixes 
<tt>get_</tt> and <tt>set_</tt>, and with the function decorator 'property'. The following generates the equivalent 
code, and is perfectly valid too:

<pre>
  class MyObj
  {
    int get_prop() const property { return realProp; }
    void set_prop(int value) property { realProp = value; }
    private int realProp;
  }
</pre>

If you implement the property accessors by explicitly writing the two methods you must make sure the return type 
of the get accessor and the parameter type of the set accessor match, otherwise the compiler will not know which 
is the correct type to use.

For interfaces the first alternative is usually the preferred way of declaring the property accessors, as it 
gets quite short and easy to read.

<pre>
  interface IProp
  {
    int prop { get const; set; }
  }
</pre>

You can also leave out either the get or set accessor. If you leave out the set accessor, then the 
property will be read-only. If you leave out the get accessor, then the property will be write-only.

Property accessors can also be implemented for global properties, which follows the same rules, except the 
functions are global.

When the property accessors have been declared it is possible to access them like ordinary properties,
and the compiler will automatically expand the expressions to the appropriate function calls, either 
<tt>set_</tt> or <tt>get_</tt> depending on how the property is used in the expression.

<pre>
  void Func()
  {
    MyObj obj;

    // Set the property value just like a normal property.
    // The compiler will convert this to a call to set_prop(10000).
    obj.prop = 10000;

    // Get the property value just a like a normal property.
    // The compiler will convert this to a call to get_prop().
    assert( obj.prop == 1000 );
  }
</pre>

Observe that as property accessors are actually a pair of methods rather than direct access to the value, 
some restrictions apply as to how they can be used in expressions that inspect and mutate in the same operation. 
Compound assignments can be used on property accessors if the owning object is a reference type, but not if 
the owning object is a value type. This is because the compiler must be able to guarantee that the object stays 
alive between the two calls to the get accessor and set accessor.

The increment and decrement operators are currently not supported. 

In such cases the expression must be expanded so that the read and write operation are performed separately, 
e.g. the increment operator must be rewritten as follows:

<pre>
  a++;     // will not work if a is a virtual property
  a += 1;  // this is OK, as long as the owner of the virtual 
           // property is a reference type or the property is global
</pre>

\section doc_script_class_prop_index Indexed property accessors

Property accessors can be used to emulate a single property or an array of properties accessed through the index 
operator. Property accessors for indexed access work the same way as ordinary property accessors, except that they
take an index argument. The get accessor should take the index argument as the only argument, and the set accessor
should take the index argument as the first argument, and the new value as the second argument.

<pre>
  string firstString;
  string secondString;

  // A global indexed get accessor
  string get_stringArray(int idx) property
  {
    switch( idx )
    {
    case 0: return firstString;
    case 1: return secondString;
    }
    return "";
  }

  // A global indexed set accessor
  void set_stringArray(int idx, const string &in value) property
  {
    switch( idx )
    {
    case 0: firstString = value; break;
    case 1: secondString = value; break;
    }
  }

  void main()
  {
    // Setting the value of the indexed properties
    stringArray[0] = "Hello";
    stringArray[1] = "World";

    // Reading the value of the indexed properties
    print(StringArray[0] + " " + stringArray[1] + "\n");
  }
</pre>

Compound assignments currently doesn't work for indexed properties.

*/
