/**

\page doc_adv_coroutine Co-routines

Co-routines is a way to allow multiple execution paths in parallel, but without the hazards of pre-emptive scheduling in 
\ref doc_adv_concurrent "multithreading" where one thread can be suspended at any moment so another can resume. Because co-routines always voluntarily
suspend themselves in favor of the next co-routine, there is no need to worry about atomic instructions and critical sections. 

Co-routines are not natively built-into the AngelScript library, but it can easily be implemented from the application side. 
In fact, the \ref doc_addon_ctxmgr add-on already provides a default implementation for this.

To implement your own version of co-routines you will need a couple of pieces:

 - The co-routine itself, which is just an instance of the \ref asIScriptContext object. Each co-routine will have its own 
   context object that holds the callstack of the co-routine.
 - A function that will permit the script to create, or spawn, new co-routines. This function will need to be able to refer 
   to starting function of the new co-routine. This reference can be done by name, or perhaps more elegantly with a 
   \ref doc_datatypes_funcptr "function pointer". Once invoked this function will instanciate the new context, and prepare 
   it with the starting function.
 - A function that will permit a co-routine to yield control to the next co-routine. This function will simply suspend the 
   current context so the next co-routine can be resumed.
 - A simple control algorithm for the co-routines. This can just be a loop that will iterate over an array of co-routines, 
   i.e. contexts, until all of them have finished executing. When a new co-routine is created it is simply appended to the 
   array to be picked up when the current co-routine yields the control.

A simple implementation of the function that spawns a new co-routine may look like this:

\code
void CreateCoRoutine(string &func)
{
  asIScriptContext *ctx = asGetActiveContext();
  if( ctx )
  {
    asIScriptEngine *engine = ctx->GetEngine();
    string mod = ctx->GetFunction()->GetModuleName();

    // We need to find the function that will be created as the co-routine
    string decl = "void " + func + "()"; 
    asIScriptFunction *funcPtr = engine->GetModule(mod.c_str())->GetFunctionByDecl(decl.c_str());
    if( funcPtr == 0 )
    {
      // No function could be found, raise an exception
      ctx->SetException(("Function '" + decl + "' doesn't exist").c_str());
      return;
    }

    // Create a new context for the co-routine
    asIScriptContext *coctx = engine->CreateContext();
    coctx->Prepare(funcPtr);

    // Add the new co-routine context to the array of co-routines
    coroutines.push_back(coctx);
  }
}
\endcode

The yield function is even simpler:

\code
void Yield()
{
  asIScriptContext *ctx = asGetActiveContext();
  if( ctx )
  { 
    // Suspend the context so the next co-routine can be resumed
    ctx->Suspend();
  }
}
\endcode

A basic control algorithm might look like this:

\code
std::vector<asIScriptContext *> coroutines;
void Execute()
{
  int n = 0;
  while( coroutines.size() > 0 )
  {
    // Resume the co-routine
    int r = coroutines[n]->Execute();
    if( r == asEXECUTION_SUSPENDED )
    {
      // Resume the next co-routine
      if( ++n == coroutines.size() )
        n = 0;
    }
    else
    {
      // The co-routine finished so let's remove it
      coroutines[n]->Release();
      coroutines.erase(n);
    }
  }
}
\endcode


\see \ref doc_addon_ctxmgr, \ref doc_samples_corout, \ref doc_adv_concurrent






*/
