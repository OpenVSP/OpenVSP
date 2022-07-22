#include "utils.h"

namespace TestVector3_2
{

static const char * const TESTNAME = "TestVector3_2";

class csVector3
{
public:
	inline csVector3() {}
	inline csVector3(const csVector3 &o) : x(o.x), y(o.y), z(o.z) {}
	inline csVector3( float fX, float fY, float fZ ) : x( fX ), y( fY ), z( fZ ) {}

	/// Multiply this vector by a scalar.
	inline csVector3& operator*= (float f)
	{ x *= f; y *= f; z *= f; return *this; }

	/// Divide this vector by a scalar.
	inline csVector3& operator/= (float f)
	{ f = 1.0f / f; x *= f; y *= f; z *= f; return *this; }

	/// Add another vector to this vector.
	inline csVector3& operator+= (const csVector3& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		return *this;
	}

	/// Subtract another vector from this vector.
	inline csVector3& operator-= (const csVector3& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;
		return *this;
	}

	float x,y,z;
};

/// Add two vectors.
inline csVector3 operator+(const csVector3& v1, const csVector3& v2)
{ return csVector3(v1.x+v2.x, v1.y+v2.y, v1.z+v2.z); }

/// Subtract two vectors.
inline csVector3 operator-(const csVector3& v1, const csVector3& v2)
{ return csVector3(v1.x-v2.x, v1.y-v2.y, v1.z-v2.z); }

/// Multiply a vector and a scalar.
inline csVector3 operator* (const csVector3& v, float f)
{ return csVector3(v.x*f, v.y*f, v.z*f); }

/// Multiply a vector and a scalar.
inline csVector3 operator* (float f, const csVector3& v)
{ return csVector3(v.x*f, v.y*f, v.z*f); }

/// Divide a vector by a scalar int.
inline csVector3 operator/ (const csVector3& v, float f)
{ return v * (1/(float)f); }

void ConstructVector3(csVector3*o)
{
	new(o) csVector3;
}

void ConstructVector3(float a, float b, float c, csVector3*o)
{
	new(o) csVector3(a,b,c);
}


const char *script1 =
"void func() {\n"
"  Vector3 meshOrigin, sideVector, frontVector;\n"
"  float stepLength = 0;\n"
"  if(true) {\n"
"    Vector3 newPos1 = meshOrigin + sideVector*0.1f + frontVector*stepLength;\n"
"  }\n"
"}\n";



bool Test()
{
	RET_ON_MAX_PORT

	bool fail = false;
	int r;
	COutStream out;

	asIScriptEngine *engine = asCreateScriptEngine(ANGELSCRIPT_VERSION);
	engine->SetMessageCallback(asMETHOD(COutStream,Callback), &out, asCALL_THISCALL);

    r = engine->RegisterObjectType ("Vector3", sizeof (csVector3), asOBJ_VALUE | asOBJ_POD | asOBJ_APP_CLASS_CK); assert( r >= 0 );
    r = engine->RegisterObjectProperty ("Vector3", "float x", asOFFSET(csVector3, x)); assert( r >= 0 );
    r = engine->RegisterObjectProperty ("Vector3", "float y", asOFFSET(csVector3, y)); assert( r >= 0 );
    r = engine->RegisterObjectProperty ("Vector3", "float z", asOFFSET(csVector3, z)); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour ("Vector3", asBEHAVE_CONSTRUCT, "void f()", asFUNCTIONPR(ConstructVector3, (csVector3*), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
    r = engine->RegisterObjectBehaviour ("Vector3", asBEHAVE_CONSTRUCT, "void f(float, float, float)", asFUNCTIONPR(ConstructVector3, (float, float, float, csVector3*), void), asCALL_CDECL_OBJLAST); assert( r >= 0 );
//    r = engine->RegisterObjectMethod ("Vector3", "float Length()", asMETHODPR(csVector3, Norm, (void) const, float), asCALL_THISCALL); assert( r >= 0 );
//    r = engine->RegisterObjectMethod ("Vector3", "void Normalize()", asMETHOD(csVector3, Normalize), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectMethod ("Vector3", "Vector3 opAdd(const Vector3 &in) const", asFUNCTIONPR(operator+, (const csVector3&, const csVector3&), csVector3), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectMethod ("Vector3", "Vector3 opSub(const Vector3 &in) const", asFUNCTIONPR(operator-, (const csVector3&, const csVector3&), csVector3), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectMethod ("Vector3", "Vector3 opMul(float) const", asFUNCTIONPR(operator*, (const csVector3&, float), csVector3), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectMethod ("Vector3", "Vector3 opMul_r(float) const", asFUNCTIONPR(operator*, (float, const csVector3&), csVector3), asCALL_CDECL_OBJLAST); assert( r >= 0 );
    r = engine->RegisterObjectMethod ("Vector3", "Vector3 opDiv(float) const", asFUNCTIONPR(operator/, (const csVector3&, float), csVector3), asCALL_CDECL_OBJFIRST); assert( r >= 0 );
    r = engine->RegisterObjectMethod ("Vector3", "Vector3 &opAddAssign(Vector3 &in)", asMETHODPR(csVector3, operator+=, (const csVector3&), csVector3&), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectMethod ("Vector3", "Vector3 &opSubAssign(Vector3 &in)", asMETHODPR(csVector3, operator+=, (const csVector3&), csVector3&), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectMethod ("Vector3", "Vector3 &opMulAssign(float)", asMETHODPR(csVector3, operator*=, (float), csVector3&), asCALL_THISCALL); assert( r >= 0 );
    r = engine->RegisterObjectMethod ("Vector3", "Vector3 &opDivAssign(float)", asMETHODPR(csVector3, operator/=, (float), csVector3&), asCALL_THISCALL); assert( r >= 0 );


	asIScriptModule *mod = engine->GetModule(0, asGM_ALWAYS_CREATE);
	mod->AddScriptSection(TESTNAME, script1);
	r = mod->Build();
	if( r < 0 )
	{
		PRINTF("%s: Failed to build\n", TESTNAME);
		TEST_FAILED;
	}
	else
	{
		// Internal return
		r = ExecuteString(engine, "func()", mod);
		if( r < 0 )
		{
			PRINTF("%s: ExecuteString() failed %d\n", TESTNAME, r);
			TEST_FAILED;
		}
	}

	engine->Release();

	return fail;
}

}
