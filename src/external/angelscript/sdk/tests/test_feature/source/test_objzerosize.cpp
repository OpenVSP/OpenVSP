#include "utils.h"

namespace TestObjZeroSize
{

static const char * const TESTNAME = "TestObjZeroSize";

class CObject
{
public:
	CObject() {val = 0;refCount = 1;}
	~CObject() {}
	void AddRef() {refCount++;}
	void Release() {if( --refCount == 0 ) delete this;}
	void Set(int v) {val = v;}
	int Get() {return val;}
	int val;
	int refCount;
};

CObject *Factory()
{
	return new CObject();
}

CObject g_obj;

CObject *CreateObject()
{
	CObject *obj = new CObject();
	
	// The constructor already initialized the reference counter with 1

	return obj;
}

bool Test()
{
	RET_ON_MAX_PORT

	bool fail = false;
	int r;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptArray(engine, true);
	engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	// Register an object type that cannot be instanciated by the script, but can be interacted with through object handles
	engine->RegisterObjectType("Object", 0, asOBJ_REF);
	engine->RegisterObjectBehaviour("Object", asBEHAVE_ADDREF, "void f()", asMETHOD(CObject, AddRef), asCALL_THISCALL);
	engine->RegisterObjectBehaviour("Object", asBEHAVE_RELEASE, "void f()", asMETHOD(CObject, Release), asCALL_THISCALL);
	engine->RegisterObjectMethod("Object", "void Set(int)", asMETHOD(CObject, Set), asCALL_THISCALL);
	engine->RegisterObjectMethod("Object", "int Get()", asMETHOD(CObject, Get), asCALL_THISCALL);
	engine->RegisterObjectProperty("Object", "int val", asOFFSET(CObject, val));

	engine->RegisterGlobalProperty("Object obj", &g_obj);
	engine->RegisterGlobalFunction("Object @CreateObject()", asFUNCTION(CreateObject), asCALL_CDECL);

	COutStream out;
	CBufferedOutStream bout;

	// Must not allow it to be declared as local variable
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = ExecuteString(engine, "Object obj;");
	if( r >= 0 || bout.buffer != "ExecuteString (1, 8) : Error   : Data type can't be 'Object'\n" )
	{
		PRINTF("%s: Didn't fail to compile as expected\n", TESTNAME);
		TEST_FAILED;
	}

	// Must not allow it to be declared as global variable
	bout.buffer = "";
	const char *script = "Object obj2;";
	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script, strlen(script));
	r = mod->Build();
	if( r >= 0 || bout.buffer != "script (1, 1) : Error   : Data type can't be 'Object'\n"
		                      /*   "script (1, 8) : Info    : Compiling Object obj2\n"
		  					     "script (1, 8) : Error   : No default constructor for object of type 'Object'.\n" */)
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	engine->DiscardModule(0);

	// It must not be allowed as sub type of array
	bout.buffer = "";
	r = ExecuteString(engine, "Object[] obj;");
	if( r >= 0 || bout.buffer != "ExecuteString (1, 7) : Error   : Data type can't be 'Object'\n"
							  /*   "array (0, 0) : Error   : The subtype has no default factory\n" */)
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	r = ExecuteString(engine, "Object @obj;");
	if( r < 0 )
	{
		PRINTF("%s: Failed to compile\n", TESTNAME);
		TEST_FAILED;
	}

	r = ExecuteString(engine, "Object@ obj = @CreateObject();");
	if( r < 0 )
	{
		PRINTF("%s: Failed to compile\n", TESTNAME);
		TEST_FAILED;
	}

	r = ExecuteString(engine, "CreateObject();");
	if( r < 0 )
	{
		PRINTF("%s: Failed to compile\n", TESTNAME);
		TEST_FAILED;
	}

	r = ExecuteString(engine, "Object@ obj = @CreateObject(); @obj = @CreateObject();");
	if( r < 0 )
	{
		PRINTF("%s: Failed to compile\n", TESTNAME);
		TEST_FAILED;
	}

	bout.buffer = "";
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = ExecuteString(engine, "Object@ obj = @CreateObject(); obj = CreateObject();");
	if( r >= 0 || bout.buffer != "ExecuteString (1, 36) : Error   : No appropriate opAssign method found in 'Object' for value assignment\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	// TODO: While the scenario isn't exactly invalid, it should probably give an error anyway
	/*
	bout.buffer = "";
	r = ExecuteString(engine, "@CreateObject() = @CreateObject();");
	if( r >= 0 || bout.buffer != "ExecuteString (1, 1) : Error   : Reference is temporary\n" )
	{
		PRINTF("%s: Didn't fail to compile as expected\n", TESTNAME);
		TEST_FAILED;
	}
	*/

	bout.buffer = "";
	r = ExecuteString(engine, "CreateObject() = CreateObject();");
	if( r >= 0 || bout.buffer != "ExecuteString (1, 16) : Error   : No appropriate opAssign method found in 'Object' for value assignment\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	// Test object with zero size as member of script class
	script = "  \n\
	 class myclass          \n\
	 {                      \n\
	   Object obj;          \n\
	 }                      \n";
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script, strlen(script));
	bout.buffer = "";
	r = mod->Build();
	if( r >= 0 )
		TEST_FAILED;
	if( bout.buffer != "script (4, 12) : Error   : Data type can't be 'Object'\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	engine->Release();

	// Success
	return fail;
}

} // namespace

