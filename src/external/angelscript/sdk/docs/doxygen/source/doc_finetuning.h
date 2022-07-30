/**

\page doc_finetuning Fine tuning

Here's a few recommendations to get the most performance out of AngelScript.

\section doc_finetuning_1 Cache the functions and types

Doing searches by function declaration or name is rather time consuming and 
should not be done more than once per function that will be called. The same 
goes for the types that you might use it.

Also try to use the actual \ref asIScriptFunction or \ref asITypeInfo pointers
instead of the ids where possible. This will save the engine from translating the  
id to the actual object.

You may use the user data in the various engine interfaces to store the cached 
information. For example, store a structure with the commonly used class methods
as \ref asITypeInfo::SetUserData "user data" in the \ref asITypeInfo interface. 
This way you will have quick access to the functions when they need to be called.



\section doc_finetuning_2 Reuse the context object

The context object is a heavy weight object and you should avoid allocating new
instances for each call. The object has been designed to be reused for multiple 
calls.

\subsection doc_finetuning_2_1 Context pool

Ideally the application will keep a simple memory pool of allocated context 
objects, where new objects are only allocated if the is no free objects in the 
pool.

By using the engine's \ref asIScriptEngine::SetContextCallbacks "context callbacks", 
this context pool will also be automatically available to the engine for use in 
internal calls and the add-ons. 

The code that wants to use a context from the pool should use the 
\ref asIScriptEngine::RequestContext "RequestContext" and \ref asIScriptEngine::ReturnContext "ReturnContext" methods
instead of the \ref asIScriptEngine::CreateContext "CreateContext" method.

Here's a simple implementation of a context pool.

\code
std::vector<asIScriptContext *> pool;
asIScriptContext *RequestContextCallback(asIScriptEngine *engine, void *param)
{
  // Get a context from the pool, or create a new
  asIScriptContext *ctx = 0;
  if( pool.size() )
  {
    ctx = *pool.rbegin();
    pool.pop_back();
  }
  else
    ctx = engine->CreateContext();

  return ctx;
}

void ReturnContextToPool(asIScriptEngine *engine, asIScriptContext *ctx, void *param)
{
  pool.push_back(ctx);
  
  // Unprepare the context to free non-reusable resources
  ctx->Unprepare();
}
\endcode

\subsection doc_finetuning_2_2 Nested calls

Another form of reusing a context is to use nested calls. 

Whenever an application registered function that is called from a script needs to 
execute another script it can reuse the already active context for a nested call.
This way it is not necessary to look for an available context from a pool or allocate 
a new context just for this call.

\code
void Func()
{
  // Get the active context
  asIScriptContext *ctx = asGetActiveContext();

  // Store the current context state so we can restore it later
  if( ctx && ctx->PushState() > 0 )
  {
    // Use the context normally, e.g.
    //  ctx->Prepare(...);
    //  ctx->Execute(...);

    // Once done, restore the previous state
    ctx->PopState();
  }
}
\endcode




\section doc_finetuning_3 Compile scripts without line cues

The line cues are normally placed in the bytecode between each script 
statement. These are where the VM will allow the execution to be suspended, and
also where the line callback is invoked.

If you do not need to receive a callback for every statement executed in a script 
then you may get a little more performance out of the script by compiling without 
the line cues. 

The line callback will still work and is guaranteed to be invoked at least once 
per loop and every function call in the script to allow the application to interrupt 
infinite loops or infinitely recursive calls.

\code
engine->SetEngineProperty(asEP_BUILD_WITHOUT_LINE_CUES, true);
\endcode





\section doc_finetuning_4 Disable thread safety

If your application only uses one thread to invoke the script engine, then it 
may be worth it to compile the library without the thread safety to gain a 
little more performance.

To do this, define the AS_NO_THREADS flag in the as_config.h header or in the 
project settings when compiling the library.




\section doc_finetuning_5 Turn off automatic garbage collection

While garbage collection is important in long running applications, it may be of interest to 
turn off the automatic garbage collection and then run the garbage collector manually in a
controlled manner. The garbage collector is incremental so you shouldn't see long stalls 
while it is running, but it will consume CPU cycles that may be needed for other things.

To turn off the automatic garbage collector set the engine property \ref asEP_AUTO_GARBAGE_COLLECT to false.

\code
engine->SetEngineProperty(asEP_AUTO_GARBAGE_COLLECT, false);
\endcode

\see \ref doc_gc




\section doc_finetuning_6 Compare native calling convention versus generic calling convention

If you have specific functions that are called very frequently it may be worth comparing the performance
between binding the functions using native calling convention versus the generic calling convention. It
is not possible to generalize and say that one is always faster than the other, as it will vary depending
on the function signature and the platforms ABI complexity.

\see \ref doc_generic, \ref doc_register_func_2






*/