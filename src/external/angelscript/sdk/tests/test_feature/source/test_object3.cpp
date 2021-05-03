#include "utils.h"

namespace TestObject3
{

static const char * const TESTNAME = "TestObject3";

#if defined(__GNUC__) || defined(__psp2__)
#undef __cdecl
#define __cdecl
#endif
struct cFloat
{
	float m_Float;
	cFloat()
	{
		m_Float = 0.0f;
	}
	cFloat(float Float)
	{
		m_Float = Float;
	}
	float operator = (float f);
	operator float()
	{
		return m_Float;
	}
	float operator += (cFloat v );
	float operator += (float v );
	float operator -= (cFloat v );
	float operator -= (float v );
	float operator /= (cFloat v );
	float operator /= (float v );
	float operator *= (cFloat v );
	float operator *= (float v );
};

float cFloat::operator = (float f)
{
	float old = m_Float;
	UNUSED_VAR(old);
	m_Float = f;
	return m_Float;
}
float cFloat::operator += (cFloat v )
{
	float old = m_Float;
	UNUSED_VAR(old);
	m_Float += v;
	return m_Float;
}
float cFloat::operator += (float v )
{
	float old = m_Float;
	UNUSED_VAR(old);
	m_Float += v;
	return m_Float;
}
float cFloat::operator -= (cFloat v )
{
	float old = m_Float;
	UNUSED_VAR(old);
	m_Float -= v;
	return m_Float;
}
float cFloat::operator -= (float v )
{
	float old = m_Float;
	UNUSED_VAR(old);
	m_Float -= v;
	return m_Float;
}
float cFloat::operator /= (cFloat v )
{
	float old = m_Float;
	UNUSED_VAR(old);
	m_Float /= v;
	return m_Float;
}
float cFloat::operator /= (float v )
{
	float old = m_Float;
	UNUSED_VAR(old);
	m_Float /= v;
	return m_Float;
}
float cFloat::operator *= (cFloat v )
{
	float old = m_Float;
	UNUSED_VAR(old);
	m_Float *= v;
	return m_Float;
}
float cFloat::operator *= (float v )
{
	float old = m_Float;
	UNUSED_VAR(old);
	m_Float *= v;
	return m_Float;
}
cFloat & __cdecl AssignFloat2Float(float a,cFloat &b)
{
	b=a;
	return b;
}
float  __cdecl OpPlusRR(cFloat *self,  cFloat* other)
{
	return (float)(*self)+(float)(*other);
}
float  __cdecl OpPlusRF(cFloat *self,  float other)
{
	assert(self);

	return (float)(*self)+(float)(other);
}
float  __cdecl OpPlusFR(float self,  cFloat* other)
{
	return (self)+(float)(*other);
}

float  __cdecl OpMulRR(cFloat *self,  cFloat* other)
{
	return (float)(*self)*(float)(*other);
}
float  __cdecl OpMulRF(cFloat *self,  float other)
{
	return (float)(*self)*(float)(other);
}
float  __cdecl OpMulFR(float self,  cFloat* other)
{
	return (float)(self)*(float)(*other);
}

bool Register(asIScriptEngine*  pSE)
{
	pSE->RegisterObjectType("Float", sizeof(cFloat), asOBJ_VALUE | asOBJ_APP_CLASS);

	if(pSE->RegisterObjectMethod("Float","Float& opAssign(float )",asFUNCTION(AssignFloat2Float),asCALL_CDECL_OBJLAST))
		return false;


	// asBEHAVE_ADD
	if(pSE->RegisterObjectMethod("Float","float  opAdd(Float &in)",asFUNCTION(OpPlusRR),  asCALL_CDECL_OBJFIRST))
		return false;
	if(pSE->RegisterObjectMethod("Float","float  opAdd(float)",asFUNCTION(OpPlusRF),  asCALL_CDECL_OBJFIRST))
		return false;
	if(pSE->RegisterObjectMethod("Float","float  opAdd_r(float)",asFUNCTION(OpPlusFR),  asCALL_CDECL_OBJLAST))
		return false;

	// asBEHAVE_MULTIPLY
	if(pSE->RegisterObjectMethod("Float", "float  opMul(Float &in)",asFUNCTION(OpMulRR),  asCALL_CDECL_OBJFIRST))
		return false;
	if(pSE->RegisterObjectMethod("Float", "float  opMul(float)",asFUNCTION(OpMulRF),  asCALL_CDECL_OBJFIRST))
		return false;
	if(pSE->RegisterObjectMethod("Float", "float  opMul_r(float)",asFUNCTION(OpMulFR),  asCALL_CDECL_OBJLAST))
		return false;

	return true;
}

cFloat& Get(int index)
{
	static cFloat m_arr[10];
	return m_arr[index];
}
void Print(float f)
{
	assert(f == 30.0f);
//	PRINTF("%f\n", f);
}

bool Test()
{
	asIScriptEngine*        pSE;
	pSE=asCreateScriptEngine(ANGELSCRIPT_VERSION);
	Register(pSE);
	pSE->RegisterGlobalFunction("Float& Get(int32)",asFUNCTION(Get),asCALL_CDECL);
	pSE->RegisterGlobalFunction("void Print(float)",asFUNCTION(Print),asCALL_CDECL);

	const char script[]="\
						   float ret=10;\n\
						   Get(0)=10.0f;\n\
						   Get(1)=10.0f;\n\
						   Get(2)=10.0f;\n\
						   ret=Get(0)+(Get(1)*2.0f);\n\
						   Print(ret);\n\
						   \n";
	ExecuteString(pSE, script);

	pSE->Release();

	return false;
}


} // namespace

