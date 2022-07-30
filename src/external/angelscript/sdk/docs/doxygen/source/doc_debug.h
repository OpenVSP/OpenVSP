/**

\page doc_debug Debugging scripts

AngelScript offers a rich interface to support the debugging of scripts. It is easy to build an embedded debugger 
that can set break points, inspect/manipulate variables in functions, visualize the call stack, etc. 

Observe that the CDebugMgr class used in the examples below doesn't exist. It is only used as an abstraction to
avoid having to write fictional debug routines.

\see \ref doc_addon_debugger for a standard implementation

\section doc_debug_1 Setting line breaks

In order to break at a specified line in the code the debugger can set the line callback function in the script context. The
VM will then invoke the callback for each statement executed, allowing the debugger to decide whether to proceed to the next
statement or not.

\code
// An example line callback
void DebugLineCallback(asIScriptContext *ctx, CDebugMgr *dbg)
{
  // Determine if we have reached a break point
  const char *scriptSection;
  int line = ctx->GetLineNumber(0, 0, &scriptSection);
  asIScriptFunction *function = ctx->GetFunction();

  // Now let the debugger check if a breakpoint has been set here
  if( dbg->IsBreakpoint(scriptSection, line, function) )
  {
    // A break point has been reached so the execution of the script should be suspended
    ctx->Suspend();
  }
}
\endcode

The line callback is set on the context with the following call:

\code
  // Set the line callback with the address of the debug manager as parameter
  ctx->SetLineCallback(asFUNCTION(DebugLineCallback), dbg, asCALL_CDECL);
\endcode

When the line callback suspends the execution the context's \ref asIScriptContext::Execute "Execute" function will return with
the code \ref asEXECUTION_SUSPENDED. The application can then go into a special message loop where the debug routines can be handled,
e.g. to \ref doc_debug_2 "view the call stack", \ref doc_debug_3 "examine variables", etc. Once the execution should continue, simply
call the Execute method again to resume it.

An alternative to suspending the script execution might be to start the message loop directly within the line callback, in which
case resuming the execution is done simply by returning from the line callback function. Which is the easiest to implement depends on
how you have implemented your application.











\section doc_debug_2 Viewing the call stack

The asIScriptContext exposes the call stack for viewing purposes, so that you can easily track the origin of calls. It is also possible 
to \ref doc_debug_3 "print the value of variables" at each level in the callstack.

Here's an example of how the entire call stack can be printed:

\code
void PrintCallstack(asIScriptContext *ctx)
{
  // Show the call stack
  for( asUINT n = 0; n < ctx->GetCallstackSize(); n++ )
  {
    asIScriptFunction *func;
    const char *scriptSection;
    int line, column;
    func = ctx->GetFunction(n);
    line = ctx->GetLineNumber(n, &column, &scriptSection);
    printf("%s:%s:%d,%d\n", scriptSection,
                            func->GetDeclaration(),
                            line, column);
  }
}
\endcode
















\section doc_debug_3 Inspecting variables

Through the context interface it is possible to inspect and even modify the value of the local variables on the 
stack. This can be done for each level in the call stack, and not just the current function that is being executed.

Here is an example for how the variables may be printed:

\code
void PrintVariables(asIScriptContext *ctx, asUINT stackLevel)
{
  asIScriptEngine *engine = ctx->GetEngine();

  // First print the this pointer if this is a class method
  int typeId = ctx->GetThisTypeId(stackLevel);
  void *varPointer = ctx->GetThisPointer(stackLevel);
  if( typeId )
  {
    printf(" this = 0x%x\n", varPointer);
  }

  // Print the value of each variable, including parameters
  int numVars = ctx->GetVarCount(stackLevel);
  for( int n = 0; n < numVars; n++ )
  {
    int typeId = ctx->GetVarTypeId(n, stackLevel); 
    void *varPointer = ctx->GetAddressOfVar(n, stackLevel);
    if( typeId == asTYPEID_INT32 )
    {
      printf(" %s = %d\n", ctx->GetVarDeclaration(n, stackLevel), *(int*)varPointer);
    }
    else if( typeId == asTYPEID_FLOAT )
    {
      printf(" %s = %f\n", ctx->GetVarDeclaration(n, stackLevel), *(float*)varPointer);
    }
    else if( typeId & asTYPEID_SCRIPTOBJECT )
    {
      asIScriptObject *obj = (asIScriptObject*)varPointer;
      if( obj )
        printf(" %s = {...}\n", ctx->GetVarDeclaration(n, stackLevel));
      else
        printf(" %s = <null>\n", ctx->GetVarDeclaration(n, stackLevel));
    }
    else if( typeId == engine->GetTypeIdByDecl("string") )
    {
      string *str = (string*)varPointer;
      if( str )
        printf(" %s = '%s'\n", ctx->GetVarDeclaration(n, stackLevel), str->c_str());
      else
        printf(" %s = <null>\n", ctx->GetVarDeclaration(n, stackLevel));
    }
    else
    {
      printf(" %s = {...}\n", ctx->GetVarDeclaration(n, stackLevel));
    }
  }
}
\endcode

The above code is only an example to give an idea of how it can be done. It is not complete and only
recognizes a few types. To make it useful it would have to be expanded to recognize all types, and
perhaps add some generic way of converting an object to human readable string for printing.

For script objects that conversion can be done by enumerating the members of an object through the 
\ref asIScriptObject interface.

The debugger may also need to be able to inspect the global variables that the functions access. As the
global variables are stored in the module, there is the place to look for them. The \ref asIScriptModule 
interface can be obtained by querying the module name from the function, and then getting the module 
pointer from the engine. Once the module is determined the global variables are enumerated much the same
way as in the example above, except that the appropriate methods on the asIScriptModule interface are used
instead.





\section doc_debug_4 Debugging internally executed scripts

Some script execution is not initiated by the application, e.g. the initialization of global variables or 
the call to the script class destructor when destroying objects from the garbage collector. If these
executions should be debugged, the application must set the context callback functions with a call to
\ref asIScriptEngine::SetContextCallbacks. The engine will invoke these callbacks to request a context
from the application when it will execute a script internally. The application can then  
attach the debugger to the context it provides to the engine.





*/
