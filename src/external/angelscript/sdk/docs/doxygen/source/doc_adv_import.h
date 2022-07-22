/**

\page doc_adv_import Import functions

\ref doc_global_import "Importing functions" is a form of \ref doc_script_shared "sharing" code between script modules. 
Unlike the <tt>shared</tt> keyword though, the <tt>import</tt> requires specific code in the application to bind the imported 
functions after the script has been compiled. 

This may be useful when the application wants to control specifically what can and cannot be imported. 

To bind all the imported functions, without any specific treatment, the application just needs to call the 
\ref asIScriptModule::BindAllImportedFunctions "BindAllImportedFunctions" method after the 
\ref doc_compile_script "build" has completed.

If fine grained control is desired, then the application should use the methods 
\ref asIScriptModule::GetImportedFunctionCount "GetImportedFunctionCount", 
\ref asIScriptModule::GetImportedFunctionDeclaration "GetImportedFunctionDeclaration", 
\ref asIScriptModule::GetImportedFunctionSourceModule "GetImportedFunctionSourceModule", 
\ref asIScriptModule::GetFunctionByDecl "GetFunctionByDecl", and 
\ref asIScriptModule::BindImportedFunction "BindImportedFunction", to enumerate and bind the imported functions one by one, 
and raise an error in case the script tries to import a function that the application doesn't allow.

Another advantage that the import function feature has over shared entities, is that the imported functions can be unbound and
then bound to another script module if there is a need to change the sourcing module. To unbind the bound functions use the methods
\ref asIScriptModule::UnbindAllImportedFunctions "UnbindAllImportedFunctions" or \ref asIScriptModule::UnbindImportedFunction "UnbindImportedFunction".


*/
