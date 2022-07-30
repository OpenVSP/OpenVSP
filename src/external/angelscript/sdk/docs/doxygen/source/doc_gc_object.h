/**

\page doc_gc_object Garbage collected objects

Reference counting as memory management has a drawback in that it is
difficult to detect circular references when determining dead objects.
AngelScript allows the application to register types with special behaviours
to support the garbage collection for detecting circular references. These
behaviours make the class a bit more complex, but you should only have to
register them for a few types, e.g. generic container classes.

\code
// Registering the garbage collected reference type
r = engine->RegisterObjectType("ref_type", 0, asOBJ_REF | asOBJ_GC); assert( r >= 0 );
\endcode

The difference between the garbage collected and non-garbage collected
types is in the addref and release behaviours, the class constructor, and
the extra support behaviours.

\see The \ref doc_addon_dict "dictionary" add-on for an example of a garbage collected object

\section doc_reg_gcref_1 GC support behaviours

The GC determines when objects should be destroyed by counting the
references it can follow for each object. If the GC can see all references
that points to an object, it knows that the object is part of a circular
reference. If all the objects involved in that circular reference have no
outside references it means that they should be destroyed.

The process of determining the dead objects uses the first four of the
behaviours below, while the destruction of the objects is done by forcing the
release of the object's references.

\code
void CGCRef::SetGCFlag()
{
    // Set the gc flag as the high bit in the reference counter
    refCount |= 0x80000000;
}

bool CGCRef::GetGCFlag()
{
    // Return the gc flag
    return (refCount & 0x80000000) ? true : false;
}

int CGCRef::GetRefCount()
{
    // Return the reference count, without the gc flag
    return (refCount & 0x7FFFFFFF);
}

void CGCRef::EnumReferences(asIScriptEngine *engine)
{
    // Call the engine::GCEnumCallback for all references to other objects held
    engine->GCEnumCallback(myref);
}

void CGCRef::ReleaseAllReferences(asIScriptEngine *engine)
{
    // When we receive this call, we are as good as dead, but
    // the garbage collector will still hold a references to us, so we
    // cannot just delete ourself yet. Just free all references to other
    // objects that we hold
    if( myref )
    {
        myref->Release();
        myref = 0;
    }
}

// Register the GC support behaviours
r = engine->RegisterObjectBehaviour("ref_type", asBEHAVE_SETGCFLAG, "void f()", asMETHOD(CGCRef,SetGCFlag), asCALL_THISCALL); assert( r >= 0 );
r = engine->RegisterObjectBehaviour("ref_type", asBEHAVE_GETGCFLAG, "bool f()", asMETHOD(CGCRef,GetGCFlag), asCALL_THISCALL); assert( r >= 0 );
r = engine->RegisterObjectBehaviour("ref_type", asBEHAVE_GETREFCOUNT, "int f()", asMETHOD(CGCRef,GetRefCount), asCALL_THISCALL); assert( r >= 0 );
r = engine->RegisterObjectBehaviour("ref_type", asBEHAVE_ENUMREFS, "void f(int&in)", asMETHOD(CGCRef,EnumReferences), asCALL_THISCALL); assert( r >= 0 );
r = engine->RegisterObjectBehaviour("ref_type", asBEHAVE_RELEASEREFS, "void f(int&in)", asMETHOD(CGCRef,ReleaseAllReferences), asCALL_THISCALL); assert( r >= 0 );
\endcode

\section doc_reg_gcref_2 Factory for garbage collection

Whenever a garbage collected class is created, the garbage collector must
be notified of it's existence. The easiest way of doing that is to have the
factory behaviour, or the class constructor call the
<code>NotifyGarbageCollectorOfNewObject()</code> method on the engine when initializing the
class.

\code
CGCRef *GCRef_Factory()
{
    // Create the object and then notify the GC of its existence
    CGCRef *obj = new CGCRef();
    asITypeInfo *type = engine->GetTypeInfoByName("gc");
    engine->NotifyGarbageCollectorOfNewObject(obj, type);
    return obj;
}
\endcode

You may want to consider caching the typeId, so that it doesn't have to be
looked up through the relatively expensive call to GetTypeIdByDecl every time
an object of this type is created.

Note, if you create objects of this type from the application side, you
must also notify the garbage collector of its existence, so it's a good idea
to make sure all code use the same way of creating objects of this type.

\section doc_reg_gcref_3 Addref and release for garbage collection

For garbage collected objects it is important to make sure the AddRef and
Release behaviours clear the GC flag. Otherwise it is possible that the GC
incorrectly determine that the object should be destroyed.

\code
void CGCRef::AddRef()
{
    // Clear the gc flag and increase the reference counter
    refCount = (refCount&0x7FFFFFFF) + 1;
}

void CGCRef::Release()
{
    // Clear the gc flag, decrease ref count and delete if it reaches 0
    refCount &= 0x7FFFFFFF;
    if( --refCount == 0 )
        delete this;
}

// Registering the addref/release behaviours
r = engine->RegisterObjectBehaviour("ref_type", asBEHAVE_ADDREF, "void f()", asMETHOD(CGCRef,AddRef), asCALL_THISCALL); assert( r >= 0 );
r = engine->RegisterObjectBehaviour("ref_type", asBEHAVE_RELEASE, "void f()", asMETHOD(CGCRef,Release), asCALL_THISCALL); assert( r >= 0 );
\endcode

\section doc_reg_gcref_value GC behaviours for value types

Value types are normally not thought of as being part of circular references as they themselves cannot be referenced, however
if a value type can hold a reference to a type, and then that type can have the value type as a member then a circular reference can 
be established preventing the reference type from being released.

To solve these situations the value types can also be registered with some of the garbage collector behaviours.

\code
// Registering the value type with garbage collected behaviour
r = engine->RegisterObjectType("value_type", sizeof(value_type), asOBJ_VALUE | asOBJ_GC | ...); assert( r >= 0 );

// Register the garbage collector behaviours
r = engine->RegisterObjectBehaviour("ref", asBEHAVE_ENUMREFS, "void f(int&in)", asMETHOD(value_type,EnumReferences), asCALL_THISCALL); assert(r >= 0);
r = engine->RegisterObjectBehaviour("ref", asBEHAVE_RELEASEREFS, "void f(int&in)", asMETHOD(value_type, ReleaseReferences), asCALL_THISCALL); assert(r >= 0);
\endcode

Only the asBEHAVE_ENUMREFS and asBEHAVE_RELEASEREFS should be registered for value types. These work the same 
way as for reference types, i.e. the asBEHAVE_ENUMREFS should call the engine's GCEnumCallback for all 
references held, and asBEHAVE_RELEASEREFS should clear all references held.

Reference types that contain value types that have GC behaviours need to have the
asBEHAVE_ENUMREFS and asBEHAVE_RELEASEREFS behaviours adapted for this by forwarding the enum and release 
call to the value type. This forward is done by calling the engine's \ref asIScriptEngine::ForwardGCEnumReferences "ForwardGCEnumReferences"
or \ref asIScriptEngine::ForwardGCReleaseReferences "ForwardGCReleaseReferences" respectively.

\code
void CGCRef2::EnumReferences(asIScriptEngine *engine)
{
    // Forward the enum call to the member value type
    engine->ForwardGCEnumReferences(valueObj, valueType);
}

void CGCRef2::ReleaseAllReferences(asIScriptEngine *engine)
{
    // When we receive this call, we are as good as dead, but
    // the garbage collector will still hold a references to us, so we
    // cannot just delete ourself yet. Just free all references to other
    // objects that we hold

    // Forward the release call to the member value type
    engine->ForwardGCReleaseReferences(valueObj, valueType);
}
\endcode

\see The \ref doc_addon_handle for a value type with GC behaviours, and the \ref doc_addon_dict for a ref type that can contain value types with GC behaviours.




\section doc_reg_gcref_4 Garbage collected objects and multi-threading

If you plan on executing scripts from multiple threads with \ref doc_adv_custom_options_engine "automatic garbage collection" turned on, or 
if you plan on running the garbage collector manually from a background thread, then you must make sure that the object type behaviours
that support the garbage collector are thread-safe. Especially the ADDREF, RELEASE, and ENUMREFS behaviours have a high probability
of being invoked from multiple threads simultaneously. The RELEASEREFS behaviour will only be invoked when the Garbage Collector has 
determined that the object is already dead so it is guaranteed not to be invoked by multiple threads. The others, GETREFCOUNT, SETGCFLAG, 
and GETGCFLAG, are not sensitive as the garbage collector just use the information as a hint. 

Making the ADDREF and RELEASE behaviours thread-safe is easy with the use of \ref asAtomicInc and \ref asAtomicDec. If the object is 
static container, i.e. the memory layout of the contents cannot change, then ENUMREFS is already thread-safe, but if memory layout can
change, e.g. dynamic arrays or hash maps, then the iteration over the content in ENUMREFS must be protected so that it doesn't break
in case the memory happen to change in the middle of the iteration.

\see \ref doc_gc_threads



*/
