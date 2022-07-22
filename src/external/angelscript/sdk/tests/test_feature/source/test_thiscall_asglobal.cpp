//
// This test was designed to test the asOBJ_CLASS flag with THISCALL
//
// Author: Andreas Jonsson
//

#include "utils.h"

namespace TestThiscallAsGlobal
{

static const char * const TESTNAME = "TestThiscallAsGlobal";

class Class1
{
public:
	void TestMe(asDWORD newValue) { a = newValue; }
	asDWORD a;
};

class Base
{
public:
	virtual void Print() { str = "Called from Base"; }

	std::string str;
};

class Derived : public Base
{
public:
	virtual void Print() { str = "Called from Derived"; }
};


class PointF
{
public:
	PointF() {x = 3.14f; y = 42.0f;};
	PointF( const PointF &other) { x = other.x; y = other.y; };

	static void ConstructorDefaultPointF(PointF *m) { new(m) PointF(); };
	static void DestructorPointF(PointF *) {  };

	float x, y;
};

class Renderer
{
public:
	Renderer() { allOK = false; }
	void DrawSprite(int, int, PointF p) { if( p.x == 3.14f && p.y == 42.0f ) allOK = true; }
	bool allOK; 
};

class A {
public:
	int SomeWorkA(int a) {
		UNUSED_VAR(a);
	//	printf("A::SomeWorkA | %lx | %i\n", this, aa);
		return aa;
	}
	int aa = 33;
};
class B {
public:
	int SomeWorkB(int b) {
		UNUSED_VAR(b);
	//	printf("B::SomeWorkB | %lx | %i\n", this, bb);
		return bb;
	}
	int bb = 44;
};

class C: public A, public B {
};

bool Test()
{
	RET_ON_MAX_PORT

	bool fail = false;
	int r;
	asIScriptEngine *engine;
	COutStream out;
	CBufferedOutStream bout;

	// THISCALL_ASGLOBAL with multiple inheritance
	// https://www.gamedev.net/forums/topic/702126-angelscript-2331-bugs-features/
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);
		engine->RegisterGlobalFunction("void assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);
		
		C* c = new C;
		r = engine->RegisterGlobalFunction("int someWork(int)", asMETHODPR(C, SomeWorkB, (int), int), asCALL_THISCALL_ASGLOBAL, c); assert(r >= 0);
		
		r = ExecuteString(engine, "assert( someWork(42) == 44 );");
		if( r != asEXECUTION_FINISHED )
			TEST_FAILED;
		
		delete c;
		
		engine->Release();
	}
	
	// Simple THISCALL_ASGLOBAL tests
	{
		engine = asCreateScriptEngine();
		engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

		Class1 c1;
		engine->RegisterGlobalFunction("void TestMe(uint val)", asMETHOD(Class1, TestMe), asCALL_THISCALL_ASGLOBAL, &c1);

		c1.a = 0;
		r = ExecuteString(engine, "TestMe(0xDEADC0DE);");
		if( r < 0 )
		{
			PRINTF("%s: ExecuteString() failed %d\n", TESTNAME, r);
			TEST_FAILED;
		}

		if( c1.a != 0xDEADC0DE )
		{
			PRINTF("Class member wasn't updated correctly\n");
			TEST_FAILED;
		}

		// Register and call a derived method
		Base *obj = new Derived();
		engine->RegisterGlobalFunction("void Print()", asMETHOD(Base, Print), asCALL_THISCALL_ASGLOBAL, obj);

		r = ExecuteString(engine, "Print()");
		if( r < 0 )
			TEST_FAILED;

		if( obj->str != "Called from Derived" )
			TEST_FAILED;

		delete obj;
		engine->Release();
	}

	// It must not be possible to register without the object pointer
	{
		engine = asCreateScriptEngine();
		bout.buffer = "";
		engine->SetMessageCallback(asMETHOD(CBufferedOutStream, Callback), &bout, asCALL_THISCALL);
		r = engine->RegisterGlobalFunction("void Fail()", asMETHOD(Class1, TestMe), asCALL_THISCALL_ASGLOBAL, 0);
		if( r != asINVALID_ARG )
			TEST_FAILED;
		if( bout.buffer != " (0, 0) : Error   : Failed in call to function 'RegisterGlobalFunction' with 'void Fail()' (Code: asINVALID_ARG, -5)\n" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}
		engine->Release();
	}

	// Test proper clean-up args passed by value
	// http://www.gamedev.net/topic/670017-weird-crash-in-userfree-on-android/
	{
		engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
		bout.buffer = "";
		r = engine->RegisterObjectType("PointF", sizeof(PointF), asOBJ_VALUE|asOBJ_POD|asOBJ_APP_CLASS_CK); assert( r >= 0);
		r = engine->RegisterObjectBehaviour( "PointF", asBEHAVE_CONSTRUCT, "void f()", asFUNCTION(PointF::ConstructorDefaultPointF), asCALL_CDECL_OBJLAST); assert( r >= 0);
		r = engine->RegisterObjectBehaviour( "PointF", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(PointF::DestructorPointF), asCALL_CDECL_OBJLAST); assert( r >= 0);

		Renderer render;
		r = engine->RegisterGlobalFunction( "void DrawSprite( int,int,PointF)", asMETHOD(Renderer, DrawSprite), asCALL_THISCALL_ASGLOBAL, &render); assert(r >= 0);
	
		r = ExecuteString(engine, "PointF p; DrawSprite(1,2,p);");
		if( r < 0 )
			TEST_FAILED;

		if( !render.allOK )
			TEST_FAILED;

		if( bout.buffer != "" )
		{
			PRINTF("%s", bout.buffer.c_str());
			TEST_FAILED;
		}

		engine->Release();
	}

	return fail;
}

} // namespace
