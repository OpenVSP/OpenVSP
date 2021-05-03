/**

\page doc_register_func Registering a function

This article aims to explain the way functions are registered with AngelScript, and some of the 
differences between C++ and AngelScript that the developer needs to be aware of in order to be 
successful in registering the application interface that the scripts will use. The principles
learned here are used in several locations, such as \ref asIScriptEngine::RegisterGlobalFunction 
"RegisterGlobalFunction", \ref asIScriptEngine::RegisterObjectMethod "RegisterObjectMethod", \ref
asIScriptEngine::RegisterObjectBehaviour "RegisterObjectBehaviour", etc.

\section doc_register_func_1 How to get the address of the application function or method

The macros \ref asFUNCTION, \ref asFUNCTIONPR, \ref asMETHOD, and \ref asMETHODPR 
have been implemented to facilitate the task of getting the function pointer and 
passing them on to the script engine.

The asFUNCTION takes the function name as the parameter, which works for all global 
functions that do not have any overloads. If you use overloads, i.e. multiple functions
with the same name but with different parameters, then you need to use asFUNCTIONPR instead.
This macro takes as parameter the function name, parameter list, and return type, so that
the C++ compiler can resolve exactly which overloaded function to take the address of.

\code
// Global function
void globalFunc();
r = engine->RegisterGlobalFunction("void globalFunc()", asFUNCTION(globalFunc), asCALL_CDECL); assert( r >= 0 );

// Overloaded global functions
void globalFunc2(int);
void globalFunc2(float);
r = engine->RegisterGlobalFunction("void globalFunc2(int)", asFUNCTIONPR(globalFunc2, (int), void), asCALL_CDECL); assert( r >= 0 );
\endcode

The same goes for asMETHOD and asMETHODPR. The difference between these and asFUNCTION/asFUNCTIONPR
is that the former take the class name as well as parameter.

\code
class Object
{
  // Class method
  void method();
  
  // Overloaded method
  void method2(int input);
  void method2(int input, int &output);
  
  // Const method
  int getAttr(int) const;
};

// Registering the class method
r = engine->RegisterObjectMethod("object", "void method()", asMETHOD(Object,method), asCALL_THISCALL); assert( r >= 0 );

// Registering the overloaded methods
r = engine->RegisterObjectMethod("object", "void method2(int)", asMETHODPR(Object, method2, (int), void), asCALL_THISCALL); assert( r >= 0 );
r = engine->RegisterObjectMethod("object", "void method2(int, int &out)", asMETHODPR(Object, method2, (int, int&), void), asCALL_THISCALL); assert( r >= 0 );

// Registering a const method
r = engine->RegisterObjectMethod("object", "int getAttr(int) const", asMETHODPR(Object, getAttr, (int) const, int), asCALL_THISCALL); assert( r >= 0 );
\endcode

\note asMETHOD doesn't work well for classes with multiple inheritances. On some compilers the method pointer ends up referring to the wrong base class due to a limitation in the C++ compiler. The solution then is to use the asMETHODPR macro.

It is possible to register a class method to be called from the script as if it was a global function. This is commonly done when exposing a singleton 
to the script interface, as the singleton's methods then look like ordinary global functions. When this is done the application must have a reference to
the object at the time of the registration and the application must guarantee that the object is alive until it is no longer possible for the scripts
to call the method.

\code
class MySingleton
{
  // Class method to be called from script as if a global function
  void MyGlobalFunc(int arg1, int arg2);
};

MySingleton single;

// Registering the singleton's method as if a global function
r = engine->RegisterGlobalFunction("void MyGlobalFunc(int, int)", asMETHOD(MySingleton, MyGlobalFunc), asCALL_THISCALL_ASGLOBAL, &single); assert( r >= 0 );
\endcode



\section doc_register_func_2 Calling convention

AngelScript accepts most common calling conventions that C++ uses, i.e. cdecl, stdcall, and thiscall. There is also a 
generic calling convention that can be used for example when native calling conventions are not supported on the target platform.

All functions and behaviours must be registered with the \ref asCALL_CDECL, \ref asCALL_STDCALL, \ref asCALL_THISCALL, or 
\ref asCALL_GENERIC flags to tell AngelScript which calling convention the application function uses. The special conventions 
\ref asCALL_CDECL_OBJLAST and \ref asCALL_CDECL_OBJFIRST can also be used wherever asCALL_THISCALL is accepted, in order to 
simulate a class method through a global function. Functor objects can also be used to emulate global functions with the 
convention \ref asCALL_THISCALL_ASGLOBAL, or class methods with the conventions \ref asCALL_THISCALL_OBJFIRST and \ref asCALL_THISCALL_OBJLAST.

If the incorrect calling convention is given on the registration you'll very likely see the application crash with 
a stack corruption whenever the script engine calls the function. cdecl is the default calling convention for all global 
functions in C++ programs, so if in doubt try with asCALL_CDECL first. The calling convention only differs from cdecl if the 
function is explicitly declared to use a different convention, or if you've set the compiler options to default to another
convention.

For class methods there is only the thiscall convention, except when the method is static, as those methods are in truth global
functions in the class namespace. Normal methods, virtual methods, and methods for classes with multiple inheritance are all
registered the same way, with asCALL_THISCALL. 

Classes with \ref doc_register_func_4 "virtual inheritance are not supported natively", and for these it will be necessary
to create wrapper functions. These wrapper functions can either be implemented manually, or the template based automatic 
wrappers from the \ref doc_addon_autowrap "add-on" can be used.

\see \ref doc_generic



\section doc_register_func_3 A little on type differences

AngelScript supports most of the same types that C++ has, but there are differences that you'll need to know when registering
functions, methods, and behaviours. Make sure you read and understand the article \ref doc_as_vs_cpp_types.

When registering functions that take references, you must make sure to inform the correct keyword that informs the intention of 
the data in the reference. For example, a parameter reference that is meant to be used as input should have the keyword 'in' 
defined after the &amp; character, and an output reference should have the keyword 'out'. \ref doc_reg_basicref "Reference types" 
can be passed as both input and output references, in which case the keyword 'inout' can be used, or simply no keyword at all.
\ref doc_register_val_type "Value types" on the other hand cannot use 'inout' references, as AngelScript cannot guarantee that 
the reference will be valid during the whole execution of the function.

When registering functions that take pointers, you need to determine what the pointer represents. If the pointer
is to a value type though, then it can only be registered as a reference. If the pointer is to a reference type, then it can 
be registered as an \ref doc_obj_handle "object handle", or as a plain reference. If you choose to use the object handle then
you need to pay attention to the reference counter in the type so you don't get problems with memory leaks or crashes due to
objects being destroyed too early.





\section doc_register_func_4 Virtual inheritance is not supported

Registering class methods for classes with virtual inheritance is not supported due to the high complexity involved with them. 
Each compiler implements the method pointers for these classes differently, and keeping the code portable would be very difficult.
This is not a great loss though, as classes with virtual inheritance are relatively rare, and it is easy to write simple proxy 
functions where the classes to exist. 

\code
class A { void SomeMethodA(); };
class B : virtual A {};
class C : virtual A {};
class D : public B, public C {};

// Need a proxy function for registering SomeMethodA for class D
void D_SomeMethodA_proxy(D *d)
{
  // The C++ compiler will resolve the virtual method for us
  d->SomeMethodA();
}

// Register the global function as if it was a class method, 
// but with calling convention asCALL_CDECL_OBJLAST
engine->RegisterObjectMethod("D", "void SomeMethodA()", asFUNCTION(D_SomeMethodA_proxy), asCALL_CDECL_OBJLAST);
\endcode

If you have a lot of classes with virtual inheritance, you should probably think about writing a template proxy function, so
you don't have to manually write all the proxy functions. 






*/
