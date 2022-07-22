/**

\page doc_as_vs_cpp_types Datatypes in AngelScript and C++


\section doc_as_vs_cpp_types_1 Primitives

Primitives in AngelScript have direct matches in C++.

<table border=0 cellspacing=0 cellpadding=0>
<tr><td width=100><b>AngelScript</b></td><td width=150><b>C++</b></td><td width=100><b>Size (bits)</b></tr>
<tr><td>void  </td><td>void              </td><td>0     </td></tr>
<tr><td>int8  </td><td>signed char       </td><td>8     </td></tr>
<tr><td>int16 </td><td>signed short      </td><td>16    </td></tr>
<tr><td>int   </td><td>signed int (*)    </td><td>32    </td></tr>
<tr><td>int64 </td><td>signed int64_t    </td><td>64    </td></tr>
<tr><td>uint8 </td><td>unsigned char     </td><td>8     </td></tr>
<tr><td>uint16</td><td>unsigned short    </td><td>16    </td></tr>
<tr><td>uint  </td><td>unsigned int (*)  </td><td>32    </td></tr>
<tr><td>uint64</td><td>unsigned uint64_t </td><td>64    </td></tr>
<tr><td>float </td><td>float             </td><td>32    </td></tr>
<tr><td>double</td><td>double            </td><td>64    </td></tr>
<tr><td>bool  </td><td>bool              </td><td>8 (**)</td></tr>
</table>


(*) An int can actually be of varying size on different platforms in C++, but most commonly it is 32 bits long. 
    AngelScript will always assume 32 bits though.

(**) On 32 bit PowerPC platforms the bool type commonly have the size of 32 bit,
     when compiled on such platforms AngelScript also uses 32 bits for the bool type

\section doc_as_vs_cpp_types_5 Strings

AngelScript expects the application to register its own \ref doc_strings "string type",
so the string types should match perfectly. 

The char* string type that is so convenient in C++ is however very difficult to use
in a scripted environment where you do not have full control over how it is used. For that
reason it is recommended that you wrap any functions that use the char* string type so
that the string is properly converted to an object that can be safely handled by both
the application and the script engine, e.g. std::string or another class of your preference.

\section doc_as_vs_cpp_types_2 Arrays

AngelScript also expects the application to register the type that should be used for 
\ref doc_arrays "dynamic arrays". Normally this is done by registering the \ref doc_addon_array add-on, but 
the application is free to do it differently.

It is also possible to have different object types for different array types, so the 
application can match the array type exactly with the types used in C++.

\section doc_as_vs_cpp_types_3 Object handles

The AngelScript object handles are reference counted pointers to objects.
This means that for object handles to work, the object must have some way of
counting references, for example an AddRef/Release method pair.

When AngelScript passes an object handle by value to a function it
increases the reference count to count for the argument instance, thus the
function is responsible for releasing the reference once it is finished with
it. In the same manner AngelScript expects any handle returned from a function
to already have the reference accounted for.

However, when registering functions/methods with AngelScript the
application can tell the library that it should automatically take care of
releasing the handle references once a function return, likewise for returned
handles. This is done by adding a + sign to the \@ type modifier. When doing
this an object handle can be safely passed to a C++ function that expects a
normal pointer, but don't release it afterwards.

\see \ref doc_obj_handle

\section doc_as_vc_cpp_types_5 Script classes and interfaces

All script classes and interfaces are seen as the \ref asIScriptObject type by
the application. The \ref asIScriptObject interface has methods to determine
the actual type of the script class or interface, as well as to interact with the
actual object instance.

\see \ref doc_use_script_class

\section doc_as_vc_cpp_types_6 Function pointers

All script function pointers are seen as the \ref asIScriptFunction type by 
the application. The \ref asIScriptFunction type has methods to obtain the name
of the function and the parameter and return types, etc. 

\section doc_as_vs_cpp_types_4 Parameter references

Because AngelScript needs to guarantee validity of pointers at all times,
it doesn't always pass references to the true object to the function
parameter. Instead it creates a copy of the object, whose reference is passed
to the function, and if the reference is marked to return a value, the clone
is copied back to the original object (if it still exists) once the function
returns.

Because of this, AngelScript's parameter references are mostly compatible
with C++ references, or pointers, except that the address normally shouldn't be
stored for later use, since the object may be destroyed once the function returns.

If it is necessary to store the address of the object, then object handles
should be used instead.

<table border=0 cellspacing=0 cellpadding=0>
<tr><td width=100 valign=top><b>Reference</b></td><td valign=top><b>Description</b></td></tr>
<tr><td valign=top>&in</td><td>A copy of the value is always taken and the 
reference to the copy is passed to the function. For script functions this is not 
useful, but it is maintained for compatibility with application registered functions.</td></tr>
<tr><td valign=top>const &in</td><td>If the life time of the value can be 
guaranteed to be valid during the execution of the function, the reference to the 
true object is passed to the function, otherwise a copy is made.</td></tr>
<tr><td valign=top>&out</td><td>A reference to an unitialized value is passed 
to the function. When the function returns the value is copied to the true reference. 
The argument expression is evaluated only after the function call. This is the best 
way to have functions return multiple values.</td></tr>
<tr><td valign=top>const &out</td><td>Useless as the function wouldn't be able 
to modify the value.</td></tr>
<tr><td valign=top>&inout</td><td>The true reference is always passed to the 
function. Only objects that support object handles can be used with this type
as they can always be guaranteed to stay alive during the call.</td></tr>
<tr><td valign=top>const &inout</td><td>The referred to object will be read-only.</td></tr>
</table>

If the application wants parameter references that work like they do in C++, 
then this can be allowed by \ref doc_adv_custom_options_lang_mod "setting an engine property".





*/
