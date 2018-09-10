#include "stdafx.h"
#include "CMxString.h"
#include "iconv.h"
#include "MxError.h"
#include "MxMemory.h"
#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

MXCORE_API int mxWCharLen(const MxWChar* str) {
	if (!str)
		return 0;

	int len = 0;
	while (*str++)
		len++;
	return len;
}

MXCORE_API long utf8_to_gbk(const char* str, char* dst, int len) {
#ifdef _WIN32
	int insize = (int)strlen(str) + 1;
	WCHAR *unicodestr = (WCHAR *)mxAlloc(insize * 2);
	int ret = MultiByteToWideChar(CP_UTF8, 0, str, -1, unicodestr, insize);
	if (!SUCCEEDED(ret)) {
		mxFree(unicodestr);
		return MX_FAILED;
	}
	ret = WideCharToMultiByte(CP_ACP, 0, unicodestr, -1, dst, len, 0, 0);
	if (!SUCCEEDED(ret)) {
		mxFree(unicodestr);
		return MX_FAILED;
	}
	mxFree(unicodestr);
	return MX_OK;
#elif defined(__APPLE__)
	CFStringRef strref = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8*)str, strlen(str), kCFStringEncodingUTF8, false);
	if (strref)
	{
		CFStringGetCString(strref, (char*)dst, len, kCFStringEncodingDOSChineseSimplif);
		CFRelease(strref);
		return MX_OK;
	}
	return MX_FAILED;
#else
	iconv_t cd = iconv_open("UTF-8", "GB2312");
	if (cd == (iconv_t)-1)
		return MX_FAILED;

	size_t insize = strlen(str);
	size_t outsize = len;
	size_t ret = iconv(cd, (char**)&src, (size_t*)&insize, &dest, &outsize);
	if (ret <= 0) {
		iconv_close(cd);
		return MX_FAILED;
	}
	iconv_close(cd);
	return MX_OK;
#endif
}

MXCORE_API long gbk_to_utf8(const char* str, char* dst, int len) {
#ifdef _WIN32
	int insize = (int)strlen(str) + 1;
	WCHAR *unicodestr = (WCHAR *)mx_malloc(insize * 2);
	int ret = MultiByteToWideChar(CP_ACP, 0, str, -1, unicodestr, insize);
	if (!SUCCEEDED(ret)) {
		mx_free(unicodestr);
		return MX_FAILED;
	}
	ret = WideCharToMultiByte(CP_UTF8, 0, unicodestr, -1, dst, len, 0, 0);
	if (!SUCCEEDED(ret)) {
		mx_free(unicodestr);
		return MX_FAILED;
	}
	mx_free(unicodestr);
	return MX_OK;
#elif defined(__APPLE__)
	CFStringRef strref = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8*)str, strlen(str), kCFStringEncodingDOSChineseSimplif, false);
	if (strref)
	{
		CFStringGetCString(strref, (char*)dst, len, kCFStringEncodingUTF8);
		CFRelease(strref);
		return MX_OK;
	}
	return MX_FAILED;
#else
	iconv_t cd = iconv_open("UTF-8", "GB2312");
	if (cd == (iconv_t)-1)
		return MX_FAILED;

	size_t insize = strlen(str);
	size_t outsize = len;
	size_t ret = iconv(cd, (char**)&src, (size_t*)&insize, &dest, &outsize);
	if (ret <= 0) {
		iconv_close(cd);
		return MX_FAILED;
	}
	iconv_close(cd);
	return MX_OK;
#endif
}

MXCORE_API long utf8_to_utf16(const char* str, MxWChar* dst, int len) {
#ifdef _WIN32
	int ret = MultiByteToWideChar(CP_UTF8, 0, str, strlen(str), (LPWSTR)dst, len);
	return SUCCEEDED(ret) ? MX_OK : MX_FAILED;
#elif defined(__APPLE__)
	CFStringRef strref = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8*)str, strlen(str), kCFStringEncodingUTF8, false);
	if (strref)
	{
		CFStringGetCString(strref, (char*)dst, len, kCFStringEncodingUTF16);
		CFRelease(strref);
		return MX_OK;
	}
	return MX_FAILED;
#else
	iconv_t cd = iconv_open("UTF-16LE", "UTF-8");
	if (cd == (iconv_t)-1)
		return MX_FAILED;

	size_t insize = strlen(str);
	size_t outsize = len;
	size_t ret = iconv(cd, (char**)&src, (size_t*)&insize, &dest, &outsize);
	if (ret <= 0) {
		iconv_close(cd);
		return MX_FAILED;
	}
	iconv_close(cd);
	return MX_OK;
#endif
}

