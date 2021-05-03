#if !defined(__CELLOS_LV2__) && !defined(__psp2__)
#	include <memory.h> // memcpy(), memcmp()
#endif
#include <string.h> // some compilers declare memcmp() here
#include <assert.h> // assert()
#include <stdio.h>  // sprintf()
#include "bstr.h"

#if AS_USE_NAMESPACE
namespace AngelScript {
#endif

void    asBStrConstruct(asBSTR *s);
void    asBStrDestruct(asBSTR *s);
asBSTR *asBStrCopy(const asBSTR *src, asBSTR *dst);
asBSTR *asBStrAppend(const asBSTR *other, asBSTR *self);
asUINT  asBStrLengthMethod(const asBSTR *s);
asBYTE *asBStrByteAt(int index, const asBSTR *s);

class CBStrFactory : public asIStringFactory
{
public:
	const void *GetStringConstant(const char *data, asUINT length)
	{
		asBSTR *str = new asBSTR;
		*str = asBStrAlloc(length);
		memcpy(*str, data, length);
		return str;
	}

	int ReleaseStringConstant(const void *str)
	{
		asBStrFree(*reinterpret_cast<asBSTR*>(const_cast<void*>(str)));
		delete reinterpret_cast<asBSTR*>(const_cast<void*>(str));
		return 0;
	}

