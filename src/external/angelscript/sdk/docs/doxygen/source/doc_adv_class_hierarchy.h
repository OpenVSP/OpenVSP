/**

\page doc_adv_class_hierarchy Class hierarchies

AngelScript cannot automatically determine relationships between registered classes, so in 
order to establish the hierarchies for use within the script language it is necessary
to do a bit more registration beyond the normal \ref doc_register_type "object registration".

Hierarchies can currently only be registered for \ref doc_reg_basicref "reference types", 
not for \ref doc_register_val_type "value types".

\section doc_adv_class_hierarchy_1 Establishing the relationship

In order to let AngelScript know that two types are related you need to register the 
reference cast operators \ref doc_script_class_conv "opCast" and \ref doc_script_class_conv "opImplCast". The 
opCast should be used if you only want to allow the cast through an explicit
call with the <tt>\ref conversion "cast&lt;class&gt;"</tt> operator. opImplCast
should be used when you want to allow the compiler to implicitly perform the cast as necessary.

Usually you'll want to use opImplCast for casts from a derived type to the base type, 
and opCast for casts from a base type to a derived type.

\code
// Example opCast behaviour
template<class A, class B>
B* refCast(A* a)
{
    // If the handle already is a null handle, then just return the null handle
    if( !a ) return 0;

    // Now try to dynamically cast the pointer to the wanted type
    B* b = dynamic_cast<B*>(a);
    if( b != 0 )
    {
        // Since the cast was made, we need to increase the ref counter for the returned handle
        b->addref();
    }
    return b;
}

// Example registration of the behaviour
r = engine->RegisterObjectMethod("base", "derived@ opCast()", asFUNCTION((refCast<base,derived>)), asCALL_CDECL_OBJLAST); assert( r >= 0 );
r = engine->RegisterObjectMethod("derived", "base@ opImplCast()", asFUNCTION((refCast<derived,base>)), asCALL_CDECL_OBJLAST); assert( r >= 0 );

// Also register the const overloads so the cast works also when the handle is read only 
r = engine->RegisterObjectMethod("base", "const derived@ opCast() const", asFUNCTION((refCast<base,derived>)), asCALL_CDECL_OBJLAST); assert( r >= 0 );
r = engine->RegisterObjectMethod("derived", "const base@ opImplCast() const", asFUNCTION((refCast<derived,base>)), asCALL_CDECL_OBJLAST); assert( r >= 0 );
\endcode

Note that it may be necessary to add extra parenthesis to the <tt>asFUNCTION</tt> macro so that the preprocessor 
doesn't interpret the <tt>,</tt> in the template declaration as the argument separator in the macro.

\section doc_adv_class_hierarchy_2 Inherited methods and properties

Just as relationships cannot be determined, there is also no way to automatically let AngelScript
add inherited methods and properties to derived types. The reason for this is that method pointers
and property offsets may differ between the base class and derived class, especially when multiple
inheritance is used, and there is no way to automatically determine exactly what the difference is.

For this reason the application needs to register all the inherited methods and properties for 
the derived classes, which may lead to a bit of duplicate code. However, you may be able to avoid
the duplication through a bit of clever thinking. Here is an example of registering the methods and 
properties for a base class and the derived class (registration of behaviours has been omitted for briefness):

Observe that this way of doing it is not possible if the derived type hides members of the base type. 
If this is the case, then there is no option but to register each visible member in the derived type explicitly, 
excluding the hidden members of the base type.

\code
// The base class
class base
{
public:
  virtual void aMethod();
  
  int aProperty;
};

// The derived class
class derived : public base
{
public:
  virtual void aNewMethod();
  
  int aNewProperty;
};

// The code to register the classes
// This is implemented as a template function, to support multiple inheritance
template <class T>
void RegisterBaseMembers(asIScriptEngine *engine, const char *type)
{
  int r;

  r = engine->RegisterObjectMethod(type, "void aMethod()", asMETHOD(T, aMethod), asCALL_THISCALL); assert( r >= 0 );
  
  r = engine->RegisterObjectProperty(type, "int aProperty", asOFFSET(T, aProperty)); assert( r >= 0 );
}

template <class T>
void RegisterDerivedMembers(asIScriptEngine *engine, const char *type)
{
  int r;

  // Register the inherited members by calling 
  // the registration of the base members
  RegisterBaseMembers<T>(engine, type);

  // Now register the new members
  r = engine->RegisterObjectMethod(type, "void aNewMethod()", asMETHOD(T, aNewMethod), asCALL_THISCALL); assert( r >= 0 );

  r = engine->RegisterObjectProperty(type, "int aNewProperty", asOFFSET(T, aNewProperty)); assert( r >= 0 );
}

void RegisterTypes(asIScriptEngine *engine)
{
  int r;

  // Register the base type
  r = engine->RegisterObjectType("base", 0, asOBJ_REF); assert( r >= 0 );
  RegisterBaseMembers<base>(engine, "base");

  // Register the derived type
  r = engine->RegisterObjectType("derived", 0, asOBJ_REF); assert( r >= 0 );
  RegisterDerivedMembers<derived>(engine, "derived");
}
\endcode


*/
