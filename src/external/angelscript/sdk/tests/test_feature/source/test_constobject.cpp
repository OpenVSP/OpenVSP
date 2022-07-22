#include "utils.h"

namespace TestConstObject
{

static const char * const TESTNAME = "TestConstObject";






class CObj
{
public: 
	CObj() {refCount = 1; val = 0; next = 0;}
	~CObj() {if( next ) next->Release();}

	CObj &operator=(CObj &other) 
	{
		val = other.val; 
		if( next ) 
			next->Release(); 
		next = other.next; 
		if( next ) 
			next->AddRef();
		return *this; 
	};

	void AddRef() {refCount++;}
	void Release() {if( --refCount == 0 ) delete this;}

	void SetVal(int _val) {this->val = _val;}
	int GetVal() const {return val;}

	int &operator[](int) {return val;}
	const int &operator[](int) const {return val;}

	int refCount;

	int val;

	CObj *next;
};

CObj *CObj_Factory()
{
	return new CObj();
}

CObj c_obj;

bool Test2();

bool Test()
{
	bool fail = Test2();

	int r;
	COutStream out;
	CBufferedOutStream bout;

	// passing a const handle on to a function expecting a const ref
	// http://www.gamedev.net/topic/681018-problem-in-arrayfind-with-a-const-param/
	{
		asIScriptEngine *engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
		RegisterScriptArray(engine, false);

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class CData { int a; } \n"
			"void func(const CData @d) \n"
			"{ \n"
			"  find(d); \n"
			"  array<CData@> arr; \n"
			"  arr.find(d); \n"
			"} \n"
			"void find(const CData@ &in f) \n"
			"{ \n"
			//"  f.a = 42; \n"  change is not allowed
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Ordinary tests
	SKIP_ON_MAX_PORT
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		// Register an object type
		r = engine->RegisterObjectType("obj", sizeof(CObj), asOBJ_REF); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("obj", asBEHAVE_FACTORY, "obj@ f()", asFUNCTION(CObj_Factory), asCALL_CDECL); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("obj", asBEHAVE_ADDREF, "void f()", asMETHOD(CObj, AddRef), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("obj", asBEHAVE_RELEASE, "void f()", asMETHOD(CObj, Release), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("obj", "obj &opAssign(const obj &in)", asMETHOD(CObj, operator=), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("obj", "int &opIndex(int)", asMETHODPR(CObj, operator[], (int), int&), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("obj", "const int &opIndex(int) const", asMETHODPR(CObj, operator[], (int) const, const int&), asCALL_THISCALL); assert(r >= 0);

		r = engine->RegisterObjectType("prop", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE); assert(r >= 0);
		r = engine->RegisterObjectProperty("prop", "int val", 0); assert(r >= 0);

		r = engine->RegisterObjectMethod("obj", "void SetVal(int)", asMETHOD(CObj, SetVal), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("obj", "int GetVal() const", asMETHOD(CObj, GetVal), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectProperty("obj", "int val", asOFFSET(CObj, val)); assert(r >= 0);
		r = engine->RegisterObjectProperty("obj", "obj@ next", asOFFSET(CObj, next)); assert(r >= 0);
		r = engine->RegisterObjectProperty("obj", "prop p", asOFFSET(CObj, val)); assert(r >= 0);

		r = engine->RegisterGlobalProperty("const obj c_obj", &c_obj); assert(r >= 0);
		r = engine->RegisterGlobalProperty("obj g_obj", &c_obj); assert(r >= 0);

		RegisterScriptString(engine);


		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		const char *script2 =
			"const string URL_SITE = \"http://www.sharkbaitgames.com\";                    \n"
			"const string URL_GET_HISCORES = URL_SITE + \"/get_hiscores.php\";             \n"
			"const string URL_CAN_SUBMIT_HISCORE = URL_SITE + \"/can_submit_hiscore.php\"; \n"
			"const string URL_SUBMIT_HISCORE = URL_SITE + \"/submit_hiscore.php\";         \n";
		mod->AddScriptSection("script1", script2, strlen(script2), 0);
		r = mod->Build();
		if (r < 0) TEST_FAILED;



		// TODO: A member array of a const object is also const

		// TODO: Parameters registered as &in and not const must make a copy of the object (even for operators)

		// A member object of a const object is also const
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		r = ExecuteString(engine, "c_obj.p.val = 1;");
		if (r >= 0) TEST_FAILED;
		if (bout.buffer != "ExecuteString (1, 13) : Error   : Reference is read-only\n") TEST_FAILED;

		c_obj.val = 0;
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		r = ExecuteString(engine, "g_obj.p.val = 1;");
		if (r < 0) TEST_FAILED;
		if (c_obj.val != 1) TEST_FAILED;

		// Allow overloading on const.
		r = ExecuteString(engine, "obj o; o[0] = 1;");
		if (r < 0) TEST_FAILED;

		// Allow return of const ref
		r = ExecuteString(engine, "int a = c_obj[0];");
		if (r < 0) TEST_FAILED;

		// Do not allow the script to call object behaviour that is not const on a const object
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		r = ExecuteString(engine, "c_obj[0] = 1;");
		if (r >= 0) TEST_FAILED;
		if (bout.buffer != "ExecuteString (1, 10) : Error   : Reference is read-only\n") TEST_FAILED;

		// Do not allow the script to take a non-const handle to a const object
		bout.buffer = "";
		r = ExecuteString(engine, "obj@ o = @c_obj;");
		if (r >= 0) TEST_FAILED;
		if (bout.buffer != "ExecuteString (1, 10) : Error   : Can't implicitly convert from 'const obj@' to 'obj@&'.\n")
			TEST_FAILED;

		// Do not allow the script to pass a const obj@ to a parameter that is not a const obj@
		const char *script =
			"void Test(obj@ o) { }";		mod->AddScriptSection("script", script, strlen(script));
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		mod->Build();

		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		r = ExecuteString(engine, "Test(@c_obj);", mod);
		if (r >= 0) TEST_FAILED;
		if (bout.buffer != "ExecuteString (1, 1) : Error   : No matching signatures to 'Test(const obj@&)'\n"
			"ExecuteString (1, 1) : Info    : Candidates are:\n"
			"ExecuteString (1, 1) : Info    : void Test(obj@ o)\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Do not allow the script to assign the object handle member of a const object
		bout.buffer = "";
		r = ExecuteString(engine, "@c_obj.next = @obj();");
		if (r >= 0) TEST_FAILED;
		if (bout.buffer != "ExecuteString (1, 13) : Error   : Reference is read-only\n")
			TEST_FAILED;

		// Allow the script to change the object the handle points to
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		r = ExecuteString(engine, "c_obj.next.val = 1;");
		if (r != 3) TEST_FAILED;

		// Allow the script take a handle to a non const object handle in a const object
		r = ExecuteString(engine, "obj @a = @c_obj.next;");
		if (r < 0) TEST_FAILED;

		// Allow the script to take a const handle to a const object
		r = ExecuteString(engine, "const obj@ o = @c_obj;");
		if (r < 0) TEST_FAILED;

		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		r = ExecuteString(engine, "obj @a; const obj @b; @a = @b;");
		if (r >= 0) TEST_FAILED;
		if (bout.buffer != "ExecuteString (1, 28) : Error   : Can't implicitly convert from 'const obj@' to 'obj@'.\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Allow a non-const handle to be assigned to a const handle
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		r = ExecuteString(engine, "obj @a; const obj @b; @b = @a;");
		if (r < 0) TEST_FAILED;

		// Do not allow the script to alter properties of a const object
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		r = ExecuteString(engine, "c_obj.val = 1;");
		if (r >= 0) TEST_FAILED;
		if (bout.buffer != "ExecuteString (1, 11) : Error   : Reference is read-only\n")
			TEST_FAILED;

		// Do not allow the script to call non-const methods on a const object
		bout.buffer = "";
		r = ExecuteString(engine, "c_obj.SetVal(1);");
		if (r >= 0) TEST_FAILED;
		if (bout.buffer != "ExecuteString (1, 7) : Error   : No matching signatures to 'obj::SetVal(const int) const'\n")
			TEST_FAILED;

		// Allow the script to call const methods on a const object
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		r = ExecuteString(engine, "c_obj.GetVal();");
		if (r < 0) TEST_FAILED;

		// Handle to const must not allow call to non-const methods
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		const char *script3 =
			"class CTest                           \n"
			"{                                     \n"
			"	int m_Int;                         \n"
			"                                      \n"
			"	void SetInt(int iInt)              \n"
			"	{                                  \n"
			"		m_Int = iInt;                  \n"
			"	}                                  \n"
			"};                                    \n"
			"void func()                           \n"
			"{                                     \n"
			"	const CTest Test;                  \n"
			"	const CTest@ TestHandle = @Test;   \n"
			"                                      \n"
			"	TestHandle.SetInt(1);              \n"
			"	Test.SetInt(1);                    \n"
			"}                                     \n";
		mod->AddScriptSection("script", script3, strlen(script3));
		r = mod->Build();
		if (r >= 0) TEST_FAILED;
		if (bout.buffer != "script (10, 1) : Info    : Compiling void func()\n"
			"script (15, 13) : Error   : No matching signatures to 'CTest::SetInt(const int) const'\n"
			"script (16, 7) : Error   : No matching signatures to 'CTest::SetInt(const int) const'\n")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		// Allow passing a const object to a function that takes a non-const object by value
		bout.buffer = "";
		const char *script4 = "void func(prop val) {}";
		mod->AddScriptSection("script", script4, strlen(script4));
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;
		r = ExecuteString(engine, "const prop val; func(val)", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Success
	return fail;
}



bool Test2()
{
	bool fail = false;

	int r;
	COutStream out;
	CBufferedOutStream bout;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	// Try calling a const method on a const object (success)
	const char *script = 
	"class MyType                  \n"
	"{                             \n"
	"   int val;                   \n"
	"   void TestConst() const     \n"
	"   {                          \n"
	"      assert(val == 5);       \n"
	"   }                          \n"
	"}                             \n"
	"void Func(const MyType &in a) \n"
	"{                             \n"
	"   a.TestConst();             \n"
	"}                             \n";

	asIScriptModule *mod = engine->GetModule("module", asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script, strlen(script));
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
	}

	// Try calling a non-const method on a const object (should fail)
	script =
	"class MyType                  \n"
	"{                             \n"
	"   int val;                   \n"
	"   void TestConst()           \n"
	"   {                          \n"
	"      assert(val == 5);       \n"
	"   }                          \n"
	"}                             \n"
	"void Func(const MyType &in a) \n"
	"{                             \n"
	"   a.TestConst();             \n"
	"}                             \n";

	engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

	bout.buffer = "";
	mod->AddScriptSection("script", script, strlen(script));
	r = mod->Build();
	if( r >= 0 )
	{
		TEST_FAILED;
	}
	if( bout.buffer != "script (9, 1) : Info    : Compiling void Func(const MyType&in)\n"
					   "script (11, 6) : Error   : No matching signatures to 'MyType::TestConst() const'\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	// Try modifying self in a const method (should fail)
	script = 
	"class MyType                  \n"
	"{                             \n"
	"   int val;                   \n"
	"   void TestConst() const     \n"
	"   {                          \n"
	"      val = 5;                \n"
	"   }                          \n"
	"}                             \n";
	
	bout.buffer = "";
	mod->AddScriptSection("script", script, strlen(script));
	r = mod->Build();
	if( r >= 0 )
	{
		TEST_FAILED;
	}
	if( bout.buffer != "script (4, 4) : Info    : Compiling void MyType::TestConst() const\n"
					   "script (6, 11) : Error   : Reference is read-only\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	// Try modifying self via 'this' in a const method (should fail)
	script = 
	"class MyType                  \n"
	"{                             \n"
	"   int val;                   \n"
	"   void TestConst() const     \n"
	"   {                          \n"
	"      this.val = 5;           \n"
	"   }                          \n"
	"}                             \n";
	
	bout.buffer = "";
	mod->AddScriptSection("script", script, strlen(script));
	r = mod->Build();
	if( r >= 0 )
	{
		TEST_FAILED;
	}
	if( bout.buffer != "script (4, 4) : Info    : Compiling void MyType::TestConst() const\n"
					   "script (6, 16) : Error   : Reference is read-only\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}

	engine->Release();

	return fail;
}

} // namespace

