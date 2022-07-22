/**

\page doc_register_api What can be registered

AngelScript requires the application developer to register the interface
that the scripts should use to interact with anything outside the script itself.

It's possible to register \ref doc_register_func "global functions" and
\ref doc_register_prop "global properties" that can be used directly by the 
scripts.

For more complex scripts it may be useful to register new \ref doc_register_type "object types" 
to complement the built-in data types. 

AngelScript doesn't have a \ref doc_strings "built-in string type" as there is no de-facto standard for string types 
in C++. Instead AngelScript permits the application to register its own preferred string type, and 
a \ref asIScriptEngine::RegisterStringFactory "string factory" that the script engine will use
to instanciate the strings. 

There is also no default \ref doc_arrays "built-in array type" as this too is something that most developers may want to have 
their own version of. The array type is registered as a \ref doc_adv_template "template", which is then set as the
\ref asIScriptEngine::RegisterDefaultArrayType "default array type". A standard \ref doc_addon_array "array add-on" is 
provided for those that do not want to implement their own array type.

\ref asIScriptEngine::RegisterInterface "Class interfaces" can be registered if you want 
to guarantee that script classes implement a specific set of class methods. Interfaces can 
be easier to use when working with script classes from the application, but they are not 
necessary as the application can easily enumerate available methods and properties even
without the interfaces.

\ref asIScriptEngine::RegisterFuncdef "Function definitions" can be registered when you wish to allow the script
to pass function pointers to the application, e.g. to implement callback routines.

\ref asIScriptEngine::RegisterEnum "Enumeration types" and 
\ref asIScriptEngine::RegisterTypedef "typedefs" can also be registered to improve readability of the scripts.




*/