MXCORE_API long gbk_to_utf16(const char* str, MxWChar* dst, int len) {
#ifdef _WIN32
	int ret = MultiByteToWideChar(CP_ACP, 0, str, strlen(str), (LPWSTR)dst, len);
	return SUCCEEDED(ret) ? MX_OK : MX_FAILED;
#elif defined(__APPLE__)
	CFStringRef strref = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8*)str, strlen(str), kCFStringEncodingDOSChineseSimplif, false);
	if (strref)
	{
		CFStringGetCString(strref, (char*)dst, len, kCFStringEncodingUTF16);
		CFRelease(strref);
		return MX_FAILED;
	}
	return MX_OK;
#else
	iconv_t cd = iconv_open("UTF-16LE", "GB2312");
	if (cd == (iconv_t)-1)
		return MX_FAILED;

	size_t insize = strlen(str);
	size_t outsize = len;
	size_t ret = iconv(cd, (char**)&src, (size_t*)&insize, &dest, &outsize);
	if (ret <= 0) {
		iconv_close(cd);
		return MX_FAILED;
	}
	iconv_close(cd);
#endif
	return MX_OK;
}

MXCORE_API long utf16_to_gbk(const MxWChar* str, char* dst, int len) {
#ifdef _WIN32
	int ret = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)str,mxWCharLen(str), dst, len, NULL, NULL);
	return SUCCEEDED(ret) ? MX_OK : MX_FAILED;
#elif defined(__APPLE__)
	CFStringRef strref = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8*)str, mxWCharLen(str) * 2, kCFStringEncodingUTF16, false);
	if (strref)
	{
		CFStringGetCString(strref, (char*)dst, len, kCFStringEncodingDOSChineseSimplif);
		CFRelease(strref);
		return MX_OK;
	}
	return MX_FAILED;
#else
	iconv_t cd = iconv_open("GB2312", "UTF-16LE");
	if (cd == (iconv_t)-1)
		return MX_FAILED;

	size_t insize = mxWCharLen(str)*2;
	size_t outsize = len;
	size_t ret = iconv(cd, (char**)&src, &insize, &dst, &outsize);
	if (ret <= 0) {
		iconv_close(cd);
		return MX_FAILED;
	}
	iconv_close(cd);
	return MX_OK;
#endif
}

MXCORE_API long utf16_to_utf8(const MxWChar* str, char* dst, int lens) {
#ifdef _WIN32
	int ret = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR)str, mxWCharLen(str), dst, lens, NULL, NULL);
	return SUCCEEDED(ret) ? MX_OK : MX_FAILED;
#elif defined(__APPLE__)
	CFStringRef strref = CFStringCreateWithBytes(kCFAllocatorDefault, (const UInt8*)str, mxWCharLen(str) * 2, kCFStringEncodingUTF16, false);
	if (strref)
	{
		CFStringGetCString(strref, (char*)dst, lens, kCFStringEncodingUTF8);
		CFRelease(strref);
		return MX_OK;
	}
	return MX_FAILED;
#else
	iconv_t cd = iconv_open("UTF-8", "UTF-16LE");
	if (cd == (iconv_t)-1)
		return MX_FAILED;

	size_t insize = mxWCharLen(str) * 2;
	size_t outsize = len;
	size_t ret = iconv(cd, (char**)&src, &insize, &dst, &outsize);
	if (ret <= 0) {
		iconv_close(cd);
		return MX_FAILED;
	}
	iconv_close(cd);
	return MX_OK;
#endif
}

CMxString::CMxString() {
	__pData = nullptr;
}
CMxString::~CMxString() {
	if (__pData) {
		mxFree(__pData);
	}
}

CMxString::CMxString(const char *sz) {
#ifdef _WIN32
	CMxString(sz, MxTextEncode_GBK);
#else
	CMxString(sz, MxTextEncode_UTF8);
#endif // _WIN32

}

CMxString::CMxString(const char *sz, MxTextEncode mte) {
	__pData = nullptr;
	int len = strlen(sz);
	if (len > 0) {
		if (MxTextEncode_GBK == mte)
		{
			__pData = (MxWChar*)mxAlloc((len+1)*sizeof(__pData));
			memset(__pData, 0, sizeof(__pData));
			gbk_to_utf16(sz, __pData, len);
		}
		else if (MxTextEncode_UTF8 == mte)
		{
			__pData = (MxWChar*)mx_malloc(len);
			memset(__pData, 0, sizeof(__pData));
			utf8_to_utf16(sz, __pData, len);
		}
	}
}

CMxString::CMxString(const MxWChar *sz) {
	__pData = nullptr;
	int len = mxWCharLen(sz);
	if (len > 0)
	{
		__pData = (MxWChar*)mxAlloc((len + 1) *sizeof(MxWChar));
		memset(__pData, 0, (len + 1) * sizeof(MxWChar));
		memcpy(__pData, sz, len * sizeof(MxWChar));
	}
}
