#include "utils.h"

namespace TestImplicitHandle
{

std::string output;
void print(std::string &str)
{
	output += str;
}

bool Test()
{
	RET_ON_MAX_PORT

	bool fail = false;
	int r;
	COutStream out;

	// Test implicit handle with default array
	// reported by Quentin Cosendey
	{
		asIScriptEngine* engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->SetEngineProperty(asEP_ALLOW_IMPLICIT_HANDLE_TYPES, true);
		engine->SetEngineProperty(asEP_DISALLOW_VALUE_ASSIGN_FOR_REF_TYPE, true);

		RegisterScriptArray(engine, true);

		asIScriptModule* mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script",
			"class @SomeClass { } \n"
			"SomeClass[] someArray; \n"          // should be the same as array<SomeClass@>
			"array<SomeClass> someArray2; \n");  // should be the same as array<SomeClass@>
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		int typeId;
		mod->GetGlobalVar(mod->GetGlobalVarIndexByName("someArray"), 0, 0, &typeId);
		asITypeInfo *typeInfo = engine->GetTypeInfoById(typeId);
		typeId = typeInfo->GetSubTypeId();
		if (!(typeId & asTYPEID_OBJHANDLE))
			TEST_FAILED;

		mod->GetGlobalVar(mod->GetGlobalVarIndexByName("someArray2"), 0, 0, &typeId);
		typeInfo = engine->GetTypeInfoById(typeId);
		typeId = typeInfo->GetSubTypeId();
		if (!(typeId & asTYPEID_OBJHANDLE))
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Test implicit handle types with auto declarations
	// https://www.gamedev.net/forums/topic/696366-there-seem-to-be-cases-where-implicit-handle-does-not-work-well/
	{
		asIScriptEngine *engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->SetEngineProperty(asEP_ALLOW_IMPLICIT_HANDLE_TYPES, true);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script",
			"class@ A \n"
			"{ \n"
			"  A(A@ a) {  } \n"
			"} \n"
			"class@ B : A \n"
			"{ \n"
			"  B(A@ a) { super(a); } \n"
			"} \n"
			"class@ C : B \n"
			"{ \n"
			"  C(A@ a) { super(a); } \n"
			"} \n"
			"int main() \n"
			"{ \n"
			"  auto a = A(null); \n"
			"  auto c = C(a); \n"
			"  auto b = B(c); \n"
			"  return 0; \n"
			"} \n");
		r = mod->Build();
		if (r < 0)
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Declaring and using implicit handle types
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
		engine->SetEngineProperty(asEP_ALLOW_IMPLICIT_HANDLE_TYPES, true);
		RegisterScriptString(engine);

		r = engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC); assert(r >= 0);
		r = engine->RegisterGlobalFunction("void print(const string &in)", asFUNCTION(print), asCALL_CDECL); assert(r >= 0);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", 
			"class@ MyClass                              \n" // Define the class to be an implicit handle type
			"{                                           \n"
			"    MyClass() { print('Created\\n'); }      \n"
			"    int func() { return 7; }                \n"
			"    string x;                               \n"
			"};                                          \n"
			"                                            \n"
			"void testClass(MyClass x)                   \n" // Parameter pass by handles
			"{                                           \n"
			"    print(x.func()+'\\n');                  \n"
			"}                                           \n"
			"                                            \n"
			"void main()                                 \n"
			"{                                           \n"
			"    MyClass p,p2;                           \n"
			"                                            \n"
			"    p = p2;                                 \n"
			"                                            \n"
			"    if (p is null) { print('Hello!\\n'); }  \n"
			"                                            \n"
			"    p = MyClass();                          \n"
			"                                            \n"
			"    print('---\\n');                        \n"
			"    testClass(p);                           \n"
			"    print('---\\n');                        \n"
			"                                            \n"
			"    print(p.func()+'\\n');                  \n"
			"}                                           \n");
		r = mod->Build();
		if (r < 0)
		{
			TEST_FAILED;
			PRINTF("Failed to compile the script\n");
		}

		r = ExecuteString(engine, "main()", mod);
		if (r != asEXECUTION_FINISHED)
		{
			TEST_FAILED;
			PRINTF("Execution failed\n");
		}

		if (output != "Hello!\nCreated\n---\n7\n---\n7\n")
		{
			TEST_FAILED;
			PRINTF("Got: \n%s", output.c_str());
		}


		// TODO: The equality operator shouldn't perform handle comparison
		/*
		r = engine->ExecuteString(0, "MyClass a; assert( a == null );");
		if( r >= 0 )
		{
			TEST_FAILED;
		}
		*/

		engine->Release();
	}

	// It should be possible to register types with asOBJ_IMPLICIT_HANDLE when the engine property is turned on
	// http://www.gamedev.net/topic/679183-asep-allow-implicit-handle-types-asobj-implicit-handle/
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		CBufferedOutStream bout;
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		engine->SetEngineProperty(asEP_ALLOW_IMPLICIT_HANDLE_TYPES, true);

		r = engine->RegisterObjectType("test1", 0, asOBJ_REF | asOBJ_IMPLICIT_HANDLE);
		if (r < 0)
			TEST_FAILED;

		r = engine->RegisterObjectBehaviour("test1", asBEHAVE_ADDREF, "void func()", asFUNCTION(0), asCALL_GENERIC);
		if (r < 0)
			TEST_FAILED;

		r = engine->RegisterObjectProperty("test1", "int a", 0);
		if (r < 0)
			TEST_FAILED;

		r = engine->RegisterObjectMethod("test1", "void func()", asFUNCTION(0), asCALL_GENERIC);
		if (r < 0)
			TEST_FAILED;

		engine->SetEngineProperty(asEP_ALLOW_IMPLICIT_HANDLE_TYPES, false);

		r = engine->RegisterObjectType("test2", 0, asOBJ_REF | asOBJ_NOCOUNT | asOBJ_IMPLICIT_HANDLE);
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != " (0, 0) : Error   : Failed in call to function 'RegisterObjectType' with 'test2' (Code: asINVALID_ARG, -5)\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}



		engine->Release();
	}

	// Success
	return fail;
}

} // namespace

