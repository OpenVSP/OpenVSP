//
// Test author: Andreas Jonsson
//

#include "utils.h"
#include <assert.h>
#include <string>
#include <vector>

using namespace std;

namespace TestStringPooled
{


class CScriptString2
{
public:
	CScriptString2();
	CScriptString2(const char *s, unsigned int len);
	CScriptString2(const std::string &s);
	~CScriptString2();

	void AddRef() const;
	void Release() const;

	CScriptString2 &operator=(const CScriptString2 &other);
	CScriptString2 &operator+=(const CScriptString2 &other);
	friend CScriptString2 *operator+(const CScriptString2 &a, const CScriptString2 &b);

	std::string buffer;

protected:
	mutable int refCount;
};

static vector<CScriptString2*> pool;

CScriptString2::CScriptString2()
{
	// Count the first reference
	refCount = 1;
}

CScriptString2::CScriptString2(const char *s, unsigned int len)
{
	refCount = 1;
	buffer.assign(s, len);
}

CScriptString2::CScriptString2(const string &s)
{
	refCount = 1;
	buffer = s;
}

CScriptString2::~CScriptString2()
{
	assert( refCount == 0 );
}

void CScriptString2::AddRef() const
{
	refCount++;
}

void CScriptString2::Release() const
{
	if( --refCount == 0 )
	{
		// Move this to the pool
		pool.push_back(const_cast<CScriptString2*>(this));
	}
}

CScriptString2 &CScriptString2::operator=(const CScriptString2 &other)
{
	// Copy only the buffer, not the reference counter
	buffer = other.buffer;

	// Return a reference to this object
	return *this;
}

CScriptString2 *operator+(const CScriptString2 &a, const CScriptString2 &b)
{
	if( pool.size() > 0 )
	{
		CScriptString2 *str = pool[pool.size()-1];
		pool.pop_back();
		str->buffer.resize(a.buffer.length() + b.buffer.length());
		str->buffer += a.buffer;
		str->buffer += b.buffer;
		str->AddRef();
		return str;
	}

	// Return a new object as a script handle
	return new CScriptString2(a.buffer + b.buffer);
}

class CStringPooledFactory : public asIStringFactory
{
public:
	CStringPooledFactory() {}
	~CStringPooledFactory() {}
	const void *GetStringConstant(const char *data, asUINT length)
	{
		if (pool.size() > 0)
		{
			CScriptString2 *str = pool[pool.size() - 1];
			pool.pop_back();
			str->buffer.assign(data, length);
			str->AddRef();
			return str;
		}

		return new CScriptString2(data, length);
	}

	int ReleaseStringConstant(const void *str)
	{
		reinterpret_cast<const CScriptString2*>(str)->Release();
		return 0;
	}

	int GetRawStringData(const void *str, char *data, asUINT *length) const
	{
		if (length)
			*length = (asUINT)reinterpret_cast<const CScriptString2*>(str)->buffer.length();
		if (data)
			memcpy(data, reinterpret_cast<const CScriptString2*>(str)->buffer.c_str(), reinterpret_cast<const CScriptString2*>(str)->buffer.length());

		return 0;
	}
};

CStringPooledFactory stringPooledFactory;

// This is the default string factory, that is responsible for creating empty string objects, e.g. when a variable is declared
static CScriptString2 *StringDefaultFactory()
{
	if( pool.size() > 0 )
	{
		CScriptString2 *str = pool[pool.size()-1];
		pool.pop_back();
		str->buffer.resize(0);
		str->AddRef();
		return str;
	}

	// Allocate and initialize with the default constructor
	return new CScriptString2();
}

// Copy constructor
static CScriptString2 *StringCopyFactory(const string &s)
{
	if( pool.size() > 0 )
	{
		CScriptString2 *str = pool[pool.size()-1];
		pool.pop_back();
		str->buffer.assign(s);
		str->AddRef();
		return str;
	}

	// Allocate and initialize with the default constructor
	return new CScriptString2(s);
}

// This is where we register the string type
void RegisterScriptString2(asIScriptEngine *engine)
{
	int r;

	// Register the type
	r = engine->RegisterObjectType("string", 0, asOBJ_REF); assert( r >= 0 );

	// Register the object operator overloads
	// Note: We don't have to register the destructor, since the object uses reference counting
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_FACTORY,    "string @f()",                 asFUNCTION(StringDefaultFactory), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_FACTORY,    "string @f(const string &in)", asFUNCTION(StringCopyFactory), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_ADDREF,     "void f()",                    asMETHOD(CScriptString2,AddRef), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("string", asBEHAVE_RELEASE,    "void f()",                    asMETHOD(CScriptString2,Release), asCALL_THISCALL); assert( r >= 0 );
	r = engine->RegisterObjectMethod("string", "string &opAssign(const string &in)", asMETHODPR(CScriptString2, operator =, (const CScriptString2&), CScriptString2&), asCALL_THISCALL); assert( r >= 0 );

	// Register the factory to return a handle to a new string
	// Note: We must register the string factory after the basic behaviours,
	// otherwise the library will not allow the use of object handles for this type
	r = engine->RegisterStringFactory("string", &stringPooledFactory); assert(r >= 0);

	r = engine->RegisterObjectMethod("string", "string@ opAdd(const string &in) const", asFUNCTIONPR(operator +, (const CScriptString2 &, const CScriptString2 &), CScriptString2*), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
}


#define TESTNAME "TestStringPooled"

static const char *script =
"string BuildStringP(string &in a, string &in b, string &in c)    \n"
"{                                                                \n"
"    return a + b + c;                                            \n"
"}                                                                \n"
"                                                                 \n"
"void TestStringP()                                               \n"
"{                                                                \n"
"    string a = \"Test\";                                         \n"
"    string b = \"string\";                                       \n"
"    int i = 0;                                                   \n"
"                                                                 \n"
"    for ( i = 0; i < 1000000; i++ )                              \n"
"    {                                                            \n"
"        string res = BuildStringP(a, \" \", b);                  \n"
"    }                                                            \n"
"}                                                                \n";

                                         
void Test(double *testTime)
{
 	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	COutStream out;
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

	RegisterScriptString2(engine);

	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script, strlen(script), 0);
	int r = mod->Build();
	if( r >= 0 )
	{
#ifndef _DEBUG
		asIScriptContext *ctx = engine->CreateContext();
		ctx->Prepare(mod->GetFunctionByDecl("void TestStringP()"));

		double time = GetSystemTimer();

		r = ctx->Execute();

		time = GetSystemTimer() - time;

		if( r != 0 )
		{
			printf("Execution didn't terminate with asEXECUTION_FINISHED\n");
			if( r == asEXECUTION_EXCEPTION )
			{
				printf("Script exception\n");
				asIScriptFunction *func = ctx->GetExceptionFunction();
				printf("Func: %s\n", func->GetName());
				printf("Line: %d\n", ctx->GetExceptionLineNumber());
				printf("Desc: %s\n", ctx->GetExceptionString());
			}
		}
		else
			*testTime = time;

		ctx->Release();
#endif
	}
	else
		printf("Build failed\n");
	engine->Release();

	// Clean up the string pool
	for( asUINT n = 0; n < pool.size(); n++ )
		delete pool[n];
	pool.clear();
}

} // namespace







