#include "utils.h"
using std::string;

namespace TestPow
{
const char *script =
"class myclass								 \n"
"{											 \n"
"	double opPow(int x)						 \n"
"	{										 \n"
"		return val ** x;					 \n"
"	}										 \n"
"	double opPow(double x)					 \n"
"	{										 \n"
"		return val ** x;					 \n"
"	}										 \n"
"	double opPow_r(double x)				 \n"
"	{										 \n"
"		return x ** val;					 \n"
"	}										 \n"
"	myclass& opPowAssign(double x)			 \n"
"	{										 \n"
"		val **= x;							 \n"
"       return this;                         \n"
"	}										 \n"
"	double val;								 \n"
"};											 \n"
"											 \n"
"void test_pow()                             \n"
"{                                           \n"
"	assert(3 ** 2 == 9);                     \n"
"   assert(9.0 ** 0.5 == 3.0);               \n"
"   assert(9 ** 0.5 == 3.0);                 \n"
"   assert(2.5 ** 2 == 6.25);                \n"
"                                            \n"
"   double  a = 2.5;					     \n"
"   int     b = 2;						     \n"
"   uint    c = 3;						     \n"
"   float   d = 0.5;					     \n"
"   int64   e = 4;						     \n"
"   									     \n"
"   assert(c ** b == 9);				     \n"
"   assert(c ** 2 == 9);                     \n"
"   assert(e ** d == 2.0);				     \n"
"   assert(a ** c == 15.625);			     \n"
"   assert(a ** b == 6.25);				     \n"
"   assert(e ** 30 == 1152921504606846976);  \n"
"                                            \n"
"	int z = 0;                               \n"
"   int o = 1;                               \n"
"                                            \n"
"   assert(z ** o == z);                     \n"
"   assert(o ** z == 1);                     \n"
"   assert(a ** 0 == 1.0);                   \n"
"   assert(a ** 1 == a);                     \n"
"   assert(b ** c * b == b ** (c + 1));      \n"
"   assert(c ** -o == 0);                    \n"
"   assert(double(e) ** -2 >= 0.062499 &&    \n"
"          double(e) ** -2 <= 0.062501);     \n"
"										     \n"
"   b **= c;                                 \n"
"   assert(b == 8);                          \n"
"   myclass obj;                             \n"
"   obj.val = 4.0;						     \n"
"   assert(obj ** 3 == 64.0);			     \n"
"   assert(obj ** 3.0 == 64.0);			     \n"
"   assert(3.0 ** obj == 81.0);			     \n"
"   obj **= 3;							     \n"
"   assert(obj.val == 64.0);			     \n"
"}                                           \n"
"                                            \n"
"void test_overflow1()                       \n"
"{                                           \n"
"   double x = 1.0e100;                      \n"
"   x = x ** 6;                              \n"
"}                                           \n"
"                                            \n"
"void test_overflow2()                       \n"
"{                                           \n"
"   int x = 3;                               \n"
"   x = x ** 21;                             \n"
"}                                           \n"
"                                            \n"
"void test_overflow3()                       \n"
"{                                           \n"
"   double x = 1.0e100;                      \n"
"   x = x ** 6.0;                            \n"
"}                                           \n";

bool Test()
{
	bool fail = false;
	COutStream out;
	CBufferedOutStream bout;
	int r;

	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

		asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
		mod->AddScriptSection("script", script, strlen(script));
		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;

		asIScriptContext *ctx = engine->CreateContext();
		asIScriptFunction *func;
		string err_str;

		func = mod->GetFunctionByName("test_pow");
		ctx->Prepare(func);
		r = ctx->Execute();
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;

		func = mod->GetFunctionByName("test_overflow1");
		ctx->Prepare(func);
		r = ctx->Execute();
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;
		else
		{
			err_str = ctx->GetExceptionString();
			if( err_str != "Overflow in exponent operation" )
				TEST_FAILED;
		}

		func = mod->GetFunctionByName("test_overflow2");
		ctx->Prepare(func);
		r = ctx->Execute();
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;
		else
		{
			err_str = ctx->GetExceptionString();
			if( err_str != "Overflow in exponent operation" )
				TEST_FAILED;
		}

		func = mod->GetFunctionByName("test_overflow3");
		ctx->Prepare(func);
		r = ctx->Execute();
		if( r != asEXECUTION_EXCEPTION )
			TEST_FAILED;
		else
		{
			err_str = ctx->GetExceptionString();
			if( err_str != "Overflow in exponent operation" )
				TEST_FAILED;
		}

		ctx->Release();
		engine->Release();
	}

	// Test overflow in constant expression
	// http://www.gamedev.net/topic/662096-removing/
	{
		asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";

		asIScriptModule *mod = engine->GetModule("test", asGM_ALWAYS_CREATE);
		mod->AddScriptSection("test", 
			"void func() { \n"
			"  int    ipow = 500    ** 3000; \n"
			"  float  fpow = 500.0f ** 3000.0f; \n"
			"  double dpow = 500.0f ** 3000.0f; \n"
			"} \n");
		r = mod->Build();
		if( r >= 0 )
			TEST_FAILED;

		if( bout.buffer != "test (1, 1) : Info    : Compiling void func()\n"
						   "test (2, 24) : Error   : Overflow in exponent operation\n"
						   "test (3, 24) : Error   : Overflow in exponent operation\n"
						   "test (4, 24) : Error   : Overflow in exponent operation\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	return fail;
}

} // namespace
