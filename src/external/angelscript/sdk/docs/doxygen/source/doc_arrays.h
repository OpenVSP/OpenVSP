/**

\page doc_arrays Custom array type

Like the \ref doc_strings "string type", AngelScript doesn't have a built-in 
dynamic array type. Instead the application developers that wishes to allow 
dynamic arrays in the scripts should register this in the way that is best 
suited for the application. To save time a readily usable add-on is provided 
with a fully implemented \ref doc_addon_array "dynamic array type".  

If you wish to create your own array type, you'll need to understand how 
\ref doc_adv_template "template types" work. A template type is a special form 
of registering a type that the engine can then use to instanciate the true 
types based on the desired sub-types. This allow a single type registration to 
cover all possible array types that the script may need, instead of the 
application having to register a specific type for each type of array.

Of course, a generic type like this will have a drawback in performance due to 
runtime checks to determine the actual type, so the application should consider
registering \ref doc_adv_template_2 "template specializations" of the most 
common array types. This will allow the best optimizations for those types, and
will also permit better interaction with the application as the template 
specialization can better match the actual array types that the application 
uses.

*/
