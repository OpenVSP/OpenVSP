/**


\page doc_global_variable Variables

Global variables may be declared in the scripts, which will then be shared between 
all contexts accessing the script module.

Variables declared globally like this are accessible from all functions. The 
value of the variables are initialized at compile time and any changes are 
maintained between calls. If a global variable holds a memory resource, e.g. 
a string, its memory is released when the module is discarded or the script engine is reset.

<pre>
  int MyValue = 0;
  const uint Flag1 = 0x01;
</pre>

Variables of primitive types are initialized before variables of non-primitive types.
This allows class constructors to access other global variables already with their
correct initial value. The exception is if the other global variable also is of a 
non-primitive type, in which case there is no guarantee which variable is initialized 
first, which may lead to null-pointer exceptions being thrown during initialization.

Be careful with calling functions that may access global variables from within the 
initialization expression of global variables. While the compiler tries to initialize the
global variables in the order they are needed, there is no guarantee that it will always
succeed. Should a function access a global variable that has not yet been initialized you
will get unpredictable behaviour or a null-pointer exception.




\page doc_global_virtprop Virtual properties

Virtual properties is a property with special behaviour for reading and writing to it. 
These can be declared globally, but are most usually found as members of classes.

<pre>
  int prop
  {
    get { return SomeValue(); }
    set { UpdateValue(value); }
  }
</pre>

\see \ref doc_script_class_prop

 





\page doc_global_interface Interfaces

An interface works like a contract, the classes that implements an interface
are guaranteed to implement the methods declared in the interface. This allows
for the use of polymorphism in that a function can specify that it wants an
object handle to an object that implements a certain interface. The function
can then call the methods on this interface without having to know the
exact type of the object that it is working with.

<pre>
  // The interface declaration
  interface MyInterface
  {
    void DoSomething();
  }

  // A class that implements the interface MyInterface
  class MyClass : MyInterface
  {
    void DoSomething()
    {
      // Do something
    }
  }
</pre>

A class can implement multiple interfaces; Simply list all the interfaces
separated by a comma.






\page doc_global_import Imports

Sometimes it may be useful to load script modules dynamically without having to recompile 
the main script, but still let the modules interact with each other. In that case the script 
may import functions from another module. This declaration is written using the import 
keyword, followed by the function signature, and then specifying which module to import from.

This allows the script to be compiled using these imported functions, without them actually 
being available at compile time. The application can then bind the functions at a later time, 
and even unbind them again.

If a script is calling an imported function that has not yet been bound the script will be 
aborted with a script exception.

<pre>
  import void MyFunction(int a, int b) from "Another module";
</pre>





\page doc_global_enums Enums

Enums are a convenient way of registering a family of integer constants that may be used throughout the script 
as named literals instead of numeric constants. Using enums often help improve the readability of the code, as
the named literal normally explains what the intention is without the reader having to look up what a numeric value
means in the manual.

Even though enums list the valid values, you cannot rely on a variable of the enum type to only contain values
from the declared list. Always have a default action in case the variable holds an unexpected value.

The enum values are declared by listing them in an enum statement. Unless a specific value is given for an enum 
constant it will take the value of the previous constant + 1. The first constant will receive the value 0, 
unless otherwise specified.

<pre>
  enum MyEnum
  {
    eValue0,
    eValue2 = 2,
    eValue3,
    eValue200 = eValue2 * 100
  }
</pre>






\page doc_global_typedef Typedefs

Typedefs are used to define aliases for other types.

Currently a typedef can only be used to define an alias for primitive types, but a future version will have
more complete support for all kinds of types.

<pre>
  typedef float  real32;
  typedef double real64;
</pre>



\page doc_global_funcdef Funcdefs

Funcdefs are used to define a function signature that will be used to store pointers to 
functions with matching signatures. With this a function pointer can be created, which is 
able to store dynamic pointers that can be invoked at a later time as a normal function call.

<pre>
  // Define a function signature for the function pointer
  funcdef bool CALLBACK(int, int);
</pre>

\see \ref doc_datatypes_funcptr for more information on how to use this




\page doc_global_namespace Namespaces

Namespaces can be used to organize large projects in logical units that may be easier 
to remember. When using namespaces it is also not necessary to worry about using names
for entities that may exist in a different part of the project under a different namespace.

<pre>
  namespace A
  {
    // Entities in a namespace see each other normally.
    void function() { variable++; }
    int variable;
  }

  namespace B
  {
    // Entities in different namespaces don't immediately see each other and 
    // can reuse the same name without causing name conflicts. By using the 
    // scoping operator the entity from the desired namespace can be explicitly
    // informed.
    void function() { A::function(); }
  }
</pre>

Observe that in order to refer to an entity from a different namespace the scoping operator
must be used, unless it is a parent namespace in which case it is only necessary if the child 
namespace declare the same entity.

<pre>
  int var;
  namespace Parent
  {
    int var;
    namespace Child
    {
      int var;
      void func()
      {
        // Accessing variable in parent namespace requires 
        // specifying the scope if an entity in a child namespace
        // uses the same name
        var = Parent::var;

        // To access variables in global scope the scoping 
        // operator without any name should be used
        Parent::var = \::var;
      }
    }
  }
  
  void func()
  {
    // Access variable in a nested namespace requires 
    // fully qualified scope specifier
    int var = Parent::Child::var;
  }
</pre>

*/
