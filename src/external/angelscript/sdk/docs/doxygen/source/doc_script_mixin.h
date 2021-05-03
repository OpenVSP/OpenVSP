/**


\page doc_script_mixin Mixin class

As \ref doc_script_class_inheritance "multiple inheritance" is not available, 
it may sometimes be necessary to implement identical code in multiple classes. 
When this is necessary it is recommended to use mixin classes to avoid writing
the identical code in multiple places.

Mixin classes allows a script to declare a partial class structure that will be 
included into multiple different class declarations. The mixin classes themselves
are not real types and cannot be instanciated. 

When a mixin class is included into a class declaration, the properties and methods 
that were declared in the mixin class will be automatically replicated into the class. 

<pre>
  // Declare a mixin class
  mixin class MyMixin
  {
    void SomeMethod() { property++; }
    int property;
  }

  // Include the mixin class into the class to receive the methods and properties
  class MyClass : MyMixin
  {
    int OtherMethod()
    {  
      SomeMethod();
      return property;
    }
  }
</pre>

Properties and methods that have already been explicitly declared in the class will 
not be included again. This way the mixin class can provide a default implementation
that can be overridden by the class that includes the mixin.

The class methods included from a mixin class will be compiled in the context of the class
that included it, so it is possible for a mixin class method to refer to properties and other 
methods that are not declared in the mixin class if the class that includes the mixin class 
provides those.

<pre>
  mixin class MyMixin
  {
    void MethodA() { print("Default behaviour"); } 
    void MethodB() { property++; }
  }
  
  class MyClass : MyMixin
  {
    // Override the default behaviour in MethodA
    void MethodA() { print("Overridden behaviour"); }

    // Declare the property that will be used by MethodB
    int property;
  }
</pre>


Mixin class methods override inherited methods from base classes, just as if the included method
had been implemented in the derived class directly. Mixin class properties on the other hand
are not included if the property is already inherited from a base class.

<pre>
  class MyBase
  {
    void MethodA() { print("Base behaviour"); }
    int property;
  }

  mixin class MyMixin
  {
    void MethodA() { print("Mixin behaviour"); }
    float property;
  }
  
  // Inheriting from base class and including mixin.
  // MyClass ends up with the property from the base 
  // class and the method from the mixin class.
  class MyClass : MyBase, MyMixin
  {
  }
</pre>


A mixin class can inform a list of interfaces that has to be implemented by the script 
class that includes the mixin class. In this case the methods for the interfaces can optionally 
be provided by the mixin class itself, or left out to be implemented by the script class directly.

A mixin class cannot inherit from other classes.

<pre>
  interface I 
  {
    void a();
    void b();	
  }
  
  mixin class M : I
  {
	// provide default implementation of a()
    void a() { print("hello from a");

    // leave the implementation of b() to the script class
  }
  
  class C : M
  {
    // a() is implemented by mixin class
	
	// b() has to be explicitly implemented by script class
	void b() { print("hello from b");
  }
</pre>


*/
