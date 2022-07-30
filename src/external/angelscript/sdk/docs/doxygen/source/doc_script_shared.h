/**


\page doc_script_shared Shared script entities

If the application uses multiple script modules to control different parts 
of the application it may sometimes be beneficial to allow parts of
the scripts to be shared between the script modules. The main benefits of 
shared entities is the reduced memory consumption, and the fact that the
type of the shared entity is the same for the modules thus simplifying the
information exchange between modules where this is used.

Shared entities have a restriction in that they cannot access non-shared
entities because the non-shared entities are exclusive to the script module 
in which they were compiled.




\section doc_script_shared_1 How to declare shared entities

To declare a shared entity simply put the keyword 'shared' before the ordinary
declaration, e.g.

<pre>
  shared class Foo
  {
    void MethodInFoo(int b) { bar = b; }
    int bar;
  }
  
  shared void GlobalFunc() {}
</pre>

If the script tries to access any non-shared entity from within the shared
entity then the compiler will give an error message. 

Obviously, in order to work the scripts in all modules that share the entity
must implement the entity the same way. If this is not done, the compiler will
give an error in the scripts that are compiled after the first script that 
implemented the shared entity.

The easiest way to guarantee that the implementation is the same is by using the
same source file, but this is not a requirement. 




\section doc_script_shared_external External shared entities

An alternative to declaring the full shared entity in each module is to declared 
the shared entity that is known to have been compiled before as external. This 
can shorten the script, both reducing the size of the source code and the time to 
compile it. 

To declare a shared entity as external, simply add the keyword 'external' before 
the declaration and end the statement with ';' after the signature that uniquely identifies
the entity. 

<pre>
  external shared class Foo;
  external shared void GlobalFunc();
</pre>

When compiling a shared entity declared as external and the shared entity hasn't been compiled
in any other module before it, the compiler will give an error message.




\section doc_script_shared_2 What can be shared

Currently only the \ref doc_script_class "class", \ref doc_global_interface "interface",
\ref doc_script_func "function", \ref doc_global_enums "enum", and 
\ref doc_datatypes_funcptr "funcdefs" entities can be shared. 

Future versions may allow \ref doc_global_variable "global variables" to be shared too.

*/
