/**

\page doc_adv_inheritappclass Inheriting from application registered class

A script class cannot directly inherit from an application registered class,
as the script classes are not compiled into native machine code like the
application classes are.

It is however possible to emulate the inheritance by using a proxy class
to hide the underlying differences in an abstract layer. The proxy class has
two parts, one is the C++ side that the application sees, and the other is
the script side that the scripts can see and inherit from.

The following is an example implementation of such a proxy class.

\code
// On the C++ side
class FooScripted
{
public:
  // Public interface that we want the script to be able to override 
  void CallMe()
  {
    // If the script side is still alive, then call the scripted function
    if( !m_isDead->Get() )
    {
      asIScriptEngine *engine = m_obj->GetEngine();
      asIScriptContext *ctx = engine->RequestContext();

      // GetMethodByDecl returns the virtual function on the script class
      // thus when calling it, the VM will execute the derived method
      ctx->Prepare(m_obj->GetObjectType()->GetMethodByDecl("void CallMe()"));
      ctx->SetObject(m_obj);
      ctx->Execute();

      engine->ReturnContext(ctx);
    }
  }

  int m_value;

  // A factory function that can be used by the script side to create
  static FooScripted *Factory()
  {
    asIScriptContext *ctx = asGetActiveContext();

    // Get the function that is calling the factory so we can be certain it is the FooScript script class
    asIScriptFunction *func = ctx->GetFunction(0);
    if( func->GetObjectType() == 0 || std::string(func->GetObjectType()->GetName()) != "FooScripted" )
    {
      ctx->SetException("Invalid attempt to manually instantiate FooScript_t");
      return 0;
    }

    // Get the this pointer from the calling function so the FooScript C++
    // class can be linked with the FooScript script class
    asIScriptObject *obj = reinterpret_cast<asIScriptObject*>(ctx->GetThisPointer(0));

    return new FooScripted(obj);
  }

  // Reference counting
  void AddRef()
  {
    m_refCount++;

    // Increment also the reference counter to the script side so
    // it isn't accidentally destroyed before the C++ side
    if( !m_isDead->Get() )
      m_obj->AddRef();
  }
  void Release()
  { 
    // Release the script instance too
    if( !m_isDead->Get() )
      m_obj->Release();

    if( --m_refCount == 0 ) delete this;
  }

  // Assignment operator
  FooScripted &operator=(const FooScripted &o)
  {
    // Copy only the content, not the script proxy class
    m_value = o.m_value;
    return *this;
  }

protected:

  // The constructor and destructor are indirectly called
  FooScripted(asIScriptObject *obj) : m_obj(0), m_isDead(0), m_value(0), m_refCount(1)
  {
    // Get the weak ref flag for the script object to
    // avoid holding a strong reference to the script class
    m_isDead = obj->GetWeakRefFlag();
    m_isDead->AddRef();

    m_obj = obj;
  }

  ~FooScripted()
  {
    // Release the weak ref flag
    m_isDead->Release();
  }

  // Reference count
  int m_refCount;

  // The C++ side holds a weak link to the script side to
  // avoid a circular reference between the C++ side and
  // script side
  asILockableSharedBool *m_isDead;
  asIScriptObject *m_obj;
};
\endcode

This type is registered with the engine as the following:

\code
void RegisterFooScripted(asIScriptEngine *engine)
{
  engine->RegisterObjectType("FooScripted_t", 0, asOBJ_REF);
  engine->RegisterObjectBehaviour("FooScripted_t", asBEHAVE_FACTORY, "FooScripted_t @f()", asFUNCTION(FooScripted::Factory), asCALL_CDECL);
  engine->RegisterObjectBehaviour("FooScripted_t", asBEHAVE_ADDREF, "void f()", asMETHOD(FooScripted, AddRef), asCALL_THISCALL);
  engine->RegisterObjectBehaviour("FooScripted_t", asBEHAVE_RELEASE, "void f()", asMETHOD(FooScripted, Release), asCALL_THISCALL);
  engine->RegisterObjectMethod("FooScripted_t", "FooScripted_t &opAssign(const FooScripted_t &in)", asMETHOD(FooScripted, operator=), asCALL_THISCALL);
  engine->RegisterObjectMethod("FooScripted_t", "void CallMe()", asMETHOD(FooScripted, CallMe), asCALL_THISCALL);
  engine->RegisterObjectProperty("FooScripted_t", "int m_value", asOFFSET(FooScripted, m_value));
}
\endcode

The script side is declared as \ref doc_script_shared "shared" so it can be used in all script modules.
It is also declared as \ref doc_script_class_inheritance_2 "abstract" so it cannot be instantiated by 
itself, only as a parent class of another script class.

This script section should preferably be included automatically by the application in 
all the modules that should be able to derive from the FooScripted class.

<pre>
  // On the script side
  shared abstract class FooScripted
  {
    // Allow scripts to create instances
    FooScripted()
    {
      // Create the C++ side of the proxy
      \@m_obj = FooScripted_t();  
    }

    // The copy constructor performs a deep copy
    FooScripted(const FooScripted &o)
    {
      // Create a new C++ instance and copy content
      \@m_obj = FooScripted_t();
      m_obj = o.m_obj; \n"
    }

    // Do a deep a copy of the C++ object
    FooScripted &opAssign(const FooScripted &o)
    {
      // copy content of C++ instance
      m_obj = o.m_obj;
      return this;
    }

    // The script side forwards the call to the C++ side
    void CallMe() { m_obj.CallMe(); }

    // The C++ side property is exposed to the script through accessors
    int m_value 
    {
      get { return m_obj.m_value; }
      set { m_obj.m_value = value; }
    }

    // The script class can be implicitly cast to the C++ type through the opImplCast method
    FooScripted_t \@opImplCast() { return m_obj; }
    
    // Hold a reference to the C++ side of the proxy
    private FooScripted_t \@m_obj;
  }
</pre>

Now the scripts classes can derive from the FooScripted class  
and access the properties and methods of the parent class normally.

<pre>
  // Implement a script class that derives from the application class
  class FooDerived : FooScripted
  {
    void CallMe()
    {
       m_value += 1;
    }
  }

  void main()
  {
    // When newly created the m_value is 0
    FooDerived d;
    assert( d.m_value == 0 );

    // When calling the method the m_value is incremented with 1
    d.CallMe();
    assert( d.m_value == 1 );
  }
</pre>

It is of course also possible to create an instance of the scripted class from
the application and access it through the FooScripted C++ proxy, thus making 
it transparent from the rest of the application that the implementation is actually
in the script.

\code
FooScripted *CreateFooDerived(asIScriptEngine *engine)
{
  // Create an instance of the FooDerived script class that inherits from the FooScripted C++ class
  asIScriptObject *obj = reinterpret_cast<asIScriptObject*>(engine->CreateScriptObject(mod->GetTypeInfoByName("FooDerived")));

  // Get the pointer to the C++ side of the FooScripted class
  FooScripted *obj2 = *reinterpret_cast<FooScripted**>(obj->GetAddressOfProperty(0));

  // Increase the reference count to the C++ object, as this is what will 
  // be used to control the life time of the object from the application side 
  obj2->AddRef();

  // Release the reference to the script side
  obj->Release();

  return obj2;
}

void Foo(asIScriptEngine *engine)
{
  FooScripted *obj = CreateFooDerived(engine);

  // Once the object is created the application can access it normally through
  // the FooScripted pointer, without having to know that the implementation
  // is actually done in the script.

  // When newly created the m_value is 0
  assert( obj->m_value == 0 );

  // When calling the method the m_value is incremented with 1 by the script 
  obj->CallMe();
  assert( obj->m_value == 1 );

  // Release the object to destroy the instance (this will also destroy the script side)
  obj->Release();
}
\endcode





*/
