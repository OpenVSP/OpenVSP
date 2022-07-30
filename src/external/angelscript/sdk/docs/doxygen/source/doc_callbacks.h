/**

\page doc_callbacks Funcdefs and script callback functions

\ref doc_global_funcdef "Funcdefs" are used to define a function signature for callbacks. This funcdef is then
used to declare variables or function parameters that can hold handles to functions of matching signature.

The application can also \ref asIScriptEngine::RegisterFuncdef "register funcdefs" as part of the application 
interface if the intention is for the script to set callbacks that will be called from the application. Once 
this is done, the application can receive the function handles as an asIScriptFunction pointer which can
then be \ref doc_call_script_func "executed" normally.

\section doc_callbacks_example An example

Let's say the application needs to allow the script to set a callback that will then be called at some event. To 
do this the application would first register the funcdef that defines the signature of the callback. Then it needs 
to register the function that will be used to set the callback from the script.

\code
  // Register a simple funcdef for the callback
  engine->RegisterFuncdef("void CallbackFunc()"); 
  
  // Register a function for setting the callback
  engine->RegisterGlobalFunction("void SetCallback(CallbackFunc @cb)", asFUNCTION(SetCallback), asCALL_CDECL);  
\endcode

With this interface, the script would be able to inform the callback like this:

<pre>
  void main()
  {
    // Tell the application what script function to call
    SetCallback(MyCallback);
  }
  
  // The signature matches the registered CallbackFunc funcdef
  void MyCallback()
  {
    ...
  }
</pre>

The implementation for the SetCallback function might look something like this.

\code
  // The callback is a script function.
  // Don't forget to release this before cleaning up the engine.
  asIScriptFunction *callback = 0;
  
  void SetCallback(asIScriptFunction *cb)
  {
    // Release the previous callback, if any
    if( callback )
      callback->Release();

    // Store the received handle for later use
    callback = cb;

    // Do not release the received script function 
    // until it won't be used any more
  }
\endcode

To call the actual callback when it is time, the application uses the script context just
like for any other \ref doc_call_script_func "call to a script function".

\section doc_callbacks_delegate Delegates

Of course, callbacks can be used with \ref doc_datatypes_funcptr "delegates" as well. Delegates
are special function objects that holds a reference to an object and the method it should call
on it. If this is exactly how the application should treat them, then the above example will
work exactly the same and the application never needs to worry about whether the callback
is actually a global function or a delegate object.

Sometimes however, it may be beneficial to break up the delegate, and have the application store
the actual object and method separately, for example if the application should use a 
\ref doc_adv_weakref "weak reference" to avoid keeping the object alive longer than desired. 
The following shows how to retrieve the internals of the delegate:

\code
  // The callback, and accompanying object, if the callback is a class method
  asIScriptFunction *callback           = 0;
  void              *callbackObject     = 0;
  asITypeInfo       *callbackObjectType = 0;
  
  void SetCallback(asIScriptFunction *cb)
  {
    // Release the previous callback, if any
    if( callback )
      callback->Release();
    if( callbackObject )
      engine->ReleaseScriptObject(callbackObject, callbackObjectType);
    callback           = 0;
    callbackObject     = 0;
    callbackObjectType = 0;
    
    if( cb && cb->GetFuncType() == asFUNC_DELEGATE )
    {
      callbackObject     = cb->GetDelegateObject();
      callbackObjectType = cb->GetDelegateObjectType();
      callback           = cb->GetDelegateFunction();
 
      // Hold on to the object and method
      engine->AddRefScriptObject(callbackObject, callbackObjectType);
      callback->AddRef();
  
      // Release the delegate, since it won't be used anymore
      cb->Release();
    }
    else
    {
      // Store the received handle for later use
      callback = cb;

      // Do not release the received script function 
      // until it won't be used any more
    }
  }
\endcode

*/
