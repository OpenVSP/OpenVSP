/**

\page doc_adv_template Template types

A template type in AngelScript works similarly to how templates work in C++. The scripts 
will be able to instantiate different forms of the template type by specifying which subtype 
that should be used. The methods for the instance will then be adapted to this subtype, so
that the correct handling of parameters and return types will be applied.

The implementation of the template type is not a C++ template though, instead it must 
be implemented as a generic class that can determine what to do dynamically at runtime based
on the subtype for which it was instantiated. This is obviously a lot less efficient than
having specific implementations for each type, and for that reason AngelScript permits the
application to register a template specialization where the extra performance is needed. 

This gives the best of both worlds, performance where the subtype is known before hand, and 
support for all other types that cannot be pre-determined.

\section doc_adv_template_1 Registering the template type

Template types can be either \ref doc_reg_basicref "reference types" or \ref doc_register_val_type "value types". 
Both are registered in a similar manner with only a few differences.

The name of the type is formed by the name of the template type plus the name of the subtype with angle brackets. 
Multiple subtypes can be informed, separated by comma. The type flag asOBJ_TEMPLATE must used to tell AngelScript 
that it is a template type that is being registered.

\code
// Register the template type as a garbage collected reference type
r = engine->RegisterObjectType("myTemplate<class T>", 0, asOBJ_REF | asOBJ_GC | asOBJ_TEMPLATE); assert( r >= 0 );

// Register another template type as a value type
r = engine->RegisterObjectType("myValueTemplate<class T>", sizeof(MyValueTempl), asOBJ_VALUE | asOBJ_TEMPLATE | asGetTypeTraits<MyValueTempl>()); assert( r >= 0 );
\endcode

The template type doesn't have to be \ref doc_gc_object "garbage collected", but since you may not know 
which subtypes it will be instantiated for, it is usually best to implement that support.

When registering the behaviours, methods, and properties for the template type the type is identified
with the name and subtype within angle brackets, but without the class token, e.g. <tt>myTemplate&lt;T&gt;</tt>.
The sub type is identified by just the name of the subtype as it was declared in the call to RegisterObjectType.

The factory/construct behaviour for the template type is also different. In order for the implementation to know
which subtype it is instantiated for, the factory/constructor receives the \ref asITypeInfo of the template instance
as a hidden first parameter. When registering the factory/constructor this hidden parameter is reflected in the declaration,
for example as <tt>int &amp;in</tt>.

\code
// Register the factory behaviour
r = engine->RegisterObjectBehaviour("myTemplate<T>", asBEHAVE_FACTORY, "myTemplate<T>@ f(int&in)", asFUNCTIONPR(myTemplateFactory, (asITypeInfo*), myTemplate*), asCALL_CDECL); assert( r >= 0 );

// Register the construct behaviour
r = engine->RegisterObjectBehaviour("myValueTemplate<T>", asBEHAVE_CONSTRUCT, "void f(int&in)", asFUNCTIONPR(myValueTemplConstructor, (asITypeInfo*, void*), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
\endcode

The list factory/constructor, used to instantiate objects with initialization lists, is registered in the same way, i.e.:

\code
// Register the list factory behaviour
r = engine->RegisterObjectBehaviour("myTemplate<T>", asBEHAVE_LIST_FACTORY, "myTemplate<T>@ f(int&in, uint)", asFUNCTIONPR(myTemplateListFactory, (asITypeInfo*, unsigned int), myTemplate*), asCALL_CDECL); assert( r >= 0 );

// Register the list constructor behaviour
r = engine->RegisterObjectBehaviour("myValueTemplate<T>", asBEHAVE_LIST_CONSTRUCT, "void f(int&in, uint)", asFUNCTIONPR(myValueTemplListConstruct, (asITypeInfo*, unsigned int, void*), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
\endcode


Remember that since the subtype must be determined dynamically at runtime, it is not possible to declare
functions to receive the subtype by value, nor to return it by value. Instead you'll have to design the
methods and behaviours to take the type by reference. It is possible to use object handles, but then
the script engine won't be able to instantiate the template type for primitives and other values types.

The same goes for object properties. Templates can have properties just like any other class, but the
properties must not be of the template subtype, since it is not known at the time of registration the size
of this type.

\see \ref doc_addon_array

\subsection doc_adv_template_1_1 On subtype replacement for template instances

When a template type is instanced in a declaration, e.g. a variable, the compiler enumerates all the members
of the template type to verify if any subtype is used which requires replacement. In most cases the replacement
is a direct one-to-one mapping, but in cases where the subtype is used as a const parameter reference, then
an additional instruction may be needed to get the expected behaviour.

The following shows a method registered to take the subtype T as a const ref. 

\code
r = engine->RegisterObjectMethod("array<T>", "int find(const T&in value) const", ...); 
\endcode

If this template is instantiated with a handle as a subtype, e.g. <tt>array<Obj\@></tt>, then the method will become:

<pre>
  int find(Obj \@const &in value) const
</pre>

This means that that the parameter takes the handle to a non-read only Obj. The actual handle cannot be 
modified, but the object the handle refers to can still be modified by the method. This in turn makes it
impossible for a script to call the method if the handle the script has is read only. 

To allow the application developer to say that the method should allow handles to read only objects, a special
keyword <tt>if_handle_then_const</tt> should be used.

\code
r = engine->RegisterObjectMethod("array<T>", "int find(const T&in if_handle_then_const value) const", ...); 
\endcode

Now this becomes:

<pre>
  int find(const Obj \@const &in value) const
</pre>

This means that the parameter takes a const handle to a read only Obj, i.e. both the handle itself and the 
object instance it refers to cannot be modified by the method. Now the script will be able to call the method
both with read only handles and non-read only handles.




\section doc_adv_template_4 Validating template instantiations at compile time

In order to avoid unnecessary runtime validations of invalid template instantiations, the application 
should preferably register the \ref asBEHAVE_TEMPLATE_CALLBACK behaviour. This is a special behaviour function
that the script engine will invoke every time a new template instance type is generated. The callback
function can then perform necessary validations to verify if the type can be handled, and if not tell
the engine that the instance isn't supported. 

The callback function must be a global function that receives an asITypeInfo pointer, and should return 
a boolean. If the template instance is valid the return value should be true. 

The function should also take a second parameter with an output reference to a boolean. This parameter 
should be set to true by the function if the template instance should not be garbage collected, which will
make AngelScript clear the asOBJ_GC flag for the object type. If the template 
instance cannot form any circular references, then it doesn't need to be garbage collected, which reduces
the work that has to be done by the garbage collector. 

\code
// Register the template callback
// Observe that the asITypeInfo pointer argument is represented by the int reference
r = engine->RegisterObjectBehaviour("myTemplate<T>", asBEHAVE_TEMPLATE_CALLBACK, "bool f(int &in, bool&out)", asFUNCTION(myTemplateCallback), asCALL_CDECL); assert( r >= 0 );
\endcode

Here's an example callback function:

\code
bool myTemplateCallback(asITypeInfo *ot, bool &dontGarbageCollect)
{
  // This template will only support primitive types
  int typeId = ot->GetSubTypeId();
  if( typeId & asTYPEID_MASK_OBJECT )
  {
    // The script is attempting to instantiate the 
    // template with an object type, this is not allowed.
    return false;
  }
  
  // Tell AngelScript that this instance doesn't require garbage collection
  dontGarbageCollect = true;
    
  // Primitive types are allowed
  return true;
}
\endcode



\section doc_adv_template_2 Template specializations

When registering a template specialization you override the template instance that AngelScript would normally
do when compiling a declaration with the template type. This allow the application to register a completely
different object with its own implementation for template specializations. Obviously it is recommended that
the template specialization is registered so that to the script writer it is transparent, i.e. try to avoid
having different method names or behaviours for the template type and template specializations.

With the exception of the type name, a template specialization is registered exactly like a \ref doc_register_type "normal type". 
The template specialization must be registered in the same namespace as the template itself.

\code
// Register a template specialization for the float subtype
r = engine->RegisterObjectType("myTemplate<float>", 0, asOBJ_REF); assert( r >= 0 );
  
// Register the factory (there are no hidden parameters for specializations)
r = engine->RegisterObjectBehaviour("myTemplate<float>", asBEHAVE_FACTORY, "myTemplate<float>@ f()", asFUNCTIONPR(myTemplateFloatFactory, (), myTemplateFloat*), asCALL_CDECL); assert( r >= 0 );
\endcode

 




 


*/
