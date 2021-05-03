/**

\page doc_module Script modules

Each \ref asIScriptModule "module" is an independent unit with it's own scope of script functions, global 
variables, and classes. All scripts executed from a module will work on the same global variables contained
within that module. In this sense a module can be thought of as a shared library that is loaded into an 
application.

\section doc_module_single_vs_multi Single module versus multiple modules

Whether it is better to use a single module with all the script code, or multiple modules with smaller and more
specialized scripts, depends on the needs of the application. 

A single module can be easier to implement, as all script code is in the same scope and can interact with full 
freedom. On the other hand the entire script must be compiled together, making it more difficult to exchange
parts of the script or load only the logic that is needed at the moment.

With multiple modules an application can also \ref doc_adv_access_mask "expose different interfaces" to scripts 
with different tasks, e.g. a script that controls the graphical user interface in an application should perhaps
not have access to the same interface used by the script that controls the artificial intelligence, and vice versa.

Multiple modules can also use the same script code, in this case they will be completely distinct, each with 
their own set of global variables, functions, and types, even though the same source code was used. There may be 
valid reasons for using this in an application, but usually it is better to use a single module with \ref doc_script_class "script classes". 
The bytecode, functions, and types will then be shared, and the variables that should be distinct for each instance 
should be declared as class members.

\section doc_module_exchange Exchanging information between modules

While modules are independent, it may sometimes be necessary to exchange information between them. This can
be accomplished in many different ways. One way is through \ref asIScriptModule::BindImportedFunction "function binding",
where one module imports functions from another module and call those directly. Another way is with \ref doc_script_shared 
"shared script entities", where handles to objects or functions can be exchanged between the modules. A third alternative
is through messaging or proxy functions exposed by the application. 

All of these alternatives require a little code from the application, as one module doesn't automatically 
see another without the application's knowledge.








*/
