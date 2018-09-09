#ifndef __CMXSTRING_H__
#define __CMXSTRING_H__

enum MxTextEncode { MxTextEncode_GBK, MxTextEncode_UTF8, MxTextEncode_UTF16};
typedef unsigned short MxWChar;

MXCORE_API int mxWCharLen(const MxWChar* str);
MXCORE_API long utf8_to_gbk(const char* str, char* dst, int len);
MXCORE_API long gbk_to_utf8(const char* str, char* dst, int len);
MXCORE_API long utf8_to_utf16(const char* str, MxWChar* dst, int len);
MXCORE_API long gbk_to_utf16(const char* str, MxWChar* dst, int len);
MXCORE_API long utf16_to_gbk(const MxWChar* str, char* dst, int len);
MXCORE_API long utf16_to_utf8(const MxWChar* str, char* dst, int len);


class MXCORE_API CMxString {
public:
public:
    CMxString();
	CMxString(const char *sz);
    CMxString(const char *sz, MxTextEncode mte);
	CMxString(const MxWChar *sz);
	~CMxString();
 
	MxWChar* mxStr() { return __pData; }
private:
	MxWChar* __pData;
};

#endif //__CMXSTRING_H__
