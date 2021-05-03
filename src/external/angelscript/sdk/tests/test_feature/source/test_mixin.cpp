#include "utils.h"

using namespace std;

namespace TestMixin
{

bool Test()
{
	bool fail = false;
	asIScriptEngine *engine;
	int r;
	CBufferedOutStream bout;
	const char *script;
	asIScriptModule *mod;

	// Mixin and namespaces
	// http://www.gamedev.net/topic/665026-error-for-non-default-namespace-interfaces-used-in-methods-of-mixin-classes-inherited-from-by-classes-in-another-namespaces/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";
		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);

		script =
			"namespace n { \n"
			"	interface i {} \n"
			"	mixin class m { \n"
			"		void f(i@) { \n"
			"			i@ a; \n"
			"		} \n"
			"	} \n"
			"} \n"
			"class c : n::m {} \n";
		mod->AddScriptSection("test", script);

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	
		engine->Release();
	}

	// Mixins and namespaces
	// http://www.gamedev.net/topic/657378-mixin-and-namespace/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);

		bout.buffer = "";
		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);

		script =
			"mixin class M1 \n"
			"{ \n"
			"  int a; \n"
			"} \n"
			"namespace N1 \n"
			"{ \n"
			"  class C1 : M1 \n"
			"  { \n"
			"    void foo() \n"
			"    { \n"
			"      a = 10; \n"
			"    } \n"
			"  } \n"
			"} \n";
		mod->AddScriptSection("test", script);

		r = mod->Build();
		if( r < 0 )
			TEST_FAILED;
		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
	
		engine->Release();
	}

	// Basic tests
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		RegisterScriptString(engine);
		mod = engine->GetModule("mod", asGM_ALWAYS_CREATE);

		// Test basic parsing of mixin classes
		{
			script =
				"mixin class Test {\n"
				"  void Method() {} \n"
				"  int Prop1, Prop2; \n"
				"} \n";

			bout.buffer = "";
			mod->AddScriptSection("", script);
			r = mod->Build();
			if( r >= 0 )
				TEST_FAILED;
			if( bout.buffer != " (0, 0) : Error   : Nothing was built in the module\n" )
			{
				PRINTF("%s", bout.buffer.c_str());
				TEST_FAILED;
			}
		}

		// Test name conflicts
		// mixin classes can be declared in namespaces to resolve conflicts
		{
			script =
				"mixin class Test {} \n"
				"mixin class Test {} \n"
				"int Test; \n"
				"namespace A { \n"
				"  mixin class Test {} \n"
				"} \n";
			mod->AddScriptSection("", script);

			bout.buffer = "";
			r = mod->Build();
			if( r >= 0 )
				TEST_FAILED;
			if( bout.buffer != " (2, 13) : Error   : Name conflict. 'Test' is a mixin class.\n"
				               " (3, 5) : Error   : Name conflict. 'Test' is a mixin class.\n" )
			{
				PRINTF("%s", bout.buffer.c_str());
				TEST_FAILED;
			}
		}

		// Mixin classes cannot be declared as 'shared' or 'final'
		{
			script =
				"mixin shared final class Test {} \n";
			mod->AddScriptSection("", script);

			bout.buffer = "";
			r = mod->Build();
			if( r >= 0 )
				TEST_FAILED;
			if( bout.buffer != " (1, 7) : Error   : Mixin class cannot be declared as 'shared'\n"
							   " (1, 14) : Error   : Mixin class cannot be declared as 'final'\n" )
			{
				PRINTF("%s", bout.buffer.c_str());
				TEST_FAILED;
			}
		}

		// Mixin classes cannot be declared as 'external' or 'abstract'
		// Reported by Dantrell B
		{
			script =
				"mixin external abstract class Test {} \n";
			mod->AddScriptSection("", script);

			bout.buffer = "";
			r = mod->Build();
			if (r >= 0)
				TEST_FAILED;
			if (bout.buffer != " (1, 7) : Error   : Mixin class cannot be declared as 'external'\n"
				" (1, 16) : Error   : Mixin class cannot be declared as 'abstract'\n")
			{
				PRINTF("%s", bout.buffer.c_str());
				TEST_FAILED;
			}
		}

		// mixin classes can implement interfaces
		{
			script = 
				"interface Intf {} \n"
				"interface Intf2 {} \n"
				"mixin class Mix : Intf, Intf2 {} \n"
				"class Clss : Mix {} \n"
				"void func() { \n"
				"  Clss c; \n"
				"  Intf @i = c; \n"
				"  Intf2 @i2 = c; \n"
				"} \n";
			mod->AddScriptSection("", script);

			bout.buffer = "";
			r = mod->Build();
			if( r < 0 )
				TEST_FAILED;
			if( bout.buffer != "" )
			{
				PRINTF("%s", bout.buffer.c_str());
				TEST_FAILED;
			}
		}

		// mixin classes cannot inherit from other classes
		// TODO: should be allowed to implement interfaces, and possibly inherit from classes
		//       when that is possible, the inheritance is simply transfered to the class that inherits the mixin class
		{
			script =
				"interface Intf {} \n"
				"class Clss {} \n"
				"mixin class Test : Intf, Clss {} \n"
				"class Clss2 : Test {} \n";
			mod->AddScriptSection("", script);

			bout.buffer = "";
			r = mod->Build();
			if( r >= 0 )
				TEST_FAILED;
			if( bout.buffer != " (3, 26) : Error   : Mixin class cannot inherit from classes\n" )
			{
				PRINTF("%s", bout.buffer.c_str());
				TEST_FAILED;
			}
		}

		// Mixin class including mixin class was crashing
		// http://www.gamedev.net/topic/630332-angelscript-mixin-classes-preview/
		{
			script = 
				"mixin class Mixin1 { int a; }; \n"
				"mixin class Mixin2 : Mixin1 { }; \n"
				"class Clss : Mixin2 {} \n";
			mod->AddScriptSection("", script);

			bout.buffer = "";
			r = mod->Build();
			if( r >= 0 )
				TEST_FAILED;
			if( bout.buffer != " (2, 22) : Error   : Mixin class cannot inherit from classes\n" )
			{
				PRINTF("%s", bout.buffer.c_str());
				TEST_FAILED;
			}
		}

		// including a mixin class adds the properties from the mixin to the class
		{
			script = 
				"mixin class Mix { \n"
				"  int prop; \n"
				"} \n"
				"class Clss : Mix { \n"
				"  void func() { prop++; } \n"
				"} \n";
			mod->AddScriptSection("", script);

			bout.buffer = "";
			r = mod->Build();
			if( r < 0 )
				TEST_FAILED;
			if( bout.buffer != "" )
			{
				PRINTF("%s", bout.buffer.c_str());
				TEST_FAILED;
			}
		}

		// properties from mixin classes are not included if they conflict with existing properties
		{
			script = 
				"mixin class Mix { int prop; int prop2; } \n"
				"class Base { float prop; } \n"
				"class Clss : Base, Mix {} \n";
			mod->AddScriptSection("", script);

			bout.buffer = "";
			r = mod->Build();
			if( r < 0 )
				TEST_FAILED;
			if( bout.buffer != "" )
			{
				PRINTF("%s", bout.buffer.c_str());
				TEST_FAILED;
			}

			asITypeInfo *ot = mod->GetTypeInfoByName("Clss");
			if( ot == 0 )
				TEST_FAILED;

			if( ot->GetPropertyCount() != 2 )
				TEST_FAILED;
			const char *name;
			int typeId;
			if( ot->GetProperty(0, &name, &typeId) < 0 )
				TEST_FAILED;
			if( string(name) != "prop" || typeId != asTYPEID_FLOAT )
				TEST_FAILED;
		}

		// methods from mixin class are included into the class
		// mixin class methods are compiled in the context of the inherited class, 
		// so they can access properties of the class
		{
			script =
				"mixin class Mix { void mthd() { prop++; } } \n"
				"class Clss : Mix { int prop; } \n"
				"class Clss2 : Mix { float prop; } \n"
				"void func() { \n"
				"  Clss c; \n"
				"  c.mthd(); \n"
				"  Clss2 c2; \n"
				"  c2.mthd(); \n"
				"} \n";
			mod->AddScriptSection("", script);

			bout.buffer = "";
			r = mod->Build();
			if( r < 0 )
				TEST_FAILED;
			if( bout.buffer != "" )
			{
				PRINTF("%s", bout.buffer.c_str());
				TEST_FAILED;
			}
		}

		// mixin class methods are included even if an inherited base class 
		// implements them (the mixin method overrides the base class' method)
		{
			script = 
				"mixin class Mix { int mthd() { return 42; } } \n"
				"class Base { int mthd() { return 24; } } \n"
				"class Clss : Base, Mix { } \n";
			mod->AddScriptSection("", script);

			bout.buffer = "";
			r = mod->Build();
			if( r < 0 )
				TEST_FAILED;
			if( bout.buffer != "" )
			{
				PRINTF("%s", bout.buffer.c_str());
				TEST_FAILED;
			}

			r = ExecuteString(engine, "Clss c; assert( c.mthd() == 42 );", mod);
			if( r != asEXECUTION_FINISHED )
				TEST_FAILED;
		}

		// mixin class methods are ignored if the class explicitly declares its own
		{
			script = 
				"mixin class Mix { int mthd() { return 42; } } \n"
				"class Clss : Mix { int mthd() { return 24; } } \n";
			mod->AddScriptSection("", script);

			bout.buffer = "";
			r = mod->Build();
			if( r < 0 )
				TEST_FAILED;
			if( bout.buffer != "" )
			{
				PRINTF("%s", bout.buffer.c_str());
				TEST_FAILED;
			}

			r = ExecuteString(engine, "Clss c; assert( c.mthd() == 24 );", mod);
			if( r != asEXECUTION_FINISHED )
				TEST_FAILED;
		}
	
		// The error messages must show that the origin of the problem is from the mixin class
		{
			script = 
				"mixin class Mix { \n"
				"  const int prop; \n"  // const properties are disallowed
				"  int mthd; \n"		// conflict with class methods will give error
				"  void func() { prop++; } \n"
				"} \n"
				"class Clss : Mix { \n"
				"  void mthd() {} \n"
				"} \n"
				"class Clss2 : Mix { \n"
				"  string prop; \n" // The mixins prop is not used, so ++ is invalid
				"} \n"
				"Mix m; \n" // mixins cannot be instanciated, i.e. the object type doesn't exist
			    "void func() { \n"
				"  Mix m; \n"
				"} \n";
			mod->AddScriptSection("test", script);

			bout.buffer = "";
			r = mod->Build();
			if( r >= 0 )
				TEST_FAILED;
			if( bout.buffer != "test (12, 1) : Error   : Identifier 'Mix' is not a data type in global namespace\n"
				             /*  "test (2, 3) : Error   : Class properties cannot be declared as const\n"
			                   "test (3, 7) : Error   : Name conflict. 'mthd' is a class method.\n"
							   "test (6, 14) : Info    : Previous error occurred while including mixin\n"
				               "test (2, 3) : Error   : Class properties cannot be declared as const\n"
							   "test (13, 1) : Info    : Compiling void func()\n"
			                   "test (14, 3) : Error   : Identifier 'Mix' is not a data type\n"
							   "test (4, 3) : Info    : Compiling void Clss2::func()\n"
			                   "test (4, 21) : Error   : Function 'opPostInc()' not found\n"*/ )
			{
				PRINTF("%s", bout.buffer.c_str());
				TEST_FAILED;
			}
		}

		// mixin classes cannot implement constructors/destructors (at least not to start with)
		{
			script = 
				"mixin class Mix { \n"
				"  Mix() {} \n"
				"  ~Mix() {} \n"
				"} \n"
				"class Clss : Mix {} \n";
			mod->AddScriptSection("test", script);

			bout.buffer = "";
			r = mod->Build();
			if( r >= 0 )
				TEST_FAILED;
			if( bout.buffer != "test (2, 3) : Error   : Mixin classes cannot have constructors or destructors\n"
			                   "test (3, 3) : Error   : Mixin classes cannot have constructors or destructors\n" )
			{
				PRINTF("%s", bout.buffer.c_str());
				TEST_FAILED;
			}
		}

		// mixin class declared in a different section than the class that will use it
		{
			const char *script1 =
				"mixin class Mix { void func() {} }\n";
			const char *script2 =
				"class Clss : Mix {}\n";

			mod->AddScriptSection("mix", script1);
			mod->AddScriptSection("clss", script2);
			bout.buffer = "";
			r = mod->Build();
			if( r < 0 )
				TEST_FAILED;
			if( bout.buffer != "" )
			{
				PRINTF("%s", bout.buffer.c_str());
				TEST_FAILED;
			}
		}

		engine->Release();
	}

	// Success
	return fail;
}

} // namespace

