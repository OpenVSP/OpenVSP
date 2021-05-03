#ifdef _MSC_VER
/*
    CString class registration for AngelScript.
    
    You are welcome to do modify this file as long as you'll keep the original copyright message.

    Copyright by Gilad Novik, 2006
*/

#pragma once
#include <atlstr.h>
#include <angelscript.h>
#if defined(AS_STRING_REGEX) || defined(AS_STRING_REGEX_ARRAY)
#include <regexpr2.h>
#endif

template<class T>
class asString
{
public:
	template<typename T> class asTraits {};
	template<> class asTraits<CStringA>
	{
	public:
		static void Register(asIScriptEngine* pEngine)
		{
			int nResult;
			nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(double)", asFUNCTION(AssignDoubleToString), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
			nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(int)", asFUNCTION(AssignIntToString), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
			nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(int64)", asFUNCTION(AssignInt64ToString), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
			nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(uint)", asFUNCTION(AssignUIntToString), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );		
			nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(uint64)", asFUNCTION(AssignUInt64ToString), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
			nResult = pEngine->RegisterGlobalFunction("int atoi(const string &in)",asFUNCTION(ToInteger),asCALL_CDECL); ATLASSERT( nResult >= 0 );
			nResult = pEngine->RegisterGlobalFunction("double atof(const string &in)",asFUNCTION(ToDouble),asCALL_CDECL); ATLASSERT( nResult >= 0 );

			nResult = pEngine->RegisterObjectMethod("string", "int remove(uint8)", asMETHOD(CStringA,Remove), asCALL_THISCALL); ATLASSERT( nResult >= 0 );
			nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_INDEX, "uint8 &f(uint)", asFUNCTION(CharAt), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
			nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_INDEX, "const uint8 &f(uint) const", asFUNCTION(CharAt), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
		}

		static void Factory(const char* szString,asUINT nLength,CStringA& szResult)
		{
			szResult.SetString(szString,nLength);
		}

		static CStringA& AssignIntToString(int i,CStringA& szDestination)
		{
			szDestination.Format("%d",i);
			return szDestination;
		}
		static CStringA& AssignInt64ToString(asINT64 i,CStringA& szDestination)
		{
			szDestination.Format("%I64d",i);
			return szDestination;
		}
		static CStringA& AssignUIntToString(unsigned int i,CStringA& szDestination)
		{
			szDestination.Format("%u",i);
			return szDestination;
		}
		static CStringA& AssignUInt64ToString(asQWORD i,CStringA& szDestination)
		{
			szDestination.Format("%I64u",i);
			return szDestination;
		}
		static CStringA& AssignDoubleToString(double f,CStringA& szDestination)
		{
			szDestination.Format("%f",f);
			return szDestination;
		}
		static int ToInteger(const CStringA& szString)
		{
			return atoi(szString);
		}
		static double ToDouble(const CStringA& szString)
		{
			return atof(szString);
		}
		static LPSTR CharAt(unsigned int i,CStringA& szSource)
		{
			if(i>=(unsigned int)szSource.GetLength())
			{
				asIScriptContext* pContext=asGetActiveContext();
				pContext->SetException("Out of range");
				return NULL;
			}
			return szSource.GetBuffer(0)+i;
		}

	};
	template<> class asTraits<CStringW>
	{
	public:
		static void Register(asIScriptEngine* pEngine)
		{
			int nResult;
			nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(double)", asFUNCTION(AssignDoubleToString), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
			nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(int)", asFUNCTION(AssignIntToString), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
			nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(int64)", asFUNCTION(AssignInt64ToString), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
			nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(uint)", asFUNCTION(AssignUIntToString), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );		
			nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(uint64)", asFUNCTION(AssignUInt64ToString), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );		
			nResult = pEngine->RegisterGlobalFunction("int atoi(const string &in)",asFUNCTION(ToInteger),asCALL_CDECL); ATLASSERT( nResult >= 0 );
			nResult = pEngine->RegisterGlobalFunction("double atof(const string &in)",asFUNCTION(ToDouble),asCALL_CDECL); ATLASSERT( nResult >= 0 );

			nResult = pEngine->RegisterObjectMethod("string", "int remove(uint16)", asMETHOD(CStringW,Remove), asCALL_THISCALL); ATLASSERT( nResult >= 0 );
			nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_INDEX, "uint16 &f(uint)", asFUNCTION(CharAt), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
			nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_INDEX, "const uint16 &f(uint) const", asFUNCTION(CharAt), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
		}

		static void Factory(const char* szString,asUINT nLength,CStringW& szResult)
		{
			int nResult=MultiByteToWideChar(CP_UTF8,0,szString,nLength,NULL,0);
			if (nResult>0)			
			{
				nResult=MultiByteToWideChar(CP_UTF8,0,szString,nLength,szResult.GetBuffer(nResult),nResult);
				szResult.ReleaseBuffer(nResult>0 ? nResult : 0);
			}
			else
				szResult.Empty();
		}

		static CStringW& AssignIntToString(int i,CStringW& szDestination)
		{
			szDestination.Format(L"%d",i);
			return szDestination;
		}
		static CStringW& AssignInt64ToString(asINT64 i,CStringW& szDestination)
		{
			szDestination.Format(L"%I64d",i);
			return szDestination;
		}
		static CStringW& AssignUIntToString(unsigned int i,CStringW& szDestination)
		{
			szDestination.Format(L"%u",i);
			return szDestination;
		}
		static CStringW& AssignUInt64ToString(asQWORD i,CStringW& szDestination)
		{
			szDestination.Format(L"%I64u",i);
			return szDestination;
		}
		static CStringW& AssignDoubleToString(double f,CStringW& szDestination)
		{
			szDestination.Format(L"%f",f);
			return szDestination;
		}
		static int ToInteger(const CStringW& szString)
		{
			return _wtoi(szString);
		}
		static double ToDouble(const CStringW& szString)
		{
			return _wtof(szString);
		}
		static LPWSTR CharAt(unsigned int i,CStringW& szSource)
		{
			if(i>=(unsigned int)szSource.GetLength())
			{
				asIScriptContext* pContext=asGetActiveContext();
				pContext->SetException("Out of range");
				return NULL;
			}
			return szSource.GetBuffer(0)+i;
		}

	};
public:
	static T Factory(asUINT nLength,const char* szString)
	{
		T szResult;
		asTraits<T>::Factory(szString,nLength,szResult);
		return szResult;
	}
	static void Construct(T *pStr)
	{
		new(pStr) T();
	}
	static void Construct(const T& szString,T *pStr)
	{
		new(pStr) T(szString);
	}
	static void Construct(const T& szString,unsigned int n,T *pStr)
	{
		new(pStr) T(szString,n);
	}
	static void Destruct(T *pStr)
	{
		pStr->~T();
	}

