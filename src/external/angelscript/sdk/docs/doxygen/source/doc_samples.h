/**

\page doc_samples Samples


This page gives a brief description of the samples that you'll find in the /sdk/samples/ folder.

 - \subpage doc_samples_tutorial
 - \subpage doc_samples_concurrent
 - \subpage doc_samples_console
 - \subpage doc_samples_corout
 - \subpage doc_samples_events
 - \subpage doc_samples_incl
 - \subpage doc_samples_asbuild
 - \subpage doc_samples_game
 - \subpage doc_samples_asrun



\page doc_samples_tutorial Tutorial

<b>Path:</b> /sdk/samples/tutorial/

This sample was written with the intention of explaining the basics of 
AngelScript, that is, how to configure the engine, load and compile a script, 
and finally execute a script function with parameters and return value.


 - \ref asIScriptEngine::SetMessageCallback
 - LineCallback() function which aborts execution when the time is up
 - Strings
 - Registered global functions
 - Script function parameters and return value
 - Retrieving information about script exceptions
 - asIScriptGeneric for when the library doesn't support native calling convention






\page doc_samples_concurrent Concurrent scripts

<b>Path:</b> /sdk/samples/concurrent/

This sample shows how to execute two or more long running scripts
\ref doc_adv_concurrent "concurrently". The scripts voluntarily hand over the control to the next script in 
the queue by calling the function sleep().


 - \ref doc_addon_ctxmgr
 - Multiple scripts running in parallel
 - sleep()
 - Strings
 - Registered global functions




\page doc_samples_console Console

<b>Path:</b> /sdk/samples/console/

This sample implements a simple interactive console, which lets the user type in 
commands and also evaluate simple script statements to manipulate the application.

The user is also able to define new variables and functions from the command line. 
These functions can then be executed to perform automated tasks. 

 - \ref doc_addon_helpers "ExecuteString"
 - \ref asIScriptModule::CompileFunction "CompileFunction", \ref asIScriptModule::CompileGlobalVar "CompileGlobalVar", \ref asIScriptModule::RemoveFunction "RemoveFunction", \ref asIScriptModule::RemoveGlobalVar "RemoveGlobalVar"
 - Enumerate global functions and variables
 - \ref doc_addon_std_string "Strings"
 - Registered global functions and properties
 - Special function _grab() with overloads to receive and print resulting value from script statements



\page doc_samples_corout Co-routines

<b>Path:</b> /sdk/samples/coroutine/

This sample shows how \ref doc_adv_coroutine "co-routines" can be implemented with AngelScript. Co-routines are
threads that can be created from the scripts, and that work together by voluntarily passing control
to each other by calling yield().

 - \ref doc_addon_ctxmgr
 - Co-routines created from the scripts with variable parameter structure.
 - \ref doc_addon_std_string
 - Registered global functions
 - Handling the variable argument type
 - Passing arguments to script functions
 - \ref doc_addon_dict


\page doc_samples_events Events

<b>Path:</b> /sdk/samples/events/

This sample has the script engine execute a long running script. The script execution is regularly 
interrupted by the application so that keyboard events can be processed, which execute another short 
script before resuming the execution of the main script. The event handling scripts change the state 
of the long running script.

 - LineCallback() function which suspends execution when the time is up
 - \ref doc_addon_std_string
 - Registered global functions
 - Scripted event handlers



\page doc_samples_incl Include directive

<b>Path:</b> /sdk/samples/include/

This sample shows how to implement a very simple preprocessor to add support for the \#include 
directive, which allow the script writer to reuse common script code. The preprocessor simply adds 
the included scripts as multiple script sections, which is ok as AngelScript is able to resolve global 
declarations independently of their order. The preprocessor also makes sure that a script file is only 
included once, so the script writer doesn't have to take extra care to avoid multiple includes or even 
complicated circular includes.

 - \ref doc_addon_build
 - LineCallback() functions which aborts execution when the time is up
 - Processing the \#include directive
 - Circular \#includes are resolved automatically




\page doc_samples_asbuild Generic compiler

<b>Path:</b> /sdk/samples/asbuild/

This sample shows how a generic bytecode compiler can be built to provide offline compilation of scripts.
It takes as input a configuration file that defines the application interface, registers it using dummy
functions and properties, then it compiles the script and saves the bytecode to a file on disk.

The configuration file should preferably be created automatically by calling the \ref doc_addon_helpers "WriteConfigToFile" 
helper function. This function should be called from the application that will execute the bytecode, after 
the application interface has been fully registered.

 - \ref doc_adv_precompile 
 - \ref doc_addon_build
 - \ref asIScriptEngine::SetMessageCallback
 - \ref asEP_INIT_GLOBAL_VARS_AFTER_BUILD
 - \ref asIScriptEngine::ParseToken
 - \ref asIScriptModule::SaveByteCode
 - \ref asIBinaryStream





\page doc_samples_asrun Command line runner

<b>Path:</b> /sdk/samples/asrun/

This samples gives a very basic command line runner for AngelScripts.

It also implements a fully functional command line debugger, with support for setting 
breakpoints, stepping through the code, examining variables, etc.

 - \ref doc_debug
 - \ref doc_addon_debugger
 - \ref doc_addon_std_string
 - \ref doc_addon_array
 - \ref doc_addon_dict
 - \ref doc_addon_file
 - \ref doc_addon_filesystem
 - \ref doc_addon_datetime
 - \ref doc_addon_ctxmgr
 - \ref doc_addon_build
 - \ref doc_addon_helpers_try
 - \ref asIScriptEngine::SetContextCallbacks
 - \ref doc_script_stdlib_system
 - Implementing a \#pragma callback

\see \subpage doc_samples_asrun_manual


\page doc_samples_asrun_manual asrun manual

asrun is a simple command-line tool that supports executing AngelScript files directly in order 
to perform simple batch tasks. The API provided by the tool allow the script to interact with 
the system to read and write files, and execute other system commands.

On Windows it can be associated, for example with the file extension .as, to allow scripts to be 
executed directly from the file browser with a double click. This makes it a handy tool even without
a terminal for command-line input. On Linux and UNIX based systems the shebang interpreter directive 
can be added as the first line on the script to make the script files executable.




\section doc_samples_asrun_usage Usage

<pre>
asrun [-d] \<script file> [\<args>]
 -d             inform if the script should be runned with debug
 \<script file>  is the script file that should be runned
 \<args>         zero or more args for the script
</pre>

These usage instructions are also presented if the tool is executed without any arguments.



\section doc_samples_asrun_script Scripts

As entry point for executing the scripts, asrun looks for one of the functions <tt>int main()</tt> or 
<tt>void main()</tt>. If neither of these functions are found an error will be reported.

The scripts can use the full set of the script language and standard library described in 
\ref doc_script "the script language" section of this manual.

The script file informed as input on the command line may include additional files with the include
directive <tt>\#include "<file name>"</tt>, so that the script files can be better organized and common
logic shared between multiple scripts. 







\section doc_samples_asrun_debug How to debug scripts

To run a script with the debugger, either add the command line argument -d 
or include <tt>\#pragma debug</tt> in the script. When this is done, the 
debugger will show a prompt like this:

<pre>
Debugging, waiting for commands. Type 'h' for help.
[dbg]>
</pre>

This will then let you set up breakpoints, step through the code, inspect 
variables, etc. Type <tt>h</tt> on the prompt to get the list of commands available.









\page doc_samples_game Game

<b>Path:</b> /sdk/samples/game/

This sample shows one way of integrating the scripting library in a game engine. It is a 
simple game where the player is trying to avoid getting eaten by zombies. Each game object type
has it's own script that controls its behaviour, these are loaded independently into separately
modules. 

 - \ref asIScriptModule
 - \ref asITypeInfo::SetUserData
 - \ref doc_global_interface
 - \ref doc_script_class
 - \ref doc_addon_build
 - \ref doc_addon_handle
 - \ref doc_script_shared


*/
