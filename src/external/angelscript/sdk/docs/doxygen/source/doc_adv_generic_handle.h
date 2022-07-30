/**

\page doc_adv_generic_handle Registering a generic handle type

Sometimes it is useful to be able to take a reference to an object without having to 
force specific implementation on the object, e.g. when providing a generic storage class
to scripts, or providing a generic messaging system. 

In AngelScript there is no common denominator for all object types, the reasons for this is 
that script classes and application registered classes have completely different implementation and
cannot be generalized under a single superclass. 

Instead, AngelScript provides an alternative by allowing the application to register a 
generic handle type. This type would be implemented according to the application's needs,
but to the script it would look like it was a generic \ref doc_script_handle "handle" that can 
reference any reference type. With this the type will be able to hold objects, 
\ref doc_datatypes_funcptr "functions", and \ref doc_addon_array "arrays". 

To register the generic handle type, the application should follow the same principles as for a 
\ref doc_register_val_type "value type", except for a few details, that will be described here.

 - The type must be registered with the additional flag \ref asOBJ_ASHANDLE. This is the flag that 
   tells AngelScript that the type simulates a generic handle. 

 - In order to allow assigning any handle to the type, the \ref doc_script_class_ops "opHndlAssign" method must be registered with
   a \ref doc_adv_var_type "variable parameter type", e.g. 'ref &opHndlAssign(const ?&in)'.
   
 - The \ref doc_script_class_ops "opEquals" method must also be registered with a \ref doc_adv_var_type in order to allow the 
   is and !is operators to behave as expected for a handle, e.g. 'bool opEquals(const ?&in)'.
   
 - Finally the \ref doc_script_class_ops "opCast" must be registered with the signature 'void opCast(?&out)' in order
   to allow a dynamic cast to any other type.

As this is a quite useful type, with really very little need for customization, a standard add-on 
with this implementation has been provided with the SDK.

\see \ref doc_addon_handle



*/
