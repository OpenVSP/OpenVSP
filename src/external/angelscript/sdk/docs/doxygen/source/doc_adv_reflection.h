/**

\page doc_adv_reflection Reflection

While the script language doesn't offer built-in functionality for reflection, the application 
interface do provide all the necessary methods to enumerate everything within the scripts. 

The article on \ref doc_debug "debugging" shows a little of these interfaces with the focus
on debugging rather than reflection. The article on \ref doc_adv_dynamic_build "dynamic compilations"
shows another aspect where scripts may be partially modified at runtime.

The following will do an overview of the available methods for enumerating the various entities in 
a script.

\section doc_adv_reflection_vars Enumerating variables and properties

Global variables in a script module are enumerated with the interface \ref asIScriptModule. 
Specifically the methods \ref asIScriptModule::GetGlobalVarCount "GetGlobalVarCount" and
\ref asIScriptModule::GetGlobalVar "GetGlobalVar". \ref asIScriptModule::GetGlobalVarIndexByName "GetGlobalVarIndexByName" and
\ref asIScriptModule::GetGlobalVarIndexByDecl "GetGlobalVarIndexByDecl" can be used if the name and/or
declaration of the desired variable is known before hand. To inspect or even modify the value
of the global variable the method \ref asIScriptModule::GetAddressOfGlobalVar "GetAddressOfGlobalVar"
should be used.

The engine interface \ref asIScriptEngine has a similar set of methods for enumerating application
registered global properties, i.e. \ref asIScriptEngine::GetGlobalPropertyCount "GetGlobalPropertyCount",
\ref asIScriptEngine::GetGlobalPropertyByIndex "GetGlobalPropertyByIndex", 
\ref asIScriptEngine::GetGlobalPropertyIndexByName "GetGlobalPropertyIndexByName", and 
\ref asIScriptEngine::GetGlobalPropertyIndexByDecl "GetGlobalPropertyIndexByDecl".

Member properties of classes are accessed through the \ref asIScriptObject interface for live 
object instances, and \ref asITypeInfo interface for inspecting the class declarations without any
live object instance.

Local variables within functions can also be enumerated as long as the script has been compiled with
debug information. These are then enumerated through the \ref asIScriptFunction interface for inspecting
the declarations, and directly through \ref asIScriptContext for inspecting and/or modifying them on the 
stack.

\section doc_adv_reflection_funcs Enumerating functions and methods

Global functions in a script are enumerated with the interface \ref asIScriptModule, using the methods
\ref asIScriptModule::GetFunctionCount "GetFunctionCount", \ref asIScriptModule::GetFunctionByIndex "GetFunctionByIndex",
\ref asIScriptModule::GetFunctionByName "GetFunctionByName", and \ref asIScriptModule::GetFunctionByDecl "GetFunctionByDecl".

The engine interface \ref asIScriptEngine also exposes methods for enumerating application registered functions in a similar manner.

To enumerate methods of classes the interface \ref asITypeInfo should be used.

\section doc_adv_reflection_types Enumerating types

\ref asIScriptModule is of course used to enumerate the types declared in the scripts too. The methods
\ref asIScriptModule::GetObjectTypeCount "GetObjectTypeCount", \ref asIScriptModule::GetObjectTypeByIndex "GetObjectTypeByIndex", and
\ref asIScriptModule::GetTypeInfoByName "GetTypeInfoByName" are for enumerating classes and interfaces. The methods
\ref asIScriptModule::GetEnumCount "GetEnumCount" and \ref asIScriptModule::GetEnumByIndex "GetEnumByIndex" are
for enumerating enums.

The \ref asIScriptEngine interface has near identical methods for enumerating the application registered types.

Many of the above methods return values called type ids that describes the type of the respective variable, property, or function argument.
In many cases the type id can be directly inspected as a bitfield to get necessary information on the what the type is. The lower bits
are just a sequence number where the first 12 numbers represents the built-in primitives, and anything higher represents either application
registered types or script declared types. The higher bits indicate if the type represents a primitive, object, or handle. Use the flags
in \ref asETypeIdFlags to do the necessary verifications on the type id.

For type ids that represent object types it may be necessary to obtain the \ref asITypeInfo instance to get further information
on what the type is. The method \ref asIScriptEngine::GetTypeInfoById "GetTypeInfoById" is used to do this translate from type id
to \ref asITypeInfo.


*/
