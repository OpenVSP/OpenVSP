/**

\page doc_adv_dynamic_build Dynamic compilations

There are various forms of dynamic compilations of scripts. In the following subtopics
I try to explain the theory of them and give some information on how they can be implemented. 


 
 

\section doc_adv_dynamic_build_ondemand On demand builds

The most common form of dynamic compilations is building new modules on demand, i.e.
as they are needed. When using this the application or game engine is typically designed
to have separate scripts to perform different tasks, e.g. menu handling, different types of AI 
controllers, player controller, etc. If these separate scripts have an abstraction layer through
the application to interact with each other it is very easy to build the scripts on demand.

To build a new script on demand, simply get a new module, add the needed script sections 
to it and build it just as you would when \ref doc_compile_script "building the first script". 
As each module is independent the previously existing module will be unaffected.

Each module can be configured to see a \ref doc_adv_access_mask "different application interface", 
so there is no need to create different engine instances.

Though modules are independent, they can still interact with each other if the application
provides the interface for that. To make the communication between modules easier one can
chose to use \ref doc_script_shared "shared script entities". Shared script entities will 
be compiled only once by the first module that includes the code. Any subsequent module that 
also include the code for the shared entity will reuse what was built by the first module, i.e.
will share the internal type and bytecode. 

Another option is to \ref doc_global_import "import global functions" from a previously 
compiled module. In this case the application must make sure the function that exports the 
functions is compiled first, then after building the module that imports the function the 
application must call the \ref asIScriptModule::BindAllImportedFunctions so conclude the imports.

\see \ref doc_samples_game "Game sample"





\section doc_adv_dynamic_build_incr Incremental builds

Besides the ordinary \ref doc_compile_script "compilation of scripts" and subsequent 
\ref doc_call_script_func "executions", AngelScript also support dynamically compiling
additional \ref asIScriptModule::CompileFunction "functions" and 
\ref asIScriptModule::CompileGlobalVar "global variables" to incrementally add to the 
scope of a module. These functions and variables will become part of the scope, just
as if they had been included in the initial script compilation which means that subsequent
executions or incremental compilations can use them.

It is also possible to dynamically remove \ref asIScriptModule::RemoveFunction "functions" 
and \ref asIScriptModule::RemoveGlobalVar "variables". Doing so doesn't immediately discard
the functions or variables, so other functions that still refer to them will not fail when
executing them. They will however no longer be visible for new compilations or when searching
for functions or variables in the module.

This kind of dynamic compilations is most useful when dealing with user interaction, 
e.g. an ingame console, or perhaps event handlers, e.g. a trigger on a GUI button click. 

\see \ref doc_addon_helpers "ExecuteString() add-on", \ref doc_samples_console "Console sample"




\section doc_adv_dynamic_build_hot Hot reloading scripts

Hot reloading is most often used when quick change & test cycles are wanted. It is quite common
for game engines to have a debug mode that continuously monitors the script files to detect any 
updates that are made, and when detected automatically reloads the scripts without restarting
the game. This allows the developers, for example, to quickly change the behaviour of the AI and 
immediately see the result without having to play through the entire level over and over again.

Hot reloading without preserving the state of existing objects is quite trivial. Simply discard 
everything, reload the scripts, and create new objects. On the other hand, hot reloading scripts
while preserving the state of existing objects can be quite tricky, but is also the most rewarding 
when implemented successfully.

The first part to reloading while preserving the state is keeping track of the object instances so 
the application knows which objects needs to be serialized when a module is hot reloaded. It is 
entirely up to the application to build and maintain this mapping. 

The second part is knowing which script files that will trigger a reload when changed. When building 
the module the application needs to store the list of the files that were added to it. If you use the
standard \ref doc_addon_build "script builder add-on", then it provides methods to enumerate the included 
sections or files.

The third part is the hardest, and that is to implement the actual serialization. Serializing object members
of value types is normally quite trivial, simply \ref doc_adv_reflection_vars "enumerate the object properties"
and store a name-value pair for each. Serializing object members of reference types needs some consideration,
as depending on the object that is referred to, either you will be able to store the actual pointer, or you
will need to store some kind of descriptor as the referred to object will also be recreated during the reload.

Once all the objects that will need to be recreated after the reload have been serialized for backup, 
the module can be rebuilt using the normal procedure for \ref doc_compile_script "compiling a script". 
Afterwards the serialized objects needs to be recreated. Once created you'll enumerate 
the members and set their values or references according to the backed up data.

\subsection doc_adv_dynamic_build_hot_1 Things to consider

 - Design the engine and script interface with hot reloading in mind from the start. Hot reloading is much 
   easier to implement the less complex interface you have, e.g. script objects that can only interact with
   other script objects indirectly through the application will not be able to hold references to objects 
   unknown by the application. 
 - \ref doc_script_shared "Shared script entities" will not be recompiled if some module or existing object 
   (including those pending destruction in the garbage collector) still refers to the code. It's recommended
   that you minimize the use of shared entities to where necessary so as to minimize the need to modify them.
 - The \ref asIScriptContext cannot be serialized so avoid reloading scripts that may still be referred to by
   a context, e.g. in a suspended state. It is quite likely that the application will crash if the module is
   reloaded while a context has one of the functions on the callstack.
 - Before discarding the original objects, consider doing a test compilation of the modified scripts in a 
   temporary module first, and only go ahead with the hot reloading if the test compilation is successful. That
   way you can catch compilation errors that would otherwise cause the recreation of the new objects to fail.
 - When recreating object instances you can take advantage of the method 
   \ref asIScriptEngine::CreateUninitializedScriptObject to create the objects without actually 
   executing the script class constructor, which may have unwanted side effects during the reload.
 - Decide what to do when the script modifications introduces new object properties or removes existing 
   properties, as you will not be able to restore the exact same object state then. Perhaps the new property
   can be filled with some default value to allow the continuation of the execution? Or perhaps the script itself
   can have a special event handler to provide the initial value of the new member?
 - The \ref doc_addon_serializer "Serializer" add-on has been implemented to aid the implementation of hot reloading
   scripts, though it may not suit all types of applications.
   
\see \ref doc_addon_serializer, \ref doc_adv_reflection

*/
