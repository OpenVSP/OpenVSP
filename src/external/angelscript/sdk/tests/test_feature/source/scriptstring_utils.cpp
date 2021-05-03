#include <assert.h>
#include "scriptstring.h"
#include "../../../add_on/scriptarray/scriptarray.h"
#include <string.h> // strstr


BEGIN_AS_NAMESPACE

// This function returns a string containing the substring of the input string
// determined by the starting index and count of characters.
//
// AngelScript signature:
// string@ substring(const string &in str, int start, int count)
static void StringSubString_Generic(asIScriptGeneric *gen)
{
    // Get the arguments
    CScriptString *str = *(CScriptString**)gen->GetAddressOfArg(0);
    int start = *(int*)gen->GetAddressOfArg(1);
    int count = *(int*)gen->GetAddressOfArg(2);

	// We really should be checking for out-of-bounds situations, but I won't spend more time on this code

    // Create the substring
    CScriptString *sub = new CScriptString();
    sub->buffer = str->buffer.substr(start,count);

    // Return the substring
    *(CScriptString**)gen->GetAddressOfReturnLocation() = sub;
}



// This function returns the index of the first position where the substring
// exists in the input string. If the substring doesn't exist in the input
// string -1 is returned.
//
// AngelScript signature:
// int findFirst(const string &in str, const string &in sub, int start)
static void StringFindFirst_Generic(asIScriptGeneric *gen)
{
    // Get the arguments
    CScriptString *str = *(CScriptString**)gen->GetAddressOfArg(0);
    CScriptString *sub = *(CScriptString**)gen->GetAddressOfArg(1);
    int start = *(int*)gen->GetAddressOfArg(2);

    // Find the substring
    int loc = (int)str->buffer.find(sub->buffer, start);

    // Return the result
    *(int*)gen->GetAddressOfReturnLocation() = loc;
}


// This function returns the index of the last position where the substring
// exists in the input string. If the substring doesn't exist in the input
// string -1 is returned.
//
// AngelScript signature:
// int findLast(const string &in str, const string &in sub, int start)
static void StringFindLast_Generic(asIScriptGeneric *gen)
{
    // Get the arguments
    CScriptString *str = *(CScriptString**)gen->GetAddressOfArg(0);
    CScriptString *sub = *(CScriptString**)gen->GetAddressOfArg(1);
    int start = *(int*)gen->GetAddressOfArg(2);

    // Find the substring
    int loc = (int)str->buffer.rfind(sub->buffer, start);

    // Return the result
    *(int*)gen->GetAddressOfReturnLocation() = loc;
}




// This function returns the index of the first character that is in
// the specified set of characters. If no such character is found -1 is
// returned.
//
// AngelScript signature:
// int findFirstOf(const string &in str, const string &in chars, int start)
static void StringFindFirstOf_Generic(asIScriptGeneric *gen)
{
    // Get the arguments
    CScriptString *str = *(CScriptString**)gen->GetAddressOfArg(0);
    CScriptString *chars = *(CScriptString**)gen->GetAddressOfArg(1);
    int start = *(int*)gen->GetAddressOfArg(2);

    // Find the substring
    int loc = (int)str->buffer.find_first_of(chars->buffer, start);

    // Return the result
    *(int*)gen->GetAddressOfReturnLocation() = loc;
}



// This function returns the index of the first character that is not in
// the specified set of characters. If no such character is found -1 is
// returned.
//
// AngelScript signature:
// int findFirstNotOf(const string &in str, const string &in chars, int start)
static void StringFindFirstNotOf_Generic(asIScriptGeneric *gen)
{
    // Get the arguments
    CScriptString *str = *(CScriptString**)gen->GetAddressOfArg(0);
    CScriptString *chars = *(CScriptString**)gen->GetAddressOfArg(1);
    int start = *(int*)gen->GetAddressOfArg(2);

    // Find the substring
    int loc = (int)str->buffer.find_first_not_of(chars->buffer, start);

    // Return the result
    *(int*)gen->GetAddressOfReturnLocation() = loc;
}



// This function returns the index of the last character that is in
// the specified set of characters. If no such character is found -1 is
// returned.
//
// AngelScript signature:
// int findLastOf(const string &in str, const string &in chars, int start)
static void StringFindLastOf_Generic(asIScriptGeneric *gen)
{
    // Get the arguments
    CScriptString *str = *(CScriptString**)gen->GetAddressOfArg(0);
    CScriptString *chars = *(CScriptString**)gen->GetAddressOfArg(1);
    int start = *(int*)gen->GetAddressOfArg(2);

    // Find the substring
    int loc = (int)str->buffer.find_last_of(chars->buffer, start);

    // Return the result
    *(int*)gen->GetAddressOfReturnLocation() = loc;
}




// This function returns the index of the last character that is not in
// the specified set of characters. If no such character is found -1 is
// returned.
//
// AngelScript signature:
// int findLastNotOf(const string &in str, const string &in chars, int start)
static void StringFindLastNotOf_Generic(asIScriptGeneric *gen)
{
    // Get the arguments
    CScriptString *str = *(CScriptString**)gen->GetAddressOfArg(0);
    CScriptString *chars = *(CScriptString**)gen->GetAddressOfArg(1);
    int start = *(int*)gen->GetAddressOfArg(2);

    // Find the substring
    int loc = (int)str->buffer.find_last_not_of(chars->buffer, start);

    // Return the result
    *(int*)gen->GetAddressOfReturnLocation() = loc;
}




