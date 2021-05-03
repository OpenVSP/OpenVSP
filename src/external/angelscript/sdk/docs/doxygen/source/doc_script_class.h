/**


\page doc_script_class_desc Script class overview

With classes the script writer can declare new data types that hold groups
of properties and methods to manipulate them. 

Script classes are reference types, which means that multiple references 
or \ref doc_script_handle "handles" can be held for the same object instance.
The classes uses automatic memory management so the object instances are only
destroyed when the last reference to the instance is cleared.

The class methods are implemented the same way as \ref doc_script_func "global functions", 
with the addition that the class method can access the class instance properties through either
directly or through the 'this' keyword in the case a local variable has the same name.

<pre>
  // The class declaration
  class MyClass
  {
    // A class method
    void DoSomething()
    {
      // The class properties can be accessed directly
      a *= 2;

      // The declaration of a local variable may hide class properties
      int b = 42;

      // In this case the class property have to be accessed explicitly
      this.b = b;
    }

    // Class properties
    int a;
    int b;
  }
</pre>

A class can implement specific methods to \ref doc_script_class_ops "overload operators".
This can simplify how the object instances are used in expressions, so that it is not
necessary to explicitly name each function, e.g. the opAdd method translates to the + operator.

Another useful feature is the ability to implement \ref doc_script_class_prop "property accessors",
which can be used either to provide virtual properties, i.e. that look like properties but really 
aren't, or to implement specific routines that must be executed everytime a property is accessed.

A script class can also \ref doc_script_class_inheritance "inherit" from other classes, and 
implement \ref doc_global_interface "interfaces".




\section doc_script_class_construct Class constructors

Class constructors are specific methods that will be used to create new instances
of the class. It is not required for a class to declare constructors, but doing
so may make it easier to use the class as it will not be necessary to first instanciate
the class and then manually set the properties.

The constructors are declared without a return type, and must have the same name as
the class itself. Multiple constructors with different parameter lists can be 
implemented for different forms of initializations.

<pre>
  class MyClass
  {
    // Implement a default constructor
    MyClass()
    {
    }

    // Implement the copy constructor
    MyClass(const MyClass &in other)
    {
      // Copy the value of the other instance
    }

    // Implement other constructors with different parameter lists
    MyClass(int a, string b) {}
    MyClass(float x, float y, float z) {}
  }
</pre>

The copy constructor is a specific constructor that the compiler can use to build
more performatic code when copies of an object must be made. Without the copy 
constructor the compiler will be forced to first instanciate the copy using the 
default constructor, and then copy the attributes with the 
\ref doc_script_class_prop "opAssign" method.

One constructor cannot call another constructor. If you wish to share 
implementations in the constructors you should use a specific method for that.

If a class isn't explicitly declared with any constructor, the compiler will automatically
provide a default constructor for the class. This automatically generated constructor will
simply call the default constructor for all object members, and set all handles to null. 
If a member cannot be initialized with a default constructor, then a compiler error will be
emitted.

How the members shall be initialized can also be defined directly in the declaration of the 
members. When this is done the initialization expression will automatically be compiled in the 
constructor without the need to write the initialization again. 

\see \ref doc_script_class_memberinit





\section doc_script_class_destruct Class destructor

It is normally not necessary to implement the class destructor as AngelScript
will by default free up any resources the objects holds when it is destroyed. 
However, there may be situations where a more explicit cleanup routine must be
done as part of the destruction of the object.

The destructor is declared similarly to the constructor, except that it must be
prefixed with the ~ symbol (also known as the bitwise not operator).

<pre>
  class MyClass
  {
    // Implement the destructor if explicit cleanup is needed
    ~MyClass()
    {
      // Perform explicit cleanup here
    }
  }
</pre>

Observe that AngelScript uses automatic memory management with garbage collection
so it may not always be easy to predict when the destructor is executed. AngelScript will also
call the destructor only once, even if the object is resurrected by adding a
reference to it while executing the destructor.

It is not possible to directly invoke the destructor. If you need to be able to 
directly invoke the cleanup, then you should implement a public method for that.




\section doc_script_class_const Const methods

Classes add a new type of \ref doc_script_func_overload "function overload", i.e. const overload. 
When a class method is accessed through a read-only reference or handle, only methods that have
been marked as constant can be invoked. When the reference or handle is writable, then both types can be 
invoked, with the preference being the non-const version in case both matches.

<pre>
  class CMyClass
  {
    int method()       { a++; return a; } 
    int method() const {      return a; }
    int a;
  }
  void Function()
  {
     CMyClass o;
     const CMyClass \@h = o;

     o.method(); // invokes the non-const version that increments the member a
     h.method(); // invokes the const version that doesn't increment the member a
  }
</pre>






\page doc_script_class_inheritance Inheritance and polymorphism

AngelScript supports single inheritance, where a derived class inherits the 
properties and methods of its base class. Multiple inheritance is not supported,
but polymorphism is supported by implementing \ref doc_global_interface "interfaces", 
and code reuse is provided by including \ref doc_script_mixin "mixin classes".

All the class methods are virtual, so it is not necessary to specify this manually. 
When a derived class overrides an implementation, it can extend 
the original implementation by specifically calling the base class' method using the
scope resolution operator. When implementing the constructor for a derived class
the constructor for the base class is called using the <code>super</code> keyword. 
If none of the base class' constructors is manually called, the compiler will 
automatically insert a call to the default constructor in the beginning. The base class'
destructor will always be called after the derived class' destructor, so there is no
need to manually do this.

<pre>
  // A derived class
  class MyDerived : MyBase
  {
    // The default constructor
    MyDerived()
    {
      // Calling the non-default constructor of the base class
      super(10);
      
      b = 0;
    }
    
    // Overloading a virtual method
    void DoSomething()
    {
      // Call the base class' implementation
      MyBase::DoSomething();
      
      // Do something more
      b = a;
    }
    
    int b;
  }
</pre>

A class that is derived from another can be implicitly cast to the base class. The same works for 
interfaces that are implemented by a class. The other direction requires an \ref conversion "explicit cast", as it
is not known at compile time if the cast is valid.

<pre>
  class A {}
  class B : A {}
  void Foo()
  {
    A \@handle_to_A;
    B \@handle_to_B;

    \@handle_to_A = A(); // OK
    \@handle_to_A = B(); // OK. The reference will be implicitly cast to A@

    \@handle_to_B = A(); // Not OK. This will give a compilation error
    \@handle_to_B = B(); // OK

    \@handle_to_A = handle_to_B; // OK. The reference will be implicitly cast to A@
    \@handle_to_B = handle_to_A; // Not OK. This will give a compilation error

    \@handle_to_B = cast<B>(handle_to_A); // OK. Though, the explicit cast will return null 
                                         // if the object in handle_to_a is not really an
                                         // instance of B
  }
</pre>

\section doc_script_class_inheritance_2 Extra control with final, abstract, and override

A class can be marked as 'final' to prevent the inheritance of it. This is an optional feature and
mostly used in larger projects where there are many classes and it may be difficult to manually 
control the correct use of all classes. It is also possible to mark individual class methods of a 
class as 'final', in which case it is still possible to inherit from the class, but the finalled
method cannot be overridden.

Another keyword that can be used to mark a class is 'abstract'. Abstract classes cannot be 
instantiated, but they can be derived from. Abstract classes are most frequently used when you
want to create a family of classes by deriving from a common base class, but do not want the
base class to be instantiated by itself. It is currently not possible to mark methods as abstract
so all methods must have an implementation even for abstract classes.

<pre>
  // A final class that cannot be inherited from
  final class MyFinal
  {
    MyFinal() {}
    void Method() {}
  }
  
  // A class with individual methods finalled
  class MyPartiallyFinal
  {
    // A final method that cannot be overridden
    void Method1() final {}

    // Normal method that can still be overridden by derived class
    void Method2() {}
  }
  
  // An abstract class
  abstract class MyAbstractBase {}
</pre>

When deriving a class it is possible to tell the compiler that a method is meant to override a method in the 
inherited base class. When this is done and there is no matching method in the base class the compiler will
emit an error, as it knows that something wasn't implemented quite the way it was meant. This is especially
useful to catch errors in large projects where a base class might be modified, but the derived classes was 
forgotten.

<pre>
  class MyBase
  {
    void Method() {}
    void Method(int) {}
  }
  
  class MyDerived : MyBase
  {
    void Method() override {}      // OK. The method is overriding a method in the base class
    void Method(float) override {} // Not OK. The method isn't overriding a method in base class
  }
</pre>






\page doc_script_class_private Protected and private class members

Class members can be declared as protected or private to control where they can be 
accessed from. Protected members cannot be accessed from outside the class. Private 
members additionally cannot be accessed by derived classes.

This can be useful in large programs where you wish to avoid programmer errors where 
properties or methods are inappropriately used.

<pre>
  // A class with private members
  class MyBase
  {
    // The following are public members
    void PublicFunc()
    {
      // The class can access its own protected and private members
      ProtectedProp = 0; // OK
      ProtectedFunc();   // OK
      PrivateProp = 0;   // OK
      PrivateFunc();     // OK
    }    
 
    int PublicProp;

    // The following are protected members
    protected void ProtectedFunc() {}
    protected int ProtectedProp;

    // The following are private members
    private void PrivateFunc() {} 
    private int PrivateProp;
  }
  
  class MyDerived : MyBase
  {
    void Func()
    {
      // The derived class can access the protected members
      // of the base class but not the private members
      ProtectedProp = 1; // OK
      ProtectedFunc();   // OK
      PrivateProp = 1;   // Error
      PrivateFunc();     // Error  
    }
  }

  void GlobalFunc()
  {
    MyBase obj;

    // Public members can be accessed normally
    obj.PublicProp = 0;  // OK
    obj.PublicFunc();    // OK

    // Accessing protected and private members will give a compiler error
    obj.ProtectedProp = 0; // Error
    obj.ProtectedFunc();   // Error
    obj.PrivateProp = 0;   // Error
    obj.PrivateFunc();     // Error
  }
</pre>






\page doc_script_class_memberinit Initialization of class members

The order in which the class members are initialized during the construction of an object becomes 
important when using inheritance, or when defining the initialization of the members directly in
the declaration. If a member is accessed before it has been initialized the script may cause a null 
handle access exception, which will abort the execution of the script.

For a simple class, the order in which the members are initialized is the same as the order in which 
they were declared. When explicit initializations are given in the declaration of the members, these 
members will be initialized last. 

<pre>
  // The order of this class will be: a, c, b, d
  class Foo
  {
    string a;
    string b = a;
    string c;
    string d = b;
  }
</pre>

When \ref doc_script_class_inheritance "inheritance" is used, the derived class' members without 
explicit initialization will be initialized before the base class' members, and the members with 
explicit initialization will be initialized after the base class' members.

<pre>
  // The order of this class will be: a, b
  class Bar
  {
    string a;
    string b = a;
  }

  // The order of this class will be: d, a, b, c
  class Foo : Bar
  {
    string c = a;
    string d;
  }
</pre>

This order of initialization has been chosen to avoid most problems with accessing members before they
have been initialized. 

All members are initialized immediately in the beginning of the defined constructor, so the rest of the 
code in the constructor can access members without worry. The exception is when the constructor explicitly
initializes a base class by calling super(), in this case the members with explicit initialization will
remain uninitialized until after the base class has been fully constructed.

<pre>
  class Bar
  {
    Bar(string val) { a = val; }
    string a;
  }

  class Foo : Bar
  {
    Foo()
    {
      // b is already initialized here

      super(b); // a will be initialized in this call

      // c is initialized right after super() returns
    }
    
    string b;
    string c = a;
  }
</pre>

Be wary about cases where a constructor or member initialization calls class methods. As class methods can
be overridden by derived classes it is possible for a base class to unwittingly access a member of the derived 
class before it has been initialized.

<pre>
  class Bar
  {
    Bar() 
    {
      DoSomething();
    }

    void DoSomething() {}
  }
  
  // This class will cause a null handle exception, because the Bar's constructor calls 
  // the DoSomething() method that accesses the member msg before it has been initialized. 
  class Foo : Bar
  {
    string msg = 'hello';
    void DoSomething() 
    { 
      print(msg); 
    }
  }
</pre>





*/
