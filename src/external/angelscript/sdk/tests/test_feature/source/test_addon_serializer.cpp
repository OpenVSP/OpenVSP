#include "utils.h"
#include "../../../add_on/scriptarray/scriptarray.h"
#include "../../../add_on/serializer/serializer.h"
#include "../../../add_on/scriptmath/scriptmathcomplex.h"

namespace Test_Addon_Serializer
{

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

class CDummy
{
public:
	CDummy() : refCount(1) {};
	void AddRef() { refCount++; }
	void Release() { refCount--; }
	int refCount;
};

bool Test()
{
	bool fail = false;
	int r;
	COutStream out;
	CBufferedOutStream bout;
 	asIScriptEngine *engine;
	asIScriptModule *mod;
	
	SKIP_ON_MAX_PORT
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		RegisterScriptString(engine);
		RegisterScriptArray(engine, false);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		// Register an application type that cannot be created from script. The serializer
		// will then keep the same pointer, while properly maintaining the refcount
		engine->RegisterObjectType("dummy", 0, asOBJ_REF);
		engine->RegisterObjectBehaviour("dummy", asBEHAVE_ADDREF, "void f()", asMETHOD(CDummy,AddRef), asCALL_THISCALL);
		engine->RegisterObjectBehaviour("dummy", asBEHAVE_RELEASE, "void f()", asMETHOD(CDummy,Release), asCALL_THISCALL);

		const char *script = 
			"float f; \n"
			"string str; \n"
			"array<int> arr; \n"
			"class CTest \n"
			"{ \n"
			"  int a; \n"
			"  string str; \n"
			"  dummy @d; \n"
			"} \n"
			"CTest @t; \n"
			"CTest a; \n"
			"CTest @b; \n"
			"CTest @t2 = @a; \n"
			"CTest @n = @a; \n"
			"array<CTest> arrOfTest; \n";

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		// Populate the handle to the external object
		CDummy dummy;
		asIScriptObject *obj = (asIScriptObject*)mod->GetAddressOfGlobalVar(mod->GetGlobalVarIndexByName("a"));
		*(CDummy**)obj->GetAddressOfProperty(2) = &dummy;
		dummy.AddRef();
		
		r = ExecuteString(engine, "f = 3.14f; \n"
			                      "str = 'test'; \n"
								  "arr.resize(3); arr[0] = 1; arr[1] = 2; arr[2] = 3; \n"
								  "a.a = 42; \n"
								  "a.str = 'hello'; \n"
								  "@b = @a; \n"
								  "@t = CTest(); \n"
								  "t.a = 24; \n"
								  "t.str = 'olleh'; \n"
								  "@t2 = t; \n"
								  "@n = null; \n"
								  "arrOfTest.insertLast(CTest());\n"
								  "arrOfTest[0].str = 'blah';\n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// Add an extra object for serialization
		asIScriptObject *scriptObj = reinterpret_cast<asIScriptObject*>(engine->CreateScriptObject(mod->GetTypeInfoByName("CTest")));
		((std::string*)(scriptObj->GetAddressOfProperty(1)))->assign("external object");

		// Reload the script while keeping the object states
		{
			CSerializer modStore;
			modStore.AddUserType(new CStringType(), "string");
			modStore.AddUserType(new CArrayType(), "array");

			modStore.AddExtraObjectToStore(scriptObj);

			r = modStore.Store(mod);
			if( r < 0 )
				TEST_FAILED;

			engine->DiscardModule(0);

			mod = engine->GetModule("2", asGM_ALWAYS_CREATE);
			mod->AddScriptSection("script", script);
			r = mod->Build();
			if( r < 0 )
				TEST_FAILED;

			r = modStore.Restore(mod);
			if( r < 0 )
				TEST_FAILED;

			// Restore the extra object
			asIScriptObject *obj2 = (asIScriptObject*)modStore.GetPointerToRestoredObject(scriptObj);
			scriptObj->Release();
			scriptObj = obj2;
			scriptObj->AddRef();
		}

		r = ExecuteString(engine, "assert(f == 3.14f); \n"
		                          "assert(str == 'test'); \n"
								  "assert(arr.length() == 3 && arr[0] == 1 && arr[1] == 2 && arr[2] == 3); \n"
								  "assert(a.a == 42); \n"
								  "assert(a.str == 'hello'); \n"
								  "assert(b is a); \n"
								  "assert(t !is null); \n"
								  "assert(t.a == 24); \n"
								  "assert(t.str == 'olleh'); \n"
								  "assert(t is t2); \n"
								  "assert(n is null); \n"
								  "assert(arrOfTest.length() == 1 && arrOfTest[0].str == 'blah'); \n"
								  "assert(a.d !is null); \n", mod);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		// The new object has the same content
		if( *(std::string*)scriptObj->GetAddressOfProperty(1) != "external object" )
			TEST_FAILED;
		scriptObj->Release();

		// After the serializer has been destroyed the refCount for the external handle must be the same as before
		if( dummy.refCount != 2 )
			TEST_FAILED;
		
		engine->Release();
	}

	// Report proper error when missing user type for non-POD type
	SKIP_ON_MAX_PORT
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
		RegisterStdString(engine);
		RegisterScriptMathComplex(engine);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char* script =
			"string str; \n"     // non-POD: error
			"complex cmplx; \n"; // POD:     no error

		mod->AddScriptSection( 0, script );
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		bout.buffer = "";
		CSerializer modStore;

		r = modStore.Store(mod);
		if( r < 0 )
			TEST_FAILED;

		r = modStore.Restore(mod);
		if( r < 0 )
			TEST_FAILED;

		if( bout.buffer != " (0, 0) : Error   : Cannot restore type 'string'\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	// Make sure it is possible to restore objects, where the constructor itself is changing other objects
	// http://www.gamedev.net/topic/604890-dynamic-reloading-script/page__st__20
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		RegisterScriptString(engine);
		RegisterScriptArray(engine, false);

		mod = engine->GetModule(0, asGM_ALWAYS_CREATE);

		const char* script =
			"array<TestScript@> arr; \n"
			"class TestScript \n"
			"{ \n"
			"         TestScript()   \n"
			"         { \n"
			"                  arr.insertLast( this ); \n"
			"         }       \n"
			"} \n"
			"void startGame()          \n"
			"{ \n"
			"         TestScript @t = TestScript(); \n"
			"}  \n";

		mod->AddScriptSection( 0, script );
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = ExecuteString(engine, "startGame()", mod);

		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		CSerializer modStore;
		modStore.AddUserType(new CStringType(), "string");
		modStore.AddUserType(new CArrayType(), "array");

		r = modStore.Store(mod);
		if( r < 0 )
			TEST_FAILED;

		engine->DiscardModule(0);

		mod = engine->GetModule("2", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script);
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		r = modStore.Restore(mod);
		if( r < 0 )
			TEST_FAILED;

		engine->Release();
	}

	return fail;
}

} // namespace