// This function takes an input string and splits it into parts by looking
// for a specified delimiter. Example:
//
// string str = "A|B||D";
// array<string@>@ array = split(str, "|");
//
// The resulting array has the following elements:
//
// {"A", "B", "", "D"}
//
// AngelScript signature:
// array<string@>@ split(const string &in str, const string &in delim)
static void StringSplit_Generic(asIScriptGeneric *gen)
{
    // Obtain a pointer to the engine
    asIScriptContext *ctx = asGetActiveContext();
    asIScriptEngine *engine = ctx->GetEngine();

	asITypeInfo *arrayType = engine->GetTypeInfoByDecl("array<string@>");

    // Create the array object
    CScriptArray *array = CScriptArray::Create(arrayType);

    // Get the arguments
    CScriptString *str   = *(CScriptString**)gen->GetAddressOfArg(0);
    CScriptString *delim = *(CScriptString**)gen->GetAddressOfArg(1);

    // Find the existence of the delimiter in the input string
    int pos = 0, prev = 0, count = 0;
    while( (pos = (int)str->buffer.find(delim->buffer, prev)) != (int)std::string::npos )
    {
        // Add the part to the array
        CScriptString *part = new CScriptString();
        part->buffer.assign(&str->buffer[prev], pos-prev);
        array->Resize(array->GetSize()+1);
        *(CScriptString**)array->At(count) = part;

        // Find the next part
        count++;
        prev = pos + (int)delim->buffer.length();
    }

    // Add the remaining part
    CScriptString *part = new CScriptString();
    part->buffer.assign(&str->buffer[prev]);
    array->Resize(array->GetSize()+1);
    *(CScriptString**)array->At(count) = part;

    // Return the array by handle
    *(CScriptArray**)gen->GetAddressOfReturnLocation() = array;
}



// This function takes as input an array of string handles as well as a
// delimiter and concatenates the array elements into one delimited string.
// Example:
//
// array<string@> array = {"A", "B", "", "D"};
// string str = join(array, "|");
//
// The resulting string is:
//
// "A|B||D"
//
// AngelScript signature:
// string@ join(const array<string@> &in array, const string &in delim)
static void StringJoin_Generic(asIScriptGeneric *gen)
{
    // Get the arguments
    CScriptArray  *array = *(CScriptArray**)gen->GetAddressOfArg(0);
    CScriptString *delim = *(CScriptString**)gen->GetAddressOfArg(1);

    // Create the new string
    CScriptString *str = new CScriptString();
    int n;
    for( n = 0; n < (int)array->GetSize() - 1; n++ )
    {
        CScriptString *part = *(CScriptString**)array->At(n);
        str->buffer += part->buffer;
        str->buffer += delim->buffer;
    }

    // Add the last part
    CScriptString *part = *(CScriptString**)array->At(n);
    str->buffer += part->buffer;

    // Return the string
    *(CScriptString**)gen->GetAddressOfReturnLocation() = str;
}



// This is where the utility functions are registered.
// The string type must have been registered first.
void RegisterScriptStringUtils(asIScriptEngine *engine)
{
    int r;

    r = engine->RegisterGlobalFunction("string@ substring(const string &in, int, int)", asFUNCTION(StringSubString_Generic), asCALL_GENERIC); assert(r >= 0);
    r = engine->RegisterGlobalFunction("int findFirst(const string &in, const string &in, int a = 0)", asFUNCTION(StringFindFirst_Generic), asCALL_GENERIC); assert(r >= 0);
    r = engine->RegisterGlobalFunction("int findFirstOf(const string &in, const string &in, int a = 0)", asFUNCTION(StringFindFirstOf_Generic), asCALL_GENERIC); assert(r >= 0);
    r = engine->RegisterGlobalFunction("int findFirstNotOf(const string &in, const string &in, int a = 0)", asFUNCTION(StringFindFirstNotOf_Generic), asCALL_GENERIC); assert(r >= 0);
    r = engine->RegisterGlobalFunction("int findLast(const string &in, const string &in, int a = -1)", asFUNCTION(StringFindLast_Generic), asCALL_GENERIC); assert(r >= 0);
    r = engine->RegisterGlobalFunction("int findLastOf(const string &in, const string &in, int a = -1)", asFUNCTION(StringFindLastOf_Generic), asCALL_GENERIC); assert(r >= 0);
    r = engine->RegisterGlobalFunction("int findLastNotOf(const string &in, const string &in, int a = -1)", asFUNCTION(StringFindLastNotOf_Generic), asCALL_GENERIC); assert(r >= 0);
    r = engine->RegisterGlobalFunction("array<string@>@ split(const string &in, const string &in)", asFUNCTION(StringSplit_Generic), asCALL_GENERIC); assert(r >= 0);
    r = engine->RegisterGlobalFunction("string@ join(const array<string@> &in, const string &in)", asFUNCTION(StringJoin_Generic), asCALL_GENERIC); assert(r >= 0);
}

END_AS_NAMESPACE
