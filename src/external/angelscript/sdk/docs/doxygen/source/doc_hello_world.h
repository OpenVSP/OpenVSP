/**

\page doc_hello_world Your first script

This tutorial will show you the basics on how to configure the engine, compile a script, and then
execute it. The code in this article is not complete, it only contains the relevant parts to explain 
the basic structure for using the script library. For complete source codes see the \ref doc_samples "samples"
that come with the SDK.

In this tutorial a couple of add-ons are used to make the code easier. You are not required to use
these in your own application, but they will most likely let you get your project up and running 
faster. You'll want to take a look at the rest of the \ref doc_addon "add-ons" later on to see what 
else may be useful for you.

\code
// Include the definitions of the script library and the add-ons we'll use.
// The project settings may need to be configured to let the compiler where
// to find these headers. Don't forget to add the source modules for the
// add-ons to your project as well so that they will be compiled into the 
// application.
#include <angelscript.h>
#include <scriptstdstring/scriptstdstring.h>
#include <scriptbuilder/scriptbuilder.h>
\endcode

Being an embedded scripting library there isn't much that AngelScript allows the scripts 
to do by themselves, so the first thing the application must do is to \ref doc_register_api "register the interface" 
that the script will have to interact with the application. The interface may consist of 
functions, variables, and even complete classes.

Pay special attention to how the \ref doc_compile_script_msg "message callback" is registered right after the engine 
is created. The message callback is used by the engine to give human readable error messages
when something isn't working as it should, e.g. a registration is done incorrectly, or a script
has an error that fails to compile. While you still need to verify the return codes, the message
callback can give you valuable information that will let you figure out what is wrong without
much effort.

\code
// Create the script engine
asIScriptEngine *engine = asCreateScriptEngine();

// Set the message callback to receive information on errors in human readable form.
int r = engine->SetMessageCallback(asFUNCTION(MessageCallback), 0, asCALL_CDECL); assert( r >= 0 );

// AngelScript doesn't have a built-in string type, as there is no definite standard 
// string type for C++ applications. Every developer is free to register its own string type.
// The SDK do however provide a standard add-on for registering a string type, so it's not
// necessary to implement the registration yourself if you don't want to.
RegisterStdString(engine);

// Register the function that we want the scripts to call 
r = engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_CDECL); assert( r >= 0 );
\endcode

After the engine has been configured, the next step is to compile the scripts that should be executed.

The following is our script that will call the registered <tt>print</tt> function to write <tt>Hello world</tt> on the 
standard output stream. Let's say it's stored in the file <tt>test.as</tt>.

<pre>
  void main()
  {
    print("Hello world\n");
  }
</pre>

Here's the code for loading the script file and compiling it. The AngelScript engine itself doesn't have 
access to the filesystem so the loading the files has to be done by the application. Here we're going to use 
the \ref doc_addon_build "script builder" add-on, which does the loading of the script files, and some preprocessing, such as 
handling \#include directives. 

\code
// The CScriptBuilder helper is an add-on that loads the file,
// performs a pre-processing pass if necessary, and then tells
// the engine to build a script module.
CScriptBuilder builder;
int r = builder.StartNewModule(engine, "MyModule"); 
if( r < 0 ) 
{
  // If the code fails here it is usually because there
  // is no more memory to allocate the module
  printf("Unrecoverable error while starting a new module.\n");
  return;
}
r = builder.AddSectionFromFile("test.as");
if( r < 0 )
{
  // The builder wasn't able to load the file. Maybe the file
  // has been removed, or the wrong name was given, or some
  // preprocessing commands are incorrectly written.
  printf("Please correct the errors in the script and try again.\n");
  return;
}
r = builder.BuildModule();
if( r < 0 )
{
  // An error occurred. Instruct the script writer to fix the 
  // compilation errors that were listed in the output stream.
  printf("Please correct the errors in the script and try again.\n");
  return;
}
\endcode

The last step is to identify the function that is to be called, and set up a context
for executing it. 

\code
// Find the function that is to be called. 
asIScriptModule *mod = engine->GetModule("MyModule");
asIScriptFunction *func = mod->GetFunctionByDecl("void main()");
if( func == 0 )
{
  // The function couldn't be found. Instruct the script writer
  // to include the expected function in the script.
  printf("The script must have the function 'void main()'. Please add it and try again.\n");
  return;
}

// Create our context, prepare it, and then execute
asIScriptContext *ctx = engine->CreateContext();
ctx->Prepare(func);
int r = ctx->Execute();
if( r != asEXECUTION_FINISHED )
{
  // The execution didn't complete as expected. Determine what happened.
  if( r == asEXECUTION_EXCEPTION )
  {
    // An exception occurred, let the script writer know what happened so it can be corrected.
    printf("An exception '%s' occurred. Please correct the code and try again.\n", ctx->GetExceptionString());
  }
}
\endcode

The exception handling above is very basic. The application may also obtain information about line number,
function, call stack, and even values of local and global variables if wanted.

Don't forget to clean up after you're done with the engine.

\code
// Clean up
ctx->Release();
engine->ShutDownAndRelease();
\endcode

\section doc_hello_world_1 Helper functions

The print function is implemented as a very simple wrapper on the printf function.

\code
// Print the script string to the standard output stream
void print(string &msg)
{
  printf("%s", msg.c_str());
}
\endcode

\see \ref doc_compile_script_msg, \ref doc_addon_build, \ref doc_addon_std_string, \ref doc_samples


*/
