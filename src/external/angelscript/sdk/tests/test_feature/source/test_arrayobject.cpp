#include <memory>

#include "stdvector.h"
#include "utils.h"

using namespace std;

namespace TestArrayObject
{

static const char * const TESTNAME = "TestArrayObject";

class CIntArray
{
public:
	CIntArray() 
	{
		length = 0; 
		buffer = new int[0];
	} 
	CIntArray(int l) 
	{
		length=l; 
		buffer = new int[l];
	}
	CIntArray(const CIntArray &other)
	{
		length = other.length;
		buffer = new int[length];
		for( int n = 0; n < length; n++ )
			buffer[n] = other.buffer[n];
	}
	~CIntArray() 
	{
		delete[] buffer;
	}

	CIntArray &operator=(const CIntArray &other) 
	{
		delete[] buffer; 
		length = other.length; 
		buffer = new int[length]; 
		memcpy(buffer, other.buffer, length*4); 
		return *this;
	}

	int size() {return length;}
	void push_back(int &v) 
	{
		int *b = new int[length+1]; 
		memcpy(b, buffer, length*4); 
		delete[] buffer; 
		buffer = b; 
		b[length++] = v;
	}
	int pop_back() 
	{
		return buffer[--length];
	}
	int &operator[](int i) 
	{
		return buffer[i];
	}

	int length;
	int *buffer;
};

void ConstructIntArray(CIntArray *a)
{
	new(a) CIntArray();
}

void ConstructIntArray(int l, CIntArray *a)
{
	new(a) CIntArray(l);
}

void DestructIntArray(CIntArray *a)
{
	a->~CIntArray();
}

class CIntArrayArray
{
public:
	CIntArrayArray() 
	{
		length = 0; 
		buffer = new CIntArray[0];
	} 
	CIntArrayArray(int l) 
	{
		length=l; 
		buffer = new CIntArray[l];
	}
	CIntArrayArray(const CIntArrayArray &other) 
	{
		length = other.length;
		buffer = new CIntArray[length];
		for( int n = 0; n < length; n++ )
			buffer[n] = other.buffer[n];
	}
	~CIntArrayArray() 
	{
		delete[] buffer;
	}

	CIntArrayArray &operator=(CIntArrayArray &other) 
	{
		delete[] buffer; 
		length = other.length; 
		buffer = new CIntArray[length]; 
		for( int n = 0; n < length; n++ )
			buffer[n] = other.buffer[n];
		return *this;
	}

	int size() {return length;}
	void push_back(CIntArray &v) 
	{
		CIntArray *b = new CIntArray[length+1]; 
		for( int n = 0; n < length; n++ )
			b[n] = buffer[n];
		delete[] buffer; 
		buffer = b; 
		b[length++] = v;
	}
	CIntArray pop_back() 
	{
		return buffer[--length];
	}
	CIntArray &operator[](int i) 
	{
		return buffer[i];
	}

