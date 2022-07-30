/**

\page doc_adv_weakref Weak references

Even though AngelScript uses a \ref doc_gc "garbage collector" to resolve cyclic references weak references
may still come in handy. Weak references are especifically useful in scenarios were an object wants to be 
able to access other objects, but do not want to kept them alive longer than necessary.

AngelScript supports weak references with the use of shared booleans. The code that wants to keep a weak
reference to an object should obtain the weakref flag from that object, which is a shared boolean, and before
using the pointer to the object it should check if this flag has been set to indicate that the object is no
longer alive.

Script classes automatically supports weak references without the script writing having to do anything. 
Application registered types on the other hand must register the behaviour \ref asBEHAVE_GET_WEAKREF_FLAG 
and implement the logic to set the flag upon destroying the object.

The following code shows how to do a thread safe implementation:

\code
class MyClass
{
public:
  MyClass() { refCount = 1; weakRefFlag = 0; }
  void AddRef() { asAtomicInc(refCount); }
  void Release() 
  {
    // If the weak ref flag exists it is because someone held a weak ref
    // and that someone may add a reference to the object at any time. It
    // is ok to check the existance of the weakRefFlag without locking here
    // because if the refCount is 1 then no other thread is currently 
    // creating the weakRefFlag.
    if( refCount == 1 && weakRefFlag )
    {
      // Set the flag to tell others that the object is no longer alive
      // We must do this before decreasing the refCount to 0 so we don't
      // end up with a race condition between this thread attempting to 
      // destroy the object and the other that temporary added a strong
      // ref from the weak ref.
      weakRefFlag->Set(true);
    }

    if( asAtomicDec(refCount) == 0 ) 
      delete this; 
  }
  asILockableSharedBool *GetWeakRefFlag()
  {
    if( !weakRefFlag )
    {
      // Lock globally so no other thread can attempt
      // to create a shared bool at the same time
      asAcquireExclusiveLock();

      // Make sure another thread didn't create the 
      // flag while we waited for the lock
      if( !weakRefFlag )
        weakRefFlag = asCreateLockableSharedBool();

      asReleaseExclusiveLock();
    }

    return weakRefFlag;
  }

  static MyClass *Factory() { return new MyClass(); }

protected:
  ~MyClass()
  {
    // Release the weak reference flag that may still
    // be accessed by the code that holds a weak reference
    if( weakRefFlag )
      weakRefFlag->Release();
  }

  int refCount;
  asILockableSharedBool *weakRefFlag;
};
\endcode

The \ref asBEHAVE_GET_WEAKREF_FLAG behaviour for this class is registered like this:

\code
engine->RegisterObjectType("MyClass", 0, asOBJ_REF);
engine->RegisterObjectBehaviour("MyClass", asBEHAVE_ADDREF, "void f()", asMETHOD(MyClass, AddRef), asCALL_THISCALL);
engine->RegisterObjectBehaviour("MyClass", asBEHAVE_RELEASE, "void f()", asMETHOD(MyClass, Release), asCALL_THISCALL);
engine->RegisterObjectBehaviour("MyClass", asBEHAVE_GET_WEAKREF_FLAG, "int &f()", asMETHOD(MyClass, GetWeakRefFlag), asCALL_THISCALL);
\endcode

The script language doesn't have a built-in syntax for weak references. Instead a standard 
\ref doc_addon_weakref "weakref add-on" has been implemented to provide this for the applications
that wants to provide this support in the scripts.





*/
