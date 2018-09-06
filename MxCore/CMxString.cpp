
#include "stdafx.h"
#include "CMxString.h"
#include "iconv.h"

// CMxString采用utf8存储
CMxString::CMxString() {
    __data = nullptr;
}

CMxString::CMxString(char *sz, char* encode = "utf-8") {
    __data = nullptr;
    int len = strlen(sz);
    if (insize > 0) {
        if (encode == "utf-8") {
            __data = new char[len+1];
            memset(__data, 0, len+1);
            memcpy(__data, sz, len);
        } else if (encode == "gbk") {
            iconv_t converter = iconv_open("utf-8","gb18030");
            if (converter != (iconv_t)-1) {
                int outsize = len*3+1;
                char *output = new char[outsize];
                memset(output,0,outsize);
                size_t ret = iconv(converter,(char**)&input,&insize,&output,&outsize);
                if (ret > 0) {
                    len = strlen(output);
                    __data = new char[len+1];
                    memset(__data, 0, len+1);
                    memcpy(__data, output, len);
                }
                delete []output;
                iconv_close(converter);
            }
            
            
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

CMxCharArray CMxString::cString() {
    if (__cdata) {
#ifdef _WIN32
        int iTextLen = ::MultiByteToWideChar( CP_UTF8, 0, str.c_str(), -1, nullptr, 0 );
#else
        
#endif
    }
    
    return CMxCharArray();
}
CMxWCharArray CMxString::wcString() {
    if (__cdata) {
#ifdef _WIN32
        int iTextLen = ::MultiByteToWideChar( CP_UTF8, 0, str.c_str(), -1, nullptr, 0 );
#else
        
#endif
    }
    
    return CMxCharArray();
}

void CMxString::__c2w() {
    if __wdata
    char* __cdata;
    wchar_t* __wdata;
}
void CMxString::__w2c() {
    
}
