/**

\page doc_register_type Registering an object type


The are two principal paths to take when registering a new type, either the
type is a reference type that is located in dynamic memory, or the type is a
value type that is located on the stack or locally as members of other objects. 
A reference type support object handles (unless restricted by application) but
cannot be passed by value to application registered functions, a value type
doesn't support handles and can be passed by value or reference to application 
registered functions.

There is no given rule when to use one or the other, but in general you'll use 
reference types when the type must be able to outlive the scope in which it is 
created, and value types when the type is normally used to perform quick 
calculations after which the object can be discarded. If the type is large or 
complex, then it is likely it should be a reference type.

 - \subpage doc_reg_basicref
 - \subpage doc_register_val_type
 - \subpage doc_reg_opbeh
 - \subpage doc_reg_objmeth
 - \subpage doc_reg_objprop







\page doc_reg_basicref Registering a reference type

The basic reference type should be registered with the following behaviours:
\ref asBEHAVE_FACTORY, \ref asBEHAVE_ADDREF, and \ref asBEHAVE_RELEASE. 

\code
// Registering the reference type
r = engine->RegisterObjectType("ref", 0, asOBJ_REF); assert( r >= 0 );
\endcode

\see The \ref doc_addon_any "any" add-on for an example of a reference type.

\see \ref doc_gc_object, \ref doc_adv_class_hierarchy, \ref doc_adv_scoped_type, and \ref doc_adv_single_ref_type for more advanced types.


\section doc_reg_basicref_1 Factory function

The factory function is the one that AngelScript will use to instantiate
objects of this type when a variable is declared. It is responsible for
allocating and initializing the object memory.

The default factory function doesn't take any parameters and should return
an object handle for the new object. Make sure the object's reference counter
is accounting for the reference being returned by the factory function, so
that the object is properly released when all references to it are removed.

\code
CRef::CRef()
{
    // Let the constructor initialize the reference counter to 1
    refCount = 1;
}

CRef *Ref_Factory()
{
    // The class constructor is initializing the reference counter to 1
    return new CRef();
}

// Registering the factory behaviour
r = engine->RegisterObjectBehaviour("ref", asBEHAVE_FACTORY, "ref@ f()", asFUNCTION(Ref_Factory), asCALL_CDECL); assert( r >= 0 );
\endcode

You may also register factory functions that take parameters, which may
then be used when initializing the object.

The factory function must be registered as a global function, but can be
implemented as a static class method, common global function, or a global
function following the generic calling convention.

Even though a factory function returns an object handle, it must not return a null handle 
unless it also sets an exception to signal that the instantiation of the object failed. 

The behaviour is undefined if a factory function returns null without setting an exception.

See also \ref doc_reg_basicref_4.

\section doc_reg_basicref_2 Addref and release behaviours

\code
void CRef::Addref()
{
    // Increase the reference counter
    refCount++;
}

void CRef::Release()
{
    // Decrease ref count and delete if it reaches 0
    if( --refCount == 0 )
        delete this;
}

// Registering the addref/release behaviours
r = engine->RegisterObjectBehaviour("ref", asBEHAVE_ADDREF, "void f()", asMETHOD(CRef,AddRef), asCALL_THISCALL); assert( r >= 0 );
r = engine->RegisterObjectBehaviour("ref", asBEHAVE_RELEASE, "void f()", asMETHOD(CRef,Release), asCALL_THISCALL); assert( r >= 0 );
\endcode

If the instances of this object will be shared between multiple threads, remember
to guarantee that the reference counter is thread safe by making the increments
and decrements with atomic instructions. 

\see \ref doc_adv_multithread

\section doc_reg_nocount Reference types without reference counting

If the application provides its own memory management that isn't based on reference counting,
then it is possible to register the type without the addref and release behaviours if the flag,
asOBJ_NOCOUNT is informed in the call to RegisterObjectType, i.e.

\code
// Registering the reference type
r = engine->RegisterObjectType("ref", 0, asOBJ_REF | asOBJ_NOCOUNT); assert( r >= 0 );
\endcode

Without the addref and release behaviours the application must be careful to not destroy any
objects that may potentially still be referenced by the script engine, e.g. in a global variable,
or other location.

Unless the objects are guaranteed to stay alive as long as the script engine is instantiated, you
may want to consider disabling global variables with engine property \ref asEP_DISALLOW_GLOBAL_VARS. 
This will make it much easier for the application to know where references to the objects are kept.
An alternative to disabling all global variables, is to selectively disallow only the global variables,
that can eventually store a reference to the object type. This can be done by 
\ref asIScriptModule::GetGlobalVarCount "enumerating the compiled global variables" after script has
been built and giving an error to the user in case he includes a variable he shouldn't. 




\section doc_reg_basicref_4 List factory function

The list factory function is a special \ref doc_reg_basicref_1 "factory function" that 
can be registered to allow a type to be created from an initialization list. The list factory
function takes only a single pointer as argument. AngelScript will pass a pointer to the 
initialization list buffer in that argument. The buffer will contain all the values necessary
to create and initialize the object. 

In order for the script engine to know what information must be placed in the buffer the
application must provide the list pattern when registering the list factory. The list pattern
is declared with a special syntax involving datatypes and the following tokens: {, }, ?, repeat, and repeat_same.

The tokens { } are used to declare that the list pattern expects a list of values or a sublist of values. 
The repeat token is used to signal that the next type or sub list can be repeated 0 or more times. 
The repeat_same token is similar to repeat except that it also tells the compiler that every time the same
list is repeated it should have the same length. Any data type can be used in the list pattern, as long 
as it can be passed by value. When a variable type is desired the token ? can be used.

Here's a couple of examples for registering list factories with list patterns:

\code
// The array type can be initialized for example with: intarray a = {1,2,3};
engine->RegisterObjectBehaviour("intarray", asBEHAVE_LIST_FACTORY, 
  "intarray@ f(int &in) {repeat int}", ...);

// The dictionary type can be initialized with: dictionary d = {{'a',1}, {'b',2}, {'c',3}};
engine->RegisterObjectBehaviour("dictionary", asBEHAVE_LIST_FACTORY, 
  "dictionary @f(int &in) {repeat {string, ?}}", ...);
  
// The grid type can be initialized with: grid a = {{1,2},{3,4}};
engine->RgisterObjectBehaviour("grid", asBEHAVE_LIST_FACTORY,
  "grid @f(int &in) {repeat {repeat_same int}}", ...);
\endcode

The list buffer passed to the factory function will be populated using the following rules:

- Whenever the pattern expects a repeat, the buffer will contain a 32bit integer with the 
  number of repeated values that will come afterwards
- Whenever the pattern expects a ?, then the buffer will contain a 32bit integer representing 
  the typeId of the value that comes after.
- Whenever the pattern expects a reference type, the buffer will contain a pointer to the object
- Whenever the pattern expects a value type, the buffer will contain the object itself
- All values in the buffer will be aligned to a 32bit boundary, unless the size of the value placed
  in the buffer is smaller than 32bits.

\see \ref doc_addon_array and \ref doc_addon_dict for example implementations of list factories.
  





\section doc_reg_noinst Registering an uninstantiable reference type

Sometimes it may be useful to register types that cannot be instantiated by
the scripts, yet can be interacted with. You can do this by registering the
type as a normal reference type, but omit the registration of the factory
behaviour. You can later register global properties, or functions that allow the
scripts to access objects created by the application via object handles.

This would be used when the application has a limited number of objects
available and doesn't want to create new ones. For example singletons, or
pooled objects.
















\page doc_register_val_type Registering a value type

When registering a value type, the size of the type must be given so that AngelScript knows how much space is needed for it.
If the type doesn't require any special treatment, i.e. doesn't contain any pointers or other resource references that must be
maintained, then the type can be registered with the flag \ref asOBJ_POD. In this case AngelScript doesn't require the default
constructor, assignment behaviour, or destructor as it will be able to automatically handle these cases the same way it handles
built-in primitives.

If you plan on passing or returning the type by value to registered functions that uses native calling convention, you also
need to inform \ref doc_reg_val_2 "how the type is implemented in the application", but if you only plan on using generic
calling conventions, or don't pass these types by value then you don't need to worry about that.



\code
// Register a primitive type, that doesn't need any special management of the content
r = engine->RegisterObjectType("pod", sizeof(pod), asOBJ_VALUE | asOBJ_POD); assert( r >= 0 );

// Register a class that must be properly initialized and uninitialized
r = engine->RegisterObjectType("val", sizeof(val), asOBJ_VALUE); assert( r >= 0 );
\endcode

\see The \ref doc_addon_std_string or the \ref doc_addon_math "complex type in the math add-on" for examples of value types
\see \ref doc_adv_generic_handle for a more specific example of a value type
\see \ref doc_gc_object for when the type may form circular references when being a member of another type


\section doc_reg_val_1 Constructor and destructor

If a constructor or destructor is needed they shall be registered the following way:

\code
void Constructor(void *memory)
{
  // Initialize the pre-allocated memory by calling the
  // object constructor with the placement-new operator
  new(memory) Object();
}

void Destructor(void *memory)
{
  // Uninitialize the memory by calling the object destructor
  ((Object*)memory)->~Object();
}

// Register the behaviours
r = engine->RegisterObjectBehaviour("val", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(Constructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
r = engine->RegisterObjectBehaviour("val", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(Destructor), asCALL_CDECL_OBJLAST); assert( r >= 0 );
\endcode

Remember to use unique names or namespaces for the wrapper functions, even if you create templated
implementations. Otherwise the linker may end up taking the address of the wrong function when registering 
the wrapper with AngelScript, which is sure to result in unexpected behaviours.

Note that you may need to include the &lt;new&gt; header to declare the placement new operator that is used 
to initialize a preallocated memory block.

\see \ref doc_reg_val_3.




\section doc_reg_val_2 Value types and native calling conventions

If the type will be passed to or from the application by value using native calling conventions it is important to inform
AngelScript of its real type in C++, otherwise AngelScript won't be able to determine exactly how C++ is treating the type in
a parameter or return value. 

To inform AngelScript of actual type in C++ the template function \ref asGetTypeTraits should preferably be used as it
will automatically determine the correct flags to pass to \ref asIScriptEngine::RegisterObjectType "RegisterObjectType" 
together with the asOBJ_VALUE flag. 

\code
// With C++11 the type can be registered with GetTypeTraits
r = engine->RegisterObjectType("complex", sizeof(complex), asOBJ_VALUE | asGetTypeTraits<complex>()); assert( r >= 0 );
\endcode

On some platforms the native calling convention may require further knowledge about the class and its members that \ref asGetTypeTraits
cannot determine in order to work properly. Whether or not the flags are needed depends on the compiler and target platform, but if the flags
are not needed AngelScript will simply ignore them so there is no harm in informing them.

AngelScript lets the application give information that cover the most common variants, e.g. the class should be treated as 
if all members are integers (or non-float primitives), or it should be treated as if all members are floats. It is also possible to inform if the class
has more constructors than the traditional default and copy constructors. This last one normally only has importance if the default and copy constructors 
are defaulted. 

<table border=0 cellspacing=0 cellpadding=0>
<tr><td>\ref asOBJ_APP_CLASS_MORE_CONSTRUCTORS &nbsp; </td><td>The C++ class has additional constructors beyond the default and copy constructors</td></tr>
<tr><td>\ref asOBJ_APP_CLASS_ALLINTS           &nbsp; </td><td>The C++ class members can be treated as if all integers</td></tr>
<tr><td>\ref asOBJ_APP_CLASS_ALLFLOATS         &nbsp; </td><td>The C++ class members can be treated as if all floats or doubles</td></tr>
<tr><td>\ref asOBJ_APP_CLASS_ALIGN8            &nbsp; </td><td>The C++ class contains members that may require 8byte alignment, e.g. a double.</td></tr>
</table>

If the flags that inform about the members are not informed and AngelScript needs them on the platform, you'll get an error message like 
"Don't support passing/returning type 'MyType' by value to application in native calling convention on this platform". 

It is difficult to explain exactly when one or the other should be used as it requires in-depth knowledge of the ABI for the 
respective system, so if you find that you really need to use these flags, make sure you perform adequate testing 
to guarantee that your functions are called correctly by the script engine. If neither of these flags work, and you're 
not able to change the class to work without them, then the only other option is to use the generic calling convention,
preferably with the \ref doc_addon_autowrap "auto wrappers".

\subsection doc_reg_val_2_nocpp11 For compilers that don't support C++11

If your compiler doesn't support C++11 features the \ref asGetTypeTraits function will not be available. In this case you 
have no option but to inform the correct flags manually. Be careful to inform the correct flags, because if the wrong flags
are used you may get unexpected behaviour when calling registered functions that passes or returns these types by value. 
Common problems are stack corruptions or invalid memory accesses. In some cases you may face more silent errors that
may be difficult to detect, e.g. the function is not returning the expected values.

There are a few different flags:

<table border=0 cellspacing=0 cellpadding=0>
<tr><td>\ref asOBJ_APP_CLASS                  &nbsp; </td><td>The C++ type is a class, struct, or union</td></tr>
<tr><td>\ref asOBJ_APP_CLASS_CONSTRUCTOR      &nbsp; </td><td>The C++ type has a default constructor</td></tr>
<tr><td>\ref asOBJ_APP_CLASS_DESTRUCTOR       &nbsp; </td><td>The C++ type has a destructor</td></tr>
<tr><td>\ref asOBJ_APP_CLASS_ASSIGNMENT       &nbsp; </td><td>The C++ type has a copy assignment operator</td></tr>
<tr><td>\ref asOBJ_APP_CLASS_COPY_CONSTRUCTOR &nbsp; </td><td>The C++ type has a copy constructor</td></tr>
<tr><td>\ref asOBJ_APP_PRIMITIVE              &nbsp; </td><td>The C++ type is a C++ primitive, but not a float or double</td></tr>
<tr><td>\ref asOBJ_APP_FLOAT                  &nbsp; </td><td>The C++ type is a float or double</td></tr>
<tr><td>\ref asOBJ_APP_ARRAY                  &nbsp; </td><td>The C++ type is an array</td></tr>
</table>

Note that these don't represent how the type will behave in the script language, only what the real type is in the host 
application. So if you want to register a C++ class that you want to behave as a primitive type in the script language
you should still use the flag \ref asOBJ_APP_CLASS. The same thing for the flags to identify that the class has a constructor, 
destructor, assignment operator, or copy constructor. These flags tell AngelScript that the class has the respective function, 
but not that the type in the script language should have these behaviours.

Observe that the C++ compiler may provide these functions automatically if one of the members of the class is of a type that 
requires it. So even if the type you want to register doesn't have a declared default constructor it may still be necessary to
register the type with the flag asOBJ_APP_CLASS_CONSTRUCTOR. The same for the other functions.

For class types there is also a shorter form of the flags for each combination of the 5 flags. They are of the form \ref asOBJ_APP_CLASS_CDAK, 
where the existence of the last letters determine if the constructor, destructor, and/or assignment behaviour are available. For example
\ref asOBJ_APP_CLASS_CDAK is defined as \ref asOBJ_APP_CLASS | \ref asOBJ_APP_CLASS_CONSTRUCTOR | \ref asOBJ_APP_CLASS_DESTRUCTOR | \ref asOBJ_APP_CLASS_ASSIGNMENT | \ref asOBJ_APP_CLASS_COPY_CONSTRUCTOR.

\code
// Register a complex type that will be passed by value to the application
r = engine->RegisterObjectType("complex", sizeof(complex), asOBJ_VALUE | asOBJ_APP_CLASS_CDAK); assert( r >= 0 );
\endcode





\section doc_reg_val_3 List constructor

The list constructor is similar to \ref doc_reg_basicref_4 "the list factory function" for reference types. 
The constructor will receive a pointer to the initialization list buffer in the exact same way, and the
expected list pattern should be registered in the same way. The difference is that the list constructor 
should be registered like a method, just as done for other \ref doc_reg_val_1 "constructors".

Example registration of a list constructor:

\code
engine->RegisterObjectBehaviour("vector3", asBEHAVE_LIST_CONSTRUCT, "void f(int &in) {float, float, float}", ...);
\endcode

\see \ref doc_addon_math "The complex math add-on" for an example value type with a list constructor.







\page doc_reg_opbeh Registering operator behaviours

In order for AngelScript to know how to work with the application registered types, it is 
necessary to register some behaviours, for example for memory management.

The memory management behaviours are described with the registration of 
\ref doc_reg_basicref "reference types" and \ref doc_register_val_type "value types".

Other advanced behaviours are described with the \ref doc_advanced_api "advanced types".

Most behaviours are implemented as ordinary class methods, except with specific names that the 
compiler can understand.

\section doc_reg_opbeh_1 Operator overloads

In AngelScript all operator overloads are implemented as \ref doc_script_class_ops "class methods with predefined names", 
which is different from C++ where both class methods and global functions may be used. Especially the dual operators, i.e.
those that take two operands, usually has one implemented as a class method, and a global function for the reverse order.

To register C++ operator overloads you'll use the methods described in \ref doc_register_func_1.

Example on how to register operator overloads

\code
class MyClass
{
  ...

  // The operator 'MyClass - int' has been implemented as a method
  MyClass operator-(int) const;

  // The operator 'int - MyClass' has been implemented as a global function
  static MyClass operator-(int, const MyClass &);
}

void RegisterMyClass(asIScriptEngine *engine)
{
  // Registering the operator 'MyClass - int'
  engine->RegisterObjectMethod("MyClass", "MyClass opSub(int) const", asMETHODPR(MyClass, operator-, (int) const, MyClass), asCALL_THISCALL); 

  // Registering the operator 'int - MyClass'
  engine->RegisterObjectMethod("MyClass", "MyClass opSub_r(int) const", asFUNCTIONPR(operator-, (int, const MyClass &), MyClass), asCALL_CDECL_OBJLAST);
}
\endcode








\page doc_reg_objmeth Registering object methods

Class methods are registered with the RegisterObjectMethod call. Both non-virtual and virtual methods are registered the same way.

Static class methods are in reality global functions so those should be \ref doc_register_func "registered as global functions" and not as object methods.

\code
// Register a class method
void MyClass::ClassMethod()
{
  // Do something
}

r = engine->RegisterObjectMethod("mytype", "void ClassMethod()", asMETHOD(MyClass,ClassMethod), asCALL_THISCALL); assert( r >= 0 );
\endcode

It is also possible to register a global function that takes a pointer to
the object as a class method. This can be used to extend the functionality of
a class when accessed via AngelScript, without actually changing the C++
implementation of the class.

\code
// Register a global function as an object method
void MyClass_MethodWrapper(MyClass *obj)
{
  // Access the object
  obj->DoSomething();
}

r = engine->RegisterObjectMethod("mytype", "void MethodWrapper()", asFUNCTION(MyClass_MethodWrapper), asCALL_CDECL_OBJLAST); assert( r >= 0 );
\endcode

\see \ref doc_register_func for more details on how the macros work.

\section doc_reg_objmeth_composite Composite members

If the application class that is being registered uses composition, then it is possible to register the methods
of the composite members like this:

\code
struct Component
{
  int DoSomething();
};

struct Object
{
  Component *comp;
};

r = engine->RegisterObjectMethod("object", "int DoSomething()", asMETHOD(Component, DoSomething), asCALL_THISCALL, 0, asOFFSET(Object, comp), true); assert( r >= 0 );
\endcode

The last parameter indicates that to reach the composite member it is necessary to dereference 
the pointer. If the composite member is inlined, then the parameter should be set to false.








\page doc_reg_objprop Registering object properties

Class member variables can be registered so that they can be directly
accessed by the script without the need for any method calls.

\code
struct MyStruct
{
  int a;
};

r = engine->RegisterObjectProperty("mytype", "int a", asOFFSET(MyStruct,a)); assert( r >= 0 );
\endcode

If a class member is indirect, i.e. the class holds a pointer to the member that is 
allocated on the heap, then it is possible to registered the property using & to tell
the script engine that an dereference is needed to access the member.

\code
struct MyStruct
{
  OtherStruct *a;
};

r = engine->RegisterObjectProperty("mytype", "othertype &a", asOFFSET(MyStruct,a)); assert( r >= 0 );
\endcode

Of course, the application must make sure the pointer is valid during the whole time 
that it may be accessed from the script.

\section doc_reg_objprop_composite Composite members

If the application class that is being registered uses composition, then it is possible to register the properties
of the composite members like this:

\code
struct Component
{
  int a;
};

struct Object
{
  Component *comp;
};

r = engine->RegisterObjectProperty("object", "comp_a", asOFFSET(Component, a), asOFFSET(Object, comp), true); assert( r >= 0 );
\endcode

The last parameter indicates that to reach the property of the composite member it is necessary to dereference 
the pointer. If the composite member is inlined, then the parameter should be set to false.



\section doc_reg_objprop_accessor Property accessors

It is also possible to expose properties through \ref doc_script_class_prop "property accessors", 
which are a pair of class methods with prefixes 'get_' and 'set_' and the function decorator 'property' for
getting and setting the property value. These methods should be registered with \ref doc_register_func "RegisterObjectMethod". 
This is especially useful when the offset of the property cannot be determined, or if the type of the property is 
not registered in the script and some translation must occur, i.e. from <tt>char*</tt> to <tt>string</tt>.

If the application class contains a C++ array as a member, it may be advantageous to expose the array
through \ref doc_script_class_prop "indexed property accessors" rather than attempting to matching the
C++ array type to a registered type in AngelScript. To do this you can create a couple of simple proxy functions
that will translate to the array access.

\note The behaviour of virtual properties can be customized with the engine property \ref doc_adv_custom_options_lang_mod "asEP_PROPERTY_ACCESSOR_MODE".

\code
struct MyStruct
{
  int array[16];
};

// Write a couple of proxy 
int MyStruct_get_array(unsigned int idx, MyStruct *o)
{
  if( idx >= 16 ) return 0;
  return o->array[idx];
}

void MyStruct_set_array(unsigned int idx, int value, MyStruct *o)
{
  if( idx >= 16 ) return;
  o->array[idx] = value;
}

// Register the proxy functions as member methods
r = engine->RegisterObjectMethod("mytype", "int get_array(uint) property", asFUNCTION(MyStruct_get_array), asCALL_CDECL_OBJLAST); assert( r >= 0 );
r = engine->RegisterObjectMethod("mytype", "void set_array(uint, int) property", asFUNCTION(MyStruct_set_array), asCALL_CDECL_OBJLAST); assert( r >= 0 );
\endcode






*/