	static unsigned int Delete(unsigned int i,T& szSource)
	{
		return (unsigned int)szSource.Delete((int)i);
	}
	static unsigned int Delete(unsigned int i,unsigned int n,T& szSource)
	{
		return (unsigned int)szSource.Delete((int)i,(int)n);
	}
	static int Find(const T& szString,T& szSource)
	{
		return szSource.Find(szString);
	}
	static int Find(const T& szString,unsigned int i,T& szSource)
	{
		return szSource.Find(szString,(int)i);
	}
	static T Mid(unsigned int i,T& szSource)
	{
		return szSource.Mid((int)i);
	}
	static T Mid(unsigned int i,unsigned int n,T& szSource)
	{
		return szSource.Mid((int)i,(int)n);
	}
	static void Insert(unsigned int i,const T& szString,T& szSource)
	{
		szSource.Insert(i,szString);
	}
	static void Replace(const T& szString1,const T& szString2,T& szSource)
	{
		szSource.Replace(szString1,szString2);
	}
	static T& Assign(const T& szString,T& szSource)
	{
		return szSource=szString;
	}
	static T& ConCat(const T& szString,T& szSource)
	{
		szSource.Append(szString);
		return szSource;
	}

	static T& AddAssignIntToString(int i,T& szDestination)
	{
		T szBuffer;
		szDestination.Append(asTraits<T>::AssignIntToString(i,szBuffer));
		return szDestination;
	}

	static T& AddAssignInt64ToString(asINT64 i,T& szDestination)
	{
		T szBuffer;
		szDestination.Append(asTraits<T>::AssignInt64ToString(i,szBuffer));
		return szDestination;
	}

	static T& AddAssignUIntToString(unsigned int i,T& szDestination)
	{
		T szBuffer;
		szDestination.Append(asTraits<T>::AssignUIntToString(i,szBuffer));
		return szDestination;
	}

	static T& AddAssignUInt64ToString(asQWORD i,T& szDestination)
	{
		T szBuffer;
		szDestination.Append(asTraits<T>::AssignUInt64ToString(i,szBuffer));
		return szDestination;
	}

