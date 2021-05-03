/**

\page doc_script_func_decl Function declaration

Global functions provide the mean to implement routines that should operate on some input and produce a result. 
The functions themselves do not keep any memory, though they can update global variables, or memory passed by 
reference.

The function is always declared together with the function body. There is no need to declare function prototypes,
as the function is globally visible regardless where it has been declared.

<pre>
  // A simple function declaration
  int AFunction(int a, int b)
  {
    // Implement the function logic here
    return a + b;
  }
</pre>

The type of the return value should be specified before the name of the function. If the function doesn't return anything
the type should be defined as 'void'. After the function name, the list of parameters is specified between parenthesis. 
Each parameter is defined by its type and name.







\page doc_script_func_ref Parameter references

The parameters and return value can be of any type that can also be used for variable declarations. In addition, it is also
possible to make a function take a value by reference, i.e. rather than a copy of the original value, the parameter will refer
to the value.

Parameter references are used mainly for two purposes; a means of providing additional output from the function, or as a more performatic 
way of passing values.

In AngelScript it is necessary to specify the intention of the parameter reference, i.e. if it is meant as input, output, or both. 
This is necessary in order for the compiler to prepare the reference in a way that it cannot be invalidated due to some action during
the processing of the function.

Input references are written as &in. As the reference is meant as input only, the actual value it refers to normally is a copy of the 
original so the function doesn't accidentally modify the original value. These are not commonly used, as they provide little benefit
over passing arguments by value. Only in some circumstances can the performance be improved, especially if the parameter is declared as 
const too.

Output references are written as &out. These references are meant to allow the function to return additional values. When going in to 
the function, the reference will point to an uninitialized value. After the function returns, the value assigned by the function will be 
copied to the destination determined by the caller.

When the reference is meant as both input and output, it is declared as &inout, or just &. In this case the reference will point to the
actual value. Only \ref doc_datatypes_obj "reference types", i.e. that can have handles to them, are allowed to be passed as inout references. This is because 
in order to guarantee that the reference will stay valid during the entire execution of the function, the value must be located in the 
memory heap.

<pre>
  void Function(const int &in a, int &out b, Object &c)
  {
    // Assigning an output value to the output reference
    b = a;
  
    // The object is an inout reference and refers to the real object
    c.DoSomething();
  }
</pre>






\page doc_script_func_retref Return references

A function can also return references, which will allow the caller to modify the value pointed to 
by the reference. To declare a function that return a reference include the &amp; symbol between the 
return type and name of the function. Add const before the type if the reference should be read only, 
i.e. it shouldn't be possible to modify the value it points to.

<pre>
  int property;
  int &Function()
  {
    // Return a reference to the property
    return property;
  }
  
  void main()
  {
    // Modify the value pointed to by the returned reference
    Function() = 1;
  }
</pre>

Due to the need to guarantee that the reference is valid even after the function returns to the 
caller there exist certain restrictions. You don't need to try to remember these restrictions as 
the compiler will give an error if you they are violated, but if you do encounter a compile
error when returning a reference it will be good to understand why it is happening so that
you can determine how to avoid it.

\section doc_script_retref_global References to global variables are allowed

As a global variable is in the global scope, the lifetime of the variable is longer than the
scope of the function. A function can thus return a reference to a global variable, or even member 
of an object reached through a global variable.

\section doc_script_refref_member References to class members are allowed

A class method can return a reference to a class property of the same object, because the caller
is required to hold a reference to the object it is known that the member will exist even after the
method returns. 

The class method is also allowed to return reference to global variables, just like any other function.

\section doc_script_retref_local Can't return reference to local variables

Because local variables must be freed when the function exits, it is not allowed to return
a reference to them. The same is also true for any parameters that the function received. The
parameters are also cleaned up when the function exits, so they too cannot be returned by 
reference.

\section doc_script_retref_deferred Can't use expressions with deferred parameters

For some function calls with arguments, there may be a necessary processing of the arguments
after the function call returned, e.g. to clean up the input object, or to assign the output 
parameters. If the function that was called is returning a reference, then that reference cannot
in turn be returned again, as it may be invalidated by the deferred evaluation of the arguments.

\section doc_script_refref_cleanup Can't use expressions that rely on local objects

All local objects must be cleaned up before a function exits. For functions that return references
this clean-up happens before the return expression is evaluated, otherwise the cleanup of the
local objects may accidentally invalidate the reference. For this reason it is not possible
to use expressions that rely on local objects to evaluate the reference that should be returned.

Primitive values can be used though, as these do not require cleanup upon exit.






\page doc_script_func_overload Function overloading

Function overloading happens when more than one function with the same name is declared with
different parameters. This is a useful feature when an operation needs to be able to work with
different types of input, yet produce similar result. 

The compiler is able to resolve which function to call by matching the type of each argument 
expression to the function parameter, and eliminating the functions where there is no possible 
conversion available. The compiler will do this for each argument from first to last. When all
arguments have been evaluated only one function should be the best match, otherwise the compiler
will give an error about the impossibility to determine the correct function to call.

The type of conversion that needs to be performed on the argument to get to the type of the 
parameter determines how well a function matches. The following list gives the order of how one 
type of conversion compares to another.

 - no conversion needed
 - conversion to const
 - size of primitive changes 
 - integer type to float type and vice versa
 - reference cast
 - object to primitive conversion
 - conversion to object
 - variable argument type

Observe that it is not possible to create overloads where the only difference is the return
type. This is because the return type is not part of the selection criteria that the compiler
uses to determine which function to call, the return type is just the result of the called 
function.

<pre>
  void Function(int a, float b, string c) {}
  void Function(string a, int b, float c) {}
  void Function(float a, string b, int c) {}

  void main()
  {
    Function(1, 2.5f, 'a');  // Will call the first overload
    Function('a', 1, 2.5f);  // Will call the second overload
    Function(2.5f, 'a', 1);  // Will call the third overload
  }  
</pre>








\page doc_script_func_defarg Default arguments

Sometimes implementing different functions for each overload is unnecessary when the difference can 
be provided with a default value to a parameter. This is where default arguments come in handy.

By defining default arguments in the declaration of the function, the script doesn't have to provide
these values specifically when calling the function as the compiler will automatically fill in the 
default arguments. 

<pre>
  void Function(int a, int b = 1, string c = "")
  {
    // Inside the function the arguments work normally
  }

  void main()
  {
    // Default arguments doesn't have to be informed
    // The following three calls produce the exact same result
    Function(0);
    Function(0,1);
    Function(0,1,"");
  }
</pre>

When defining a default argument to one of the parameters, all subsequent parameters must have a 
default argument too.

The default argument expression can include references to variables or call functions, but only if
the variables or functions are visible in the global scope.

<pre>
  int myvar = 42;
  void Function(int a, int b = myvar) {}
  void main()
  {
    int myvar = 1;
    Function(1);    // This will use the global myvar and not the local myvar
  }
</pre>

The special 'void' expression can be used as default argument to make an optional output parameter.

<pre>
  void func(int &out output = void) { output = 42; }
</pre>



\page doc_script_anonfunc Anonymous functions

Anonymous functions, or lambdas as they are sometimes called, are functions that are 
declared locally for use with \ref doc_datatypes_funcptr "function handles". 

The following is a quick demonstration on how to use anonymous functions.

<pre>
  funcdef bool CMP(int first, int second);
  
  void main()
  {
    int valueA = 1, valueB = 2;
    
    bool result1 = func(valueA, valueB, function(a,b){ return a == b; });
    bool result2 = func(valueA, valueB, function(a,b){ return a != b; });
  }
  
  bool func(int a, int b, CMP \@f)
  {
    return f(a,b);
  }
</pre>

The anonymous function takes on the signature of the function handle it is assigned to, 
so the type of the arguments and the return type doesn't have to be explicitly declared.

It is not yet possible for anonymous functions to access variables declared in the same 
scope as the function, i.e. they cannot be used as closures.

If there are multiple matching uses for the anonymous function it will be necessary to 
explicitly inform the parameter types, so the ambiguity can be resolved.

<pre>
  funcdef void A(int);
  funcdef void B(float);
  void func(A@) {}
  void func(B@) {}
  
  void main()
  {
    // Explicitly specify the type to tell the compiler that A is wanted
    func(function(int a) {});
  }
</pre>


*/
