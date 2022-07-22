/**

\page doc_obj_handle Object handles to the application


In AngelScript an object handle is a reference counted pointer to an object. In the scripts 
they are used to pass the objects around by reference instead of by value. Depending on how
an application type is registered the type will support handles.

\see \ref doc_register_type, \ref doc_script_handle in the script language





\section doc_obj_handle_3 Managing the reference counter in functions

Whenever the object handle is passed by value from the application to the script engine, 
or vice versa its reference should be accounted for. This means that the application must 
release any object handles it receives as parameters when it no longer needs them, it also 
means that the application must increase the reference counter for any object handle being 
returned to the script engine. This also applies to the \ref doc_generic 
"generic calling convention".

A function that creates an object and returns it to the script engine might look like this:

\code
// Registered as "obj@ CreateObject()"
obj *CreateObject()
{
  // The constructor already initializes the ref count to 1
  return new obj();
}
\endcode

A function that receives an object handle from the script and stores it in a global variable might look like this:

\code
// Registered as "void StoreObject(obj@)"
obj *o = 0;
void StoreObject(obj *newO)
{
  // Release the old object handle
  if( o ) o->Release();

  // Store the new object handle
  o = newO;
}
\endcode

A function that retrieves a previously stored object handle might look like this:

\code
// Registered as "obj@ RetrieveObject()"
obj *RetrieveObject()
{
  // Increase the reference counter to account for the returned handle
  if( o ) o->AddRef();

  // It is ok to return null if there is no previous handle stored
  return o;
}
\endcode

A function that receives an object handle in the parameter, but doesn't store it looks like this:

\code
// Registered as "void DoSomething(obj@)"
void DoSomething(obj *o)
{
  // When finished with the object it must be released
  if( o ) o->Release();
}
\endcode





\section doc_obj_handle_4 Auto handles can make it easier

The application can use auto handles (\@+) to alleviate some of the work of managing the reference counter. 
When registering the function or method with AngelScript, add a plus sign to the object handles that 
AngelScript should automatically manage. For parameters AngelScript will then release the reference after 
the function returns, and for the return value AngelScript will increase the reference on the returned 
pointer. The reference for the returned value is increased before the parameters are released, so it is 
possible to have the function return one of the parameters.

\code
// Registered as "obj@+ ChooseObj(obj@+, obj@+)"
obj *ChooseObj(obj *a, obj *b)
{
  // Because of the auto handles AngelScript will
  // automatically manage the reference counters
  return some_condition ? a : b;
}
\endcode

The auto handles works the same way for the \ref doc_generic "generic calling convention".

*/
