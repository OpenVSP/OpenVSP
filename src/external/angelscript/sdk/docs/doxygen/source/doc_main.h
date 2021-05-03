/**

\mainpage Introduction

\image html aslogo.png 

<center>Version 2.35.1</center>

\ref doc_overview "AngelScript" is a \ref doc_license "free, open source", flexible, and cross-platform scripting library meant to be
embedded in applications. The purpose is to provide an easy to use library that is powerful, but that isn't weighed 
down by a large amount of rarely used features.

Development of AngelScript begun in February, 2003, with the first public release on March 28th, 2003, with only the most basic
of functionality. Ever since that day the world has seen frequent releases with new features and improvements. The author is still 
dedicated to the continued improvement and growth of this library. 

The official site for the library is <a href="http://www.angelcode.com/angelscript" target="_blank">http://www.angelcode.com/angelscript</a>.


\page main_topics Developer manual

 - \subpage doc_license
 - \subpage doc_start
 - \subpage doc_understanding_as
 - \subpage doc_register_api_topic
 - \subpage doc_compile_script
 - \subpage doc_call_script_func
 - \subpage doc_use_script_class
 - \subpage doc_callbacks
 - \subpage doc_advanced
 - \subpage doc_samples
 - \subpage doc_addon









\page doc_start Getting started

 - \subpage doc_overview
 - \subpage doc_compile_lib
 - \subpage doc_hello_world
 - \subpage doc_good_practice






\page doc_register_api_topic Registering the application interface

 - \subpage doc_register_api
 - \subpage doc_register_func
 - \subpage doc_register_prop
 - \subpage doc_register_type
 - \subpage doc_advanced_api
 

 
 

\page doc_advanced_api Advanced application interface

 - \subpage doc_strings
 - \subpage doc_arrays
 - \subpage doc_gc_object
 - \subpage doc_generic
 - \subpage doc_adv_generic_handle
 - \subpage doc_adv_scoped_type
 - \subpage doc_adv_single_ref_type
 - \subpage doc_adv_class_hierarchy
 - \subpage doc_adv_var_type
 - \subpage doc_adv_template
 - \subpage doc_adv_weakref
 - \subpage doc_cpp_exceptions




\page doc_advanced Advanced topics

 - \subpage doc_debug
 - \subpage doc_adv_timeout
 - \subpage doc_gc
 - \subpage doc_adv_multithread
 - \subpage doc_adv_concurrent
 - \subpage doc_adv_coroutine
 - \subpage doc_adv_import
 - \subpage doc_adv_dynamic_build
 - \subpage doc_adv_precompile
 - \subpage doc_finetuning
 - \subpage doc_adv_access_mask
 - \subpage doc_adv_namespace
 - \subpage doc_adv_dynamic_config
 - \subpage doc_adv_custom_options
 - \subpage doc_adv_reflection
 - \subpage doc_adv_inheritappclass
 - \subpage doc_adv_jit_topic






\page doc_adv_jit_topic JIT compilation

 - \subpage doc_adv_jit
 - \subpage doc_adv_jit_1



\page doc_script The script language

This is the reference documentation for the AngelScript scripting language.

 - \subpage doc_script_global
 - \subpage doc_script_statements
 - \subpage doc_expressions
 - \subpage doc_datatypes
 - \subpage doc_script_func
 - \subpage doc_script_class
 - \subpage doc_script_handle
 - \subpage doc_script_shared
 - \subpage doc_operator_precedence
 - \subpage doc_reserved_keywords
 - \subpage doc_script_bnf
 - \subpage doc_script_stdlib



\page doc_script_global Global entities

All global declarations share the same namespace so their names may not
conflict. This includes extended data types and built-in functions registered
by the host application. Also, all declarations are visible to all, e.g. a
function to be called does not have to be declared above the function that calls
it.

 - \subpage doc_global_func
 - \subpage doc_global_variable
 - \subpage doc_global_virtprop
 - \subpage doc_global_class
 - \subpage doc_global_interface
 - \subpage doc_script_mixin
 - \subpage doc_global_enums
 - \subpage doc_global_funcdef
 - \subpage doc_global_typedef
 - \subpage doc_global_namespace
 - \subpage doc_global_import

 
\page doc_global_func Functions

\ref doc_script_func "Global functions" provide the mean to implement 
routines that should operate on some input and produce a result. 

<pre>
  void foo()
  {
    // Do something
  }
</pre>



\page doc_script_func Functions

Functions are declared globally, and consists of a signature where the types of the arguments and the return value
is defined, and a body where the implementation is declared. 

- \subpage doc_script_func_decl
- \subpage doc_script_func_ref
- \subpage doc_script_func_retref
- \subpage doc_script_func_overload
- \subpage doc_script_func_defarg
- \subpage doc_script_anonfunc


\page doc_global_class Script classes

\ref doc_script_class "Script classes" are normally used to group together functions with values that will be 
operated upon by those functions. Multiple instances of a class can exists where each instance has different values. 

<pre>
  class Foo
  {
    void bar() { value++; }
    int value;
  }
</pre>


 

\page doc_script_class Script classes

Script classes are declared globally and provides an
easy way of grouping properties and methods into logical units. The syntax
for classes is similar to C++ and Java.

 - \subpage doc_script_class_desc
 - \subpage doc_script_class_inheritance
 - \subpage doc_script_class_private
 - \subpage doc_script_class_ops
 - \subpage doc_script_class_prop
 - \subpage doc_script_class_memberinit





*/