	static T& AddAssignDoubleToString(double f,T& szDestination)
	{
		T szBuffer;
		szDestination.Append(asTraits<T>::AssignDoubleToString(f,szBuffer));
		return szDestination;
	}

#ifdef AS_STRING_REGEX
	static T ParseRegexString(const T& szString1, const T& szString2)
	{
		T szResult;
		try
		{
			regex::basic_rpattern<T::PCXSTR> rxExpression((T::PCXSTR)szString2,regex::SINGLELINE|regex::MULTILINE|regex::NOCASE);
			regex::basic_match_results<T::PCXSTR> results;
			if (rxExpression.match((T::PCXSTR)szString1,results).matched)
				for (size_t i=(results.cbackrefs()==1) ? 0 : 1,count=results.cbackrefs();i<count;i++)
					szResult.Append(results.backref(i).first,(int)results.rlength(i));
		}
		catch (regex::bad_regexpr& e)
		{
			asIScriptContext* pContext=asGetActiveContext();
			ATLASSERT(pContext);
			pContext->SetException(e.what());
		}
		return szResult;
	}
#endif

#ifdef AS_STRING_REGEX_ARRAY
	static asIScriptArray& ParseRegexArray(const T& szExpression,const T& szFlags,T& szData)
	{
		asIScriptContext* pContext=asGetActiveContext();
		ATLASSERT(pContext);
		asIScriptEngine* pEngine=pContext->GetEngine();
		ATLASSERT(pEngine);
		asIScriptArray* pArray=(asIScriptArray*)pEngine->CreateScriptObject(pEngine->GetTypeIdByDecl(0,"string[]"));
		ATLASSERT(pArray);
		try
		{
			regex::REGEX_FLAGS nFlags=regex::NOFLAGS;
			if (szFlags.Find('i')>=0)
				nFlags|=regex::NOCASE;
			if (szFlags.Find('m')>=0)
				nFlags|=regex::MULTILINE;
			if (szFlags.Find('s')>=0)
				nFlags|=regex::SINGLELINE;
			if (szFlags.Find('r')>=0)
				nFlags|=regex::RIGHTMOST;
			if (szFlags.Find('x')>=0)
				nFlags|=regex::EXTENDED;
			regex::basic_rpattern<T::PCXSTR> rxExpression((T::PCXSTR)szExpression,nFlags);
			regex::basic_match_results<T::PCXSTR> Results;
			if (szFlags.Find('g')<0)
			{	// Single search
				if (rxExpression.match((T::PCXSTR)szData,Results))
				{
					size_t nCount=rxExpression.cgroups()-1;
					if (nCount>=1)
					{
						pArray->Resize(nCount);
						for (size_t nIndex=1;nIndex<=nCount;nIndex++)
							((T*)pArray->GetElementPointer(nIndex-1))->SetString(Results.backref(nIndex).first,Results.rlength(nIndex));
					}
					else
					{
						pArray->Resize(1);
						((T*)pArray->GetElementPointer(0))->SetString(Results.backref(0).first,Results.rlength(0));
					}
				}
			}
			else
			{	// Global search
				size_t nGroups=rxExpression.cgroups()-1;
				asUINT nStart=0;
				for (T::PCXSTR szBegin=szData;rxExpression.match(szBegin,Results);szBegin=Results.backref(0).second)
				{
					if (nGroups>=1)
					{
						pArray->Resize(nStart+nGroups);
						for (size_t nIndex=1;nIndex<=nGroups;nIndex++,nStart++)
							((T*)pArray->GetElementPointer(nStart))->SetString(Results.backref(nIndex).first,Results.rlength(nIndex));
					}
					else
					{
						pArray->Resize(++nStart);
						((T*)pArray->GetElementPointer(nStart-1))->SetString(Results.backref(0).first,Results.rlength(0));
					}
				}
			}
		}
		catch (regex::bad_regexpr& e)
		{
			pContext->SetException(e.what());
		}
		return *pArray;
	}
#endif

	static void Register(asIScriptEngine* pEngine)
	{
		int nResult;

		// Register the CString type
		nResult = pEngine->RegisterObjectType("string",sizeof(T),asOBJ_CLASS_CDA); ATLASSERT( nResult >= 0 );

		// Register the CString factory
		nResult = pEngine->RegisterStringFactory("string", asFUNCTION(Factory), asCALL_CDECL); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,  "void f()",                      asFUNCTIONPR(Construct,(T*),void), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,  "void f(const string &in)",      asFUNCTIONPR(Construct,(const T&,T*),void), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_CONSTRUCT,  "void f(const string &in,uint)", asFUNCTIONPR(Construct,(const T&,unsigned int,T*),void), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_DESTRUCT,   "void f()",                      asFUNCTION(Destruct), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_ASSIGNMENT, "string &f(const string &in)",   asFUNCTION(Assign), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_ADD_ASSIGN, "string &f(const string &in)",   asFUNCTION(ConCat), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );

