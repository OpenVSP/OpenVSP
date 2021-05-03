//
// Tests importing functions from other modules
//
// Test author: Andreas Jonsson
//

#include <memory>
#include <vector>
#include "utils.h"
#include "../../../add_on/scriptarray/scriptarray.h"
#include "../../../add_on/scripthandle/scripthandle.h"


namespace TestSaveLoad
{

using namespace std;

static const char * const TESTNAME = "TestSaveLoad";



static const char * const g_script1 =
"import void Test() from 'DynamicModule';     \n"
"OBJ g_obj;                                   \n"
"array<A@> g_a = {A(),A()};                   \n"
"array<string> g_s = {'a','b','c'};           \n"
"A @gHandle;                                  \n"
"funcdef void func_t(OBJ, float, A @);        \n"
"void func(OBJ o, float f, A @a) {}           \n"
"enum ETest {}                                \n"
"void main()                                  \n"
"{                                            \n"
"  Test();                                    \n"
"  TestStruct();                              \n"
"  TestArray();                               \n"
"  GlobalCharArray.resize(1);                 \n"
"  string s = ARRAYTOHEX(GlobalCharArray);    \n"
"  func_t @f = func;                          \n"
"  f(OBJ(), 1, A());                          \n"
"}                                            \n"
"void TestObj(OBJ &out obj)                   \n"
"{                                            \n"
"}                                            \n"
"void TestStruct()                            \n"
"{                                            \n"
"  A a;                                       \n"
"  a.a = 2;                                   \n"
"  A@ b = @a;                                 \n"
"}                                            \n"
"void TestArray()                             \n"
"{                                            \n"
"  A[] c(3);                                  \n"
"  int[] d(2);                                \n"
"  A[]@[] e(1);                               \n"
"  @e[0] = @c;                                \n"
"}                                            \n"
"class A                                      \n"
"{                                            \n"
"  int a;                                     \n"
"  ETest e;                                   \n"
"};                                           \n"
"void TestHandle(A @a)                        \n"
"{                                            \n"
"}                                            \n"
"interface MyIntf                             \n"
"{                                            \n"
"  void test();                               \n"
"}                                            \n"
"class MyClass : MyIntf                       \n"
"{                                            \n"
"  array<int> arr = {1000,200,40,1};          \n"
"  int sum = arr[0] + arr[1] + arr[2] + arr[3]; \n"
"  void test() {number = sum;}                \n"
"}                                            \n";

static const char * const g_script2 =
"void Test()                               \n"
"{                                         \n"
"  int[] a(3);                             \n"
"  a[0] = 23;                              \n"
"  a[1] = 13;                              \n"
"  a[2] = 34;                              \n"
"  if( a[0] + a[1] + a[2] == 23+13+34 )    \n"
"    number = 1234567890;                  \n"
"}                                         \n";

static const char * const g_script3 =
"float[] f(5);       \n"
"void Test(int a) {} \n";

static const char * const g_script4 =
"class CheckCollision                          \n"
"{                                             \n"
"	Actor@[] _list1;                           \n"
"                                              \n"
"	void Initialize() {                        \n"
"		_list1.resize(1);                      \n"
"	}                                          \n"
"                                              \n"
"	void Register(Actor@ entity){              \n"
"		@_list1[0] = @entity;                  \n"
"	}                                          \n"
"}                                             \n"
"                                              \n"
"CheckCollision g_checkCollision;              \n"
"                                              \n"
"class Shot : Actor {                          \n"
"	void Initialize(int a = 0) {               \n"
"		g_checkCollision.Register(this);       \n"
"	}                                          \n"
"}                                             \n"
"interface Actor {  }				           \n"
"InGame g_inGame;                              \n"
"class InGame					   	           \n"
"{									           \n"
"	Ship _ship;						           \n"
"	void Initialize(int level)		           \n"
"	{								           \n"
"		g_checkCollision.Initialize();         \n"
"		_ship.Initialize();	                   \n"
"	}						   		           \n"
"}									           \n"
"class Ship : Actor							   \n"
"{											   \n"
"   Shot@[] _shots;							   \n"
"	void Initialize()						   \n"
"	{										   \n"
"		_shots.resize(5);					   \n"
"                                              \n"
"		for (int i=0; i < 5; i++)              \n"
"		{                                      \n"
"			Shot shot;						   \n"
"			@_shots[i] = @shot;                \n"
"			_shots[i].Initialize();	           \n"
"		}                                      \n"
"	}										   \n"
"}											   \n";

// Make sure the handle can be explicitly taken for class properties, array members, and global variables
static const char * const g_script5 =
"IsoMap      _iso;                                      \n"
"IsoSprite[] _sprite;                                   \n"
"                                                       \n"
"int which = 0;                                         \n"
"                                                       \n"
"bool Initialize() {                                    \n"
"  if (!_iso.Load('data/iso/map.imp'))                  \n"
"    return false;                                      \n"
"                                                       \n"
"  _sprite.resize(100);                                 \n"
"                                                       \n"
"  if (!_sprite[0].Load('data/iso/pacman.spr'))         \n"
"    return false;                                      \n"
"                                                       \n"
"  for (int i=1; i < 100; i++) {                        \n"
"    if (!_sprite[i].Load('data/iso/residencia1.spr'))  \n"
"      return false;                                    \n"
"  }                                                    \n"
"                                                       \n"
"                                                       \n"
"   _iso.AddEntity(_sprite[0], 0, 0, 0);                \n"
"                                                       \n"
"   return true;                                        \n"
"}                                                      \n";

class CScriptStringFactory : public asIStringFactory
{
public:
	CScriptStringFactory() {}
	const void *GetStringConstant(const char *data, asUINT length)
	{
		return new string(data, length);
	}
	int ReleaseStringConstant(const void *str)
	{
		if (!str)
			return -1;
		delete reinterpret_cast<const string*>(str);
		return 0;
	}
	int GetRawStringData(const void *str, char *data, asUINT *length) const
	{
		if (!str)
			return -1;
		if (length)
			*length = (asUINT)reinterpret_cast<const string*>(str)->length();
		if (data)
			memcpy(data, reinterpret_cast<const string*>(str)->c_str(), reinterpret_cast<const string*>(str)->length());
		return 0;
	}
};

static CScriptStringFactory stringFactory;

bool fail = false;
int number = 0;
int number2 = 0;
CScriptArray* GlobalCharArray = 0;

void print(const string &)
{
}

int getInt()
{
	return 42;
}

void ArrayToHexStr(asIScriptGeneric *gen)
{
	new(gen->GetAddressOfReturnLocation()) string();
}

asIScriptEngine *ConfigureEngine(int version)
{
	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	RegisterScriptArray(engine, true);
	RegisterStdString(engine);

	// Register a property with the built-in array type
	GlobalCharArray = (CScriptArray*)engine->CreateScriptObject(engine->GetTypeInfoById(engine->GetTypeIdByDecl("uint8[]")));
	int r = engine->RegisterGlobalProperty("uint8[] GlobalCharArray", GlobalCharArray); assert( r >= 0 );

	// Register function that use the built-in array type
	r = engine->RegisterGlobalFunction("string ARRAYTOHEX(uint8[] &in)", asFUNCTION(ArrayToHexStr), asCALL_GENERIC); assert( r >= 0 );


	if( version == 1 )
	{
		// The order of the properties shouldn't matter
		engine->RegisterGlobalProperty("int number", &number);
		engine->RegisterGlobalProperty("int number2", &number2);
	}
	else
	{
		// The order of the properties shouldn't matter
		engine->RegisterGlobalProperty("int number2", &number2);
		engine->RegisterGlobalProperty("int number", &number);
	}
	engine->RegisterObjectType("OBJ", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);

	return engine;
}

void TestScripts(asIScriptEngine *engine)
{
	int r;

	// Bind the imported functions
	asIScriptModule *mod = engine->GetModule(0);
	r = mod->BindAllImportedFunctions(); assert( r >= 0 );

	// Verify if handles are properly resolved
	asIScriptFunction *func = mod->GetFunctionByDecl("void TestHandle(A @)");
	if( func == 0 ) 
	{
		PRINTF("%s: Failed to identify function with handle\n", TESTNAME);
		TEST_FAILED;
	}

	ExecuteString(engine, "main()", mod);

	if( number != 1234567890 )
	{
		PRINTF("%s: Failed to set the number as expected\n", TESTNAME);
		TEST_FAILED;
	}

	// Call an interface method on a class that implements the interface
	asITypeInfo *type = engine->GetModule(0)->GetTypeInfoByName("MyClass");
	asIScriptObject *obj = (asIScriptObject*)engine->CreateScriptObject(type);

	int intfTypeId = engine->GetModule(0)->GetTypeIdByDecl("MyIntf");
	type = engine->GetTypeInfoById(intfTypeId);
	if( type == 0 )
		TEST_FAILED;
	else
		func = type->GetMethodByDecl("void test()");
	asIScriptContext *ctx = engine->CreateContext();
	r = ctx->Prepare(func);
	if( r < 0 ) TEST_FAILED;
	ctx->SetObject(obj);
	ctx->Execute();
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	if( ctx ) ctx->Release();
	if( obj ) obj->Release();

	if( number != 1241 )
	{
		PRINTF("%s: Interface method failed\n", TESTNAME);
		TEST_FAILED;
	}
}

void ConstructFloatArray(vector<float> *p)
{
	new(p) vector<float>;
}

void ConstructFloatArray(int s, vector<float> *p)
{
	new(p) vector<float>(s);
}

void DestructFloatArray(vector<float> *p)
{
	p->~vector<float>();
}

void IsoMapFactory(asIScriptGeneric *gen)
{
	*(int**)gen->GetAddressOfReturnLocation() = new int(1);
}

void IsoSpriteFactory(asIScriptGeneric *gen)
{
	*(int**)gen->GetAddressOfReturnLocation() = new int(1);
}

void DummyAddref(asIScriptGeneric *gen)
{
	int *object = (int*)gen->GetObject();
	(*object)++;
}

void DummyRelease(asIScriptGeneric *gen)
{
	int *object = (int*)gen->GetObject();
	(*object)--;
	if( *object == 0 )
		delete object;
}

void Dummy(asIScriptGeneric *)
{
}

static string _out;
void output(asIScriptGeneric *gen)
{
	string *str = (string*)gen->GetArgAddress(0);
	_out += *str;
}

bool Test2();

class Tmpl
{
public:
	Tmpl() {refCount = 1;}
	void AddRef() {refCount++;}
	void Release() {if( --refCount == 0 ) delete this;}
	static Tmpl *TmplFactory(asITypeInfo*) {return new Tmpl;}
	static bool TmplCallback(asITypeInfo * /*ot*/, bool & /*dontGC*/) {return false;}
	int refCount;
};

bool TestAndrewPrice();

static asIScriptFunction *g_func = 0;

class NGUIWidget
{
public:
	float alpha;

	NGUIWidget()
	{
		alpha = 0;
	};
};

class NGUISymbol : public NGUIWidget
{
};

void NGUIWidgetCastGeneric(asIScriptGeneric *gen)
{
	NGUIWidget* wgt = (NGUIWidget*)gen->GetObject();

	gen->SetReturnAddress(wgt);
}

NGUISymbol symbol_inst;

bool Test()
{
	int r;
	COutStream out;
	CBufferedOutStream bout;
	asIScriptEngine* engine;
	asIScriptModule* mod;

	// test loading bytecode with function returning string by value
	// reported by Quentin Cosendey
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		RegisterStdString(engine);

		CBytecodeStream stream(__FILE__);

		{
			mod = engine->GetModule(0, asGM_ALWAYS_CREATE); assert(mod != NULL);

			r = mod->AddScriptSection("main", 
				"class C { \n"
				"		string m() { \n"
				"		return 'yes'; \n"
				"	} \n"
				"} \n"
				"void main() { \n"
				"	//print('OK'); \n"
				"} \n"); assert(r >= 0);
			r = mod->Build(); assert(r >= 0);
			r = mod->SaveByteCode(&stream); assert(r >= 0);
			mod->Discard();
		}

		{
			mod = engine->GetModule(0, asGM_ALWAYS_CREATE); assert(mod != NULL);

			r = mod->LoadByteCode(&stream);
			if (r < 0)
				TEST_FAILED;
			mod->Discard();
		}

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = engine->ShutDownAndRelease(); assert(r >= 0);
	}

