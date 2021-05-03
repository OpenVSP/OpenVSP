/**

\page doc_adv_concurrent Concurrent scripts

The script engine can run multiple scripts in parallel, i.e. concurrent scripts. This can be done easily 
using \ref doc_adv_multithread "multithreading" where each thread runs its own script context, but this article
is going to explain how it is done without multithreading.

In order to execute multiple scripts in parallel, each script must have it's own \ref asIScriptContext, where 
the context is set up as for a \ref doc_call_script_func "normal function call". Then the application needs to 
set a timeout for each context. When the timeout is reached, the context should be \ref asIScriptContext::Suspend 
"suspended", so that the next context can execute for a while. 

Here's a very simple example of how this can be done:

\code
// The contexts have already been prepared before this function is called
void ExecuteScripts(std::vector<asIScriptContext *> contexts)
{
  // This function will run until all scripts have completed
  while( contexts.size() > 0 )
  {
    for( asUINT n = 0; n < contexts.size(); n++ )
    {
      // Set a 10 millisecond timeout for this context
      SetTimeoutForContext(contexts[n], 10);

      // Resume the execution of this context by calling Execute
      int r = contexts[n]->Execute();

      // Remove the timeout so it won't be triggered accidentally
      RemoveTimeoutForContext();

      // Determine if the script completed, or was timed out
      if( r == asEXECUTION_SUSPENDED )
      {
        // The script will continue in the next iteration
      }
      else
      {
        // The script has completed the execution, so we take it out of the list of scripts
        contexts[n--] = contexts.back();
        contexts.pop_back();
      }
    }
  }
}
\endcode

The application can manage the execution of the scripts in a simple round-robin fashion, where each script gets 
equal amount of execution time, or a more intricate management algorithm can be built, e.g. to give more execution
time to high priority scripts etc. 

The time out function, i.e. SetTimeoutForContext in the example above, can be implemented in two different ways. 
Through the use of \ref asIScriptContext::SetLineCallback "line callbacks", where the context will invoke the callback for
each statement in the script. The callback can then check if the timeout limit has been reached and suspend the context.

The other way is through the use of a timeout thread, where the thread simply sleeps until the timeout limit has been reached,
and when the thread wakes up it suspends the context (if it is still running). 

The timeout thread is probably the easiest to implement, and also doesn't impact the performance as much as the line 
callback. The line callback may still have to be used if the target OS doesn't support multithreading though.

Here's a simple example of how to implement the timeout function with a separate thread:

\code
static HANDLE            thread_handle = 0;
static asIScriptContext *thread_ctx;

DWORD WINAPI TimeoutThread(void *sleeptime)
{
  Sleep(*reinterpret_cast<int*>(sleeptime));
  if( thread_ctx )
    thread_ctx->Suspend();

  return 0;
}

void SetTimeoutForContext(asIScriptContext *ctx, int milliseconds)
{
  thread_ctx    = ctx;
  thread_handle = CreateThread(0, 50, TimeoutThread, reinterpret_cast<void*>(&milliseconds), 0, 0);
}

void RemoveTimeoutForContext()
{
  // TerminateThread should be used with extreme care, but in this 
  // case the TimeoutThread can't do any harm even if interrupted 
  // in the middle of the execution
  TerminateThread(thread_handle, 0);
  thread_handle = 0;
}
\endcode

Observe that the routines for multithreading usually differ a lot depending on the target system. The above code 
is for Windows and will most likely require adaption to work on any other system.

\see \ref doc_addon_ctxmgr, \ref doc_samples_concurrent, \ref doc_adv_coroutine






*/