	int length;
	CIntArray *buffer;
};

void ConstructIntArrayArray(CIntArrayArray *a)
{
	new(a) CIntArrayArray();
}

void ConstructIntArrayArray(int l, CIntArrayArray *a)
{
	new(a) CIntArrayArray(l);
}

void DestructIntArrayArray(CIntArrayArray *a)
{
	a->~CIntArrayArray();
}

static const char *script1 =
"void Test()                         \n"
"{                                   \n"
"   TestInt();                       \n"
"   Test2D();                        \n"
"}                                   \n"
"                                    \n"
"void TestInt()                      \n"
"{                                   \n"
"   int[] A(5);                      \n"
"   Assert(A.size() == 5);           \n"
"	A.push_back(6);                  \n"
"   Assert(A.size() == 6);           \n"
"	A.pop_back();                    \n"
"   Assert(A.size() == 5);           \n"
"	A[1] = 20;                       \n"
"	Assert(A[1] == 20);              \n"
"   char[] B(5);                     \n"
"   Assert(B.size() == 5);           \n"
// TODO: Add support for initialization list for value types as well
//"   int[] c = {2,3};                 \n"
//"   Assert(c.size() == 2);           \n"
"}                                   \n"
"                                    \n"
"void Test2D()                       \n"
"{                                   \n"
"   int[][] A(2);                    \n"
"   int[] B(2);                      \n"
"   A[0] = B;                        \n"
"   A[1] = B;                        \n"
"                                    \n"
"   A[0][0] = 0;                     \n"
"   A[0][1] = 1;                     \n"
"   A[1][0] = 2;                     \n"
"   A[1][1] = 3;                     \n"
"                                    \n"
"   Assert(A[0][0] == 0);            \n"
"   Assert(A[0][1] == 1);            \n"
"   Assert(A[1][0] == 2);            \n"
"   Assert(A[1][1] == 3);            \n"
"}                                   \n";

using namespace std;

bool Test2();

bool Test()
{
	RET_ON_MAX_PORT

	bool fail = Test2();
	int r;

 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	RegisterScriptArray(engine, true);

	// Verify that it is possible to register arrays of registered types
	r = engine->RegisterObjectType("char", sizeof(int), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_PRIMITIVE); assert( r>= 0 );
	r = engine->RegisterObjectType( "char[]", sizeof(CIntArray), asOBJ_VALUE | asOBJ_APP_CLASS_CDA ); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("char[]", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DestructIntArray), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("char[]", asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(ConstructIntArray, (CIntArray *), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("char[]", asBEHAVE_CONSTRUCT, "void f(int)", asFUNCTIONPR(ConstructIntArray, (int, CIntArray *), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("char[]", "int &opIndex(int)", asMETHOD(CIntArray, operator[]), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("char[]", "int size()", asMETHOD(CIntArray, size), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectType("char[][]", sizeof(CIntArrayArray), asOBJ_VALUE | asOBJ_APP_CLASS_CDA); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("char[][]", asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(ConstructIntArrayArray, (CIntArrayArray *), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("char[][]", asBEHAVE_CONSTRUCT, "void f(int)", asFUNCTIONPR(ConstructIntArrayArray, (int, CIntArrayArray *), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("char[][]", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DestructIntArrayArray), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("char[][]", "int size()", asMETHOD(CIntArrayArray, size), asCALL_THISCALL); assert( r >= 0 );

	// Verify that it is possible to register arrays of built-in types
	r = engine->RegisterObjectType("int[]", sizeof(CIntArray), asOBJ_VALUE | asOBJ_APP_CLASS_CDA); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("int[]", asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(ConstructIntArray, (CIntArray *), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("int[]", asBEHAVE_CONSTRUCT, "void f(int)", asFUNCTIONPR(ConstructIntArray, (int, CIntArray *), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("int[]", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DestructIntArray), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("int[]", "int[] &opAssign(int[]&in)", asMETHOD(CIntArray, operator=), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("int[]", "int &opIndex(int)", asMETHOD(CIntArray, operator[]), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("int[]", "int size()", asMETHOD(CIntArray, size), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("int[]", "void push_back(int &in)", asMETHOD(CIntArray, push_back), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("int[]", "int pop_back()", asMETHOD(CIntArray, pop_back), asCALL_THISCALL); assert( r >= 0 );

//	RegisterVector<CIntArray>("int[][]", "int[]", engine);

	r = engine->RegisterObjectType("int[][]", sizeof(CIntArrayArray), asOBJ_VALUE | asOBJ_APP_CLASS_CDA); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("int[][]", asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(ConstructIntArrayArray, (CIntArrayArray *), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("int[][]", asBEHAVE_CONSTRUCT, "void f(int)", asFUNCTIONPR(ConstructIntArrayArray, (int, CIntArrayArray *), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("int[][]", asBEHAVE_DESTRUCT, "void f()", asFUNCTION(DestructIntArrayArray), asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("int[][]", "int[][] &opAssign(int[][]&in)", asMETHOD(CIntArrayArray, operator=), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("int[][]", "int[] &opIndex(int)", asMETHOD(CIntArrayArray, operator[]), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("int[][]", "int size()", asMETHOD(CIntArrayArray, size), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("int[][]", "void push_back(int[] &in)", asMETHOD(CIntArrayArray, push_back), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("int[][]", "int[] pop_back()", asMETHOD(CIntArrayArray, pop_back), asCALL_THISCALL); assert( r >= 0 );


	engine->RegisterGlobalFunction("void Assert(bool)", asFUNCTION(Assert), asCALL_GENERIC);

	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1, strlen(script1), 0);
	r = mod->Build();
	if( r < 0 )
	{
		TEST_FAILED;
		PRINTF("%s: Failed to compile the script\n", TESTNAME);
	}


	asIScriptContext *ctx = engine->CreateContext();
	r = ExecuteString(engine, "Test()", mod, ctx);
	if( r != asEXECUTION_FINISHED )
	{
		PRINTF("%s: Failed to execute script\n", TESTNAME);

		if( r == asEXECUTION_EXCEPTION )
			PRINTF("%s", GetExceptionInfo(ctx).c_str());
		
		TEST_FAILED;
	}
	
	if( ctx ) ctx->Release();

	
	// Test a compile error, to make sure the error is reported as expected
	CBufferedOutStream bout;
	engine->SetMessageCallback(asMETHOD(CBufferedOutStream,Callback), &bout, asCALL_THISCALL);
	r = ExecuteString(engine, "char[] t; t[0][0] = 1;");
	if( r >= 0 )
	{
		TEST_FAILED;
	}
	if( bout.buffer != "ExecuteString (1, 15) : Error   : Type 'int&' doesn't support the indexing operator\n" )
	{
		PRINTF("%s", bout.buffer.c_str());
		TEST_FAILED;
	}
	

	engine->Release();

	// Success
	return fail;
}

//---------------------------------------------------
// Reported by dxj19831029
class Value {
public:
	Value() {
		a = 4;
	}
	// Add copy constructor to work on gcc 64bit
	Value(const Value &v) : a(v.a) {}
	int a;
};
static Value v_static;
class A {
public:
	A() {}
	A(const A &) {}
	Value &operator[] (int ) {
		return v_static;
	}
};
class AArray {
public:
	AArray() {}
	AArray(const AArray &) {}
	A operator[] (int ) {
		return A();
	}
};
class AArrayArray {
public:
	AArrayArray() {}
	AArrayArray(const AArrayArray &) {}
	AArray operator[] (int ) {
		return AArray();
	}
};


bool Test2()
{
	int nRet;
	bool fail = false;
	COutStream out;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream, Callback), &out, asCALL_THISCALL);
	RegisterScriptArray(engine, true);

	nRet = engine->RegisterObjectType("Value", sizeof(Value),    asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CK);	assert( nRet >= 0 );

	nRet = engine->RegisterObjectType("A", sizeof(A),    asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CK);	assert( nRet >= 0 );
	nRet = engine->RegisterObjectMethod("A", "Value &opIndex(int)", asMETHODPR(A, operator[], (int ), Value &), asCALL_THISCALL); 	assert( nRet >= 0 );
	nRet = engine->RegisterObjectType("A[]", sizeof(AArray),    asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CK);	assert( nRet >= 0 );
	nRet = engine->RegisterObjectMethod("A[]", "A opIndex( int )", asMETHODPR(AArray, operator[], (int ), A), asCALL_THISCALL); 	assert( nRet >= 0 );
	nRet = engine->RegisterObjectType("A[][]", sizeof(AArrayArray),    asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CK);	assert( nRet >= 0 );
	nRet = engine->RegisterObjectMethod("A[][]", "A[] opIndex(int)", asMETHODPR(AArrayArray, operator[], (int ), AArray), asCALL_THISCALL); 	assert( nRet >= 0 );

	nRet = ExecuteString(engine, "A[][] f;");	assert( nRet >= 0 );
	nRet = ExecuteString(engine, "A[][] f; f[0];");	assert( nRet >= 0 );
	nRet = ExecuteString(engine, "A[][] f; f[0][0];");	assert( nRet >= 0 );
	nRet = ExecuteString(engine, "A[][] f; f[0][0][0];");	assert( nRet >= 0 );

	engine->Release();

	return fail;
}

} // namespace