	// test loading bytecode with function returning object by value
	// reported by Phong Ba
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		CBytecodeStream stream(__FILE__);

		r = engine->RegisterObjectType("vObj", sizeof(int), asOBJ_VALUE | asOBJ_POD); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("vObj", asBEHAVE_CONSTRUCT, "void f()", NULL, asCALL_GENERIC);

		{
			mod = engine->GetModule(0, asGM_ALWAYS_CREATE); assert(mod != NULL);

			r = mod->AddScriptSection("main", "class vClass { vObj abc() {return vObj();} }"); assert(r >= 0);
			r = mod->Build(); assert(r >= 0);
			r = mod->SaveByteCode(&stream); assert(r >= 0);
			mod->Discard();
		}

		{
			mod = engine->GetModule(0, asGM_ALWAYS_CREATE); assert(mod != NULL);

			r = mod->LoadByteCode(&stream);
			if (r < 0)
				TEST_FAILED;
			mod->Discard();
		}

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = engine->ShutDownAndRelease(); assert(r >= 0);
	}

	// Test loading bytecode with invalid stream
	// https://www.gamedev.net/forums/topic/709287-crash-when-loading-bytecode/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		class CBrokenStream : public asIBinaryStream
		{
			int Write(const void* ptr, asUINT size)
			{
				return 0;
			}

			int Read(void* ptr, asUINT size)
			{
				// noop, we don't write anything to ptr at all
				return 0;
			}
		};

		CBrokenStream stream;
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE); assert(mod != NULL);
		r = mod->LoadByteCode(&stream);
		if (r >= 0)
			TEST_FAILED;
		mod->Discard();

		if( bout.buffer != " (0, 0) : Error   : LoadByteCode failed. The bytecode is invalid. Number of bytes read from stream: 9\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = engine->ShutDownAndRelease();
	}

	// Test bytecode loading with value type and list constructor
	// Reported by Phong Ba
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
	
		CBytecodeStream stream(__FILE__);

		r = engine->RegisterObjectType("vObj", 1, asOBJ_VALUE | asOBJ_POD); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("vObj", asBEHAVE_LIST_CONSTRUCT, "void f(int &in) {repeat int}", asFUNCTION(0), asCALL_GENERIC); assert(r >= 0);

		{
			mod = engine->GetModule(0, asGM_ALWAYS_CREATE); assert(mod != NULL);

			r = mod->AddScriptSection("main", "void main() {vObj t = {1, 2, 3};}"); assert(r >= 0);
			r = mod->Build(); assert(r >= 0);
			r = mod->SaveByteCode(&stream); assert(r >= 0);
			mod->Discard();
		}

		{
			mod = engine->GetModule(0, asGM_ALWAYS_CREATE); assert(mod != NULL);

			r = mod->LoadByteCode(&stream); assert(r >= 0);
			mod->Discard();
		}

		if (bout.buffer != "") 
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = engine->ShutDownAndRelease(); assert(r >= 0);		
	}

	// Test crash with mismatched shared classes
	// Reported by Julius Narvilas/MrFloat
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";
	
		CBytecodeStream stream1("a");
		CBytecodeStream stream2("b");	
		
		// Compile script 2
		{
			const char* file2 = ""
				"shared class Test1 { \n"
				//mismatching functions
				"	int function2() { return 0; } \n"
				""
				"}\n";

			mod = engine->GetModule("test2", asGM_ALWAYS_CREATE);
			r = mod->AddScriptSection("test2", file2, strlen(file2));
			r = mod->Build();
			if( r < 0 )
				TEST_FAILED;
			r = mod->SaveByteCode(&stream2);
			mod->Discard();
		}
		
		// Compile script 1
		{
				const char* file1 = ""
				"shared class Test1 { \n"
				//mismatching functions
				"	int function1() { return 0; } \n"
				""
				"}\n";

			mod = engine->GetModule("test1", asGM_ALWAYS_CREATE);
			r = mod->AddScriptSection("test1", file1, strlen(file1));
			r = mod->Build();
			if( r < 0 )
				TEST_FAILED;
			r = mod->SaveByteCode(&stream1);
			mod->Discard();
		}
		
		// Load the previously saved bytecode over the first module
		asIScriptModule* mod2 = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		r = mod2->LoadByteCode(&stream2);
		if( r < 0 )
			TEST_FAILED;

		//invalid bytecode with outdated shared class into a new module
		asIScriptModule* mod1_1 = engine->GetModule("test1_1", asGM_ALWAYS_CREATE);
		r = mod1_1->LoadByteCode(&stream1);
		if( r >= 0 )
			TEST_FAILED;

		//just need another compilation to trigger "engine->signatureIds" array search
		asIScriptModule* mod1_2 = engine->GetModule("test1_2", asGM_ALWAYS_CREATE);
		stream1.Restart();
		r = mod1_2->LoadByteCode(&stream1);
		if( r >= 0 )
			TEST_FAILED;
		
		if (bout.buffer != " (0, 0) : Error   : Shared type 'Test1' doesn't match the original declaration in other module\n"
						   " (0, 0) : Error   : LoadByteCode failed. The bytecode is invalid. Number of bytes read from stream: 90\n"
						   " (0, 0) : Error   : Shared type 'Test1' doesn't match the original declaration in other module\n"
						   " (0, 0) : Error   : LoadByteCode failed. The bytecode is invalid. Number of bytes read from stream: 90\n") 
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}
	
	// Test same function in different namespaces
	// https://www.gamedev.net/forums/topic/697445-loadbytecode-fails-when-there-is-a-function-with-the-same-name-in-the-namespace/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection(
			"test",
			"namespace X { shared float A()  {return 0.f;} }"
			"namespace Y { shared float A()  {return 0.f;} }"
			"void test() {"
			"	float v;"
			"	v = Y::A();"
			"}");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		CBytecodeStream bc1("test");
		r = mod->SaveByteCode(&bc1);
		assert(r >= 0);
		if (r < 0)
			TEST_FAILED;

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&bc1);
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "") 
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test external shared classes in namespaces with virtual methods while loading bytecode
	// https://www.gamedev.net/forums/topic/697083-loading-bytecode-fails-when-combining-namespace-and-shared-virtual-method/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"namespace X {"
			"shared class A"
			"{"
			"}"
			"shared class B : A"
			"{"
			"  void bar() {}"
			"}}"
		);
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		asIScriptModule *mod2 = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		mod2->AddScriptSection("test",
			"namespace X {"
			"external shared class A;"
			"external shared class B;"
			"}"
			"namespace Y {"
			"class Test : X::A"
			"{"
			"  X::B b;"
			"  void foo() { b.bar(); }"
			"}}"
		);
		r = mod2->Build();
		if (r < 0)
			TEST_FAILED;

		CBytecodeStream bc1("test");
		r = mod->SaveByteCode(&bc1); assert(r >= 0);
		if (r < 0)
			TEST_FAILED;

		CBytecodeStream bc2("test2");
		r = mod2->SaveByteCode(&bc2); assert(r >= 0);
		if (r < 0)
			TEST_FAILED;

		mod2->Discard();

		// game loop
		for (unsigned i = 0; i < 10; ++i) {
			engine->GarbageCollect(); // run DeleteDiscardedModules();
		}

		mod2 = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		r = mod2->LoadByteCode(&bc2);
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test external shared classes in namespaces while loading bytecode
	// https://www.gamedev.net/forums/topic/697035-reading-bytecode-fails-when-combining-namespace-and-shared-class/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"namespace X {"
			"shared class A"
			"{"
			"  A()"
			"    {}"
			"}};"
		);
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		asIScriptModule *mod2 = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		mod2->AddScriptSection("test",
			"namespace X {"
			"external shared class A;"
			"}"
			"namespace Y {"
			"class Test : X::A"
			"{"
			"  Test() {}"
			"}};"
		);
		r = mod2->Build();
		if (r < 0)
			TEST_FAILED;

		CBytecodeStream bc1("test");
		r = mod->SaveByteCode(&bc1); assert(r >= 0);
		if (r < 0)
			TEST_FAILED;

		CBytecodeStream bc2("test2");
		r = mod2->SaveByteCode(&bc2); assert(r >= 0);
		if (r < 0)
			TEST_FAILED;

		mod2->Discard();

		// game loop
		for (unsigned i = 0; i < 10; ++i) {
			engine->GarbageCollect(); // run DeleteDiscardedModules();
		}

		mod2 = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		r = mod2->LoadByteCode(&bc2);
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test lambda in shared class
	// https://www.gamedev.net/forums/topic/696953-using-lambda-function-in-shared-class-fails-to-read-bytecode/
	// Don't save bytecode for inherited methods, especially if the base class is declared as external
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		int called = 0;
		engine->RegisterGlobalProperty("int called", &called);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"shared funcdef void TestFunc(); \n"
			"shared class TestBase \n"
			"{ \n"
			"  void callfn(TestFunc@ fn) \n"
			"    { \n"
			"       fn(); \n"
			"    } \n"
			"  void test() \n"
			"    { \n"
			"      callfn( function() { called = 42; } ); \n"
			"    } \n"
			"}; \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		asIScriptModule *mod2 = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		mod2->AddScriptSection("test",
			"external shared funcdef void TestFunc(); \n"
			"external shared class TestBase; \n"
			"class Test : TestBase \n"
			"{ \n"
			"}; \n"
			"class Test2 : TestBase \n" // The second class shares the same base, thus must also work
			"{ \n"
			"}; \n");
		r = mod2->Build();
		if (r < 0)
			TEST_FAILED;

		r = ExecuteString(engine, "Test t; t.test();", mod2);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		if( called != 42 )
			TEST_FAILED;
		called = 0;

		CBytecodeStream bc1("test");
		r = mod->SaveByteCode(&bc1); assert(r >= 0);
		if (r < 0)
			TEST_FAILED;

		CBytecodeStream bc2("test2");
		r = mod2->SaveByteCode(&bc2); assert(r >= 0);
		if (r < 0)
			TEST_FAILED;

		engine->ShutDownAndRelease();
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalProperty("int called", &called);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&bc1);
		if (r < 0)
			TEST_FAILED;

		mod2 = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		r = mod2->LoadByteCode(&bc2);
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		r = ExecuteString(engine, "Test t; t.test();", mod2);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		if( called != 42 )
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test loading bytecode with existing shared interfaces with inheritance
	// reported by Zakhar
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"shared interface ScriptObject {} \n"
			"shared interface BaseEffect : ScriptObject {} \n"
			"class A : BaseEffect {} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		asIScriptModule *mod2 = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		mod2->AddScriptSection("test",
			"shared interface ScriptObject {} \n"
			"shared interface BaseEffect : ScriptObject {} \n"
			"class B : BaseEffect {} \n");
		r = mod2->Build();
		if (r < 0)
			TEST_FAILED;

		CBytecodeStream bc1("test");
		r = mod->SaveByteCode(&bc1); assert(r >= 0);
		if (r < 0)
			TEST_FAILED;

		CBytecodeStream bc2("test2");
		r = mod2->SaveByteCode(&bc2); assert(r >= 0);
		if (r < 0)
			TEST_FAILED;

		engine->ShutDownAndRelease();
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&bc1);
		if (r < 0)
			TEST_FAILED;

		mod2 = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		r = mod2->LoadByteCode(&bc2);
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test forever loop where the return instruction is never reached
	// https://www.gamedev.net/forums/topic/696323-when-using-an-infinite-loop-using-for-statement-and-stopping-with-savebytecode/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void main() \n"
			"{ \n"
			"  for (;;) \n"
			"  { \n"
			"  } \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		CBytecodeStream bc1("test");
		r = mod->SaveByteCode(&bc1); assert(r >= 0);
		if (r < 0)
			TEST_FAILED;

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&bc1);
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test saving interfaces with inheritance
	// https://www.gamedev.net/forums/topic/696191-using-interface-inheritance-i-stopped-with-savebytecode/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"interface A \n"
			"{ \n"
			"}; \n"
			"interface B : A \n"
			"{ \n"
			"}; \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		CBytecodeStream bc1("test");
		r = mod->SaveByteCode(&bc1); assert(r >= 0);
		if (r < 0)
			TEST_FAILED;

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&bc1);
		if (r < 0)
			TEST_FAILED;

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		
		engine->ShutDownAndRelease();
	}

	// Test saving and loading modules that have shared classes where the
	// content of the shared class was inherited from the other module
	// Reported by Phong Ba
	{
		r = 0;
		engine = asCreateScriptEngine();
		asIScriptContext *ctx = NULL;

		CBytecodeStream bc1(__FILE__"1");
		CBytecodeStream bc2(__FILE__"2");

		{
			asIScriptModule *mod1 = NULL;
			asIScriptModule *mod2 = NULL;

			const char *script1 =
				//"shared int func1() {return 1;}\n"
				//"shared class Cls {int method() {return 1 + func1();} };\n"
				"shared class Cls {int method() {return 2;} };\n"
				;

			const char *script2 =
				"shared class Cls {};\n"
				"int main() {Cls c; return 1 + c.method();}"
				;

			mod1 = engine->GetModule("1", asGM_ALWAYS_CREATE); assert(mod1 != NULL);
			mod2 = engine->GetModule("2", asGM_ALWAYS_CREATE); assert(mod2 != NULL);

			r = mod1->AddScriptSection("main", script1); assert(r >= 0);
			r = mod1->Build(); assert(r >= 0);

			r = mod2->AddScriptSection("main", script2); assert(r >= 0);
			r = mod2->Build(); assert(r >= 0);

			r = mod1->SaveByteCode(&bc1); assert(r >= 0);
			r = mod2->SaveByteCode(&bc2); assert(r >= 0);

			ctx = engine->CreateContext(); assert(ctx != NULL);
			r = ctx->Prepare(mod2->GetFunctionByDecl("int main()"));assert(r >= 0);
			r = ctx->Execute(); assert(r >= 0);
			r = ctx->GetReturnDWord();assert(r == 3);
			r = ctx->Release(); assert(r >= 0);

			mod1->Discard();
			mod2->Discard();
		}

		{ // <== The bytecode won't load even Cls using func1 or not.
			asIScriptModule *mod1 = NULL;
			asIScriptModule *mod2 = NULL;

			mod1 = engine->GetModule("1", asGM_ALWAYS_CREATE); assert(mod1 != NULL);
			mod2 = engine->GetModule("2", asGM_ALWAYS_CREATE); assert(mod2 != NULL);

			r = mod1->LoadByteCode(&bc1);
			if (r < 0)
				TEST_FAILED;
			r = mod2->LoadByteCode(&bc2);
			if (r < 0)
				TEST_FAILED;

			ctx = engine->CreateContext(); assert(ctx != NULL);
			r = ctx->Prepare(mod2->GetFunctionByDecl("int main()"));assert(r >= 0);
			r = ctx->Execute(); assert(r >= 0);
			r = ctx->GetReturnDWord();
			if (r != 3)
				TEST_FAILED;
			r = ctx->Release(); assert(r >= 0);
		}

		r = engine->ShutDownAndRelease(); assert(r >= 0);
	}

	// Test saving and loading modules that have shared functions 
	// that use other shared functions not declared in the module
	// Reported by Phong Ba
	{
		r = 0;
		engine = asCreateScriptEngine();

		CBytecodeStream bc1(__FILE__"1");
		CBytecodeStream bc2(__FILE__"2");

		{
			asIScriptModule *mod1 = NULL;
			asIScriptModule *mod2 = NULL;

			asIScriptContext *ctx = NULL;

			const char *script1 =
				"shared int func1() {return 1;}\n"
				"shared int func2(int p){return p + func1();}\n";

			const char *script2 =
				//"shared int func1() {}\n" // <== Commented it will cause LoadByteCode fail.
				"shared int func2(int p) {}\n"
				"int main() {return func2(2);}";

			mod1 = engine->GetModule("1", asGM_ALWAYS_CREATE); assert(mod1 != NULL);
			mod2 = engine->GetModule("2", asGM_ALWAYS_CREATE); assert(mod2 != NULL);

			r = mod1->AddScriptSection("main", script1); assert(r >= 0);
			r = mod1->Build(); assert(r >= 0);

			r = mod2->AddScriptSection("main", script2); assert(r >= 0);
			r = mod2->Build(); assert(r >= 0);

			r = mod1->SaveByteCode(&bc1);
			if (r < 0)
				TEST_FAILED;
			r = mod2->SaveByteCode(&bc2);
			if (r < 0)
				TEST_FAILED;

			ctx = engine->CreateContext(); assert(ctx != NULL);
			r = ctx->Prepare(mod2->GetFunctionByDecl("int main()")); assert(r >= 0);
			r = ctx->Execute(); assert(r >= 0);
			r = ctx->GetReturnDWord();
			if (r != 3)
				TEST_FAILED;
			r = ctx->Release(); assert(r >= 0);

			mod1->Discard();
			mod2->Discard();
		}  // <== The script run (directly) correctly with or without redeclare of func1

		{  // <== The second module won't load the bytecode when we save the bytecode without redeclare of func1 (caused by the func2 call the func1, it won't happen if func2 is not calling the func1)
			asIScriptModule *mod1 = NULL;
			asIScriptModule *mod2 = NULL;

			mod1 = engine->GetModule("1", asGM_ALWAYS_CREATE); assert(mod1 != NULL);
			mod2 = engine->GetModule("2", asGM_ALWAYS_CREATE); assert(mod2 != NULL);

			r = mod1->LoadByteCode(&bc1);
			if (r < 0)
				TEST_FAILED;
			r = mod2->LoadByteCode(&bc2);
			if (r < 0)
				TEST_FAILED;

			asIScriptContext *ctx = engine->CreateContext(); assert(ctx != NULL);
			r = ctx->Prepare(mod2->GetFunctionByDecl("int main()")); assert(r >= 0);
			r = ctx->Execute(); assert(r >= 0);
			r = ctx->GetReturnDWord(); 
			if (r != 3)
				TEST_FAILED;
			r = ctx->Release(); assert(r >= 0);
		}

		r = engine->ShutDownAndRelease(); assert(r >= 0);
	}

	// Test adjustement of pointers in call to constructor with asBC_ALLOC instruction
	// http://www.gamedev.net/topic/682729-assert-failure-when-saving-bytecode-in-adjustgetoffset-linux-amd64/
	{
		engine = asCreateScriptEngine();

		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		RegisterStdString(engine);

		engine->RegisterObjectType("vec4", 16, asOBJ_VALUE | asOBJ_POD);
		engine->RegisterObjectBehaviour("vec4", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("vec4", asBEHAVE_CONSTRUCT, "void f(const vec4 &in)", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("vec4", "vec4 &opAssign(const vec4 &in)", asFUNCTION(0), asCALL_GENERIC);

		engine->RegisterObjectType("FontSetup", 4, asOBJ_VALUE| asOBJ_APP_CLASS_CDAK);
		engine->RegisterObjectBehaviour("FontSetup", asBEHAVE_CONSTRUCT, "void f( const string &in _name, int32 _size, vec4 _color, bool _shadowed = false )", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectBehaviour("FontSetup", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(0), asCALL_GENERIC);

		engine->RegisterGlobalProperty("const vec4 white", (void*)1);

		engine->SetEngineProperty(asEP_INIT_GLOBAL_VARS_AFTER_BUILD, false);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"FontSetup selectionListFont('OpenSans - Regular', 70, white, true); \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		CBytecodeStream bc("test");
		r = mod->SaveByteCode(&bc);
		if (r < 0)
			TEST_FAILED;
		else
		{
			asDWORD crc32 = ComputeCRC32(&bc.buffer[0], asUINT(bc.buffer.size()));
			if (crc32 != 0x3DC90FE)
			{
				PRINTF("The saved byte code has different checksum than the expected. Got 0x%X\n", crc32);
				TEST_FAILED;
			}

			mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
			r = mod->LoadByteCode(&bc);
			if (r < 0)
				TEST_FAILED;
		}

		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test saving/loading bytecode with asBC_ChkNullS
	// http://www.gamedev.net/topic/681386-bytecode-x64-x32/
	{
		engine = asCreateScriptEngine();

		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class Foo {} \n"
			"void func(Foo &, Foo &) {} \n"
			"void main() { \n"
			"  Foo @f; \n"
			"  func(f, f); \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		asIScriptFunction *func = mod->GetFunctionByName("main");
		asBYTE expect[] =
		{
			asBC_SUSPEND,asBC_PshVPtr,asBC_PshVPtr,asBC_ChkNullS,asBC_ChkNullS,asBC_CALL,asBC_SUSPEND,asBC_FREE, asBC_RET
		};
		if (!ValidateByteCode(func, expect))
			TEST_FAILED;

		CBytecodeStream bc("test");
		r = mod->SaveByteCode(&bc);
		if (r < 0)
			TEST_FAILED;

		asDWORD crc32 = ComputeCRC32(&bc.buffer[0], asUINT(bc.buffer.size()));
		if (crc32 != 0x1636A342)
		{
			PRINTF("The saved byte code has different checksum than the expected. Got 0x%X\n", crc32);
			TEST_FAILED;
		}

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&bc);
		if (r < 0)
			TEST_FAILED;
		
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test saving/loading bytecode with asBC_ClrVPtr when the variable is a null pointer
	// http://www.gamedev.net/topic/677759-crash-on-ios-arm64/
	{
		engine = asCreateScriptEngine();

		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		engine->RegisterObjectType("NGUIWidget", sizeof(NGUIWidget), asOBJ_REF | asOBJ_NOCOUNT);
		engine->RegisterObjectProperty("NGUIWidget", "float alpha", asOFFSET(NGUIWidget, alpha));


		engine->RegisterObjectType("NGUISymbol", sizeof(NGUISymbol), asOBJ_REF | asOBJ_NOCOUNT);
		engine->RegisterObjectMethod("NGUISymbol", "NGUIWidget@ opImplCast()", asFUNCTION(NGUIWidgetCastGeneric), asCALL_GENERIC);
		engine->RegisterObjectProperty("NGUISymbol", "float alpha", asOFFSET(NGUISymbol, alpha));


		engine->RegisterGlobalProperty("NGUISymbol inst", &symbol_inst);


		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"void main() \n"
			"{ \n"
			"	NGUIWidget@ wgt = @inst; \n"
			"	wgt.alpha = 0.58f; \n"
			"}\n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;
		if (bout.buffer != "")
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		CBytecodeStream bc("blah");
		r = mod->SaveByteCode(&bc);
		if (r < 0)
			TEST_FAILED;

		asDWORD crc = ComputeCRC32(&bc.buffer[0], asUINT(bc.buffer.size()));
		if (crc != 710615252u)
		{
			PRINTF("Wrong checksum. Got %u\n", crc);
			TEST_FAILED;
		}

		r = mod->LoadByteCode(&bc);
		if (r < 0)
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if (r != asEXECUTION_FINISHED)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test saving bytecode where indirectly defined funcdefs are used
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterScriptHandle(engine);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"namespace A { void func() {} }\n"
			"namespace B { int func(int) { return 0; } }\n"
			"void main() \n"
			"{ \n"
			"  ref @r1 = A::func; \n"
			"  ref @r2 = B::func; \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		CBytecodeStream bc("blah");
		r = mod->SaveByteCode(&bc);
		if (r < 0)
			TEST_FAILED;

		r = mod->LoadByteCode(&bc);
		if (r < 0)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test problem with scripts calling constructor with value type passed before reference
	// http://www.gamedev.net/topic/671244-error-when-saving-bytecode-on-x64/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		// Turn off bytecode optimization to guarantee the scenario we're testing occurs
		engine->SetEngineProperty(asEP_OPTIMIZE_BYTECODE, false);

		RegisterStdString(engine);

		// Register additional constructor for test
		r = engine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT, "void f(int, const string& in)", asFUNCTION(0), asCALL_GENERIC); assert(r >= 0);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"int main() \n"
			"{ \n"
			"  string s(1, ''); \n"
			"  return 0; \n"
			"} \n");
		r = mod->Build();
		if( r < 0 ) 
			TEST_FAILED;

		CBytecodeStream bcStream("blah");
		r = mod->SaveByteCode(&bcStream);
		if( r < 0 )
			TEST_FAILED;

		asIScriptFunction *func = mod->GetFunctionByName("main");
		asBYTE expect[] = 
			{
				// TODO: runtime optimize: Repeated JitEntry, SUSPEND should be removed
				asBC_JitEntry,asBC_SUSPEND,asBC_JitEntry,asBC_SUSPEND,asBC_JitEntry,asBC_PGA,asBC_SetV4,asBC_PshV4,asBC_PSF,asBC_CALLSYS,asBC_JitEntry,
				asBC_SUSPEND,asBC_JitEntry,asBC_SetV4,asBC_PSF,asBC_CALLSYS,asBC_JitEntry,asBC_CpyVtoR4,asBC_JMP,asBC_RET
			};
		if( !ValidateByteCode(func, expect) )
			TEST_FAILED;

		mod = engine->GetModule("test2", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&bcStream);
		if( r < 0 )
			TEST_FAILED;

		func = mod->GetFunctionByName("main");
		if( !ValidateByteCode(func, expect) )
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test problem with saving/loading bytecode containing templates with multiple subtypes
	// Reported by Phong Ba
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		r = engine->RegisterObjectType("dictionary<class K, class V>", 0, asOBJ_REF | asOBJ_TEMPLATE); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("dictionary<K, V>", asBEHAVE_FACTORY, "dictionary<K, V>@ f(int&in)", asFUNCTION(0), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("dictionary<K, V>", asBEHAVE_ADDREF, "void f()", asFUNCTION(0), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("dictionary<K, V>", asBEHAVE_RELEASE, "void f()", asFUNCTION(0), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterObjectMethod("dictionary<K, V>", "int get_Count() const property", asFUNCTION(0), asCALL_GENERIC); assert(r >= 0);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", "void main(){ dictionary<int,int> d; int cnt = d.Count; }");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		CBytecodeStream stream(__FILE__"1");
		r = mod->SaveByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->ShutDownAndRelease();
	}

	// Test WriteConfigToStream/ConfigEngineFromStream with template types and dependencies
	// http://www.gamedev.net/topic/664405-scripthelper-config-helpers-not-working-correctly/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		// Register types with dependencies
		r = engine->RegisterObjectType("type", 0, asOBJ_REF|asOBJ_NOCOUNT); assert( r >= 0 );
		RegisterScriptArray(engine, false);
		r = engine->RegisterObjectBehaviour("type", asBEHAVE_FACTORY, "type @f()", asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectMethod("type", "void func(array<int> @+)", asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );

		stringstream s;
		r = WriteConfigToStream(engine, s);
		if( r < 0 )
			TEST_FAILED;

		engine->ShutDownAndRelease();

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		r = ConfigEngineFromStream(engine, s);
		if( r < 0 )
			TEST_FAILED;

		mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"void main() { \n"
			"  array<int> a = {1,2,3}; \n"
			"  type t; \n"
			"  t.func(a); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		engine->ShutDownAndRelease();

		if( bout.buffer != "config (56, 0) : Warning : Cannot register template callback without the actual implementation\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	}

	// Test WriteConfigToStream/ConfigEngineFromStream with namespaces
	// http://www.gamedev.net/topic/664405-scripthelper-config-helpers-not-working-correctly/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		r = engine->SetDefaultNamespace("test::sub"); assert( r >= 0 );
		r = engine->RegisterObjectType("foo", 0, asOBJ_REF | asOBJ_NOCOUNT); assert( r >= 0 );
		r = engine->SetDefaultNamespace("blah"); assert( r >= 0 );
		r = engine->RegisterGlobalFunction("test::sub::foo @bar(test::sub::foo @)", asFUNCTION(0), asCALL_GENERIC); assert( r >= 0 );
		r = engine->SetDefaultNamespace(""); assert( r >= 0 );
		r = engine->RegisterGlobalProperty("test::sub::foo @g", (void*)1); assert( r >= 0 );

		stringstream s;
		r = WriteConfigToStream(engine, s);
		if( r < 0 )
			TEST_FAILED;

		// skip the first two lines, since they vary depending on library version, platform, and compilation mode
		string output = s.str();
		size_t pos = output.find('\n');
		pos = output.find('\n', pos+1);
		output = output.substr(pos+1);

		if( output != 
					"// Engine properties\n"
					"ep 0 0\n"
					"ep 1 0\n"
					"ep 2 1\n"
					"ep 3 1\n"
					"ep 4 0\n"
					"ep 5 0\n"
					"ep 6 0\n"
					"ep 7 0\n"
					"ep 8 0\n"
					"ep 9 1\n"
					"ep 10 0\n"
					"ep 11 1\n"
					"ep 12 0\n"
					"ep 13 0\n"
					"ep 14 3\n" // asEP_PROPERTY_ACCESSOR_MODE
					"ep 15 0\n"
					"ep 16 1\n"
					"ep 17 0\n"
					"ep 18 0\n"
					"ep 19 1\n"
					"ep 20 0\n"
					"ep 21 0\n"
					"ep 22 0\n"
					"ep 23 0\n"
					"ep 24 0\n"
					"ep 25 0\n"
					"ep 26 1\n"
					"ep 27 100\n"
					"ep 28 1\n"
					"ep 29 4096\n"
					"ep 30 10\n"
					"ep 31 0\n"
					"\n"
					"// Enums\n"
					"\n"
					"// Types\n"
					"access ffffffff\n"
					"namespace \"test::sub\"\n"
					"objtype \"foo\" 262145\n"
					"\n"
					"// Template type members\n"
					"\n"
					"// Type members\n"
					"\n"
					"// Functions\n"
					"namespace \"blah\"\n"
					"func \"test::sub::foo@ bar(test::sub::foo@)\"\n"
					"\n"
					"// Properties\n"
					"namespace \"\"\n"
					"prop \"test::sub::foo@ g\"\n"
					"\n"
					"// String factory\n"
					"\n"
					"// Default array type\n" )
		{
			PRINTF("%s", output.c_str());
			TEST_FAILED;
		}

		engine->Release();

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		r = ConfigEngineFromStream(engine, s);
		if( r < 0 )
			TEST_FAILED;

		stringstream s2;
		r = WriteConfigToStream(engine, s2);
		if( r < 0 )
			TEST_FAILED;

		// Take out the engine property asEP_INIT_GLOBAL_VARS_AFTER_BUILD before 
		// comparison, since it will not be maintained by the WriteConfigFromEngine
		output = s.str();
		string output2 = s2.str();
		pos = output.find("ep 9");
		output = output.substr(0, pos) + output.substr(pos+7);
		output2 = output2.substr(0, pos) + output2.substr(pos+7);

		if( output != output2 )
		{
			PRINTF("%s", s2.str().c_str());
			TEST_FAILED;
		}
		
		engine->Release();
	}

	// Test save/load with funcdef and imported functions
	// http://www.gamedev.net/topic/657621-using-global-funcdef-setter-with-imported-function-gives-assert-or-invalid-bytecode/
	SKIP_ON_MAX_PORT
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		struct T
		{
			static void set_funcdef_var(asIScriptFunction*f) { if( g_func ) g_func->Release(); g_func = f; }
		};

		const char *script =
			"import void bar() from \"somewhere\";"
			"void test1(){ @foo = test2; }"
			"void test2(){ @foo = bar;   }";

		engine->RegisterFuncdef( "void MyVoid()" );
		engine->RegisterGlobalFunction( "void set_foo(MyVoid@) property", asFUNCTION(T::set_funcdef_var), asCALL_CDECL );

		asIScriptModule* module = engine->GetModule( "script", asGM_ALWAYS_CREATE );
		module->AddScriptSection( "script", script );
		if( module->Build() < 0 ) // assert (-DNDEBUG not present)
			TEST_FAILED;

		r = ExecuteString(engine, "test2()", module);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		if( g_func == 0 || string(g_func->GetName()) != "bar" )
			TEST_FAILED;

		asIScriptContext *ctx = engine->CreateContext();
		ctx->Prepare(g_func);
		r = ctx->Execute();
		if( r != asEXECUTION_EXCEPTION ) // should fail since the imported function is not bound
			TEST_FAILED;
		if( string(ctx->GetExceptionString()) == "Unbound function called" )
		ctx->Release();

		CBytecodeStream bytecode("");
		if( module->SaveByteCode( &bytecode ) < 0 )
			TEST_FAILED;

		asIScriptModule* module_bytecode = engine->GetModule( "script_bytecode", asGM_ALWAYS_CREATE );
		if( module_bytecode->LoadByteCode( &bytecode ) < 0 ) // error (-DNDEBUG present)
			TEST_FAILED;

		if( g_func )
		{
			g_func->Release();
			g_func = 0;
		}

		r = ExecuteString(engine, "test2()", module);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		if( g_func == 0 || string(g_func->GetName()) != "bar" )
			TEST_FAILED;

		if( g_func )
		{
			g_func->Release();
			g_func = 0;
		}

		engine->Release();
	}

	// Test saving and loading with template in a namespace
	// http://www.gamedev.net/topic/658862-loading-bytecode-bug/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->SetDefaultNamespace( "reflection" );
		engine->RegisterObjectType( "type", sizeof( 4 ), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS );
		engine->RegisterObjectType( "typeof<class T>", 0, asOBJ_REF | asOBJ_TEMPLATE | asOBJ_NOCOUNT);
		engine->RegisterObjectBehaviour("typeof<T>", asBEHAVE_FACTORY, "typeof<T> @f(int&in)", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("typeof<T>", "type opImplConv()", asFUNCTION(0), asCALL_GENERIC);
		engine->SetDefaultNamespace( "" );

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class A {} \n"
			"void func() { \n"
			"  reflection::type t = reflection::typeof<A>(); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		CBytecodeStream stream(__FILE__"1");
		mod->SaveByteCode(&stream);

		if( mod->LoadByteCode(&stream) != 0 )
			TEST_FAILED;
		
		engine->Release();
	}

	// Test loading bytecode that tries to access objects that don't exist
	// This test is designed to fail loading when loading the bytecode for 
	// the a function, thus testing that the clean up is appropriate
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->SetDefaultNamespace( "reflection" );
		engine->RegisterObjectType( "type", sizeof( 4 ), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS );
		engine->RegisterObjectType( "typeof<class T>", 0, asOBJ_REF | asOBJ_TEMPLATE | asOBJ_NOCOUNT);
		engine->RegisterObjectBehaviour("typeof<T>", asBEHAVE_FACTORY, "typeof<T> @f(int&in)", asFUNCTION(0), asCALL_GENERIC);
		engine->RegisterObjectMethod("typeof<T>", "type opImplConv()", asFUNCTION(0), asCALL_GENERIC);
		engine->SetDefaultNamespace( "" );

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class A {} \n"
			"void func() { \n"
			"  reflection::type t = reflection::typeof<A>(); \n"
			"} \n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		CBytecodeStream stream(__FILE__"1");
		mod->SaveByteCode(&stream);

		engine->Release();

		// Recreate the engine without the template type
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";
		engine->SetDefaultNamespace( "reflection" );
		engine->RegisterObjectType( "type", sizeof( 4 ), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS );
//		engine->RegisterObjectType( "typeof<class T>", 0, asOBJ_REF | asOBJ_TEMPLATE | asOBJ_NOCOUNT);
//		engine->RegisterObjectBehaviour("typeof<T>", asBEHAVE_FACTORY, "typeof<T> @f(int&in)", asFUNCTION(0), asCALL_GENERIC);
//		engine->RegisterObjectMethod("typeof<T>", "type opImplConv()", asFUNCTION(0), asCALL_GENERIC);
		engine->SetDefaultNamespace( "" );

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		if( mod->LoadByteCode(&stream) >= 0 )
			TEST_FAILED;
		
		if( bout.buffer != " (0, 0) : Error   : Template type 'typeof' doesn't exist\n"
						   " (0, 0) : Error   : LoadByteCode failed. The bytecode is invalid. Number of bytes read from stream: 120\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test saving and loading script with string literal
	{
		// Write the configuration to stream
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);
		stringstream strm;
		WriteConfigToStream(engine, strm);
		engine->Release();

		// Configure engine from stream and compile the script to bytecode
		strm.seekp(0);
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		ConfigEngineFromStream(engine, strm, "test", &stringFactory);

		const char *script = 
			"void func() { \n"
			"  'test'; \n"
			"  return; \n"
			"} \n";

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		CBytecodeStream stream(__FILE__"1");
		r = mod->SaveByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;
		engine->Release();

		// Load the bytecode
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);

		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);
		if( mod->LoadByteCode(&stream) != 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Test saving and loading script with array of classes initialized from initialization list
	if( !strstr(asGetLibraryOptions(), "AS_NO_MEMBER_INIT") )
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, true);

		const char *script = 
			"array<A@> g_a = {A()}; \n"
			"class A {}             \n";

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(":1", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		CBytecodeStream stream(__FILE__"1");
		mod->SaveByteCode(&stream);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		if( mod->LoadByteCode(&stream) != 0 )
			TEST_FAILED;

		// The garbage collector must not complain about not being able to release objects
		engine->Release();
	}

	// Test repeated save/loads with shared interfaces and funcdefs
	// http://www.gamedev.net/topic/656784-wrong-bytecode-with-funcdef-in-shared-interface/
	{
		CBytecodeStream stream1(__FILE__"shared1");
		CBytecodeStream stream2(__FILE__"shared2");
		CBytecodeStream stream3(__FILE__"shared1");
		CBytecodeStream stream4(__FILE__"shared2");

		{
			engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
			engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

			asIScriptModule *mod1 = engine->GetModule("1", asGM_ALWAYS_CREATE);
			mod1->AddScriptSection("test",
				"funcdef void CALLBACK(); \n");
			r = mod1->Build();
			if( r < 0 )
				TEST_FAILED;

			asIScriptModule *mod2 = engine->GetModule("2", asGM_ALWAYS_CREATE);
			mod2->AddScriptSection("test",
				"funcdef void CALLBACK(); \n"
				"void Foo1(CALLBACK@){} \n"
				"void Foo2(){Foo1(null);} \n");
			r = mod2->Build();
			if( r < 0 )
				TEST_FAILED;

			r = mod1->SaveByteCode(&stream1);
			if( r < 0 )
				TEST_FAILED;

			r = mod2->SaveByteCode(&stream2);
			if( r < 0 )
				TEST_FAILED;

			engine->Release();
		}

		asDWORD crc1 = ComputeCRC32(&stream2.buffer[0], asUINT(stream2.buffer.size()));

		{
			engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
			engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

			asIScriptModule *mod1 = engine->GetModule("1", asGM_ALWAYS_CREATE);
			r = mod1->LoadByteCode(&stream1);
			if( r < 0 )
				TEST_FAILED;

			asIScriptModule *mod2 = engine->GetModule("2", asGM_ALWAYS_CREATE);
			r = mod2->LoadByteCode(&stream2);
			if( r < 0 )
				TEST_FAILED;

			r = mod1->SaveByteCode(&stream3);
			if( r < 0 )
				TEST_FAILED;

			r = mod2->SaveByteCode(&stream4);
			if( r < 0 )
				TEST_FAILED;

			engine->Release();
		}

		asDWORD crc2 = ComputeCRC32(&stream4.buffer[0], asUINT(stream4.buffer.size()));

		if( crc1 != crc2 )
			TEST_FAILED;

		if( stream4.buffer.size() == stream2.buffer.size() )
		{
			for( size_t b = 0; b < stream4.buffer.size(); ++b )
				if( stream4.buffer[b] != stream2.buffer[b] )
					PRINTF("streams differ on byte %d\n", (int)b);
		}
		else
			PRINTF("streams differ in size\n");

		{
			engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
			engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

			asIScriptModule *mod1 = engine->GetModule("1", asGM_ALWAYS_CREATE);
			r = mod1->LoadByteCode(&stream3);
			if( r < 0 )
				TEST_FAILED;

			asIScriptModule *mod2 = engine->GetModule("2", asGM_ALWAYS_CREATE);
			r = mod2->LoadByteCode(&stream4);
			if( r < 0 )
				TEST_FAILED;

			engine->Release();
		}
	}

	// Test multiple modules with shared enums and shared classes
	// http://www.gamedev.net/topic/632922-huge-problems-with-precompilde-byte-code/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"shared enum ResourceType {} \n"
			"shared class Resource \n"
			"{ \n"
			"	void getType(ResourceType) {} \n"
			"} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		CBytecodeStream stream(__FILE__"shared");
		r = mod->SaveByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		mod = engine->GetModule("2", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Test problem reported by Andre Santee
	// http://www.gamedev.net/topic/635623-assertion-failed-while-using-function-handles/
	if( !strstr(asGetLibraryOptions(), "AS_NO_MEMBER_INIT") )
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"class Foo \n"
			"{ \n"
			"    int a = 42; \n"
			"} \n"
			"class Bar \n"
			"{ \n"
			"    float b = 3.14f; \n"
			"} \n"
			"funcdef void TEST_FUNC_HANDLE(Foo, Bar); \n"
			"void testFunction(TEST_FUNC_HANDLE@ func) \n"
			"{ \n"
			"    func(Foo(), Bar()); \n"
			"} \n"
			"void callback(Foo f, Bar b) \n"
			"{ \n"
			"  assert( f.a == 42 && b.b == 3.14f ); \n"
			"  called = true; \n"
			"} \n"
			"bool called = false;\n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		CBytecodeStream stream(__FILE__"shared");
		r = mod->SaveByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		mod = engine->GetModule("2", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "testFunction(callback); assert( called );", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}


	Test2();
	TestAndrewPrice();


	// Test saving/loading with array of function pointers
	// http://www.gamedev.net/topic/627737-bytecode-loading-error/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, false);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef void F(); \n"
			"array<F@> arr = { f }; \n"
			"void f() {} \n");

		r = mod->Build(); 
		if( r < 0 )
			TEST_FAILED;
		
		CBytecodeStream stream(__FILE__"1");
		
		r = mod->SaveByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;
		
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream); 
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Test shared interface with function pointers
	// http://www.gamedev.net/topic/639243-funcdef-inside-shared-interface-interface-already-implement-warning/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test",
			"funcdef void fdef(); \n"
			"shared interface iface \n"
			"{ \n"
			"	fdef@ dummy(); \n"
			"} \n");

		r = mod->Build(); 
		if( r < 0 )
			TEST_FAILED;
		
		CBytecodeStream stream(__FILE__"1");
		
		r = mod->SaveByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;
		
		engine->Release();

		// Load the bytecode in two different modules
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

		stream.Restart();
		mod = engine->GetModule("A", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		stream.Restart();
		mod = engine->GetModule("B", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		engine->Release();

		// Load the bytecode twice, replacing the module
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

		stream.Restart();
		mod = engine->GetModule("A", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		stream.Restart();
		mod = engine->GetModule("A", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	if( !strstr(asGetLibraryOptions(), "AS_NO_MEMBER_INIT") )
	{
		engine = ConfigureEngine(0);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(":1", g_script1);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		// Validate the number of global functions
		if( mod->GetFunctionCount() != 6 )
			TEST_FAILED;

		mod = engine->GetModule("DynamicModule", asGM_ALWAYS_CREATE);
		mod->AddScriptSection(":2", g_script2);
		mod->Build();

		TestScripts(engine);
		asUINT currentSize, totalDestroyed, totalDetected;
		engine->GetGCStatistics(&currentSize, &totalDestroyed, &totalDetected);

		// Save the compiled byte code
		CBytecodeStream stream(__FILE__"1");
		CBytecodeStream stream2(__FILE__"2");
		mod = engine->GetModule(0);
		mod->SaveByteCode(&stream);
		mod->SaveByteCode(&stream2, true);

#ifndef STREAM_TO_FILE
		if (stream.buffer.size() != 2115)
			PRINTF("The saved byte code is not of the expected size. It is %d bytes\n", (int)stream.buffer.size());
		asUINT zeroes = stream.CountZeroes();
		if (zeroes != 499)
		{
			PRINTF("The saved byte code contains a different amount of zeroes than the expected. Counted %d\n", zeroes);
			// Mac OS X PPC has more zeroes, probably due to the bool type being 4 bytes
		}
		asDWORD crc32 = ComputeCRC32(&stream.buffer[0], asUINT(stream.buffer.size()));
		if( crc32 != 0x7983F177)
		{
			PRINTF("The saved byte code has different checksum than the expected. Got 0x%X\n", crc32);
			TEST_FAILED;
		}

		// Without debug info
		if (stream2.buffer.size() != 1755)
			PRINTF("The saved byte code without debug info is not of the expected size. It is %d bytes\n", (int)stream2.buffer.size());
		zeroes = stream2.CountZeroes();
		if (zeroes != 389)
			PRINTF("The saved byte code without debug info contains a different amount of zeroes than the expected. Counted %d\n", zeroes);
#endif
		// Test loading without releasing the engine first
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		if( mod->LoadByteCode(&stream) != 0 )
			TEST_FAILED;

		if( mod->GetFunctionCount() != 6 )
			TEST_FAILED;
		else if( string(mod->GetFunctionByIndex(0)->GetScriptSectionName()) != ":1" )
			TEST_FAILED;

		// Make sure the parameter names were loaded
		const char *paramName;
		mod->GetFunctionByName("func")->GetParam(1, 0, 0, &paramName);
		if( paramName == 0 || string(paramName) != "f" )
			TEST_FAILED;

		mod = engine->GetModule("DynamicModule", asGM_ALWAYS_CREATE);
		mod->AddScriptSection(":2", g_script2);
		mod->Build();

		TestScripts(engine);

		// Test loading for a new engine
		GlobalCharArray->Release();
		GlobalCharArray = 0;

		engine->Release();
		engine = ConfigureEngine(1);

		stream2.Restart();
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->LoadByteCode(&stream2);

		if( mod->GetFunctionCount() != 6 )
			TEST_FAILED;

		// Make sure the parameter names were not loaded
		mod->GetFunctionByName("func")->GetParam(1, 0, 0, &paramName);
		if( paramName != 0 )
			TEST_FAILED;

		mod = engine->GetModule("DynamicModule", asGM_ALWAYS_CREATE);
		mod->AddScriptSection(":2", g_script2);
		mod->Build();

		TestScripts(engine);
		asUINT currentSize2, totalDestroyed2, totalDetected2;
		engine->GetGCStatistics(&currentSize2, &totalDestroyed2, &totalDetected2);
		if( currentSize != currentSize2 ||
			totalDestroyed != totalDestroyed2 ||
			totalDetected != totalDetected2 )
			TEST_FAILED;

		GlobalCharArray->Release();
		GlobalCharArray = 0;
		engine->Release();

		//---------------------------------------
		// A tiny file for comparison
#ifndef STREAM_TO_FILE
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", "void f() {}");
		mod->Build();
		CBytecodeStream streamTiny(__FILE__"tiny");
		mod->SaveByteCode(&streamTiny, true);
		engine->Release();

		asBYTE expected[] = {0x01,0x00,0x00,0x00,0x00,0x00,0x01,0x66,0x02,0x66,0x00,0x40,0x50,0x00,0x00,0x01,0x00,0x00,0x00,0x02,0x3F,0x0A,0x00,0x00,0x01,0x72,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
		bool match = true;
		for( asUINT n = 0; n < streamTiny.buffer.size(); n++ )
			if( streamTiny.buffer[n] != expected[n] )
			{
				match = false;
				break;
			}
		if( !match )
		{
			PRINTF("Tiny module gave a different result than expected:\n");
			PRINTF("got     : ");
			for( asUINT n = 0; n < streamTiny.buffer.size(); n++ )
				PRINTF("%0.2X", streamTiny.buffer[n]);
			PRINTF("\n");
			PRINTF("expected: ");
			for( asUINT m = 0; m < sizeof(expected); m++ )
				PRINTF("%0.2X", expected[m]);
			PRINTF("\n");
			TEST_FAILED;
		}
#endif
	}

	// Test saving/loading global variable of registered value type
	// http://www.gamedev.net/topic/638529-wrong-function-called-on-bytecode-restoration/
	SKIP_ON_MAX_PORT
	{
		struct A
		{
			static void Construct1(int *a) { *a = 1; }
			static void Construct2(int *a) { *a = 2; }
		};

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->RegisterObjectType("A", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
		engine->RegisterObjectBehaviour("A", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(A::Construct1), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectProperty("A", "int val", 0);

		engine->RegisterObjectType("B", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
		engine->RegisterObjectBehaviour("B", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(A::Construct2), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectProperty("B", "int val", 0);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("A", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", "A a; B b;");
		r = mod->Build();
		if( r != 0 )
			TEST_FAILED;

		CBytecodeStream stream2(__FILE__"2");
		mod->SaveByteCode(&stream2);

		engine->Release();

		// Register the types in a different order this time
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		engine->RegisterObjectType("B", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
		engine->RegisterObjectBehaviour("B", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(A::Construct2), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectProperty("B", "int val", 0);

		engine->RegisterObjectType("A", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
		engine->RegisterObjectBehaviour("A", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(A::Construct1), asCALL_CDECL_OBJLAST);
		engine->RegisterObjectProperty("A", "int val", 0);

		mod = engine->GetModule("A", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream2);
		if( r != 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "assert( a.val == 1 ); \n"
								  "assert( b.val == 2 ); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	//-----------------------------------------
	// Saving bytecode for a module that failed to compile shouldn't be allowed
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script3", g_script3);
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;
		CBytecodeStream stream2(__FILE__"2");
		r = mod->SaveByteCode(&stream2);
		if( r >= 0 )
			TEST_FAILED;

		engine->Release();
	}

	//-----------------------------------
	// save/load with overloaded array types should work as well
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		RegisterScriptArray(engine, true);
		r = engine->RegisterObjectType("float[]", sizeof(vector<float>), asOBJ_VALUE | asOBJ_APP_CLASS_CDA); assert(r >= 0);
#ifndef AS_MAX_PORTABILITY
		r = engine->RegisterObjectBehaviour("float[]", asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(ConstructFloatArray, (vector<float> *), void), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("float[]", asBEHAVE_CONSTRUCT, "void f(int)", asFUNCTIONPR(ConstructFloatArray, (int, vector<float> *), void), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("float[]", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DestructFloatArray), asCALL_CDECL_OBJLAST); assert(r >= 0);
		r = engine->RegisterObjectMethod("float[]", "float[] &opAssign(float[]&in)", asMETHODPR(vector<float>, operator=, (const std::vector<float> &), vector<float>&), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("float[]", "float &opIndex(int)", asMETHODPR(vector<float>, operator[], (vector<float>::size_type), float &), asCALL_THISCALL); assert(r >= 0);
		r = engine->RegisterObjectMethod("float[]", "int length()", asMETHOD(vector<float>, size), asCALL_THISCALL); assert(r >= 0);
#else
		r = engine->RegisterObjectBehaviour("float[]", asBEHAVE_CONSTRUCT, "void f()", WRAP_OBJ_LAST_PR(ConstructFloatArray, (vector<float> *), void), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("float[]", asBEHAVE_CONSTRUCT, "void f(int)", WRAP_OBJ_LAST_PR(ConstructFloatArray, (int, vector<float> *), void), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterObjectBehaviour("float[]", asBEHAVE_DESTRUCT, "void f()", WRAP_OBJ_LAST(DestructFloatArray), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterObjectMethod("float[]", "float[] &opAssign(float[]&in)", WRAP_MFN_PR(vector<float>, operator=, (const std::vector<float> &), vector<float>&), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterObjectMethod("float[]", "float &opIndex(int)", WRAP_MFN_PR(vector<float>, operator[], (vector<float>::size_type), float &), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterObjectMethod("float[]", "int length()", WRAP_MFN(vector<float>, size), asCALL_GENERIC); assert(r >= 0);
#endif

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script3", g_script3);
		mod->Build();
		
		CBytecodeStream stream3(__FILE__"3");
		mod->SaveByteCode(&stream3);
		
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->LoadByteCode(&stream3);
		ExecuteString(engine, "Test(3)", mod);
		
		engine->Release();
	}

	//------------------------------------
	// Test problem detected by TheAtom
	// http://www.gamedev.net/topic/623170-crash-on-bytecode-loading/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule("0", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("0",
			"shared class T\n"
			"{\n"
			"        void f() { }\n"
			"};\n"
			"shared class T2 : T\n"
			"{\n"
			"};\n"
			"class T3 : T\n"
			"{\n"
			"        void f() { T::f(); }\n"
			"};\n");
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		CBytecodeStream stream(__FILE__"0");
		mod->SaveByteCode(&stream);

		engine->Release();

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule("1", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		stream.Restart();

		mod = engine->GetModule("2", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	//---------------------------------
	// Must be possible to load scripts with classes declared out of order
	// Built-in array types must be able to be declared even though the complete script structure hasn't been loaded yet
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, true);
		RegisterScriptString(engine);
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", g_script4);
		r = mod->Build();
		if( r < 0 ) 
			TEST_FAILED;
		else
		{
			// Test the script with compiled byte code
			asIScriptContext *ctx = engine->CreateContext();
			r = ExecuteString(engine, "g_inGame.Initialize(0);", mod, ctx);
			if( r != asEXECUTION_FINISHED )
			{
				if( r == asEXECUTION_EXCEPTION ) PRINTF("%s", GetExceptionInfo(ctx).c_str());
				TEST_FAILED;
			}
			if( ctx ) ctx->Release();

			// Save the bytecode
			CBytecodeStream stream4(__FILE__"4");
			mod = engine->GetModule(0);
			mod->SaveByteCode(&stream4);
			engine->Release();

			// Now load the bytecode into a fresh engine and test the script again
			engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
			engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
			RegisterScriptArray(engine, true);
			RegisterScriptString(engine);
			mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
			mod->LoadByteCode(&stream4);
			r = ExecuteString(engine, "g_inGame.Initialize(0);", mod);
			if( r != asEXECUTION_FINISHED )
				TEST_FAILED;
		}
		engine->Release();
	}
	//----------------
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, true);
		RegisterScriptString(engine);
		r = engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert( r >= 0 );

		r = engine->RegisterObjectType("IsoSprite", sizeof(int), asOBJ_REF); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("IsoSprite", asBEHAVE_FACTORY, "IsoSprite@ f()", asFUNCTION(IsoSpriteFactory), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("IsoSprite", asBEHAVE_ADDREF, "void f()", asFUNCTION(DummyAddref), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("IsoSprite", asBEHAVE_RELEASE, "void f()", asFUNCTION(DummyRelease), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectMethod("IsoSprite", "IsoSprite &opAssign(const IsoSprite &in)", asFUNCTION(Dummy), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectMethod("IsoSprite", "bool Load(const string &in)", asFUNCTION(Dummy), asCALL_GENERIC); assert( r >= 0 );

		r = engine->RegisterObjectType("IsoMap", sizeof(int), asOBJ_REF); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("IsoMap", asBEHAVE_FACTORY, "IsoMap@ f()", asFUNCTION(IsoSpriteFactory), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("IsoMap", asBEHAVE_ADDREF, "void f()", asFUNCTION(DummyAddref), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("IsoMap", asBEHAVE_RELEASE, "void f()", asFUNCTION(DummyRelease), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectMethod("IsoMap", "IsoMap &opAssign(const IsoMap &in)", asFUNCTION(Dummy), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectMethod("IsoMap", "bool AddEntity(const IsoSprite@+, int col, int row, int layer)", asFUNCTION(Dummy), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectMethod("IsoMap", "bool Load(const string &in)", asFUNCTION(Dummy), asCALL_GENERIC); assert( r >= 0 );

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", g_script5);
		r = mod->Build();
		if( r < 0 ) 
			TEST_FAILED;
		else
		{
			// Test the script with compiled byte code
			asIScriptContext *ctx = engine->CreateContext();
			r = ExecuteString(engine, "Initialize();", mod, ctx);
			if( r != asEXECUTION_FINISHED )
			{
				if( r == asEXECUTION_EXCEPTION ) PRINTF("%s", GetExceptionInfo(ctx).c_str());
				TEST_FAILED;
			}
			if( ctx ) ctx->Release();

			// Save the bytecode
			CBytecodeStream stream(__FILE__"5");
			mod = engine->GetModule(0);
			mod->SaveByteCode(&stream);
			engine->Release();

			// Now load the bytecode into a fresh engine and test the script again
			engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
			engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
			RegisterScriptArray(engine, true);
			RegisterScriptString(engine);
			r = engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert( r >= 0 );

			r = engine->RegisterObjectType("IsoSprite", sizeof(int), asOBJ_REF); assert( r >= 0 );
			r = engine->RegisterObjectBehaviour("IsoSprite", asBEHAVE_FACTORY, "IsoSprite@ f()", asFUNCTION(IsoSpriteFactory), asCALL_GENERIC); assert( r >= 0 );
			r = engine->RegisterObjectBehaviour("IsoSprite", asBEHAVE_ADDREF, "void f()", asFUNCTION(DummyAddref), asCALL_GENERIC); assert( r >= 0 );
			r = engine->RegisterObjectBehaviour("IsoSprite", asBEHAVE_RELEASE, "void f()", asFUNCTION(DummyRelease), asCALL_GENERIC); assert( r >= 0 );
			r = engine->RegisterObjectMethod("IsoSprite", "IsoSprite &opAssign(const IsoSprite &in)", asFUNCTION(Dummy), asCALL_GENERIC); assert( r >= 0 );
			r = engine->RegisterObjectMethod("IsoSprite", "bool Load(const string &in)", asFUNCTION(Dummy), asCALL_GENERIC); assert( r >= 0 );

			r = engine->RegisterObjectType("IsoMap", sizeof(int), asOBJ_REF); assert( r >= 0 );
			r = engine->RegisterObjectBehaviour("IsoMap", asBEHAVE_FACTORY, "IsoMap@ f()", asFUNCTION(IsoMapFactory), asCALL_GENERIC); assert( r >= 0 );
			r = engine->RegisterObjectBehaviour("IsoMap", asBEHAVE_ADDREF, "void f()", asFUNCTION(DummyAddref), asCALL_GENERIC); assert( r >= 0 );
			r = engine->RegisterObjectBehaviour("IsoMap", asBEHAVE_RELEASE, "void f()", asFUNCTION(DummyRelease), asCALL_GENERIC); assert( r >= 0 );
			r = engine->RegisterObjectMethod("IsoMap", "IsoMap &opAssign(const IsoMap &in)", asFUNCTION(Dummy), asCALL_GENERIC); assert( r >= 0 );
			r = engine->RegisterObjectMethod("IsoMap", "bool AddEntity(const IsoSprite@+, int col, int row, int layer)", asFUNCTION(Dummy), asCALL_GENERIC); assert( r >= 0 );
			r = engine->RegisterObjectMethod("IsoMap", "bool Load(const string &in)", asFUNCTION(Dummy), asCALL_GENERIC); assert( r >= 0 );

			mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
			mod->LoadByteCode(&stream);
			r = ExecuteString(engine, "Initialize();", mod);
			if( r != asEXECUTION_FINISHED )
				TEST_FAILED;
		}
		engine->Release();
	}

	//------------------------------
	// Test to make sure the script constants are stored correctly
	{
		const char *script = "void main()                 \n"
		"{                                                \n"
		"	int i = 123;                                  \n"
		"                                                 \n"
		"   output( ' i = (' + i + ')' + 'aaa' + 'bbb' ); \n"
		"}                                                \n";
		
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);
		r = engine->RegisterGlobalFunction("void output(const string &in)", asFUNCTION(output), asCALL_GENERIC); assert( r >= 0 );

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(0, script);
		r = mod->Build();
		
		ExecuteString(engine, "main()", mod);
		if( _out != " i = (123)aaabbb" )
			TEST_FAILED;

		CBytecodeStream stream(__FILE__);
		mod->SaveByteCode(&stream);
		engine->Release();

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);
		r = engine->RegisterGlobalFunction("void output(const string &in)", asFUNCTION(output), asCALL_GENERIC); assert( r >= 0 );
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		
		mod->LoadByteCode(&stream);

		_out = "";
		ExecuteString(engine, "main()", mod);
		if( _out != " i = (123)aaabbb" )
			TEST_FAILED;

		engine->Release();
	}

	//-------------------------------
	// Test that registered template classes are stored correctly
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, false);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		const char *script = 
			"void main() \n"
			"{ \n"
			"	array< int > intArray = {0,1,2}; \n"
			"	uint tmp = intArray.length(); \n"
			"   assert( tmp == 3 ); \n"
			"}; \n";

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(0, script);
		r = mod->Build();
		
		CBytecodeStream stream(__FILE__);
		mod->SaveByteCode(&stream);
		engine->Release();

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, false);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->LoadByteCode(&stream);

		if( ExecuteString(engine, "main()", mod) != asEXECUTION_FINISHED )
			TEST_FAILED;
		
		engine->Release();
	}

	// Test loading script with out of order template declarations 
	{
		const char *script = 
			"class HogeManager \n"
			"{ \n"
			"  array< Hoge >@ hogeArray; \n"
			"} \n"
			"class Hoge {}; \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, false);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(0, script);
		r = mod->Build();
		
		CBytecodeStream stream(__FILE__);
		mod->SaveByteCode(&stream);
		engine->Release();

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, false);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Test loading script with out of order template declarations 
	{
		const char *script = 
			"class HogeManager \n"
			"{ \n"
			"  HogeManager() \n"
			"  { \n"
			"    array< Hoge >@ hogeArray; \n"
			"  } \n"
			"} \n"
			"class Hoge {}; \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, false);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(0, script);
		r = mod->Build();
		
		CBytecodeStream stream(__FILE__);
		mod->SaveByteCode(&stream);
		engine->Release();

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, false);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Test loading byte code that uses the enum types
	{
		const char *script = 
			"array< ColorKind > COLOR_KIND_TABLE = { ColorKind_Red }; \n"
			"enum ColorKind { ColorKind_Red }; \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, false);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(0, script);
		r = mod->Build();
		
		CBytecodeStream stream(__FILE__);
		mod->SaveByteCode(&stream);
		engine->Release();

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, false);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Test loading and executing bytecode
	{
		const char *script = 
			"interface IObj {}; \n"
		    "class Hoge : IObj {}; \n"
		    "void main(int a = 0) \n"
		    "{ \n"
		    "    Hoge h; \n"
		    "    IObj@ objHandle = h; \n"
		    "    Hoge@ hogeHandle = cast< Hoge@ >( objHandle ); \n"
		    "}; \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(0, script);
		r = mod->Build();
		
		CBytecodeStream stream(__FILE__);
		mod->SaveByteCode(&stream);
		engine->Release();

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test that property offsets are properly mapped
	{
		asQWORD test;

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterObjectType("test", 8, asOBJ_VALUE | asOBJ_POD);
		engine->RegisterObjectProperty("test", "int a", 0);
		engine->RegisterObjectProperty("test", "int b", 4);

		engine->RegisterGlobalProperty("test t", &test);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(0, "void func() { t.a = 1; t.b = 2; }");
		mod->Build();

		r = ExecuteString(engine, "func()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		if( *(int*)(&test) != 1 || *((int*)(&test)+1) != 2 )
			TEST_FAILED;

		CBytecodeStream stream(__FILE__);
		mod->SaveByteCode(&stream);
		engine->Release();

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		engine->RegisterObjectType("test", 8, asOBJ_VALUE | asOBJ_POD);
		engine->RegisterObjectProperty("test", "int a", 4); // Switch order of the properties
		engine->RegisterObjectProperty("test", "int b", 0);

		engine->RegisterGlobalProperty("test t", &test);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "func()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		if( *(int*)(&test) != 2 || *((int*)(&test)+1) != 1 )
			TEST_FAILED;

		engine->Release();
	}

	// Test that value types are adjusted for different sizes
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		engine->RegisterObjectType("test", 4, asOBJ_VALUE | asOBJ_POD);
		engine->RegisterObjectProperty("test", "int16 a", 0);
		engine->RegisterObjectProperty("test", "int16 b", 2);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(0, 
			"void func() { int a = 1; test b; int c = 2; b.a = a; b.b = c; check(b); } \n"
			"void check(test t) { assert( t.a == 1 ); \n assert( t.b == 2 ); \n } \n");
		mod->Build();

		r = ExecuteString(engine, "func()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		CBytecodeStream stream(__FILE__);
		mod->SaveByteCode(&stream);
		engine->Release();

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		engine->RegisterObjectType("test", 8, asOBJ_VALUE | asOBJ_POD); // Different size
		engine->RegisterObjectProperty("test", "int16 a", 4); // Switch order of the properties
		engine->RegisterObjectProperty("test", "int16 b", 0);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "func()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test loading and executing bytecode
	{
		const char *script = 
			"interface ITest\n"
			"{\n"
			"}\n"
			"class Test : ITest\n"
			"{\n"
			"	ITest@[] arr;\n"
			"	void Set(ITest@ e)\n"
			"	{\n"
			"		arr.resize(1);\n"
			"		@arr[0]=e;\n"
			"	}\n"
			"}\n"
			"void main()\n"
			"{\n"
			"	Test@ t=Test();\n"
			"	t.Set(t);\n"
			"}\n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, true);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection(0, script);
		r = mod->Build();
		
		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		CBytecodeStream stream(__FILE__);
		mod->SaveByteCode(&stream);
		engine->Release();

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, true);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Test 
	{
		CBytecodeStream stream(__FILE__"1");

		const char *script = 
			"interface ITest1 { } \n"
			"interface ITest2 { } \n"
			" \n"
			"CTest@[] Array1; \n"
			" \n"
			"class CTest : ITest1 \n"
			"{ \n"
			"	CTest() \n"
			"	{ \n"
			"		Index=0; \n"
			"		@Field=null; \n"
			"	} \n"
			" \n"
			"	int Index; \n"
			"	ITest2@ Field; \n"
			"} \n"
			" \n"
			"int GetTheIndex() \n"
			"{ \n"
			"  return Array1[0].Index; \n"
			"} \n"
			" \n"
			"void Test() \n"
			"{ \n"
			"  Array1.resize(1); \n"
			"  CTest test(); \n"
			"  @Array1[0] = test; \n"
			"  GetTheIndex(); \n"
			"} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, true);

		mod = engine->GetModule("1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection(0, script);
		r = mod->Build();

		r = ExecuteString(engine, "Test()", mod);

		mod->SaveByteCode(&stream);
		engine->Release();

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterScriptArray(engine, true);

		mod = engine->GetModule("1", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "Test()", mod);

		engine->Release();
	}

	// Test two modules with same interface
	{
		CBytecodeStream stream(__FILE__"1");

		const char *script = 
			"interface ITest \n"
			"{ \n"
			"  ITest@ test(); \n"
			"} \n"
			"class CTest : ITest \n"
			"{ \n"
			"  ITest@ test() \n"
			"  { \n"
			"    return this; \n"
			"  } \n"
			"} \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule("1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection(0, script);
		r = mod->Build();

		mod = engine->GetModule("2", asGM_ALWAYS_CREATE);
		mod->AddScriptSection(0, script);
		r = mod->Build();

		mod->SaveByteCode(&stream);
		engine->Release();

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		mod = engine->GetModule("1", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		stream.Restart();
		mod = engine->GetModule("2", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Test loading bytecode, where the code uses a template instance that the template callback doesn't allow
	// The loading of the bytecode must fail graciously in this case, and display intelligent error message to 
	// allow the script writer to find the error in the original code.
	{
		CBytecodeStream stream(__FILE__"1");

		const char *script = 
			"tmpl<int> t; \n";

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		r = engine->RegisterObjectType("tmpl<class T>", 0, asOBJ_REF | asOBJ_TEMPLATE); assert( r >= 0 );
#ifndef AS_MAX_PORTABILITY
		r = engine->RegisterObjectBehaviour("tmpl<T>", asBEHAVE_FACTORY, "tmpl<T>@ f(int&in)", asFUNCTIONPR(Tmpl::TmplFactory, (asITypeInfo*), Tmpl*), asCALL_CDECL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("tmpl<T>", asBEHAVE_ADDREF, "void f()", asMETHOD(Tmpl,AddRef), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("tmpl<T>", asBEHAVE_RELEASE, "void f()", asMETHOD(Tmpl,Release), asCALL_THISCALL); assert( r >= 0 );
#else
		r = engine->RegisterObjectBehaviour("tmpl<T>", asBEHAVE_FACTORY, "tmpl<T>@ f(int&in)", WRAP_FN_PR(Tmpl::TmplFactory, (asITypeInfo*), Tmpl*), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("tmpl<T>", asBEHAVE_ADDREF, "void f()", WRAP_MFN(Tmpl,AddRef), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("tmpl<T>", asBEHAVE_RELEASE, "void f()", WRAP_MFN(Tmpl,Release), asCALL_GENERIC); assert( r >= 0 );
#endif
		//r = engine->RegisterObjectBehaviour("tmpl<T>", asBEHAVE_TEMPLATE_CALLBACK, "bool f(int&in)", asFUNCTION(Tmpl::TmplCallback), asCALL_CDECL); assert( r >= 0 );

		mod = engine->GetModule("1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection(0, script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		mod->SaveByteCode(&stream);
		engine->Release();

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		bout.buffer = "";

		r = engine->RegisterObjectType("tmpl<class T>", 0, asOBJ_REF | asOBJ_TEMPLATE); assert( r >= 0 );
#ifndef AS_MAX_PORTABILITY
		r = engine->RegisterObjectBehaviour("tmpl<T>", asBEHAVE_FACTORY, "tmpl<T>@ f(int&in)", asFUNCTIONPR(Tmpl::TmplFactory, (asITypeInfo*), Tmpl*), asCALL_CDECL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("tmpl<T>", asBEHAVE_ADDREF, "void f()", asMETHOD(Tmpl,AddRef), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("tmpl<T>", asBEHAVE_RELEASE, "void f()", asMETHOD(Tmpl,Release), asCALL_THISCALL); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("tmpl<T>", asBEHAVE_TEMPLATE_CALLBACK, "bool f(int&in, bool&out)", asFUNCTION(Tmpl::TmplCallback), asCALL_CDECL); assert( r >= 0 );
#else
		r = engine->RegisterObjectBehaviour("tmpl<T>", asBEHAVE_FACTORY, "tmpl<T>@ f(int&in)", WRAP_FN_PR(Tmpl::TmplFactory, (asITypeInfo*), Tmpl*), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("tmpl<T>", asBEHAVE_ADDREF, "void f()", WRAP_MFN(Tmpl,AddRef), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("tmpl<T>", asBEHAVE_RELEASE, "void f()", WRAP_MFN(Tmpl,Release), asCALL_GENERIC); assert( r >= 0 );
		r = engine->RegisterObjectBehaviour("tmpl<T>", asBEHAVE_TEMPLATE_CALLBACK, "bool f(int&in, bool&out)", WRAP_FN(Tmpl::TmplCallback), asCALL_GENERIC); assert( r >= 0 );
#endif

		mod = engine->GetModule("1", asGM_ALWAYS_CREATE);
		bout.buffer = "";
		r = mod->LoadByteCode(&stream);
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != " (0, 0) : Error   : Attempting to instantiate invalid template type 'tmpl<int>'\n"
			               " (0, 0) : Error   : LoadByteCode failed. The bytecode is invalid. Number of bytes read from stream: 111\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Test problem reported by Philip Bennefall
	{
		const char *script = 
			"class dummy\n"
			"{\n"
			"  bool set_callback(menu_callback@ callback, string user_data)\n"
			"  {\n"
			"    @callback_handle=@callback;\n"
			"    callback_data=user_data;\n"
			"    return true;\n"
			"  }\n"
			"  void do_something()\n"
			"  {\n"
			"    if(@callback_handle!=null)\n"
			"    {\n"
			"      int callback_result=callback_handle(this, callback_data);\n"
			"    }\n"
			"  }\n"
			"  menu_callback@ callback_handle;\n"
			"  string callback_data;\n"
			"}\n"
			"funcdef int menu_callback(dummy@, string);\n"
			"void main()\n"
			"{\n"
			"}\n";

		CBytecodeStream stream(__FILE__"1");

		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		RegisterStdString(engine);

		mod = engine->GetModule("1", asGM_ALWAYS_CREATE);
		mod->AddScriptSection(0, script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		mod->SaveByteCode(&stream);

		mod = engine->GetModule("2", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	// Test problem reported by Markus Larsson from Skygoblin
	{
		const char *script1 = 
			"void main() {"
			"  print(\"a\" + \"b\");"
			"}";

		const char *script2 =
			"void main() {"
			"  if(getInt()==1)"
			"    print(\"a\" + \"b\");"
			"}";

		asIScriptContext* ctx;
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		RegisterStdString(engine);
#ifndef AS_MAX_PORTABILITY
		engine->RegisterGlobalFunction("void print(const string& in)", asFUNCTION(print), asCALL_CDECL);
		engine->RegisterGlobalFunction("int getInt()", asFUNCTION(getInt), asCALL_CDECL);
#else
		engine->RegisterGlobalFunction("void print(const string& in)", WRAP_FN(print), asCALL_GENERIC);
		engine->RegisterGlobalFunction("int getInt()", WRAP_FN(getInt), asCALL_GENERIC);
#endif
		
		ctx = engine->CreateContext();
		
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		r = mod->AddScriptSection(":1", script1, strlen(script1), 0); assert (r >= 0);
		r = mod->Build(); 
		if( r < 0 )
			TEST_FAILED;
		
		CBytecodeStream stream(__FILE__"1");
		
		r = mod->SaveByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;
		
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream); 
		if( r < 0 )
			TEST_FAILED;
		
		ctx->Prepare(mod->GetFunctionByDecl("void main()"));
		r = ctx->Execute();
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
	
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		CBytecodeStream stream2(__FILE__"2");
		r = mod->AddScriptSection(":1", script2, strlen(script2), 0); assert (r >= 0);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		
		r = mod->SaveByteCode(&stream2);
		if( r < 0 )
			TEST_FAILED;
		
		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream2);
		if( r < 0 )
			TEST_FAILED;
		
		ctx->Prepare(mod->GetFunctionByDecl("void main()"));
		r = ctx->Execute();		
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		ctx->Release();
		engine->Release();
	}

	// Test problem on 64bit
	// http://www.gamedev.net/topic/628452-linux-x86-64-not-loading-or-saving-bytecode-correctly/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert( bool )", asFUNCTION(Assert), asCALL_GENERIC);

		mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"enum TestEnum \n"
			"{ \n"
			"  TestEnum_A = 42 \n"
			"} \n"
			"class NonPrimitive \n"
			"{ \n"
			"  int val; \n"
			"} \n"
			"void Foo( int a, TestEnum e, NonPrimitive o ) \n"
			"{ \n"
			"  assert( a == 1 ); \n"
			"  assert( e == TestEnum_A ); \n"
			"  assert( o.val == 513 ); \n"
			"} \n"
			"void main() \n"
			"{ \n"
			"  NonPrimitive o; \n"
			"  o.val = 513; \n"
			"  Foo( 1, TestEnum_A, o ); \n"
			"} \n");

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		CBytecodeStream stream(__FILE__"1");
		r = mod->SaveByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		asIScriptModule *mod2 = engine->GetModule("mod2", asGM_ALWAYS_CREATE);
		r = mod2->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "main()", mod2);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->Release();
	}

	// Success
	return fail;
}

bool Test2()
{
	int r;
	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);

	const char *script = 
		"enum ENUM1{          \n"
		"_ENUM_1 = 1          \n"
		"}                    \n"
		"void main()          \n"
		"{                    \n"
		"int item = _ENUM_1;  \n"
		"}                    \n";

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	r = mod->AddScriptSection("script", script, strlen(script));
	r = mod->Build();
	if( r < 0 )
		TEST_FAILED;

	CBytecodeStream stream(__FILE__"6");
	mod = engine->GetModule(0);
	r = mod->SaveByteCode(&stream);
	if( r < 0 )
		TEST_FAILED;
	engine->Release();

	engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	r = mod->LoadByteCode(&stream);
	if( r < 0 )
		TEST_FAILED;

	r = ExecuteString(engine, "main()", mod);
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	engine->Release();

	return fail;
}

class CAPStringFactory : public asIStringFactory
{
public:
	const void *GetStringConstant(const char *data, asUINT length)
	{
		char **str = new char*;
		*str = new char[length+1];
		memcpy(*str, data, length);
		(*str)[length] = 0;
		return str;
	}

	int ReleaseStringConstant(const void *str)
	{
		delete[] *reinterpret_cast<char**>(const_cast<void*>(str));
		delete reinterpret_cast<char**>(const_cast<void*>(str));
		return 0;
	}

	int GetRawStringData(const void *str, char *data, asUINT *length) const
	{
		if (length) *length = (asUINT)strlen(*reinterpret_cast<char**>(const_cast<void*>(str)));
		if (data) memcpy(data, *reinterpret_cast<void**>(const_cast<void*>(str)), strlen(*reinterpret_cast<char**>(const_cast<void*>(str))));
		return 0;
	}
} APStringFactory;

void APStringConstruct(const char **s)
{
	*s = 0;
}

bool TestAndrewPrice()
{
	COutStream out;
	CBufferedOutStream bout;

	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->SetEngineProperty(asEP_COPY_SCRIPT_SECTIONS, true);
		RegisterScriptArray(engine, true);

		// This POD type doesn't have an opAssign, so the bytecode will have asBC_COPY 
		engine->RegisterObjectType("char_ptr", sizeof(char*), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE);
#ifndef AS_MAX_PORTABILITY
		engine->RegisterObjectBehaviour("char_ptr", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(APStringConstruct), asCALL_CDECL_OBJLAST);
#else
		engine->RegisterObjectBehaviour("char_ptr", asBEHAVE_CONSTRUCT, "void f()", WRAP_OBJ_LAST(APStringConstruct), asCALL_GENERIC);
#endif
		engine->RegisterStringFactory("char_ptr", &APStringFactory);

		asIScriptModule *mod = engine->GetModule("Test", asGM_ALWAYS_CREATE);
		char Data2[] = 
			"const char_ptr[] STORAGE_STRINGS = {'Storage[0]','Storage[1]'}; ";
		mod->AddScriptSection("Part2",Data2,(int)strlen(Data2));
		int r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		mod->BindAllImportedFunctions();

		CBytecodeStream stream(__FILE__"1");
		r = mod->SaveByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		mod = engine->GetModule("Test2", asGM_ALWAYS_CREATE);
		r = mod->LoadByteCode(&stream);
		if( r < 0 )
			TEST_FAILED;

		CScriptArray *arr = reinterpret_cast<CScriptArray*>(mod->GetAddressOfGlobalVar(0));
		if( arr == 0 || arr->GetSize() != 2 || strcmp(*reinterpret_cast<const char**>(arr->At(1)), "Storage[1]") != 0 )
			TEST_FAILED;

		engine->Release();

		// Try loading the bytecode again, except this time without configuring the engine
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		
		mod = engine->GetModule("Test3", asGM_ALWAYS_CREATE);
		stream.Restart();
		bout.buffer = "";
		r = mod->LoadByteCode(&stream);
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != " (0, 0) : Error   : Template type 'array' doesn't exist\n"
			               " (0, 0) : Error   : LoadByteCode failed. The bytecode is invalid. Number of bytes read from stream: 13\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		RegisterScriptArray(engine, true);
		stream.Restart();
		bout.buffer = "";
		r = mod->LoadByteCode(&stream);
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != " (0, 0) : Error   : Object type 'char_ptr' doesn't exist\n"
						   " (0, 0) : Error   : LoadByteCode failed. The bytecode is invalid. Number of bytes read from stream: 20\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	return fail;
}

} // namespace