	int GetRawStringData(const void *str, char *data, asUINT *length) const
	{
		if (length) *length = asBStrLength(*reinterpret_cast<asBSTR*>(const_cast<void*>(str)));
		if (data) memcpy(data, *reinterpret_cast<void**>(const_cast<void*>(str)), asBStrLength(*reinterpret_cast<asBSTR*>(const_cast<void*>(str))));
		return 0;
	}
} bstrFactory;

void RegisterBStr(asIScriptEngine *engine)
{
	int r;

	// Register the bstr type
	r = engine->RegisterObjectType("bstr", sizeof(asBSTR), asOBJ_VALUE | asOBJ_APP_PRIMITIVE); assert( r >= 0 );

	// Register the bstr factory
	r = engine->RegisterStringFactory("bstr", &bstrFactory); assert(r >= 0);

	// Register the object methods
	r = engine->RegisterObjectMethod("bstr", "uint length() const", asFUNCTION(asBStrLengthMethod), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	// Register the object operator overloads
	r = engine->RegisterObjectBehaviour("bstr", asBEHAVE_CONSTRUCT,  "void f()",                  asFUNCTION(asBStrConstruct),   asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectBehaviour("bstr", asBEHAVE_DESTRUCT,   "void f()",                  asFUNCTION(asBStrDestruct),    asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("bstr",                         "bstr &opAssign(const bstr &in)",   asFUNCTION(asBStrCopy),        asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("bstr",                         "bstr &opAddAssign(const bstr &in)",   asFUNCTION(asBStrAppend),      asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("bstr",                         "uint8 &opIndex(int)",             asFUNCTION(asBStrByteAt),      asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("bstr",                         "const uint8 &opIndex(int) const", asFUNCTION(asBStrByteAt),      asCALL_CDECL_OBJLAST); assert( r >= 0 );
	r = engine->RegisterObjectMethod("bstr",                         "bstr f(bstr &in)",          asFUNCTION(asBStrConcatenate), asCALL_CDECL_OBJLAST); assert( r >= 0 );

	// Register the global operator overloads
	r = engine->RegisterObjectMethod("bstr",       "int opCmp(const bstr &in) const", asFUNCTION(asBStrCompare), asCALL_CDECL_OBJFIRST); assert( r >= 0 );

	// Register useful functions
	r = engine->RegisterGlobalFunction("bstr bstrSubstr(bstr &in, uint, uint)", asFUNCTION(asBStrSubstr),                           asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("bstr bstrFormat(int)",                  asFUNCTION((asBSTR (*)(int))asBStrFormat),          asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("bstr bstrFormat(uint)",                 asFUNCTION((asBSTR (*)(unsigned int))asBStrFormat), asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("bstr bstrFormat(float)",                asFUNCTION((asBSTR (*)(float))asBStrFormat),        asCALL_CDECL); assert( r >= 0 );
	r = engine->RegisterGlobalFunction("bstr bstrFormat(double)",               asFUNCTION((asBSTR (*)(double))asBStrFormat),       asCALL_CDECL); assert( r >= 0 );
}

asBSTR asBStrAlloc(asUINT length)
{
	unsigned char *str = new unsigned char[length+4+1];
	*((asUINT*)str)  = length; // Length of string
	str[length+4] = 0;      // Null terminated

	return str + 4;
}

void asBStrFree(asBSTR str)
{
	if( str == 0 ) return;

	unsigned char *p = str-4;
	delete[] p;
}

asUINT asBStrLength(asBSTR str)
{
	if( str == 0 ) return 0;

	unsigned char *p = str-4;

	return *(asUINT*)(p);
}

asUINT asBStrLengthMethod(const asBSTR *str)
{
	if( str == 0 ) return 0;

	return asBStrLength(*str);
}

void asBStrConstruct(asBSTR *s)
{
	// Set the pointer to 0
	*s = 0;
}

void asBStrDestruct(asBSTR *s)
{
	// Free the bstr
	asBStrFree(*s);
}

asBSTR *asBStrCopy(const asBSTR *src, asBSTR *dst)
{
	// Free the destination bstr
	asBStrFree(*dst);

	// Make a copy of the source bstr
	int len = asBStrLength(*src);
	*dst = asBStrAlloc(len);
	memcpy(*dst, *src, len);

	return dst;
}

asBSTR *asBStrAppend(const asBSTR *other, asBSTR *self)
{
	asBSTR concat = asBStrConcatenate(self, other);

	// Free the original bstr
	asBStrFree(*self);

	*self = concat;

	return self;
}

asBSTR asBStrConcatenate(const asBSTR *left, const asBSTR *right)
{
	int ll = asBStrLength(*left);
	int lr = asBStrLength(*right);

	// Allocate a new string for the concatenation
	asBSTR str = asBStrAlloc(ll + lr);

	memcpy(str, *left, ll);
	memcpy(str+ll, *right, lr);

	return str;
}

int asBStrCompare(const asBSTR *s1, const asBSTR *s2)
{
	if( *s1 == 0 && *s2 == 0 ) return 0;
	if( *s1 == 0 ) return -1;
	if( *s2 == 0 ) return 1;

	asUINT l1 = asBStrLength(*s1);
	asUINT l2 = asBStrLength(*s2);

	asUINT len = l1 < l2 ? l1 : l2;

	int cmp = memcmp(*s1, *s2, len);

	if( cmp == 0 )
	{
		if( l1 == l2 )
			return 0;
		if( l1 < l2 )
			return -1;
		else
			return 1;
	}

	return cmp;
}

asBYTE *asBStrByteAt(int index, const asBSTR *s)
{
	if( index < 0 || index >= (signed)asBStrLength(*s) )
	{
		// The script is trying to access memory that isn't
		// allowed so we'll throw an exception.
		asIScriptContext *context = asGetActiveContext();

		// Should this function be called by the host application directly
		// then there will not be any active context to set the exception on
		if( context )
			context->SetException("Out of range");

		// Don't return any objects to the script engine after
		// setting an exception as they will not be released
		return 0;
	}

	return *s + index;
}


asBSTR asBStrFormat(int number)
{
	char str[50];
	sprintf(str, "%d", number);
	int len = (int)strlen(str);

	// We must allocate a new bstr that the script engine will free afterwards
	asBSTR bstr = asBStrAlloc(len);

	memcpy(bstr, str, len);

	return bstr;
}

asBSTR asBStrFormat(unsigned int number)
{
	char str[50];
	sprintf(str, "%u", number);
	int len = (int)strlen(str);

	// We must allocate a new bstr that the script engine will free afterwards
	asBSTR bstr = asBStrAlloc(len);

	memcpy(bstr, str, len);

	return bstr;
}

asBSTR asBStrFormat(float number)
{
	char str[50];
	sprintf(str, "%#6g", number);
	int len = (int)strlen(str);

	// We must allocate a new bstr that the script engine will free afterwards
	asBSTR bstr = asBStrAlloc(len);

	memcpy(bstr, str, len);

	return bstr;
}

asBSTR asBStrFormat(double number)
{
	char str[50];
	sprintf(str, "%#6g", number);
	int len = (int)strlen(str);

	// We must allocate a new bstr that the script engine will free afterwards
	asBSTR bstr = asBStrAlloc(len);

	memcpy(bstr, str, len);

	return bstr;
}

asBSTR asBStrFormatBits(asDWORD bits)
{
	char str[50];
	sprintf(str, "%X", (unsigned int)(bits));
	int len = (int)strlen(str);

	// We must allocate a new bstr that the script engine will free afterwards
	asBSTR bstr = asBStrAlloc(len);

	memcpy(bstr, str, len);

	return bstr;
}

asBSTR asBStrSubstr(const asBSTR &str, asUINT start, asUINT count)
{
	asUINT len = asBStrLength(str);

	len -= start;

	if( len < count ) count = len;

	// We must allocate a new bstr that the script engine will free afterwards
	asBSTR sub = asBStrAlloc(count);

	memcpy(sub, str+start, count);

	return sub;
}

#ifdef AS_USE_NAMESPACE
}
#endif