		// Register the global operator overloads
		nResult = pEngine->RegisterGlobalBehaviour(asBEHAVE_EQUAL,       "bool f(const string &in, const string &in)",   asFUNCTIONPR(operator==, (const T&, const T&), bool), asCALL_CDECL); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterGlobalBehaviour(asBEHAVE_NOTEQUAL,    "bool f(const string &in, const string &in)",   asFUNCTIONPR(operator!=, (const T&, const T&), bool), asCALL_CDECL); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterGlobalBehaviour(asBEHAVE_LEQUAL,      "bool f(const string &in, const string &in)",   asFUNCTIONPR(operator<=, (const T&, const T&), bool), asCALL_CDECL); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterGlobalBehaviour(asBEHAVE_GEQUAL,      "bool f(const string &in, const string &in)",   asFUNCTIONPR(operator>=, (const T&, const T&), bool), asCALL_CDECL); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterGlobalBehaviour(asBEHAVE_LESSTHAN,    "bool f(const string &in, const string &in)",   asFUNCTIONPR(operator< , (const T&, const T&), bool), asCALL_CDECL); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterGlobalBehaviour(asBEHAVE_GREATERTHAN, "bool f(const string &in, const string &in)",   asFUNCTIONPR(operator> , (const T&, const T&), bool), asCALL_CDECL); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterGlobalBehaviour(asBEHAVE_ADD,         "string f(const string &in, const string &in)", asFUNCTIONPR(operator+ , (const T&, const T&), T), asCALL_CDECL); ATLASSERT( nResult >= 0 );

		// Register the object methods
		nResult = pEngine->RegisterObjectMethod("string", "uint length() const", asMETHOD(T,GetLength), asCALL_THISCALL); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectMethod("string", "bool isEmpty() const", asMETHOD(T,IsEmpty), asCALL_THISCALL); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectMethod("string", "void empty()", asMETHOD(T,Empty), asCALL_THISCALL); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectMethod("string", "uint delete(uint)", asFUNCTIONPR(Delete,(unsigned int,T&),unsigned int), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectMethod("string", "uint delete(uint,uint)", asFUNCTIONPR(Delete,(unsigned int,unsigned int,T&),unsigned int), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectMethod("string", "int find(const string &in) const", asFUNCTIONPR(Find,(const T&,T&),int), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectMethod("string", "int find(const string &in,uint) const", asFUNCTIONPR(Find,(const T&,unsigned int,T&),int), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectMethod("string", "void insert(uint,const string &in)", asFUNCTIONPR(Insert,(unsigned int,const T&),void), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectMethod("string", "string left(uint) const", asMETHOD(T,Left), asCALL_THISCALL); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectMethod("string", "string right(uint) const", asMETHOD(T,Right), asCALL_THISCALL); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectMethod("string", "string mid(uint) const", asFUNCTIONPR(Mid,(unsigned int,T&),T), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectMethod("string", "string mid(uint,uint) const", asFUNCTIONPR(Mid,(unsigned int,unsigned int,T&),T), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectMethod("string", "void replace(const string &in,const string &in)", asFUNCTION(Replace), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
#ifdef AS_STRING_REGEX
		nResult = pEngine->RegisterGlobalBehaviour(asBEHAVE_MODULO, "string f(const string &in, const string &in)", asFUNCTION(ParseRegexString), asCALL_CDECL); ATLASSERT( nResult >= 0 );
#endif
#ifdef AS_STRING_REGEX_ARRAY
		nResult = pEngine->RegisterObjectMethod("string", "string[]@ parse(const string &in, const string &in) const", asFUNCTION(ParseRegexArray), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
#endif

		// Automatic conversion from values
		asTraits<T>::Register(pEngine);
		nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_ADD_ASSIGN, "string &f(double)", asFUNCTION(AddAssignDoubleToString), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_ADD_ASSIGN, "string &f(int)", asFUNCTION(AddAssignIntToString), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_ADD_ASSIGN, "string &f(int64)", asFUNCTION(AddAssignInt64ToString), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_ADD_ASSIGN, "string &f(uint)", asFUNCTION(AddAssignUIntToString), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
		nResult = pEngine->RegisterObjectBehaviour("string", asBEHAVE_ADD_ASSIGN, "string &f(uint64)", asFUNCTION(AddAssignUInt64ToString), asCALL_CDECL_OBJLAST); ATLASSERT( nResult >= 0 );
	}
};
#endif
