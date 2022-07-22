/**

\page doc_use_script_class Using script classes 

When there are multiple objects controlled by the same script implementation it may be favourable to use script classes,
rather than global script functions. Using script classes each instance can have it's own set of variables within the 
class, contrary to the global functions that needs to rely on global variables to store persistent information. 

Of course, it would be possible to duplicate the script modules, so that there is one module for each object instance, but
that would be impose a rather big overhead for the application. Script classes don't have that overhead, as all instances
share the same module, and thus the same bytecode and function ids, etc. 





\section doc_use_script_class_1 Instantiating the script class

Before instantiating the script class you need to know which class to instantiate. Exactly how this is done 
depends on the application, but here are some suggestions.

If the application knows the name of the class, either hardcoded or from some configuration, the class type
can easily be obtained by calling the module's \ref asIScriptModule::GetTypeIdByDecl "GetTypeIdByDecl" with the name
of the class. The application can also choose to identify the class through some properties of the class, e.g.
if the class implements a predefined \ref asIScriptEngine::RegisterInterface "interface". Then the application 
can enumerate the class types implemented in the script with \ref asIScriptModule::GetObjectTypeByIndex "GetObjectTypeByIndex"
and then examine the type through the \ref asITypeInfo interface.

A third option, if you're using the \ref doc_addon_build "script builder add-on", is to use the metadata to identify
the class. If you choose this option, use the \ref asIScriptModule to enumerate the declared types and then query the
\ref doc_addon_build "CScriptBuilder" for their metadata.

Once the object type is known you create the instance by calling the class' factory function, passing it the necessary
arguments, e.g. a pointer to the application object which the script class should be bound to. The factory function id
is found by querying the \ref asITypeInfo. 

\code
// Get the object type
asIScriptModule *module = engine->GetModule("MyModule");
asITypeInfo *type = module->GetTypeInfoByDecl("MyClass");

// Get the factory function from the object type
asIScriptFunction *factory = type->GetFactoryByDecl("MyClass @MyClass()");

// Prepare the context to call the factory function
ctx->Prepare(factory);

// Execute the call
ctx->Execute();

// Get the object that was created
asIScriptObject *obj = *(asIScriptObject**)ctx->GetAddressOfReturnValue();

// If you're going to store the object you must increase the reference,
// otherwise it will be destroyed when the context is reused or destroyed.
obj->AddRef();
\endcode

The factory function is \ref doc_call_script_func "called as a regular global function" and returns a handle to 
the newly instanciated class.





\section doc_use_script_class_2 Calling a method on the script class

Calling the methods of the script classes are similar to \ref doc_call_script_func "calling global functions" except
that you obtain the function id from the \ref asITypeInfo, and you must set the object pointer along with the 
rest of the function arguments.

\code
// Obtain the function object that represents the class method
asIScriptFunction *func = type->GetMethodByDecl("void method()");

// Prepare the context for calling the method
ctx->Prepare(func);

// Set the object pointer
ctx->SetObject(obj);

// Execute the call
ctx->Execute();
\endcode





\section doc_use_script_class_3 Receiving script classes

In order for the application to register a function that receives a script class it must first know the type. Of course,
since the class is declared in the script it isn't possible to know the type before the script is compiled. Instead the 
application can register an \ref doc_global_interface "interface" with the engine. The function can then be registered to receive a handle to that interface.

\code
// Register an interface
engine->RegisterInterface("IMyObj");

// You can also register methods with the interface if you wish to force the script class to implement them
engine->RegisterInterfaceMethod("IMyObj", "void RequiredMethod()");

// Register a function that receives a handle to the interface
engine->RegisterGlobalFunction("void ReceiveMyObj(IMyObj @obj)", asFUNCTION(ReceiveMyObj), asCALL_CDECL);
\endcode

The function that receives the interface should be implemented to take a pointer to an \ref asIScriptObject. 

\code
asIScriptObject *gObj = 0;
void ReceiveMyObj(asIScriptObject *obj)
{
  // Do something with the object
  if( obj )
  {
    if( doStore )
    {
      // If the object is stored, we shouldn't release the handle
      gObj = obj;
    }
    else
    {
      // If the object is not stored, we must release the handle before returning
      obj->Release();
    }
  }
}
\endcode

If you don't want to use interfaces like this, then you may want to look into the
\ref doc_adv_var_type "variable argument type" or the generic \ref doc_addon_handle "script handle add-on", 
which are ways that can be used to receive values and objects of which the type is not known beforehand.



\section doc_use_script_class_4 Returning script classes

Returning a script class from a registered function involves much of the same as 
\ref doc_use_script_class_3 "receiving them". In order to register the function either an interface
needs to be used, or the generic \ref doc_addon_handle "script handle add-on" can
be used. 

\code
// The global variable is initialized elsewhere
asIScriptObject *gObj;

asIScriptObject *ReturnMyObj()
{
  if( gObj == 0 )
    return 0;

  // Increase the refcount to account for the returned handle
  gObj->AddRef();
  return gObj;
}
\endcode

This function can be registered as following:

\code
// Register an interface
engine->RegisterInterface("IMyObj");

// Register a function that returns a handle to the interface
engine->RegisterGlobalFunction("IMyObj @ReturnMyObj()", asFUNCTION(ReturnMyObj), asCALL_CDECL);
\endcode

*/
