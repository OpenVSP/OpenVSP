/**

\page doc_versions Versions

The library's version number is composed of 3 numbers, and sometimes an additional letter. Examples:

 - 1.10.1d
 - 2.8.0a
 - 2.30.2

The first number is the major version. This is updated whenever there is a major redesign, or a major milestone happens. When
this number is changed backwards compatibility is not necessarily maintained by the library.

The second number is the application interface version. This is updated whenever there is a change in the interface, e.g.
a new method is added or an existing method has its signature modified. Usually the change is incremental so that there is no 
need to modify the application code unless the new functions are wanted, but a recompilation is still necessary. Sometimes parts 
of the interface are deprecated, but whenever possible the deprecated parts can be turned back on through a compile time flag.

The third number is the minor version. This is updated when the back-end logic in the library is modified, e.g. adding 
or modifying a script language feature, but no interface change is done. No change in the application code should be required 
during an upgrade when only the minor version has changed.

The additional letter is used when the change is purely bug fixes. In this case the binary application interface should be
untouched, so an application that loads the library as a shared object (dll) should be able to upgrade the library without 
recompiling the application.
 

\section doc_versions_milestones History

The following is a bit of history for the library that shows how it has evolved since the beginning to the current date. The
complete change list can be found on the AngelScript site and in the SDK.

\subsection doc_versions_2003 2003 - Birth and first public release

The library was first conceived in early 2003, with the first working (internal) version available in March, 2003. 

At the end of June, 2003 the library's first public release with version 1.0 was made. 

\subsection doc_versions_2005 2005 - Version 2, sand box, object handles, script classes, and garbage collection

In the beginning of 2005 I made the difficult decision to do a major change of direction. Version 1 had several flaws that
I couldn't solve without a major redesign. The worst was that the script language wasn't properly sand-boxed, i.e. the 
script writer had to be careful when writing scripts in order to crash the application or cause worse problems. The script
compiler didn't make any attempts to guarantee that a value passed by reference to a function actually stayed alive during the
call, and the script was allowed to use and manipulate pointers to access memory directly.

Version 2.0 included the concept of object handles, \@, and parameter references with direction keywords, i.e. <tt>in</tt> & <tt>out</tt>. 
Object handles are basically smart pointers that the compiler uses to guarantee the proper life time of objects, and the direction keywords
for parameter references are used to tell the compiler what the reference will be used for. This in turn allows the compiler to take 
necessary actions to guarantee the lifetime of the value, e.g. to create a temporary object that will receive the output value, which
will only be assigned to the final destination after the called function returns to the caller.

With the release of 2.2.0 in mid-2005 the script language got its first version of script classes, at the time called <tt>struct</tt>. Before this 
the script language didn't support defining any new types, and scripts could only implement global functions for interacting
the built-in types as well as application registered types.

With the introduction of classes the need for the incremental garbage collector was also born, as the scripts could now 
create objects with circular relationships that the simple reference counting memory model couldn't handle by itself.

\subsection doc_versions_2006 2006 - Script interface

In 2006, with the release of 2.7.0, the concept of interfaces was introduced in the script language. Allowing the application
to register a script interface that the script class could implement made for an easier interaction between application and
script. 

\subsection doc_versions_2009 2009 - Inheritance, template types, operator overloads, and JIT compilation

Only in 2009, with version 2.15.2, was class inheritance implemented, though only single inheritance, to avoid complicating both the library and script 
language. Multiple interfaces are still allowed to support polymorphism.

Still in 2009, with version 2.16.1, the the library added support for registering templated types, to allow the application to 
create its own custom container types. The built-in array type would eventually be completely replaced by this and a default 
array add-on is provided.

With version 2.16.3 the script classes were enhanced to allow implementing operator overloads, thus making the language even
more flexible and powerful.

Version 2.17.0, released in August 2009, introduced an interface to support external JIT compilers. While the AngelScript
library doesn't provide its own JIT compiler, a third party JIT compiler was soon created with quick adoption by the community.

\subsection doc_versions_2010 2010 - Function pointers

Support for function pointers in the script language was added in 2010 with version 2.18.2. Now the application can allow
scripts to setup its own callbacks.

\subsection doc_versions_2011 2011 - Automatic garbage collection and debugging

With the release of version 2.21.0 in 2011 the script engine gained support for automatic garbage collection to simplify this
task for the application developer. Of course, the application developers that prefer to have full control can still do it 
manually by turning off the automatic collection.

This release also saw the implementation of a first standard debugger add-on that can be used by applications to add support
for stepping through the scripts and inspecting variables for proper run-time debugging.

\subsection doc_versions_2012 2012 - Namespaces and mixins

Namespaces were added to the script language in 2012 with version 2.22.2.

Mixin classes were introduced with version 2.25.0 to provide better support for code re-use even without multiple inheritance.

\subsection doc_versions_2013 2013 - Improved template types, delegates, weak references, and initialization lists

The template types that were first introduced in 2009 were enhanced with version 2.26.1 to support multiple subtypes in order 
to give greater flexibility for the application. 

Support for delegates was added with version 2.26.3.

Version 2.27.0 included support for weak references, so that is possible to have greater control over which handles keep objects 
alive or not. 

Support for registrable factory functions that takes initialization lists was added in 2.28.0. This allows the application to
define the specific pattern that should be used for different types using a simple syntax. 

\subsection doc_versions_2014 2014 - Named arguments and auto 

In version 2.29.0 the script language gained support for named arguments, and <tt>auto</tt> for automatically determining the 
type of variables based on the initialization expression.

\subsection doc_versions_2015 2015 - Anonymous functions

Anonymous functions were first implemented in version 2.30.2.

\subsection doc_versions_2016 2016 - Child funcdefs

Funcdefs can be declared as members of classes as of version 2.31.0.

\subsection doc_versions_2017 2017 - external keyword and anonymous initialization lists

The <tt>external</tt> keyword was introduced in version 2.32.0 to explicitly include shared entities from a previously compiled module. This version also introduced anonymouus initialization lists for more compact scripts.

\subsection doc_versions_2018 2018 - Try-catch statements and explicit constructors

In version 2.33.0 the language was improved to support try-catch statements, and for flagging constructors as explicit for better control over what type conversions can happen implicitly.

\subsection doc_versions_2019 2019 - Explicit property keyword

With version 2.34.0 virtual property access must be explicitly flagged with the property keyword so that functions starting with get_ or set_ won't automatically be interpreted as virtual properties.

\subsection doc_versions_2022 2022 - Serialization of the context stack

With version 2.36.0 the ability to serialize the context stack was implemented.

*/
