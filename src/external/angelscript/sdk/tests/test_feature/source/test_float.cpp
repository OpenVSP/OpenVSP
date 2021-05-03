#include "utils.h"

namespace TestFloat
{

static const char *script =
"void TestFloat()                               \n"
"{                                              \n"
"  float a = 2, b = 3, c = 1;                   \n"
"  c = a + b;                                   \n"
"  a = b + 23;                                  \n"
"  b = 12 + c;                                  \n"
"  c = a - b;                                   \n"
"  a = b - 23;                                  \n"
"  b = 12 - c;                                  \n"
"  c = a * b;                                   \n"
"  a = b * 23;                                  \n"
"  b = 12 * c;                                  \n"
"  c = a / b;                                   \n"
"  a = b / 23;                                  \n"
"  b = 12 / c;                                  \n"
"  c = a % b;                                   \n"
"  a = b % 23;                                  \n"
"  b = 12 % c;                                  \n"
"  a++;                                         \n"
"  ++a;                                         \n"
"  a += b;                                      \n"
"  a += 3;                                      \n"
"  a /= c;                                      \n"
"  a /= 5;                                      \n"
"  a = b = c;                                   \n"
"  func( a-1, b, c );                           \n"
"  a = -b;                                      \n"
"  a = func2();                                 \n"
"}                                              \n"
"void func(float a, float &in b, float &out c)  \n"
"{                                              \n"
"  c = a + b;                                   \n"
"  b = c;                                       \n"
"  g = g;                                       \n"
"}                                              \n"
"float g = 0;                                   \n"
"float func2()                                  \n"
"{                                              \n"
"  return g + 1;                                \n"
"}                                              \n";


static const char *script2 =
"void start()           \n"
"{                      \n"
"float test = 1.9f;     \n"
"print(test);           \n"
"print(test + test);    \n"
"}                      \n";

void print_gen(asIScriptGeneric *gen)
{
	float val = *(float*)gen->GetAddressOfArg(0);
	UNUSED_VAR(val);
}


bool Test()
{
	bool fail = false;
	COutStream out;
 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
	engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	int r = ExecuteString(engine, "float a = 1e5; float b = 1.0e5; assert( a == b ); \n");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;


	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script, strlen(script));
 	r = mod->Build();
	if( r < 0 ) TEST_FAILED;


	engine->RegisterGlobalFunction("void print(float)", asFUNCTION(print_gen), asCALL_GENERIC);

	mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection("script", script2, strlen(script2));
	r = mod->Build();
	if( r < 0 ) TEST_FAILED;

	ExecuteString(engine, "start()", mod);

#if !defined(__psp2__)
	// The locale affects the way the compiler reads float values
	setlocale(LC_NUMERIC, "");
#endif

	float f;
	engine->RegisterGlobalProperty("float f", &f);
	r = ExecuteString(engine, "f = 3.14f;");
	if( r != asEXECUTION_FINISHED )
		TEST_FAILED;

	if( f < 3.139999f || f > 3.140001f )
		TEST_FAILED;

#if !defined(__psp2__)
	setlocale(LC_NUMERIC, "C");
#endif

	engine->Release();

	// Test that compiler properly scans the digits
	// http://www.gamedev.net/topic/662758-formatfloat-fails-for-documented-dbl-max/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);

		RegisterStdString(engine);
		RegisterScriptArray(engine, false);
		RegisterStdStringUtils(engine);

		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		r = ExecuteString(engine, "double d = 1.79769313486231e+308; \n"
								  "string s = formatFloat(d, 'e'); \n"
								  "assert( s == '2e+308' ); \n"
								  "d = 2.22507385850720e-308; \n"
								  "s = formatFloat(d, 'e'); \n"
								  "assert( s == '2e-308' ); \n"
								  "float f = 3.402823466e+38f; \n"
								  "s = formatFloat(f, 'e'); \n"
								  "assert( s == '3e+38' || s == '3e+038' ); \n" // The formatting varies from compiler to compiler, and version too
								  "f = 1.175494351e-38f; \n"
								  "s = formatFloat(f, 'e'); \n"
								  "assert( s == '1e-38' || s == '1e-038' ); \n" // The formatting varies from compiler to compiler, and version too
						);
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		engine->ShutDownAndRelease();
	}

	// Success
 	return fail;
}

} // namespace

