/**

\page doc_addon Add-ons

This page gives a brief description of the add-ons that you'll find in the /sdk/add_on/ folder.

 - \subpage doc_addon_application
 - \subpage doc_addon_script

\page doc_addon_application Application modules

 - \subpage doc_addon_build
 - \subpage doc_addon_ctxmgr
 - \subpage doc_addon_debugger
 - \subpage doc_addon_serializer
 - \subpage doc_addon_helpers
 - \subpage doc_addon_autowrap

\page doc_addon_script Script extensions

 - \subpage doc_addon_std_string
 - \subpage doc_addon_array
 - \subpage doc_addon_any
 - \subpage doc_addon_handle
 - \subpage doc_addon_weakref
 - \subpage doc_addon_dict
 - \subpage doc_addon_file
 - \subpage doc_addon_filesystem
 - \subpage doc_addon_math
 - \subpage doc_addon_grid
 - \subpage doc_addon_datetime
 - \subpage doc_addon_helpers_try



\page doc_addon_datetime datetime object

<b>Path:</b> /sdk/add_on/datetime/

The <code>CDateTime</code> class provides a way for scripts to get the system date and time.

Register the type with the <code>RegisterScriptDateTime(asIScriptEngine*)</code> function.

\note This class requires C++11 or later to compile.

\section doc_addon_datetime_1 Public C++ interface

\code
class CDateTime
{
public:
  // Constructors
  CDateTime();
  CDateTime(const CDateTime &other);
  CDateTime(asUINT year, asUINT month, asUINT day, asUINT hour, asUINT minute, asUINT second);

  // Copy the stored value from another any object
  CDateTime &operator=(const CDateTime &other);

  // Accessors
  asUINT getYear() const;
  asUINT getMonth() const;
  asUINT getDay() const;
  asUINT getHour() const;
  asUINT getMinute() const;
  asUINT getSecond() const;
  
  // Setters
  // Returns true if valid
  bool setDate(asUINT year, asUINT month, asUINT day);
  bool setTime(asUINT hour, asUINT minute, asUINT second);
  
  // Operators
  // Return difference in seconds
  asINT64          operator-(const CDateTime &other) const;
  CDateTime        operator+(asINT64 seconds) const;
  friend CDateTime operator+(asINT64 seconds, const CDateTime &other);
  CDateTime &      operator+=(asINT64 seconds);
  CDateTime        operator-(asINT64 seconds) const;
  friend CDateTime operator-(asINT64 seconds, const CDateTime &other);
  CDateTime &      operator-=(asINT64 seconds);
  bool             operator==(const CDateTime &other) const;
  bool             operator<(const CDateTime &other) const;
};
\endcode

\section doc_addon_datetime_2 Public script interface

\see \ref doc_script_stdlib_datetime "datetime in the script language"




\page doc_addon_serializer Serializer

<b>Path:</b> /sdk/add_on/serializer/

The <code>CSerializer</code> implements support for serializing the values of global variables in a 
module, for example in order to reload a slightly modified version of the script without reinitializing
everything. It will resolve primitives and script classes automatically, including references and handles.
For application registered types, the application needs to implement callback objects to show how
these should be serialized.

The implementation currently has some limitations:

 - It can only serialize to memory, i.e. it is not possible to save the values to a file.
 - If the variables changed type when restoring, the serializer cannot restore the value.
 - The serializer will attempt to backup all objects, but in some cases an application may
   not want to backup the actual object, but only a reference to it, e.g. an internal application
   object referenced by the script. Currently there is no way of telling the serializer to do 
   differently in this case.
 - If the module holds references to objects from another module it will probably fail in 
   restoring the values.


\todo Show how to serialize extra objects too. And explain about memory management for restored objects

\todo Explain that handles to registered types without factories will be kept as-is

\todo Registered pod-types do not need a special user type as the serializer will simply keep a bitwise copy

\section doc_addon_serializer_1 Public C++ interface

\code
class CSerializer
{
public:
  CSerializer();
  ~CSerializer();

  // Add implementation for serializing user types
  void AddUserType(CUserType *ref, const std::string &name);

  // Store all global variables in the module
  int Store(asIScriptModule *mod);

  // Restore all global variables after reloading script
  int Restore(asIScriptModule *mod);
  
  // Store extra objects that are not seen from the module's global variables
  void AddExtraObjectToStore(asIScriptObject *object);

  // Return new pointer to restored object
  void *GetPointerToRestoredObject(void *originalObject);
};
\endcode

\section doc_addon_serializer_2 Example usage

\code
struct CStringType;
struct CArrayType;

void RecompileModule(asIScriptEngine *engine, asIScriptModule *mod)
{
  string modName = mod->GetName();

  // Tell the serializer how the user types should be serialized
  // by adding the implementations of the CUserType interface
  CSerializer backup;
  backup.AddUserType(new CStringType(), "string");
  backup.AddUserType(new CArrayType(), "array");

  // Backup the values of the global variables
  backup.Store(mod);
  
  // Application can now recompile the module
  CompileModule(modName);

  // Restore the values of the global variables in the new module
  mod = engine->GetModule(modName.c_str(), asGM_ONLY_IF_EXISTS);
  backup.Restore(mod);
}

// This serializes the std::string type
struct CStringType : public CUserType
{
  void Store(CSerializedValue *val, void *ptr)
  {
    val->SetUserData(new std::string(*(std::string*)ptr));
  }
  void Restore(CSerializedValue *val, void *ptr)
  {
    std::string *buffer = (std::string*)val->GetUserData();
    *(std::string*)ptr = *buffer;
  }
  void CleanupUserData(CSerializedValue *val)
  {
    std::string *buffer = (std::string*)val->GetUserData();
    delete buffer;
  }
};

// This serializes the CScriptArray type
struct CArrayType : public CUserType
{
  void Store(CSerializedValue *val, void *ptr)
  {
    CScriptArray *arr = (CScriptArray*)ptr;

    for( unsigned int i = 0; i < arr->GetSize(); i++ )
      val->m_children.push_back(new CSerializedValue(val ,"", "", arr->At(i), arr->GetElementTypeId()));
  }
  void Restore(CSerializedValue *val, void *ptr)
  {
    CScriptArray *arr = (CScriptArray*)ptr;
    arr->Resize(asUINT(val->m_children.size()));

    for( size_t i = 0; i < val->m_children.size(); ++i )
      val->m_children[i]->Restore(arr->At(asUINT(i)), arr->GetElementTypeId());
  }
};
\endcode







\page doc_addon_debugger Debugger

<b>Path:</b> /sdk/add_on/debugger/

The <code>CDebugger</code> implements common debugging functionality for scripts, e.g.
setting breakpoints, stepping through the code, examining values of variables, etc.

To use the debugger the line callback should be set in the context. This will allow the 
debugger to take over whenever a breakpoint is reached, so the script can be debugged.

By default the debugger uses the standard in and standard out streams to interact with the
user, but this can be easily overloaded by deriving from the <code>CDebugger</code> class and implementing
the methods <code>TakeCommands</code> and <code>Output</code>. With this it is possible to implement a graphical 
interface, or even remote debugging for an application.

The application developer may also be interested in registering to-string callbacks for registered types
with calls to <code>RegisterToStringCallback</code>. Optionally the <code>ToString</code> method in the 
debugger can be overridden to implement custom to-string logic.

\see The sample \ref doc_samples_asrun for a complete example of how to use the debugger

\section doc_addon_debugger_1 Public C++ interface

\code
class CDebugger
{
public:
  CDebugger();
  virtual ~CDebugger();

  // Register callbacks to handle to-string conversions of application types
  // The expandMembersLevel is a counter for how many recursive levels the members should be expanded.
  // If the object that is being converted to a string has members of its own the callback should call
  // the debugger's ToString passing in expandMembersLevel - 1.
  typedef std::string (*ToStringCallback)(void *obj, int expandMembersLevel, CDebugger *dbg);
  virtual void RegisterToStringCallback(const asITypeInfo *type, ToStringCallback callback);
  
  // User interaction
  virtual void TakeCommands(asIScriptContext *ctx);
  virtual void Output(const std::string &str);

  // Line callback invoked by context
  virtual void LineCallback(asIScriptContext *ctx);

  // Commands
  virtual void PrintHelp();
  virtual void AddFileBreakPoint(const std::string &file, int lineNbr);
  virtual void AddFuncBreakPoint(const std::string &func);
  virtual void ListBreakPoints();
  virtual void ListLocalVariables(asIScriptContext *ctx);
  virtual void ListGlobalVariables(asIScriptContext *ctx);
  virtual void ListMemberProperties(asIScriptContext *ctx);
  virtual void ListStatistics(asIScriptContext *ctx);
  virtual void PrintCallstack(asIScriptContext *ctx);
  virtual void PrintValue(const std::string &expr, asIScriptContext *ctx);

  // Helpers
  virtual bool InterpretCommand(const std::string &cmd, asIScriptContext *ctx);
  virtual bool CheckBreakPoint(asIScriptContext *ctx);
  virtual std::string ToString(void *value, asUINT typeId, int expandMembersLevel, asIScriptEngine *engine);
  
  // Optionally set the engine pointer in the debugger so it can be retrieved
  // by callbacks that need it. This will hold a reference to the engine.
  virtual void SetEngine(asIScriptEngine *engine);
  virtual asIScriptEngine *GetEngine();
};
\endcode

\section doc_addon_debugger_2 Example usage

\code
CDebugger dbg;
int ExecuteWithDebug(asIScriptContext *ctx)
{
  // Tell the context to invoke the debugger's line callback
  ctx->SetLineCallback(asMETHOD(CDebugger, LineCallback), &dbg, asCALL_THISCALL);

  // Allow the user to initialize the debugging before moving on
  dbg.TakeCommands(ctx);

  // Execute the script normally. If a breakpoint is reached the 
  // debugger will take over the control loop.
  return ctx->Execute();
}
\endcode






\page doc_addon_ctxmgr Context manager

<b>Path:</b> /sdk/add_on/contextmgr/

The <code>CContextMgr</code> is a class designed to aid the management of multiple simultaneous 
scripts executing in parallel. It supports both \ref doc_adv_concurrent "concurrent script threads" 
and \ref doc_adv_coroutine "co-routines". 

If the application doesn't need multiple contexts, i.e. all scripts that are executed 
always complete before the next script is executed, then this class is not necessary.

The context manager uses \ref asIScriptEngine::RequestContext to take advantage of any context
callbacks registered with the engine, e.g. for debugging or pooling.

Multiple context managers can be used, for example when you have a group of scripts controlling 
in-game objects, and another group of scripts controlling GUI elements, then each of these groups
may be managed by different context managers.

Observe that the context manager class hasn't been designed for multi-threading, so you need to
be careful if your application needs to execute scripts from multiple threads.

\see The samples \ref doc_samples_concurrent and \ref doc_samples_corout for uses

\section doc_addon_ctxmgr_1 Public C++ interface

\code
class CContextMgr
{
public:
  CContextMgr();
  ~CContextMgr();

  // Set the function that the manager will use to obtain the time in milliseconds.
  void SetGetTimeCallback(TIMEFUNC_t func);

  // Registers the following:
  //
  //  void sleep(uint milliseconds)
  //
  // The application must set the get time callback for this to work
  void RegisterThreadSupport(asIScriptEngine *engine);

  // Registers the following:
  //
  //  funcdef void coroutine(dictionary@)
  //  void createCoRoutine(coroutine @func, dictionary @args)
  //  void yield()
  void RegisterCoRoutineSupport(asIScriptEngine *engine);

  // Create a new context, prepare it with the function, then return 
  // it so that the application can pass the argument values. The context
  // will be released by the manager after the execution has completed.
  // Set keepCtxAfterExecution to true if the application needs to retrieve
  // information from the context after it the script has finished. 
  asIScriptContext *AddContext(asIScriptEngine *engine, asIScriptContext *func, bool keepCtxAfterExecution = false);

  // If the context was kept after the execution, this method must be 
  // called when the application is done with the context so it can be
  // returned to the pool for reuse.
  void DoneWithContext(asIScriptContext *ctx);
  
  // Create a new context, prepare it with the function, then return
  // it so that the application can pass the argument values. The context
  // will be added as a co-routine in the same thread as the currCtx.
  asIScriptContext *AddContextForCoRoutine(asIScriptContext *currCtx, asIScriptContext *func);

  // Execute each script that is not currently sleeping. The function returns after 
  // each script has been executed once. The application should call this function
  // for each iteration of the message pump, or game loop, or whatever.
  // Returns the number of scripts still in execution.
  int ExecuteScripts();

  // Put a script to sleep for a while
  void SetSleeping(asIScriptContext *ctx, asUINT milliSeconds);

  // Switch the execution to the next co-routine in the group.
  // Returns true if the switch was successful.
  void NextCoRoutine();

  // Abort all scripts
  void AbortAll();
};
\endcode

\section doc_addon_ctxmgr_2 Public script interface

\see \ref doc_script_stdlib_coroutine





\page doc_addon_array array template object

<b>Path:</b> /sdk/add_on/scriptarray/

The <code>array</code> type is a \ref doc_adv_template "template object" that allow the scripts to declare arrays of any type.
Since it is a generic class it is not the most performatic due to the need to determine characteristics at 
runtime. For that reason it is recommended that the application registers a \ref doc_adv_template_2 "template specialization" for the
array types that are most commonly used.

The type is registered with <code>RegisterScriptArray(asIScriptEngine *engine, bool defaultArrayType)</code>. The second 
parameter should be set to true if you wish to allow the syntax form <code>type[]</code> to declare arrays.

Compile the add-on with the pre-processor define AS_USE_STLNAMES=1 to register the methods with the same names as used by C++ STL where 
the methods have the same significance. Not all methods from STL is implemented in the add-on, but many of the most frequent once are 
so a port from script to C++ and vice versa might be easier if STL names are used.

Compile the add-on with the pre-processor define AS_USE_ACCESSORS=1 to register length as a virtual property instead of the method length().

\section doc_addon_array_1 Public C++ interface

\code
class CScriptArray
{
public:
  // Set the memory functions that should be used by all CScriptArrays
  static void SetMemoryFunctions(asALLOCFUNC_t allocFunc, asFREEFUNC_t freeFunc);

  // Factory functions
  static CScriptArray *Create(asITypeInfo *arrayType);
  static CScriptArray *Create(asITypeInfo *arrayType, asUINT length);
  static CScriptArray *Create(asITypeInfo *arrayType, asUINT length, void *defaultValue);
  static CScriptArray *Create(asITypeInfo *arrayType, void *listBuffer);

  // Memory management
  void AddRef() const;
  void Release() const;

  // Type information
  asITypeInfo   *GetArrayObjectType() const;
  int            GetArrayTypeId() const;
  int            GetElementTypeId() const;

  // Get the current size
  asUINT GetSize() const;
  
  // Returns true if the array is empty
  bool IsEmpty() const;

  // Pre-allocates memory for elements
  void Reserve(asUINT numElements);
  
  // Resize the array
  void Resize(asUINT numElements);
  
  // Get a pointer to an element. Returns 0 if out of bounds
  void       *At(asUINT index);
  const void *At(asUINT index) const;

  // Set value of an element. 
  // The value arg should be a pointer to the value that will be copied to the element.
  // Remember, if the array holds handles the value parameter should be the 
  // address of the handle. The refCount of the object will also be incremented
  void  SetValue(asUINT index, void *value);

  // Copy the contents of one array to another (only if the types are the same)
  CScriptArray &operator=(const CScriptArray&);

  // Compare two arrays
  bool operator==(const CScriptArray &) const;

  // Array manipulation
  void InsertAt(asUINT index, void *value);
  void RemoveAt(asUINT index);
  void InsertLast(void *value);
  void RemoveLast();
  void SortAsc();
  void SortAsc(asUINT startAt, asUINT count);
  void SortDesc();
  void SortDesc(asUINT startAt, asUINT count);
  void Sort(asUINT startAt, asUINT count, bool asc);
  void Sort(asIScriptFunction *less, asUINT startAt, asUINT count);
  void Reverse();
  int  Find(void *value) const;
  int  Find(asUINT startAt, void *value) const;
  int  FindByRef(void *ref) const;
  int  FindByRef(asUINT startAt, void *ref) const;
  
  // Returns the address of the inner buffer for direct manipulation
  void *GetBuffer();
};
\endcode

\section doc_addon_array_2 Public script interface

\see \ref doc_datatypes_arrays "Arrays in the script language"

\section doc_addon_array_4 C++ example

This function shows how a script array can be instantiated 
from the application and then passed to the script.

\code
// Registered with AngelScript as 'array<string> @CreateArrayOfString()'
CScriptArray *CreateArrayOfStrings()
{
  // If called from the script, there will always be an active 
  // context, which can be used to obtain a pointer to the engine.
  asIScriptContext *ctx = asGetActiveContext();
  if( ctx )
  {
    asIScriptEngine* engine = ctx->GetEngine();

    // The script array needs to know its type to properly handle the elements.
    // Note that the object type should be cached to avoid performance issues
    // if the function is called frequently.
    asITypeInfo* t = engine->GetTypeInfoByDecl("array<string>");

    // Create an array with the initial size of 3 elements
    CScriptArray* arr = CScriptArray::Create(t, 3);
    for( asUINT i = 0; i < arr->GetSize(); i++ )
    {
      // Set the value of each element
      string val("test");
      arr->SetValue(i, &val);
    }

    // The ref count for the returned handle was already set in the array's constructor
    return arr;
  }
  return 0;
}
\endcode












\page doc_addon_grid grid template object

<b>Path:</b> /sdk/add_on/scriptgrid/

The <code>grid</code> type is a \ref doc_adv_template "template object" that allow the scripts to declare 2D grids of any type.
In many ways it is similar to the \ref doc_addon_array, but it is specialized for use with areas.

The type is registered with <code>RegisterScriptGrid(asIScriptEngine *engine)</code>. 

\section doc_addon_grid_1 Public C++ interface

\code
class CScriptGrid
{
public:
  // Set the memory functions that should be used by all CScriptGrids
  static void SetMemoryFunctions(asALLOCFUNC_t allocFunc, asFREEFUNC_t freeFunc);

  // Factory functions
  static CScriptGrid *Create(asITypeInfo *gridType);
  static CScriptGrid *Create(asITypeInfo *gridType, asUINT width, asUINT height);
  static CScriptGrid *Create(asITypeInfo *gridType, asUINT width, asUINT height, void *defaultValue);
  static CScriptGrid *Create(asITypeInfo *gridType, void *listBuffer);

  // Memory management
  void AddRef() const;
  void Release() const;

  // Type information
  asITypeInfo   *GetGridObjectType() const;
  int            GetGridTypeId() const;
  int            GetElementTypeId() const;

  // Size
  asUINT GetWidth() const;
  asUINT GetHeight() const;
  void   Resize(asUINT width, asUINT height);
  
  // Get a pointer to an element. Returns 0 if out of bounds
  void       *At(asUINT x, asUINT y);
  const void *At(asUINT x, asUINT y) const;

  // Set value of an element. 
  // The value arg should be a pointer to the value that will be copied to the element.
  // Remember, if the grid holds handles the value parameter should be the 
  // address of the handle. The refCount of the object will also be incremented
  void  SetValue(asUINT x, asUINT y, void *value);
};
\endcode

\section doc_addon_grid_2 Public script interface

<pre>
  class grid<T>
  {
    grid();
    grid(uint width, uint height);
    grid(uint width, uint height, const T &in fillValue);
  
    uint width() const;
    uint height() const;
    void resize(uint w, uint h);
    
    T &opIndex(uint x, uint y);
    const T &opIndex(uint x, uint y) const;
  }
</pre>

<b>grid()</b><br>
<b>grid(uint width, uint height)</b><br>
<b>grid(uint width, height, const T &in fillValue)</b><br>

The constructors initializes the grid object. The default constructor will create an zero sized grid.

<b>uint width() const</b><br>
<b>uint height() const</b><br>

Returns the width and height of the grid.

<b>void resize(uint w, uint h)</b>

Resizes the grid to the new dimension. The elements that still fit in the grid will keep their values.

<b>T &opIndex(uint x, uint y)</b><br>
<b>const T &opIndex(uint x, uint y) const</b><br>

The index operator returns a reference to one of the elements. If the index is out of bounds a script
exception will be raised.



\section doc_addon_grid_3 Example usage in script

<pre>
  // Initialize a 5x5 map
  grid<int> map = {{1,0,1,1,1},
                   {0,0,1,0,0},
                   {0,1,1,0,1},
                   {0,1,1,0,1},
                   {0,0,0,0,1}};
   
  // A function to verify if the next area is walkable
  bool canWalk(uint x, uint y)
  {
    // If the map in the destination is 
    // clear, it is possible to wark there
    return map[x,y] == 0;
  }
</pre>























\page doc_addon_any any object

<b>Path:</b> /sdk/add_on/scriptany/

The <code>any</code> type is a generic container that can hold any value. It is a reference type.

The type is registered with <code>RegisterScriptAny(asIScriptEngine*)</code>.

\section doc_addon_any_1 Public C++ interface

\code
class CScriptAny
{
public:
  // Constructors
  CScriptAny(asIScriptEngine *engine);
  CScriptAny(void *ref, int refTypeId, asIScriptEngine *engine);

  // Memory management
  int AddRef() const;
  int Release() const;

  // Copy the stored value from another any object
  CScriptAny &operator=(const CScriptAny&);
  int CopyFrom(const CScriptAny *other);

  // Store the value, either as variable type, integer number, or real number
  void Store(void *ref, int refTypeId);
  void Store(asINT64 &value);
  void Store(double &value);

  // Retrieve the stored value, either as variable type, integer number, or real number
  bool Retrieve(void *ref, int refTypeId) const;
  bool Retrieve(asINT64 &value) const;
  bool Retrieve(double &value) const;

  // Get the type id of the stored value
  int GetTypeId() const;
};
\endcode

\section doc_addon_any_2 Public script interface

<pre>
  class any
  {
    any();
    any(? &in value);
    any(int64 &in value);
    any(double &in value);

    any &opAssign(const any &in other);
  
    void store(? &in value);
    void store(int64 &in value);
    void store(double &in value);
  
    bool retrieve(? &out value) const;
    bool retrieve(int64 &out value) const;
    bool retrieve(double &out value) const;
  }
</pre>

<b>any()</b><br>
<b>any(? &in value)</b><br>
<b>any(int64 &in value)</b><br>
<b>any(double &in value)</b><br>

The default constructor creates an empty object, and the second initializes the object with the provided value.

The int64 and double overloads make sure that all numbers are converted to 64bit before being stored in the object.

<b>any &opAssign(const any &in other)</b><br>

The assignment operator will copy the contained value from the other object.

<b>void store(? &in value)</b><br>
<b>void store(int64 &in value)</b><br>
<b>void store(double &in value)</b><br>

These methods sets the value in the object.

The int64 and double overloads make sure that all numbers are converted to 64bit before being stored in the object.

<b>bool retrieve(? &out value) const</b><br>
<b>bool retrieve(int64 &out value) const</b><br>
<b>bool retrieve(double &out value) const</b><br>

These methods retrieve the value stored in the object. The methods will return true if the stored value is 
compatible with the requested type.


\section doc_addon_any_3 Example usage

In the scripts it can be used as follows:

<pre>
  int value;
  obj object;
  obj \@handle;
  any a,b,c;
  a.store(value);      // store the value
  b.store(\@handle);    // store an object handle
  c.store(object);     // store a copy of the object
  
  a.retrieve(value);   // retrieve the value
  b.retrieve(\@handle); // retrieve the object handle
  c.retrieve(object);  // retrieve a copy of the object
</pre>

In C++ the type can be used as follows:

\code
CScriptAny *myAny;
int typeId = engine->GetTypeIdByDecl("string@");
CScriptString *str = new CScriptString("hello world");
myAny->Store((void*)&str, typeId);
myAny->Retrieve((void*)&str, typeId);
\endcode










\page doc_addon_handle ref object

<b>Path:</b> /sdk/add_on/scripthandle/

The <code>ref</code> type is a generic container that can hold any handle. 
It is a value type, but behaves very much like an object handle.

The type is registered with <code>RegisterScriptHandle(asIScriptEngine*)</code>.

\see \ref doc_adv_generic_handle

\section doc_addon_handle_1 Public C++ interface

\code
class CScriptHandle 
{
public:
  // Constructors
  CScriptHandle();
  CScriptHandle(const CScriptHandle &other);
  CScriptHandle(void *ref, int typeId);
  ~CScriptHandle();

  // Copy the stored reference from another handle object
  CScriptHandle &operator=(const CScriptHandle &other);
  
  // Set the reference
  void Set(void *ref, asITypeInfo *type);

  // Compare equalness
  bool operator==(const CScriptHandle &o) const;
  bool operator!=(const CScriptHandle &o) const;
  bool opEquals(void *ref, int typeId) const;

  // Dynamic cast to desired handle type
  void Cast(void **outRef, int typeId);

  // Returns the type of the reference held
  asITypeInfo   *GetType() const;
  int            GetTypeId() const;
  
  // Returns the reference
  void *GetRef();
};
\endcode

\section doc_addon_handle_3 Public script interface

\see \ref doc_datatypes_ref "ref in the script language"

\section doc_addon_handle_4 Example usage from C++

Even though the CScriptHandle is a value type, when registering properties 
of its type they should be registered as handles. The same goes for function 
arguments and return types.

\code
CScriptHandle g_handle;

void Function(CScriptHandle handle)
{
  ... use the methods of CScriptHandle to determine the true object held in it
}

void Register(asIScriptEngine *engine)
{
  int r;
  r = engine->RegisterGlobalProperty("ref @g_handle", &g_handle); assert( r >= 0 );
  r = engine->RegisterGlobalFunction("void Function(ref @)", asFUNCTION(Function), asCALL_CDECL); assert( r >= 0 );
}
\endcode

To set an object pointer in the handle from the application, you'll use the 
Set() method passing a pointer to the object and the type of the object.

To retrieve an object pointer from the application you'll use the Cast() method
passing in a pointer to the pointer and the wanted type id. If the type id given
doesn't match the stored handle the returned pointer will be null.

To retrieve an object of an unknown type use the GetType() or GetTypeId() to
determine the type stored in the handle, then use the Cast() method.










\page doc_addon_weakref weakref object

<b>Path:</b> /sdk/add_on/weakref/

The <code>weakref</code> type is a template type for holding weak references to 
objects, i.e. the references that will not keep the referred object alive.

The type is registered with <code>RegisterScriptWeakRef(asIScriptEngine*)</code>.

\see \ref doc_adv_weakref

\section doc_addon_weakref_1 Public C++ interface

\code
class CScriptWeakRef 
{
public:
  // Constructors
  CScriptWeakRef(asITypeInfo *type);
  CScriptWeakRef(const CScriptWeakRef &other);
  CScriptWeakRef(void *ref, asITypeInfo *type);

  ~CScriptWeakRef();

  // Copy the stored value from another weakref object
  CScriptWeakRef &operator=(const CScriptWeakRef &other);

  // Compare equalness
  bool operator==(const CScriptWeakRef &o) const;
  bool operator!=(const CScriptWeakRef &o) const;

  // Sets a new reference
  CScriptWeakRef &Set(void *newRef);

  // Returns the object if it is still alive
  // This will increment the refCount of the returned object
  void *Get() const;

  // Returns true if the contained reference is the same
  bool Equals(void *ref) const;
  
  // Returns the type of the reference held
  asITypeInfo *GetRefType() const;
};
\endcode

\section doc_addon_weakref_2 Public script interface

\see \ref doc_datatypes_weakref "weakref in the script language"



  










\page doc_addon_std_string string object

<b>Path:</b> /sdk/add_on/scriptstdstring/

This add-on registers the <code>std::string</code> type as-is with AngelScript. This gives
perfect compatibility with C++ functions that use <code>std::string</code> in parameters or
as return type.

A potential drawback is that the <code>std::string</code> type is a value type, thus may 
increase the number of copies taken when string values are being passed around
in the script code. However, this is most likely only a problem for scripts 
that perform a lot of string operations.

Register the type with <code>RegisterStdString(asIScriptEngine*)</code>. Register the optional
split method and global join function with <code>RegisterStdStringUtils(asIScriptEngine*)</code>. 
The optional functions require that the \ref doc_addon_array has been registered first.

Compile the add-on with the pre-processor define AS_USE_STLNAMES=1 to register the methods with the same names as used by C++ STL where 
the methods have the same significance. Not all methods from STL is implemented in the add-on, but many of the most frequent ones are 
so a port from script to C++ and vice versa might be easier if STL names are used.

Compile the add-on with the pre-processor define AS_USE_ACCESSORS=1 to register length as a virtual property instead of the method length().

\section doc_addon_std_string_1 Public C++ interface

Refer to the <code>std::string</code> implementation for your compiler.

\section doc_addon_std_string_2 Public script interface

\see \ref doc_script_stdlib_string "Strings in the script language"







\page doc_addon_dict dictionary object 

<b>Path:</b> /sdk/add_on/scriptdictionary/

The dictionary object maps string values to values or objects of other types. 

Register with <code>RegisterScriptDictionary(asIScriptEngine*)</code>.

Compile the add-on with the pre-processor define AS_USE_STLNAMES=1 to register the methods with the same names as used by C++ STL where 
the methods have the same significance. Not all methods from STL is implemented in the add-on, but many of the most frequent once are 
so a port from script to C++ and vice versa might be easier if STL names are used.

\section doc_addon_dict_1 Public C++ interface

\code
typedef std::string dictKey;

class CScriptDictionary
{
public:
  // Factory functions
  static CScriptDictionary *Create(asIScriptEngine *engine);

  // Reference counting
  void AddRef() const;
  void Release() const;

  // Perform a shallow copy of the other dictionary
  CScriptDictionary &operator=(const CScriptDictionary &other);

  // Sets a key/value pair
  void Set(const dictKey &key, void *value, int typeId);
  void Set(const dictKey &key, asINT64 &value);
  void Set(const dictKey &key, double &value);

  // Gets the stored value. Returns false if the value isn't compatible with informed type  
  bool Get(const dictKey &key, void *value, int typeId) const;
  bool Get(const dictKey &key, asINT64 &value) const;
  bool Get(const dictKey &key, double &value) const;

  // Index accessors. If the dictionary is not const it inserts the value if it doesn't already exist
  // If the dictionary is const then a script exception is set if it doesn't exist and a null pointer is returned
  CScriptDictValue *operator[](const dictKey &key);
  const CScriptDictValue *operator[](const dictKey &key) const;
  
  // Returns the type id of the stored value, or negative if it doesn't exist
  int  GetTypeId(const dictKey &key) const;

  // Returns true if the key is set
  bool Exists(const dictKey &key) const;
  
  // Returns true if the dictionary is empty
  bool IsEmpty() const;
  
  // Returns the number of keys in the dictionary
  asUINT GetSize() const;
  
  // Deletes the key
  bool Delete(const dictKey &key);
  
  // Deletes all keys
  void DeleteAll();

  // Get an array of all keys
  CScriptArray *GetKeys() const;

  // STL style iterator
  class CIterator
  {
  public:
    void operator++();    // Pre-increment
    void operator++(int); // Post-increment

    bool operator==(const CIterator &other) const;
    bool operator!=(const CIterator &other) const;

    // Accessors
    const dictKey &GetKey() const;
    int            GetTypeId() const;
    bool           GetValue(asINT64 &value) const;
    bool           GetValue(double &value) const;
    bool           GetValue(void *value, int typeId) const;
    const void *   GetAddressOfValue() const;
  };
  
  CIterator begin() const;
  CIterator end() const;
  CIterator find(const dictKey &key) const;
};
\endcode

\section doc_addon_dict_2 Public script interface

\see \ref doc_datatypes_dictionary "Dictionaries in the script language"

\section doc_addon_dict_3 Example usage from C++

Here's a skeleton for iterating over the entries in the dictionary. For brevity the code doesn't show how to interpret the values, 
for more information on that see \ref asETypeIdFlags and \ref asIScriptEngine::GetTypeInfoById.

\code
void iterateDictionary(CScriptDictionary *dict)
{
	// Iterate over each entry
	for (auto it : *dict)
	{
		// Determine the name of the key
		std::string keyName = it.GetKey();
		cout << "\"" << keyName << "\"" << " = ";
		
		// Get the type and address of the value
		int typeId = it.GetTypeId();
		const void *addressOfValue = it.GetAddressOfValue();

		// Cast the value to the correct C++ type according to the typeId and then print it
		...
	}
}
\endcode






\page doc_addon_file file object 

<b>Path:</b> /sdk/add_on/scriptfile/

This object provides support for reading and writing files.

Register with <code>RegisterScriptFile(asIScriptEngine*)</code>.

If you do not want to provide write access for scripts then you can compile 
the add on with the define AS_WRITE_OPS 0, which will disable support for writing. 
This define can be made in the project settings or directly in the header.

\section doc_addon_file_1 Public C++ interface

\code
class CScriptFile
{
public:
  // Constructor
  CScriptFile();

  // Memory management
  void AddRef() const;
  void Release() const;

  // Opening and closing file handles
  // mode = "r" -> open the file for reading
  // mode = "w" -> open the file for writing (overwrites existing files)
  // mode = "a" -> open the file for appending
  int Open(const std::string &filename, const std::string &mode);
  int Close();
  
  // Returns the size of the file
  int GetSize() const;
  
  // Returns true if the end of the file has been reached
  bool IsEOF() const;

  // Reads a specified number of bytes into the string
  std::string ReadString(unsigned int length);
  
  // Reads to the next new-line character
  std::string ReadLine();

  // Reads a signed integer
  asINT64     ReadInt(asUINT bytes);

  // Reads an unsigned integer
  asQWORD     ReadUInt(asUINT bytes);

  // Reads a float
  float       ReadFloat();

  // Reads a double
  double      ReadDouble();
    
  // Writes a string to the file
  int WriteString(const std::string &str);
  
  int WriteInt(asINT64 v, asUINT bytes);
  int WriteUInt(asQWORD v, asUINT bytes);
  int WriteFloat(float v);
  int WriteDouble(double v);

  // File cursor manipulation
  int GetPos() const;
  int SetPos(int pos);
  int MovePos(int delta);

  // Determines the byte order of the binary values (default: false)
  // Big-endian = most significant byte first
  bool mostSignificantByteFirst;
};
\endcode

\section doc_addon_file_2 Public script interface

\see \ref doc_script_stdlib_file





\page doc_addon_filesystem filesystem object 

<b>Path:</b> /sdk/add_on/scriptfile/

This object provides support for inspecting directories on the filesystem.

Register with <code>RegisterScriptFileSystem(asIScriptEngine*)</code>.

\section doc_addon_filesystem_1 Public C++ interface

\code
class CScriptFileSystem
{
public:
  CScriptFileSystem();

  void AddRef() const;
  void Release() const;

  // Sets the current path that should be used in other calls when using relative paths
  // It can use relative paths too, so moving up a directory is used by passing in ".."
  bool ChangeCurrentPath(const std::string &path);
  std::string GetCurrentPath() const;

  // Returns true if the path is a directory. Input can be either a full path or a relative path.
  // This method does not return the dirs '.' and '..'
  bool IsDir(const std::string &path) const;

  // Returns true if the path is a link. Input can be either a full path or a relative path
  bool IsLink(const std::string &path) const;

  // Returns the size of file. Input can be either a full path or a relative path
  asINT64 GetSize(const std::string &path) const;
  
  // Returns a list of the files in the current path
  CScriptArray *GetFiles() const;

  // Returns a list of the directories in the current path
  CScriptArray *GetDirs() const;
  
  // Creates a new directory. Returns 0 on success
  int MakeDir(const std::string &path);

  // Removes a directory. Will only remove the directory if it is empty. Returns 0 on success
  int RemoveDir(const std::string &path);

  // Deletes a file. Returns 0 on success
  int DeleteFile(const std::string &path);

  // Copies a file. Returns 0 on success
  int CopyFile(const std::string &source, const std::string &target);

  // Moves or renames a file or directory. Returns 0 on success
  int Move(const std::string &source, const std::string &target);
  
  // Gets the date and time of the file/dir creation
  CDateTime GetCreateDateTime(const std::string &path) const;
  
  // Gets the date and time of the file/dir modification
  CDateTime GetModifyDateTime(const std::string &path) const; 
};
\endcode

\section doc_addon_filesystem_2 Public script interface

\see \ref doc_script_stdlib_filesystem





\page doc_addon_math math functions

<b>Path:</b> /sdk/add_on/scriptmath/

This add-on registers the math functions from the standard C runtime library with the script 
engine. Use <code>RegisterScriptMath(asIScriptEngine*)</code> to perform the registration.

By defining the preprocessor word AS_USE_FLOAT=0, the functions will be registered to take 
and return doubles instead of floats.

The function <code>RegisterScriptMathComplex(asIScriptEngine*)</code> registers a type that 
represents a complex number, i.e. a number with real and imaginary parts.

\section doc_addon_math_1 Public script interface

<pre>
  // Trigonometric functions
  float cos(float rad);
  float sin(float rad);
  float tan(float rad);
  
  // Inverse trigonometric functions
  float acos(float val);
  float asin(float val);
  float atan(float val);
  float atan2(float y, float x);
  
  // Hyperbolic functions
  float cosh(float rad);
  float sinh(float rad);
  float tanh(float rad);
  
  // Logarithmic functions
  float log(float val);
  float log10(float val);
  
  // Power to
  float pow(float val, float exp);
  
  // Square root
  float sqrt(float val);

  // Absolute value
  float abs(float val);

  // Ceil and floor functions
  float ceil(float val);
  float floor(float val);
  
  // Returns the fraction
  float fraction(float val);

  // Approximate float comparison, to deal with numeric imprecision
  bool closeTo(float a, float b, float epsilon = 0.00001f);
  bool closeTo(double a, double b, double epsilon = 0.0000000001);
  
  // Conversion between floating point and IEEE 754 representations
  float  fpFromIEEE(uint raw); 
  double fpFromIEEE(uint64 raw);
  uint   fpToIEEE(float fp);
  uint64 fpToIEEE(double fp);
</pre>

\subsection doc_addon_math_funcs Functions

<b>cos, sin, tan</b>

Calculates the trigonometric functions cosine, sine, and tangent. The input angle should be given in radian.

<b>acos, asin, atan</b>

Calculates the inverse of the trigonometric functions cosine, sine, and tangent. The returned angle is given in radian.

<b>atan2</b>

Calculates the inverse of the trigonometric function tangent. The input is the y and x proportions. The returned angle is given in radian.

<b>cosh, sinh, tanh</b>

Calculates the hyperbolic of the cosine, sine, and tangent. The input angle should be given in radian.

<b>log, log10</b>

Calculates the logarithm of the input value. log is the natural logarithm and log10 is the base-10 logarithm.

<b>pow</b>

Calculates the based raised to the power of exponent.

<b>sqrt</b>

Calculates the square root of the value.

<b>abs</b>

Returns the absolute value.

<b>ceil, floor</b>

ceil returns the closest integer number that is higher or equal to the input. Floor returns the closest integer number that is lower or equal to the input.

<b>fraction</b>

Returns the fraction of the number, i.e. what remains after taking away the integral number.

<b>closeTo</b>

Due to numerical errors with the binary representation of real numbers it is often difficult to do direct comparisons of two float values. The closeTo function will return true of the two values are almost equal, allowing for a small difference up to the size of the epsilon value.

<b>fpFromIEEE, fpToIEEE</b>

Translates the float to and from IEEE 754 representation. This can be used if one wishes to directly inspect or manipulate the floating point value in the binary representation.

\subsection doc_addon_math_complex The complex type

<pre>
  // This type represents a complex number with real and imaginary parts
  class complex
  {
    // Constructors
    complex();
    complex(const complex &in);
    complex(float r);
    complex(float r, float i);

    // Equality operator
    bool opEquals(const complex &in) const;

    // Compound assignment operators
    complex &opAddAssign(const complex &in);
    complex &opSubAssign(const complex &in);
    complex &opMulAssign(const complex &in);
    complex &opDivAssign(const complex &in);
    
    // Math operators
    complex opAdd(const complex &in) const;
    complex opSub(const complex &in) const;
    complex opMul(const complex &in) const;
    complex opDiv(const complex &in) const;
    
    // Returns the absolute value (magnitude)
    float abs() const;

    // Swizzle operators
    complex get_ri() const;
    void set_ri(const complex &in);
    complex get_ir() const;
    void set_ir(const complex &in);
    
    // The real and imaginary parts
    float r;
    float i;
  }
</pre>

<b>complex</b>

The constructors allow for implicit construction, making a copy, implicit conversion from float, and explicit initialization from two float values.

<b>=, !=</b>

Compares two complex values.

<b>=, +=, -=, *=, /=</b>

Assign and compound assignment of complex values.

<b>+, -, *, /</b>

Math operators on complex values.

<b>abs</b>

Returns the absolute (magnitude) of the complex value.

<b>r, i</b>

Retrieves the real and imaginary part of the complex value.

<b>ri, ir</b>

Swizzle operators return a complex value with the ordering of the real and imaginary parts as indicated by the name.






\page doc_addon_build Script builder

<b>Path:</b> /sdk/add_on/scriptbuilder/

This class is a helper class for loading and building scripts, with a basic pre-processor 
that supports conditional compilation, include directives, pragma directives, and metadata declarations.

By default the script builder resolves include directives by loading the included file 
from the relative directory of the file it is included from. If you want to do this in another
way, then you should implement the \ref doc_addon_build_1_1 "include callback" which will
let you process the include directive in a custom way, e.g. to load the included file from 
memory, or to support multiple search paths. The include callback should call the AddSectionFromFile or
AddSectionFromMemory to include the section in the current build.

If the application should to support pragma directives, it must register the \ref doc_addon_build_1_2 "pragma callback"
to process the text provided for the pragma directive. Without the callback the script builder will give an error 
when encountering a pragma directive.

The script builder will also remove any lines that start with <tt>#!</tt> as comments. This is done 
to support shebang interpreter directives often used on Linux and UNIX based operative systems.

If you do not want process metadata then you can compile the add-on with the define 
AS_PROCESS_METADATA 0, which will exclude the code for processing this. This define
can be made in the project settings or directly in the header.


\section doc_addon_build_1 Public C++ interface

\code
class CScriptBuilder
{
public:
  // Start a new module
  int StartNewModule(asIScriptEngine *engine, const char *moduleName);

  // Load a script section from a file on disk
  // Returns  1 if the file was included
  //          0 if the file had already been included before
  //         <0 on error
  int AddSectionFromFile(const char *filename);

  // Load a script section from memory
  // Returns  1 if the section was included
  //          0 if a section with the same name had already been included before
  //         <0 on error
  int AddSectionFromMemory(const char *sectionName,
                           const char *scriptCode, 
                           unsigned int scriptLength = 0,
                           int lineOffset = 0);

  // Build the added script sections
  int BuildModule();

  // Returns the script engine
  asIScriptEngine *GetEngine();

  // Returns the current module
  asIScriptModule *GetModule();

  // Register the callback for resolving include directive
  void SetIncludeCallback(INCLUDECALLBACK_t callback, void *userParam);

  // Register the callback for resolving pragma directive
  void SetPragmaCallback(PRAGMACALLBACK_t callback, void *userParam);

  // Add a pre-processor define for conditional compilation
  void DefineWord(const char *word);

  // Enumerate included script sections
  unsigned int GetSectionCount() const;
  string       GetSectionName(unsigned int idx) const;
  
  // Get metadata declared for classes, interfaces, and enums
  // Each metadata block, i.e. [...], is returned as a separate string
  std::vector<std::string> GetMetadataStringForType(int typeId);

  // Get metadata declared for functions
  // Each metadata block, i.e. [...], is returned as a separate string
  std::vector<std::string> GetMetadataStringForFunc(asIScriptFunction *func);

  // Get metadata declared for global variables
  // Each metadata block, i.e. [...], is returned as a separate string
  std::vector<std::string> GetMetadataStringForVar(int varIdx);

  // Get metadata declared for a class method
  // Each metadata block, i.e. [...], is returned as a separate string
  // Use the asIScriptFunction for the virtual method for lookup
  std::vector<std::string> GetMetadataStringForTypeMethod(int typeId, asIScriptFunction *method);

  // Get metadata declared for a class property
  // Each metadata block, i.e. [...], is returned as a separate string
  std::vector<std::string> GetMetadataStringForTypeProperty(int typeId, int varIdx);
};
\endcode

\subsection doc_addon_build_1_1 The include callback signature

\code
// This callback will be called for each #include directive encountered by the
// builder. The callback should call the AddSectionFromFile or AddSectionFromMemory
// to add the included section to the script. If the include cannot be resolved
// then the function should return a negative value to abort the compilation.
typedef int (*INCLUDECALLBACK_t)(const char *include, const char *from, CScriptBuilder *builder, void *userParam);
\endcode

\subsection doc_addon_build_1_2 The pragma callback signature

\code
// This callback will be called for each #pragma directive encountered by the builder.
// The application can interpret the pragmaText and decide what do to based on that.
// If the callback returns a negative value the builder will report an error and abort the compilation.
typedef int(*PRAGMACALLBACK_t)(const std::string &pragmaText, CScriptBuilder &builder, void *userParam);
\endcode


\section doc_addon_build_2 Include directives

Example script with include directive:

<pre>
  \#include "commonfuncs.as"
  
  void main()
  {
    // Call a function from the included file
    CommonFunc();
  }
</pre>





\section doc_addon_build_condition Conditional programming

The builder supports conditional programming through the \#if/\#endif preprocessor directives.
The application may define a word with a call to DefineWord(), then the scripts may check
for this definition in the code in order to include/exclude parts of the code.

This is especially useful when scripts are shared between different binaries, for example, in a 
client/server application.

Example script with conditional compilation:

<pre>
  class CObject
  {
    void Process()
    {
  \#if SERVER
      // Do some server specific processing
  \#endif

  \#if CLIENT
      // Do some client specific processing
  \#endif 

      // Do some common processing
    }
  }
</pre>





\section doc_addon_build_metadata Metadata in scripts

Metadata can be added before script class, interface, function, and global variable 
declarations. The metadata is removed from the script by the builder class and stored
for post build lookup by the type id, function id, or variable index.

Exactly what the metadata looks like is up to the application. The builder class doesn't
impose any rules, except that the metadata should be added between brackets []. After 
the script has been built the application can obtain the metadata strings and interpret
them as it sees fit. Multiple metadata blocks, i.e. [], can be defined for each entity. 

Example script with metadata:

<pre>
  [factory func = CreateOgre]
  class COgre
  {
    [editable] 
    vector3 myPosition;
    
    [editable]
    [range [10, 100]]
    int     myStrength;
  }
  
  [factory]
  COgre \@CreateOgre()
  {
    return \@COgre();
  }
</pre>

Example usage:

\code
CScriptBuilder builder;
int r = builder.StartNewModule(engine, "my module");
if( r >= 0 )
  r = builder.AddSectionFromMemory(script);
if( r >= 0 )
  r = builder.BuildModule();
if( r >= 0 )
{
  // Find global variables that have been marked as editable by user
  asIScriptModule *mod = engine->GetModule("my module");
  int count = mod->GetGlobalVarCount();
  for( int n = 0; n < count; n++ )
  {
    vector<string> metadata = builder.GetMetadataStringForVar(n);
    for( int m = 0; m < metadata.size(); m++ )
    {
      if( metadata[m] == "editable" )
      {
        // Show the global variable in a GUI
        ...
      }
    }
  }
}
\endcode




\page doc_addon_autowrap Automatic wrapper functions

<b>Path:</b> /sdk/add_on/autowrapper/aswrappedcall.h

This header file declares some macros and template functions that will let the application developer
automatically generate wrapper functions using the \ref doc_generic "generic calling convention" with 
a simple call to a preprocessor macro. This is useful for those platforms where the native calling 
conventions are not yet supported.

The macros are defined as:

\code
// Wrap a global function with implicit or explicit signature
#define WRAP_FN(name)
#define WRAP_FN_PR(name, Parameters, ReturnType)

// Wrap a class method with implicit or explicit signature
#define WRAP_MFN(ClassType, name)
#define WRAP_MFN_PR(ClassType, name, Parameters, ReturnType)

// Wrap a global function that will emulate a class method and receives the 'this' pointer as the first argument
#define WRAP_OBJ_FIRST(name)
#define WRAP_OBJ_FIRST_PR(name, Parameters, ReturnType)

// Wrap a global function that will emulate a class method and receives the 'this' pointer as the last argument
#define WRAP_OBJ_LAST(name)
#define WRAP_OBJ_LAST_PR(name, Parameters, ReturnType)

// Wrap a constructor with an explicit signature
#define WRAP_CON(ClassType, Parameters)

// Wrap a destructor
#define WRAP_DES(ClassType)
\endcode

As you can see they are very similar to the \ref doc_register_func_1 "asFUNCTION" and 
\ref doc_register_func_1 "asMETHOD" macros,  and are used the same way.

Unfortunately the template functions needed to perform this generation are quite complex and older
compilers may not be able to handle them. One such example is Microsoft Visual C++ 6, though luckily 
it has no need for them as AngelScript already supports native calling conventions for it.

\section doc_addon_autowrap_1 Example usage

\code
#include "aswrappedcall.h"

// The application function that we want to register
void DoSomething(std::string param1, int param2);

// Registering the wrapper with AngelScript
void RegisterWrapper(asIScriptEngine *engine)
{
  int r;

  // The WRAP_FN macro automatically implements and returns the function pointer of the generic wrapper
  // function. Observe, that the calling convention should be set as asCALL_GENERIC in this case.
  r = engine->RegisterGlobalFunction("void DoSomething(string, int)", WRAP_FN(DoSomething), asCALL_GENERIC); assert( r >= 0 );
}
\endcode

\section doc_addon_autowrap_2 Adding support for more parameters

The aswrappedcall.h header file is by default prepared to support functions with up to 4 arguments. 
If you have a need for more arguments then you can use the generator that you find in the sub-directory 
to prepare a new header file.

Open the generateheader.cpp in an editor and set the max_args variable to the number of arguments you need
and then compile and run the code. It will print the new header file to the standard output so you just need
to direct this to a file.




\page doc_addon_helpers_try Exception routines

<b>Path:</b> /sdk/add_on/scripthelper/

The exception handling routines are registered by the application 
with a call to RegisterExceptionRoutines.

\section doc_addon_helpers_try_1 Public C++ interface

\code
// Register the exception routines.
//  'void throw(const string &msg)'
//  'string getExceptionInfo()'
void RegisterExceptionRoutines(asIScriptEngine *engine);
\endcode

\section doc_add_helpers_try_2 Public script interface

\see \ref doc_script_stdlib_exception




\page doc_addon_helpers Helper functions

<b>Path:</b> /sdk/add_on/scripthelper/

These helper functions simplify the implementation of common tasks. They can be used as is
or can serve as the starting point for your own framework.

\section doc_addon_helpers_1 Public C++ interface

\code
// Compare relation between two objects of the same type.
// Uses the object's opCmp method to perform the comparison.
// Returns a negative value if the comparison couldn't be performed.
int CompareRelation(asIScriptEngine *engine, void *leftObj, void *rightObj, int typeId, int &result);

// Compare equality between two objects of the same type.
// Uses the object's opEquals method to perform the comparison, or if that doesn't exist the opCmp method.
// Returns a negative value if the comparison couldn't be performed.
int CompareEquality(asIScriptEngine *engine, void *leftObj, void *rightObj, int typeId, bool &result);

// Compile and execute simple statements.
// The module is optional. If given the statements can access the entities compiled in the module.
// The caller can optionally provide its own context, for example if a context should be reused.
int ExecuteString(asIScriptEngine *engine, const char *code, asIScriptModule *mod = 0, asIScriptContext *ctx = 0);

// Compile and execute simple statements with option of return value.
// The module is optional. If given the statements can access the entitites compiled in the module.
// The caller can optionally provide its own context, for example if a context should be reused.
int ExecuteString(asIScriptEngine *engine, const char *code, void *ret, int retTypeId, asIScriptModule *mod = 0, asIScriptContext *ctx = 0);

// Format the details of the script exception into a human readable text.
// Whenever the asIScriptContext::Execute method returns asEXECUTION_EXCEPTION, the application 
// can call this function to get more information about that exception in a human readable form.
// The information obtained includes the current function, the script source section, 
// program position in the source section, and the exception description itself.
std::string GetExceptionInfo(asIScriptContext *ctx, bool showStack = false);

// Write registered application interface to file.
// This function creates a file with the configuration for the offline compiler, asbuild, in the samples.
// If you wish to use the offline compiler you should call this function from you application after the 
// application interface has been fully registered. This way you will not have to create the configuration
// file manually.
int WriteConfigToFile(asIScriptEngine *engine, const char *filename);

// Write the registered application interface to a text stream. 
int WriteConfigToStream(asIScriptEngine *engine, std::ostream &strm); 

// Loads an interface from a text stream and configures the engine with it. This will not 
// set the correct function pointers, so it is not possible to use this engine to execute
// scripts, but it can be used to compile scripts and save the byte code.
int ConfigEngineFromStream(asIScriptEngine *engine, std::istream &strm, const char *nameOfStream = "config", asIStringFactory *stringFactory = 0);
\endcode

\section doc_addon_helpers_2 Example

To compare two script objects the application can execute the following code:

\code
void Compare(asIScriptObject *a, asIScriptObject *b)
{
  asIScriptEngine *engine = a->GetEngine();
  int typeId = a->GetTypeId();

  int cmp;
  int r = CompareRelation(engine, a, b, typeId, cmp);
  if( r < 0 )
  {
    cout << "The relation between a and b cannot be established b" << endl;
  }
  else
  {
    if( cmp < 0 )
      cout << "a is smaller than b" << endl;
    else if( cmp == 0 )
      cout << "a is equal to b" << endl;
    else
      cout << "a is greater than b" << endl;
  }
}
\endcode

*/  
