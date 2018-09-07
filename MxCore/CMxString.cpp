
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

//gbk char* to utf8 char*
std::string gbk_to_utf8(const char* gbkStr) {
    if (!gbkStr)
        return nullptr;
    
    size_t len = strlen(gbkStr);
    if (len <= 0)
        return nullptr;
    
    iconv_t converter = iconv_open("utf-8", "gb18030");
    size_t uft8StrLen = len * 3 + 1;
    char *uft8Str = new char[uft8StrLen];
    memset(uft8Str, 0, uft8StrLen);
    size_t ret = iconv(converter, (char**)&gbkStr, (size_t*)&len, &uft8Str, (size_t*)&uft8StrLen);
    std::string outStr = uft8Str;
    delete[] uft8Str;
    iconv_close(converter);
    return outStr;
}

//utf8 char* to gbk char*
std::string uft8_to_gbk(const char* utf8Str) {
    if (!utf8Str)
        return "";
    
    size_t len = strlen(utf8Str);
    if (len <= 0)
        return "";
    
    iconv_t converter = iconv_open("gb18030" , "utf-8");
    size_t gbkStrLen = len * 3 + 1;
    char *gbkStr = new char[gbkStrLen];
    memset(gbkStr, 0, gbkStrLen);
    size_t ret = iconv(converter, (char**)&utf8Str, (size_t*)&len, &gbkStr, (size_t*)&gbkStrLen);
    std::string outStr = gbkStr;
    delete[] gbkStr;
    iconv_close(converter);
    return outStr;
}

std::wstring to_cstr(const char* cstr) {
#ifdef _WIN32
    int nChars = ::WideCharToMultiByte( CP_UTF8, 0, buffer, len, NULL, 0, NULL, NULL);
    if (nChars == 0)
        return "";
    
    std::string newbuffer;
    newbuffer.resize(nChars) ;
    ::WideCharToMultiByte( CP_UTF8, 0, buffer, len, const_cast<char*>(newbuffer.c_str()), nChars, NULL, NULL);
    return newbuffer;
#else
    
#endif
}

std::string to_wstr(const char* wstr) {
    
}

//windows  gbk -> utf8
std::string to_utf8(const wchar_t* buffer, int len)
{
#ifdef _WIN32
    int nChars = ::WideCharToMultiByte( CP_UTF8, 0, buffer, len, NULL, 0, NULL, NULL);
    if (nChars == 0)
        return "";
    
    std::string newbuffer;
    newbuffer.resize(nChars) ;
    ::WideCharToMultiByte( CP_UTF8, 0, buffer, len, const_cast<char*>(newbuffer.c_str()), nChars, NULL, NULL);
    return newbuffer;
#endif
    std::string newbuffer = buffer;
}

std::string to_utf8(const std::wstring& str)
{
    return to_utf8(str.c_str(), (int)str.size());
}

// convert UTF-8 string to wstring
std::wstring utf8_to_wstring (const std::string& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.from_bytes(str);
}

// convert wstring to UTF-8 string
std::string wstring_to_utf8 (const std::wstring& str)
{
    std::wstring_convert<std::codecvt_utf8<wchar_t>> myconv;
    return myconv.to_bytes(str);
}

int encodingConvert(const char *tocode, const char *fromcode,
                    char *inbuf, size_t inlength, char *outbuf, size_t outlength)
{
#ifndef _WIN32
    
    char **inbuffer = &inbuf;
    char **outbuffer = &outbuf;
    
    iconv_t cd;
    size_t ret;
    cd = iconv_open(tocode, fromcode);
    if((size_t)cd == -1)
        return -1;
    ret = iconv(cd, inbuffer, &inlength, outbuffer, &outlength);
    if(ret == -1)
        return -1;
    iconv_close(cd);
#endif
    
    return 0;
}

//把wchar_t数据写到文件中， 文件中的数据都是以windows下的格式来存放。
int WriteWstringToBuffer(const std::wstring &wstr, void *buffer)
{
    int inLength = (wstr.size() + 1) * sizeof(wchar_t);
    int outLength = inLength / 2;  //从Linux下写入wchar_t的数据到文件的长度
    char *inBuffer = new char[inLength]();
    memcpy(inBuffer, wstr.c_str(), inLength);
    char *outBuffer = new char[outLength]();
    
    int ret = EncodingCovert("UTF-16LE", "UTF-32LE", inBuffer, inLength, outBuffer, outLength);
    
    memcpy(buffer, outBuffer, outLength);
    
    delete[] inBuffer;
    delete[] outBuffer;
    
    return (ret == -1)?ret:0;
}

//从文件中读取wchar_t数据， 文件中的数据都是以windows下的格式来存放。
int ReadWstringFromBuffer(std::wstring &wstr, void *buffer, int bufLength)
{
    int inLength = bufLength;
    int outLength = inLength * 2;
    char *inBuffer = new char[inLength]();
    memcpy(inBuffer, buffer, inLength);
    char *outBuffer = new char[outLength]();
    
    int ret = EncodingCovert("UTF-32LE", "UTF-LE", inBuffer, inLength, outBuffer, outLength);
    
    wstr = outBuffer;
    
    delete[] inBuffer;
    delete[] outBuffer;
    
    return (ret == -1)?ret:0;
}
