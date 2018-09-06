
#include "stdafx.h"
#include "CMxString.h"
#include "iconv.h"

static char* convert(const char* src, const char* dstEncode, const char* srcEncode) {
	size_t srcLen = strlen(src);
	if (srcLen > 0) {
		iconv_t converter = iconv_open(dstEncode, srcEncode);
		if (converter != (iconv_t)-1) {
			size_t destLen = srcLen * 3 + 1;
			char *dest = new char[destLen];
			memset(dest, 0, destLen);
			size_t ret = iconv(converter, (char**)&src, (size_t*)&srcLen, &dest, (size_t*)&destLen);
			char* retStr = nullptr;
			if (ret > 0) {
				destLen = strlen(dest);
				retStr = new char[destLen + 1];
				memset(retStr, 0, destLen + 1);
				memcpy(retStr, dest, destLen);
			}
			delete[] dest;
			iconv_close(converter);
			return retStr;
		}
	}
	return nullptr;
}

// CMxString采用utf8存储
CMxString::CMxString() {
	__pData = nullptr;
}

CMxString::CMxString(const char *sz, const char* encode = "utf-8") {
	__pData = nullptr;
	size_t len = strlen(sz);
    if (len > 0) {
        if (encode == "utf-8") {
			__pData = new char[len+1];
            memset(__pData, 0, len+1);
            memcpy(__pData, sz, len);
        } else if (encode == "gbk") {
			__pData = convert(sz, "utf-8", "gb18030");
        }
    }
    
    /*
     NSString* Wchar2NSString(wchar_t* inStr, int len)
     {
     size_t size = wcstombs(NULL, inStr, 0);
     char *outStr = new char[size+1];
     memset(outStr, 0, (size+1)*sizeof(char));
     size_t ret = wcstombs(outStr, inStr, size+1);
     if(ret == -1) {
     delete [] outStr;
     outStr = NULL;
     }
     
     NSString* string = [NSString stringWithCString:outStr encoding:NSUTF8StringEncoding];
     delete [] outStr;
     return string;
     }
     
     wchar_t* NSString2Wchar(NSString* inStr)
     {
     
     const char* cString = [inStr cStringUsingEncoding:NSUTF8StringEncoding];
     setlocale(LC_CTYPE, "UTF-8");
     size_t size = mbstowcs(NULL, cString, 0);
     wchar_t *outStr = new wchar_t[size+1];
     if (outStr) {
     memset(outStr, 0, (size+1)*sizeof(wchar_t));
     size_t ret = mbstowcs(outStr, cString, size+1);
     if(ret == -1) {
     delete [] outStr;
     outStr = NULL;
     }
     }
     return outStr;
     }
     
     wstring UTF8ToUnicode( const string& str )
     {
     int  len = 0;
     len = str.length();
     int  unicodeLen = ::MultiByteToWideChar( CP_UTF8,
     0,
     str.c_str(),
     -1,
     NULL,
     0 );
     wchar_t *  pUnicode;
     pUnicode = new  wchar_t[unicodeLen+1];
     memset(pUnicode,0,(unicodeLen+1)*sizeof(wchar_t));
     ::MultiByteToWideChar( CP_UTF8,
     0,
     str.c_str(),
     -1,
     (LPWSTR)pUnicode,
     unicodeLen );
     wstring  rt;
     rt = ( wchar_t* )pUnicode;
     delete  pUnicode;
     
     return  rt;
     }
     
     string UnicodeToUTF8( const wstring& str )
     {
     char*     pElementText;
     int    iTextLen;
     // wide char to multi char
     iTextLen = WideCharToMultiByte( CP_UTF8,
     0,
     str.c_str(),
     -1,
     NULL,
     0,
     NULL,
     NULL );
     pElementText = new char[iTextLen + 1];
     memset( ( void* )pElementText, 0, sizeof( char ) * ( iTextLen + 1 ) );
     ::WideCharToMultiByte( CP_UTF8,
     0,
     str.c_str(),
     -1,
     pElementText,
     iTextLen,
     NULL,
     NULL );
     string strText;
     strText = pElementText;
     delete[] pElementText;
     return strText;
     }
     */
}

CMxString::CMxString(const char *sz) {
#ifdef _WIN32
	CMxString(sz, "gbk");
#else
	CMxString(sz, "utf-8");
#endif // _WIN32
}

CMxCharArray CMxString::cStr() {
#ifdef _WIN32
	char* gbkStr = convert(__pData, "gb18030", "utf-8");
	return CMxCharArray(gbkStr);
#endif
	size_t len = strlen(__pData);
	if (len > 0) {
		char* utf8Str = new char[len + 1];
		memset(__pData, 0, len + 1);
		memcpy(utf8Str, __pData, len);
		return  CMxCharArray(utf8Str);
	}
	return CMxCharArray(nullptr);
}

CMxWCharArray CMxString::wcStr() {
	return CMxWCharArray(nullptr);
}